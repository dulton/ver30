#include "event_transaction_center.h"

#include "alarm_input.h"
#include "alarm_output.h"
#include "event_process_inforecord.h"
#include "human_detect.h"

AlarmEventConfigInfo g_CurStartedEvent[MAX_NUM_EVENT_TYPE];
AlarmInputInfo g_CurStartedAlaramIn[MAX_NUM_GPIO_IN];


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


size_t EventTransactionCenter::m_IsStartGPIOInputEx[MAX_NUM_GPIO_IN] = {0};
size_t EventTransactionCenter::m_IsStartGPIOOutputEx[MAX_NUM_GPIO_OUT] = {0};




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

	memset(g_CurStartedEvent, 0, sizeof(g_CurStartedEvent[0])*MAX_NUM_EVENT_TYPE);
	memset(g_CurStartedAlaramIn, 0, sizeof(g_CurStartedAlaramIn[0])*MAX_NUM_GPIO_IN);

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

	#if 0
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
	#endif
	Result = StopGPIOAlarmInputEx(0);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    Result = StopGPIOAlarmOutputEx(0);
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
	
    GMI_RESULT Result = GMI_SUCCESS;
    do
	{
		if((NULL == Parameter)
			|| (sizeof(AlarmInputInfo) > ParameterLength))
		{
			fprintf(stderr, "ConfigureAlarmEvent GMI_INVALID_PARAMETER.\n");
			Result = GMI_INVALID_PARAMETER;
			break;
		}

		AlarmInputInfo *InfoPtr = (AlarmInputInfo *)Parameter;

		if((0 > InfoPtr->s_InputNumber)
	  	|| ((MAX_NUM_GPIO_IN-1) < InfoPtr->s_InputNumber))
		{
			fprintf(stderr, "ConfigureGPIOAlarmInput InputNumber %d error.\n", InfoPtr->s_InputNumber);
			Result = GMI_INVALID_PARAMETER;
			break;
		}
		
		memcpy(&(g_CurStartedAlaramIn[InfoPtr->s_InputNumber]), (AlarmInputInfo *)Parameter, sizeof(AlarmInputInfo));
		if(FLAG_EVENT_ENABLE == g_CurStartedAlaramIn[InfoPtr->s_InputNumber].s_EnableFlag)
		{
			Result = StartGPIOAlarmInputEx(Parameter, sizeof(AlarmInputInfo));
					
		}
		else
		{
			Result = StopGPIOAlarmInputEx(InfoPtr->s_InputNumber);
					
		}
		
	}while(0);

    return Result;
}

GMI_RESULT EventTransactionCenter::ConfigureGPIOAlarmOutput( const void_t *Parameter, size_t ParameterLength )
{
    GMI_RESULT Result = GMI_SUCCESS;
	struct AlarmOutputInfo *InfoPtr;

	do
	{
		if((sizeof(AlarmOutputInfo) > ParameterLength)
			|| (NULL == Parameter))
		{
			Result = GMI_INVALID_PARAMETER;
			break;
		}
		InfoPtr = (struct AlarmOutputInfo *)Parameter;

		if(FLAG_EVENT_ENABLE == InfoPtr->s_EnableFlag)
		{
			StartGPIOAlarmOutputEx(Parameter, sizeof(AlarmOutputInfo));
		}
		else
		{
			StopGPIOAlarmOutputEx(InfoPtr->s_OutputNumber);
		}
	}while(0);
	
	
    return Result;
}


GMI_RESULT EventTransactionCenter::ConfigureAlarmEvent(const enum AlarmEventType EventType, const void *Parameter, size_t ParameterLength)
{
	GMI_RESULT Result = GMI_SUCCESS;

	do
	{
		if((NULL == Parameter)
			|| (sizeof(AlarmEventConfigInfo) > ParameterLength)
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
				//case e_AlarmEventType_AlarmInput:	
					//Result = StartGPIOAlarmInput();
					//break;
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
				//case e_AlarmEventType_AlarmInput:
					//Result = StopGPIOAlarmInput();
					//break;
				case e_AlarmEventType_HumanDetect:
					Result = StopHumanDetect();
					break;
				default:
					fprintf(stderr, "stop event:EventType %d no support.\n", EventType);
					break;
			}
		}

		#if 0
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
		#endif

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

#if 0
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
#endif

GMI_RESULT EventTransactionCenter::StartGPIOAlarmInputEx(const void *Parameter, size_t ParamterLength)
{
	GMI_RESULT Result = GMI_SUCCESS;
	
	if((0 == ParamterLength)
		|| (NULL == Parameter)
		|| (ParamterLength < sizeof(struct AlarmInputInfo)))
	{
		fprintf(stderr, "StartGPIOAlarmInputEx GMI_INVALID_PARAMETER.\n");
		Result = GMI_INVALID_PARAMETER;
		return Result;
	}

	struct AlarmInputInfo Info;
	memset(&Info, 0, sizeof(Info));
	memcpy(&Info, Parameter, sizeof(Info));
	if((0 > Info.s_InputNumber)
	  || ((MAX_NUM_GPIO_IN-1) < Info.s_InputNumber))
	{
		fprintf(stderr, "StartGPIOAlarmInputEx InputNumber %d error.\n", Info.s_InputNumber);
		Result = GMI_INVALID_PARAMETER;
		return Result;
	}

	if(1 == m_IsStartGPIOInputEx[Info.s_InputNumber])
	{
		fprintf(stderr, "StartGPIOAlarmInputEx has done.\n");
		return GMI_SUCCESS;
	}
    ReferrencePtr<AlarmInput> AlarmInputDetector( BaseMemoryManager::Instance().New<AlarmInput>( e_EventDetectorType_Passive, EVENT_DETECTOR_ID_ALARM_INPUT, Info.s_InputNumber ) );
    if ( NULL == AlarmInputDetector.GetPtr() )
    {
        m_Center->Deinitialize();
        printf( "allocating AlarmInputDetector object failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

	if(Info.s_CheckTime < 100)
	{
    	Info.s_CheckTime = 100;
	}

	printf("StartGPIOAlarmInputEx Info.s_InputNumber=%d\n", Info.s_InputNumber);
    Result = m_Center->RegisterEventDetector( AlarmInputDetector, &Info, sizeof(Info) );
    if ( FAILED( Result ) )
    {
        m_Center->Deinitialize();
        printf( "EventCenter RegisterEventDetector failed \n" );
        return Result;
    }
	m_IsStartGPIOInputEx[Info.s_InputNumber] = 1;
    return Result;
}

GMI_RESULT EventTransactionCenter::StopGPIOAlarmInputEx(size_t InIoIndex)
{
	GMI_RESULT Result = GMI_SUCCESS;
	if((0 > InIoIndex)
	  || ((MAX_NUM_GPIO_IN-1) < InIoIndex))
	{
		fprintf(stderr, "StartGPIOAlarmInputEx InIoIndex %d error.\n", InIoIndex);
		Result = GMI_INVALID_PARAMETER;
		return Result;
	}
	
	if(0 == m_IsStartGPIOInputEx[InIoIndex])
	{
		fprintf(stderr, "StartGPIOAlarmInputEx %d has done.\n", InIoIndex);
		return GMI_SUCCESS;
	}
	printf("StopGPIOAlarmInputEx InIoIndex %d\n", InIoIndex);
    Result = m_Center->UnregisterEventDetector( EVENT_DETECTOR_ID_ALARM_INPUT, InIoIndex);
    if ( FAILED( Result ) )
    {
        return Result;
    }
	m_IsStartGPIOInputEx[InIoIndex] = 0;

    return Result;
}

#if 0
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

	for(i=1; i<=MAX_NUM_EVENT_TYPE; i++)
	{
		//if(0 < CheckCurBitValidByStrategyId(i, EVENT_PROCESSOR_ID_ALARM_OUTPUT))
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
#endif

GMI_RESULT EventTransactionCenter::StartGPIOAlarmOutputEx(const void *Parameter, size_t ParamterLength)
{
	GMI_RESULT Result = GMI_SUCCESS;
	
	if((0 == ParamterLength)
		|| (NULL == Parameter)
		|| (ParamterLength < sizeof(struct AlarmOutputInfo)))
	{
		fprintf(stderr, "StartGPIOAlarmOutputEx GMI_INVALID_PARAMETER.\n");
		Result = GMI_INVALID_PARAMETER;
		return Result;
	}

	
    struct AlarmOutputInfo Info;
	memset(&Info, 0, sizeof(Info));
	memcpy(&Info, Parameter, sizeof(Info));

	if((0 > Info.s_OutputNumber)
	  || ((MAX_NUM_GPIO_OUT-1) < Info.s_OutputNumber))
	{
		fprintf(stderr, "StartGPIOAlarmOutputEx OutputNumber %d error.\n", Info.s_OutputNumber);
		Result = GMI_INVALID_PARAMETER;
		return Result;
	}
	
	if(1 == m_IsStartGPIOOutputEx[Info.s_OutputNumber])
	{
		fprintf(stderr, "StartGPIOAlarmOutputEx has done.\n");
		return GMI_SUCCESS;
	}

	printf("********Alarm out*******\n");
	printf("s_DelayTime=%d\n", Info.s_DelayTime);
	printf("**************************\n\n");
	
	int32_t i = 0, j = 0;
    ReferrencePtr<AlarmOutput> AlarmOutputProcessor( BaseMemoryManager::Instance().New<AlarmOutput>( EVENT_PROCESSOR_ID_ALARM_OUTPUT, Info.s_OutputNumber ) );
    if ( NULL == AlarmOutputProcessor.GetPtr() )
    {
        m_Center->Deinitialize();
        printf( "allocating AlarmOutputProcessor object failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

	struct DetectorInfo TmpDetectInfo;
	memset(&TmpDetectInfo, 0, sizeof(TmpDetectInfo));
	for(i=1; i<=MAX_NUM_EVENT_TYPE; i++)
	{
		if(i == e_AlarmEventType_AlarmInput)
		{
			for(j=0; j<MAX_NUM_GPIO_IN; j++)
			{
				TmpDetectInfo.s_DetectorId = i;
				TmpDetectInfo.s_Index = j;
				AlarmOutputProcessor->AddDetectorId( TmpDetectInfo );
				printf("AlarmOutputProcessor AlarmInput %d %d\n", i, j);
			}
		}
		else
		{
			TmpDetectInfo.s_DetectorId = i;
			TmpDetectInfo.s_Index = 0;
			AlarmOutputProcessor->AddDetectorId( TmpDetectInfo );
			printf("AlarmOutputProcessor %d\n", i);
		}
	}

    //AlarmOutputProcessor->AddDetectorId( EVENT_DETECTOR_ID_ALARM_INPUT );
	//AlarmOutputProcessor->AddDetectorId( EVENT_DETECTOR_ID_HUMAN_DETECT);

    AlarmOutputProcessor->SetEventCallback( m_Callback, m_UserData );

    Result = m_Center->RegisterEventProcessor( AlarmOutputProcessor, &Info, sizeof(Info) );
    if ( FAILED( Result ) )
    {
        m_Center->Deinitialize();
        printf( "EventCenter RegisterEventProcessor failed \n" );
        return Result;
    }

	m_IsStartGPIOOutputEx[Info.s_OutputNumber] = 1;
    return Result;
}

GMI_RESULT EventTransactionCenter::StopGPIOAlarmOutputEx(size_t OutIoIndex)
{
	GMI_RESULT Result = GMI_SUCCESS;
	if((0 > OutIoIndex)
	  || ((MAX_NUM_GPIO_OUT-1) < OutIoIndex))
	{
		fprintf(stderr, "StopGPIOAlarmOutputEx OutIoIndex %d error.\n", OutIoIndex);
		Result = GMI_INVALID_PARAMETER;
		return Result;
	}
	
	if(0 == m_IsStartGPIOOutputEx[OutIoIndex])
	{
		fprintf(stderr, "StopGPIOAlarmOutputEx %d has done.\n", OutIoIndex);
		return GMI_SUCCESS;
	}
    Result = m_Center->UnregisterEventProcessor( EVENT_PROCESSOR_ID_ALARM_OUTPUT, OutIoIndex );
    if ( FAILED( Result ) )
    {
        return Result;
    }
	m_IsStartGPIOOutputEx[OutIoIndex] = 0;

    return Result;
}


GMI_RESULT EventTransactionCenter::StartAlarmInfoRecord()
{
	if(1 == m_IsStartInfoRecord)
	{
		fprintf(stderr, "StartAlarmInfoRecord has done.\n");
		return GMI_SUCCESS;
	}
	int32_t i = 0, j = 0;
    ReferrencePtr<EventProcessInfoRecord> AlarmInfoRecordProcessor( BaseMemoryManager::Instance().New<EventProcessInfoRecord>( EVENT_PROCESSOR_ID_INFO_RECORD, 0 ) );
    if ( NULL == AlarmInfoRecordProcessor.GetPtr() )
    {
        m_Center->Deinitialize();
        printf( "allocating AlarmInfoRecordProcessor object failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

	
	struct DetectorInfo TmpDetectInfo;
	memset(&TmpDetectInfo, 0, sizeof(TmpDetectInfo));
	for(i=1; i<=MAX_NUM_EVENT_TYPE; i++)
	{
		if(i == e_AlarmEventType_AlarmInput)
		{
			for(j=0; j<MAX_NUM_GPIO_IN; j++)
			{
				TmpDetectInfo.s_DetectorId = i;
				TmpDetectInfo.s_Index = j;
				AlarmInfoRecordProcessor->AddDetectorId( TmpDetectInfo );
				printf("AlarmInfoRecordProcessor AlarmInput %d %d\n", i, j);
			}
		}
		else
		{
			TmpDetectInfo.s_DetectorId = i;
			TmpDetectInfo.s_Index = 0;
			AlarmInfoRecordProcessor->AddDetectorId( TmpDetectInfo );
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
    GMI_RESULT Result = m_Center->UnregisterEventProcessor( EVENT_PROCESSOR_ID_INFO_RECORD, 0 );
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
    ReferrencePtr<HumanDetect> HumanDetector( BaseMemoryManager::Instance().New<HumanDetect>( e_EventDetectorType_Passive, EVENT_DETECTOR_ID_HUMAN_DETECT, 0) );
    if ( NULL == HumanDetector.GetPtr() )
    {
        m_Center->Deinitialize();
        printf( "allocating HumanDetector object failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    struct HumanDetectInfo Info;
    Info.s_CheckTime = g_CurStartedEvent[e_AlarmEventType_HumanDetect-1].s_CheckTime;
	if(Info.s_CheckTime < 200)
	{
		Info.s_CheckTime = 200;
	}
	printf("********human detect*******\n");
	printf("s_CheckTime=%d\n", g_CurStartedEvent[e_AlarmEventType_HumanDetect-1].s_CheckTime);
	printf("s_MinSensVal=%d\n", g_CurStartedEvent[e_AlarmEventType_HumanDetect-1].s_MinSensVal);
	printf("s_MaxSensVal=%d\n", g_CurStartedEvent[e_AlarmEventType_HumanDetect-1].s_MaxSensVal);
	printf("**************************\n\n");
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
    GMI_RESULT Result = m_Center->UnregisterEventDetector( EVENT_DETECTOR_ID_HUMAN_DETECT, 0);
    if ( FAILED( Result ) )
    {
        return Result;
    }
	m_IsStartHumanDetect = 0;

    return Result;
}

