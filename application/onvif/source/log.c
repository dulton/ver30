#include "gmi_system_headers.h"
#include "ipc_fw_v3.x_setting.h"
#include "ipc_fw_v3.x_resource.h"
#include "log.h"
#include "share_memory_log_client.h"

#ifdef DEBUG_ONVIF
int        g_LogCount = 0;
int        g_FLogFile1 = 0;
time_t     Now;
struct tm *pTime;
char_t     g_Buffer[1024];
#endif

// LogClient  g_Client;

int LogInitial()
{
#ifdef DEBUG_ONVIF
    char szCmdBuffer[255] = {"0"};

    g_LogCount = 0;
    memset(szCmdBuffer, 0, 255);
    snprintf(szCmdBuffer, 255, "cp %s %s", LOG_FILE1, LOG_FILE1_OLD);
    system(szCmdBuffer);

    g_FLogFile1 = open(LOG_FILE1, O_CREAT|O_RDWR);
#else

    uint32_t ModuleId = 0;
    char_t   ModuleName[MAX_PATH_LENGTH] = {0};
    char_t   ModulePipeName[MAX_PATH_LENGTH] = {0};
    long_t   ModulePipeMutexId = 0;
    char_t   PeerPipeName[MAX_PATH_LENGTH] = {0};
    long_t   PeerPipeMutexId = 0;
    char_t   ServerPipeName[MAX_PATH_LENGTH] = {0};
    long_t   ServerPipeMutexId = 0;
	
	ONVIF_INFO("GetOnvifServerLogConfig start\n");
    GMI_RESULT Result = GetOnvifServerLogConfig(&ModuleId, ModuleName, ModulePipeName, &ModulePipeMutexId, PeerPipeName, &PeerPipeMutexId, ServerPipeName, &ServerPipeMutexId );
    if (FAILED(Result))
    {
        ONVIF_ERROR("sys server get log client config error \n");
        return Result;
    }
	ONVIF_INFO("GetOnvifServerLogConfig stop\n");

	ONVIF_INFO("g_Client.Initialize start\n");
    Result = g_Client.Initialize(ModuleId, ModuleName, ModulePipeName, ModulePipeMutexId, PeerPipeName, PeerPipeMutexId, ServerPipeName, ServerPipeMutexId );
    if (FAILED(Result))
    {
        ONVIF_ERROR("sys server log client initialization error \n");
        return Result;
    }
	ONVIF_INFO("g_Client.Initialize end\n");

    //g_DefaultLogClient = &g_Client;
#endif
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, " LogInitial successfully... \n" );
    return 0;
}



