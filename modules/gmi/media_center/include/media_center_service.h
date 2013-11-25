#pragma once

#include "gmi_system_headers.h"
#include "media_center.h"
#include "server_command_pipeline_manager.h"
#include "udp_session.h"

class MediaCenterService
{
public:
    MediaCenterService(void);
    ~MediaCenterService(void);

    GMI_RESULT  Initialize( long_t Server_IP, uint16_t Server_UDP_Port, size_t SessionBufferSize );
    GMI_RESULT  Deinitialize();
    GMI_RESULT  RegisterCommandExecutor( SafePtr<BaseCommandExecutor> CommandExecutor );
    GMI_RESULT  UnregisterCommandExecutor( uint32_t CommandId );

    GMI_RESULT  Run( boolean_t UseCallerThreadDoDispatchLoop );

    ReferrencePtr<MediaCenter>& GetMediaCenter()
    {
        return m_MediaCenter;
    }

private:
    GMI_RESULT  InitializeNetwork( long_t Server_IP, uint16_t Server_UDP_Port, size_t SessionBufferSize );
    GMI_RESULT  DeinitializeNetwork();

    static void_t* DispatchThread( void_t *Argument );
    void_t* DispatchEntry();

private:
    long_t                                                                                 m_Server_IP;
    uint16_t                                                                               m_Server_UDP_Port;
    size_t                                                                                 m_SessionBufferSize;
    ReferrencePtr<GMI_Socket>                                                              m_UDP_Socket;
    std::map<uint64_t,ReferrencePtr<UDPSession,DefaultObjectDeleter,MultipleThreadModel> > m_UDP_Sessions;
    ReferrencePtr<BaseCommandPipelineManager>                                              m_CommandPipeline;
    ReferrencePtr<MediaCenter>                                                             m_MediaCenter;

    boolean_t                                                                              m_UseCallerThreadDoDispatchLoop;
    GMI_Thread							                                                   m_DispatchThread;
    boolean_t                                                                              m_ThreadWorking;
    boolean_t                                                                              m_ThreadExitFlag;
};
