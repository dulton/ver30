#include <SimpleRTPSink.hh>

#include "G711RealTimeStreamServerMediaSubsession.hh"
#include "G711RealTimeStreamSource.hh"

G711RealTimeStreamServerMediaSubsession * G711RealTimeStreamServerMediaSubsession::createNew(UsageEnvironment & env, const SubStreamInfo & info,
    Boolean reuseFirstSource) {
    return new G711RealTimeStreamServerMediaSubsession(env, info, reuseFirstSource);
}

G711RealTimeStreamServerMediaSubsession::G711RealTimeStreamServerMediaSubsession(UsageEnvironment& env, const SubStreamInfo & info, Boolean reuseFirstSource)
    : OnDemandServerMediaSubsession(env, reuseFirstSource)
    , fBitsPerSample(8)
    , fSamplingFrequency(8000)
    , fNumChannels(1)
    , fSubStreamInfo(info)
{
}

G711RealTimeStreamServerMediaSubsession::~G711RealTimeStreamServerMediaSubsession() {
}

FramedSource * G711RealTimeStreamServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned & estBitrate) {
    estBitrate = fSamplingFrequency * fBitsPerSample * fNumChannels / 1000;

    return G711RealTimeStreamSource::createNew(envir(), fSubStreamInfo);
}

RTPSink * G711RealTimeStreamServerMediaSubsession::createNewRTPSink(Groupsock * rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
    FramedSource * /*inputSource*/) {
    const char    * mimeType          = "PCMA";
    unsigned char   payloadFormatCode = rtpPayloadTypeIfDynamic;

    if (fSamplingFrequency == 8000 && fNumChannels == 1) {
        payloadFormatCode = 8; // a static RTP payload type
    }

    return SimpleRTPSink::createNew(envir(), rtpGroupsock, payloadFormatCode, fSamplingFrequency, "audio", mimeType, fNumChannels);
}

