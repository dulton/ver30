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

	struct AlarmUploadInf AlmUploadInf;
	memcpy(&AlmUploadInf, Parameter, sizeof(struct AlarmUploadInf)); 
		
    switch (EventId)
    {
    case EVENT_DETECTOR_ID_ALARM_INPUT:    	   	
    	if (e_EventType_Start == Type)
    	{
    		char_t UserData[64] = {0};
    		sprintf(UserData, "GPIO Alarm On:Input Port%d Description %s\n", AlmUploadInf.s_ExtraInfo.s_IoNum, AlmUploadInf.s_Description);
    		USER_LOG(g_DefaultLogClient, SYS_LOGMAJOR_ALARM, SYS_LOGMINOR_ALRAM_IN, USER_NAME, strlen(USER_NAME), UserData, strlen(UserData));
    	}
    	else if (e_EventType_End == Type)
    	{
    		char_t UserData[64] = {0};
    		sprintf(UserData, "GPIO Alarm Off:Input Port%d Description %s\n", AlmUploadInf.s_ExtraInfo.s_IoNum, AlmUploadInf.s_Description);
    		USER_LOG(g_DefaultLogClient, SYS_LOGMAJOR_ALARM, SYS_LOGMINOR_ALRAM_IN, USER_NAME, strlen(USER_NAME), UserData, strlen(UserData));
    	}    	
    	break;
    case EVENT_DETECTOR_ID_HUMAN_DETECT:    	 	
    	if (e_EventType_Start == Type)
    	{
    		char_t UserData[64] = {0};
    		sprintf(UserData, "PIR Alarm On:Description %s\n", AlmUploadInf.s_Description);
    		USER_LOG(g_DefaultLogClient, SYS_LOGMAJOR_ALARM, SYS_LOGMINOR_PIR_ALARM_IN, USER_NAME, strlen(USER_NAME), UserData, strlen(UserData));
    	}
    	else if (e_EventType_End == Type)
    	{
    		char_t UserData[64] = {0};
    		sprintf(UserData, "PIR Alarm Off:Description %s\n", AlmUploadInf.s_Description);
    		USER_LOG(g_DefaultLogClient, SYS_LOGMAJOR_ALARM, SYS_LOGMINOR_PIR_ALARM_IN, USER_NAME, strlen(USER_NAME), UserData, strlen(UserData));
    	}    	
    	break;
    default:
    	break;
    }
    
    
	return;
}


GMI_RESULT Alarm::CheckConfig(int32_t AlarmId, int32_t Index, const void_t *Parameter, size_t ParameterLength)
{
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
		SYS_INFO("====Alarm In Config======\n");
		SYS_INFO("EnableFlag                    = %d\n", SysAlarmInCfg.s_EnableFlag);
		SYS_INFO("Index                      	= %d\n", Index);
		SYS_INFO("SysAlarmInCfg.s_NormalStatus  = %d\n", SysAlarmInCfg.s_NormalStatus);
		SYS_INFO("s_LinkAlarmStrategy           = 0x%x\n", SysAlarmInCfg.s_LinkAlarmStrategy);
		SYS_INFO("s_CheckTime                   = %d\n", SysAlarmInCfg.s_CheckTime);
		SYS_INFO("s_LinkAlarmExtInfo.s_IoNum    = 0x%x\n", SysAlarmInCfg.s_LinkAlarmExtInfo.s_IoNum);
		SYS_INFO("s_LinkAlarmExtInfo.s_OperateCmd    = %d\n", SysAlarmInCfg.s_LinkAlarmExtInfo.s_OperateCmd);
		SYS_INFO("s_LinkAlarmExtInfo.s_OperateSeqNum = %d\n", SysAlarmInCfg.s_LinkAlarmExtInfo.s_OperateSeqNum);	
		
		if ((uint32_t)Index != SysAlarmInCfg.s_InputNumber)
		{
			SYS_ERROR("index %d, but SysAlarmInCfg.s_InputNumber %d\n", Index, SysAlarmInCfg.s_InputNumber);
			return GMI_INVALID_PARAMETER;
		}

		if (SysAlarmInCfg.s_NormalStatus != 0 && SysAlarmInCfg.s_NormalStatus != 1)
		{
			SYS_ERROR("SysAlarmInCfg.s_NormalStatus %d error\n", SysAlarmInCfg.s_NormalStatus);
			return GMI_INVALID_PARAMETER;
		}		

		if (SysAlarmInCfg.s_LinkAlarmExtInfo.s_OperateCmd < 0 || SysAlarmInCfg.s_LinkAlarmExtInfo.s_OperateCmd > 3)
		{
			SYS_ERROR("SysAlarmInCfg.s_LinkAlarmExtInfo.s_OperateCmd %d error\n", SysAlarmInCfg.s_LinkAlarmExtInfo.s_OperateCmd);
			return GMI_INVALID_PARAMETER;
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

		SYS_INFO("====PIR Config======\n");
		SYS_INFO("s_AlarmId 					= %d\n", SysAlarmPIRConfig.s_AlarmId);
		SYS_INFO("EnableFlag                    = %d\n", SysAlarmPIRConfig.s_EnableFlag);		
		SYS_INFO("s_LinkAlarmStrategy           = 0x%x\n", SysAlarmPIRConfig.s_LinkAlarmStrategy);
		SYS_INFO("s_CheckTime                   = %d\n", SysAlarmPIRConfig.s_CheckTime);
		SYS_INFO("s_LinkAlarmExtInfo.s_IoNum    = 0x%x\n", SysAlarmPIRConfig.s_LinkAlarmExtInfo.s_IoNum);
		SYS_INFO("s_LinkAlarmExtInfo.s_OperateCmd    = %d\n", SysAlarmPIRConfig.s_LinkAlarmExtInfo.s_OperateCmd);
		SYS_INFO("s_LinkAlarmExtInfo.s_OperateSeqNum = %d\n", SysAlarmPIRConfig.s_LinkAlarmExtInfo.s_OperateSeqNum);
		SYS_INFO("s_LinkAlarmExtInfo.s_DelayTime = %d\n", SysAlarmPIRConfig.s_LinkAlarmExtInfo.s_DelayTime);				
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

		SYS_INFO("====Alarm Out Config======\n");		
		SYS_INFO("EnableFlag 					= %d\n", SysAlarmOutConfig.s_EnableFlag);		
		SYS_INFO("s_OutputNumber                = %d\n", SysAlarmOutConfig.s_OutputNumber);		
		SYS_INFO("s_NormalStatus                = %d\n", SysAlarmOutConfig.s_NormalStatus);
		SYS_INFO("s_DelayTime                   = %d\n", SysAlarmOutConfig.s_DelayTime);	
		if ((uint32_t)Index != SysAlarmOutConfig.s_OutputNumber)
		{
			SYS_ERROR("index %d, but SysAlarmOutConfig.s_OutputNumber %d\n", Index, SysAlarmOutConfig.s_OutputNumber);
			return GMI_INVALID_PARAMETER;
		}

		if (SysAlarmOutConfig.s_NormalStatus != 0 && SysAlarmOutConfig.s_NormalStatus != 1)
		{
			SYS_ERROR("SysAlarmOutConfig.s_NormalStatus %d error\n", SysAlarmOutConfig.s_NormalStatus);
			return GMI_INVALID_PARAMETER;
		}				
		break;
	default:
		return GMI_NOT_SUPPORT;
	}

	return GMI_SUCCESS;
}


GMI_RESULT Alarm::Config(int32_t AlarmId, int32_t Index, const void_t *Parameter, size_t ParameterLength)
{
	GMI_RESULT Result = GMI_SUCCESS;

	Result = CheckConfig(AlarmId, Index, Parameter, ParameterLength);
	if (FAILED(Result))
	{
		SYS_ERROR("check config fail, Result = 0x%lx\n", Result);
		return Result;
	}
	
	switch (AlarmId)
	{
	case SYS_DETECTOR_ID_ALARM_INPUT:
		SysPkgAlarmInConfig SysAlarmInCfg;
		memset(&SysAlarmInCfg, 0, sizeof(SysPkgAlarmInConfig));		
		memcpy(&SysAlarmInCfg, Parameter, ParameterLength);

		struct AlarmInputInfo AlmInInfo;
		memset(&AlmInInfo, 0, sizeof(struct AlarmInputInfo));
		AlmInInfo.s_EnableFlag        = SysAlarmInCfg.s_EnableFlag;
		AlmInInfo.s_InputNumber       = Index;
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
		memcpy(&SysAlarmPIRConfig, Parameter, ParameterLength);

		struct AlarmEventConfigInfo AlarmEventConfig;
		memset(&AlarmEventConfig, 0, sizeof(struct AlarmEventConfigInfo));
		AlarmEventConfig.s_AlarmEventType      = EVENT_DETECTOR_ID_HUMAN_DETECT;
		AlarmEventConfig.s_EnableFlag          = SysAlarmPIRConfig.s_EnableFlag;
		AlarmEventConfig.s_LinkAlarmStrategy   = SysAlarmPIRConfig.s_LinkAlarmStrategy;
		AlarmEventConfig.s_CheckTime           = SysAlarmPIRConfig.s_CheckTime;
		AlarmEventConfig.s_LinkAlarmExtInfo.s_IoNum         = SysAlarmPIRConfig.s_LinkAlarmExtInfo.s_IoNum;
		AlarmEventConfig.s_LinkAlarmExtInfo.s_OperateCmd    = SysAlarmPIRConfig.s_LinkAlarmExtInfo.s_OperateCmd;
		AlarmEventConfig.s_LinkAlarmExtInfo.s_OperateSeqNum = SysAlarmPIRConfig.s_LinkAlarmExtInfo.s_OperateSeqNum;
		AlarmEventConfig.s_ExtData.s_HumanDetectExInfo.s_MaxSensVal = SysAlarmPIRConfig.s_AlarmUnionExtData.s_PIRDetectInfo.s_Sensitive;
		Result = m_EventCenter.ConfigureAlarmEvent(EVENT_DETECTOR_ID_HUMAN_DETECT, &AlarmEventConfig, sizeof(struct AlarmEventConfigInfo));
		if (FAILED(Result))
		{
			SYS_ERROR("ConfigureAlarmEvent fail, Result = 0x%lx\n", Result);
			return Result;
		}	
		break;
	case SYS_PROCESSOR_ID_ALARM_OUTPUT:
		SysPkgAlarmOutConfig SysAlarmOutConfig;
		memset(&SysAlarmOutConfig, 0, sizeof(SysPkgAlarmOutConfig));		
		memcpy(&SysAlarmOutConfig, Parameter, ParameterLength);

		struct AlarmOutputInfo AlarmOutInfo;
		memset(&AlarmOutInfo, 0, sizeof(struct AlarmOutputInfo));
		AlarmOutInfo.s_EnableFlag   = SysAlarmOutConfig.s_EnableFlag;
		AlarmOutInfo.s_OutputNumber = Index;
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


GMI_RESULT Alarm::CheckSchedule(int32_t ScheduleId, int32_t Index, const void_t *Parameter, size_t ParameterLength)
{
	SysPkgAlarmScheduleTime SysAlarmScheduleTime;	
	if (ParameterLength != sizeof(SysPkgAlarmScheduleTime))
	{
		SYS_ERROR("ParameterLength is %d, but real length should be %d\n", ParameterLength, sizeof(SysPkgAlarmScheduleTime));
		return GMI_INVALID_PARAMETER;
	}
	memcpy(&SysAlarmScheduleTime, Parameter, ParameterLength);

	SYS_INFO("======Schedule Config====\n");
	SYS_INFO("s_ScheduleId = %d\n", SysAlarmScheduleTime.s_ScheduleId);
	SYS_INFO("s_Index      = %d\n", SysAlarmScheduleTime.s_Index);	
	for (int32_t i = 0; i < DAYS_OF_WEEK; i++)
	{
		for (int32_t j = 0; j < TIME_SEGMENT_OF_DAY; j++)
		{
			SYS_INFO("s_StartTime[%d][%d]  = %d\n", i, j, SysAlarmScheduleTime.s_ScheduleTime[i][j].s_StartTime);
			SYS_INFO("s_EndTime[%d][%d]    = %d\n", i, j, SysAlarmScheduleTime.s_ScheduleTime[i][j].s_EndTime);
		}
	}

	if ((uint32_t)Index != SysAlarmScheduleTime.s_Index)
	{
		SYS_ERROR("Index %d, but %d\n", Index, SysAlarmScheduleTime.s_Index);
		return GMI_INVALID_PARAMETER;
	}

	if ((uint32_t)ScheduleId != SysAlarmScheduleTime.s_ScheduleId)
	{
		SYS_ERROR("scheduleId %d, but %d\n", ScheduleId, SysAlarmScheduleTime.s_ScheduleId);
		return GMI_INVALID_PARAMETER;
	}

	for (int32_t i = 0; i < DAYS_OF_WEEK; i++)
	{
		for (int32_t j = 0; j < TIME_SEGMENT_OF_DAY; j++)
		{
			if (SysAlarmScheduleTime.s_ScheduleTime[i][j].s_EndTime > (24*60))
			{
				SYS_ERROR("s_EndTime[%d][%d] = %d error, should less than 24*60\n", i, j, SysAlarmScheduleTime.s_ScheduleTime[i][j].s_EndTime);
				return GMI_INVALID_PARAMETER;
			}

			if (SysAlarmScheduleTime.s_ScheduleTime[i][j].s_EndTime < SysAlarmScheduleTime.s_ScheduleTime[i][j].s_StartTime)
			{
				SYS_ERROR("s_EndTime[%d][%d] = %d is less than s_StartTime[%d][%d]  = %d\n", i, j, SysAlarmScheduleTime.s_ScheduleTime[i][j].s_EndTime, i, j, SysAlarmScheduleTime.s_ScheduleTime[i][j].s_StartTime);
				return GMI_INVALID_PARAMETER;
			}
		}
	}

	return GMI_SUCCESS;
}


GMI_RESULT Alarm::Schedule(int32_t ScheduleId, int32_t Index, const void_t *Parameter, size_t ParameterLength)
{
	GMI_RESULT Result = GMI_SUCCESS;

	Result = CheckSchedule(ScheduleId, Index, Parameter, ParameterLength);
	if (FAILED(Result))
	{
		SYS_ERROR("CheckSchedule fail, Result = 0x%lx\n", Result);
		return Result;
	}
	
	SysPkgAlarmScheduleTime SysAlarmScheduleTime;	
	memcpy(&SysAlarmScheduleTime, Parameter, ParameterLength);
	
	struct AlarmScheduleTimeInfo AlmScheduleTimeInfo;
	memset(&AlmScheduleTimeInfo, 0, sizeof(struct AlarmScheduleTimeInfo));
		
	switch (ScheduleId)
	{
	case SYS_SCHEDULE_TIME_ID_ALARM_IN:				
		AlmScheduleTimeInfo.s_ScheduleId   = SCHEDULE_TIME_ID_ALARM_IN;
		AlmScheduleTimeInfo.s_Index        = Index;
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
			SYS_ERROR("ConfigureAlarmInScheduleTime fail, Result = 0x%lx\n", Result);
			return Result;
		}	 
		break;
	case SYS_SCHEDULE_TIME_ID_PIR_DETECT:		
		AlmScheduleTimeInfo.s_ScheduleId   = SCHEDULE_TIME_ID_HUMAN_DETECT;
		AlmScheduleTimeInfo.s_Index        = Index;
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
			SYS_ERROR("ConfigureAlarmPIRScheduleTime fail, Result = 0x%lx\n", Result);
			return Result;
		}	 
		break;
	case SYS_SCHEDULE_TIME_ID_ALARM_OUT:
		AlmScheduleTimeInfo.s_ScheduleId   = SCHEDULE_TIME_ID_ALARM_OUT;
		AlmScheduleTimeInfo.s_Index        = Index;
		for (int32_t i = 0; i < 7; i++)
		{
			for (int32_t j = 0; j < MAX_SEG_TIME_PERDAY; j++)
			{
				AlmScheduleTimeInfo.s_ScheduleTime[i][j].s_StartTime = SysAlarmScheduleTime.s_ScheduleTime[i][j].s_StartTime;
				AlmScheduleTimeInfo.s_ScheduleTime[i][j].s_EndTime   = SysAlarmScheduleTime.s_ScheduleTime[i][j].s_EndTime;
			}
		}

		Result = m_EventCenter.ConfigureAlarmScheduleTime(SCHEDULE_TIME_ID_ALARM_OUT, &AlmScheduleTimeInfo, sizeof(struct AlarmScheduleTimeInfo));
		if (FAILED(Result))
		{
			SYS_ERROR("ConfigureAlarmOutScheduleTime fail, Result = 0x%lx\n", Result);
			return Result;
		}	 
		break;
	default:
		return GMI_NOT_SUPPORT;
	}

	return Result;
}

