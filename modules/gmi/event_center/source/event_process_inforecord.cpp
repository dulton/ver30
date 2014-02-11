#include "event_process_inforecord.h"

#if defined( __linux__ )
#include "gmi_brdwrapperdef.h"
#include "gmi_brdwrapper.h"
#endif
static const char_t *G_EventDetectorTypeName[] =
{
	"Alarm Input",
	"Human detect",
};

static uint32_t G_EventDetectorTypeNameArraySize = 2;

EventProcessInfoRecord::EventProcessInfoRecord( uint32_t EventProcessorId )
    : EventProcessor( EventProcessorId )
{
}

EventProcessInfoRecord::~EventProcessInfoRecord(void)
{
}

void EventRecodPrt(const char *fmt, ...)
{
	static pthread_mutex_t LogMutex = PTHREAD_MUTEX_INITIALIZER;
	FILE *Fp = NULL;
	char_t LogFileName[256];
	int32_t LogFileLen = 0;
	va_list Valist;
	time_t CurTime;

	char_t StrTime[256];
    struct tm *Tmp;
   

	memset(LogFileName, 0, sizeof(LogFileName));
	sprintf(LogFileName, "%s", FILE_PATH_EVENT_LOG);
	
	time(&CurTime);
    Tmp=localtime(&CurTime);
	memset(StrTime, 0, sizeof(StrTime));
    sprintf(StrTime, "%04d-%02d-%02d %02d:%02d:%02d ",
            (1900+Tmp->tm_year), (1+Tmp->tm_mon), Tmp->tm_mday,
            Tmp->tm_hour, Tmp->tm_min, Tmp->tm_sec);
	
	pthread_mutex_lock(&LogMutex);
	Fp = fopen(LogFileName, "a+");
	if(NULL == Fp)
	{
		pthread_mutex_unlock(&LogMutex);
		fprintf(stderr, "WriteMediaLog open log file error\n");
		return;
	}

	fseek(Fp, 0L, SEEK_END);
	LogFileLen = ftell(Fp);

	if(LogFileLen >= LENGTH_EVENT_LOG_MAX)
	{
		fclose(Fp);
		Fp = NULL;
		Fp = fopen(LogFileName, "w+");
		if(NULL == Fp)
		{
			pthread_mutex_unlock(&LogMutex);
			fprintf(stderr, "WriteMediaLog create log file error\n");
			return;
		}
	}

	fprintf(Fp, "%s ", StrTime);
	va_start(Valist, fmt);
	vfprintf(Fp, fmt, Valist);
	va_end(Valist);
	fprintf(Fp, "\n");
	fclose(Fp);
	Fp = NULL;
	pthread_mutex_unlock(&LogMutex);
}


GMI_RESULT  EventProcessInfoRecord::Notify( uint32_t EventId, enum EventType Type, void_t *Parameter, size_t ParameterLength )
{
    std::vector<uint32_t>::iterator DetectorIdIt = m_DetectorIds.begin(), DetectorIdEnd = m_DetectorIds.end();
    for ( ; DetectorIdIt != DetectorIdEnd ; ++DetectorIdIt )
    {
        if ( *DetectorIdIt == EventId )
        {
        	if((EventId <= G_EventDetectorTypeNameArraySize) && (EventId > 0))
        	{ 		
				EventRecodPrt("[%s] tigger!", G_EventDetectorTypeName[EventId-1]);
        	}
			else
			{
				EventRecodPrt("[%d] tigger!", EventId);
			}
        }
    }
	
    return GMI_SUCCESS;
}

GMI_RESULT  EventProcessInfoRecord::Start( const void_t *Parameter, size_t ParameterLength )
{
    return GMI_SUCCESS;
}

GMI_RESULT  EventProcessInfoRecord::Stop()
{
    return GMI_SUCCESS;
}

