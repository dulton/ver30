
#ifndef __SYS_COMMAND_PROCESS_H__
#define __SYS_COMMAND_PROCESS_H__

#include "rudp_session_manager.h"
#include "unix_tcp_session_manager.h"
#include "system_packet.h"
#include "system_service_manager.h"
#include "base_session_manager.h"


class SysCommandProcessor
{
public:
    SysCommandProcessor( uint16_t LocalRudpCPort, uint16_t LocalRudpSPort, size_t  SessionBufferSize );
    ~SysCommandProcessor();
    GMI_RESULT Initialize( void_t *Argument );
    GMI_RESULT Deinitialize();

private:
    GMI_RESULT ContextInitialize();
    GMI_RESULT ContextDeinitialize();
    GMI_RESULT RegisterCommand();
    GMI_RESULT Start();
    GMI_RESULT Stop();

private:
    uint16_t m_LocalRudpCPort;
    uint16_t m_LocalRudpSPort;
    size_t m_SessionBufferSize;
    ReferrencePtr<RUDPSessionManager> m_SessionManager;
    ReferrencePtr<UnixTcpSessionManager> m_UnixTcpSessionManager;
    ReferrencePtr<BaseCommandPipelineManager> m_CommandPipeline;
    ReferrencePtr<SystemPacket> m_Packet;
    ReferrencePtr<SystemServiceManager> m_ServiceManager;
};

#endif

