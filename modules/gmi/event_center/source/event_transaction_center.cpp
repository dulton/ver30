#include "event_transaction_center.h"

#include "alarm_input.h"
#include "alarm_output.h"
#include "event_process_inforecord.h"
#include "human_detect.h"

AlarmEventConfigInfo g_CurStartedEvent[MAX_NUM_EVENT_TYPE] = {0};


int32_t CheckCurBitValid(uint32_t BitPos)
{
	int32_t i = 0;
	int32_t IsValid = 0;
	if(BitPos < 1)
	{
		return -1;
	}
	for(i = 0; i<MAX_NUM_EVENT_TYPE; i++)
	{
		if(0 < (g_CurStartedEvent[i].s_LinkAlarmStrategy & (1<<(BitPos-1))))
		{
			IsValid = 1;
			break;
		}
	}
	return IsValid;
}

int32_t CheckCurBitValidByStrategyId(uint32_t EventId, uint32_t StrategyId)
{
	int32_t i = 0;
	int32_t IsValid = 0;
	if((EventId < 0) || (EventId > MAX_NUM_EVENT_TYPE-1))
	{
		fprintf(stderr, "CheckCurBitValidByProcessorId EventId %d error.\n", EventId);
		return -1;
	}
	if(0 < (g_CurStartedEvent[EventId].s_LinkAlarmStrategy & (1<<(StrategyId-1))))
	{
		IsValid = 1;
	}
	return IsValid;
}


size_t EventTransactionCenter::m_IsStartGPIOInput = 0;
size_t EventTransactionCenter::m_IsStartHumanDetect = 0;

size_t EventTransactionCenter::m_IsStartGPIOOutput = 0;
size_t EventTransactionCenter::m_IsStartInfoRecord = 0;






EventTransactionCenter::EventTransactionCenter()
    : m_Center( NULL )
    , m_Callback( NULL )
    , m_UserData( NULL )
{
}

EventTransactionCenter::~EventTransactionCenter(void)
{
}

GMI_RESULT EventTransactionCenter::Start( const void_t *Parameter, size_t ParameterLength, EventCallback Callback, void_t *UserData )
{
	//int32_t IsStartOk = 1;
    m_Callback = Callback;
    m_UserData = UserData;

    m_Center = BaseMemoryManager::Instance().New<EventCenter>();
    if ( NULL == m_Center.GetPtr() )
    {
        printf( "allocating EventCenter object failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    GMI_RESULT Result = m_Center->Initialize( NULL, 0 );
    if ( FAILED( Result ) )
    {
        printf( "EventCenter Initialize failed \n" );
        return Result;
    }

	#if 0
	do
	{
		//start detector
		if(FLAG_EVENT_ENABLE == g_CurStartedEvent[EVENT_DETECTOR_ID_ALARM_INPUT-1].s_EnableFlag)
		{
		    Result = StartGPIOAlarmInput();
		    if ( FAILED( Result ) )
		    {
				IsStartOk = 0;
				break;
		    }
		}

		if(FLAG_EVENT_ENABLE == g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_EnableFlag)
		{
			Result = StartHumanDetect();
			if ( FAILED( Result ) )
			{
				IsStartOk = 0;
				break;
			}
		}


		//start processor
		if(0 < CheckCurBitValid(EVENT_PROCESSOR_ID_ALARM_OUTPUT))
		{
			Result = StartGPIOAlarmOutput();
		    if ( FAILED( Result ) )
		    {
		        IsStartOk = 0;
				break;
		    }
		}
		

		if(0 < CheckCurBitValid(EVENT_PROCESSOR_ID_INFO_RECORD))
		{
			Result = StartAlarmInfoRecord();
			if ( FAILED( Result ) )
			{
				IsStartOk = 0;
				break;
			}
		}
		
	    Result = m_Center->Start();
	    if ( FAILED( Result ) )
	    {
	        IsStartOk = 0;
			break;
	    }
	}while(0);

	if(0 == IsStartOk)
	{
		StopGPIOAlarmInput();
	    StopGPIOAlarmOutput();
		StopHumanDetect();
		StopAlarmInfoRecord();
	    m_Center->Deinitialize();
	    m_Center = NULL;
	}
	#endif
	Result = m_Center->Start();
    if ( FAILED( Result ) )
    {
    	fprintf(stderr, "m_Center start error.\n");
        m_Center->Deinitialize();
	    m_Center = NULL;
    }

    return Result;
}

GMI_RESULT EventTransactionCenter::Stop()
{
    GMI_RESULT Result = m_Center->Stop();
    if ( FAILED( Result ) )
    {
        return Result;
    }

    Result = StopGPIOAlarmInput();
    if ( FAILED( Result ) )
    {
        return Result;
    }

    Result = StopGPIOAlarmOutput();
    if ( FAILED( Result ) )
    {
        return Result;
    }
		
    Result = StopHumanDetect();
    if ( FAILED( Result ) )
    {
        return Result;
    }

	Result = StopAlarmInfoRecord();
    if ( FAILED( Result ) )
    {
        return Result;
    }

    Result = m_Center->Deinitialize();

    if ( FAILED( Result ) )
    {
        return Result;
    }

    return Result;
}

GMI_RESULT EventTransactionCenter::ConfigureGPIOAlarmInput( const void_t *Parameter, size_t ParameterLength )
{
    return GMI_NOT_IMPLEMENT;
}

GMI_RESULT EventTransactionCenter::ConfigureGPIOAlarmOutput( const void_t *Parameter, size_t ParameterLength )
{
    return GMI_NOT_IMPLEMENT;
}


GMI_RESULT EventTransactionCenter::ConfigureAlarmEvent(const enum AlarmEventType EventType, const void *Parameter, size_t ParamterLength)
{
	GMI_RESULT Result = GMI_SUCCESS;

	do
	{
		if((NULL == Parameter)
			|| (0 >= ParamterLength)
			|| ((1 > EventType) && (MAX_NUM_EVENT_TYPE < EventType)))
		{
			fprintf(stderr, "ConfigureAlarmEvent GMI_INVALID_PARAMETER.\n");
			Result = GMI_INVALID_PARAMETER;
			break;
		}
		memcpy(&(g_CurStartedEvent[EventType-1]), (AlarmEventConfigInfo *)Parameter, sizeof(AlarmEventConfigInfo));
		if(FLAG_EVENT_ENABLE == g_CurStartedEvent[EventType-1].s_EnableFlag)
		{
			switch(EventType)
			{
				case e_AlarmEventType_AlarmInput:	
					Result = StartGPIOAlarmInput();
					break;
				case e_AlarmEventType_HumanDetect:
					Result = StartHumanDetect();
					break;
				default:
					fprintf(stderr, "start event:EventType %d no support.\n", EventType);
					break;
			}
		}
		else
		{
			switch(EventType)
			{
				case e_AlarmEventType_AlarmInput:
					Result = StopGPIOAlarmInput();
					break;
				case e_AlarmEventType_HumanDetect:
					Result = StopHumanDetect();
					break;
				default:
					fprintf(stderr, "stop event:EventType %d no support.\n", EventType);
					break;
			}
		}

		//start processor
		if(0 < CheckCurBitValid(EVENT_PROCESSOR_ID_ALARM_OUTPUT))
		{
			Result = StartGPIOAlarmOutput();
		    if ( FAILED( Result ) )
		    {
				fprintf(stderr, "ConfigureAlarmEvent StartGPIOAlarmOutput error.\n");
				break;
		    }
		}
		else
		{
			Result = StopGPIOAlarmOutput();
		    if ( FAILED( Result ) )
		    {
				fprintf(stderr, "ConfigureAlarmEvent StopGPIOAlarmOutput error.\n");
				break;
		    }
		}
		

		if(0 < CheckCurBitValid(EVENT_PROCESSOR_ID_INFO_RECORD))
		{
			Result = StartAlarmInfoRecord();
			if ( FAILED( Result ) )
			{
				fprintf(stderr, "ConfigureAlarmEvent StartAlarmInfoRecord error.\n");
				break;
			}
		}
		else
		{
			Result = StopAlarmInfoRecord();
			if ( FAILED( Result ) )
			{
				fprintf(stderr, "ConfigureAlarmEvent StopAlarmInfoRecord error.\n");
				break;
			}
		}
		
	}while(0);

    return Result;
}

GMI_RESULT EventTransactionCenter::StartGPIOAlarmInput()
{
	if(1 == m_IsStartGPIOInput)
	{
		fprintf(stderr, "StartGPIOAlarmInput has done.\n");
		return GMI_SUCCESS;
	}
    ReferrencePtr<AlarmInput> AlarmInputDetector( BaseMemoryManager::Instance().New<AlarmInput>( e_EventDetectorType_Passive, EVENT_DETECTOR_ID_ALARM_INPUT ) );
    if ( NULL == AlarmInputDetector.GetPtr() )
    {
        m_Center->Deinitialize();
        printf( "allocating AlarmInputDetector object failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    struct AlarmInputInfo Info;
    Info.s_InputNumber = 0;
#if defined( __linux__ )
    strcpy( Info.s_Name, "AlarmOutput" );
#elif defined( _WIN32 )
    strcpy_s( Info.s_Name, "AlarmOutput" );
#endif
    Info.s_CheckTime = 100;
    Info.s_TriggerType = e_AlarmInputTriggerType_UsuallyClosed;
    //Info.s_ScheduleTimeNumber = 1;
    //Info.s_ScheduleTime[0].s_StartTime = 0x106000000000000;//Monday, AM 06:00:00
    //Info.s_ScheduleTime[0].s_EndTime   = 0x112000000000000;//Monday, PM 06:00:00

    GMI_RESULT Result = m_Center->RegisterEventDetector( AlarmInputDetector, &Info, sizeof(Info) );
    if ( FAILED( Result ) )
    {
        m_Center->Deinitialize();
        printf( "EventCenter RegisterEventDetector failed \n" );
        return Result;
    }
	m_IsStartGPIOInput = 1;
    return Result;
}

GMI_RESULT EventTransactionCenter::StopGPIOAlarmInput()
{
	if(0 == m_IsStartGPIOInput)
	{
		fprintf(stderr, "StopGPIOAlarmInput has done.\n");
		return GMI_SUCCESS;
	}
    GMI_RESULT Result = m_Center->UnregisterEventDetector( EVENT_DETECTOR_ID_ALARM_INPUT );
    if ( FAILED( Result ) )
    {
        return Result;
    }
	m_IsStartGPIOInput = 0;

    return Result;
}

GMI_RESULT EventTransactionCenter::StartGPIOAlarmOutput()
{
	if(1 == m_IsStartGPIOOutput)
	{
		fprintf(stderr, "StartGPIOAlarmOutput has done.\n");
		return GMI_SUCCESS;
	}
	int32_t i = 0;
    ReferrencePtr<AlarmOutput> AlarmOutputProcessor( BaseMemoryManager::Instance().New<AlarmOutput>( EVENT_PROCESSOR_ID_ALARM_OUTPUT ) );
    if ( NULL == AlarmOutputProcessor.GetPtr() )
    {
        m_Center->Deinitialize();
        printf( "allocating AlarmOutputProcessor object failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

	for(i=0; i<MAX_NUM_EVENT_TYPE; i++)
	{
		if(0 < CheckCurBitValidByStrategyId(i, EVENT_PROCESSOR_ID_ALARM_OUTPUT))
		{
			AlarmOutputProcessor->AddDetectorId( i );
			printf("AlarmOutputProcessor %d\n", i);
		}
	}

    //AlarmOutputProcessor->AddDetectorId( EVENT_DETECTOR_ID_ALARM_INPUT );
	//AlarmOutputProcessor->AddDetectorId( EVENT_DETECTOR_ID_HUMAN_DETECT);

    AlarmOutputProcessor->SetEventCallback( m_Callback, m_UserData );

    struct AlarmOutputInfo Info;
    Info.s_OutputNumber = 0;
#if defined( __linux__ )
    strcpy( Info.s_Name, "AlarmInput" );
#elif defined( _WIN32 )
    strcpy_s( Info.s_Name, "AlarmInput" );
#endif
    Info.s_WorkMode = e_AlarmOutputWorkMode_DelayAutoTrigger;
    Info.s_DelayTime = 10;
    //Info.s_ScheduleTimeNumber = 1;
    //Info.s_ScheduleTime[0].s_TimeType  = e_TimeType_WeekCycle;
    //Info.s_ScheduleTime[0].s_StartTime = 0x106000000000000;//Monday, AM 06:00:00
    //Info.s_ScheduleTime[0].s_EndTime   = 0x112000000000000;//Monday, PM 06:00:00

    GMI_RESULT Result = m_Center->RegisterEventProcessor( AlarmOutputProcessor, &Info, sizeof(Info) );
    if ( FAILED( Result ) )
    {
        m_Center->Deinitialize();
        printf( "EventCenter RegisterEventProcessor failed \n" );
        return Result;
    }

	m_IsStartGPIOOutput = 1;
    return Result;
}

GMI_RESULT EventTransactionCenter::StopGPIOAlarmOutput()
{
	if(0 == m_IsStartGPIOOutput)
	{
		fprintf(stderr, "StopGPIOAlarmOutput has done.\n");
		return GMI_SUCCESS;
	}
    GMI_RESULT Result = m_Center->UnregisterEventProcessor( EVENT_PROCESSOR_ID_ALARM_OUTPUT );
    if ( FAILED( Result ) )
    {
        return Result;
    }
	m_IsStartGPIOOutput = 0;

    return Result;
}

GMI_RESULT EventTransactionCenter::StartAlarmInfoRecord()
{
	if(1 == m_IsStartInfoRecord)
	{
		fprintf(stderr, "StartAlarmInfoRecord has done.\n");
		return GMI_SUCCESS;
	}
	int32_t i = 0;
    ReferrencePtr<EventProcessInfoRecord> AlarmInfoRecordProcessor( BaseMemoryManager::Instance().New<EventProcessInfoRecord>( EVENT_PROCESSOR_ID_INFO_RECORD ) );
    if ( NULL == AlarmInfoRecordProcessor.GetPtr() )
    {
        m_Center->Deinitialize();
        printf( "allocating AlarmInfoRecordProcessor object failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

	for(i=0; i<MAX_NUM_EVENT_TYPE; i++)
	{
		if(0 < CheckCurBitValidByStrategyId(i, EVENT_PROCESSOR_ID_INFO_RECORD))
		{
			AlarmInfoRecordProcessor->AddDetectorId( i );
			printf("AlarmInfoRecordProcessor %d\n", i);
		}
	}
    //AlarmInfoRecordProcessor->AddDetectorId( EVENT_DETECTOR_ID_ALARM_INPUT );
	
	//AlarmInfoRecordProcessor->AddDetectorId( EVENT_DETECTOR_ID_HUMAN_DETECT);


    GMI_RESULT Result = m_Center->RegisterEventProcessor( AlarmInfoRecordProcessor, NULL, 0);
    if ( FAILED( Result ) )
    {
        m_Center->Deinitialize();
        printf( "EventCenter RegisterEventProcessor failed \n" );
        return Result;
    }
	m_IsStartInfoRecord = 1;

    return Result;
}

GMI_RESULT EventTransactionCenter::StopAlarmInfoRecord()
{
	if(0 == m_IsStartInfoRecord)
	{
		fprintf(stderr, "StopAlarmInfoRecord has done.\n");
		return GMI_SUCCESS;
	}
    GMI_RESULT Result = m_Center->UnregisterEventProcessor( EVENT_PROCESSOR_ID_INFO_RECORD );
    if ( FAILED( Result ) )
    {
        return Result;
    }
	
	m_IsStartInfoRecord = 0;

    return Result;
}


GMI_RESULT EventTransactionCenter::StartHumanDetect()
{
	if(1 == m_IsStartHumanDetect)
	{
		fprintf(stderr, "StartHumanDetect has done.\n");
		return GMI_SUCCESS;
	}
    ReferrencePtr<HumanDetect> HumanDetector( BaseMemoryManager::Instance().New<HumanDetect>( e_EventDetectorType_Passive, EVENT_DETECTOR_ID_HUMAN_DETECT) );
    if ( NULL == HumanDetector.GetPtr() )
    {
        m_Center->Deinitialize();
        printf( "allocating HumanDetector object failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    struct HumanDetectInfo Info;
    Info.s_CheckTime = 1000;
    //Info.s_ScheduleTimeNumber = 1;
    //Info.s_ScheduleTime[0].s_StartTime = 0x106000000000000;//Monday, AM 06:00:00
    //Info.s_ScheduleTime[0].s_EndTime   = 0x112000000000000;//Monday, PM 06:00:00

    GMI_RESULT Result = m_Center->RegisterEventDetector( HumanDetector, &Info, sizeof(Info) );
    if ( FAILED( Result ) )
    {
        m_Center->Deinitialize();
        printf( "EventCenter RegisterEventDetector failed \n" );
        return Result;
    }
	m_IsStartHumanDetect = 1;

    return Result;
}

GMI_RESULT EventTransactionCenter::StopHumanDetect()
{
	if(0 == m_IsStartHumanDetect)
	{
		fprintf(stderr, "StopHumanDetect has done.\n");
		return GMI_SUCCESS;
	}
    GMI_RESULT Result = m_Center->UnregisterEventDetector( EVENT_DETECTOR_ID_HUMAN_DETECT);
    if ( FAILED( Result ) )
    {
        return Result;
    }
	m_IsStartHumanDetect = 0;

    return Result;
}

