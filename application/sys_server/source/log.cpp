#include "gmi_system_headers.h"
#include "process_utils_headers.h"
#include "ipc_fw_v3.x_resource.h"
#include "ipc_fw_v3.x_setting.h"
#include "log.h"
#include "share_memory_log_client.h"


#ifdef DEBUG_SYS_SERVER
int        g_LogCount = 0;
int        g_FLogFile1 = 0;
int        g_FLogFile2;
int        g_LogCount2;
time_t     Now;
struct tm *pTime;
char_t     g_Buffer[1024];
#endif

//LogClient  g_Client;

GMI_RESULT GetMediaCenterServerLogConfig( uint32_t *ModuleId, char_t *ModuleName, char_t *ModulePipeName, long_t *ModulePipeMutexId, char_t *PeerPipeName, long_t *PeerPipeMutexId, char_t *ServerPipeName, long_t *ServerPipeMutexId )
{
    *ModuleId = GMI_LOG_MODULE_CONTROL_CENTER_SERVER_ID;
#if defined( __linux__ )
    strcpy( ModuleName, GMI_LOG_MODULE_CONTROL_CENTER_SERVER_NAME );

#if 0
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "media center server, GetMediaCenterClientPipeName, Default_MediaCenterClientPipeName=%s \n", LOG_MEDIA_CENTER_DEFAULT_CLIENT_PIPE_NAME );
    Result = GMI_XmlRead(Handle, MEDIA_CENTER_SERVER_CONFIG_PATH, MEDIA_CENTER_SERVER_CONFIG_LOG_CLIENT_PIPE_NAME, LOG_MEDIA_CENTER_DEFAULT_CLIENT_PIPE_NAME, ModulePipeName, GMI_CONFIG_READ_ONLY );
    if ( FAILED( Result ) )
    {
        return Result;
    }
    printf( "media center server, GetMediaCenterClientPipeName, Default_MediaCenterClientPipeName=%d, Config_MediaCenterClientPipeName=%d \n", LOG_MEDIA_CENTER_DEFAULT_CLIENT_PIPE_NAME, ModulePipeName );

    printf( "media center server, GetMediaCenterClientPipeMutexId, Default_MediaCenterClientPipeMutexId=%d \n", LOG_MEDIA_CENTER_DEFAULT_CLIENT_PIPE_MUTEX_ID );
    Result = GMI_XmlRead(Handle, MEDIA_CENTER_SERVER_CONFIG_PATH, MEDIA_CENTER_SERVER_CONFIG_LOG_CLIENT_PIPE_MUTEX_ID, LOG_MEDIA_CENTER_DEFAULT_CLIENT_PIPE_MUTEX_ID, (int32_t *) ModulePipeMutexId, GMI_CONFIG_READ_ONLY );
    if ( FAILED( Result ) )
    {
        return Result;
    }
    printf( "media center server, GetMediaCenterClientPipeMutexId, Default_MediaCenterClientPipeMutexId=%d, Config_MediaCenterClientPipeMutexId=%ld \n", LOG_MEDIA_CENTER_DEFAULT_CLIENT_PIPE_MUTEX_ID, ModulePipeMutexId );

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }
#else
    // strcpy( ModulePipeName, LOG_CONTROL_CENTER_DEFAULT_CLIENT_PIPE_NAME );
    // *ModulePipeMutexId = LOG_CONTROL_CENTER_DEFAULT_CLIENT_PIPE_MUTEX_ID;

    // strcpy( PeerPipeName, LOG_CONTROL_CENTER_DEFAULT_PEER_PIPE_NAME );
    // *PeerPipeMutexId = LOG_CONTROL_CENTER_DEFAULT_PEER_PIPE_MUTEX_ID;

    // strcpy( ServerPipeName, LOG_SERVER_DEFAULT_PIPE_NAME);
    // *ServerPipeMutexId = LOG_SERVER_DEFAULT_PIPE_MUTEX_ID;
#endif

#elif defined( _WIN32 )
    strcpy_s( ModuleName,     MAX_PATH_LENGTH, GMI_LOG_MODULE_MEDIA_CENTER_SERVER_NAME );

    strcpy_s( ModulePipeName, MAX_PATH_LENGTH, LOG_MEDIA_CENTER_DEFAULT_CLIENT_PIPE_NAME );
    *ModulePipeMutexId = LOG_MEDIA_CENTER_DEFAULT_CLIENT_PIPE_MUTEX_ID;

    strcpy_s( PeerPipeName, MAX_PATH_LENGTH, LOG_MEDIA_CENTER_DEFAULT_PEER_PIPE_NAME );
    *PeerPipeMutexId = LOG_MEDIA_CENTER_DEFAULT_PEER_PIPE_MUTEX_ID;

    strcpy_s( ServerPipeName, MAX_PATH_LENGTH, LOG_SERVER_DEFAULT_PIPE_NAME );
    *ServerPipeMutexId = LOG_SERVER_DEFAULT_PIPE_MUTEX_ID;
#endif
    return GMI_SUCCESS;
}


int LogInitial()
{
#ifdef DEBUG_SYS_SERVER
    char szCmdBuffer[255] = {"0"};

    g_LogCount = 0;
    memset(szCmdBuffer, 0, 255);
    snprintf(szCmdBuffer, 255, "cp %s %s", LOG_FILE1, LOG_FILE1_OLD);
    system(szCmdBuffer);

    g_FLogFile1 = open(LOG_FILE1, O_CREAT|O_RDWR);

    g_LogCount2 = 0;
    memset(szCmdBuffer, 0, 255);
    snprintf(szCmdBuffer, 255, "cp %s %s", LOG_FILE2, LOG_FILE2_OLD);
    system(szCmdBuffer);

    g_FLogFile2 = open(LOG_FILE2, O_CREAT|O_RDWR);
#else

    uint32_t ModuleId = 0;
    char_t   ModuleName[MAX_PATH_LENGTH] = {0};
    char_t   ModulePipeName[MAX_PATH_LENGTH] = {0};
    long_t   ModulePipeMutexId = 0;
    char_t   PeerPipeName[MAX_PATH_LENGTH] = {0};
    long_t   PeerPipeMutexId = 0;
    char_t   ServerPipeName[MAX_PATH_LENGTH] = {0};
    long_t   ServerPipeMutexId = 0;

    GMI_RESULT Result = GetMediaCenterServerLogConfig(&ModuleId, ModuleName, ModulePipeName, &ModulePipeMutexId, PeerPipeName, &PeerPipeMutexId, ServerPipeName, &ServerPipeMutexId );
    if (FAILED(Result))
    {
        SYS_ERROR("sys server get log client config error \n");
        return Result;
    }

    Result = g_Client.Initialize( ModuleId, ModuleName, ModulePipeName, ModulePipeMutexId, PeerPipeName, PeerPipeMutexId, ServerPipeName, ServerPipeMutexId, GMI_IPC_LOG_FILE_PATH );
    if (FAILED(Result))
    {
        SYS_ERROR("sys server log client initialization error \n");
        return Result;
    }

    g_DefaultLogClient = &g_Client;

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, " LogInitial successfully... \n" );
#endif
    return 0;
}


#if 0
#include "gmi_system_headers.h"
#include "process_utils_headers.h"
#include "ipc_fw_v3.x_resource.h"
#include "log.h"

#define LOG_CLIENT_MODULE_NAME "SYS_SERVER"
LogClient g_SysLogClient;


GMI_RESULT LogClientInit()
{
    char_t ModuleName[64];

    sprintf( ModuleName, "%s", LOG_CLIENT_MODULE_NAME);

    GMI_RESULT Result = g_SysLogClient.Initialize( ModuleName,
                        LOG_CONTROL_CENTER_DEFAULT_CLIENT_PIPE_NAME,
                        LOG_CONTROL_CENTER_DEFAULT_CLIENT_PIPE_MUTEX_ID,
                        LOG_CONTROL_CENTER_DEFAULT_PEER_PIPE_NAME,
                        LOG_CONTROL_CENTER_DEFAULT_PEER_PIPE_MUTEX_ID,
                        DebugLevel_Info,
                        ApplicationLevel_Info,
                        LOG_SERVER_DEFAULT_PIPE_NAME,
                        LOG_SERVER_DEFAULT_PIPE_MUTEX_ID );
    if ( FAILED( Result ) )
    {
        printf( "Log Client initialization error \n" );
    }

    return Result;
}


void LogClientUnInit()
{
    g_SysLogClient.Deinitialize();
    return;
}
#endif

