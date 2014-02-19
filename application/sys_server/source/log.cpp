#include "gmi_system_headers.h"
#include "ipc_fw_v3.x_resource.h"
#include "ipc_fw_v3.x_setting.h"
#include "log.h"
#include "process_utils_headers.h"
#include "share_memory_log_client.h"
#include "sys_info_readonly.h"


#define SYSTEM_SERVER_CONFIG_PATH                "/Config/sys_server/"
#define SYSTEM_SERVER_CONFIG_LOG_SERVER_PORT     "log_server_port"
#define SYSTEM_SERVER_CONFIG_LOG_CLIENT_PORT     "log_client_port"
#define SYSTEM_SERVER_CONFIG_DEBUG_LOG_LEVEL     "debug_log_level"

#ifdef DEBUG_SYS_SERVER
int        g_LogCount = 0;
int        g_FLogFile1 = 0;
int        g_FLogFile2;
int        g_LogCount2;
time_t     Now;
struct tm *pTime;
char_t     g_Buffer[1024];
#endif

ShareMemoryLogClient g_Client;

GMI_RESULT GetSysServerLogConfig( uint32_t *ModuleId, char_t *ModuleName, uint16_t *ServerPort, uint16_t *ClientPort, uint32_t *DebugLogLevel )
{
    *ModuleId = GMI_LOG_MODULE_CONTROL_CENTER_SERVER_ID;
    strcpy(ModuleName, GMI_LOG_MODULE_CONTROL_CENTER_SERVER_NAME );

    FD_HANDLE  Handle = NULL;    
    GMI_RESULT Result = SysInfoOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if (FAILED(Result))
    {
    	SYS_ERROR("Result = 0x%lx\n", Result);
        return Result;
    }
    
    int32_t TempServerPort = 0;
    Result = SysInfoRead(Handle, SYSTEM_SERVER_CONFIG_PATH, SYSTEM_SERVER_CONFIG_LOG_SERVER_PORT, LOG_SERVER_DEFAULT_SERVER_PORT, &TempServerPort);
    if (FAILED(Result))
    {
    	SYS_ERROR("Result = 0x%lx\n", Result);
        SysInfoClose(Handle);
        return Result;
    }    
    *ServerPort = (uint16_t)TempServerPort;
    
    int32_t TempClientPort = 0;    
    Result = SysInfoRead(Handle, SYSTEM_SERVER_CONFIG_PATH, SYSTEM_SERVER_CONFIG_LOG_CLIENT_PORT, LOG_CONTROL_CENTER_DEFAULT_PORT, &TempClientPort);
    if (FAILED(Result))
    {
    	SYS_ERROR("Result = 0x%lx\n", Result);
        SysInfoClose(Handle);
        return Result;
    }
    *ClientPort = (uint16_t)TempClientPort;   
    SysInfoClose(Handle);

    Result = SysInfoOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if (FAILED(Result))
    {
    	SYS_ERROR("Result = 0x%lx\n", Result);
        return Result;
    }
    
    Result = SysInfoRead(Handle, SYSTEM_SERVER_CONFIG_PATH, SYSTEM_SERVER_CONFIG_DEBUG_LOG_LEVEL, GMI_LOG_MODULE_CONTROL_CENTER_SERVER_DEBUG_LOG_LEVEL, (int32_t *) DebugLogLevel);
    if (FAILED(Result))
    {
    	SYS_ERROR("Result = 0x%lx\n", Result);
        SysInfoClose(Handle);
        return Result;
    }
    
    SysInfoClose(Handle);
    
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
//#else

    uint32_t ModuleId = 0;
    char_t   ModuleName[MAX_PATH_LENGTH] = {0};
    uint16_t ServerPort = 0;
    uint16_t ClientPort = 0;
    uint32_t ServerDebugLogLevel = 0;

    GMI_RESULT Result = GetSysServerLogConfig(&ModuleId, ModuleName, &ServerPort, &ClientPort, &ServerDebugLogLevel);
    if (FAILED(Result))
    {
        SYS_ERROR("sys server get log client config error, Result = 0x%lx\n", Result);
        return Result;
    }

    //debug log config info
    SYS_INFO("DebugLog Config ModuleId   %u\n", ModuleId);
    SYS_INFO("DebugLog Config ModuleName %s\n", ModuleName);
    SYS_INFO("DebugLog Config ServerPort %d\n", ServerPort);
    SYS_INFO("DebugLog Config ClientPort %d\n", ClientPort);
    SYS_INFO("DebugLog Config ServerDebugLogLevel %u\n", ServerDebugLogLevel);
    
    Result = g_Client.Initialize(ModuleId, ModuleName, ServerPort, ClientPort, GMI_IPC_LOG_FILE_PATH, ServerDebugLogLevel);
    if (FAILED(Result))
    {
        SYS_ERROR("sys server log client initialization error \n");
        return Result;
    }

    g_DefaultShareMemoryLogClient  = &g_Client;

    DEBUG_LOG(g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, " LogInitial successfully... \n" );
#endif
    return 0;
}


