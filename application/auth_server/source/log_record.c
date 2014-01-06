#include "log_record.h"
#include "gmi_system_headers.h"
#include "pthread.h"
#include "stdlib.h"
#include "stdarg.h"


#ifndef LOG_SERVER_OK
#define NAME_FILE_MEDIA_LOG          "/opt/log/auth_control_log"
#define LENGTH_LOG_MAX                500*1024 

LogClientTmp *LogClientHdTmp;


void ERR_PRINT_LOG(LogClientTmp *LogClientHd, enum DebugLogLevel PrtLevel, const char *FileName, const char *FuncName, int32_t LineNum, const char *fmt, ...)
{
	static pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER;
	FILE *fp = NULL;
	char LogFileName[256];
	int32_t LogFileLen = 0;
	va_list ap;
	time_t curr;

	char strTime[256];
    struct tm *p;
	int32_t TmpLen = 0;
   

	memset(LogFileName, 0, sizeof(LogFileName));
	sprintf(LogFileName, "%s", NAME_FILE_MEDIA_LOG);
	
	time(&curr);
    p=localtime(&curr);
	memset(strTime, 0, sizeof(strTime));
    TmpLen = sprintf(strTime, "%04d-%02d-%02d %02d:%02d:%02d ",
            (1900+p->tm_year), (1+p->tm_mon), p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec);

	if((NULL != FileName) && (NULL != FuncName))
	{
		sprintf(strTime+TmpLen, "[%s] [%s] [%d] ", FileName, FuncName, LineNum);
	}
	
	pthread_mutex_lock(&logMutex);
	fp = fopen(LogFileName, "a+");
	if(NULL == fp)
	{
		pthread_mutex_unlock(&logMutex);
		fprintf(stderr, "WriteMediaLog open log file error\n");
		return;
	}

	fseek(fp, 0L, SEEK_END);
	LogFileLen = ftell(fp);

	if(LogFileLen >= LENGTH_LOG_MAX)
	{
		fclose(fp);
		fp = NULL;
		fp = fopen(LogFileName, "w+");
		if(NULL == fp)
		{
			pthread_mutex_unlock(&logMutex);
			fprintf(stderr, "WriteMediaLog create log file error\n");
			return;
		}
	}

	fprintf(fp, "%s ", strTime);
	va_start(ap, fmt);
	vfprintf(fp, fmt, ap);
	va_end(ap);
	fprintf(fp, "\n");
	fclose(fp);
	fp = NULL;
	pthread_mutex_unlock(&logMutex);
}
#endif
