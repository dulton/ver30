#ifndef __STREAM_CTRL__
#define __STREAM_CTRL__

#include "rtsp_service.h"

class StreamCtrl
{
public:
    StreamCtrl();
    ~StreamCtrl();

    GMI_RESULT Initialize(RtspService * Service);
    GMI_RESULT Uninitialize();

    inline boolean_t Initialized() const { return m_Initialized; }

private:
    static void_t SocketHandlerProc(void_t * Data, int32_t Mask);
    void_t SocketHandler();

    RtspService * m_RtspService;
    int32_t       m_SockFd;
    boolean_t     m_Initialized;
};

#endif // __STREAM_CTRL__
