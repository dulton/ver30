#include "alarm_output.h"

#if defined( __linux__ )
#include "gmi_brdwrapperdef.h"
#include "gmi_brdwrapper.h"
#endif

AlarmOutput::AlarmOutput( uint32_t EventProcessorId, uint32_t Index )
    : EventProcessor( EventProcessorId, Index )
    , m_OutputNumber( 0 )
    , m_Name()
    , m_WorkMode( e_AlarmOutputWorkMode_DelayAutoTrigger )
    , m_DelayTime( 0 )
    , m_ScheduleTimes()
{
}

AlarmOutput::~AlarmOutput(void)
{
}

GMI_RESULT  AlarmOutput::SetName( const char_t *Name )
{
    if ( NULL == Name )
    {
        m_Name = NULL;
    }
    else
    {
        size_t NameLength = strlen( Name );
        m_Name = BaseMemoryManager::Instance().News<char_t>(NameLength+1);
        if ( NULL == m_Name.GetPtr() )
        {
            return GMI_OUT_OF_MEMORY;
        }
        if ( 0 < NameLength )
        {
            memcpy( m_Name.GetPtr(), Name, NameLength );
        }
        (m_Name.GetPtr())[NameLength] = 0;
    }

    return GMI_SUCCESS;
}

GMI_RESULT  AlarmOutput::AddScheduleTime( const ScheduleTimeInfo *Schedule )
{
    m_ScheduleTimes.push_back( *Schedule );
    return GMI_SUCCESS;
}

GMI_RESULT  AlarmOutput::ListScheduleTime( uint32_t *ItemNumber, ScheduleTimeInfo *Schedule )
{
    if ( *ItemNumber < m_ScheduleTimes.size() )
    {
        return GMI_NOT_ENOUGH_SPACE;
    }

    *ItemNumber = (uint32_t) m_ScheduleTimes.size();
    if ( 0 < *ItemNumber )
    {
        for ( uint32_t i = 0; i < *ItemNumber; ++i )
        {
            *Schedule = m_ScheduleTimes[i];
        }
    }
    return GMI_SUCCESS;
}

GMI_RESULT  AlarmOutput::Notify( uint32_t EventId, uint32_t Index, enum EventType Type, void_t *Parameter, size_t ParameterLength )
{
    std::vector<struct DetectorInfo>::iterator DetectorIdIt = m_DetectorIds.begin(), DetectorIdEnd = m_DetectorIds.end();
	GMI_RESULT Result = GMI_SUCCESS;
	int32_t BreakFlag = 0;
	for ( ; DetectorIdIt != DetectorIdEnd ; ++DetectorIdIt )
    {
		if( ((*DetectorIdIt).s_DetectorId == EventId)
				&& ((0 < EventId) && (EventId <= MAX_NUM_EVENT_TYPE)) )
		{
			switch(EventId)
			{
				case EVENT_DETECTOR_ID_ALARM_INPUT:
					if(0 < (g_CurStartedAlaramIn[Index].s_LinkAlarmStrategy & (1<<(EventId-1))))
					{
						Result = GMI_BrdSetAlarmOutput( GMI_ALARM_MODE_GPIO, m_OutputNumber, (e_EventType_Start == Type) ? (uint8_t)e_AlarmInputStatus_Opened : (uint8_t)e_AlarmInputStatus_Closed );
			            if ( FAILED( Result ) )
			            {
			                return Result;
			            }
			            if ( NULL != m_Callback )
			            {
			                m_Callback( m_UserData, EventId, Type, Parameter, ParameterLength );
			            }
					}
					BreakFlag = 1;
					break;
				case EVENT_DETECTOR_ID_HUMAN_DETECT:
					if(0 < (g_CurStartedEvent[EventId-1].s_LinkAlarmStrategy & (1<<(EventId-1))))
			        {
			            GMI_RESULT Result = GMI_BrdSetAlarmOutput( GMI_ALARM_MODE_GPIO, m_OutputNumber, (e_EventType_Start == Type) ? (uint8_t)e_AlarmInputStatus_Opened : (uint8_t)e_AlarmInputStatus_Closed );
			            if ( FAILED( Result ) )
			            {
			                return Result;
			            }
			            if ( NULL != m_Callback )
			            {
			                m_Callback( m_UserData, EventId, Type, Parameter, ParameterLength );
			            }
			        }
					BreakFlag = 1;
					break;
				default:
					fprintf(stderr, "AlarmOutput::Notify EventId %d error.\n", EventId);
					break;
			}
			
		}
		if(1 == BreakFlag)
		{
			break;
		}
    }

    return GMI_SUCCESS;
}

GMI_RESULT  AlarmOutput::Start( const void_t *Parameter, size_t ParameterLength )
{
    const struct AlarmOutputInfo *Info = (const struct AlarmOutputInfo *) Parameter;
    SetOutputNumber( Info->s_OutputNumber );
    SetName( Info->s_Name );
    SetWorkMode( (enum AlarmOutputWorkMode) Info->s_WorkMode );
    SetDelayTime( Info->s_DelayTime );
	#if 0
    for ( uint32_t i = 0; i < Info->s_ScheduleTimeNumber; ++i )
    {
        AddScheduleTime( &(Info->s_ScheduleTime[i]) );
    }
	#endif

    return GMI_SUCCESS;
}

GMI_RESULT  AlarmOutput::Stop()
{
    return GMI_SUCCESS;
}
