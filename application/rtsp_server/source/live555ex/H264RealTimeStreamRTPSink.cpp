#include <Base64.hh>
#include <H264VideoStreamFramer.hh>

#include "H264RealTimeStreamRTPSink.hh"

H264RealTimeStreamRTPSink::H264RealTimeStreamRTPSink(UsageEnvironment & env, Groupsock * RTPgs, unsigned char rtpPayloadFormat)
    : VideoRTPSink(env, RTPgs, rtpPayloadFormat, 90000, "H264")
    , fFmtpSDPLine(NULL)
    , fSPS(NULL)
    , fSPSSize(0)
    , fPPS(NULL)
    , fPPSSize(0)
{
}

H264RealTimeStreamRTPSink::~H264RealTimeStreamRTPSink() {
    delete[] fFmtpSDPLine;
    delete[] fSPS;
    delete[] fPPS;

    stopPlaying();
}

H264RealTimeStreamRTPSink * H264RealTimeStreamRTPSink::createNew(UsageEnvironment & env, Groupsock * RTPgs, unsigned char rtpPayloadFormat) {
    return new H264RealTimeStreamRTPSink(env, RTPgs, rtpPayloadFormat);
}

void H264RealTimeStreamRTPSink::doSpecialFrameHandling(unsigned /*fragmentationOffset*/, unsigned char * /*frameStart*/,
    unsigned /*numBytesInFrame*/, struct timeval framePresentationTime, unsigned /*numRemainingBytes*/) {
    if (fSource != NULL) {
        H264VideoStreamFramer * framerSource = (H264VideoStreamFramer*)fSource;
        if (framerSource->pictureEndMarker()) {
            setMarkerBit();
            framerSource->pictureEndMarker() = False;
        }
    }

    setTimestamp(framePresentationTime);
}

char const * H264RealTimeStreamRTPSink::auxSDPLine() {
    // Generate a new "a=fmtp:" line each time, using our SPS and PPS (if we have them),
    // otherwise parameters from our framer source (in case they've changed since the last time that
    // we were called):
    u_int8_t * sps     = fSPS;
    unsigned   spsSize = fSPSSize;
    u_int8_t * pps     = fPPS;
    unsigned   ppsSize = fPPSSize;
    if (sps == NULL || pps == NULL) {
        // We need to get SPS and PPS from our framer source:
        if (fSource == NULL) return NULL;
        H264VideoStreamFramer * framerSource = (H264VideoStreamFramer *)fSource;

        framerSource->getSPSandPPS(sps, spsSize, pps, ppsSize);
        if (sps == NULL || pps == NULL) return NULL; // our source isn't ready
    }

    u_int32_t profile_level_id;
    if (spsSize < 4) { // sanity check
        profile_level_id = 0;
    } else {
        profile_level_id = (sps[1]<<16)|(sps[2]<<8)|sps[3]; // profile_idc|constraint_setN_flag|level_idc
    }
  
    // Set up the "a=fmtp:" SDP line for this stream:
    char       * sps_base64  = base64Encode((char*)sps, spsSize);
    char       * pps_base64  = base64Encode((char*)pps, ppsSize);
    char const * fmtpFmt     = "a=fmtp:%d packetization-mode=1;profile-level-id=%06X;sprop-parameter-sets=%s,%s\r\n";
    unsigned     fmtpFmtSize = strlen(fmtpFmt) + 3 /* max char len */ + 6 /* 3 bytes in hex */ + strlen(sps_base64) + strlen(pps_base64);
    char       * fmtp        = new char[fmtpFmtSize];

    sprintf(fmtp, fmtpFmt, rtpPayloadType(), profile_level_id, sps_base64, pps_base64);

    delete[] sps_base64;
    delete[] pps_base64;
    delete[] fFmtpSDPLine;

    fFmtpSDPLine = fmtp;
    return fFmtpSDPLine;
}

