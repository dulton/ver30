#ifndef __FD_LISTENER_H__
#define __FD_LISTENER_H__

#include <map>

#include "task_queue.h"

class FdListener
{
public:
    FdListener(TaskQueue & Queue, uint32_t MaxSchedulerGranularity);
    ~FdListener();

    GMI_RESULT Initialize();
    GMI_RESULT Uninitialize();

    inline boolean_t Initialized() const { return m_Initialized; }

    void_t RegisterFdHandler(int32_t Fd, uint32_t Flags, TaskScheduler::BackgroundHandlerProc * Proc, void_t * ClientData);
    void_t UnregisterFdHandler(int32_t Fd);
    void_t MoveFdHandler(int32_t OldFd, int32_t NewFd);

private:
    typedef struct tagHandler
    {
        TaskScheduler::BackgroundHandlerProc * s_Proc;
        void_t                               * s_ClientData;
    } Handler;

    static void_t OnHandleProc(void_t * Data);
    void_t OnHandle();

    TaskQueue                  & m_TaskQueue;
    uint32_t                     m_MaxSchedulerGranularity;
    boolean_t                    m_Initialized;

    std::map<int32_t, Handler>   m_FdTable;

    int32_t                      m_MaxFdNum;

    fd_set                       m_ReadFds;
    fd_set                       m_WriteFds;
    fd_set                       m_ExceptionFds;

};

#endif // __FD_LISTENER_H__

