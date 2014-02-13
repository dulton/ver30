#ifndef __LOG_H__
#define __LOG_H__
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "log_client.h"

//debug switch
#define DEBUG_ONVIF
#ifdef DEBUG_ONVIF
extern int        g_LogCount;
extern int        g_FLogFile1;
extern boolean_t  g_ErrorEnable;
extern boolean_t  g_InfoEnable;
extern time_t     Now;
extern struct tm *pTime;
extern char_t     g_Buffer[1024];
#define LOG_FILE1     "/opt/log/onvif"
#define LOG_FILE1_OLD "/opt/log/onvif_old"

#define ONVIF_ERROR(format, args...)\
do {\
	if (g_ErrorEnable)\
    {\
	time(&Now);\
	pTime = localtime(&Now);\
	printf("[ONVIF]!!! [%02d-%02d %02d:%02d:%02d][%s: %d] " format, pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec,__FILE__, __LINE__, ##args);\
	if (g_FLogFile1 > 0)\
		{\
			g_LogCount++;\
			if (g_LogCount < 200)\
				{\
					memset(g_Buffer, 0, sizeof(g_Buffer));\
					snprintf(g_Buffer, sizeof(g_Buffer), "[ONVIF]!!! [%02d-%02d %02d:%02d:%02d][%s: %d]" format, pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec,__FILE__, __LINE__, ##args);\
					write(g_FLogFile1, g_Buffer, strlen(g_Buffer));\
				}\
				else\
					{\
						g_LogCount = 0;\
						lseek(g_FLogFile1, 0, SEEK_SET);\
					}\
				}\
			}\
			} while (0)
#define ONVIF_INFO(format, args...)\
	do {\
		if (g_InfoEnable)\
		{\
		time(&Now);\
		pTime = localtime(&Now);\
		printf("[ONVIF]::: [%02d-%02d %02d:%02d:%02d]" format, pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec, ##args);\
		if (g_FLogFile1 > 0)\
			{\
				g_LogCount++;\
				if (g_LogCount < 200)\
					{\
						memset(g_Buffer, 0, sizeof(g_Buffer));\
						snprintf(g_Buffer, sizeof(g_Buffer), "[ONVIF]::: [%02d-%02d %02d:%02d:%02d]" format, pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec, ##args);\
						write(g_FLogFile1, g_Buffer, strlen(g_Buffer));\
					}\
					else\
						{\
							g_LogCount = 0;\
							lseek(g_FLogFile1, 0, SEEK_SET);\
						}\
					}\
					}\
				} while (0)

#else
#define ONVIF_ERROR(format, args...)
#define ONVIF_INFO(format, args...)
#endif

int LogInitial(boolean_t ErrEnable, boolean_t InfoEnable);

#endif

