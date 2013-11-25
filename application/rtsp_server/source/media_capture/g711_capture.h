#ifndef __G711_CAPTURE_H__
#define __G711_CAPTURE_H__

#include <ipc_media_data_client.h>

#include "media_capture.h"

#define MAX_G711_FRAME_COUNT 10

class G711FrameCapture : public MediaFrameCapture
{
public:
    G711FrameCapture(const SubStreamInfo & Info, uint32_t MaxFrameCount = MAX_G711_FRAME_COUNT);
    virtual ~G711FrameCapture();

    virtual MediaFrame * GetNextFrame(MediaFrame * PrevFrame);
    virtual void_t RecycleLastFrame(MediaFrame * LastFrame);

private:
    virtual GMI_RESULT StartCaptureImpl();
    virtual GMI_RESULT StopCaptureImpl();

    static void_t * ThreadEntry(void_t * Data);

    uint32_t ThreadEntryImpl();

    G711Frame * FreeFrame();
    void_t EnQueue(G711Frame & NewFrame);

    IPC_MediaDataClient   m_MediaDataClient;

#ifndef USE_OSAL_THREAD
    pthread_t             m_Thread;
#else
    GMI_Thread            m_Thread;
#endif
    GMI_Mutex             m_Mutex;

    boolean_t             m_Working;
    boolean_t             m_WorkDone;

    uint32_t              m_MaxFrameCount;
    uint32_t              m_UsingFrameCount;

    uint8_t             * m_FrameBuf;
    G711Frame           * m_G711FrameArray;

    Entry                 m_Head;
};

#endif // __G711_CAPTURE_H__

