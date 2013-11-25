#ifndef __LOG_H__
#define __LOG_H__
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#ifdef __cpluscplus
extern "C"
{
#endif
    int32_t SysClientLogInitial();
    int32_t SysClientLogUninitial();

//debug switch
#define DEBUG_SYS_CLIENT
#ifdef DEBUG_SYS_CLIENT
    extern int        g_SysClientLogCount;
    extern FILE      *g_SysClientFLogFile1;
    extern time_t     g_SysClientNow;
    extern struct tm *g_SysClientTime;
#define LOG_FILE1     "/opt/log/sys_client_log"
#define LOG_FILE1_OLD "/opt/log/sys_client_old"
#define SYS_CLIENT_ERROR(format, args...)\
do {\
	time(&g_SysClientNow);\
	g_SysClientTime = localtime(&g_SysClientNow);\
	if (g_SysClientFLogFile1 != NULL)\
		{\
			g_SysClientLogCount++;\
			if (g_SysClientLogCount < 1000)\
				{\
					fprintf(g_SysClientFLogFile1, "[SYS_CLIENT]!!! [%02d-%02d %02d:%02d:%02d][%s: %d] " format "\n", g_SysClientTime->tm_mon+1, g_SysClientTime->tm_mday, g_SysClientTime->tm_hour, g_SysClientTime->tm_min, g_SysClientTime->tm_sec,__FILE__, __LINE__, ##args);\
				}\
				else\
					{\
						g_SysClientLogCount = 0;\
						fseek(g_SysClientFLogFile1, 0, SEEK_SET);\
					}\
				}\
			} while (0)

#define SYS_CLIENT_INFO(format, args...)\
			do {\
	time(&g_SysClientNow);\
	g_SysClientTime = localtime(&g_SysClientNow);\
	if (g_SysClientFLogFile1 != NULL)\
		{\
			g_SysClientLogCount++;\
			if (g_SysClientLogCount < 1000)\
				{\
					fprintf(g_SysClientFLogFile1, "[SYS_CLIENT]::: [%02d-%02d %02d:%02d:%02d]" format "\n", g_SysClientTime->tm_mon+1, g_SysClientTime->tm_mday, g_SysClientTime->tm_hour, g_SysClientTime->tm_min, g_SysClientTime->tm_sec, ##args);\
				}\
				else\
					{\
						g_SysClientLogCount = 0;\
						fseek(g_SysClientFLogFile1, 0, SEEK_SET);\
					}\
				}\
			} while (0)
#else
#define SYS_CLIENT_ERROR(format, args...)
#define SYS_CLIENT_INFO(format, args...)
#endif


#ifdef __cpluscplus
}
#endif
#endif

