#ifndef _G711_REAL_TIME_STREAM_SOURCE_HH
#define _G711_REAL_TIME_STREAM_SOURCE_HH

#include <ipc_media_data_client.h>

#include <FramedSource.hh>

#include "media_capture.h"
#include "common_def.h"

class  G711RealTimeStreamSource : public FramedSource {
public:
    static G711RealTimeStreamSource * createNew(UsageEnvironment & env, const SubStreamInfo & info);

protected:
    G711RealTimeStreamSource(UsageEnvironment & env, MediaFrameCapture * frameCapture);
    // called only by createNew()

    virtual ~G711RealTimeStreamSource();

private:
    // redefined virtual functions:
    virtual void doGetNextFrame();
    virtual void doStopGettingFrames();

    static void tryGetNextFrame(G711RealTimeStreamSource * source);

    MediaFrameCapture * fFrameCapture;
    G711Frame         * fCurrentFrame;

    Boolean             fFirstFrame;
};

#endif

