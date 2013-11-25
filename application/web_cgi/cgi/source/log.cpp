#include "log.h"
#include "gmi_system_headers.h"

#ifdef DEBUG_CGI
int        g_LogCount = 0;
FILE      *g_FLogFile1 = NULL;
time_t     Now;
struct tm *pTime;
#endif

int32_t LogInitial()
{
#ifdef DEBUG_CGI
    g_LogCount = 0;
    g_FLogFile1 = fopen(LOG_FILE1, "w");
#endif
    return 0;
}


int32_t LogUninitial()
{
#ifdef DEBUG_CGI
    g_LogCount = 0;
    if (NULL != g_FLogFile1)
    {
        fclose(g_FLogFile1);
    }
#endif
    return 0;
}