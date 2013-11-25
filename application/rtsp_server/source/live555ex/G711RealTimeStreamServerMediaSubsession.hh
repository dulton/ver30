#ifndef _G711_REAL_TIME_STREAM_SERVER_MEDIA_SUBSESSION_HH
#define _G711_REAL_TIME_STREAM_SERVER_MEDIA_SUBSESSION_HH

#include <OnDemandServerMediaSubsession.hh>

#include "substream_info.h"

class G711RealTimeStreamServerMediaSubsession: public OnDemandServerMediaSubsession {
public:
  static G711RealTimeStreamServerMediaSubsession * createNew(UsageEnvironment & env, const SubStreamInfo & info, Boolean reuseFirstSource);

protected:
    G711RealTimeStreamServerMediaSubsession(UsageEnvironment & env, const SubStreamInfo & info, Boolean reuseFirstSource);
    // called only by createNew();
    virtual ~G711RealTimeStreamServerMediaSubsession();

protected: // redefined virtual functions
    virtual FramedSource * createNewStreamSource(unsigned clientSessionId, unsigned & estBitrate);
    virtual RTPSink * createNewRTPSink(Groupsock * rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource * inputSource);

private:
    unsigned char fBitsPerSample;
    unsigned      fSamplingFrequency;
    unsigned      fNumChannels;
    SubStreamInfo fSubStreamInfo;
};

#endif
