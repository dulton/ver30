#include "MyH264VideoStreamFramer.hh"

#include <assert.h>
#include <fcntl.h>
#include "GroupsockHelper.hh"
#include "H264StreamParser.hh"
#include <stdio.h>
#if defined( __linux__ )
#include <sys/ioctl.h>
#endif

MyH264VideoStreamFramer*
MyH264VideoStreamFramer::createNew(UsageEnvironment & env, FramedSource * inputSource)
{
    return new MyH264VideoStreamFramer(env,inputSource);
}

MyH264VideoStreamFramer*
MyH264VideoStreamFramer::createNew(UsageEnvironment & env, int streamID, uint16_t IPCMediaDataDispatchServerPort, uint16_t IPCMediaDataDispatchClientPort )
{
    return new MyH264VideoStreamFramer(env, streamID, IPCMediaDataDispatchServerPort, IPCMediaDataDispatchClientPort);
}

MyH264VideoStreamFramer::MyH264VideoStreamFramer(UsageEnvironment & env, FramedSource * inputSource)
    : H264VideoStreamFramer(env,inputSource),fFrameRate(30),fPictureEndMarker(False)
{
    assert(inputSource);
    fParser = new H264FileStreamParser(this, inputSource);
    fBitstreamParser = NULL;
    fStreamID = -1;
}

MyH264VideoStreamFramer::MyH264VideoStreamFramer(UsageEnvironment & env, int streamID, uint16_t IPCMediaDataDispatchServerPort, uint16_t IPCMediaDataDispatchClientPort )
    : H264VideoStreamFramer(env, NULL),fFrameRate(30),fPictureEndMarker(False)
{
    assert(streamID >= 0 && streamID < 4);
    fBitstreamParser = new H264BitStreamParser( streamID, IPCMediaDataDispatchServerPort, IPCMediaDataDispatchClientPort );
    fParser = NULL;
    fStreamID = streamID;
}

MyH264VideoStreamFramer::~MyH264VideoStreamFramer()
{
    printf("~MyH264VideoStreamFramer\n");
    if (fBitstreamParser)
        delete fBitstreamParser;
    if (fParser)
        delete fParser;
}

void MyH264VideoStreamFramer::doGetNextFrame()
{
//	printf("				  StreamFramer::doGetNextFrame\n");		//jay
    if (fBitstreamParser != NULL)
        fBitstreamParser->registerReadInterest(fTo, fMaxSize);
    if (fParser != NULL)
        fParser->registerReadInterest(fTo, fMaxSize);
    continueReadProcessing();
//	printf("				  StreamFramer::doGetNextFrame - end\n");	//jay
}

Boolean MyH264VideoStreamFramer::isH264VideoStreamFramer() const
{
    return True;
}

Boolean MyH264VideoStreamFramer::pictureEndMarker()
{
    return fPictureEndMarker;
}

Boolean MyH264VideoStreamFramer::currentNALUnitEndsAccessUnit()
{
    if ( fBitstreamParser != NULL )
    {
        if (fBitstreamParser->GetNalType() == NALU_TYPE_SPS || fBitstreamParser->GetNalType() == NALU_TYPE_PPS )
        {
            return False;
        }
    }
    return True;
}

char* MyH264VideoStreamFramer:: getSPS()
{
    if (fParser != NULL)
        return fParser->getParsersps();
    else
        return fBitstreamParser->getParsersps();
}

char* MyH264VideoStreamFramer::getPPS()
{
    if (fParser != NULL)
        return fParser->getParserpps();
    else
        return fBitstreamParser->getParserpps();
}

char* MyH264VideoStreamFramer::getProfileLevelID()
{
    if (fParser != NULL)
        //return fParser->getPreID();
        return NULL;
    else
        return fBitstreamParser->getPreID();
}

int MyH264VideoStreamFramer::getStreamID()
{
    return fStreamID;
}

void MyH264VideoStreamFramer::continueReadProcessing(void* clientData,
        unsigned char* /*ptr*/,unsigned/* size*/,struct timeval /*presentationTime*/)
{
    MyH264VideoStreamFramer* framer = (MyH264VideoStreamFramer*) clientData;
    framer->continueReadProcessing();
}

void MyH264VideoStreamFramer::continueReadProcessing()
{
    unsigned acquiredFrameSize;
    u_int64_t frameDuration_usec;

    if (fBitstreamParser != NULL && fParser == NULL)
    {
        if (fBitstreamParser->parse() < 0)
            return;

        if (fBitstreamParser->GetNalType() == NALU_TYPE_AUD)  	//IP NALU
        {
            //TODO: zhaomin tell us, we should ignore this nal to make some tool can playback video corrrectly.
        }

        //printf("nalu_type %d, size %d\n", fBitstreamParser->fNaluType, fBitstreamParser->fNaluSize);		//jay
        acquiredFrameSize = fBitstreamParser->GetNaluSize();
        if (acquiredFrameSize > 0)
        {
            fFrameSize = acquiredFrameSize;

            fPresentationTime = fBitstreamParser->GetPTS();

            fDurationInMicroseconds = 0;		//schedule as quick as possible after get one NALU

            //printf( "MyH264VideoStreamFramer::continueReadProcessing 1, second=%d, millisecond=%d \n", fPresentationTime.tv_sec, fPresentationTime.tv_usec );
            afterGetting(this);
        }
    }
    else
    {
        frameDuration_usec = static_cast<u_int64_t> (1000000 / fFrameRate);
        acquiredFrameSize = fParser->parse();
        //	printf("nalu_type %d\n", fParser->fNaluType);		//jay
        if (acquiredFrameSize > 0)
        {
            fFrameSize = acquiredFrameSize;

            if (fParser->fNaluType == NALU_TYPE_SPS)  		//SPS NALU
            {
                gettimeofday(&fPresentationTime, NULL);
            }
            else if (fParser->fNaluType == NALU_TYPE_AUD)  	//IP NALU
            {
                fPresentationTime.tv_usec += (long) frameDuration_usec;
            }

            while (fPresentationTime.tv_usec >= 1000000)
            {
                fPresentationTime.tv_usec -= 1000000;
                ++fPresentationTime.tv_sec;
            }
            //		if (fParser->fNaluType == 7 || fParser->fNaluType == 1)
            if (fParser->fNaluType == NALU_TYPE_AUD)
                fDurationInMicroseconds = (unsigned int )frameDuration_usec;

            //printf( "MyH264VideoStreamFramer::continueReadProcessing 2, second=%d, millisecond=%d \n", fPresentationTime.tv_sec, fPresentationTime.tv_usec );
            afterGetting(this);
        }
    }
}
