#ifndef _AMBARELLA_REAL_TIME_STREAM_RTP_SINK_HH
#define _AMBARELLA_REAL_TIME_STREAM_RTP_SINK_HH

#include <VideoRTPSink.hh>

class H264RealTimeStreamRTPSink: public VideoRTPSink {
public:
    static H264RealTimeStreamRTPSink * createNew(UsageEnvironment & env, Groupsock * RTPgs, unsigned char rtpPayloadFormat);

protected:
    H264RealTimeStreamRTPSink(UsageEnvironment & env, Groupsock * RTPgs, unsigned char rtpPayloadFormat);
    // called only by createNew()

    virtual ~H264RealTimeStreamRTPSink();

protected: // redefined virtual functions:
    virtual char const * auxSDPLine();

private: // redefined virtual functions:
    virtual Boolean sourceIsCompatibleWithUs(MediaSource & source) {
        // Our source must be an appropriate framer:
        return source.isH264VideoStreamFramer();
    }

    virtual void doSpecialFrameHandling(unsigned fragmentationOffset, unsigned char * frameStart, unsigned numBytesInFrame,
        struct timeval framePresentationTime, unsigned numRemainingBytes);

    virtual Boolean frameCanAppearAfterPacketStart(unsigned char const * frameStart, unsigned numBytesInFrame) const {
        return False;
    }

private:
    char     * fFmtpSDPLine;
    u_int8_t * fSPS;
    unsigned   fSPSSize;
    u_int8_t * fPPS;
    unsigned   fPPSSize;
};

#endif

