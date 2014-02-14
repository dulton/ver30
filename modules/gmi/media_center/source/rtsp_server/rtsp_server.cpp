#include "rtsp_server.h"

#include "gmi_media_ctrl.h"
#include "ipc_fw_v3.x_resource.h"
#include "ipc_fw_v3.x_setting.h"
#if defined( __linux__ )
#include "sys_info_readonly.h"
#endif

GMI_RtspServer::GMI_RtspServer(void)
    : m_Scheduler( NULL )
    , m_Environment( NULL )
    , m_AuthenticationDB( NULL )
    , m_RtspServer( NULL )
    , m_RtspEventLoopThread()
    , m_RtspEventLoopThreadExitFlag( 0 )
    , m_RtspEventLoopThreadWorking( false )
    , m_Streams()
{
}

GMI_RtspServer::~GMI_RtspServer(void)
{
}

GMI_RESULT GMI_RtspServer::Initialize  ( uint16_t RTSP_Port )
{
    TaskScheduler *Scheduler = BasicTaskScheduler::createNew();
    if ( NULL == Scheduler )
    {
        return GMI_OUT_OF_MEMORY;
    }

    UsageEnvironment *Environment = BasicUsageEnvironment::createNew(*Scheduler);
    if ( NULL == Environment )
    {
        delete m_Scheduler;
        m_Scheduler = NULL;
        return GMI_OUT_OF_MEMORY;
    }

    UserAuthenticationDatabase  *AuthenticationDB = new UserAuthenticationDatabase( "GMI" );
    if ( NULL == AuthenticationDB )
    {
        m_Environment->reclaim();
        m_Environment = NULL;

        delete m_Scheduler;
        m_Scheduler = NULL;
        return GMI_OUT_OF_MEMORY;
    }

    AuthenticationDB->addUserRecord( "root", "123456" );
    //RTSPServer *RtspServer = RTSPServer::createNew( *Environment, RTSP_Port, AuthenticationDB );
    MyRTSPServer *RtspServer = MyRTSPServer::createNew( *Environment, RTSP_Port, NULL );
    if ( NULL == AuthenticationDB )
    {
        delete m_AuthenticationDB;
        m_AuthenticationDB = NULL;

        m_Environment->reclaim();
        m_Environment = NULL;

        delete m_Scheduler;
        m_Scheduler = NULL;
        return GMI_OUT_OF_MEMORY;
    }

    m_Scheduler = Scheduler;
    m_Environment = Environment;
    m_AuthenticationDB = AuthenticationDB;
    m_RtspServer = RtspServer;
    return GMI_SUCCESS;
}

GMI_RESULT GMI_RtspServer::Deinitialize()
{
    Medium::close( m_RtspServer );
    m_RtspServer = NULL;

    delete m_AuthenticationDB;
    m_AuthenticationDB = NULL;

    m_Environment->reclaim();
    m_Environment = NULL;

    delete m_Scheduler;
    m_Scheduler = NULL;
    return GMI_SUCCESS;
}

static char const* descriptionString = "Session streamed by \"GMI RTSP Server\"";
static Boolean is_ssm = False;
static Boolean reuseFirstSource = True;

GMI_RESULT GMI_RtspServer::Add   ( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, void_t *CodecParameter, size_t CodecParameterLength, FD_HANDLE *TransportHandle )
{
    if ( MEDIA_VIDEO != MediaType )
    {
        return GMI_INVALID_PARAMETER;
    }

    uint32_t   ServerMulticastAddress = 0;
    GMI_RESULT Result = GetServerMulticastAddress( MediaId, &ServerMulticastAddress );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    uint16_t ServerRtpPort = 0;
    Result = GetServer_RTP_UDP_Port( MediaId, &ServerRtpPort );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    uint16_t ClientRtpPort = 0;
    Result = GetClient_RTP_UDP_Port( MediaId, &ClientRtpPort );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    uint16_t IPCMediaDataDispatchServerPort = 0;
    Result = GetIPCMediaDataDispatchServerPort( MediaId, &IPCMediaDataDispatchServerPort );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    uint16_t IPCMediaDataDispatchClientPort = 0;
    Result = GetIPCMediaDataDispatchClientPort( MediaId, &IPCMediaDataDispatchClientPort );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    StreamInfo *Info = BaseMemoryManager::Instance().New<StreamInfo>();

    Info->s_Scheduler = BasicTaskScheduler::createNew();
    Info->s_Environment = BasicUsageEnvironment::createNew(*Info->s_Scheduler);

    char streamName[32], inputFileName[32];
#if defined( __linux__ )
    sprintf(streamName, "stream%d", (MediaId+1));
    sprintf(inputFileName, "live_stream%d", (MediaId+1));
#elif defined( _WIN32 )
    sprintf_s(streamName, 32, "stream%d", (MediaId+1));
    sprintf_s(inputFileName, 32, "live_stream%d", (MediaId+1));
#endif

    Info->s_MediaSession = ServerMediaSession::createNew(*m_Environment, streamName, streamName, descriptionString, is_ssm /*SSM*/);
    ServerMediaSubsession* subsession = H264VideoFileServerMediaSubsession::createNew(*Info->s_Environment, inputFileName, reuseFirstSource, IPCMediaDataDispatchServerPort, IPCMediaDataDispatchClientPort, ServerRtpPort, ServerMulticastAddress );

    Info->s_MediaSession->addSubsession(subsession);
    subsession->setServerAddressAndPortForSDP(0, ClientRtpPort );
    m_RtspServer->addServerMediaSession(Info->s_MediaSession);

    char *url = m_RtspServer->rtspURL(Info->s_MediaSession);
    *m_Environment << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;

    m_Streams.push_back( Info );

    *TransportHandle = Info;
    return GMI_SUCCESS;
}

GMI_RESULT GMI_RtspServer::Remove( FD_HANDLE Transport )
{
    StreamInfo *Info = (StreamInfo *) Transport;

    Medium::close( Info->s_MediaSession );
    Info->s_MediaSession = NULL;

    Info->s_Environment->reclaim();
    Info->s_Environment = NULL;

    delete Info->s_Scheduler;
    Info->s_Scheduler = NULL;

    std::vector<StreamInfo*>::iterator StreamInfoIt = m_Streams.begin(), StreamInfoEnd = m_Streams.end();
    for ( ; StreamInfoIt != StreamInfoEnd; ++StreamInfoIt )
    {
        if ( (StreamInfo*) Transport == *StreamInfoIt )
        {
            m_Streams.erase( StreamInfoIt );
            break;
        }
    }

    return GMI_SUCCESS;
}

GMI_RESULT GMI_RtspServer::Start ()
{
    m_RtspEventLoopThreadExitFlag = 0;
    GMI_RESULT Result = m_RtspEventLoopThread.Create( NULL, 0, RtspThreadEntry, this );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    Result = m_RtspEventLoopThread.Start();
    if ( FAILED( Result ) )
    {
        m_RtspEventLoopThread.Destroy();
        return Result;
    }

    return GMI_SUCCESS;
}

GMI_RESULT GMI_RtspServer::Stop  ()
{
    m_RtspEventLoopThreadExitFlag = 1;
    while ( m_RtspEventLoopThreadWorking )
    {
        GMI_Sleep( 1 );
    }

    m_RtspEventLoopThread.Destroy();

    return GMI_SUCCESS;
}

void_t GMI_RtspServer::SetCallback( RTSP_EVENT_CALLBACK Callback, void_t *UserData )
{
    if ( NULL != m_RtspServer )
    {
        m_RtspServer->SetCallback( Callback, UserData );
    }
}

void_t* GMI_RtspServer::RtspThreadEntry( void_t *Parameter )
{
    GMI_RtspServer *Server = (GMI_RtspServer *) Parameter;
    return Server->RtspEventLoop();
}

void_t* GMI_RtspServer::RtspEventLoop()
{
    m_RtspEventLoopThreadWorking = true;
    m_Scheduler->doEventLoop( &m_RtspEventLoopThreadExitFlag );
    m_RtspEventLoopThreadWorking = false;
    return reinterpret_cast<void_t*>(GMI_SUCCESS);
}

GMI_RESULT GMI_RtspServer::GetServerMulticastAddress( int32_t StreamId, uint32_t *MulticastAddress )
{
    char_t  MulticastAddressKey[MAX_PATH_LENGTH];

    int32_t ServerMulticastAddress = 0;
    int32_t Default_MulticastAddress = 0;

    switch ( StreamId )
    {
    case 0:
        Default_MulticastAddress = inet_addr( GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO1_MULTICAST_ADDRESS );
        break;
    case 1:
        Default_MulticastAddress = inet_addr( GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO2_MULTICAST_ADDRESS );
        break;
    case 2:
        Default_MulticastAddress = inet_addr( GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO3_MULTICAST_ADDRESS );
        break;
    case 3:
        Default_MulticastAddress = inet_addr( GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO4_MULTICAST_ADDRESS );
        break;
    default:
        return GMI_INVALID_PARAMETER;
    }
#if defined( __linux__ )
    sprintf( MulticastAddressKey, "video_%d_%s", StreamId, ONVIF_MEDIA_SERVER_RTSP_CONFIG_SERVER_MULTICAST_ADDRESS );
#elif defined( _WIN32 )
    sprintf_s( MulticastAddressKey, MAX_PATH_LENGTH, "video_%d_%s", StreamId, ONVIF_MEDIA_SERVER_RTSP_CONFIG_SERVER_MULTICAST_ADDRESS );
#endif

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = SysInfoOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    //printf( "GMI_RtspServer::GetServerMulticastAddress, Default_MulticastAddress=%x \n", Default_MulticastAddress );

    Result = SysInfoRead(Handle, ONVIF_MEDIA_SERVER_RTSP_CONFIG_PATH, MulticastAddressKey, Default_MulticastAddress, &ServerMulticastAddress );
    if ( FAILED( Result ) )
    {
		SysInfoClose(Handle);
        return Result;
    }

    Result = SysInfoClose(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    //printf( "GMI_RtspServer::GetServerMulticastAddress, Default_MulticastAddress=%x, ServerMulticastAddress=%x \n", Default_MulticastAddress, ServerMulticastAddress );
#endif

    *MulticastAddress = (uint32_t) ServerMulticastAddress;
    return GMI_SUCCESS;
}

GMI_RESULT GMI_RtspServer::GetServer_RTP_UDP_Port( int32_t StreamId, uint16_t *UDP_Port )
{
    char_t  UDPPortKey[MAX_PATH_LENGTH];

    int32_t Server_RTP_UDP_Port = 0;
    int32_t Default_RTP_UDP_Port = 0;

    switch ( StreamId )
    {
    case 0:
        Default_RTP_UDP_Port = GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO1_RTP;
        break;
    case 1:
        Default_RTP_UDP_Port = GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO2_RTP;
        break;
    case 2:
        Default_RTP_UDP_Port = GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO3_RTP;
        break;
    case 3:
        Default_RTP_UDP_Port = GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO4_RTP;
        break;
    default:
        return GMI_INVALID_PARAMETER;
    }
#if defined( __linux__ )
    sprintf( UDPPortKey, "video_%d_%s", StreamId, ONVIF_MEDIA_SERVER_RTSP_CONFIG_SERVER_RTP_UDP_PORT );
#elif defined( _WIN32 )
    sprintf_s( UDPPortKey, MAX_PATH_LENGTH, "video_%d_%s", StreamId, ONVIF_MEDIA_SERVER_RTSP_CONFIG_SERVER_RTP_UDP_PORT );
#endif

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = SysInfoOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    //printf( "GMI_RtspServer::GetServer_RTP_UDP_Port, Default_RTP_UDP_Port=%d \n", Default_RTP_UDP_Port );

    Result = SysInfoRead(Handle, ONVIF_MEDIA_SERVER_RTSP_CONFIG_PATH, UDPPortKey, Default_RTP_UDP_Port, &Server_RTP_UDP_Port );
    if ( FAILED( Result ) )
    {
		SysInfoClose(Handle);
        return Result;
    }

    Result = SysInfoClose(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    //printf( "GMI_RtspServer::GetServer_RTP_UDP_Port, Default_RTP_UDP_Port=%d, Server_RTP_UDP_Port=%d \n", Default_RTP_UDP_Port, Server_RTP_UDP_Port );
#endif

    *UDP_Port = (uint16_t) Server_RTP_UDP_Port;
    return GMI_SUCCESS;
}

GMI_RESULT GMI_RtspServer::GetServer_RTCP_UDP_Port( int32_t StreamId, uint16_t *UDP_Port )
{
    char_t  UDPPortKey[MAX_PATH_LENGTH];

    int32_t Server_RTCP_UDP_Port = 0;
    int32_t Default_RTCP_UDP_Port = 0;

    switch ( StreamId )
    {
    case 0:
        Default_RTCP_UDP_Port = GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO1_RTCP;
        break;
    case 1:
        Default_RTCP_UDP_Port = GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO2_RTCP;
        break;
    case 2:
        Default_RTCP_UDP_Port = GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO3_RTCP;
        break;
    case 3:
        Default_RTCP_UDP_Port = GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO4_RTCP;
        break;
    default:
        return GMI_INVALID_PARAMETER;
    }

#if defined( __linux__ )
    sprintf( UDPPortKey, "video_%d_%s", StreamId, ONVIF_MEDIA_SERVER_RTSP_CONFIG_SERVER_RTCP_UDP_PORT );
#elif defined( _WIN32 )
    sprintf_s( UDPPortKey, MAX_PATH_LENGTH, "video_%d_%s", StreamId, ONVIF_MEDIA_SERVER_RTSP_CONFIG_SERVER_RTCP_UDP_PORT );
#endif

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = SysInfoOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    //printf( "GMI_RtspServer::GetServer_RTCP_UDP_Port, Default_RTCP_UDP_Port=%d \n", Default_RTCP_UDP_Port );

    Result = SysInfoRead(Handle, ONVIF_MEDIA_SERVER_RTSP_CONFIG_PATH, UDPPortKey, Default_RTCP_UDP_Port, &Server_RTCP_UDP_Port );
    if ( FAILED( Result ) )
    {
		SysInfoClose(Handle);
        return Result;
    }

    Result = SysInfoClose(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    //printf( "GMI_RtspServer::GetServer_RTCP_UDP_Port, Default_RTCP_UDP_Port=%d, Server_RTCP_UDP_Port=%d \n", Default_RTCP_UDP_Port, Server_RTCP_UDP_Port );
#endif

    *UDP_Port = (uint16_t) Server_RTCP_UDP_Port;
    return GMI_SUCCESS;
}

GMI_RESULT GMI_RtspServer::GetClient_RTP_UDP_Port( int32_t StreamId, uint16_t *UDP_Port )
{
    char_t  UDPPortKey[MAX_PATH_LENGTH];

    int32_t Client_RTP_UDP_Port = 0;
    int32_t Default_RTP_UDP_Port = 0;

    switch ( StreamId )
    {
    case 0:
        Default_RTP_UDP_Port = GMI_ONVIF_RTSP_CLIENT_ENCODE_VIDEO1_RTP;
        break;
    case 1:
        Default_RTP_UDP_Port = GMI_ONVIF_RTSP_CLIENT_ENCODE_VIDEO2_RTP;
        break;
    case 2:
        Default_RTP_UDP_Port = GMI_ONVIF_RTSP_CLIENT_ENCODE_VIDEO3_RTP;
        break;
    case 3:
        Default_RTP_UDP_Port = GMI_ONVIF_RTSP_CLIENT_ENCODE_VIDEO4_RTP;
        break;
    default:
        return GMI_INVALID_PARAMETER;
    }

#if defined( __linux__ )
    sprintf( UDPPortKey, "video_%d_%s", StreamId, ONVIF_MEDIA_SERVER_RTSP_CONFIG_CLIENT_RTP_UDP_PORT );
#elif defined( _WIN32 )
    sprintf_s( UDPPortKey, MAX_PATH_LENGTH, "video_%d_%s", StreamId, ONVIF_MEDIA_SERVER_RTSP_CONFIG_CLIENT_RTP_UDP_PORT );
#endif

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = SysInfoOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    //printf( "GMI_RtspServer::GetClient_RTP_UDP_Port, Default_RTP_UDP_Port=%d \n", Default_RTP_UDP_Port );

    Result = SysInfoRead(Handle, ONVIF_MEDIA_SERVER_RTSP_CONFIG_PATH, UDPPortKey, Default_RTP_UDP_Port, &Client_RTP_UDP_Port );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    Result = SysInfoClose(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    //printf( "GMI_RtspServer::GetClient_RTP_UDP_Port, Default_RTP_UDP_Port=%d, Client_RTP_UDP_Port=%d \n", Default_RTP_UDP_Port, Client_RTP_UDP_Port );
#endif

    *UDP_Port = (uint16_t) Client_RTP_UDP_Port;
    return GMI_SUCCESS;
}

GMI_RESULT GMI_RtspServer::GetClient_RTCP_UDP_Port( int32_t StreamId, uint16_t *UDP_Port )
{
    char_t  UDPPortKey[MAX_PATH_LENGTH];

    int32_t Client_RTCP_UDP_Port = 0;
    int32_t Default_RTCP_UDP_Port = 0;

    switch ( StreamId )
    {
    case 0:
        Default_RTCP_UDP_Port = GMI_ONVIF_RTSP_CLIENT_ENCODE_VIDEO1_RTCP;
        break;
    case 1:
        Default_RTCP_UDP_Port = GMI_ONVIF_RTSP_CLIENT_ENCODE_VIDEO2_RTCP;
        break;
    case 2:
        Default_RTCP_UDP_Port = GMI_ONVIF_RTSP_CLIENT_ENCODE_VIDEO3_RTCP;
        break;
    case 3:
        Default_RTCP_UDP_Port = GMI_ONVIF_RTSP_CLIENT_ENCODE_VIDEO4_RTCP;
        break;
    default:
        return GMI_INVALID_PARAMETER;
    }

#if defined( __linux__ )
    sprintf( UDPPortKey, "video_%d_%s", StreamId, ONVIF_MEDIA_SERVER_RTSP_CONFIG_CLIENT_RTCP_UDP_PORT );
#elif defined( _WIN32 )
    sprintf_s( UDPPortKey, MAX_PATH_LENGTH, "video_%d_%s", StreamId, ONVIF_MEDIA_SERVER_RTSP_CONFIG_CLIENT_RTCP_UDP_PORT );
#endif

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = SysInfoOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    //printf( "GMI_RtspServer::GetClient_RTCP_UDP_Port, Default_RTP_UDP_Port=%d \n", Default_RTCP_UDP_Port );

    Result = SysInfoRead(Handle, ONVIF_MEDIA_SERVER_RTSP_CONFIG_PATH, UDPPortKey, Default_RTCP_UDP_Port, &Client_RTCP_UDP_Port );
    if ( FAILED( Result ) )
    {
		SysInfoClose(Handle);
        return Result;
    }

    Result = SysInfoClose(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    //printf( "GMI_RtspServer::GetClient_RTCP_UDP_Port, Default_RTCP_UDP_Port=%d, Client_RTCP_UDP_Port=%d \n", Default_RTCP_UDP_Port, Client_RTCP_UDP_Port );
#endif

    *UDP_Port = (uint16_t) Client_RTCP_UDP_Port;
    return GMI_SUCCESS;
}

GMI_RESULT GMI_RtspServer::GetIPCMediaDataDispatchServerPort( int32_t StreamId, uint16_t *UDP_Port )
{
    char_t  UDPPortKey[MAX_PATH_LENGTH];

    int32_t Server_UDP_Port = 0;
    int32_t Default_UDP_Port = 0;

    switch ( StreamId )
    {
    case 0:
        Default_UDP_Port = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1;
        break;
    case 1:
        Default_UDP_Port = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO2;
        break;
    case 2:
        Default_UDP_Port = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO3;
        break;
    case 3:
        Default_UDP_Port = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO4;
        break;
    default:
        return GMI_INVALID_PARAMETER;
    }

#if defined( __linux__ )
    sprintf( UDPPortKey, "video_%d_%s", StreamId, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SERVER_UDP_PORT );
#elif defined( _WIN32 )
    sprintf_s( UDPPortKey, MAX_PATH_LENGTH, "video_%d_%s", StreamId, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SERVER_UDP_PORT );
#endif

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = SysInfoOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    //printf( "GMI_RtspServer::GetIPCMediaDataDispatchServerPort, Default_UDP_Port=%d \n", Default_UDP_Port );

    Result = SysInfoRead(Handle, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH, UDPPortKey, Default_UDP_Port, &Server_UDP_Port );
    if ( FAILED( Result ) )
    {
		SysInfoClose(Handle);
        return Result;
    }

    Result = SysInfoClose(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    //printf( "GMI_RtspServer::GetIPCMediaDataDispatchServerPort, Default_UDP_Port=%d, Server_UDP_Port=%d \n", Default_UDP_Port, Server_UDP_Port );
#endif

    *UDP_Port = (uint16_t) Server_UDP_Port;
    return GMI_SUCCESS;
}

GMI_RESULT GMI_RtspServer::GetIPCMediaDataDispatchClientPort( int32_t StreamId, uint16_t *UDP_Port )
{
    char_t  UDPPortKey[MAX_PATH_LENGTH];

    int32_t Client_UDP_Port = 0;
    int32_t Default_UDP_Port = 0;

    switch ( StreamId )
    {
    case 0:
        Default_UDP_Port = GMI_STREAMING_MEDIA_ONVIF_ENCODE_VIDEO1;
        break;
    case 1:
        Default_UDP_Port = GMI_STREAMING_MEDIA_ONVIF_ENCODE_VIDEO2;
        break;
    case 2:
        Default_UDP_Port = GMI_STREAMING_MEDIA_ONVIF_ENCODE_VIDEO3;
        break;
    case 3:
        Default_UDP_Port = GMI_STREAMING_MEDIA_ONVIF_ENCODE_VIDEO4;
        break;
    default:
        return GMI_INVALID_PARAMETER;
    }

#if defined( __linux__ )
    sprintf( UDPPortKey, "video_%d_%s", StreamId, ONVIF_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_CLIENT_UDP_PORT );
#elif defined( _WIN32 )
    sprintf_s( UDPPortKey, MAX_PATH_LENGTH, "video_%d_%s", StreamId, ONVIF_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_CLIENT_UDP_PORT );
#endif

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = SysInfoOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    //printf( "GMI_RtspServer::GetIPCMediaDataDispatchClientPort, Default_UDP_Port=%d \n", Default_UDP_Port );

    Result = SysInfoRead(Handle, ONVIF_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH, UDPPortKey, Default_UDP_Port, &Client_UDP_Port );
    if ( FAILED( Result ) )
    {
		SysInfoClose(Handle);
        return Result;
    }

    Result = SysInfoClose(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    //printf( "GMI_RtspServer::GetIPCMediaDataDispatchClientPort, Default_UDP_Port=%d, Server_UDP_Port=%d \n", Default_UDP_Port, Client_UDP_Port );
#endif

    *UDP_Port = (uint16_t) Client_UDP_Port;
    return GMI_SUCCESS;
}
