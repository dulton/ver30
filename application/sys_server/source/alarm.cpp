#include <time.h>
#include "alarm.h"
#include "ipc_fw_v3.x_resource.h"
#include "event_process_header.h"
#include "log.h"

Alarm::Alarm()
{
}


Alarm::~Alarm()
{
}


GMI_RESULT Alarm::Initialize()
{	
	m_AlarmSession = BaseMemoryManager::Instance().New<AlarmSession>(GMI_SYS_SERVER_C_WARING_PORT, GMI_SDK_S_WARING_PORT);
    if (NULL == m_AlarmSession.GetPtr())
    {
    	SYS_ERROR("m_AlarmSession new fail\n");
    	return GMI_OUT_OF_MEMORY;
    }

    GMI_RESULT Result = m_AlarmSession->Initialize();
    if (FAILED(Result))
    {
    	SYS_ERROR("Alarm Session Initialize fail, Result = 0x%lx\n", Result);
    	return Result;
    }

    Result = m_EventCenter.Start(NULL, 0, EventProcess, this);
    if (FAILED(Result))
    {
    	m_AlarmSession->Deinitialize();
    	SYS_ERROR("event center start fail, Result = 0x%lx\n", Result);
    	return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT Alarm::Deinitialize()
{
	m_EventCenter.Stop();
	m_AlarmSession->Deinitialize();
	m_AlarmSession = NULL;

	return GMI_SUCCESS;
}


GMI_RESULT Alarm::Report(SysPkgAlarmInfor *SysAlarmInfor)
{	
	size_t Transfered;
	GMI_RESULT Result = m_AlarmSession->Send((uint8_t*)SysAlarmInfor, sizeof(SysPkgAlarmInfor), &Transfered);
	if (FAILED(Result))
	{
		SYS_ERROR("send alarm info to sdk fail, Result = 0x%lx\n", Result);
		return Result;
	}

	return GMI_SUCCESS;
}


void Alarm::EventProcess(void_t *UserData, uint32_t EventId, enum EventType Type, void_t *Parameter, size_t ParameterLength)
{
	struct timeval CurrentTime;
    gettimeofday( &CurrentTime, NULL );
    SYS_INFO( "EventProcess: UserData=%p, EventId=%d, Type=%d, Parameter=%p, ParameterLength=%d, current time=%ld:%06ld \n", UserData, EventId, Type, Parameter, ParameterLength, CurrentTime.tv_sec, CurrentTime.tv_usec );
	return;
}


GMI_RESULT Alarm::Config(int32_t AlarmId, const void_t *Parameter, size_t ParameterLength)
{
	GMI_RESULT Result = GMI_SUCCESS;
	
	switch (AlarmId)
	{
	case SYS_DETECTOR_ID_ALARM_INPUT:
		SysPkgAlarmInConfig SysAlarmInCfg;
		memset(&SysAlarmInCfg, 0, sizeof(SysPkgAlarmInConfig));
		if (ParameterLength != sizeof(SysPkgAlarmInConfig))
		{
			SYS_ERROR("ParameterLength is %d, but real length should be %d\n", ParameterLength, sizeof(SysPkgAlarmInConfig));
			return GMI_INVALID_PARAMETER;
		}
		memcpy(&SysAlarmInCfg, Parameter, ParameterLength);

		struct AlarmInputInfo AlmInInfo;
		memset(&AlmInInfo, 0, sizeof(struct AlarmInputInfo));
		AlmInInfo.s_EnableFlag        = SysAlarmInCfg.s_EnableFlag;
		AlmInInfo.s_InputNumber       = SysAlarmInCfg.s_InputNumber;
		AlmInInfo.s_NormalStatus      = (enum AlarmInputStatus)SysAlarmInCfg.s_NormalStatus;
		AlmInInfo.s_LinkAlarmStrategy = SysAlarmInCfg.s_LinkAlarmStrategy;
		AlmInInfo.s_CheckTime         = SysAlarmInCfg.s_CheckTime;
		AlmInInfo.s_LinkAlarmExtInfo.s_IoNum         = SysAlarmInCfg.s_LinkAlarmExtInfo.s_IoNum;
		AlmInInfo.s_LinkAlarmExtInfo.s_OperateCmd    = SysAlarmInCfg.s_LinkAlarmExtInfo.s_OperateCmd;
		AlmInInfo.s_LinkAlarmExtInfo.s_OperateSeqNum = SysAlarmInCfg.s_LinkAlarmExtInfo.s_OperateSeqNum;
		memcpy(AlmInInfo.s_Name, SysAlarmInCfg.s_Name, sizeof(AlmInInfo.s_Name));

		Result = m_EventCenter.ConfigureGPIOAlarmInput(&AlmInInfo, sizeof(struct AlarmInputInfo));
		if (FAILED(Result))
		{
			SYS_ERROR("ConfigureGPIOAlarmInput fail, Result = 0x%lx\n", Result);
			return Result;
		}		
		break;
	case SYS_DETECTOR_ID_PIR:
		SysPkgAlarmEventConfig SysAlarmPIRConfig;
		memset(&SysAlarmPIRConfig, 0, sizeof(SysPkgAlarmEventConfig));
		if (ParameterLength != sizeof(SysPkgAlarmEventConfig))
		{
			SYS_ERROR("ParameterLength is %d, but real length should be %d\n", ParameterLength, sizeof(SysPkgAlarmEventConfig));
			return GMI_INVALID_PARAMETER;
		}
		memcpy(&SysAlarmPIRConfig, Parameter, ParameterLength);

		struct AlarmEventConfigInfo AlarmEventConfig;
		memset(&AlarmEventConfig, 0, sizeof(struct AlarmEventConfigInfo));
		AlarmEventConfig.s_AlarmEventType      = (enum AlarmEventType)EVENT_DETECTOR_ID_HUMAN_DETECT;
		AlarmEventConfig.s_EnableFlag          = SysAlarmPIRConfig.s_EnableFlag;
		AlarmEventConfig.s_LinkAlarmStrategy   = SysAlarmPIRConfig.s_LinkAlarmStrategy;
		AlarmEventConfig.s_CheckTime           = SysAlarmPIRConfig.s_CheckTime;
		AlarmEventConfig.s_LinkAlarmExtInfo.s_IoNum         = SysAlarmPIRConfig.s_LinkAlarmExtInfo.s_IoNum;
		AlarmEventConfig.s_LinkAlarmExtInfo.s_OperateCmd    = SysAlarmPIRConfig.s_LinkAlarmExtInfo.s_OperateCmd;
		AlarmEventConfig.s_LinkAlarmExtInfo.s_OperateSeqNum = SysAlarmPIRConfig.s_LinkAlarmExtInfo.s_OperateSeqNum;
		AlarmEventConfig.s_ExtData.s_HumanDetectExInfo.s_MaxSensVal = SysAlarmPIRConfig.s_AlarmUnionExtData.s_PIRDetectInfo.s_Sensitive;
		Result = m_EventCenter.ConfigureAlarmEvent((enum AlarmEventType)EVENT_DETECTOR_ID_HUMAN_DETECT, &AlarmEventConfig, sizeof(struct AlarmEventConfigInfo));
		if (FAILED(Result))
		{
			SYS_ERROR("ConfigureAlarmEvent fail, Result = 0x%lx\n", Result);
			return Result;
		}	
		break;
	case SYS_PROCESSOR_ID_ALARM_OUTPUT:
		SysPkgAlarmOutConfig SysAlarmOutConfig;
		memset(&SysAlarmOutConfig, 0, sizeof(SysPkgAlarmOutConfig));
		if (ParameterLength != sizeof(SysPkgAlarmOutConfig))
		{
			SYS_ERROR("ParameterLength is %d, but real length should be %d\n", ParameterLength, sizeof(SysPkgAlarmOutConfig));
			return GMI_INVALID_PARAMETER;
		}
		memcpy(&SysAlarmOutConfig, Parameter, ParameterLength);

		struct AlarmOutputInfo AlarmOutInfo;
		memset(&AlarmOutInfo, 0, sizeof(struct AlarmOutputInfo));
		AlarmOutInfo.s_EnableFlag   = SysAlarmOutConfig.s_EnableFlag;
		AlarmOutInfo.s_OutputNumber = SysAlarmOutConfig.s_OutputNumber;
		AlarmOutInfo.s_NormalStatus = (enum AlarmOutputStatus)SysAlarmOutConfig.s_NormalStatus;
		AlarmOutInfo.s_DelayTime    = SysAlarmOutConfig.s_DelayTime;		
		memcpy(AlarmOutInfo.s_Name, SysAlarmOutConfig.s_Name, sizeof(AlarmOutInfo.s_Name));
		Result = m_EventCenter.ConfigureGPIOAlarmOutput(&AlarmOutInfo, sizeof(struct AlarmInputInfo));
		if (FAILED(Result))
		{
			SYS_ERROR("ConfigureGPIOAlarmOutput fail, Result = 0x%lx\n", Result);
			return Result;
		}		
		break;
	default:
		return GMI_NOT_SUPPORT;
	}

	return Result;
}


GMI_RESULT Alarm::Sechdule(int32_t ScheduleId, const void_t *Parameter, size_t ParameterLength)
{
	GMI_RESULT Result = GMI_SUCCESS;

	SysPkgAlarmScheduleTime SysAlarmScheduleTime;
	memset(&SysAlarmScheduleTime, 0, sizeof(SysPkgAlarmScheduleTime));
	if (ParameterLength != sizeof(SysPkgAlarmScheduleTime))
	{
		SYS_ERROR("ParameterLength is %d, but real length should be %d\n", ParameterLength, sizeof(SysPkgAlarmOutConfig));
		return GMI_INVALID_PARAMETER;
	}

	struct AlarmScheduleTimeInfo AlmScheduleTimeInfo;
	memset(&AlmScheduleTimeInfo, 0, sizeof(struct AlarmScheduleTimeInfo));
		
	switch (ScheduleId)
	{
	case SYS_SCHEDULE_TIME_ID_ALARM_IN:				
		AlmScheduleTimeInfo.s_ScheduleId   = SCHEDULE_TIME_ID_ALARM_IN;
		AlmScheduleTimeInfo.s_Index        = SysAlarmScheduleTime.s_Index;
		for (int32_t i = 0; i < 7; i++)
		{
			for (int32_t j = 0; j < MAX_SEG_TIME_PERDAY; j++)
			{
				AlmScheduleTimeInfo.s_ScheduleTime[i][j].s_StartTime = SysAlarmScheduleTime.s_ScheduleTime[i][j].s_StartTime;
				AlmScheduleTimeInfo.s_ScheduleTime[i][j].s_EndTime   = SysAlarmScheduleTime.s_ScheduleTime[i][j].s_EndTime;
			}
		}

		Result = m_EventCenter.ConfigureAlarmScheduleTime(SCHEDULE_TIME_ID_ALARM_IN, &AlmScheduleTimeInfo, sizeof(struct AlarmScheduleTimeInfo));
		if (FAILED(Result))
		{
			SYS_ERROR("ConfigureAlarmScheduleTime fail, Result = 0x%lx\n", Result);
			return Result;
		}	 
		break;
	case SYS_SCHEDULE_TIME_ID_PIR_DETECT:		
		AlmScheduleTimeInfo.s_ScheduleId   = SCHEDULE_TIME_ID_HUMAN_DETECT;
		AlmScheduleTimeInfo.s_Index        = SysAlarmScheduleTime.s_Index;
		for (int32_t i = 0; i < 7; i++)
		{
			for (int32_t j = 0; j < MAX_SEG_TIME_PERDAY; j++)
			{
				AlmScheduleTimeInfo.s_ScheduleTime[i][j].s_StartTime = SysAlarmScheduleTime.s_ScheduleTime[i][j].s_StartTime;
				AlmScheduleTimeInfo.s_ScheduleTime[i][j].s_EndTime   = SysAlarmScheduleTime.s_ScheduleTime[i][j].s_EndTime;
			}
		}

		Result = m_EventCenter.ConfigureAlarmScheduleTime(SCHEDULE_TIME_ID_HUMAN_DETECT, &AlmScheduleTimeInfo, sizeof(struct AlarmScheduleTimeInfo));
		if (FAILED(Result))
		{
			SYS_ERROR("ConfigureAlarmScheduleTime fail, Result = 0x%lx\n", Result);
			return Result;
		}	 
		break;
	default:
		return GMI_NOT_SUPPORT;
	}

	return Result;
}

