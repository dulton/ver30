#include "fd_listener.h"

FdListener::FdListener(TaskQueue & Queue, uint32_t MaxSchedulerGranularity)
    : m_TaskQueue(Queue)
    , m_MaxSchedulerGranularity(MaxSchedulerGranularity)
    , m_Initialized(false)
    , m_FdTable()
    , m_MaxFdNum(0)
{
    FD_ZERO(&m_ReadFds);
    FD_ZERO(&m_WriteFds);
    FD_ZERO(&m_ExceptionFds);
}

FdListener::~FdListener()
{
    if (Initialized())
    {
        Uninitialize();
    }
}

GMI_RESULT FdListener::Initialize()
{
    if (!m_TaskQueue.Initialized())
    {
        PRINT_LOG(WARNING, "MUST initialize TaskQueue first");
        return GMI_FAIL;
    }

    if (Initialized())
    {
        PRINT_LOG(WARNING, "FdListener is already initialized");
        return GMI_ALREADY_OPERATED;
    }

    m_TaskQueue.AddTask(0, OnHandleProc, this);

    m_Initialized = true;

    return GMI_SUCCESS;
}

GMI_RESULT FdListener::Uninitialize()
{
    if (!Initialized())
    {
        PRINT_LOG(WARNING, "TaskQueue is not initialized yet");
        return GMI_ALREADY_OPERATED;
    }

    m_Initialized = false;

    return GMI_SUCCESS;
}


void_t FdListener::RegisterFdHandler(int32_t Fd, uint32_t Flags, TaskScheduler::BackgroundHandlerProc * Proc, void_t * ClientData)
{
    if (!Initialized())
    {
        PRINT_LOG(WARNING, "TaskQueue is not initialized yet");
        return;
    }

    Handler Hndler = {Proc, ClientData};
    m_FdTable[Fd] = Hndler;

    if (Flags & SOCKET_READABLE)
    {
        FD_SET(Fd, &m_ReadFds);
    }

    if (Flags & SOCKET_WRITABLE)
    {
        FD_SET(Fd, &m_WriteFds);
    }

    if (Flags & SOCKET_EXCEPTION)
    {
        FD_SET(Fd, &m_ExceptionFds);
    }

    if (m_MaxFdNum < Fd)
    {
        m_MaxFdNum = Fd;
    }
}

void_t FdListener::UnregisterFdHandler(int32_t Fd)
{
    if (!Initialized())
    {
        PRINT_LOG(WARNING, "TaskQueue is not initialized yet");
        return;
    }

    std::map<int32_t, Handler>::iterator it = m_FdTable.find(Fd);
    if (it != m_FdTable.end())
    {
        m_FdTable.erase(it);
    }

    FD_CLR(Fd, &m_ReadFds);
    FD_CLR(Fd, &m_WriteFds);
    FD_CLR(Fd, &m_ExceptionFds);
}

void_t FdListener::MoveFdHandler(int32_t OldFd, int32_t NewFd)
{
    if (!Initialized())
    {
        PRINT_LOG(WARNING, "TaskQueue is not initialized yet");
        return;
    }

    std::map<int32_t, Handler>::iterator it = m_FdTable.find(OldFd);
    if (it != m_FdTable.end())
    {
        m_FdTable[NewFd] = it->second;
        m_FdTable.erase(it);
    }

    if (FD_ISSET(OldFd, &m_ReadFds))
    {
        FD_CLR(OldFd, &m_ReadFds);
        FD_SET(NewFd, &m_ReadFds);
    }

    if (FD_ISSET(OldFd, &m_WriteFds))
    {
        FD_CLR(OldFd, &m_WriteFds);
        FD_SET(NewFd, &m_WriteFds);
    }

    if (FD_ISSET(OldFd, &m_ExceptionFds))
    {
        FD_CLR(OldFd, &m_ExceptionFds);
        FD_SET(NewFd, &m_ExceptionFds);
    }

    if (m_MaxFdNum < NewFd)
    {
        m_MaxFdNum = NewFd;
    }
}

void_t FdListener::OnHandle()
{
    fd_set         ReadFds      = m_ReadFds;
    fd_set         WriteFds     = m_WriteFds;
    fd_set         ExceptionFds = m_ExceptionFds;
    struct timeval TimedOut     = {0, 0};

    int32_t RetVal = select(m_MaxFdNum + 1, &ReadFds, &WriteFds, &ExceptionFds, &TimedOut);
    if (RetVal < 0)
    {
        PRINT_LOG(ERROR, "Failed to select fds, errno = %d", errno);
        return;
    }
    else if (RetVal > 0)
    {
        for (int32_t Fd = 0; Fd <= m_MaxFdNum; Fd ++)
        {
            int32_t Flags = 0;
            if (FD_ISSET(Fd, &ReadFds) && FD_ISSET(Fd, &m_ReadFds))
            {
                Flags |= SOCKET_READABLE;
            }

            if (FD_ISSET(Fd, &WriteFds) && FD_ISSET(Fd, &m_WriteFds))
            {
                Flags |= SOCKET_WRITABLE;
            }

            if (FD_ISSET(Fd, &ExceptionFds) && FD_ISSET(Fd, &m_ExceptionFds))
            {
                Flags |= SOCKET_EXCEPTION;
            }

            if (Flags != 0)
            {
                std::map<int32_t, Handler>::iterator it = m_FdTable.find(Fd);
                if (it != m_FdTable.end())
                {
                    it->second.s_Proc(it->second.s_ClientData, Flags);
                }
            }
        }
    }

    m_TaskQueue.AddTask(m_MaxSchedulerGranularity, OnHandleProc, this);
}

void_t FdListener::OnHandleProc(void_t * Data)
{
    ASSERT(Data != NULL, "Data MUST NOT be non-pointer");
    FdListener * This = (FdListener *)Data;
    This->OnHandle();
}

