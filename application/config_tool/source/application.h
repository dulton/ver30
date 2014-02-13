#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "common_def.h"

#include <tool_protocol.h>

class Application : public Singleton<Application>
{
public:
    Application(int32_t argc, const char_t * argv []);
    ~Application();

    GMI_RESULT Start();
    GMI_RESULT Stop();

    inline boolean_t IsRunning() const { return m_Running; }

    inline GMI_RESULT AddDelayTask(CbOnSchedule Proc, void_t * Data, uint32_t Sec, uint32_t USec)
    {
        if (m_GtpHandle != GTP_INVALID_HANDLE)
        {
            return GtpAddDelayTask(m_GtpHandle, Proc, Data, Sec, USec); 
        }

        PRINT_LOG(WARNING, "Task list is not prepared yet");
        return GMI_INVALID_OPERATION;
    }

    inline GMI_RESULT UpdateDelayTask(CbOnSchedule Proc, void_t * Data, uint32_t Sec, uint32_t USec)
    {
        if (m_GtpHandle != GTP_INVALID_HANDLE)
        {
            return GtpUpdateDelayTask(m_GtpHandle, Proc, Data, Sec, USec); 
        }

        PRINT_LOG(WARNING, "Task list is not prepared yet");
        return GMI_INVALID_OPERATION;
    }

    inline GMI_RESULT CancelDelayTask(CbOnSchedule Proc, void_t * Data)
    {
        if (m_GtpHandle != GTP_INVALID_HANDLE)
        {
            return GtpCancelDelayTask(m_GtpHandle, Proc, Data); 
        }

        PRINT_LOG(WARNING, "Task list is not prepared yet");
        return GMI_INVALID_OPERATION;
    }

private:
    static void_t SignalStop(int32_t SignalNumber);
    static void_t SignalPipe(int32_t SignalNumber);

    static void_t HeartBeatTask(void_t * Data);

    GMI_RESULT Run();

    boolean_t      m_Running;
    const char_t * m_Interface;
    uint32_t       m_TimedOut;

    GtpHandle      m_GtpHandle;

private:
    // Defined copy constructor, but not implement it
    // So that we can avoid copy construction
    Application(const Application & App);
    Application & operator = (const Application & App);
};

#endif // __APPLICATION_H__

