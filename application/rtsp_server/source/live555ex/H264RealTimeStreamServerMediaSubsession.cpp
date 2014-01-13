#include "H264RealTimeStreamRTPSink.hh"
#include "H264RealTimeStreamServerMediaSubsession.hh"
#include "H264RealTimeStreamFramer.hh"
#include "H264RealTimeStreamFramerEx.hh"

#include <H264VideoRTPSink.hh>

H264RealTimeStreamServerMediaSubsession * H264RealTimeStreamServerMediaSubsession::createNew(UsageEnvironment & env, const SubStreamInfo & info,
    Boolean reuseFirstSource) {
    return new H264RealTimeStreamServerMediaSubsession(env, info, reuseFirstSource);
}

H264RealTimeStreamServerMediaSubsession::H264RealTimeStreamServerMediaSubsession(UsageEnvironment & env, const SubStreamInfo & info, Boolean reuseFirstSource)
    : OnDemandServerMediaSubsession(env, reuseFirstSource)
    , fAuxSDPLine(NULL)
    , fDoneFlag(0)
    , fDummyRTPSink(NULL)
    , fSubStreamInfo(info) {
}

H264RealTimeStreamServerMediaSubsession::~H264RealTimeStreamServerMediaSubsession() {
    delete[] fAuxSDPLine;
}

static void afterPlayingDummy(void * clientData) {
    H264RealTimeStreamServerMediaSubsession * subsess = (H264RealTimeStreamServerMediaSubsession *)clientData;
    subsess->afterPlayingDummy1();
}

void H264RealTimeStreamServerMediaSubsession::afterPlayingDummy1() {
    // Unschedule any pending 'checking' task:
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    // Signal the event loop that we're done:
    setDoneFlag();
}

static void checkForAuxSDPLine(void * clientData) {
    H264RealTimeStreamServerMediaSubsession * subsess = (H264RealTimeStreamServerMediaSubsession *)clientData;
    subsess->checkForAuxSDPLine1();
}

void H264RealTimeStreamServerMediaSubsession::checkForAuxSDPLine1() {
    char const * dasl;

    if (fAuxSDPLine != NULL) {
        // Signal the event loop that we're done:
        setDoneFlag();
    } else if (fDummyRTPSink != NULL && (dasl = fDummyRTPSink->auxSDPLine()) != NULL) {
        fAuxSDPLine = strDup(dasl);
        fDummyRTPSink = NULL;

        // Signal the event loop that we're done:
        setDoneFlag();
    } else {
        // try again after a brief delay:
        int uSecsToDelay = 100000; // 100 ms
        nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)checkForAuxSDPLine, this);
    }
}

char const * H264RealTimeStreamServerMediaSubsession::getAuxSDPLine(RTPSink * rtpSink, FramedSource * inputSource) {
    if (fAuxSDPLine != NULL) return fAuxSDPLine; // it's already been set up (for a previous client)

    if (fDummyRTPSink == NULL) { // we're not already setting it up for another, concurrent stream
        // Note: For H264 video files, the 'config' information ("profile-level-id" and "sprop-parameter-sets") isn't known
        // until we start reading the file.  This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
        // and we need to start reading data from our file until this changes.
        fDummyRTPSink = rtpSink;

        // Start reading the file:
        fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

        // Check whether the sink's 'auxSDPLine()' is ready:
        checkForAuxSDPLine(this);
    }

    envir().taskScheduler().doEventLoop(&fDoneFlag);

    return fAuxSDPLine;
}

FramedSource * H264RealTimeStreamServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned & estBitrate) {
    estBitrate = fSubStreamInfo.s_MaxBitRate / 1000; // kbps, estimate

    // Create a framer for the Video Elementary Stream:

    return H264RealTimeStreamFramerEx::createNew(envir(), fSubStreamInfo);
    //return H264RealTimeStreamFramer::createNew(envir(), fSubStreamInfo);
}

RTPSink * H264RealTimeStreamServerMediaSubsession::createNewRTPSink(Groupsock * rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource * /*inputSource*/) {
#ifdef USE_H264_VIDEO_RTP_SINK

    return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);

#else // USE_H264_VIDEO_RTP_SINK

    return H264RealTimeStreamRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);

#endif // USE_H264_VIDEO_RTP_SINK
}

