#include "alarm_input.h"

#if defined( __linux__ )
#include "gmi_brdwrapperdef.h"
#include "gmi_brdwrapper.h"
#endif//__linux__

AlarmInput::AlarmInput( enum EventDetectorType Type, uint32_t EventDetectorId, uint32_t Index )
    : EventDetector( Type, EventDetectorId, Index )
    , m_InputNumber( 0 )
    , m_Name()
    , m_CheckTime( 1000 )
    , m_ScheduleTimes()
    , m_DetectThread()
    , m_ThreadWorking( false )
    , m_ThreadExitFlag( false )
    , m_GPIOInputStatus( e_AlarmInputStatus_Closed )
{
}

AlarmInput::~AlarmInput(void)
{
}

GMI_RESULT  AlarmInput::SetName( const char_t *Name )
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

GMI_RESULT  AlarmInput::AddScheduleTime( const ScheduleTimeInfo *Schedule )
{
    m_ScheduleTimes.push_back( *Schedule );
    return GMI_SUCCESS;
}

GMI_RESULT  AlarmInput::ListScheduleTime( uint32_t *ItemNumber, ScheduleTimeInfo *Schedule )
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

GMI_RESULT  AlarmInput::Start( const void_t *Parameter, size_t ParameterLength )
{
    GMI_RESULT Result = EventDetector::Start( Parameter, ParameterLength );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    const struct AlarmInputInfo *Info = (const struct AlarmInputInfo *) Parameter;
    SetInputNumber( Info->s_InputNumber );
    SetName( Info->s_Name );
    SetCheckTime( Info->s_CheckTime );

    m_GPIOInputStatus = ( e_AlarmInputStatus_Opened == Info->s_NormalStatus ) ? e_AlarmInputStatus_Opened : e_AlarmInputStatus_Closed;

    m_ThreadWorking  = false;
    m_ThreadExitFlag = false;

    Result = m_DetectThread.Create( NULL, 0, AlarmInputDetectThread, this );
    if ( FAILED( Result ) )
    {
        EventDetector::Stop();
        return Result;
    }

    Result = m_DetectThread.Start();
    if ( FAILED( Result ) )
    {
        m_DetectThread.Destroy();
        EventDetector::Stop();
        return Result;
    }

    return Result;
}

GMI_RESULT  AlarmInput::Stop()
{
    m_ThreadExitFlag = true;
    while ( m_ThreadWorking ) GMI_Sleep( 10 );
    m_DetectThread.Destroy();

    return EventDetector::Stop();
}

void_t* AlarmInput::AlarmInputDetectThread( void_t *Argument )
{
    AlarmInput *Detecter = reinterpret_cast<AlarmInput*> ( Argument );
    void_t *Return = Detecter->DetectEntry();
    return Return;
}

void_t* AlarmInput::DetectEntry()
{
    GMI_RESULT Result = GMI_FAIL;
    m_ThreadWorking   = true;

    uint8_t GPIOStatus = 0;
	uint8_t LastGPIOStatus = 0;
	time_t			   CurrTime;
	struct tm		   CurrTm;
	uint32_t           Curhm;
	int32_t 		   CurrDay;
	int32_t            IsFirstExcute = 1;

    while( !m_ThreadExitFlag )
    {
		CurrTime = time(NULL);
		CurrTm   = *localtime(&CurrTime);
		CurrDay  = CurrTm.tm_wday;
		Curhm    = (CurrTm.tm_hour * 60) + CurrTm.tm_min;	
		if(((Curhm < g_CurStartedAlarmIn[GetInputNumber()].s_ScheduleTime[CurrDay].s_StartTime)
			|| (Curhm > g_CurStartedAlarmIn[GetInputNumber()].s_ScheduleTime[CurrDay].s_EndTime)))
		{
			fprintf(stderr, "Alarm input is not in the ScheduleTime\n");
			GMI_Sleep(2000);
			continue;
		}

		if((GetCheckTime() != g_CurStartedAlarmIn[GetInputNumber()].s_CheckTime)
			&& (g_CurStartedAlarmIn[GetInputNumber()].s_CheckTime >= 200))
		{
			SetCheckTime(g_CurStartedAlarmIn[GetInputNumber()].s_CheckTime);
		}
		
        Result = GMI_BrdGetAlarmInput( GMI_ALARM_MODE_GPIO, 0, &GPIOStatus );
		if(m_GPIOInputStatus != g_CurStartedAlarmIn[GetInputNumber()].s_NormalStatus)
		{
			m_GPIOInputStatus = g_CurStartedAlarmIn[GetInputNumber()].s_NormalStatus;
		}
		
		if(1 == IsFirstExcute)
		{
			LastGPIOStatus = m_GPIOInputStatus;
			IsFirstExcute = 0;
		}

        if ( GPIOStatus != LastGPIOStatus )
        {
            LastGPIOStatus = GPIOStatus;
			if(LastGPIOStatus != m_GPIOInputStatus)
			{
            	m_ProcessCenter->Notify( GetId(), GetInputNumber(), e_EventType_Start, 0, 0 );
			}
			else
			{
				m_ProcessCenter->Notify( GetId(), GetInputNumber(), e_EventType_End, 0, 0 );
			}
		}

        GMI_Sleep( GetCheckTime() );
    }
    m_ThreadWorking   = false;
    return (void_t *) size_t(Result);
}
