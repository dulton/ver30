#ifndef __RUDP_SESSION_MANAGER_H__
#define __RUDP_SESSION_MANAGER_H__

#include "rudp_session.h"
#include "gmi_system_headers.h"
#include "base_session_manager.h"

class RUDPSessionManager : public BaseSessionManager
{
public:
    RUDPSessionManager( uint16_t LocalRudpCPort, uint16_t LocalRudpSPort, size_t SessionBufferSize );
    virtual ~RUDPSessionManager(void);
    virtual GMI_RESULT  Initialize( void_t *Argument, size_t ArgumentSize);
    virtual GMI_RESULT  Deinitialize();
    virtual GMI_RESULT  Start( ReferrencePtr<BaseCommandPipelineManager> CommandPipeline );
    virtual GMI_RESULT  Stop();

    ReferrencePtr<RUDPSession, DefaultObjectDeleter, MultipleThreadModel> GetUDPSession( int32_t index );

private:
    static void* ReceiveThread( void *Argument);
    void* ReceiveEntry();

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
    uint16_t 					m_LocalRudpCPort;
    uint16_t                    m_LocalRudpSPort;
    size_t                      m_SessionBufferSize;
    FD_HANDLE                   m_RudpSendSocket;
    FD_HANDLE                   m_RudpRecvSocket;

    GMI_Thread                  m_ReceiveThread;
    boolean_t                   m_ThreadWorking;
    boolean_t                   m_ThreadExitFlag;

    std::map< uint64_t, ReferrencePtr<RUDPSession, DefaultObjectDeleter, MultipleThreadModel> > m_RUDP_Sessions;
    ReferrencePtr<BaseCommandPipelineManager> m_CommandPipeline;
};

#endif

