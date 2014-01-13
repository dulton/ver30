#ifndef _H264_REAL_TIME_STREAM_FRAMER_EX_HH
#define _H264_REAL_TIME_STREAM_FRAMER_EX_HH

#include <H264VideoStreamFramer.hh>

#include <ipc_media_data_client.h>

#include "media_capture.h"

#include "common_def.h"

class H264RealTimeStreamFramerEx: public H264VideoStreamFramer {
public:
    static H264RealTimeStreamFramerEx * createNew(UsageEnvironment & env,  const SubStreamInfo & info);

protected:
    H264RealTimeStreamFramerEx(UsageEnvironment & env, IPC_MediaDataClient * mediaDataClient, const SubStreamInfo & info);
    virtual ~H264RealTimeStreamFramerEx();

private:
    virtual void doGetNextFrame();
    virtual void doStopGettingFrames();

    static void tryGetNextFrame(H264RealTimeStreamFramerEx * source);

    IPC_MediaDataClient * fMediaDataClient;
    SubStreamInfo         fSubStreamInfo;
    H264Frame             fCurrentFrame;

    Boolean               fNeedNextFrame;

    unsigned              fCurrentNaluUnitIndex;
    unsigned              fOffset;

    Boolean               fNeedSPS;
    Boolean               fNeedPPS;

    Boolean               fFirstFrame;
    Boolean               fNeedReleaseFrame;

    struct timeval        fTimeStamp;
    unsigned              fFrameDuration;
};

#endif

