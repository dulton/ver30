#ifndef _H264_REAL_TIME_STREAM_RTP_SINK_HH
#define _H264_REAL_TIME_STREAM_RTP_SINK_HH

#include <VideoRTPSink.hh>

// Define simple RTP packet
class RTPPacket
{
public:
    RTPPacket() : fPayloadSize(0), fSeqNum(0) {
        memset(fBuffer, 0x00, sizeof(fBuffer));
        fBuffer[0] = 0x80;
    }

    inline void setPayloadType(u_int8_t payloadType) {
        fBuffer[1] &= 0x80;
        fBuffer[1] |= (payloadType & 0x7f);
    }

    inline void setMarkerBit() {
        fBuffer[1] |= 0x80;
    }

    inline void removeMarkerBit() {
        fBuffer[1] &= 0x7f;
    }

    inline void increaseSequenceNumer() {
        fSeqNum ++;
        u_int16_t bytes = htons(fSeqNum);
        memcpy(fBuffer + 2, &bytes, 2);
    }

    inline void setTimestamp(u_int32_t timestamp) {
        u_int32_t bytes = htonl(timestamp);
        memcpy(fBuffer + 4, &bytes, 4);
    }

    inline void setSSRC(u_int32_t SSRC) {
        u_int32_t bytes = htonl(SSRC);
        memcpy(fBuffer + 8, &bytes, 4);
    }

    inline u_int8_t * payload() {
        return fBuffer + 12;
    }

    inline unsigned maxPayloadSize() const {
        return sizeof(fBuffer) - 12;
    }

    inline void setPayloadSize(unsigned size) {
        fPayloadSize = size;
    }

    inline u_int8_t * packet() {
        return fBuffer;
    }

    inline unsigned packetSize() const {
        return fPayloadSize + 12;
    }

    inline unsigned payloadSize() const {
        return fPayloadSize;
    }

    inline unsigned headerSize() const {
        return 12;
    }

private:
    u_int8_t  fBuffer[1448];
    unsigned  fPayloadSize;
    u_int32_t fSeqNum;
};


class H264RealTimeStreamRTPSink: public RTPSink {
public:
    static H264RealTimeStreamRTPSink * createNew(UsageEnvironment & env, Groupsock * RTPgs, unsigned char rtpPayloadFormat);

    virtual Boolean continuePlaying();

protected:
    H264RealTimeStreamRTPSink(UsageEnvironment & env, Groupsock * RTPgs, unsigned char rtpPayloadFormat);
    // called only by createNew()

    virtual ~H264RealTimeStreamRTPSink();

private: // redefined virtual functions:
    virtual Boolean sourceIsCompatibleWithUs(MediaSource & source) {
        // Our source must be an appropriate framer:
        return source.isH264VideoStreamFramer();
    }

    virtual char const * sdpMediaType() const {
        return "video";
    };

    virtual char const * auxSDPLine();

    static void afterGettingFrameProc(void * clientData, unsigned numBytesRead, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);
    void afterGettingFrame(unsigned numBytesRead, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);

    static void sendNextPacketProc(void * clientData);
    void sendNextPacket();

private:
    char      * fFmtpSDPLine;
    u_int8_t  * fSPS;
    unsigned    fSPSSize;
    u_int8_t  * fPPS;
    unsigned    fPPSSize;
    RTPPacket   fPacket;
    Boolean     fWaiting;
    Boolean     fSending;
    Boolean     fFirstPacketInFrame;
};

#endif

