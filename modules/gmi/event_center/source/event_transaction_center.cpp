#include "event_transaction_center.h"
#include "event_common_header.h"
#include "alarm_input.h"
#include "alarm_output.h"
#include "event_process_inforecord.h"
#include "human_detect.h"
#include "event_process_light.h"


AlarmEventConfigInfoEx g_CurStartedEvent[MAX_NUM_EVENT_TYPE];
AlarmInputInfoEx g_CurStartedAlarmIn[MAX_NUM_GPIO_IN];
AlarmOutputInfoEx g_CurStartedAlarmOut[MAX_NUM_GPIO_OUT];
uint64_t        g_AlarmMessageId = 0;

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
		if(0 < (g_CurStartedEvent[i].s_AlarmEventConfigInfo.s_LinkAlarmStrategy & (1<<(BitPos-1))))
		{
			IsValid = 1;
			break;
		}
	}
	return IsValid;
}

int32_t CalcHumanDetectMapValue(int32_t Flag, int32_t Value)
{
	if(Value > 100)
	{
		Value = 100;
	}
	else if(Value < 0)
	{
		Value = 0;
	}

	int32_t RetVal = 0;
	
	switch(Flag)
	{
		case FLAG_MIN_VALUE:
			RetVal = (((AVG_REF_VALUE_HUMAN_DETECT - CHG_REF_VALUE_HUMAN_DETECT) - MIN_REF_VALUE_HUMAN_DETECT) * Value)/100
				    + MIN_REF_VALUE_HUMAN_DETECT;
			break;
		case FLAG_MAX_VALUE:
			RetVal = ((MAX_REF_VALUE_HUMAN_DETECT - (AVG_REF_VALUE_HUMAN_DETECT + CHG_REF_VALUE_HUMAN_DETECT)) * (100-Value))/100
				     + (AVG_REF_VALUE_HUMAN_DETECT + CHG_REF_VALUE_HUMAN_DETECT);
			break;
		default:
			fprintf(stderr, "CalcHumanDetectMapValue Flag %d error.\n", Flag);
			break;
	}

	return RetVal;
}



size_t EventTransactionCenter::m_IsStartHumanDetect = 0;
size_t EventTransactionCenter::m_IsStartInfoRecord = 0;
size_t EventTransactionCenter::m_IsStartGPIOInputEx[MAX_NUM_GPIO_IN] = {0};
size_t EventTransactionCenter::m_IsStartGPIOOutputEx[MAX_NUM_GPIO_OUT] = {0};
size_t EventTransactionCenter::m_IsStartLinkLight = 0;



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
	memset(g_CurStartedAlarmIn, 0, sizeof(g_CurStartedAlarmIn[0])*MAX_NUM_GPIO_IN);
	memset(g_CurStartedAlarmOut, 0, sizeof(g_CurStartedAlarmOut[0])*MAX_NUM_GPIO_OUT);

	Result = m_Center->Start();
    if ( FAILED( Result ) )
    {
    	fprintf(stderr, "m_Center start error.\n");
        m_Center->Deinitialize();
	    m_Center = NULL;
    }
	printf("Event Start done.\n");

    return Result;
}

GMI_RESULT EventTransactionCenter::Stop()
{
    GMI_RESULT Result = m_Center->Stop();
	int32_t i = 0;
    if ( FAILED( Result ) )
    {
        return Result;
    }

	for(i=0; i<MAX_NUM_GPIO_IN; i++)
	{
		Result = StopGPIOAlarmInputEx(i);
	    if ( FAILED( Result ) )
	    {
	        //return Result;
	    }
	}

	for(i=0; i<MAX_NUM_GPIO_OUT; i++)
	{
	    Result = StopGPIOAlarmOutputEx(i);
	    if ( FAILED( Result ) )
	    {
	        //return Result;
	    }
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
	printf("Event Stop done.\n");
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
		
		memcpy(&(g_CurStartedAlarmIn[InfoPtr->s_InputNumber]), (AlarmInputInfo *)Parameter, sizeof(AlarmInputInfo));
		if(FLAG_EVENT_ENABLE == g_CurStartedAlarmIn[InfoPtr->s_InputNumber].s_AlarmInputInfo.s_EnableFlag)
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
		if((InfoPtr->s_OutputNumber < 0)
			|| (InfoPtr->s_OutputNumber > (MAX_NUM_GPIO_OUT-1)))
		{
			Result = GMI_INVALID_PARAMETER;
			break;
		}
		memcpy(&(g_CurStartedAlarmOut[InfoPtr->s_OutputNumber]), InfoPtr, sizeof(AlarmOutputInfo));
	
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


GMI_RESULT EventTransactionCenter::ConfigureAlarmEvent(const size_t EventType, const void *Parameter, size_t ParameterLength)
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
		if(FLAG_EVENT_ENABLE == g_CurStartedEvent[EventType-1].s_AlarmEventConfigInfo.s_EnableFlag)
		{
			switch(EventType)
			{
				case EVENT_DETECTOR_ID_HUMAN_DETECT:
					if(0 >= g_CurStartedEvent[EventType-1].s_AlarmEventConfigInfo.s_ExtData.s_HumanDetectExInfo.s_MaxSensVal)
					{
						g_CurStartedEvent[EventType-1].s_AlarmEventConfigInfo.s_ExtData.s_HumanDetectExInfo.s_MinSensVal 
							= CalcHumanDetectMapValue(FLAG_MIN_VALUE, g_CurStartedEvent[EventType-1].s_AlarmEventConfigInfo.s_ExtData.s_HumanDetectExInfo.s_Sensitivity);
						g_CurStartedEvent[EventType-1].s_AlarmEventConfigInfo.s_ExtData.s_HumanDetectExInfo.s_MaxSensVal
							= CalcHumanDetectMapValue(FLAG_MAX_VALUE, g_CurStartedEvent[EventType-1].s_AlarmEventConfigInfo.s_ExtData.s_HumanDetectExInfo.s_Sensitivity);
					}
					fprintf(stderr, "Human detect s_Sensitivity = %d, MinSensVal = %d, MaxSensVal = %d\n", 
						g_CurStartedEvent[EventType-1].s_AlarmEventConfigInfo.s_ExtData.s_HumanDetectExInfo.s_Sensitivity,
						g_CurStartedEvent[EventType-1].s_AlarmEventConfigInfo.s_ExtData.s_HumanDetectExInfo.s_MinSensVal,
						g_CurStartedEvent[EventType-1].s_AlarmEventConfigInfo.s_ExtData.s_HumanDetectExInfo.s_MaxSensVal);
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
				case EVENT_DETECTOR_ID_HUMAN_DETECT:
					Result = StopHumanDetect();
					break;
				default:
					fprintf(stderr, "stop event:EventType %d no support.\n", EventType);
					break;
			}
		}

		if(1)//0 < CheckCurBitValid(EVENT_PROCESSOR_ID_INFO_UPLOAD))
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

		if(0 < CheckCurBitValid(EVENT_PROCESSOR_ID_LINK_LIGHT))
		{
			Result = StartAlarmLinkLight(Parameter, sizeof(AlarmEventConfigInfo));
			if ( FAILED( Result ) )
			{
				fprintf(stderr, "ConfigureAlarmEvent StartAlarmLinkLight error.\n");
				break;
			}
		}
		else
		{
			Result = StopAlarmLinkLight();
			if ( FAILED( Result ) )
			{
				fprintf(stderr, "ConfigureAlarmEvent StopAlarmLinkLight error.\n");
				break;
			}
		}
		
	}while(0);

    return Result;
}

GMI_RESULT EventTransactionCenter::ConfigureAlarmScheduleTime(size_t ScheduleId,const void_t * Parameter, size_t Parameterlength)
{
	GMI_RESULT Result = GMI_SUCCESS;

	if((0 == Parameterlength)
		|| (NULL == Parameter)
		|| (Parameterlength < sizeof(struct AlarmScheduleTimeInfo)))
	{
		fprintf(stderr, "ConfigureAlarmScheduleTime GMI_INVALID_PARAMETER.\n");
		Result = GMI_INVALID_PARAMETER;
		return Result;
	}

	AlarmScheduleTimeInfo *TmpInfo = (AlarmScheduleTimeInfo *)Parameter;
	
	do
	{
		switch(ScheduleId)
		{
			case SCHEDULE_TIME_ID_ALARM_IN:
				if((TmpInfo->s_Index < 0)
					|| (TmpInfo->s_Index > (MAX_NUM_GPIO_IN-1)))
				{
					fprintf(stderr, "ConfigureAlarmScheduleTime  Index %d SCHEDULE_TIME_ID_ALARM_IN error.\n", TmpInfo->s_Index);
					Result = GMI_INVALID_PARAMETER;
					break;
				}
				memcpy(&(g_CurStartedAlarmIn[TmpInfo->s_Index].s_ScheduleTime[0][0]), &(TmpInfo->s_ScheduleTime[0][0]), sizeof(ScheduleTimeInfo)*7*MAX_SEG_TIME_PERDAY);
				break;
			case SCHEDULE_TIME_ID_ALARM_OUT:
				if((TmpInfo->s_Index < 0)
					|| (TmpInfo->s_Index > (MAX_NUM_GPIO_OUT-1)))
				{
					fprintf(stderr, "ConfigureAlarmScheduleTime  Index %d SCHEDULE_TIME_ID_ALARM_OUT error.\n", TmpInfo->s_Index);
					Result = GMI_INVALID_PARAMETER;
					break;
				}
				memcpy(&(g_CurStartedAlarmOut[TmpInfo->s_Index].s_ScheduleTime[0][0]), &(TmpInfo->s_ScheduleTime[0][0]), sizeof(ScheduleTimeInfo)*7*MAX_SEG_TIME_PERDAY);
				break;
			case SCHEDULE_TIME_ID_HUMAN_DETECT:
				memcpy(&(g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_ScheduleTime[0][0]), &(TmpInfo->s_ScheduleTime[0][0]), sizeof(ScheduleTimeInfo)*7*MAX_SEG_TIME_PERDAY);
				break;
			default:
				fprintf(stderr, "ConfigureAlarmScheduleTime  ScheduleId %d error.\n", ScheduleId);
				break;
		}
	}while(0);
	
	
	return Result;
}


GMI_RESULT EventTransactionCenter::StartGPIOAlarmInputEx(const void *Parameter, size_t ParameterLength)
{
	GMI_RESULT Result = GMI_SUCCESS;
	
	if((0 == ParameterLength)
		|| (NULL == Parameter)
		|| (ParameterLength < sizeof(struct AlarmInputInfo)))
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

	if(Info.s_CheckTime < 200)
	{
    	Info.s_CheckTime = 200;
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
	
	printf("StopGPIOAlarmInputEx InIoIndex %d start\n", InIoIndex);
	if(0 == m_IsStartGPIOInputEx[InIoIndex])
	{
		fprintf(stderr, "StartGPIOAlarmInputEx %d has done.\n", InIoIndex);
		return GMI_SUCCESS;
	}
    Result = m_Center->UnregisterEventDetector( EVENT_DETECTOR_ID_ALARM_INPUT, InIoIndex);
    if ( FAILED( Result ) )
    {
        return Result;
    }
	m_IsStartGPIOInputEx[InIoIndex] = 0;
	printf("StopGPIOAlarmInputEx InIoIndex %d end\n", InIoIndex);

    return Result;
}

GMI_RESULT EventTransactionCenter::StartGPIOAlarmOutputEx(const void *Parameter, size_t ParameterLength)
{
	GMI_RESULT Result = GMI_SUCCESS;
	
	if((0 == ParameterLength)
		|| (NULL == Parameter)
		|| (ParameterLength < sizeof(struct AlarmOutputInfo)))
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
		if(i == EVENT_DETECTOR_ID_ALARM_INPUT)
		{
			for(j=0; j<MAX_NUM_GPIO_IN; j++)
			{
				TmpDetectInfo.s_DetectorId = i;
				TmpDetectInfo.s_Index = j;
				AlarmOutputProcessor->AddDetectorId( TmpDetectInfo );
				//printf("AlarmOutputProcessor AlarmInput %d %d\n", i, j);
			}
		}
		else
		{
			TmpDetectInfo.s_DetectorId = i;
			TmpDetectInfo.s_Index = 0;
			AlarmOutputProcessor->AddDetectorId( TmpDetectInfo );
			//printf("AlarmOutputProcessor %d\n", i);
		}
	}

    //AlarmOutputProcessor->AddDetectorId( EVENT_DETECTOR_ID_ALARM_INPUT );
	//AlarmOutputProcessor->AddDetectorId( EVENT_DETECTOR_ID_HUMAN_DETECT);

    //AlarmOutputProcessor->SetEventCallback( m_Callback, m_UserData );

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
	
	printf("StopGPIOAlarmOutputEx OutIoIndex %d start\n", OutIoIndex);
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
	printf("StopGPIOAlarmOutputEx OutIoIndex %d end\n", OutIoIndex);

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
    ReferrencePtr<EventProcessInfoRecord> AlarmInfoRecordProcessor( BaseMemoryManager::Instance().New<EventProcessInfoRecord>( EVENT_PROCESSOR_ID_INFO_UPLOAD, 0 ) );
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
		if(i == EVENT_DETECTOR_ID_ALARM_INPUT)
		{
			for(j=0; j<MAX_NUM_GPIO_IN; j++)
			{
				TmpDetectInfo.s_DetectorId = i;
				TmpDetectInfo.s_Index = j;
				AlarmInfoRecordProcessor->AddDetectorId( TmpDetectInfo );
				//printf("AlarmInfoRecordProcessor AlarmInput %d %d\n", i, j);
			}
		}
		else
		{
			TmpDetectInfo.s_DetectorId = i;
			TmpDetectInfo.s_Index = 0;
			AlarmInfoRecordProcessor->AddDetectorId( TmpDetectInfo );
			//printf("AlarmInfoRecordProcessor %d\n", i);
		}
	}
   
    AlarmInfoRecordProcessor->SetEventCallback( m_Callback, m_UserData );

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
	
	printf("StopAlarmInfoRecord start\n");
    GMI_RESULT Result = m_Center->UnregisterEventProcessor( EVENT_PROCESSOR_ID_INFO_UPLOAD, 0 );
    if ( FAILED( Result ) )
    {
        return Result;
    }
	
	m_IsStartInfoRecord = 0;
	printf("StopAlarmInfoRecord end\n");

    return Result;
}


GMI_RESULT EventTransactionCenter::StartAlarmLinkLight(const void *Parameter, size_t ParameterLength)
{
	GMI_RESULT Result = GMI_SUCCESS;
	
	if((0 == ParameterLength)
		|| (NULL == Parameter)
		|| (ParameterLength < sizeof(struct AlarmEventConfigInfo)))
	{
		fprintf(stderr, "StartAlarmLinkLight GMI_INVALID_PARAMETER.\n");
		Result = GMI_INVALID_PARAMETER;
		return Result;
	}

	
    struct AlarmEventConfigInfo Info;
	memset(&Info, 0, sizeof(Info));
	memcpy(&Info, Parameter, sizeof(Info));
	
	if(1 == m_IsStartLinkLight)
	{
		fprintf(stderr, "StartAlarmLinkLight has done.\n");
		return GMI_SUCCESS;
	}

	printf("********link light*******\n");
	printf("s_DelayTime=%d\n", Info.s_LinkAlarmExtInfo.s_DelayTime);
	printf("**************************\n\n");
	
    ReferrencePtr<EventProcessLight> EventProcessLightProcessor( BaseMemoryManager::Instance().New<EventProcessLight>( EVENT_PROCESSOR_ID_LINK_LIGHT, 0 ) );
    if ( NULL == EventProcessLightProcessor.GetPtr() )
    {
        m_Center->Deinitialize();
        printf( "allocating EventProcessLightProcessor object failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

	struct DetectorInfo TmpDetectInfo;
	memset(&TmpDetectInfo, 0, sizeof(TmpDetectInfo));
	TmpDetectInfo.s_DetectorId = EVENT_DETECTOR_ID_HUMAN_DETECT;
	TmpDetectInfo.s_Index = 0;
	
	EventProcessLightProcessor->AddDetectorId( TmpDetectInfo );

    Result = m_Center->RegisterEventProcessor( EventProcessLightProcessor, &Info, sizeof(Info) );
    if ( FAILED( Result ) )
    {
        m_Center->Deinitialize();
        printf( "EventCenter RegisterEventProcessor failed \n" );
        return Result;
    }

	m_IsStartLinkLight = 1;
    return Result;
}

GMI_RESULT EventTransactionCenter::StopAlarmLinkLight()
{
	GMI_RESULT Result = GMI_SUCCESS;
	
	if(0 == m_IsStartLinkLight)
	{
		fprintf(stderr, "StopAlarmLinkLight has done.\n");
		return GMI_SUCCESS;
	}
    Result = m_Center->UnregisterEventProcessor( EVENT_PROCESSOR_ID_LINK_LIGHT, 0 );
    if ( FAILED( Result ) )
    {
        return Result;
    }
	m_IsStartLinkLight = 0;
	printf("StopAlarmLinkLight end\n");

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
    Info.s_CheckTime = g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_AlarmEventConfigInfo.s_CheckTime;
	if(Info.s_CheckTime < 200)
	{
		Info.s_CheckTime = 200;
	}
	printf("********human detect*******\n");
	printf("s_CheckTime=%d\n", g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_AlarmEventConfigInfo.s_CheckTime);
	printf("s_MinSensVal=%d\n", g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_AlarmEventConfigInfo.s_ExtData.s_HumanDetectExInfo.s_MinSensVal);
	printf("s_MaxSensVal=%d\n", g_CurStartedEvent[EVENT_DETECTOR_ID_HUMAN_DETECT-1].s_AlarmEventConfigInfo.s_ExtData.s_HumanDetectExInfo.s_MaxSensVal);
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
	
	printf("StopHumanDetect start\n");
    GMI_RESULT Result = m_Center->UnregisterEventDetector( EVENT_DETECTOR_ID_HUMAN_DETECT, 0);
    if ( FAILED( Result ) )
    {
        return Result;
    }
	m_IsStartHumanDetect = 0;
	printf("StopHumanDetect end\n");

    return Result;
}

