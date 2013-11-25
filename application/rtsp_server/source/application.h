#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "rtsp_service.h"
#include "stream_ctrl.h"

#include "common_def.h"

class Application : public Singleton<Application>
{
public:
    Application(int32_t argc, const char_t * argv []);
    ~Application();

    GMI_RESULT Start();
    GMI_RESULT Stop();

    inline boolean_t IsRunning() const { return (m_StopFlag == 0 ? true : false); }

private:
    static void_t SignalStop(int32_t SignalNumber);
    static void_t SignalPipe(int32_t SignalNumber);

    static void_t HeartBeatTask(void_t * Data);

    GMI_RESULT Run();

    uint8_t                  m_StopFlag;
    RtspService            * m_RtspService;
    StreamCtrl               m_StreamCtrl;
    RtspService::TaskToken   m_HearbeatTask;

private:
    // Defined copy constructor, but not implement it
    // So that we can avoid copy construction
    Application(const Application & App);
    Application & operator = (const Application & App);
};

#endif // __APPLICATION_H__

