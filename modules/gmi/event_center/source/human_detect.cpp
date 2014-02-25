#include "human_detect.h"

#if defined( __linux__ )
#include "gmi_brdwrapperdef.h"
#include "gmi_brdwrapper.h"
#endif//__linux__

HumanDetect::HumanDetect( enum EventDetectorType Type, uint32_t EventDetectorId, uint32_t Index )
    : EventDetector( Type, EventDetectorId, Index )
    , m_CheckTime( 1000 )
    , m_ScheduleTimes()
    , m_DetectThread()
    , m_ThreadWorking( false )
    , m_ThreadExitFlag( false )
{
}

HumanDetect::~HumanDetect(void)
{
}

GMI_RESULT  HumanDetect::AddScheduleTime( const ScheduleTimeInfo *Schedule )
{
    m_ScheduleTimes.push_back( *Schedule );
    return GMI_SUCCESS;
}

GMI_RESULT  HumanDetect::ListScheduleTime( uint32_t *ItemNumber, ScheduleTimeInfo *Schedule )
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

GMI_RESULT  HumanDetect::Start( const void_t *Parameter, size_t ParameterLength )
{
    GMI_RESULT Result = EventDetector::Start( Parameter, ParameterLength );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    const struct HumanDetectInfo *Info = (const struct HumanDetectInfo *) Parameter;
    SetCheckTime( Info->s_CheckTime );
	#if 0
    for ( uint32_t i = 0; i < Info->s_ScheduleTimeNumber; ++i )
    {
        AddScheduleTime( &(Info->s_ScheduleTime[i]) );
    }
	#endif

    m_ThreadWorking  = false;
    m_ThreadExitFlag = false;

    Result = m_DetectThread.Create( NULL, 0, HumanDetectThread, this );
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

GMI_RESULT  HumanDetect::Stop()
{
    m_ThreadExitFlag = true;
    while ( m_ThreadWorking ) GMI_Sleep( 10 );
    m_DetectThread.Destroy();

    return EventDetector::Stop();
}

void_t* HumanDetect::HumanDetectThread( void_t *Argument )
{
    HumanDetect *Detecter = reinterpret_cast<HumanDetect*> ( Argument );
    void_t *Return = Detecter->DetectEntry();
    return Return;
}

void_t* HumanDetect::DetectEntry()
{
    GMI_RESULT Result = GMI_FAIL;
    m_ThreadWorking   = true;

    //uint8_t GPIOStatus = 0;
	//int8_t s_GPIOStatus = 0;
	time_t       CurrTime;
	struct timeval     TimeVal;
	struct tm    CurrTm;
	uint32_t     Curhm;
	int32_t      CurrDay;
	int32_t      CurTrigVal = 0;
	AlarmUploadInf     AlarmUploaddata;
	int32_t     i = 0;

    while( !m_ThreadExitFlag )
    {
    	gettimeofday(&TimeVal, NULL);
		CurrTime = TimeVal.tv_sec;
		CurrTm   = *localtime(&CurrTime);
		CurrDay  = CurrTm.tm_wday;
		Curhm    = (CurrTm.tm_hour * 60) + CurrTm.tm_min;
		CurTrigVal = 0xffff;
		for(i=0; i<MAX_SEG_TIME_PERDAY; i++)
		{
			if(((Curhm >= g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_ScheduleTime[CurrDay][i].s_StartTime)
				&& (Curhm <= g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_ScheduleTime[CurrDay][i].s_EndTime)))
			{
				break;
			}
		}
		if(i >= MAX_SEG_TIME_PERDAY)
		{
			fprintf(stderr, "human detect is not in the ScheduleTime\n");
			GMI_Sleep(2000);
			continue;
		}

		if((GetCheckTime() != g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_AlarmEventConfigInfo.s_CheckTime)
			&& (g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_AlarmEventConfigInfo.s_CheckTime >= 200))
		{
			SetCheckTime(g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_AlarmEventConfigInfo.s_CheckTime);
		}

		#if 0
        Result = GMI_BrdGetAlarmInput( 0, 0, &GPIOStatus );

        if ( GPIOStatus != s_GPIOStatus )
        {
            s_GPIOStatus = (enum AlarmInputStatus) GPIOStatus;
            m_ProcessCenter->Notify( GetId(), 0, (e_AlarmInputStatus_Opened == s_GPIOStatus) ? e_EventType_Start : e_EventType_End, NULL, 0 );
        }
		#endif

		Result = GMI_BrdGetAnalogInputValue(GMI_ANALOG_ADC3_PORT, &CurTrigVal);
		if( SUCCEEDED(Result)
			&& (CurTrigVal != 0xffff)
			&& (CurTrigVal > 0)
			&& (g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_AlarmEventConfigInfo.s_ExtData.s_HumanDetectExInfo.s_MinSensVal < g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_AlarmEventConfigInfo.s_ExtData.s_HumanDetectExInfo.s_MaxSensVal)
			&& ((CurTrigVal < (int32_t)g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_AlarmEventConfigInfo.s_ExtData.s_HumanDetectExInfo.s_MinSensVal) || (CurTrigVal > (int32_t)g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_AlarmEventConfigInfo.s_ExtData.s_HumanDetectExInfo.s_MaxSensVal)))
		{
			if(g_AlarmMessageId > 0xEFFFFFFF)
			{
				g_AlarmMessageId = 0;
			}	
			AlarmUploaddata.s_AlarmId = ++g_AlarmMessageId;
			AlarmUploaddata.s_AlarmType = EVENT_DETECTOR_ID_HUMAN_DETECT;
			AlarmUploaddata.s_ExtraInfo.s_IoNum = 0;
			AlarmUploaddata.s_TimeSec = TimeVal.tv_sec;
			AlarmUploaddata.s_TimeUsec = TimeVal.tv_usec;
			strcpy(AlarmUploaddata.s_Description, "human detect appear.");
			AlarmUploaddata.s_OnOff = e_EventType_Start;
            m_ProcessCenter->Notify( GetId(), 0, e_EventType_Start, &AlarmUploaddata, sizeof(AlarmUploaddata) );
		}

        GMI_Sleep( GetCheckTime() );
    }
    m_ThreadWorking   = false;
    return (void_t *) size_t(Result);
}

