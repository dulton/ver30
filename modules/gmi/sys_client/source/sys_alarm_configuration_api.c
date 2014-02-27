#include "log.h"
#include "sys_client.h"
#include "sys_command_excute.h"
#include "sys_env_types.h"
#include "gmi_system_headers.h"


GMI_RESULT SysGetAlarmConfig(uint16_t SessionId, uint32_t AuthValue, int32_t AlarmId, int32_t Index, void_t *Parameter, uint32_t ParameterLength)
{
    GMI_RESULT   Result = GMI_SUCCESS;  
    SysAttr      SysReqAttr        = {0};
    SysAttr      *SysRspAttrPtr    = NULL;
    SysAttr      *SysRspAttrTmpPtr = NULL;
    uint16_t     RspAttrCnt        = 0;
    uint16_t     ReqAttrCnt        = 1;
    boolean_t    Exist             = false;
    SysPkgGetAlarmConfig SysGetAlarmConfig;

    if (NULL == Parameter)
    {
       return GMI_INVALID_PARAMETER;
    }

    do
    {            
        memset(&SysGetAlarmConfig, 0, sizeof(SysPkgGetAlarmConfig));
        SysGetAlarmConfig.s_AlarmId = htonl(AlarmId);
        SysGetAlarmConfig.s_Index   = htonl(Index);
        SysReqAttr.s_Type       = TYPE_GET_ALMCONFIG;
        SysReqAttr.s_Attr       = (void_t*)&SysGetAlarmConfig;
        SysReqAttr.s_AttrLength = sizeof(SysPkgGetAlarmConfig);
        Result = SysGetCmdExcuteWithAttrs(SessionId, AuthValue, SYSCODE_GET_ALMCFG_REQ, ReqAttrCnt, &SysReqAttr, &RspAttrCnt, &SysRspAttrPtr);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SysGetCmdExcuteWithAttrs fail, Result = 0x%lx\n", Result);
            break;
        }

        if (RspAttrCnt <= 0
            || SysRspAttrPtr == NULL)
        {
            SYS_CLIENT_ERROR("RspAttrCnt %d incorrect or SysRspAttrPtr is Null\n", RspAttrCnt);
            break;
        }
        
        SysRspAttrTmpPtr = SysRspAttrPtr;
        switch (AlarmId)
        {
        case SYS_DETECTOR_ID_ALARM_INPUT:
            if (SysRspAttrTmpPtr->s_Type == TYPE_ALARM_IN
                && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgAlarmInConfig))
            {
                SysPkgAlarmInConfig SysAlarmInConfig;                
                memcpy(&SysAlarmInConfig, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
                SysAlarmInConfig.s_EnableFlag   = ntohl(SysAlarmInConfig.s_EnableFlag);
	    	    SysAlarmInConfig.s_InputNumber  = ntohl(SysAlarmInConfig.s_InputNumber);
	    	    SysAlarmInConfig.s_CheckTime    = ntohl(SysAlarmInConfig.s_CheckTime);
	    	    SysAlarmInConfig.s_NormalStatus = ntohl(SysAlarmInConfig.s_NormalStatus);
	    	    SysAlarmInConfig.s_LinkAlarmStrategy = ntohl(SysAlarmInConfig.s_LinkAlarmStrategy);
	    	    SysAlarmInConfig.s_LinkAlarmExtInfo.s_OperateSeqNum = ntohl(SysAlarmInConfig.s_LinkAlarmExtInfo.s_OperateSeqNum);	    	
	    	    memcpy(Parameter, &SysAlarmInConfig, sizeof(SysPkgAlarmInConfig));
	    	    Exist = true;
            }
            break;
        case SYS_DETECTOR_ID_PIR:
            if (SysRspAttrTmpPtr->s_Type == TYPE_ALARM_EVENT
                && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgAlarmEventConfig))
            {
                SysPkgAlarmEventConfig SysAlarmEventConfig;
                memcpy(&SysAlarmEventConfig, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
                SysAlarmEventConfig.s_AlarmId = ntohl(SysAlarmEventConfig.s_AlarmId);
			    SysAlarmEventConfig.s_EnableFlag = ntohl(SysAlarmEventConfig.s_EnableFlag);
			    SysAlarmEventConfig.s_CheckTime  = ntohl(SysAlarmEventConfig.s_CheckTime);
			    SysAlarmEventConfig.s_LinkAlarmStrategy = ntohl(SysAlarmEventConfig.s_LinkAlarmStrategy);
			    SysAlarmEventConfig.s_AlarmUnionExtData.s_PIRDetectInfo.s_Sensitive = ntohl(SysAlarmEventConfig.s_AlarmUnionExtData.s_PIRDetectInfo.s_Sensitive);	    	
			    SysAlarmEventConfig.s_LinkAlarmExtInfo.s_OperateSeqNum = ntohl(SysAlarmEventConfig.s_LinkAlarmExtInfo.s_OperateSeqNum);    			    				                
			    memcpy(Parameter, &SysAlarmEventConfig, sizeof(SysPkgAlarmEventConfig));
			    Exist = true;
            }
            break;
        case SYS_PROCESSOR_ID_ALARM_OUTPUT:
            if (SysRspAttrTmpPtr->s_Type == TYPE_ALARM_OUT
                && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgAlarmOutConfig))
            {
                SysPkgAlarmOutConfig SysAlarmOutConfig;
                memcpy(&SysAlarmOutConfig, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
                SysAlarmOutConfig.s_EnableFlag   = ntohl(SysAlarmOutConfig.s_EnableFlag);
	    	    SysAlarmOutConfig.s_OutputNumber = ntohl(SysAlarmOutConfig.s_OutputNumber);
	    	    SysAlarmOutConfig.s_NormalStatus = ntohl(SysAlarmOutConfig.s_NormalStatus);
	    	    SysAlarmOutConfig.s_DelayTime    = ntohl(SysAlarmOutConfig.s_DelayTime);  
	    	    memcpy(Parameter, &SysAlarmOutConfig, sizeof(SysPkgAlarmEventConfig));  
	    	    Exist = true;            
            }
            break;
        default:
            break;
        }
            
        if (!Exist)
        {
            SYS_CLIENT_ERROR("not found valid data\n");
            break;
        }

        SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
        return GMI_SUCCESS;
    }
    while (0);

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysSetAlarmConfig(uint16_t SessionId, uint32_t AuthValue, int32_t AlarmId, int32_t Index, const void_t *Parameter, uint32_t ParameterLength)
{
    GMI_RESULT Result     = GMI_SUCCESS;
    uint16_t   ReqAttrCnt = 1;
    SysAttr    SysReqAttr = {0};  
    SysPkgAlarmInConfig SysAlarmInConfig;
    SysPkgAlarmOutConfig SysAlarmOutConfig;
    SysPkgAlarmEventConfig SysAlarmEventConfig;    

    do
    {
        switch (AlarmId)
        {
        case SYS_DETECTOR_ID_ALARM_INPUT:
            memcpy(&SysAlarmInConfig, Parameter, sizeof(SysPkgAlarmInConfig));
            SysAlarmInConfig.s_EnableFlag   = htonl(SysAlarmInConfig.s_EnableFlag);
	    	SysAlarmInConfig.s_InputNumber  = htonl(Index);
	    	SysAlarmInConfig.s_CheckTime    = htonl(SysAlarmInConfig.s_CheckTime);
	    	SysAlarmInConfig.s_NormalStatus = htonl(SysAlarmInConfig.s_NormalStatus);
	    	SysAlarmInConfig.s_LinkAlarmStrategy = htonl(SysAlarmInConfig.s_LinkAlarmStrategy);
	    	SysAlarmInConfig.s_LinkAlarmExtInfo.s_OperateSeqNum = htonl(SysAlarmInConfig.s_LinkAlarmExtInfo.s_OperateSeqNum);	    		    	
	    	
	    	SysReqAttr.s_Type = TYPE_ALARM_IN;
            SysReqAttr.s_Attr = (void_t*)&SysAlarmInConfig;
            SysReqAttr.s_AttrLength = sizeof(SysPkgAlarmInConfig);
            break;
        case SYS_DETECTOR_ID_PIR:
            memcpy(&SysAlarmEventConfig, Parameter, sizeof(SysPkgAlarmEventConfig));
            SysAlarmEventConfig.s_AlarmId    = htonl(SysAlarmEventConfig.s_AlarmId);
			SysAlarmEventConfig.s_EnableFlag = htonl(SysAlarmEventConfig.s_EnableFlag);
			SysAlarmEventConfig.s_CheckTime  = htonl(SysAlarmEventConfig.s_CheckTime);
			SysAlarmEventConfig.s_LinkAlarmStrategy = htonl(SysAlarmEventConfig.s_LinkAlarmStrategy);
			SysAlarmEventConfig.s_AlarmUnionExtData.s_PIRDetectInfo.s_Sensitive = htonl(SysAlarmEventConfig.s_AlarmUnionExtData.s_PIRDetectInfo.s_Sensitive);	    	
			SysAlarmEventConfig.s_LinkAlarmExtInfo.s_OperateSeqNum = htonl(SysAlarmEventConfig.s_LinkAlarmExtInfo.s_OperateSeqNum);    			    							
			
			SysReqAttr.s_Type = TYPE_ALARM_EVENT;
            SysReqAttr.s_Attr = (void_t*)&SysAlarmEventConfig;
            SysReqAttr.s_AttrLength = sizeof(SysPkgAlarmEventConfig);
            break;
        case SYS_PROCESSOR_ID_ALARM_OUTPUT:
            memcpy(&SysAlarmOutConfig, Parameter, sizeof(SysPkgAlarmOutConfig));
            SysAlarmOutConfig.s_EnableFlag   = htonl(SysAlarmOutConfig.s_EnableFlag);
	    	SysAlarmOutConfig.s_OutputNumber = htonl(Index);
	    	SysAlarmOutConfig.s_NormalStatus = htonl(SysAlarmOutConfig.s_NormalStatus);
	    	SysAlarmOutConfig.s_DelayTime    = htonl(SysAlarmOutConfig.s_DelayTime);
	    	
	    	SysReqAttr.s_Type = TYPE_ALARM_OUT;
            SysReqAttr.s_Attr = (void_t*)&SysAlarmOutConfig;
            SysReqAttr.s_AttrLength = sizeof(SysPkgAlarmOutConfig);
            break;
        default:
            return GMI_NOT_SUPPORT;
        }     
  
        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_ALMCFG_REQ, ReqAttrCnt, &SysReqAttr );
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("set video source fail, Result = 0x%lx\n", Result);
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return GMI_FAIL;
}


GMI_RESULT SysGetAlmScheduleTime(uint16_t SessionId, uint32_t AuthValue, int32_t ScheduleId, int32_t Index, SysPkgAlarmScheduleTime *SysAlarmSchTime)
{
    GMI_RESULT   Result = GMI_SUCCESS;  
    SysAttr      SysReqAttr        = {0};
    SysAttr      *SysRspAttrPtr    = NULL;
    SysAttr      *SysRspAttrTmpPtr = NULL;
    uint16_t     RspAttrCnt        = 0;
    uint16_t     ReqAttrCnt        = 1;
    boolean_t    Exist             = false;    
    SysPkgGetAlarmScheduleTime GetAlarmScheduleTime = {0}; 
    SysPkgAlarmScheduleTime    SysAlarmScheduleTime = {0};         


    do
    {            
        memset(&GetAlarmScheduleTime, 0, sizeof(SysPkgGetAlarmScheduleTime));      
        GetAlarmScheduleTime.s_ScheduleId = htonl(ScheduleId);
		GetAlarmScheduleTime.s_Index      = htonl(Index);      
        SysReqAttr.s_Type       = TYPE_GET_ALMDEPLOY;
        SysReqAttr.s_Attr       = (void_t*)&GetAlarmScheduleTime;
        SysReqAttr.s_AttrLength = sizeof(SysPkgGetAlarmScheduleTime);
        Result = SysGetCmdExcuteWithAttrs(SessionId, AuthValue, SYSCODE_GET_ALMDEPLOY_REQ, ReqAttrCnt, &SysReqAttr, &RspAttrCnt, &SysRspAttrPtr);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SysGetCmdExcuteWithAttrs fail, Result = 0x%lx\n", Result);
            break;
        }

        if (RspAttrCnt <= 0
            || SysRspAttrPtr == NULL)
        {
            SYS_CLIENT_ERROR("RspAttrCnt %d incorrect or SysRspAttrPtr is Null\n", RspAttrCnt);
            break;
        }
        
        SysRspAttrTmpPtr = SysRspAttrPtr;
        if (SysRspAttrTmpPtr->s_Type == TYPE_ALMDEPLOY
                && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgAlarmScheduleTime))
        {
            memcpy(&SysAlarmScheduleTime, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
            SysAlarmScheduleTime.s_ScheduleId = ntohl(SysAlarmScheduleTime.s_ScheduleId);
    		SysAlarmScheduleTime.s_Index      = ntohl(SysAlarmScheduleTime.s_Index);
    		for (int32_t i = 0; i < DAYS_OF_WEEK; i++)
    		{
    			for (int32_t j = 0; j < TIME_SEGMENT_OF_DAY; j++)
    			{
    				SysAlarmScheduleTime.s_ScheduleTime[i][j].s_StartTime = ntohl(SysAlarmScheduleTime.s_ScheduleTime[i][j].s_StartTime);
    				SysAlarmScheduleTime.s_ScheduleTime[i][j].s_EndTime   = ntohl(SysAlarmScheduleTime.s_ScheduleTime[i][j].s_EndTime);
    			}
    		}
    		
    		memcpy(SysAlarmSchTime, &SysAlarmScheduleTime, sizeof(SysPkgAlarmScheduleTime)); 
    		Exist = true;
        }                
            
        if (!Exist)
        {
            SYS_CLIENT_ERROR("not found valid data\n");
            break;
        }

        SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
        return GMI_SUCCESS;
    }
    while (0);

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysSetAlmScheduleTime(uint16_t SessionId, uint32_t AuthValue, int32_t ScheduleId, int32_t Index, SysPkgAlarmScheduleTime *SysAlarmSchTime)
{
    GMI_RESULT Result     = GMI_SUCCESS;
    uint16_t   ReqAttrCnt = 1;
    SysAttr    SysReqAttr = {0};  
    SysPkgAlarmScheduleTime SysAlarmScheduleTime;      

    do
    {        
         memcpy(&SysAlarmScheduleTime, SysAlarmSchTime, sizeof(SysPkgAlarmScheduleTime));
         if (SysAlarmScheduleTime.s_ScheduleId != (uint32_t)ScheduleId
            || SysAlarmScheduleTime.s_Index != (uint32_t)Index)
         {
            SYS_CLIENT_ERROR("invalid parameter\n");
            return GMI_INVALID_PARAMETER;
         }
         
         SysAlarmScheduleTime.s_ScheduleId = ntohl(ScheduleId);
		 SysAlarmScheduleTime.s_Index  = ntohl(Index);
		 for (int32_t i = 0; i < DAYS_OF_WEEK; i++)
		 {
		    for (int32_t j = 0; j < TIME_SEGMENT_OF_DAY; j++)
		    {
		        SysAlarmScheduleTime.s_ScheduleTime[i][j].s_StartTime = ntohl(SysAlarmScheduleTime.s_ScheduleTime[i][j].s_StartTime);
		        SysAlarmScheduleTime.s_ScheduleTime[i][j].s_EndTime   = ntohl(SysAlarmScheduleTime.s_ScheduleTime[i][j].s_EndTime);
		    }
		 }    	
		 
		SysReqAttr.s_Type = TYPE_ALMDEPLOY;
        SysReqAttr.s_Attr = (void_t*)&SysAlarmScheduleTime;
        SysReqAttr.s_AttrLength = sizeof(SysPkgAlarmScheduleTime);
         
        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_ALMDEPLOY_REQ, ReqAttrCnt, &SysReqAttr );
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("set video source fail, Result = 0x%lx\n", Result);
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return GMI_FAIL;  
}



