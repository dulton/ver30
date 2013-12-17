#include "media_center_service.h"

#include "application_packet.h"
#include "media_center_packet.h"
#include "share_memory_log_client.h"

#define MEDIA_CENTER_SERVER_MAX_TIMEOUT 500 // ms unit

MediaCenterService::MediaCenterService(void)
    : m_Server_IP( 0 )
    , m_Server_UDP_Port( 0 )
    , m_SessionBufferSize( 0 )
    , m_UDP_Socket()
    , m_UDP_Sessions()
    , m_CommandPipeline()
    , m_MediaCenter()
    , m_UseCallerThreadDoDispatchLoop( false )
    , m_DispatchThread()
    , m_ThreadWorking( false )
    , m_ThreadExitFlag( false )
{
}

MediaCenterService::~MediaCenterService(void)
{
}

GMI_RESULT MediaCenterService::Initialize( long_t Server_IP, uint16_t Server_UDP_Port, size_t SessionBufferSize )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::Initialize begin, passed Server_IP=%x, Server_UDP_Port=%d, SessionBufferSize=%d \n", ntohl(Server_IP), ntohs(Server_UDP_Port), SessionBufferSize );

    GMI_RESULT Result = InitializeNetwork( Server_IP, Server_UDP_Port, SessionBufferSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::Initialize, InitializeNetwork failed, function return %x \n ", (uint32_t) Result );
        return Result;
    }

    m_MediaCenter = BaseMemoryManager::Instance().New<MediaCenter>();
    if ( NULL == m_MediaCenter.GetPtr() )
    {
        DeinitializeNetwork();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::Initialize, MediaCenter object allocation failed, function return %x \n ", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    ReferrencePtr<ApplicationPacket> Packet( BaseMemoryManager::Instance().New<ApplicationPacket>() );
    if ( NULL == Packet.GetPtr() )
    {
        m_MediaCenter = NULL;
        DeinitializeNetwork();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::Initialize, ApplicationPacket object allocation failed, function return %x \n ", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    Result = Packet->Initialize( (const uint8_t *)GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION );
    if ( FAILED( Result ) )
    {
        Packet = NULL;
        m_MediaCenter = NULL;
        DeinitializeNetwork();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::Initialize, ApplicationPacket object initialization failed, function return %x \n ", (uint32_t) Result );
        return Result;
    }

    m_CommandPipeline = BaseMemoryManager::Instance().New<ServerCommandPipelineManager>();
    if ( NULL == m_CommandPipeline.GetPtr() )
    {
        Packet->Deinitialize();
        Packet = NULL;
        m_MediaCenter = NULL;
        DeinitializeNetwork();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::Initialize, ServerCommandPipelineManager object allocation failed, function return %x \n ", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    Result = m_CommandPipeline->Initialize();
    if ( FAILED( Result ) )
    {
        m_CommandPipeline = NULL;
        Packet->Deinitialize();
        Packet = NULL;
        m_MediaCenter = NULL;
        DeinitializeNetwork();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::Initialize, ServerCommandPipelineManager object initialization failed, function return %x \n ", (uint32_t) Result );
        return Result;
    }

    Result = m_CommandPipeline->RegisterPacket( Packet );
    if ( FAILED( Result ) )
    {
        m_CommandPipeline->Deinitialize();
        m_CommandPipeline = NULL;
        Packet->Deinitialize();
        Packet = NULL;
        m_MediaCenter = NULL;
        DeinitializeNetwork();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::Initialize, ServerCommandPipelineManager object RegisterPacket failed, function return %x \n ", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::Initialize end, passed Server_IP=%x, Server_UDP_Port=%d, SessionBufferSize=%d, function return =%x \n", ntohl(Server_IP), ntohs(Server_UDP_Port), SessionBufferSize, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterService::Deinitialize()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::Deinitialize begin \n" );

    if ( !m_UseCallerThreadDoDispatchLoop )
    {
        m_ThreadExitFlag = true;
        while ( m_ThreadWorking ) GMI_Sleep( 10 );
        m_UseCallerThreadDoDispatchLoop = false;
        m_DispatchThread.Destroy();
    }

    if ( NULL != m_CommandPipeline.GetPtr() )
    {
        m_CommandPipeline->Stop();
        m_CommandPipeline->Deinitialize();
        m_CommandPipeline = NULL;
    }

    if ( NULL != m_MediaCenter.GetPtr() )
    {
        m_MediaCenter->ReleaseCodecResource();
        m_MediaCenter = NULL;
    }

    DeinitializeNetwork();
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::Deinitialize end, function return %x \n", (uint32_t) GMI_SUCCESS );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterService::RegisterCommandExecutor( SafePtr<BaseCommandExecutor> CommandExecutor )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::RegisterCommandExecutor begin, passed CommandExecutor type=%d, id=%d \n", CommandExecutor->GetCommandType(), CommandExecutor->GetCommandId() );
    GMI_RESULT Result = m_CommandPipeline->RegisterCommandExecutor( CommandExecutor );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::RegisterCommandExecutor end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterService::UnregisterCommandExecutor( uint32_t CommandId )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::UnregisterCommandExecutor begin, passed CommandId=%d\n", CommandId );
    GMI_RESULT Result = m_CommandPipeline->UnregisterCommandExecutor( CommandId );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::UnregisterCommandExecutor end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterService::Run( boolean_t UseCallerThreadDoDispatchLoop )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::Run begin, passed UseCallerThreadDoDispatchLoop=%d\n", UseCallerThreadDoDispatchLoop );

    GMI_RESULT Result = m_CommandPipeline->Start( 1, 1, 20 );// 20ms is an experience value, to improve command response speed.
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterService::Run, CommandPipeline start failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_CommandPipeline->Run( false );
    if ( FAILED( Result ) )
    {
        m_CommandPipeline->Stop();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterService::Run, CommandPipeline run failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    m_UseCallerThreadDoDispatchLoop = UseCallerThreadDoDispatchLoop;

    m_ThreadWorking = false;
    m_ThreadExitFlag = false;

    if ( UseCallerThreadDoDispatchLoop )
    {
        void_t *ReturnValue = DispatchEntry();
        size_t ReturnInteger = (size_t) ReturnValue;
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::Run end, function return %x \n", (uint32_t) ReturnInteger );
        return (GMI_RESULT) ReturnInteger;
    }

    Result = m_DispatchThread.Create( NULL, 0, MediaCenterService::DispatchThread, this );
    if ( FAILED( Result ) )
    {
        m_CommandPipeline->Stop();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterService::Run, DispatchThread.Create failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_DispatchThread.Start();
    if ( FAILED( Result ) )
    {
        m_DispatchThread.Destroy();
        m_CommandPipeline->Stop();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterService::Run, DispatchThread.Start failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::Run end, function return %x \n", Result );
    return Result;
}

GMI_RESULT MediaCenterService::InitializeNetwork( long_t Server_IP, uint16_t Server_UDP_Port, size_t SessionBufferSize )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::InitializeNetwork begin, passed Server_IP=%x, Server_UDP_Port=%d, SessionBufferSize=%d \n", ntohl(Server_IP), ntohs(Server_UDP_Port), SessionBufferSize );

    m_UDP_Socket = BaseMemoryManager::Instance().New<GMI_Socket>();
    if ( NULL == m_UDP_Socket.GetPtr() )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::InitializeNetwork, GMI_Socket object allocation failed, function return %x \n ", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    GMI_RESULT Result = m_UDP_Socket->Create( SPF_INET, ST_DATAGRAM, SP_UDP );
    if ( FAILED( Result ) )
    {
        m_UDP_Socket = NULL;
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::InitializeNetwork, GMI_Socket object creation failed, function return %x \n ", (uint32_t) Result );
        return Result;
    }

    struct sockaddr_in SocketAddress;
    SocketAddress.sin_family = (int16_t) GMI_GetSystemSocketProtocolFamily( SPF_INET );
    SocketAddress.sin_addr.s_addr = Server_IP;
    SocketAddress.sin_port = Server_UDP_Port;
    Result = m_UDP_Socket->Bind( (struct sockaddr *) &SocketAddress, sizeof(sockaddr_in) );
    if ( FAILED( Result ) )
    {
        m_UDP_Socket->Close();
        m_UDP_Socket = NULL;
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::InitializeNetwork, GMI_Socket object bind failed, function return %x \n ", (uint32_t) Result );
        return Result;
    }

    m_Server_IP         = Server_IP;
    m_Server_UDP_Port   = Server_UDP_Port;
    m_SessionBufferSize = SessionBufferSize;

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::InitializeNetwork end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterService::DeinitializeNetwork()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::DeinitializeNetwork begin \n" );

    GMI_RESULT Result = m_UDP_Socket->Close();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::DeinitializeNetwork, GMI_Socket object closing failed, function return %x \n ", (uint32_t) Result );
        return Result;
    }

    m_UDP_Socket = NULL;

    std::map< uint64_t, ReferrencePtr<UDPSession,DefaultObjectDeleter,MultipleThreadModel> >::iterator SessionIt = m_UDP_Sessions.begin(), SessionEnd = m_UDP_Sessions.end();
    for ( ; SessionIt != SessionEnd; ++SessionIt )
    {
        SessionIt->second->Close();
    }

    m_UDP_Sessions.clear();
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::DeinitializeNetwork end, function return %x \n ", (uint32_t) Result );
    return Result;
}

void_t* MediaCenterService::DispatchThread( void_t *Argument )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::DispatchThread begin, Argument=%p \n", Argument );
    MediaCenterService *Dispatcher = reinterpret_cast<MediaCenterService*> ( Argument );
    void_t *Return = Dispatcher->DispatchEntry();
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::DispatchThread end, Return=%p \n", Return );
    return Return;
}

void_t* MediaCenterService::DispatchEntry()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::DispatchEntry begin \n" );
    m_ThreadWorking = true;

    GMI_RESULT Result = GMI_FAIL;

    int32_t SelectResult = 0;
    FD_HANDLE SocketFD = m_UDP_Socket->GetFDHandle();

    fd_set ReadFDs, ExceptFDs;
    FD_ZERO( &ReadFDs );
    FD_ZERO( &ExceptFDs );
#if defined( _WIN32 )
    FD_SET( (SOCKET) SocketFD, &ReadFDs );
    FD_SET( (SOCKET) SocketFD, &ExceptFDs );
#elif defined( __linux__ )
    FD_SET( (int32_t) SocketFD, &ReadFDs );
    FD_SET( (int32_t) SocketFD, &ExceptFDs );
#endif
    struct timeval Timeout;
    Timeout.tv_sec = 0;
    Timeout.tv_usec = MEDIA_CENTER_SERVER_MAX_TIMEOUT*1000;

    fd_set TempReadFDs, TempExceptFDs;
    struct timeval TempTimeout;

    static const int32_t MaxUDPPacketSize = UDP_MAX_MESSAGE_LENGTH;
    uint8_t ReceiveBuffer[MaxUDPPacketSize];
    struct sockaddr_in From;
    long_t FromLength = 0;
    size_t Transferred = 0;

    while ( !m_ThreadExitFlag )
    {
        TempReadFDs = ReadFDs;
        TempExceptFDs = ExceptFDs;
        TempTimeout = Timeout;

#if defined( __linux__ )
        SelectResult = ::select( (int32_t)SocketFD+1, &TempReadFDs, NULL, &TempExceptFDs, &TempTimeout );
#elif defined( _WIN32 )
        SelectResult = ::select( 0, &TempReadFDs, NULL, &TempExceptFDs, &TempTimeout );
#endif

        if ( 0 > SelectResult )
        {
            if ( EINTR == errno )
            {
                continue;
            }
            Result = GMI_FAIL;
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterService::DispatchEntry, SelectResult=%d, errno=%d \n", SelectResult, errno );
            break;
        }

        if ( 0 == SelectResult )
        {
            continue;
        }

        FromLength = sizeof(From);
        Result = m_UDP_Socket->ReceiveFrom( ReceiveBuffer, MaxUDPPacketSize, 0, (struct sockaddr *) &From, &FromLength, &Transferred );
        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterService::DispatchEntry, UDP_Socket->ReceiveFrom return %x \n", (uint32_t) Result );
            break;
        }
#if defined( _WIN32 )
        uint64_t UDPSessionKey = From.sin_addr.S_un.S_addr;
#elif defined( __linux__ )
        uint64_t UDPSessionKey = From.sin_addr.s_addr;
#endif

        UDPSessionKey <<= 32;
        UDPSessionKey += From.sin_port;
        std::map< uint64_t, ReferrencePtr<UDPSession,DefaultObjectDeleter,MultipleThreadModel> >::iterator SessionIt = m_UDP_Sessions.find( UDPSessionKey );
        if ( m_UDP_Sessions.end() == SessionIt )
        {
            ReferrencePtr<UDPSession,DefaultObjectDeleter,MultipleThreadModel> UDP_Session( BaseMemoryManager::Instance().New<UDPSession>() );
            if ( NULL == UDP_Session.GetPtr() )
            {
                Result = GMI_OUT_OF_MEMORY;
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterService::DispatchEntry, UDP_Session object allocation failed, function return %x \n", (uint32_t) Result );
                break;
            }
#if defined( _WIN32 )
            Result = UDP_Session->Open( m_UDP_Socket, From.sin_addr.S_un.S_addr, From.sin_port, m_SessionBufferSize );
#elif defined( __linux__ )
            Result = UDP_Session->Open( m_UDP_Socket, From.sin_addr.s_addr, From.sin_port, m_SessionBufferSize );
#endif
            if ( FAILED( Result ) )
            {
                UDP_Session = NULL;
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterService::DispatchEntry, UDP_Session object opening failed, function return %x \n", (uint32_t) Result );
                break;
            }

            m_UDP_Sessions.insert( std::pair<uint64_t, ReferrencePtr<UDPSession,DefaultObjectDeleter,MultipleThreadModel> >(UDPSessionKey, UDP_Session) );
            SessionIt = m_UDP_Sessions.find( UDPSessionKey );
        }

        SessionIt->second->Receive( ReceiveBuffer, Transferred );
        m_CommandPipeline->Parse( SessionIt->second );
    }

    m_ThreadWorking = false;
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenterService::DispatchEntry end, function return %x \n", (uint32_t) Result );
    return (void_t *) size_t(Result);
}
