// log_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "gmi_system_headers.h"
#include "log_printer.h"
#include "log_publisher.h"
#include "log_tcp_server.h"
#include "ipc_fw_v3.x_resource.h"
#include "ipc_fw_v3.x_setting.h"
#if defined( __linux__ )
#include "gmi_config_api.h"
#include "gmi_daemon_heartbeat_api.h"
#endif
#include "share_memory_log_server.h"

void* DaemonHeartbeatProc( void *Argument );

GMI_RESULT GetLogPublishServerAddress( uint32_t *Address );
GMI_RESULT GetLogPublishServerPort( uint16_t *Port );
GMI_RESULT GetLogServerConfig( uint16_t *ServerPort, long_t *ShareMemoryKey, size_t *ShareMemorySize, long_t *IpcMutexKey, uint32_t *DebugLogLevel );
GMI_RESULT GetHeartbeatInterval( uint32_t *Interval );

static uint32_t l_Heartbeat_Interval = GMI_LOG_SERVER_HEARTBEAT_INTERVAL;

#if defined( __linux__ )
void_t SIGPIPE_SignalProcess( int32_t SignalNumber )
{
    printf( "log_server: read side of pipe is already close. SignalNumber=%d\n", SignalNumber );
}
#endif

#define TEST_NO_HEARTBEAT_CASE           0
#define TEST_HEARTBEAT_COUNT             30
#define INIT_NETWORK                     1

#define LOOPBACK_IP                      "127.0.0.1"
#define DEFAULT_IP                       "0.0.0.0"
#define DEFAULT_LOG_PUBLISH_SERVER_IP    "192.168.0.247"

#if INIT_NETWORK
class NetworkInitializer;
#endif

#if defined( _WIN32 )
int32_t _tmain( int32_t argc, _TCHAR* argv[] )
#elif defined( __linux__ )
int32_t main( int32_t argc, char_t* argv[] )
#endif
{
#if defined( __linux__ )
    signal( SIGPIPE , SIGPIPE_SignalProcess );
#endif

#if INIT_NETWORK
    SafePtr<NetworkInitializer> Initializer( BaseMemoryManager::Instance().New<NetworkInitializer>() );
    if ( NULL == Initializer.GetPtr() )
    {
        printf( "network initialization fail \n" );
        return -1;
    }
#endif

    LogPrinter Printer;
    GMI_RESULT Result = Printer.Initialize( NULL, 0 );
    if ( FAILED( Result ) )
    {
        printf( "log printer initialization fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Printer.ProcessUserLog( true );
    Printer.ProcessDebugLog( true );

    uint32_t PublishServerAddress = 0;
    Result = GetLogPublishServerAddress( &PublishServerAddress );
    if ( FAILED( Result ) )
    {
        Printer.Deinitialize();
        printf( "get log publish server address fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    uint16_t PublishServerPort = 0;
    Result = GetLogPublishServerPort( &PublishServerPort );
    if ( FAILED( Result ) )
    {
        Printer.Deinitialize();
        printf( "get log publish server port fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    struct sockaddr_in SocketAddress;
    SocketAddress.sin_family = (int16_t) GMI_GetSystemSocketProtocolFamily( SPF_INET );
    SocketAddress.sin_addr.s_addr = PublishServerAddress;
    SocketAddress.sin_port = PublishServerPort;

    Log_TCP_Server Publisher;
    Result = Publisher.Initialize( &SocketAddress, sizeof(sockaddr_in) );
    if ( FAILED( Result ) )
    {
        Printer.Deinitialize();
        printf( "Publisher.Initialize failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Publisher.ProcessUserLog( true );
    Publisher.ProcessDebugLog( true );

    uint16_t ServerPort = 0;
    long_t   ShareMemoryKey = 0;
    size_t   ShareMemorySize = 0;
    long_t   IpcMutexKey = 0;
    uint32_t DebugLogLevel = 0;
    Result = GetLogServerConfig( &ServerPort, &ShareMemoryKey, &ShareMemorySize, &IpcMutexKey, &DebugLogLevel );
    if ( FAILED( Result ) )
    {
        Publisher.Deinitialize();
        Printer.Deinitialize();
        printf( "get log server config fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = GetHeartbeatInterval( &l_Heartbeat_Interval );
    if ( FAILED( Result ) )
    {
        Publisher.Deinitialize();
        Printer.Deinitialize();
        printf( "get log server heartbeat interval fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

#define USE_PIPE_TO_IMPLEMENT_LOG 1

    ShareMemoryLogServer Server;
#if USE_PIPE_TO_IMPLEMENT_LOG
    GMI_Thread HeartBeatThread;
    Result = HeartBeatThread.Create( NULL, 0, DaemonHeartbeatProc, &Server );
    if ( FAILED( Result ) )
    {
        Publisher.Deinitialize();
        Printer.Deinitialize();
        printf( "heartbeat thread creating fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = HeartBeatThread.Start();
    if ( FAILED( Result ) )
    {
        HeartBeatThread.Destroy();
        Publisher.Deinitialize();
        Printer.Deinitialize();
        printf( "heartbeat thread starting fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }
#endif

    Result = Server.Initialize( ServerPort, ShareMemoryKey, ShareMemorySize, IpcMutexKey, GMI_IPC_LOG_FILE_PATH, DebugLogLevel );
    if ( FAILED( Result ) )
    {
#if USE_PIPE_TO_IMPLEMENT_LOG
        HeartBeatThread.Stop();
        HeartBeatThread.Destroy();
#endif
        Publisher.Deinitialize();
        Printer.Deinitialize();
        printf( "log_server initialization fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Server.Register( &Printer );
    if ( FAILED( Result ) )
    {
#if USE_PIPE_TO_IMPLEMENT_LOG
        HeartBeatThread.Stop();
        HeartBeatThread.Destroy();
#endif
        Server.Deinitialize();
        Publisher.Deinitialize();
        Printer.Deinitialize();
        printf( "log_server register log printer fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Server.Register( &Publisher );
    if ( FAILED( Result ) )
    {
        Server.Unregister( &Printer );
        Server.Deinitialize();
        Publisher.Deinitialize();
        Printer.Deinitialize();
        printf( "log_server register log publisher fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

#if USE_PIPE_TO_IMPLEMENT_LOG
    Result = Server.Run( true );
#else
    Result = Server.Run( false );
#endif
    if ( FAILED( Result ) )
    {
#if USE_PIPE_TO_IMPLEMENT_LOG
        HeartBeatThread.Stop();
        HeartBeatThread.Destroy();
#endif
        Server.Unregister( &Publisher );
        Server.Unregister( &Printer );
        Server.Deinitialize();
        Publisher.Deinitialize();
        Printer.Deinitialize();
        printf( "log_server run fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

#if !USE_PIPE_TO_IMPLEMENT_LOG
    DaemonHeartbeatProc( &Server );
#endif

    Result = Server.Unregister( &Publisher );
    Result = Server.Unregister( &Printer );
    Result = Server.Deinitialize();
#if USE_PIPE_TO_IMPLEMENT_LOG
    Result = HeartBeatThread.Stop();
    Result = HeartBeatThread.Destroy();
#endif
    Result = Publisher.Deinitialize();
    Result = Printer.Deinitialize();
    return 0;
}

void* DaemonHeartbeatProc( void *Argument )
{
#if defined( __linux__ )
    uint32_t Flags = APPLICATION_RUNNING;

    DAEMON_DATA_CFG DaemonData;
    GMI_RESULT Result = GMI_DaemonInit( &DaemonData, LOG_SERVER_ID, GMI_DAEMON_HEARDBEAT_SERVER, GMI_DAEMON_HEARDBEAT_LOG );
    if ( FAILED( Result ) )
    {
        printf( "log_server initialization fail \n" );
        return (void_t*) GMI_FAIL;
    }

    do
    {
        GMI_RESULT Result = GMI_DaemonRegister(&DaemonData);
        if ( SUCCEEDED( Result ) )
        {
            break;
        }
        GMI_Sleep( 10 );
    }
    while ( 1 );
#endif

#if TEST_NO_HEARTBEAT_CASE
    long_t Count = TEST_HEARTBEAT_COUNT;
#endif

    while ( 1 )
    {
#if defined( __linux__ )
#if TEST_NO_HEARTBEAT_CASE
        if ( 0 < Count )
        {
            GMI_DaemonReport(&DaemonData, &Flags );
            --Count;
            if (APPLICATION_QUIT == Flags)
            {
                break;
            }
        }
#else
        GMI_DaemonReport(&DaemonData, &Flags );
        if (APPLICATION_QUIT == Flags)
        {
            break;
        }
#endif
#endif
        GMI_Sleep( l_Heartbeat_Interval );
    }

#if defined( __linux__ )
    GMI_DaemonUnInit(&DaemonData);
#endif
    return (void_t*) GMI_SUCCESS;
}


#define LOG_SERVER_CONFIG_SERVER_UDP_PORT         "server_udp_port"

#define LOG_SERVER_CONFIG_PUBLISH_SERVER_ADDRESS  "publish_server_address"
#define LOG_SERVER_CONFIG_PUBLISH_SERVER_PORT     "publish_server_port"
#define LOG_SERVER_CONFIG_DEBUG_LOG_LEVEL         "debug_log_level"

#define LOG_SERVER_CONFIG_HEARTBEAT_INTERVAL      "heartbeat_interval"

GMI_RESULT GetLogPublishServerAddress( uint32_t *Address )
{
#if defined( __linux__ )

    // upgrading server use "0.0.0.0" as upgrading server socket address, we can be the same as it, use 0 directly as log server address
    //*Address = inet_addr(LOOPBACK_IP);//2013/08/01
    *Address = inet_addr(DEFAULT_IP);//2013/09/05

    printf( "GetLogPublishServerAddress, Address=%d \n", *Address );

#elif defined( _WIN32 )
    const size_t ServerIPLength = 128;
    char_t   ServerIP[ServerIPLength];
    memset( ServerIP, 0, ServerIPLength );

    printf( "please input log publish server IP, for example: %s\n", DEFAULT_LOG_PUBLISH_SERVER_IP );
#if defined( __linux__ )
    int32_t ScanfResult = scanf( "%s", ServerIP );
#elif defined( _WIN32 )
    int32_t ScanfResult = scanf_s( "%s", ServerIP, ServerIPLength );
#endif
    READ_MYSELF( ScanfResult );

    *Address = inet_addr(ServerIP);
#endif
    return GMI_SUCCESS;
}

GMI_RESULT GetLogPublishServerPort( uint16_t *Port )
{
#if defined( __linux__ )

    int32_t ServerPort = 0;

    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "GetLogPublishServerPort, Default_Port=%d \n", GMI_LOG_PUBLISH_SERVER_PORT );
    Result = GMI_XmlRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_PUBLISH_SERVER_PORT, GMI_LOG_PUBLISH_SERVER_PORT, &ServerPort, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }
    printf( "GetLogPublishServerPort, Default_Port=%d, ServerPort=%d \n", GMI_LOG_PUBLISH_SERVER_PORT, ServerPort );
    *Port = htons((uint16_t)ServerPort);

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

#elif defined( _WIN32 )
    uint32_t ServerPort = 0;

    printf( "please input log publish server port, for example: %d\n", GMI_LOG_PUBLISH_SERVER_PORT );
#if defined( __linux__ )
    int32_t ScanfResult = scanf( "%d", &ServerPort );
#elif defined( _WIN32 )
    int32_t ScanfResult = scanf_s( "%d", &ServerPort );
#endif
    READ_MYSELF( ScanfResult );

    *Port = htons((uint16_t)ServerPort);
#endif
    return GMI_SUCCESS;
}

GMI_RESULT GetLogServerConfig( uint16_t *ServerPort, long_t *ShareMemoryKey, size_t *ShareMemorySize, long_t *IpcMutexKey, uint32_t *DebugLogLevel )
{
    int32_t TempServerPort = 0;
#if defined( __linux__ )

    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "log server, GetLogServerUDPPort, Default_UDP_Port=%d \n", LOG_SERVER_DEFAULT_SERVER_PORT );
    Result = GMI_XmlRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_SERVER_UDP_PORT, LOG_SERVER_DEFAULT_SERVER_PORT, (int32_t *) &TempServerPort, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }
    printf( "log server, GetLogServerUDPPort, Default_UDP_Port=%d, UDP_Port=%d \n", LOG_SERVER_DEFAULT_SERVER_PORT, TempServerPort );

    printf( "log server, GetLogServerShareMemoryKey, DefaultShareMemoryKey=%d \n", GMI_LOG_SERVER_DEFAUL_SHARE_MEMORY_KEY );
    Result = GMI_XmlRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_SHARE_MEMORY_KEY, GMI_LOG_SERVER_DEFAUL_SHARE_MEMORY_KEY, (int32_t *) ShareMemoryKey, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }
    printf( "log server, GetLogServerShareMemoryKey, DefaultShareMemoryKey=%d, ShareMemoryKey=%ld \n", GMI_LOG_SERVER_DEFAUL_SHARE_MEMORY_KEY, *ShareMemoryKey );

    printf( "log server, GetLogServerIpcMutexKey, DefaultIpcMutexKey=%d \n", GMI_LOG_SERVER_DEFAULT_SHARE_MEMORY_IPC_MUTEX_ID );
    Result = GMI_XmlRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_SHARE_MEMORY_MUTEX_ID, GMI_LOG_SERVER_DEFAULT_SHARE_MEMORY_IPC_MUTEX_ID, (int32_t *) IpcMutexKey, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }
    printf( "log server, GetLogServerIpcMutexKey, DefaultIpcMutexKey=%d, IpcMutexKey=%ld \n", GMI_LOG_SERVER_DEFAULT_SHARE_MEMORY_IPC_MUTEX_ID, *IpcMutexKey );

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    Result = GMI_XmlOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "log server, GetLogServerShareMemorySize, DefaultShareMemorySize=%d \n", GMI_LOG_DEFAULT_SHARE_MEMORY_SIZE );
    Result = GMI_XmlRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_SHARE_MEMORY_SIZE, GMI_LOG_DEFAULT_SHARE_MEMORY_SIZE, (int32_t *) ShareMemorySize, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }
    printf( "log server, GetLogServerShareMemorySize, DefaultShareMemorySize=%d, ShareMemorySize=%d \n", GMI_LOG_DEFAULT_SHARE_MEMORY_SIZE, *ShareMemorySize );

    printf( "log server, GetLogServerDebugLevel, DefaultDebugLogLevel=%d \n", GMI_LOG_MODULE_LOG_DEBUG_LOG_LEVEL );
    Result = GMI_XmlRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_DEBUG_LOG_LEVEL, GMI_LOG_MODULE_LOG_DEBUG_LOG_LEVEL, (int32_t *) DebugLogLevel, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }
    printf( "log server, GetLogServerDebugLevel, DefaultDebugLogLevel=%d, DebugLogLevel=%d \n", GMI_LOG_MODULE_LOG_DEBUG_LOG_LEVEL, *DebugLogLevel );

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

#elif defined( _WIN32 )
    TempServerPort    = LOG_SERVER_DEFAULT_SERVER_PORT;
    *ShareMemoryKey   = GMI_LOG_SERVER_DEFAUL_SHARE_MEMORY_KEY;
    *IpcMutexKey      = GMI_LOG_SERVER_DEFAULT_SHARE_MEMORY_IPC_MUTEX_ID;

    *ShareMemorySize  = GMI_LOG_DEFAULT_SHARE_MEMORY_SIZE;
    *DebugLogLevel    = GMI_LOG_MODULE_LOG_DEBUG_LOG_LEVEL;
#endif
    *ServerPort       = (uint16_t) TempServerPort;

    return GMI_SUCCESS;
}

GMI_RESULT GetHeartbeatInterval( uint32_t *Interval )
{
#if defined( __linux__ )

    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "log server, GetHeartbeatInterval, Default_Interval=%d \n", GMI_LOG_SERVER_HEARTBEAT_INTERVAL );
    Result = GMI_XmlRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_HEARTBEAT_INTERVAL, GMI_LOG_SERVER_HEARTBEAT_INTERVAL, (int32_t *) Interval, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }
    printf( "log server, GetHeartbeatInterval, Default_Interval=%d, Interval=%d \n", GMI_LOG_SERVER_HEARTBEAT_INTERVAL, *Interval );

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

#elif defined( _WIN32 )
    *Interval = GMI_LOG_SERVER_HEARTBEAT_INTERVAL;
#endif
    return GMI_SUCCESS;
}

#if INIT_NETWORK
class NetworkInitializer
{
public:
    NetworkInitializer()
    {
#if defined( _WIN32 )
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;

        wVersionRequested = MAKEWORD( 2, 2 );

        err = WSAStartup( wVersionRequested, &wsaData );
        if ( err != 0 )
        {
            /* Tell the user that we could not find a usable */
            /* WinSock DLL.                                  */
            return;
        }

        /* Confirm that the WinSock DLL supports 2.2.*/
        /* Note that if the DLL supports versions greater    */
        /* than 2.2 in addition to 2.2, it will still return */
        /* 2.2 in wVersion since that is the version we      */
        /* requested.                                        */

        if ( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 )
        {
            /* Tell the user that we could not find a usable */
            /* WinSock DLL.                                  */
            WSACleanup();
            return;
        }

        /* The WinSock DLL is acceptable. Proceed. */
#elif defined( __linux__ )
#endif
    }

    ~NetworkInitializer()
    {
#if defined( _WIN32 )
        WSACleanup();
#elif defined( __linux__ )
#endif
    }
};
#endif
