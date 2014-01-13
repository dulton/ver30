#ifndef _H264_REAL_TIME_STREAM_FRAMER_HH
#define _H264_REAL_TIME_STREAM_FRAMER_HH

#include <ipc_media_data_client.h>

#include <H264VideoStreamFramer.hh>

#include "media_capture.h"
#include "common_def.h"

class H264RealTimeStreamFramer: public H264VideoStreamFramer {
public:
    static H264RealTimeStreamFramer * createNew(UsageEnvironment & env,  const SubStreamInfo & info);

protected:
    H264RealTimeStreamFramer(UsageEnvironment & env, MediaFrameCapture * frameCapture);
    virtual ~H264RealTimeStreamFramer();

private:
    virtual void doGetNextFrame();
    virtual void doStopGettingFrames();

    static void tryGetNextFrame(H264RealTimeStreamFramer * source);

    MediaFrameCapture * fFrameCapture;
    H264Frame         * fCurrentFrame;

    Boolean             fNeedNextFrame;

    uint32_t            fCurrentNaluUnitIndex;
    uint32_t            fOffset;

    Boolean             fNeedSPS;
    Boolean             fNeedPPS;

    Boolean             fFirstFrame;
};

#endif

