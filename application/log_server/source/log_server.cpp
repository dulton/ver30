// log_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "gmi_system_headers.h"
#include "log_printer.h"
#include "log_publisher.h"
#include "log_server.h"
#include "log_tcp_server.h"
#include "ipc_fw_v3.x_resource.h"
#include "ipc_fw_v3.x_setting.h"
#if defined( __linux__ )
#include "gmi_config_api.h"
#include "gmi_daemon_heartbeat_api.h"
#endif

void* DaemonHeartbeatProc( void *Argument );

GMI_RESULT GetHeartbeatInterval( uint32_t *Interval );
GMI_RESULT GetLogServerPipeName( char_t *PipeName, size_t *BufferLength );
GMI_RESULT GetLogServerPipeMutexId( uint32_t *MutexId );
GMI_RESULT GetLogPublishServerAddress( uint32_t *Address );
GMI_RESULT GetLogPublishServerPort( uint16_t *Port );

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

#if defined( __linux__ )
    char_t PipeName[MAX_CHAR_BUF_LEN];
    size_t PipeNameLength = MAX_CHAR_BUF_LEN;
#elif defined( _WIN32 )
    char_t PipeName[MAX_PATH_LENGTH];
    size_t PipeNameLength = MAX_PATH_LENGTH;
#endif
    Result = GetLogServerPipeName( PipeName, &PipeNameLength );
    if ( FAILED( Result ) )
    {
        Publisher.Deinitialize();
        Printer.Deinitialize();
        printf( "get log server pipe name fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    uint32_t MutexId = 0;
    Result = GetLogServerPipeMutexId( &MutexId );
    if ( FAILED( Result ) )
    {
        Publisher.Deinitialize();
        Printer.Deinitialize();
        printf( "get log server pipe mutex id fail, Result=%x \n", (uint32_t) Result );
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

    LogServer Server;
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

    Result = Server.Initialize( PipeName, MutexId, GMI_IPC_LOG_FILE_PATH );
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

#define LOG_SERVER_CONFIG_PATH                    "/Config/log_server/"
#define LOG_SERVER_CONFIG_HEARTBEAT_INTERVAL      "heartbeat_interval"
#define LOG_SERVER_CONFIG_SERVER_PIPE_NAME        "server_pipe_name"
#define LOG_SERVER_CONFIG_SERVER_MUTEX_ID         "server_mutex_id"

#define LOG_SERVER_CONFIG_PUBLISH_SERVER_ADDRESS  "publish_server_address"
#define LOG_SERVER_CONFIG_PUBLISH_SERVER_PORT     "publish_server_port"

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

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "log server, GetHeartbeatInterval, Default_Interval=%d, Interval=%d \n", GMI_LOG_SERVER_HEARTBEAT_INTERVAL, *Interval );

#elif defined( _WIN32 )
    *Interval = GMI_LOG_SERVER_HEARTBEAT_INTERVAL;
#endif
    return GMI_SUCCESS;
}

GMI_RESULT GetLogServerPipeName( char_t *PipeName, size_t *BufferLength )
{
#if defined( __linux__ )

#if 1
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        printf( "GetLogServerPipeName, GMI_XmlOpen, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( "log server, GetLogServerPipeName, Default_PipeName=%s \n", LOG_SERVER_DEFAULT_PIPE_NAME );

    Result = GMI_XmlRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_SERVER_PIPE_NAME, LOG_SERVER_DEFAULT_PIPE_NAME, PipeName, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        printf( "GetLogServerPipeName, GMI_XmlRead, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        printf( "GetLogServerPipeName, GMI_XmlFileSave, Result=%x \n", (uint32_t) Result );
        return Result;
    }
#else
    // GMI_XmlRead can not read string for now, so we copy a test string
    strcpy( PipeName, LOG_SERVER_DEFAULT_PIPE_NAME );
#endif
    printf( "log server, GetLogServerPipeName, Default_PipeName=%s, PipeName=%s \n", LOG_SERVER_DEFAULT_PIPE_NAME, PipeName );

#elif defined( _WIN32 )
    strcpy_s( PipeName, *BufferLength, LOG_SERVER_DEFAULT_PIPE_NAME );
#endif
    *BufferLength = strlen( PipeName );
    return GMI_SUCCESS;
}

GMI_RESULT GetLogServerPipeMutexId( uint32_t *MutexId )
{
#if defined( __linux__ )

    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "log server, GetLogServerPipeMutexId, Default_MutexId=%d \n", LOG_SERVER_DEFAULT_PIPE_MUTEX_ID );

    Result = GMI_XmlRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_SERVER_MUTEX_ID, LOG_SERVER_DEFAULT_PIPE_MUTEX_ID, (int32_t *) MutexId, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "log server, GetLogServerPipeMutexId, Default_MutexId=%d, MutexId=%d \n", LOG_SERVER_DEFAULT_PIPE_MUTEX_ID, *MutexId );

#elif defined( _WIN32 )
    *MutexId = LOG_SERVER_DEFAULT_PIPE_MUTEX_ID;
#endif
    return GMI_SUCCESS;
}

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

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "GetLogPublishServerPort, Default_Port=%d, ServerPort=%d \n", GMI_LOG_PUBLISH_SERVER_PORT, ServerPort );

    *Port = htons((uint16_t)ServerPort);
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
