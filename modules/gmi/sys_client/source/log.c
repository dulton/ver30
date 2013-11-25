#include "gmi_system_headers.h"
#include "ipc_fw_v3.x_resource.h"
#include "log.h"

#ifdef DEBUG_SYS_CLIENT
int        g_SysClientLogCount = 0;
FILE      *g_SysClientFLogFile1 = NULL;
time_t     g_SysClientNow;
struct tm *g_SysClientTime;
#endif

int32_t SysClientLogInitial()
{
#ifdef DEBUG_SYS_CLIENT
    g_SysClientLogCount = 0;
    g_SysClientFLogFile1 = fopen(LOG_FILE1, "w");
#endif
    return 0;
}


int32_t SysClientLogUninitial()
{
#ifdef DEBUG_SYS_CLIENT
    g_SysClientLogCount = 0;
    if (NULL != g_SysClientFLogFile1)
    {
        fclose(g_SysClientFLogFile1);
    }
#endif
    return 0;
}


