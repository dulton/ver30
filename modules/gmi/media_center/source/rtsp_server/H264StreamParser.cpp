#include <assert.h>
#include "gmi_config_api.h"
#include "gmi_media_ctrl.h"
#include "gmi_system_headers.h"
#include "GroupsockHelper.hh"
#include "H264StreamParser.hh"
#include "MyH264VideoStreamFramer.hh"
#include "ipc_fw_v3.x_resource.h"
#include "ipc_media_data_dispatch.h"
#include "share_memory_log_client.h"

#define USR_DEFINED_ERROR			2

//+++base64encode+++
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static void base64encode(char *out,const unsigned char* in,int len)
{
    for (; len >= 3; len -= 3)
    {
        *out ++ = cb64[ in[0]>>2];
        *out ++ = cb64[ ((in[0]&0x03)<<4)|(in[1]>>4) ];
        *out ++ = cb64[ ((in[1]&0x0f) <<2)|((in[2] & 0xc0)>>6) ];
        *out ++ = cb64[ in[2]&0x3f];
        in += 3;
    }
    if (len > 0)
    {
        unsigned char fragment;
        *out ++ = cb64[ in[0]>>2];
        fragment =	(in[0] &0x03)<<4 ;
        if (len > 1)
            fragment |= in[1] >>4 ;
        *out ++ = cb64[ fragment ];
        *out ++ = (len <2) ? '=' : cb64[ ((in[1]&0x0f)<<2)];
        *out ++ = '=';
    }
    *out = '\0';
}
//---base64encode---

static int findNalu_amba (NALU * nalus, u_int32_t * count, u_int8_t *bitstream, u_int32_t streamSize)
{
    int index = -1;
    u_int8_t * bs = bitstream;
    u_int32_t head;
    u_int8_t nalu_type;

    u_int8_t *last_byte_bitstream = bitstream + streamSize - 1;

    while (bs <= last_byte_bitstream)
    {

        head = (bs[3] << 24) | (bs[2] << 16) | (bs[1] << 8) | bs[0];
        //	printf("head 0x%x: 0x%x, 0x%x, 0x%x, 0x%x\n", head, bs[0], bs[1], bs[2], bs[3]);

        if (head == 0x01000000)  	// little ending
        {
            index++;
            // we find a nalu
            bs += 4;		// jump to nalu type
            nalu_type = 0x1F & bs[0];
            nalus[index].nalu_type = nalu_type;
            nalus[index].addr = bs;

            if (index  > 0)  	// Not the first NALU in this stream
            {
                nalus[index -1].size = nalus[index].addr - nalus[index -1].addr - 4; // cut off 4 bytes of delimiter
            }
//			printf("	nalu type %d\n", nalus[index].nalu_type);	//jay

            if ((nalu_type == NALU_TYPE_NONIDR) ||
                    (nalu_type == NALU_TYPE_IDR))
            {
                // Calculate the size of the last NALU in this stream
                nalus[index].size =  last_byte_bitstream - bs + 1;
                break;
            }
        }
        else if (bs[3] != 0)
        {
            bs += 4;
        }
        else if (bs[2] != 0)
        {
            bs += 3;
        }
        else if (bs[1] != 0)
        {
            bs += 2;
        }
        else
        {
            bs += 1;
        }
    }

    *count = index + 1;
    if (*count == 0)
    {
        //	printf("No nalu found in the bitstream!\n");
        return -1;
    }
    return 0;
}


H264BitStreamParser::H264BitStreamParser( int32_t streamID, uint16_t IPCMediaDataDispatchServerPort, uint16_t IPCMediaDataDispatchClientPort )
    : fsps(NULL), fpps(NULL), fTo(NULL), fNaluCurrIndex(0), fNaluCount(0), fStreamID(streamID), fConnected(false), fMediaDataClient(), fFrameBuffer( NULL ), fMaxFrameLength( 1024*1024 ), fExtraData( NULL ), fMaxExtraDataLength( 1024 )
    , fIPCMediaDataDispatchServerPort( IPCMediaDataDispatchServerPort ), fIPCMediaDataDispatchClientPort( IPCMediaDataDispatchClientPort )
    , fFrameCount( 0 ), fTotalDelay( 0 ), fMaxDelay( 0 ), fMinDelay( 1000000 ), fTotalFrameInterval(0), fMaxFrameInterval(0), fMinFrameInterval(1000000)
#if SAVE_FILE
    , fVideoFile( NULL )
#endif
{
    fLastFrameTimeStamp.tv_sec = 0;
    fLastFrameTimeStamp.tv_usec = 0;
#if SAVE_FILE
    char_t FileName[MAX_PATH_LENGTH];
#if defined( __linux__ )
    sprintf( FileName, "%p_H264.264", this );
#elif defined( _WIN32 )
    sprintf_s( FileName, MAX_PATH_LENGTH, "%p_H264.264", this );
#endif
    printf( "File Name : %s\n", FileName );
#if defined( __linux__ )
    fVideoFile = fopen( FileName, "wb" );
#elif defined( _WIN32 )
    fopen_s( &fVideoFile, FileName, "wb" );
#endif
#endif
}

H264BitStreamParser::~H264BitStreamParser()
{
#if SAVE_FILE
    if ( NULL != fVideoFile )
    {
        fclose( fVideoFile );
        fVideoFile = NULL;
    }
#endif

    if ( fConnected )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264BitStreamParser::~H264BitStreamParser, unreigster from media center server... \n" );
        GMI_RESULT Result = fMediaDataClient.Unregister();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264BitStreamParser::~H264BitStreamParser, MediaDataClient.Unregister return %x \n", (uint32_t) Result );
        fMediaDataClient.Deinitialize();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264BitStreamParser::~H264BitStreamParser, MediaDataClient.Deinitialize return %x \n", (uint32_t) Result );

        BaseMemoryManager::Instance().Deletes( fFrameBuffer );
        fFrameBuffer = NULL;
        fMaxFrameLength = 0;

        BaseMemoryManager::Instance().Deletes( fExtraData );
        fExtraData = NULL;
        fMaxExtraDataLength = 0;
    }
    if (fsps)
        delete[] fsps;
    if (fpps)
        delete[] fpps;
}

void H264BitStreamParser:: registerReadInterest(unsigned char* to,unsigned maxSize)
{
    fTo = to;
    fLimit = to + maxSize;
}

int H264BitStreamParser::parse()
{
    if ( !fConnected )
    {
        fFrameBuffer = BaseMemoryManager::Instance().News<uint8_t>( fMaxFrameLength );
        if ( NULL == fFrameBuffer )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264BitStreamParser::parse, allocating frame buffer failed(%d bytes), no good way to tell upper caller what happened, so we exit for now \n", fMaxFrameLength );
            exit(1);
        }

        fExtraData = BaseMemoryManager::Instance().News<uint8_t>( fMaxExtraDataLength );
        if ( NULL == fExtraData )
        {
            BaseMemoryManager::Instance().Deletes( fFrameBuffer );
            fFrameBuffer = NULL;
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264BitStreamParser::parse, allocating extra data buffer failed(%d bytes), no good way to tell upper caller what happened, so we exit for now \n", fMaxExtraDataLength );
            exit(1);
        }

        GMI_RESULT Result = fMediaDataClient.Initialize( fIPCMediaDataDispatchClientPort, ONVIF_STREAM_APPLICATION_ID );
        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264BitStreamParser::parse, MediaDataClient.Initialize(ClientPort=%d,AppId=%d) return %x, no good way to tell upper caller what happened, so we exit for now \n",
                       ntohs(fIPCMediaDataDispatchClientPort), ONVIF_STREAM_APPLICATION_ID, (uint32_t) Result );
            exit(1);
        }

        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264BitStreamParser::parse, reigster to media center server... \n" );

        do
        {
            Result = fMediaDataClient.Register( fIPCMediaDataDispatchServerPort, MEDIA_VIDEO_H264, (uint32_t)fStreamID, true, NULL, NULL );
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Loop, "H264BitStreamParser::parse, m_MediaDataClient.Register(fIPCMediaDataDispatchServerPort=%d,MediaType=%d, fStreamID=%d, Result=%x\n",
                       ntohs(fIPCMediaDataDispatchServerPort), MEDIA_VIDEO_H264, fStreamID, (int32_t) Result );
            if ( GMI_WAIT_TIMEOUT == Result )
            {
                GMI_Sleep( 100 );//temp code, later redesign this piece of code
            }
        }
        while ( GMI_WAIT_TIMEOUT == Result );

        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264BitStreamParser::parse, register to media center server failed, MediaDataClien.Register return %x, no good way to tell upper caller what happened, so we exit for now \n", (uint32_t) Result );
            exit(1);
        }

        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264BitStreamParser::parse, reigstered to media center server \n" );
        fConnected = true;
    }

    while (fNaluCurrIndex >= fNaluCount)   	//No NAL unit currently in the buffer.	Read a new one.
    {

        GMI_RESULT Result = GMI_FAIL;
        size_t FrameLength = 0;
        size_t ExtraDataLength = 0;
        do
        {
            FrameLength = fMaxFrameLength;
            ExtraDataLength = fMaxExtraDataLength;

            Result = fMediaDataClient.Read( fFrameBuffer, &FrameLength, &fPTS, fExtraData, &ExtraDataLength );

            if ( FAILED( Result ) )
            {
                switch ( Result )
                {
                case GMI_TRY_AGAIN_ERROR:
                {
                    continue;
                }
                default:
                    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264BitStreamParser::parse, unexpected error, Result=%x \n", (int32_t)Result );
                    break;
                }
            }
            else
            {
                break;
            }
        }
        while ( 1 );

        if ( SUCCEEDED( Result ) )
        {
#if SAVE_FILE
            fwrite( fFrameBuffer, 1, FrameLength, fVideoFile );
#endif
            findNalu_amba(fNalus, &fNaluCount, fFrameBuffer, FrameLength);
            //printf( " H264BitStreamParser::parse, FrameTS=%d---%d \n", fPTS.tv_sec, fPTS.tv_usec );
            //ExtMediaEncInfo *EncInfo = (ExtMediaEncInfo *) fExtraData;
            //printf( " H264BitStreamParser::parse, FrameType=%d, FrameNumber=%d, Length=%d \n", EncInfo->s_FrameType, EncInfo->s_FrameNum, EncInfo->s_Length );

            // calculate video delay, uint is microsecond
            ++fFrameCount;
//#if defined( _WIN32 )
            struct timeval CurrentSystemTime;
            gettimeofday1( &CurrentSystemTime, NULL );
            uint64_t Delay = ((uint64_t)CurrentSystemTime.tv_sec - (uint64_t)fPTS.tv_sec)*1000000 + (uint64_t)CurrentSystemTime.tv_usec - (uint64_t)fPTS.tv_usec;
//#else
//            struct timespec CurrentSystemTime;
//            clock_gettime( CLOCK_MONOTONIC, &CurrentSystemTime );
//            uint64_t Delay = ((uint64_t)CurrentSystemTime.tv_sec - (uint64_t)fPTS.tv_sec)*1000000 + (uint64_t)CurrentSystemTime.tv_nsec/1000 - (uint64_t)fPTS.tv_usec;
//#endif
            if ( Delay > fMaxDelay )
            {
                fMaxDelay = Delay;
            }
            else if ( Delay < fMinDelay )
            {
                fMinDelay = Delay;
            }
            fTotalDelay += Delay;

            if ( 0 == fLastFrameTimeStamp.tv_sec && 0 == fLastFrameTimeStamp.tv_usec )
            {
                fLastFrameTimeStamp = fPTS;
            }

            uint64_t Interval = ((uint64_t)fPTS.tv_sec - (uint64_t)fLastFrameTimeStamp.tv_sec)*1000000 + (uint64_t)fPTS.tv_usec - (uint64_t)fLastFrameTimeStamp.tv_usec;
            if ( Interval > fMaxFrameInterval )
            {
                fMaxFrameInterval = Interval;
            }
            else if ( Interval < fMinFrameInterval && 0 != Interval )
            {
                fMinFrameInterval = Interval;
            }
            fTotalFrameInterval += Interval;
            fLastFrameTimeStamp = fPTS;

            if ( 0 == fFrameCount%600 )
            {
                uint64_t AverageDelay = fTotalDelay/fFrameCount;
                printf( "StreamID=%d, FrameCount=%llu, TotalDelay=%llu, Delay=%llu, MaxDelay=%llu, MinDelay=%llu, AverageDelay=%llu, (uint:us) \n", fStreamID, fFrameCount, fTotalDelay, Delay, fMaxDelay, fMinDelay, AverageDelay );

                uint64_t AverageFrameInterval = fTotalFrameInterval/(fFrameCount-1);
                printf( "MediaId=%d, FrameCount=%llu, TotalFrameInterval=%llu, Interval=%lld, MaxFrameInterval=%llu, MinFrameInterval=%llu, AverageFrameInterval=%llu, (uint:us) \n", fStreamID, fFrameCount, fTotalFrameInterval, Interval, fMaxFrameInterval, fMinFrameInterval, AverageFrameInterval );
            }
        }
        fNaluCurrIndex = 0;
    }

    // filter out the SEI nalu
    // filter out the AUD nalu
    while (fNalus[fNaluCurrIndex].nalu_type == NALU_TYPE_SEI || fNalus[fNaluCurrIndex].nalu_type == NALU_TYPE_AUD)
    {
        assert(fNaluCurrIndex <= fNaluCount);
        fNaluCurrIndex++;
    }
    assert((fNaluSize = fNalus[fNaluCurrIndex].size) > 0);
    fNaluType = fNalus[fNaluCurrIndex].nalu_type;

    if (fTo != NULL)
    {
        memcpy(fTo, fNalus[fNaluCurrIndex].addr, fNalus[fNaluCurrIndex].size);
    }
    else
    {
        if (fNaluType == NALU_TYPE_SPS)
        {
            unsigned char* sps = fNalus[fNaluCurrIndex].addr;
            if (fsps != NULL)
                delete[] fsps;
            fsps = new char[fNaluSize/3 *4 + 4 + 4]; // 1 extra byte for '\0'
            base64encode(fsps, sps, fNaluSize);
            memcpy(profileLevelID, sps + 1, 3);
        }
        if (fNaluType == NALU_TYPE_PPS)
        {
            unsigned char* pps = fNalus[fNaluCurrIndex].addr;
            if (fpps != NULL)
                delete[] fpps;
            fpps = new char[fNaluSize/3 *4 + 4 + 4]; // 1 extra byte for '\0'
            base64encode(fpps, pps, fNaluSize);
        }
    }
    fNaluCurrIndex++;
    return 0;
}

char* H264BitStreamParser::getParsersps()
{
    return fsps;
}

char* H264BitStreamParser::getParserpps()
{
    return fpps;
}
char* H264BitStreamParser::getPreID()
{
    return profileLevelID;
}

//H264FileStreamParser
H264FileStreamParser::H264FileStreamParser(FramedSource * usingSource, FramedSource * inputSource)
    :StreamParser(inputSource, FramedSource::handleClosure, usingSource, &MyH264VideoStreamFramer::continueReadProcessing, usingSource),
     fUsingSource(usingSource),fsps(NULL),fpps(NULL), profileLevelID(0), fCurrentParseState(PARSING_START_SEQUENCE)
{

}

H264FileStreamParser::~H264FileStreamParser()
{
    delete[] fsps;
    delete[] fpps;
}

//reset saved the parser state
void H264FileStreamParser::restoreSavedParserState()
{
    StreamParser::restoreSavedParserState();
    fTo = fSavedTo;
    fNumTruncatedBytes = fSavedNumTruncatedBytes;
}

void H264FileStreamParser::setParseState(MyH264ParseState parseState)
{
    fSavedTo = fTo;
    fSavedNumTruncatedBytes = fNumTruncatedBytes;
    fCurrentParseState = parseState;
    saveParserState();
}

unsigned H264FileStreamParser::getParseState()
{
    return fCurrentParseState;
}

void H264FileStreamParser:: registerReadInterest(unsigned char* to,unsigned maxSize)
{
    fStartOfFrame = fTo = fSavedTo = to;
    fLimit = to + maxSize;
    fNumTruncatedBytes = fSavedNumTruncatedBytes = 0;
}

unsigned H264FileStreamParser::parse()
{
    while (1)
    {
        try
        {
            switch (fCurrentParseState)
            {
            case PARSING_START_SEQUENCE:
                //				printf("parseStartSequence\n");	//jay
                return parseStartSequence();
            case PARSING_NAL_UNIT:
                //				printf("parseNALUnit\n");	//jay
                return parseNALUnit();
            default:
                return 0;
            }
        }
        catch (int e)
        {
//			printf("catch %d!\n", e);		//jay
            if (e == USR_DEFINED_ERROR)
                continue;
            else
                return 0;
        }
    }
}

unsigned H264FileStreamParser::parseStartSequence()
{
//get 4 bytes
    u_int32_t test = test4Bytes();
//find the startcode
    while (test != 0x00000001)
    {
        skipBytes(1);
        test = test4Bytes();
    }
//set the state to PARSING_START_SEQUNCE
    setParseState(PARSING_START_SEQUENCE);
    return parseNALUnit();
}

unsigned H264FileStreamParser::parseNALUnit()
{
    unsigned char* sps = NULL;
    unsigned char* pps = NULL;
    skipBytes(1);
    u_int32_t head= test4Bytes();
    skipBytes(3);
    u_int8_t Type = 0;

    u_int32_t test = test4Bytes();
    switch (head&0xffffff1f)
    {
    case SPSHEAD:
        //		printf("SPS\n");		//jay
        Type = 7;
        sps = fTo + 1;
        break;
    case PPSHEAD:
        //		printf("PPS\n");		//jay
        Type = 8;
        pps = fTo + 1;
        break;
    case IDRHEAD:
        //		printf("IDR Frame\n");		//jay
        Type = 5;
        break;
    case NALUHEAD:
        //		printf("IP Frame\n");		//jay
        Type = 1;
        break;
    case AUDHEAD:
        //		printf("AUD\n");		//jay
        Type = 9;
        //		throw USR_DEFINED_ERROR;
        break;
    case SEIHEAD:
        //		printf("SEI\n");		//jay
        Type = 6;
        //		throw USR_DEFINED_ERROR;
        break;
    default:
        Type = 0;
        break;
    }

    while (test != 0x00000001)
    {
        saveByte(get1Byte());
        test = test4Bytes();
    }

    if (Type == 7)
    {
        if (fsps != NULL)
            delete[] fsps;
        fsps = new char[curFrameSize()*4/3 +4];
        base64encode(fsps,sps,curFrameSize()-1);
        memcpy(((char*)&profileLevelID),sps,1);
        memcpy(((char*)&profileLevelID)+1,sps+1,1);
        memcpy(((char*)&profileLevelID)+2,sps+2,1);
    }
    else if (Type == 8)
    {
        if (fpps != NULL)
            delete[] fpps;
        fpps = new char[curFrameSize()*4/3 +4];
        base64encode(fpps,pps,curFrameSize()-1);
    }
//	printf("curFrameSize %d\n", curFrameSize());
    fNaluType = Type;
    return curFrameSize();
}

void H264FileStreamParser::saveByte(u_int8_t byte)
{
//	printf("saveByte 0x%x\n", byte);	//jay
    if (fTo >= fLimit)
    {
        ++fNumTruncatedBytes;
        return;
    }
    *fTo++ = byte;
}

void H264FileStreamParser::save4Bytes(u_int32_t word)
{
    if (fTo+4 > fLimit)
    {
        fNumTruncatedBytes += 4;
        return;
    }
    *fTo++ = (word>>24);
    *fTo++ = (word>>16);
    *fTo++ = (word>>8);
    *fTo++ = (word);
}

//save data until we see a sync word(0x00000001)
void H264FileStreamParser::saveToNextCode(u_int32_t& curWord)
{
    while (curWord != 0x00000001)
    {
        save4Bytes(curWord);
        curWord = get4Bytes();
    }
}

void H264FileStreamParser::skipToNextCode(u_int32_t& curWord)
{
    while (curWord != 0x00000001)
    {
        curWord = get4Bytes();
    }
}

unsigned H264FileStreamParser::curFrameSize()
{
    return static_cast<unsigned>(fTo - fStartOfFrame);
}
char* H264FileStreamParser::getParsersps()
{
    return fsps;
}

char* H264FileStreamParser::getParserpps()
{
    return fpps;
}
unsigned int H264FileStreamParser::getPreID()
{
    return profileLevelID;
}


