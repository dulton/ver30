#ifndef __H264_STREAM_PARSER_HH__
#define __H264_STREAM_PARSER_HH__

#include "ipc_media_data_client.h"
#include "StreamParser.hh"

#define SAVE_FILE 0

struct NALU
{
	u_int32_t timeStamp;
	u_int16_t don;
	u_int32_t size;
	u_int8_t * addr;
	u_int8_t nalu_type;
};

#define MAX_NALU_NUM_PER_FRAME	8
class H264BitStreamParser
{
public:
	H264BitStreamParser( int32_t streamID, uint16_t IPCMediaDataDispatchServerPort, uint16_t IPCMediaDataDispatchClientPort );
	virtual ~H264BitStreamParser();
	void registerReadInterest(unsigned char* to,unsigned maxSize);

	virtual int parse();
	char* getParsersps();
	char* getParserpps();
	char* getPreID();
	u_int32_t GetNalType() const { return fNaluType; }
	u_int32_t GetNaluSize() const { return fNaluSize; }
	struct timeval& GetPTS() { return fPTS; }
	
protected:
	unsigned parseNALUnit();
	unsigned curFrameSize();

protected:
	char* fsps;
	char* fpps;
	char profileLevelID[3];
	unsigned char* fTo;
	unsigned char* fLimit;
	NALU fNalus[MAX_NALU_NUM_PER_FRAME];
	u_int32_t fNaluCurrIndex;
	u_int32_t fNaluCount;
	u_int32_t fStreamID;

	u_int32_t fNaluType;
	u_int32_t fNaluSize;
	struct timeval fPTS;

private:
	boolean_t           fConnected;
	IPC_MediaDataClient fMediaDataClient;
	uint8_t             *fFrameBuffer;
	size_t              fMaxFrameLength;
	uint8_t             *fExtraData;
	size_t              fMaxExtraDataLength;

	uint16_t            fIPCMediaDataDispatchServerPort;
	uint16_t            fIPCMediaDataDispatchClientPort;

	uint64_t            fFrameCount;
	uint64_t            fTotalDelay;
	uint64_t            fMaxDelay;
	uint64_t            fMinDelay;

	struct timeval      fLastFrameTimeStamp;
    uint64_t            fTotalFrameInterval;
    uint64_t            fMaxFrameInterval;
    uint64_t            fMinFrameInterval;
#if SAVE_FILE
    FILE                *fVideoFile;
#endif
};

enum MyH264ParseState
{
	PARSING_START_SEQUENCE,
	PARSING_NAL_UNIT
};

class H264FileStreamParser:public StreamParser
{
public:
	H264FileStreamParser(FramedSource* usingSource,FramedSource* inputSource);
	virtual ~H264FileStreamParser();
	void registerReadInterest(unsigned char* to,unsigned maxSize);

	virtual unsigned parse();
	char* getParsersps();
	char* getParserpps();
	unsigned int getPreID();
	unsigned numTruncatedBytes() const;
	unsigned getParseState();
	unsigned fNaluType;
protected:
	void setParseState(MyH264ParseState parseState);
	unsigned parseStartSequence();
	unsigned parseNALUnit();
	void saveByte(u_int8_t byte);
	void save4Bytes(u_int32_t word);
	void saveToNextCode(u_int32_t& curWord);
	void skipToNextCode(u_int32_t& curWord);
	unsigned curFrameSize();
protected:
	FramedSource* fUsingSource;
	char* fsps;
	char* fpps;
	unsigned int profileLevelID;
	unsigned char* fStartOfFrame;
	unsigned char* fTo;
	unsigned char* fLimit;
	unsigned char* fSavedTo;
	unsigned fNumTruncatedBytes; //�ְ�?
	unsigned fSavedNumTruncatedBytes;
private:
	virtual void restoreSavedParserState();
	MyH264ParseState fCurrentParseState;
};

//-----------------------
#define AUDHEAD 0x00000109
#define SEIHEAD 0x00000106
#define SPSHEAD 0x00000107
#define PPSHEAD 0x00000108
#define IDRHEAD 0x00000105
#define NALUHEAD 0x00000101


#define NALU_TYPE_NONIDR	1
#define NALU_TYPE_IDR		5
#define NALU_TYPE_SEI		6
#define NALU_TYPE_SPS		7
#define NALU_TYPE_PPS		8
#define NALU_TYPE_AUD		9

#endif
