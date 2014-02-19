#include "ipc_fw_v3.x_setting.h"
#include "ipc_fw_v3.x_resource.h"
#include "user_log_query.h"
#include "log.h"


UserLogQuery::UserLogQuery(void)
{
}


UserLogQuery::~UserLogQuery(void)
{
}


GMI_RESULT UserLogQuery::Initialize(void)
{
	m_UserLogPtr = NULL;
	
	UserLogQueryerInitializationParameter Parameter;
	Parameter.s_SourceType                 = USER_LOG_SOURCE_TYPE_FILE_PATH_REFERENCE;
    Parameter.s_Source.u_FilePathReference = GMI_LOG_DEFAULT_USER_LOG_FILE_PATH;
    Parameter.s_SourceIpcMutexKey          = GMI_USER_LOG_IPC_MUTEX_KEY;
	
	GMI_RESULT Result = m_UserLogQueryer.Initialize(&Parameter, sizeof(UserLogQueryerInitializationParameter));
    if (FAILED(Result))
    {
        SYS_ERROR( "UserLogQueryer initializing failed,Result = 0x%lx\n", Result );
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT UserLogQuery::Deinitialize(void)
{
	m_UserLogPtr  = NULL;
	
	GMI_RESULT Result = m_UserLogQueryer.Deinitialize();
    if (FAILED(Result))
    {
        printf( "UserLogQueryer deinitializing fail, Result = 0x%lx\n", Result);
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT UserLogQuery::CheckTime(struct tm* ptTime)
{
    struct tm tTime;

    if (ptTime == NULL)
    {
        SYS_ERROR("Input param is null.\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Input param is null.\n");
        return GMI_FAIL;
    }

    int32_t Day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    memcpy(&tTime,ptTime,sizeof(struct tm));
    tTime.tm_year += 1900;
    tTime.tm_mon  += 1;

    if (tTime.tm_year < 2000 || tTime.tm_year > 2037)
    {
        return GMI_FAIL;
    }

    if (tTime.tm_mon < 1 || tTime.tm_mon > 12)
    {
        return GMI_FAIL;
    }

    if (tTime.tm_mon == 2 && (tTime.tm_year & 3) == 0)
    {
        Day[tTime.tm_mon-1] = 29;
    }

    if (tTime.tm_mday < 1 || tTime.tm_mday > Day[tTime.tm_mon-1])
    {
        return GMI_FAIL;
    }

    if (tTime.tm_hour < 0 || tTime.tm_hour > 23)
    {
        return GMI_FAIL;
    }

    if (tTime.tm_min < 0 || tTime.tm_min > 59)
    {
        return GMI_FAIL;
    }

    if (tTime.tm_sec < 0 || tTime.tm_sec > 59)
    {
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}


GMI_RESULT UserLogQuery::SubQuery(int32_t MajorType, int32_t MinorType, char_t StartTim[32], char_t EndTim[32])
{
	SYS_INFO("in %s.......\n", __func__);
	uint16_t Type;
	uint16_t SubType;
	uint64_t StartTime;
	uint64_t EndTime;
	struct tm STime;
	struct tm ETime;
	struct timeval Tv;
	
	Type      = (uint16_t)MajorType;
	SubType   = (uint16_t)MinorType;
	sscanf(StartTim, "%d-%d-%d %d:%d:%d", &STime.tm_year, &STime.tm_mon, &STime.tm_mday, &STime.tm_hour, &STime.tm_min, &STime.tm_sec);
	sscanf(EndTim, "%d-%d-%d %d:%d:%d", &ETime.tm_year, &ETime.tm_mon, &ETime.tm_mday, &ETime.tm_hour, &ETime.tm_min, &ETime.tm_sec);
	STime.tm_year -= 1900;
	STime.tm_mon  -= 1;
	ETime.tm_year -= 1900;
	ETime.tm_mon  -= 1;

	GMI_RESULT Result = CheckTime(&STime);
	if (FAILED(Result))
	{
		SYS_ERROR("check time fail, Result = 0x%lx\n", Result);
		return Result;
	}

	Result = CheckTime(&ETime);
	if (FAILED(Result))
	{
		SYS_ERROR("check time fail, Result = 0x%lx\n", Result);
		return Result;
	}
	
	Tv.tv_sec  = mktime(&STime);
	Tv.tv_usec = 0;
	StartTime  = (uint64_t)Tv.tv_sec << 32 | Tv.tv_usec;
	Tv.tv_sec  = mktime(&ETime);
	Tv.tv_usec = 0;
	EndTime    = (uint64_t)Tv.tv_sec << 32 | Tv.tv_usec;

	//param 
	SYS_INFO("Type     %d\n", Type);
	SYS_INFO("SubType  %d\n", SubType);
	SYS_INFO("StartTim %s\n", StartTim);
	SYS_INFO("EndTim   %s\n", EndTim);
	
	if (NULL != m_UserLogPtr)
	{
		BaseMemoryManager::Instance().Deletes<UserLogStorageInfo>(m_UserLogPtr);
		m_UserLogPtr = NULL;
	}
	
	//try get user log
    UserLogStorageInfo UserLog;           
    uint32_t UserLogNumber = 1;    
    Result = m_UserLogQueryer.Query(Type, SubType, StartTime, EndTime, &UserLog, &UserLogNumber);
    //space not enough, then allocate more space to get all user log.
    if (GMI_NOT_ENOUGH_SPACE == Result)
    {        
        SYS_INFO("Query fail because not enough space, so allocate more space to get more log, Result = 0x%lx\n", Result);
        SYS_INFO("UserLogNumber %d\n", UserLogNumber);
        m_UserLogPtr = BaseMemoryManager::Instance().News<UserLogStorageInfo>(UserLogNumber);
        if (NULL == m_UserLogPtr)
        {
        	SYS_ERROR("allocate user log fail\n");
        	return GMI_OUT_OF_MEMORY;
        }

        Result = m_UserLogQueryer.Query(Type, SubType, StartTime, EndTime, m_UserLogPtr, &UserLogNumber);
        if (FAILED(Result))
        {
        	BaseMemoryManager::Instance().Deletes<UserLogStorageInfo>(m_UserLogPtr);
        	m_UserLogPtr = NULL;
        	SYS_ERROR("Query user log fail, Result");
        	return Result;	
        }   

		m_TotalUserLogNum = UserLogNumber;
		m_LeftUserLogNum = m_TotalUserLogNum;
		SYS_INFO("out %s.......\n", __func__);
        return GMI_SUCCESS;
    }
    //fail
    else if (FAILED(Result))
    {
    	SYS_ERROR("Query fail, Result = 0x%lx\n", Result);
    	SYS_ERROR("abnormal out %s.......\n", __func__);
    	return Result;
    }
    //get single log
    else
    {
    	if (0 < UserLogNumber)
    	{
    		m_UserLogPtr = BaseMemoryManager::Instance().News<UserLogStorageInfo>(UserLogNumber);
	        if (NULL == m_UserLogPtr)
	        {
	        	SYS_INFO("allocate user log fail\n");
	        	return GMI_OUT_OF_MEMORY;
	        }
	        memcpy(m_UserLogPtr, &UserLog, sizeof(UserLogStorageInfo));	
    	}    	
    	m_TotalUserLogNum = UserLogNumber;
    	m_LeftUserLogNum = m_TotalUserLogNum;
    	SYS_INFO("out %s.......\n", __func__);
    	return GMI_SUCCESS;
    }    
}


GMI_RESULT UserLogQuery::Query(SysPkgLogInfoSearch *SysLogInfoSearch, SysPkgLogInfoInt *SyLogInfoInt, SysPkgLogInfo SysLogInfo[])
{
	SYS_INFO("in %s.......\n", __func__);
	if (NULL == SysLogInfoSearch
		|| NULL == SyLogInfoInt
		|| NULL == SysLogInfo)
	{
		return GMI_INVALID_PARAMETER;		
	}

	SysPkgLogInfoSearch LogInfoSearch;
	memcpy(&LogInfoSearch, SysLogInfoSearch, sizeof(SysPkgLogInfoSearch));	
	//query again
	if (LogInfoSearch.s_SelectMode != SysLogInfoSearch_Last.s_SelectMode
	    || LogInfoSearch.s_MajorType != SysLogInfoSearch_Last.s_MajorType
		|| LogInfoSearch.s_MinorType != SysLogInfoSearch_Last.s_MinorType
		|| 0 != strcmp(LogInfoSearch.s_StartTime, SysLogInfoSearch_Last.s_StartTime)
		|| 0 != strcmp(LogInfoSearch.s_StopTime, SysLogInfoSearch_Last.s_StopTime))
	{
		GMI_RESULT Result = SubQuery(LogInfoSearch.s_MajorType, LogInfoSearch.s_MinorType, LogInfoSearch.s_StartTime, LogInfoSearch.s_StopTime);
		if (FAILED(Result))
		{
			SYS_ERROR("Query fail, Result = 0x%lx\n", Result);
			return Result;
		}		
	}

	SyLogInfoInt->s_Total = m_TotalUserLogNum;
	SyLogInfoInt->s_Count = (uint32_t)LogInfoSearch.s_MaxNum > m_LeftUserLogNum ? m_LeftUserLogNum : LogInfoSearch.s_MaxNum; 	
	uint32_t Offset = (uint32_t)LogInfoSearch.s_Offset;
	if (Offset > m_TotalUserLogNum)
	{
		SYS_ERROR("Offset %d, m_TotalUserLogNum %d\n", Offset, m_TotalUserLogNum);
		return GMI_INVALID_PARAMETER;
	}
	
	if (m_LeftUserLogNum > 0)
	{		
		for (int32_t i = 0; i < SyLogInfoInt->s_Count; i++)
		{
			memset(&SysLogInfo[i], 0, sizeof(SysPkgLogInfo));
			SysLogInfo[i].s_LogId     = m_UserLogPtr[Offset].s_Index;
			SysLogInfo[i].s_MajorType = m_UserLogPtr[Offset].s_Type;
			SysLogInfo[i].s_MinorType = m_UserLogPtr[Offset].s_Subtype;
			time_t Sec;
			Sec = m_UserLogPtr[Offset].s_LogTime >> 32;
			struct tm *LocalTime = localtime(&Sec);
			sprintf(SysLogInfo[i].s_LogTime, "%d-%d-%d %d:%d:%d", \
				LocalTime->tm_year+1900, LocalTime->tm_mon+1, LocalTime->tm_mday, LocalTime->tm_hour, LocalTime->tm_min, LocalTime->tm_sec);
			strcpy(SysLogInfo[i].s_UserName, m_UserLogPtr[Offset].s_UserName);
			strcpy(SysLogInfo[i].s_LogData, (char_t*)m_UserLogPtr[Offset].s_SpecificData);
			Offset++;
		}

		m_LeftUserLogNum -= SyLogInfoInt->s_Count;
	}		
	SYS_INFO("out %s.......\n", __func__);
	return GMI_SUCCESS;
}

