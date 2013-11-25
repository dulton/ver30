#ifndef __RTSP_SERVICE_H__
#define __RTSP_SERVICE_H__

#include "common_def.h"

#include "substream_info.h"

class RtspService
{
public:
    virtual ~RtspService();

    typedef void_t * TaskToken;
    typedef void_t (* TaskFunc)(void_t * Data);
    typedef void_t (* FdListener)(void_t * Data, int32_t Mask);

    static RtspService * CreateInstance();

    virtual GMI_RESULT Initialize() = 0;
    virtual GMI_RESULT Uninitiialize() = 0;

    virtual GMI_RESULT StartStream(uint32_t StreamId, uint32_t SubStreamNum, const SubStreamInfo * SubStreams) = 0;
    virtual GMI_RESULT StopStream(uint32_t StreamId) = 0;
    virtual boolean_t IsStreamStarted(uint32_t StreamId) = 0;

    virtual void_t DoEventLoop(uint8_t * StopFlag = NULL) = 0;

    virtual TaskToken AddDelayedTask(TaskFunc Func, void_t * Data, uint32_t USec) = 0;
    virtual void_t RemoveDelayedTask(TaskToken TaskId) = 0;

    virtual void_t AddFdListener(int32_t Fd, FdListener Listener, void_t * Data) = 0;
    virtual void_t RemoveFdListener(int32_t Fd) = 0;

protected:
    RtspService();
};

#endif // __RTSP_SERVICE_H__

