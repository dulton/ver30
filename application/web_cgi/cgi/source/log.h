#ifndef __LOG_H__
#define __LOG_H__

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

//debug switch
#define DEBUG_CGI
#ifdef DEBUG_CGI
extern int        g_LogCount;
extern FILE      *g_FLogFile1;
extern time_t     Now;
extern struct tm *pTime;
#define LOG_FILE1     "/opt/log/cgi_log"
#define LOG_FILE1_OLD "/opt/log/cgi_log_old"
#define CGI_ERROR(format, args...)\
do {\
	time(&Now);\
	pTime = localtime(&Now);\
	if (g_FLogFile1 != NULL)\
		{\
			g_LogCount++;\
			if (g_LogCount < 1000)\
				{\
					fprintf(g_FLogFile1, "[GMI_CGI]!!! [%02d-%02d %02d:%02d:%02d][%s: %d] " format "\n", pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec,__FILE__, __LINE__, ##args);\
				}\
				else\
					{\
						g_LogCount = 0;\
						fseek(g_FLogFile1, 0, SEEK_SET);\
					}\
				}\
			} while (0)

#define CGI_INFO(format, args...)\
			do {\
	time(&Now);\
	pTime = localtime(&Now);\
	if (g_FLogFile1 != NULL)\
		{\
			g_LogCount++;\
			if (g_LogCount < 1000)\
				{\
					fprintf(g_FLogFile1, "[GMI_CGI]::: [%02d-%02d %02d:%02d:%02d]" format "\n", pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec, ##args);\
				}\
				else\
					{\
						g_LogCount = 0;\
						fseek(g_FLogFile1, 0, SEEK_SET);\
					}\
				}\
			} while (0)
#else
#define CGI_ERROR(format, args...)
#define CGI_INFO(format, args...)
#endif

int32_t LogInitial();
int32_t LogUninitial();
#endif

