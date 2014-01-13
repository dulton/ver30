#ifndef _H264_REAL_TIME_STREAM_SERVER_MEDIA_SUBSESSION_HH
#define _H264_REAL_TIME_STREAM_SERVER_MEDIA_SUBSESSION_HH

#include <OnDemandServerMediaSubsession.hh>

#include "substream_info.h"

class H264RealTimeStreamServerMediaSubsession: public OnDemandServerMediaSubsession {
public:
    static H264RealTimeStreamServerMediaSubsession * createNew(UsageEnvironment & env, const SubStreamInfo & info, Boolean reuseFirstSource);

    // Used to implement "getAuxSDPLine()":
    void checkForAuxSDPLine1();
    void afterPlayingDummy1();

protected:
    H264RealTimeStreamServerMediaSubsession(UsageEnvironment & env, const SubStreamInfo & info, Boolean reuseFirstSource);
    // called only by createNew();
    virtual ~H264RealTimeStreamServerMediaSubsession();

    void setDoneFlag() { fDoneFlag = ~0; }

protected: // redefined virtual functions
    virtual char const * getAuxSDPLine(RTPSink * rtpSink, FramedSource * inputSource);
    virtual FramedSource * createNewStreamSource(unsigned clientSessionId, unsigned & estBitrate);
    virtual RTPSink * createNewRTPSink(Groupsock * rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource * inputSource);

private:
    char          * fAuxSDPLine;
    char            fDoneFlag;    // used when setting up "fAuxSDPLine"
    RTPSink       * fDummyRTPSink;
    SubStreamInfo   fSubStreamInfo;
};

#endif

