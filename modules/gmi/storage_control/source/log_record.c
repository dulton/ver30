#include "log_record.h"
#include "gmi_system_headers.h"
#include "pthread.h"
#include "stdlib.h"
#include "stdarg.h"

/****************version introduce*************************/
//20131113:support read file of capability in libmedia.a
/******************************************************/

#define NAME_FILE_MEDIA_LOG          "/opt/log/storage_control_log"
#define LENGTH_LOG_MAX                500*1024 
LogClient LogClientHd;

void ERR_PRINT_LOG(LogClient *LogClientHd, enum DebugLogLevel PrtLevel, char *FileName, const char *FuncName, int32_t LineNum, const char *fmt, ...)
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

	struct timeval CurTime;
	gettimeofday(&CurTime, 0);

	
	time(&curr);
    p=localtime(&curr);
	memset(strTime, 0, sizeof(strTime));
    TmpLen = sprintf(strTime, "%04d-%02d-%02d %02d:%02d:%02d ms:%ld.%ld",
            (1900+p->tm_year), (1+p->tm_mon), p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec,  CurTime.tv_sec, CurTime.tv_usec/1000);

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


void ERR_PRINT_LOG_TEST(LogClient *LogClientHd, enum DebugLogLevel PrtLevel, char *FileName, const char *FuncName, int32_t LineNum, const char *fmt, ...)
{
	va_list ap;
	time_t curr;
	char strTime[256];
    struct tm *p;
	int32_t TmpLen = 0;
   

	struct timeval CurTime;
	gettimeofday(&CurTime, 0);

	
	time(&curr);
    p=localtime(&curr);
	memset(strTime, 0, sizeof(strTime));
    TmpLen = sprintf(strTime, "%04d-%02d-%02d %02d:%02d:%02d ms:%ld.%ld",
            (1900+p->tm_year), (1+p->tm_mon), p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec,  CurTime.tv_sec, CurTime.tv_usec/1000);

	if((NULL != FileName) && (NULL != FuncName))
	{
		sprintf(strTime+TmpLen, "[%s] [%s] [%d] ", FileName, FuncName, LineNum);
	}
	
	fprintf(stderr, "%s ", strTime);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}

