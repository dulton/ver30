#ifndef __UNIX_TCP_SESSION_MANAGER_H__
#define __UNIX_TCP_SESSION_MANAGER_H__

#include "unix_tcp_session.h"
#include "gmi_system_headers.h"
#include "base_session_manager.h"

#define Offsetof(TYPE, MEMBER)  ((int)&((TYPE*)0)->MEMBER)
#define QNUM  (20)
#define UNIX_SERVER_PATH "/tmp/sys_server"

class UnixTcpSessionManager : public BaseSessionManager
{
public:
    UnixTcpSessionManager(size_t SessionBufferSize );
    virtual ~UnixTcpSessionManager(void);
    virtual GMI_RESULT  Initialize( void_t *Argument, size_t ArgumentSize);
    virtual GMI_RESULT  Deinitialize();
    virtual GMI_RESULT  Start( ReferrencePtr<BaseCommandPipelineManager> CommandPipeline );
    virtual GMI_RESULT  Stop();

private:
    GMI_RESULT Listen(const char_t* Name, int32_t *FdPtr);
    GMI_RESULT Accept(int32_t ListenFd, uid_t *UidPtr, int32_t *ClientFdPtr);
    static void* ServerThread(void *Argument);
    void* Server();
    static void* ProcessThread(void *Argument);
    void* ProcessEntry(int32_t ClientFd);
private:
    inline uint32_t TimeStamp(void)
    {
        uint32_t Stamp;
        struct timeval Now;
        gettimeofday(&Now, NULL);
        Stamp = Now.tv_sec * 1000 + Now.tv_usec/1000;
        return Stamp;
    }
private:
    size_t                      m_SessionBufferSize;
    int32_t                     m_ListenFd;

    GMI_Thread                  m_ServerThread;
    boolean_t                   m_ThreadWorking;
    boolean_t                   m_ThreadExitFlag;

    ReferrencePtr<BaseCommandPipelineManager> m_CommandPipeline;
};

#endif




