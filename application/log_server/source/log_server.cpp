// log_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "gmi_system_headers.h"
#include "log_printer.h"
#include "log_publisher.h"
#include "log_repository.h"
#include "log_tcp_server.h"
#include "ipc_fw_v3.x_resource.h"
#include "ipc_fw_v3.x_setting.h"
#if defined( __linux__ )
#include "gmi_config_api.h"
#include "gmi_daemon_heartbeat_api.h"
#include "sys_info_readonly.h"
#endif
#include "share_memory_log_server.h"

void* DaemonHeartbeatProc( void *Argument );

GMI_RESULT GetLogRepositoryUserLogParameter( char_t *UserLogFilePath, int32_t *UserLogStorageLimitMode, int32_t *UserLogStorageLimitParameter, long_t *UserLogShareMemoryKey, int32_t *UserLogShareMemorySize, long_t *UserLogIpcMutexKey );
GMI_RESULT GetLogRepositoryDebugLogParameter( char_t *DebugLogFilePath, int32_t *DebugLogStorageLimitMode, int32_t *DebugLogStorageLimitParameter, long_t *DebugLogShareMemoryKey, int32_t *DebugLogShareMemorySize, long_t *DebugLogIpcMutexKey );
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

	GMI_RESULT Result = SysInfoReadInitialize();
    if ( FAILED( Result ) )
    {
        printf( "SysInfoReadInitialize fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }
	
    LogPrinter Printer;
    Result = Printer.Initialize( NULL, 0 );
    if ( FAILED( Result ) )
    {
        printf( "log printer initialization fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Printer.ProcessUserLog( true );
    Printer.ProcessDebugLog( true );


    char_t   UserLogFilePath[MAX_PATH_LENGTH];
    int32_t  UserLogStorageLimitMode      = 0;
    int32_t  UserLogStorageLimitParameter = 0;
    long_t   UserLogShareMemoryKey        = 0;
    int32_t  UserLogShareMemorySize       = 0;
    long_t   UserLogIpcMutexKey           = 0;

    Result = GetLogRepositoryUserLogParameter( UserLogFilePath, &UserLogStorageLimitMode, &UserLogStorageLimitParameter, &UserLogShareMemoryKey, &UserLogShareMemorySize, &UserLogIpcMutexKey );
    if ( FAILED( Result ) )
    {
        Printer.Deinitialize();
        printf( "log repository user log parameter getting fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    char_t   DebugLogFilePath[MAX_PATH_LENGTH];
    int32_t  DebugLogStorageLimitMode      = 0;
    int32_t  DebugLogStorageLimitParameter = 0;
    long_t   DebugLogShareMemoryKey        = 0;
    int32_t  DebugLogShareMemorySize       = 0;
    long_t   DebugLogIpcMutexKey           = 0;

    Result = GetLogRepositoryDebugLogParameter( DebugLogFilePath, &DebugLogStorageLimitMode, &DebugLogStorageLimitParameter, &DebugLogShareMemoryKey, &DebugLogShareMemorySize, &DebugLogIpcMutexKey );
    if ( FAILED( Result ) )
    {
        Printer.Deinitialize();
		SysInfoReadDeinitialize();
        printf( "log repository debug log parameter getting fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    struct LogRepositoryInitializationParameter LogInitializationParameter;
    LogInitializationParameter.s_UserLogFilePathType                  = LOG_REPOSITORY_DATA_TYPE_STRING_REFERENCE;
    LogInitializationParameter.s_UserLogFilePath.u_FilePathReference  = UserLogFilePath;
    LogInitializationParameter.s_UserLogStorageLimitMode              = (LogStorageLimitMode) UserLogStorageLimitMode;
    LogInitializationParameter.s_UserLogStorageLimitParameter         = UserLogStorageLimitParameter;
    LogInitializationParameter.s_UserLogShareMemoryKey                = UserLogShareMemoryKey;
    LogInitializationParameter.s_UserLogShareMemorySize               = UserLogShareMemorySize;
    LogInitializationParameter.s_UserLogIpcMutexKey                   = UserLogIpcMutexKey;

    LogInitializationParameter.s_DebugLogFilePathType                 = LOG_REPOSITORY_DATA_TYPE_STRING_REFERENCE;
    LogInitializationParameter.s_DebugLogFilePath.u_FilePathReference = DebugLogFilePath;
    LogInitializationParameter.s_DebugLogStorageLimitMode             = (LogStorageLimitMode) DebugLogStorageLimitMode;
    LogInitializationParameter.s_DebugLogStorageLimitParameter        = DebugLogStorageLimitParameter;
    LogInitializationParameter.s_DebugLogShareMemoryKey               = DebugLogShareMemoryKey;
    LogInitializationParameter.s_DebugLogShareMemorySize              = DebugLogShareMemorySize;
    LogInitializationParameter.s_DebugLogIpcMutexKey                  = DebugLogIpcMutexKey;

    LogRepository Repository;
    Result = Repository.Initialize( &LogInitializationParameter, sizeof(LogInitializationParameter) );
    if ( FAILED( Result ) )
    {
        Printer.Deinitialize();
        printf( "Repository.Initialize failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Repository.ProcessUserLog( true );
    Repository.ProcessDebugLog( false );


    uint32_t PublishServerAddress = 0;
    Result = GetLogPublishServerAddress( &PublishServerAddress );
    if ( FAILED( Result ) )
    {
        Repository.Deinitialize();
        Printer.Deinitialize();
		SysInfoReadDeinitialize();
        printf( "get log publish server address fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    uint16_t PublishServerPort = 0;
    Result = GetLogPublishServerPort( &PublishServerPort );
    if ( FAILED( Result ) )
    {
        Repository.Deinitialize();
        Printer.Deinitialize();
        SysInfoReadDeinitialize();
        printf( "get log publish server port fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    struct sockaddr_in SocketAddress;
    SocketAddress.sin_family      = (int16_t) GMI_GetSystemSocketProtocolFamily( SPF_INET );
    SocketAddress.sin_addr.s_addr = PublishServerAddress;
    SocketAddress.sin_port        = PublishServerPort;

    Log_TCP_Server Publisher;
    Result = Publisher.Initialize( &SocketAddress, sizeof(sockaddr_in) );
    if ( FAILED( Result ) )
    {
        Repository.Deinitialize();
        Printer.Deinitialize();
        SysInfoReadDeinitialize();
        printf( "Publisher.Initialize failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Publisher.ProcessUserLog( true );
    Publisher.ProcessDebugLog( true );

    uint16_t ServerPort      = 0;
    long_t   ShareMemoryKey  = 0;
    size_t   ShareMemorySize = 0;
    long_t   IpcMutexKey     = 0;
    uint32_t DebugLogLevel   = 0;
    Result = GetLogServerConfig( &ServerPort, &ShareMemoryKey, &ShareMemorySize, &IpcMutexKey, &DebugLogLevel );
    if ( FAILED( Result ) )
    {
        Publisher.Deinitialize();
        Repository.Deinitialize();
        Printer.Deinitialize();
        SysInfoReadDeinitialize();
        printf( "get log server config fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = GetHeartbeatInterval( &l_Heartbeat_Interval );
    if ( FAILED( Result ) )
    {
        Publisher.Deinitialize();
        Repository.Deinitialize();
        Printer.Deinitialize();
        SysInfoReadDeinitialize();
        printf( "get log server heartbeat interval fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

#define USE_PIPE_TO_IMPLEMENT_LOG 0

    ShareMemoryLogServer Server;
#if USE_PIPE_TO_IMPLEMENT_LOG
    GMI_Thread HeartBeatThread;
    Result = HeartBeatThread.Create( NULL, 0, DaemonHeartbeatProc, &Server );
    if ( FAILED( Result ) )
    {
        Publisher.Deinitialize();
        Repository.Deinitialize();
        Printer.Deinitialize();
        SysInfoReadDeinitialize();
        printf( "heartbeat thread creating fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = HeartBeatThread.Start();
    if ( FAILED( Result ) )
    {
        HeartBeatThread.Destroy();
        Publisher.Deinitialize();
        Repository.Deinitialize();
        Printer.Deinitialize();
        SysInfoReadDeinitialize();
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
        Repository.Deinitialize();
        Printer.Deinitialize();
        SysInfoReadDeinitialize();
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
        Repository.Deinitialize();
        Printer.Deinitialize();
		SysInfoReadDeinitialize();
        printf( "log_server register log printer fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Server.Register( &Repository );
    if ( FAILED( Result ) )
    {
#if USE_PIPE_TO_IMPLEMENT_LOG
        HeartBeatThread.Stop();
        HeartBeatThread.Destroy();
#endif
        Server.Unregister( &Printer );
        Server.Deinitialize();
        Publisher.Deinitialize();
        Repository.Deinitialize();
        Printer.Deinitialize();
        printf( "log_server register log printer fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Server.Register( &Publisher );
    if ( FAILED( Result ) )
    {
        Server.Unregister( &Repository );
        Server.Unregister( &Printer );
        Server.Deinitialize();
        Publisher.Deinitialize();
        Repository.Deinitialize();
        Printer.Deinitialize();
        SysInfoReadDeinitialize();
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
        Server.Unregister( &Repository );
        Server.Unregister( &Printer );
        Server.Deinitialize();
        Publisher.Deinitialize();
        Repository.Deinitialize();
        Printer.Deinitialize();
        SysInfoReadDeinitialize();
        printf( "log_server run fail, Result=%x \n", (uint32_t) Result );
        return Result;
    }

#if !USE_PIPE_TO_IMPLEMENT_LOG
    DaemonHeartbeatProc( &Server );
#endif

    Result = Server.Unregister( &Publisher );
    Result = Server.Unregister( &Repository );
    Result = Server.Unregister( &Printer );
    Result = Server.Deinitialize();
#if USE_PIPE_TO_IMPLEMENT_LOG
    Result = HeartBeatThread.Stop();
    Result = HeartBeatThread.Destroy();
#endif
    Result = Publisher.Deinitialize();
    Result = Repository.Deinitialize();
    Result = Printer.Deinitialize();
    Result = SysInfoReadDeinitialize();
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

#define LOG_SERVER_CONFIG_USER_LOG_FILE_PATH                 "user_log_file_path"
#define LOG_SERVER_CONFIG_USER_LOG_STORAGE_LIMET_MODE        "user_log_storage_limit_mode"
#define LOG_SERVER_CONFIG_USER_LOG_STORAGE_LIMET_PARAMETER   "user_log_storage_limit_parameter"
#define LOG_SERVER_CONFIG_USER_LOG_SHARE_MEMORY_KEY          "user_log_share_memory_key"
#define LOG_SERVER_CONFIG_USER_LOG_SHARE_MEMORY_SIZE         "user_log_share_memory_size"
#define LOG_SERVER_CONFIG_USER_LOG_IPC_MUTEX_KEY             "user_log_ipc_mutex_key"

GMI_RESULT GetLogRepositoryUserLogParameter( char_t *UserLogFilePath, int32_t *UserLogStorageLimitMode, int32_t *UserLogStorageLimitParameter, long_t *UserLogShareMemoryKey, int32_t *UserLogShareMemorySize, long_t *UserLogIpcMutexKey )
{
#if defined( __linux__ )

    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = SysInfoOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "GetUserLogShareMemoryKey, Default_ShareMemoryKey=%d \n", GMI_USER_LOG_SHARE_MEMORY_KEY );
    Result = SysInfoRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_USER_LOG_SHARE_MEMORY_KEY, GMI_USER_LOG_SHARE_MEMORY_KEY, (int32_t*)UserLogShareMemoryKey);
    if ( FAILED( Result ) )
    {        
		SysInfoClose(Handle);
        return Result;
    }
    printf( "GetUserLogShareMemoryKey, Default_ShareMemoryKey=%d, ShareMemoryKey=%ld \n", GMI_USER_LOG_SHARE_MEMORY_KEY, *UserLogShareMemoryKey );

    printf( "GetUserLogIpcMutexKey, Default_IpcMutexKey=%d \n", GMI_USER_LOG_IPC_MUTEX_KEY );
    Result = SysInfoRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_USER_LOG_IPC_MUTEX_KEY, GMI_USER_LOG_IPC_MUTEX_KEY, (int32_t*)UserLogIpcMutexKey );
    if ( FAILED( Result ) )
    {       
		SysInfoClose(Handle);
        return Result;
    }
    printf( "GetUserLogIpcMutexKey, Default_IpcMutexKey=%d, IpcMutexKey=%ld \n", GMI_USER_LOG_IPC_MUTEX_KEY, *UserLogIpcMutexKey );
	
	Result = SysInfoClose(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    Result = SysInfoOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "log server, GetUserLogFilePath, Default_UserLogFilePath=%s \n", GMI_LOG_DEFAULT_USER_LOG_FILE_PATH );
    Result = SysInfoRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_USER_LOG_FILE_PATH, GMI_LOG_DEFAULT_USER_LOG_FILE_PATH, UserLogFilePath);
    if ( FAILED( Result ) )
    {        
		SysInfoClose(Handle);
        return Result;
    }
    printf( "log server, GetUserLogFilePath, Default_UserLogFilePath=%s, UserLogFilePath=%s \n", GMI_LOG_DEFAULT_USER_LOG_FILE_PATH, UserLogFilePath );

    printf( "log server, GetUserLogStorageLimitMode, Default_UserLogStorageLimitMode=%d \n", GMI_LOG_DEFAULT_USER_LOG_STORAGE_LIMIT_MODE );
    Result = SysInfoRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_USER_LOG_STORAGE_LIMET_MODE, GMI_LOG_DEFAULT_USER_LOG_STORAGE_LIMIT_MODE, UserLogStorageLimitMode );
    if ( FAILED( Result ) )
    {        
		SysInfoClose(Handle);
        return Result;
    }
    printf( "log server, GetUserLogStorageLimitMode, Default_UserLogStorageLimitMode=%d, UserLogStorageLimitMode=%d \n", GMI_LOG_DEFAULT_USER_LOG_STORAGE_LIMIT_MODE, *UserLogStorageLimitMode );

    printf( "log server, GetUserLogStorageLimitParameter, Default_UserLogStorageLimitParameter=%d \n", GMI_LOG_DEFAULT_USER_LOG_STORAGE_LIMIT_PARAMETER );
    Result = SysInfoRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_USER_LOG_STORAGE_LIMET_PARAMETER, GMI_LOG_DEFAULT_USER_LOG_STORAGE_LIMIT_PARAMETER, UserLogStorageLimitParameter);
    if ( FAILED( Result ) )
    {        
		SysInfoClose(Handle);
        return Result;
    }
    printf( "log server, GetUserLogStorageLimitParameter, Default_UserLogStorageLimitParameter=%d, UserLogStorageLimitParameter=%d \n", GMI_LOG_DEFAULT_USER_LOG_STORAGE_LIMIT_PARAMETER, *UserLogStorageLimitParameter );

    printf( "log server, GetUserUserLogShareMemorySize, Default_UserLogShareMemorySize=%d \n", GMI_LOG_DEFAULT_USER_LOG_SHARE_MEMORY_SIZE );
    Result = SysInfoRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_USER_LOG_SHARE_MEMORY_SIZE, GMI_LOG_DEFAULT_USER_LOG_SHARE_MEMORY_SIZE, UserLogShareMemorySize );
    if ( FAILED( Result ) )
    {
		SysInfoClose(Handle);
        return Result;
    }
    printf( "log server, GetUserUserLogShareMemorySize, Default_UserLogShareMemorySize=%d, UserLogShareMemorySize=%d \n", GMI_LOG_DEFAULT_USER_LOG_SHARE_MEMORY_SIZE, *UserLogShareMemorySize );
   	Result = SysInfoClose(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }
#elif defined( _WIN32 )
    strcpy_s( UserLogFilePath, MAX_PATH_LENGTH, "gmi_user.log" );
    *UserLogStorageLimitMode      = GMI_LOG_DEFAULT_USER_LOG_STORAGE_LIMIT_MODE;
    *UserLogStorageLimitParameter = GMI_LOG_DEFAULT_USER_LOG_STORAGE_LIMIT_PARAMETER;
    *UserLogShareMemoryKey        = GMI_USER_LOG_SHARE_MEMORY_KEY;
    *UserLogShareMemorySize       = GMI_LOG_DEFAULT_USER_LOG_SHARE_MEMORY_SIZE;
    *UserLogIpcMutexKey           = GMI_USER_LOG_IPC_MUTEX_KEY;
#endif

    return GMI_SUCCESS;
}

GMI_RESULT GetLogRepositoryDebugLogParameter( char_t *DebugLogFilePath, int32_t *DebugLogStorageLimitMode, int32_t *DebugLogStorageLimitParameter, long_t *DebugLogShareMemoryKey, int32_t *DebugLogShareMemorySize, long_t *DebugLogIpcMutexKey )
{
#if defined( __linux__ )
    strcpy( DebugLogFilePath, "/opt/log/gmi_debug.log" );
    *DebugLogStorageLimitMode      = 0;
    *DebugLogStorageLimitParameter = 0;
    *DebugLogShareMemoryKey        = 0;
    *DebugLogShareMemorySize       = 0;
    *DebugLogIpcMutexKey           = 0;
#elif defined( _WIN32 )
    strcpy_s( DebugLogFilePath, MAX_PATH_LENGTH, "gmi_debug.log" );
    *DebugLogStorageLimitMode      = 0;
    *DebugLogStorageLimitParameter = 0;
    *DebugLogShareMemoryKey        = 0;
    *DebugLogShareMemorySize       = 0;
    *DebugLogIpcMutexKey           = 0;
#endif

    return GMI_SUCCESS;
}

#define LOG_SERVER_CONFIG_PUBLISH_SERVER_ADDRESS  "publish_server_address"
#define LOG_SERVER_CONFIG_PUBLISH_SERVER_PORT     "publish_server_port"

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
    GMI_RESULT Result = SysInfoOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "GetLogPublishServerPort, Default_Port=%d \n", GMI_LOG_PUBLISH_SERVER_PORT );
    Result = SysInfoRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_PUBLISH_SERVER_PORT, GMI_LOG_PUBLISH_SERVER_PORT, &ServerPort );
    if ( FAILED( Result ) )
    {
		SysInfoClose(Handle);
        return Result;
    }
    printf( "GetLogPublishServerPort, Default_Port=%d, ServerPort=%d \n", GMI_LOG_PUBLISH_SERVER_PORT, ServerPort );
    *Port = htons((uint16_t)ServerPort);

	Result = SysInfoClose(Handle);
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

#define LOG_SERVER_CONFIG_SERVER_UDP_PORT         "server_udp_port"
#define LOG_SERVER_CONFIG_DEBUG_LOG_LEVEL         "debug_log_level"

GMI_RESULT GetLogServerConfig( uint16_t *ServerPort, long_t *ShareMemoryKey, size_t *ShareMemorySize, long_t *IpcMutexKey, uint32_t *DebugLogLevel )
{
    int32_t TempServerPort = 0;
#if defined( __linux__ )

    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = SysInfoOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "log server, GetLogServerUDPPort, Default_UDP_Port=%d \n", LOG_SERVER_DEFAULT_SERVER_PORT );
    Result = SysInfoRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_SERVER_UDP_PORT, LOG_SERVER_DEFAULT_SERVER_PORT, (int32_t *) &TempServerPort );
    if ( FAILED( Result ) )
    {       
		SysInfoClose(Handle);
        return Result;
    }
    printf( "log server, GetLogServerUDPPort, Default_UDP_Port=%d, UDP_Port=%d \n", LOG_SERVER_DEFAULT_SERVER_PORT, TempServerPort );

    printf( "log server, GetLogServerShareMemoryKey, DefaultShareMemoryKey=%d \n", GMI_LOG_SERVER_DEFAUL_SHARE_MEMORY_KEY );
    Result = SysInfoRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_SHARE_MEMORY_KEY, GMI_LOG_SERVER_DEFAUL_SHARE_MEMORY_KEY, (int32_t *) ShareMemoryKey );
    if ( FAILED( Result ) )
    {    
		SysInfoClose(Handle);
        return Result;
    }
    printf( "log server, GetLogServerShareMemoryKey, DefaultShareMemoryKey=%d, ShareMemoryKey=%ld \n", GMI_LOG_SERVER_DEFAUL_SHARE_MEMORY_KEY, *ShareMemoryKey );

    printf( "log server, GetLogServerIpcMutexKey, DefaultIpcMutexKey=%d \n", GMI_LOG_SERVER_DEFAULT_SHARE_MEMORY_IPC_MUTEX_KEY );
    Result = SysInfoRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_SHARE_MEMORY_MUTEX_KEY, GMI_LOG_SERVER_DEFAULT_SHARE_MEMORY_IPC_MUTEX_KEY, (int32_t *) IpcMutexKey);
    if ( FAILED( Result ) )
    {  
		SysInfoClose(Handle);
        return Result;
    }
    printf( "log server, GetLogServerIpcMutexKey, DefaultIpcMutexKey=%d, IpcMutexKey=%ld \n", GMI_LOG_SERVER_DEFAULT_SHARE_MEMORY_IPC_MUTEX_KEY, *IpcMutexKey );

    Result = SysInfoClose(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    Result = SysInfoOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {    
        return Result;
    }

    printf( "log server, GetLogServerShareMemorySize, DefaultShareMemorySize=%d \n", GMI_LOG_DEFAULT_SHARE_MEMORY_SIZE );
    Result = SysInfoRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_SHARE_MEMORY_SIZE, GMI_LOG_DEFAULT_SHARE_MEMORY_SIZE, (int32_t *) ShareMemorySize );
    if ( FAILED( Result ) )
    {  
		SysInfoClose(Handle);
        return Result;
    }
    printf( "log server, GetLogServerShareMemorySize, DefaultShareMemorySize=%d, ShareMemorySize=%d \n", GMI_LOG_DEFAULT_SHARE_MEMORY_SIZE, *ShareMemorySize );

    printf( "log server, GetLogServerDebugLevel, DefaultDebugLogLevel=%d \n", GMI_LOG_MODULE_LOG_DEBUG_LOG_LEVEL );
    Result = SysInfoRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_DEBUG_LOG_LEVEL, GMI_LOG_MODULE_LOG_DEBUG_LOG_LEVEL, (int32_t *) DebugLogLevel );
    if ( FAILED( Result ) )
    {    
		SysInfoClose(Handle);
        return Result;
    }
    printf( "log server, GetLogServerDebugLevel, DefaultDebugLogLevel=%d, DebugLogLevel=%d \n", GMI_LOG_MODULE_LOG_DEBUG_LOG_LEVEL, *DebugLogLevel );

    Result = SysInfoClose(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

#elif defined( _WIN32 )
    TempServerPort    = LOG_SERVER_DEFAULT_SERVER_PORT;
    *ShareMemoryKey   = GMI_LOG_SERVER_DEFAUL_SHARE_MEMORY_KEY;
    *IpcMutexKey      = GMI_LOG_SERVER_DEFAULT_SHARE_MEMORY_IPC_MUTEX_KEY;

    *ShareMemorySize  = GMI_LOG_DEFAULT_SHARE_MEMORY_SIZE;
    *DebugLogLevel    = GMI_LOG_MODULE_LOG_DEBUG_LOG_LEVEL;
#endif
    *ServerPort       = (uint16_t) TempServerPort;

    return GMI_SUCCESS;
}

#define LOG_SERVER_CONFIG_HEARTBEAT_INTERVAL      "heartbeat_interval"

GMI_RESULT GetHeartbeatInterval( uint32_t *Interval )
{
#if defined( __linux__ )

    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = SysInfoOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "log server, GetHeartbeatInterval, Default_Interval=%d \n", GMI_LOG_SERVER_HEARTBEAT_INTERVAL );
    Result = SysInfoRead(Handle, LOG_SERVER_CONFIG_PATH, LOG_SERVER_CONFIG_HEARTBEAT_INTERVAL, GMI_LOG_SERVER_HEARTBEAT_INTERVAL, (int32_t *) Interval );
    if ( FAILED( Result ) )
    {
		SysInfoClose(Handle);
        return Result;
    }
    printf( "log server, GetHeartbeatInterval, Default_Interval=%d, Interval=%d \n", GMI_LOG_SERVER_HEARTBEAT_INTERVAL, *Interval );

	Result = SysInfoClose(Handle);
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
