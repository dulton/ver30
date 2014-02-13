
#ifndef __LOG_H__
#define __LOG_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "share_memory_log_client.h"

#define DEBUG_SYS_SERVER
#ifdef DEBUG_SYS_SERVER
extern int        g_LogCount;
extern int        g_FLogFile1;  
extern int        g_FLogFile2;
extern int        g_LogCount2;
extern time_t     Now; 
extern struct tm *pTime;
extern char_t     g_Buffer[1024];
#define LOG_FILE1     "/opt/log/sys_server"
#define LOG_FILE1_OLD "/opt/log/sys_server_old"
#define LOG_FILE2     "/opt/log/sys_server_err"
#define LOG_FILE2_OLD "/opt/log/sys_server_err_old"

#define SYS_ERROR(format, args...)\
do {\
	time(&Now);\
	pTime = localtime(&Now);\
	printf("[SYS_SERVER]!!! [%02d-%02d %02d:%02d:%02d][%s: %d]" format, pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec,__FILE__, __LINE__, ##args);\
	if (g_FLogFile1 > 0)\
		{\
			g_LogCount++;\
			if (g_LogCount < 500)\
				{\
					memset(g_Buffer, 0, sizeof(g_Buffer));\
					snprintf(g_Buffer, sizeof(g_Buffer), "[SYS_SERVER]!!! [%02d-%02d %02d:%02d:%02d][%s: %d]" format, pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec,__FILE__, __LINE__, ##args);\
					write(g_FLogFile1, g_Buffer, strlen(g_Buffer));\
				}\
				else\
					{\
						g_LogCount = 0;\
						lseek(g_FLogFile1, 0, SEEK_SET);\
					}\
				}\
			} while (0)
#define SYS_INFO(format, args...)\
	do {\
		time(&Now);\
		pTime = localtime(&Now);\
		printf("[SYS_SERVER]::: [%02d-%02d %02d:%02d:%02d]" format, pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec, ##args);\
		if (g_FLogFile1 > 0)\
			{\
				g_LogCount++;\
				if (g_LogCount < 500)\
					{\
						memset(g_Buffer, 0, sizeof(g_Buffer));\
						snprintf(g_Buffer, sizeof(g_Buffer), "[SYS_SERVER]::: [%02d-%02d %02d:%02d:%02d]" format, pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec, ##args);\
						write(g_FLogFile1, g_Buffer, strlen(g_Buffer));\
					}\
					else\
						{\
							g_LogCount = 0;\
							lseek(g_FLogFile1, 0, SEEK_SET);\
						}\
					}\
				} while (0)

#else
#define SYS_ERROR(format, args...)
#define SYS_INFO(format, args...)
#endif

int LogInitial();

#if 0
#include "log_header.h"
#include "log_client.h"

#ifdef __cpluscplus
extern "C"
{
#endif

    GMI_RESULT LogClientInit();

    void LogClientUnInit();

    extern LogClient g_SysLogClient;

#ifdef __cpluscplus
}
#endif
#endif
#endif
