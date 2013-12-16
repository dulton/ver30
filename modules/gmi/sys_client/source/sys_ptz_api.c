#include "log.h"
#include "sys_client.h"
#include "sys_command_excute.h"
#include "sys_env_types.h"
#include "gmi_system_headers.h"

GMI_RESULT SysPtzControl(uint16_t SessionId, uint32_t AuthValue, SysPkgPtzCtrl * SysPkgPtzCtrlPtr)
{
    GMI_RESULT    Result = GMI_SUCCESS;
    uint16_t      ReqAttrCnt = 1;
    SysAttr       SysReqAttr = {0};
    SysPkgPtzCtrl SysPtzCtrl = {0};

    memset(&SysPtzCtrl, 0, sizeof(SysPkgPtzCtrl));
    memcpy(&SysPtzCtrl, SysPkgPtzCtrlPtr, sizeof(SysPkgPtzCtrl));
    SysPtzCtrl.s_PtzId    = htonl(SysPtzCtrl.s_PtzId );
    SysPtzCtrl.s_PtzCmd   = htonl(SysPtzCtrl.s_PtzCmd);
    SysPtzCtrl.s_Param[0] = htonl(SysPtzCtrl.s_Param[0]);
    SysPtzCtrl.s_Param[1] = htonl(SysPtzCtrl.s_Param[1]);
    SysPtzCtrl.s_Param[2] = htonl(SysPtzCtrl.s_Param[2]);
    SysPtzCtrl.s_Param[3] = htonl(SysPtzCtrl.s_Param[3]);

    SysReqAttr.s_Type       = TYPE_CTLPTZ;
    SysReqAttr.s_Attr       = (void_t*)&SysPtzCtrl;
    SysReqAttr.s_AttrLength = sizeof( SysPkgPtzCtrl );


    Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_CTL_PTZ_REQ, ReqAttrCnt, &SysReqAttr);
    if ( FAILED(Result) )
    {
        goto errExit;
    }

    return GMI_SUCCESS;
errExit:
    return GMI_FAIL;
}


GMI_RESULT SysGetAutoFocusCfg(uint16_t SessionId, uint32_t AuthValue, SysPkgAutoFocus *SysAutoFocusPtr)
{
    GMI_RESULT Result;
    boolean_t  Exist            = false;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;

    if (NULL == SysAutoFocusPtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_AUTOFOCUS_REQ, &RspAttrCnt, &SysRspAttrPtr);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SysGetCmdExcute fail, Result = 0x%lx\n", Result);
            break;
        }

        if (RspAttrCnt <= 0
                || SysRspAttrPtr == NULL)
        {
            SYS_CLIENT_ERROR("RspAttrCnt %d incorrect or SysRspAttrPtr is Null\n", RspAttrCnt);
            break;
        }

        SysRspAttrTmpPtr = SysRspAttrPtr;
        for (uint32_t i = 0; i < RspAttrCnt; i++)
        {
            if (SysRspAttrTmpPtr->s_Type == TYPE_AUTO_FOCUS
                    && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgAutoFocus))
            {
                memcpy(SysAutoFocusPtr, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
                SysAutoFocusPtr->s_FocusMode = ntohl(SysAutoFocusPtr->s_FocusMode);
                Exist = true;
                break;
            }
            SysRspAttrTmpPtr++;
        }

        if (!Exist)
        {
            SYS_CLIENT_ERROR("not found valid data\n");
            break;
        }

        SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
        return GMI_SUCCESS;
    }
    while(0);

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysSetAutoFocusCfg(uint16_t SessionId, uint32_t AuthValue, SysPkgAutoFocus *SysAutoFocusPtr)
{
    GMI_RESULT   Result = GMI_SUCCESS;
    uint16_t     ReqAttrCnt = 1;
    SysAttr      SysReqAttr = {0};
    SysPkgAutoFocus SysAutoFocus;

    if (SysAutoFocusPtr == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        memset(&SysAutoFocus, 0, sizeof(SysPkgAutoFocus));
        memcpy(&SysAutoFocus, SysAutoFocusPtr, sizeof(SysPkgAutoFocus));
        SysAutoFocus.s_FocusMode = htonl(SysAutoFocus.s_FocusMode);

        SysReqAttr.s_Type       = TYPE_AUTO_FOCUS;
        SysReqAttr.s_Attr       = (void_t*)&SysAutoFocus;
        SysReqAttr.s_AttrLength = sizeof(SysPkgAutoFocus);

        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_AUTOFOCUS_REQ, ReqAttrCnt, &SysReqAttr);
        if (FAILED(Result))
        {
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return GMI_FAIL;
}


GMI_RESULT SysFocusGlobalScan(uint16_t SessionId, uint32_t AuthValue)
{
    SysPkgPtzCtrl SysPtzCtrl;

    memset(&SysPtzCtrl, 0, sizeof(SysPkgPtzCtrl));
    SysPtzCtrl.s_PtzId  = 1;
    SysPtzCtrl.s_PtzCmd = SYS_PTZCMD_GMI_MANUAL_FOCUS;

    return SysPtzControl(SessionId, AuthValue, &SysPtzCtrl);
}


GMI_RESULT SysSearchPtzPresetInfo(uint16_t SessionId, uint32_t AuthValue, uint32_t PresetIndex, boolean_t *Setted,  char_t PresetName[128])
{    
	GMI_RESULT	 Result = GMI_SUCCESS;	
	SysAttr		 SysReqAttr = {0};
	SysPkgPresetSearch SysPresetSearch;
	SysAttr      *SysRspAttrPtr    = NULL;
    SysAttr      *SysRspAttrTmpPtr = NULL;
    uint16_t     RspAttrCnt        = 0;
    uint16_t     ReqAttrCnt        = 1;
    boolean_t    Exist             = false;

	if (NULL == Setted
	   || NULL == PresetName)
	{
	   return GMI_INVALID_PARAMETER;
	}

	do
	{
		memset(&SysPresetSearch, 0, sizeof(SysPkgPresetSearch));
		SysPresetSearch.s_PtzId = htonl(1);
		SysPresetSearch.s_PresetIndex = htonl(PresetIndex);

		SysReqAttr.s_Type	   = TYPE_PTZPRESET_SEARCH;
		SysReqAttr.s_Attr	   = (void_t*)&SysPresetSearch;
		SysReqAttr.s_AttrLength = sizeof(SysPkgPresetSearch);
		Result = SysGetCmdExcuteWithAttrs(SessionId, AuthValue, SYSCODE_SEARCH_PTZPRESET_REQ, ReqAttrCnt, &SysReqAttr, &RspAttrCnt, &SysRspAttrPtr);
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

		SysPkgPtzPresetInfo SysPtzPresetInfo;
		SysRspAttrTmpPtr = SysRspAttrPtr;
	    for (int32_t i = 0; i < RspAttrCnt; i++)
	    {	    	
	    	if (SysRspAttrTmpPtr->s_Type == TYPE_PTZPRESET
                    && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgPtzPresetInfo))
        	{
        		memcpy(&SysPtzPresetInfo, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
        		SysPtzPresetInfo.s_PtzId = ntohl(SysPtzPresetInfo.s_PtzId);
        		SysPtzPresetInfo.s_PresetIndex = ntohl(SysPtzPresetInfo.s_PresetIndex);
        		Exist = true;
        		break;
        	}
	    }

	    if (!Exist)
        {
            SYS_CLIENT_ERROR("not found valid data\n");
            break;
        }
        
		*Setted = (SysPtzPresetInfo.s_PresetIndex >> 16 ) & 0x01 ? true : false;
        strcpy(PresetName, SysPtzPresetInfo.s_PresetName);
        
        SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
        return GMI_SUCCESS;
	}
	while (0);

	SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysSetPtzPresetInfo(uint16_t SessionId, uint32_t AuthValue, SysPkgPtzPresetInfo *SysPtzPresetInfoPtr)
{
    GMI_RESULT Result     = GMI_SUCCESS;  
    uint16_t   ReqAttrCnt = 1;
    SysAttr    SysReqAttr = {0};
    SysPkgPtzPresetInfo SysPtzPresetInfo;

    do
    {
        if (NULL == SysPtzPresetInfoPtr)
        {
            return GMI_INVALID_PARAMETER;
        }

        memset(&SysPtzPresetInfo, 0, sizeof(SysPkgPtzPresetInfo));
        memcpy(&SysPtzPresetInfo, SysPtzPresetInfoPtr, sizeof(SysPkgPtzPresetInfo));
        SysPtzPresetInfo.s_PtzId       = htonl(SysPtzPresetInfo.s_PtzId);
        SysPtzPresetInfo.s_PresetIndex = htonl(SysPtzPresetInfo.s_PresetIndex);
        SysReqAttr.s_Type       = TYPE_PTZPRESET;
        SysReqAttr.s_Attr       = (void_t*)&SysPtzPresetInfo;
        SysReqAttr.s_AttrLength = sizeof(SysPkgPtzPresetInfo);
        
        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_PTZPRESET_REQ, ReqAttrCnt, &SysReqAttr);
        if (FAILED(Result))
        {
            break;
        }

        return GMI_SUCCESS;    
    }
    while (0);

    return GMI_FAIL;
}


GMI_RESULT SysGetMaxPresetNum(uint16_t SessionId, uint32_t AuthValue, int32_t *MaxNumPtr)
{
    if (NULL == MaxNumPtr)
    {
        return GMI_FAIL;
    }

    *MaxNumPtr = MAX_PRESETS;

    return GMI_SUCCESS;
}


GMI_RESULT SysGetPtzPresetInfo(uint16_t SessionId, uint32_t AuthValue, int32_t MaxNum, SysPkgPtzPresetInfo *SysPtzPresetInfoPtr, int32_t *RspNum)
{
    uint8_t    i;
    GMI_RESULT Result           = GMI_SUCCESS;
    boolean_t  Exist            = false;
    uint16_t   RspAttrCnt       = 0;
    uint16_t   PresetCnt        = 0;
    uint16_t   RspCnt           = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;

    do 
    {
        Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_PTZPRESET_REQ, &RspAttrCnt, &SysRspAttrPtr);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SysGetCmdExcute fail, Result = 0x%lx\n", Result);
            printf("SysGetCmdExcute fail, Result = 0x%lx\n", Result);
            break;
        }

        if (RspAttrCnt <= 0
                || SysRspAttrPtr == NULL)
        {
            SYS_CLIENT_ERROR("RspAttrCnt %d incorrect or SysRspAttrPtr is Null\n", RspAttrCnt);
            printf("RspAttrCnt %d incorrect or SysRspAttrPtr is Null\n", RspAttrCnt);
            break;
        }

        PresetCnt = MaxNum >= RspAttrCnt ?  RspAttrCnt : MaxNum;
        SysRspAttrTmpPtr = SysRspAttrPtr;
        SYS_CLIENT_INFO("PresetCnt %d\n", PresetCnt);
        for (i = 0; i < PresetCnt; i++)
        {        
            SYS_CLIENT_INFO("s_Type %d, s_AttrLength %d\n", SysRspAttrTmpPtr->s_Type, SysRspAttrTmpPtr->s_AttrLength);   
            //printf("s_Type %d, s_AttrLength %d\n", SysRspAttrTmpPtr->s_Type, SysRspAttrTmpPtr->s_AttrLength);  
            if (SysRspAttrTmpPtr->s_Type == TYPE_PTZPRESET
                && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgPtzPresetInfo))
            {
                memcpy(&SysPtzPresetInfoPtr[i], SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
                SysPtzPresetInfoPtr[i].s_PtzId       = ntohl(SysPtzPresetInfoPtr[i].s_PtzId);
                SysPtzPresetInfoPtr[i].s_PresetIndex = ntohl(SysPtzPresetInfoPtr[i].s_PresetIndex);    
                Exist = true;     
                RspCnt++;   
            }
            SysRspAttrTmpPtr++;
        }

        if (!Exist)
        {
            SYS_CLIENT_ERROR("not found valid data\n");
            break;
        }

        *RspNum = RspCnt;
        SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
        return GMI_SUCCESS;
    }
    while (0);

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}