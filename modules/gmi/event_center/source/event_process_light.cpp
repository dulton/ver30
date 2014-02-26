#include "event_process_light.h"

#if defined( __linux__ )
#include "gmi_brdwrapperdef.h"
#include "gmi_brdwrapper.h"
#endif

EventProcessLight::EventProcessLight( uint32_t EventProcessorId, uint32_t Index )
    : EventProcessor( EventProcessorId, Index )
    , m_DelayTime( 0 )
    , m_TriggedTime( 0 )
    , m_OperationLock()
{
}

EventProcessLight::~EventProcessLight(void)
{
}

GMI_RESULT  EventProcessLight::Notify( uint32_t EventId, uint32_t Index, enum EventType Type, void_t *Parameter, size_t ParameterLength )
{
    std::vector<struct DetectorInfo>::iterator DetectorIdIt = m_DetectorIds.begin(), DetectorIdEnd = m_DetectorIds.end();
	GMI_RESULT Result = GMI_SUCCESS;
	int32_t BreakFlag = 0;
	time_t  CurrTime;
	CurrTime = time(NULL);

	
	m_OperationLock.Lock( TIMEOUT_INFINITE );
	for ( ; DetectorIdIt != DetectorIdEnd ; ++DetectorIdIt )
    {
    	
		if( ((*DetectorIdIt).s_DetectorId == EventId)
				&& ((0 < EventId) && (EventId <= MAX_NUM_EVENT_TYPE)) )
		{
			switch(EventId)
			{
				case EVENT_DETECTOR_ID_HUMAN_DETECT:				
					printf("EventId=%d\n", EventId);
					if(0 < (g_CurStartedEvent[EventId-1].s_AlarmEventConfigInfo.s_LinkAlarmStrategy & (1<<(EVENT_PROCESSOR_ID_LINK_LIGHT-1))))
			        {
			            Result = GMI_BrdSetAlarmOutput( GMI_ALARM_MODE_LIGHT, 0, (e_EventType_Start == Type) ? 1 : 0 );
						if ( FAILED( Result ) )
			            {	            	
							m_OperationLock.Unlock();
			                return Result;
			            }
						printf("GMI_ALARM_MODE_LIGHT open\n");
						if(e_EventType_Start == Type)
						{
							BreakFlag = 1;
						}
						else
						{
							BreakFlag = 2;
						}
			        }
					break;
				default:
					fprintf(stderr, "AlarmOutput::Notify EventId %d error.\n", EventId);
					break;
			}
			
		}
		if(0 <  BreakFlag)
		{
			if(1 == BreakFlag)
			{
				SetTriggedTime(CurrTime);
			}
			else if(2 == BreakFlag)
			{
				SetTriggedTime(0);
			}
			break;
		}
    }
	m_OperationLock.Unlock();

    return GMI_SUCCESS;
}

GMI_RESULT  EventProcessLight::Start( const void_t *Parameter, size_t ParameterLength )
{
    const struct AlarmEventConfigInfo *Info = (const struct AlarmEventConfigInfo *) Parameter;
    SetDelayTime( Info->s_LinkAlarmExtInfo.s_DelayTime);

	GMI_RESULT Result = m_TimerThread.Create( NULL, 0, TimerThread, this );
    if ( FAILED( Result ) )
    {
        EventProcessLight::Stop();
        return Result;
    }

    Result = m_TimerThread.Start();
    if ( FAILED( Result ) )
    {
        m_TimerThread.Destroy();
        EventProcessLight::Stop();
        return Result;
    }

	Result = m_OperationLock.Create( NULL );
    if ( FAILED( Result ) )
    {
        m_TimerThread.Destroy();
        EventProcessLight::Stop();
        return Result;
    }

    return GMI_SUCCESS;
}

GMI_RESULT  EventProcessLight::Stop()
{
	m_ThreadExitFlag = true;
    while ( m_ThreadWorking ) GMI_Sleep( 10 );
    m_TimerThread.Destroy();
	m_OperationLock.Destroy();

    return GMI_SUCCESS;
}

void_t* EventProcessLight::TimerThread( void_t *Argument )
{
    EventProcessLight *TimerOperate = reinterpret_cast<EventProcessLight*> ( Argument );
    void_t *Return = TimerOperate->TimerEntry();
    return Return;
}

void_t* EventProcessLight::TimerEntry()
{
    GMI_RESULT Result = GMI_FAIL;
    m_ThreadWorking   = true;

 	time_t       CurrTime;

    while( !m_ThreadExitFlag )
    {
    	CurrTime = time(NULL);
		if(GetDelayTime() != g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_AlarmEventConfigInfo.s_LinkAlarmExtInfo.s_DelayTime)
		{
			SetDelayTime(g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_AlarmEventConfigInfo.s_LinkAlarmExtInfo.s_DelayTime);
		}
		
		m_OperationLock.Lock( TIMEOUT_INFINITE );
		if((GetTriggedTime() > 0)
			&&(CurrTime > (GetTriggedTime()+GetDelayTime())))
		{	
			printf("GMI_ALARM_MODE_LIGHT close[%u]\n", (uint32_t)CurrTime);
            Result = GMI_BrdSetAlarmOutput( GMI_ALARM_MODE_LIGHT, 0, 0 );
			if ( FAILED( Result ) )
            {	            	
			    fprintf(stderr, "GMI_BrdSetAlarmOutput GMI_ALARM_MODE_LIGHT fail\n");
            }
			SetTriggedTime(0);
		}
		m_OperationLock.Unlock();
        GMI_Sleep( 1000 );
    }
    m_ThreadWorking   = false;
    return (void_t *) size_t(Result);
}


