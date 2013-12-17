/*
*realize the struct of data from network to host
*
*/

#include "log.h"
#include "sys_utilitly_unix.h"
#include "sys_client.h"
#include "sys_command_excute.h"
#include "sys_env_types.h"
#include "gmi_system_headers.h"
#include "des.h"
#include "gmi_daemon_heartbeat_api.h"
#include "ipc_fw_v3.x_resource.h"
#include "ipc_fw_v3.x_setting.h"
#include "gmi_brdwrapperdef.h"
#include "gmi_brdwrapper.h"



#define  LAST_START_TIME_FILE  "/opt/log/LastStartTime"


static SysPkgTimeZoneInnerUse l_TimezoneTable[] =
{
    {8,   "CST 08:00:00", "CST+08:00:00"},
    {0,   "CST-00:00:00"},
    {-10, "CST-10:00:00"},
    {-9,  "CST-09:00:00"},
    {-8,  "CST-08:00:00"},
    {-7,  "CST-07:00:00"},
    {-6,  "CST-06:00:00"},
    {-5,  "CST-05:00:00"},
    {-4,  "CST-04:30:00"},
    {-4,  "CST-04:00:00"},
    {-3,  "CST-03:30:00"},
    {-3,  "CST-03:00:00"},
    {-2,  "CST-02:00:00"},
    {-1,  "CST-01:00:00"},
    {1,   "CST 01:00:00", "CST+01:00:00"},
    {2,   "CST 02:00:00", "CST+02:00:00"},
    {3,   "CST 03:00:00", "CST+03:00:00"},
    {3,   "CST 03:30:00", "CST+03:30:00"},
    {4,   "CST 04:00:00", "CST+04:00:00"},
    {4,   "CST 04:30:00", "CST+04:30:00"},
    {5,   "CST 05:00:00", "CST+05:00:00"},
    {5,   "CST 05:30:00", "CST+05:30:00"},
    {5,   "CST 05:45:00", "CST+05:45:00"},
    {6,   "CST 06:00:00", "CST+06:00:00"},
    {6,   "CST 06:30:00", "CST+06:30:00"},
    {7,   "CST 07:00:00", "CST+07:00:00"},
    {8,   "CST 08:00:00", "CST+08:00:00"},
    {9,   "CST 09:00:00", "CST+09:00:00"},
    {9,   "CST 09:30:00", "CST+09:30:00"},
    {10,  "CST 10:00:00", "CST+10:00:00"},
    {11,  "CST 11:00:00", "CST+11:00:00"},
    {12,  "CST 12:00:00", "CST+12:00:00"},
    {13,  "CST 13:00:00", "CST+13:00:00"}
};

GMI_RESULT SysGetDeviceIP(uint16_t SessionId, uint32_t AuthValue, SysPkgIpInfo *SysPkgIpInfoPtr)
{
    uint8_t    i;
    boolean_t  Exist            = false;
    GMI_RESULT Result           = GMI_SUCCESS;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;

    if (SysPkgIpInfoPtr == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_IPINFO_REQ, &RspAttrCnt, &SysRspAttrPtr);
    if (FAILED(Result))
    {
        goto errExit;
    }

    if (RspAttrCnt <= 0
            || SysRspAttrPtr == NULL)
    {
        goto errExit;
    }

    SysRspAttrTmpPtr = SysRspAttrPtr;
    for (i = 0; i < RspAttrCnt; i++)
    {
        if (SysRspAttrTmpPtr->s_Type == TYPE_IPINFOR
                && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgIpInfo))
        {
            memcpy(SysPkgIpInfoPtr, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
            SysPkgIpInfoPtr->s_NetId = ntohl(SysPkgIpInfoPtr->s_NetId);
            Exist = true;
            break;
        }

        SysRspAttrTmpPtr++;
    }

    if (!Exist)
    {
        SYS_CLIENT_ERROR("not found valid data, AttrCnt %d\n", RspAttrCnt);
        SysRspAttrTmpPtr = SysRspAttrPtr;
        for (i = 0; i < RspAttrCnt; i++)
        {
            SYS_CLIENT_ERROR("AttrId %d, AttrType %d, AttrLength %d\n", i, SysRspAttrTmpPtr->s_Type, SysRspAttrTmpPtr->s_AttrLength);
            SysRspAttrTmpPtr++;
        }
        goto errExit;
    }

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_SUCCESS;
errExit:
    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysSetDeviceIP(uint16_t SessionId, uint32_t AuthValue, SysPkgIpInfo *SysPkgIpInfoPtr)
{
    GMI_RESULT   Result = GMI_SUCCESS;
    uint16_t     ReqAttrCnt = 1;
    SysAttr      SysReqAttr = {0};
    SysPkgIpInfo SysIpInfo;

    if (SysPkgIpInfoPtr == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    memset(&SysIpInfo, 0, sizeof(SysPkgIpInfo));
    memcpy(&SysIpInfo, SysPkgIpInfoPtr, sizeof(SysPkgIpInfo));
    SysIpInfo.s_NetId = htonl(SysIpInfo.s_NetId);

    SysReqAttr.s_Type       = TYPE_IPINFOR;
    SysReqAttr.s_Attr       = (void_t*)&SysIpInfo;
    SysReqAttr.s_AttrLength = sizeof(SysPkgIpInfo);

    Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_IPINFO_REQ, ReqAttrCnt, &SysReqAttr);
    if (FAILED(Result))
    {
        goto errExit;
    }

    return GMI_SUCCESS;
errExit:
    return GMI_FAIL;
}


GMI_RESULT SysGetDeviceInfo(uint16_t SessionId, uint32_t AuthValue, SysPkgDeviceInfo *SysPkgDeviceInfoPtr)
{
    uint8_t    i;
    boolean_t  Exist           = false;
    GMI_RESULT Result          = GMI_SUCCESS;
    uint16_t  RspAttrCnt       = 0;
    SysAttr  *SysRspAttrPtr    = NULL;
    SysAttr  *SysRspAttrTmpPtr = NULL;

    if (SysPkgDeviceInfoPtr == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_DEVICEINFO_REQ, &RspAttrCnt, &SysRspAttrPtr);
    if (FAILED(Result))
    {
        goto errExit;
    }

    if (RspAttrCnt <= 0
            || SysRspAttrPtr == NULL)
    {
        goto errExit;
    }

    SysRspAttrTmpPtr = SysRspAttrPtr;
    for (i = 0; i < RspAttrCnt; i++)
    {
        if (SysRspAttrTmpPtr->s_Type == TYPE_IPCNAME
                && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgDeviceInfo))
        {
            memcpy(SysPkgDeviceInfoPtr, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
            SysPkgDeviceInfoPtr->s_DeviceId = ntohl(SysPkgDeviceInfoPtr->s_DeviceId);
            Exist = true;
            break;
        }
        SysRspAttrTmpPtr++;
    }

    if (!Exist)
    {
        SYS_CLIENT_ERROR("not found valid data, AttrCnt %d\n", RspAttrCnt);
        SysRspAttrTmpPtr = SysRspAttrPtr;
        for (i = 0; i < RspAttrCnt; i++)
        {
            SYS_CLIENT_ERROR("AttrId %d, AttrType %d, AttrLength %d\n", i, SysRspAttrTmpPtr->s_Type, SysRspAttrTmpPtr->s_AttrLength);
            SysRspAttrTmpPtr++;
        }
        goto errExit;
    }

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_SUCCESS;
errExit:
    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysSetDeviceInfo(uint16_t SessionId, uint32_t AuthValue, SysPkgDeviceInfo *SysPkgDeviceInfoPtr)
{
    GMI_RESULT Result     = GMI_SUCCESS;
    uint16_t   ReqAttrCnt = 1;
    SysAttr    SysReqAttr = {0};
    SysPkgDeviceInfo SysDeviceInfo;

    memset(&SysDeviceInfo, 0, sizeof(SysPkgDeviceInfo));
    memcpy(&SysDeviceInfo, SysPkgDeviceInfoPtr, sizeof(SysPkgDeviceInfo));
    SysDeviceInfo.s_DeviceId = htonl(SysDeviceInfo.s_DeviceId);

    SysReqAttr.s_Type = TYPE_IPCNAME;
    SysReqAttr.s_Attr = (void_t*)&SysDeviceInfo;
    SysReqAttr.s_AttrLength = sizeof(SysPkgDeviceInfo);

    Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_DEVICEINFO_REQ, ReqAttrCnt, &SysReqAttr );
    if (FAILED(Result))
    {
        goto errExit;
    }

    return GMI_SUCCESS;
errExit:
    return GMI_FAIL;
}


GMI_RESULT SysCtrlSystem(uint16_t SessionId, uint32_t AuthValue, int32_t SysCtrlCmd, int32_t SysModuleId)
{
    GMI_RESULT Result     = GMI_SUCCESS;

    switch (SysCtrlCmd)
    {
    case SYS_SYSTEM_CTRL_REBOOT:
    {
        uint16_t   ReqAttrCnt = 1;
        SysAttr    SysReqAttr = {0};

        SysCtrlCmd  = htonl(SysCtrlCmd);
        SysReqAttr.s_Type = TYPE_INTVALUE;
        SysReqAttr.s_Attr = (void_t*)&SysCtrlCmd;
        SysReqAttr.s_AttrLength = sizeof(int32_t);
        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_CTL_SYSTEM_REQ, ReqAttrCnt, &SysReqAttr);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SysSetCmdExcute fail, Result = 0x%lx\n", Result);
            return GMI_FAIL;
        }
    }
    break;
    case SYS_SYSTEM_CTRL_DEFAULT_HARD:
    case SYS_SYSTEM_CTRL_DEFAULT_SOFT:
    {
        uint16_t   ReqAttrCnt = 2;
        SysAttr    SysReqAttr[2];

        memset(SysReqAttr, 0, sizeof(SysReqAttr));
        SysCtrlCmd  = htonl(SysCtrlCmd);
        SysReqAttr[0].s_Type = TYPE_INTVALUE;
        SysReqAttr[0].s_Attr = (void_t*)&SysCtrlCmd;
        SysReqAttr[0].s_AttrLength = sizeof(int32_t);

        SysModuleId  = htonl(SysModuleId);
        SysReqAttr[1].s_Type = TYPE_INTVALUE;
        SysReqAttr[1].s_Attr = (void_t*)&SysModuleId;
        SysReqAttr[1].s_AttrLength = sizeof(int32_t);
        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_CTL_SYSTEM_REQ, ReqAttrCnt, SysReqAttr);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SysSetCmdExcute fail, Result = 0x%lx\n", Result);
            return GMI_FAIL;
        }
    }
    break;
    default:
        return GMI_NOT_SUPPORT;
    }

    return GMI_SUCCESS;
}


GMI_RESULT GMI_ConfigToolIrCultOpen(uint16_t SessionId, uint32_t AuthValue)
{
    GMI_RESULT Result = GMI_SUCCESS;
    
    do
    {  
        Result = GMI_BrdIrcutOpen();
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("GMI_ConfigToolIrCultOpen fail, Result = 0x%lx\n", Result);
            break;
        }
        
        return Result;
    }
    while(0);
    
    return Result;
}

GMI_RESULT GMI_ConfigToolIrCultClose(uint16_t SessionId, uint32_t AuthValue)
{
    GMI_RESULT Result = GMI_SUCCESS;
    
    do
    {
        Result = GMI_BrdIrcutClose();
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("GMI_ConfigToolIrCultClose fail, Result = 0x%lx\n", Result);
            break;
        }
        return Result;
    }
    while(0);
    
    return Result;
}

GMI_RESULT GMI_ConfigToolOpenDcIris(uint16_t SessionId, uint32_t AuthValue)
{
    GMI_RESULT Result	  = GMI_SUCCESS;
    
    do
    {  
        Result = GMI_BrdDcirsValueUpdate(100);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("GMI_BrdDcirsValueUpdate fail, Result = 0x%lx\n", Result);
            break;
        }
        
        return Result;
    }
    while(0);
    
    return Result;
}

GMI_RESULT GMI_ConfigToolCloseDcIris(uint16_t SessionId, uint32_t AuthValue)
{
    GMI_RESULT Result	  = GMI_SUCCESS;
    
    do
    {
        Result = GMI_BrdDcirsValueUpdate(1000);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("GMI_BrdDcirsValueUpdate fail, Result = 0x%lx\n", Result);
            break;
        }
        return Result;
    }
    while(0);
    
    return Result;
}

GMI_RESULT GMI_ConfigToolWatchDogTest(uint16_t SessionId, uint32_t AuthValue)
{
    GMI_RESULT Result     = GMI_SUCCESS;

    do
    {

	Result = GMI_BrdWatchdogReset();
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("GMI_BrdWatchdogReset fail, Result = 0x%lx\n", Result);
            break;
        }

    	return GMI_SUCCESS;
    }
    while(0);
    
    return GMI_FAIL;
}

GMI_RESULT GMI_ConfigToolSetMac(uint16_t SessionId, uint32_t AuthValue,const char_t *MacAddrs)
{
    GMI_RESULT Result     = GMI_SUCCESS;

    do
    {       
	char_t SystemMac[64];
	SYS_CLIENT_ERROR("GMI_ConfigToolSetMac Start MAC = [%s]\n",MacAddrs);

	memset(SystemMac, 0, sizeof(SystemMac));
	sprintf(SystemMac, "nandwrite -E %s", (char_t *)MacAddrs);
	SYS_CLIENT_ERROR("GMI_ConfigToolSetMac end MAC = [%s]\n",SystemMac);

	if(system(SystemMac) < 0)
	{
		Result = GMI_FAIL;
		return Result;
	}
	
	SYS_CLIENT_ERROR("GMI_ConfigToolSetMac Start MAC = [%s]\n",SystemMac);

	struct stat FileInfo;		 
	Result = stat("/etc/udev/rules.d/70-persistent-net.rules", &FileInfo);
	if (0 == Result)
	{
		memset(SystemMac, 0, sizeof(SystemMac));
		sprintf(SystemMac, "rm -f %s", "/etc/udev/rules.d/70-persistent-net.rules");
		system(SystemMac);
	}	

	SYS_CLIENT_ERROR("GMI_ConfigToolSetMac Start MAC = [%s]\n",SystemMac);

    	return GMI_SUCCESS;
    }
    while(0);
    
    return GMI_FAIL;
}

GMI_RESULT GMI_ConfigToolAfConfigDetect(uint16_t SessionId, uint32_t AuthValue,int32_t *FileFlags)
{
    int32_t Result;
    struct stat FileInfo;
    int32_t TmpFileFlags = 0;
    
    Result = stat("/opt/config/focus_range_DF003.cfg", &FileInfo);
    if (0 == Result)
    {
        TmpFileFlags += 1;
    }	 
    
    Result = stat("/opt/config/focus_zoom_DF003.cfg", &FileInfo);
    if (0 == Result)
    {
        TmpFileFlags += 1;
    }  
    
    Result = stat("/opt/config/param_day.cfg", &FileInfo);
    if (0 == Result)
    {
        TmpFileFlags += 1;
    }  
    
    Result = stat("/opt/config/param_night.cfg", &FileInfo);
    if (0 == Result)
    {
        TmpFileFlags += 1;
    }  
    
    Result = stat("/opt/config/motor_lens.cfg", &FileInfo);
    if (0 == Result)
    {
        TmpFileFlags += 1;
    }  

    if(TmpFileFlags < 5)
        *FileFlags = 0;
    else
        *FileFlags = 1;
 
     return GMI_SUCCESS;
}


/*==============================================================
name				:	GMI_BrdSetSystemNetworkMac
function			:  Get Eth interface Link state
lgorithm implementation	:	no
global variable			:	no
parameter declaration		:     byEthId  ,Ethernet port
                                               Link  ,Pointer to Ethernet Port state. 0 Link down  1 Link up
return				:    FAIL:  GMI_FAIL
                                     SUCCESS : GMI_SUCCESS
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	6/13/2013	1.0.0.0.0     minchao.wang         establish
================================================================*/
GMI_RESULT  GMI_SetSystemNetworkMac(const char_t *NetMac)
{
    if(NULL == NetMac)
    {
	return GMI_INVALID_PARAMETER;
    }

    char_t  CmdBuffer[255];

    memset(CmdBuffer, 0, sizeof(CmdBuffer));
    sprintf(CmdBuffer,"nandwrite -E %s",(char_t *)NetMac);
    if(system(CmdBuffer) < 0)
    {
	return GMI_FAIL;
    }
        
    memset(CmdBuffer, 0, sizeof(CmdBuffer));
    sprintf(CmdBuffer, "rm -f %s", "/etc/udev/rules.d/70-persistent-net.rules");
    if(system(CmdBuffer) < 0)
    {
   // return GMI_FAIL;
    }   

    return GMI_SUCCESS;
}


GMI_RESULT GMI_FactoryDefaultLocalApi(void)
{
    int32_t Result;
    char_t  Cmd[128];
    struct stat FileInfo;

    Result = stat(GMI_SETTING_CONFIG_FILE_NAME, &FileInfo);
    if (0 == Result)
    {
        memset(Cmd, 0, sizeof(Cmd));
        sprintf(Cmd, "rm -f %s", GMI_SETTING_CONFIG_FILE_NAME);
        system(Cmd);
    }   

    Result = stat(GMI_RESOURCE_CONFIG_FILE_NAME, &FileInfo);
    if (0 == Result)
    {
        memset(Cmd, 0, sizeof(Cmd));
        sprintf(Cmd, "rm -f %s", GMI_RESOURCE_CONFIG_FILE_NAME);
        system(Cmd);
    }

    Result = stat(GMI_DAEMON_CONFIG_FILE, &FileInfo);
    if (0 == Result)
    {
        memset(Cmd, 0, sizeof(Cmd));
        sprintf(Cmd, "rm -f %s", GMI_DAEMON_CONFIG_FILE);
        system(Cmd);
    }

    return GMI_SUCCESS;
}

GMI_RESULT GMI_WriteNetDefaultCfg(void)
{
    FILE       *Fp = NULL;
    Fp = fopen(NETWORK_CONFIG_FILE, "wb");
    if(NULL == Fp)
    {
        return GMI_FAIL;
    }

    fprintf(Fp, "# Configure Loopback\n");
    fprintf(Fp, "auto lo\n");
    fprintf(Fp, "iface lo inet loopback\n");
    fprintf(Fp, "auto eth0\n");
    fprintf(Fp, "iface eth0 inet static\n");
    fprintf(Fp, "address      %s\n", NET_DEFAULT_IP);
    fprintf(Fp, "netmask      %s\n", NET_DEFAULT_MASK);
    fprintf(Fp, "gateway      %s\n", NET_DEFAULT_GATEWAY);

    if(Fp != NULL)
    {
        fclose(Fp);
        Fp = NULL;
    }

    return GMI_SUCCESS;
}

GMI_RESULT FactorySimpleDefaultAllLocal(uint16_t SessionId, uint32_t AuthValue)
{
    GMI_RESULT Result;

    DaemonData_t DaemonData;    
    Result = GMI_DaemonInit( &DaemonData, WEB_SERVER_ID, GMI_DAEMON_HEARDBEAT_SERVER, GMI_DAEMON_HEARDBEAT_WEB);
    if(FAILED(Result))
    {
    	GMI_DaemonUnInit(&DaemonData);
    	return GMI_FAIL;
    }

    do
    {
        Result = GMI_FactoryDefaultLocalApi();
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("FactoryDefaultLocalApi fail, Result = 0x%lx\n", Result);
            break;
        }

	Result = GMI_SystemReboot(&DaemonData, GMI_MONITOR_TO_SDK_PORT_DEFAULT);
	if (FAILED(Result))
	{
		SYS_CLIENT_ERROR("GMI_SystemReboot fail, Result = 0x%lx\n", Result);
		break;
	}
		
	GMI_DaemonUnInit(&DaemonData);

        return GMI_SUCCESS;
    }
    while(0);

    GMI_DaemonUnInit(&DaemonData);

    return Result;
}

GMI_RESULT FactoryDefaultAllLocal(uint16_t SessionId, uint32_t AuthValue)
{
    GMI_RESULT Result;

    DaemonData_t DaemonData;    
    Result = GMI_DaemonInit( &DaemonData, WEB_SERVER_ID, GMI_DAEMON_HEARDBEAT_SERVER, GMI_DAEMON_HEARDBEAT_WEB);
    if(FAILED(Result))
    {
    	GMI_DaemonUnInit(&DaemonData);
    	return GMI_FAIL;
    }

    do
    {
        Result = GMI_FactoryDefaultLocalApi();
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("FactoryDefaultLocalApi fail, Result = 0x%lx\n", Result);
            break;
        }
        
	Result =  GMI_WriteNetDefaultCfg();
	if (FAILED(Result))
	{
		SYS_CLIENT_ERROR("FactoryDefaultLocalApi fail, Result = 0x%lx\n", Result);
		break;
	}

	Result = GMI_SystemReboot(&DaemonData, GMI_MONITOR_TO_SDK_PORT_DEFAULT);
	if (FAILED(Result))
	{
		SYS_CLIENT_ERROR("GMI_SystemReboot fail, Result = 0x%lx\n", Result);
		break;
	}
        
        GMI_DaemonUnInit(&DaemonData);
        return GMI_SUCCESS;
    }
    while(0);

    GMI_DaemonUnInit(&DaemonData);

    return Result;
}


GMI_RESULT FactorySimpleDefaultAll(uint16_t SessionId, uint32_t AuthValue)
{
    GMI_RESULT Result;

    do
    {
        Result = SysCtrlSystem(SessionId, AuthValue, SYS_SYSTEM_CTRL_DEFAULT_SOFT, SYS_CONFIG_MODULE_ALL);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SysCtrlSystem fail, Result = 0x%lx\n", Result);
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return Result;
}


GMI_RESULT FactoryDefaultAll(uint16_t SessionId, uint32_t AuthValue)
{
    GMI_RESULT Result;

    do
    {
        Result = SysCtrlSystem(SessionId, AuthValue, SYS_SYSTEM_CTRL_DEFAULT_HARD, SYS_CONFIG_MODULE_ALL);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SysCtrlSystem fail, Result = 0x%lx\n", Result);
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return Result;
}

GMI_RESULT FactoryDefaultImaging(uint16_t SessionId, uint32_t AuthValue)
{
    GMI_RESULT Result;

    do
    {
        Result = SysCtrlSystem(SessionId, AuthValue, SYS_SYSTEM_CTRL_DEFAULT_HARD, SYS_CONFIG_MODULE_IMAGING);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SysCtrlSystem fail, Result = 0x%lx\n", Result);
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return Result;
}

GMI_RESULT FactoryDefaultStreamCombine(uint16_t SessionId, uint32_t AuthValue)
{
    GMI_RESULT Result;

    do
    {
        Result = SysCtrlSystem(SessionId, AuthValue, SYS_SYSTEM_CTRL_DEFAULT_HARD, SYS_CONFIG_MODULE_STREAM_COMBINE);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("reset stream combine fail, Result = 0x%lx\n", Result);
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return Result;
}


GMI_RESULT FactoryDefaultVideoEncode(uint16_t SessionId, uint32_t AuthValue)
{
    GMI_RESULT Result;

    do
    {
        Result = SysCtrlSystem(SessionId, AuthValue, SYS_SYSTEM_CTRL_DEFAULT_HARD, SYS_CONFIG_MODULE_VIDEO_ENCODE);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("reset video encode fail, Result = 0x%lx\n", Result);
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return Result;
}


GMI_RESULT SysReboot(uint16_t SessionId, uint32_t AuthValue)
{
    GMI_RESULT Result = GMI_SUCCESS;

    do
    {
        GMI_RESULT Result = SysCtrlSystem(SessionId, AuthValue, SYS_SYSTEM_CTRL_REBOOT, 0);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SysCtrlSystem fail, Result = 0x%lx\n", Result);
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return Result;
}


GMI_RESULT SysGetTime(uint16_t SessionId, uint32_t AuthValue, SysPkgDateTimeType *SysDateTimeTypePtr, SysPkgSysTime *SysTimePtr, SysPkgTimeZone *SysTimezonePtr, SysPkgNtpServerInfo *SysNtpServerInfoPtr)
{
    uint8_t    i;
    boolean_t  Exist            = false;
    GMI_RESULT Result           = GMI_SUCCESS;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;

    if (NULL == SysTimePtr
            || NULL == SysTimezonePtr
            || NULL == SysDateTimeTypePtr)
    {
        SYS_CLIENT_ERROR("invalid param\n");
        return GMI_INVALID_PARAMETER;
    }

    Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_TIME_REQ, &RspAttrCnt, &SysRspAttrPtr);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("SysGetCmdExcute fail, Result = 0x%lx\n", Result);
        goto errExit;
    }
    if (RspAttrCnt <= 0
            || SysRspAttrPtr == NULL)
    {
        SYS_CLIENT_ERROR("RspAttrCnt %d incorrect or SysRspAttrPtr is Null\n", RspAttrCnt);
        goto errExit;
    }

    SysRspAttrTmpPtr = SysRspAttrPtr;
    for (i = 0; i < RspAttrCnt; i++)
    {
        switch (SysRspAttrTmpPtr->s_Type)
        {
        case TYPE_SYSTIME:
        {
            if (SysRspAttrTmpPtr->s_AttrLength != sizeof(SysPkgSysTime))
            {
                SYS_CLIENT_ERROR("Get AttrLength = %d incorrect\n", SysRspAttrTmpPtr->s_AttrLength);
                goto errExit;
            }

            memcpy(SysTimePtr, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
            SysTimePtr->s_Year   = ntohl(SysTimePtr->s_Year);
            SysTimePtr->s_Month  = ntohl(SysTimePtr->s_Month);
            SysTimePtr->s_Day    = ntohl(SysTimePtr->s_Day);
            SysTimePtr->s_Hour   = ntohl(SysTimePtr->s_Hour);
            SysTimePtr->s_Minute = ntohl(SysTimePtr->s_Minute);
            SysTimePtr->s_Second = ntohl(SysTimePtr->s_Second);
            Exist = true;
        }
        break;
        case TYPE_TIMEZONE:
        {
            if (SysRspAttrTmpPtr->s_AttrLength != sizeof(SysPkgTimeZone))
            {
                SYS_CLIENT_ERROR("Get AttrLength = %d incorrect\n", SysRspAttrTmpPtr->s_AttrLength);
                goto errExit;
            }

            memcpy(SysTimezonePtr, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
            SysTimezonePtr->s_TimeZone = ntohl(SysTimezonePtr->s_TimeZone);
            Exist = true;
        }
        break;
        case TYPE_NTPSERVER:
        {
            if (SysRspAttrTmpPtr->s_AttrLength != sizeof(SysPkgNtpServerInfo))
            {
                SYS_CLIENT_ERROR("Get AttrLength = %d incorrect\n", SysRspAttrTmpPtr->s_AttrLength);
                goto errExit;
            }

            memcpy(SysNtpServerInfoPtr, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
            Exist = true;
        }
        break;
        case TYPE_TIMETYPE:
        {
            if (SysRspAttrTmpPtr->s_AttrLength != sizeof(SysPkgDateTimeType))
            {
                SYS_CLIENT_ERROR("Get AttrLength = %d incorrect\n", SysRspAttrTmpPtr->s_AttrLength);
                goto errExit;
            }

            memcpy(SysDateTimeTypePtr, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
            SysDateTimeTypePtr->s_Type        = ntohl(SysDateTimeTypePtr->s_Type);
            SysDateTimeTypePtr->s_NtpInterval = ntohl(SysDateTimeTypePtr->s_NtpInterval);
            Exist = true;
        }
        break;
        default:
            SYS_CLIENT_ERROR("invalid Attr type %d\n", SysRspAttrTmpPtr->s_Type);
            goto errExit;
        }

        SysRspAttrTmpPtr++;
    }

    if (!Exist)
    {
        SYS_CLIENT_ERROR("not found valid data\n");
        goto errExit;
    }

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_SUCCESS;
errExit:
    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysGetNtpServer(uint16_t SessionId, uint32_t AuthValue, SysPkgNtpServerInfo *SysNtpServerInfoPtr)
{
    uint8_t    i;
    boolean_t  Exist            = false;
    GMI_RESULT Result           = GMI_SUCCESS;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;

    if (NULL == SysNtpServerInfoPtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_TIME_REQ, &RspAttrCnt, &SysRspAttrPtr);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("SysGetCmdExcute fail, Result = 0x%lx\n", Result);
        goto errExit;
    }
    if (RspAttrCnt <= 0
            || SysRspAttrPtr == NULL)
    {
        SYS_CLIENT_ERROR("RspAttrCnt %d incorrect or SysRspAttrPtr is Null\n", RspAttrCnt);
        goto errExit;
    }

    SysRspAttrTmpPtr = SysRspAttrPtr;
    for (i = 0; i < RspAttrCnt; i++)
    {
        switch (SysRspAttrTmpPtr->s_Type)
        {
        case TYPE_SYSTIME:
        case TYPE_TIMEZONE:
        case TYPE_TIMETYPE:
            break;
        case TYPE_NTPSERVER:
        {
            if (SysRspAttrTmpPtr->s_AttrLength != sizeof(SysPkgNtpServerInfo))
            {
                SYS_CLIENT_ERROR("Get AttrLength = %d incorrect\n", SysRspAttrTmpPtr->s_AttrLength);
                goto errExit;
            }

            memcpy(SysNtpServerInfoPtr, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
            Exist = true;
        }
        break;
        default:
            SYS_CLIENT_ERROR("invalid Attr type %d\n", SysRspAttrTmpPtr->s_Type);
            break;
        }

        SysRspAttrTmpPtr++;
    }

    if (!Exist)
    {
        SYS_CLIENT_ERROR("not found valid data\n");
        goto errExit;
    }

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_SUCCESS;
errExit:
    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysSetTime(uint16_t SessionId, uint32_t AuthValue, SysPkgDateTimeType *SysDateTimeTypePtr, SysPkgSysTime *SysTimePtr, SysPkgTimeZone *SysTimezonePtr, SysPkgNtpServerInfo *SysNtpServerInfoPtr)
{
    uint32_t    i;
    GMI_RESULT Result        = GMI_SUCCESS;
    uint16_t   ReqAttrCnt    = 4;
    SysAttr    SysReqAttr[4];
    SysPkgSysTime       SysTime;
    SysPkgTimeZone      SysTimezone;
    SysPkgDateTimeType  SysDateTimeType;
    SysPkgNtpServerInfo SysNtpServerInfo;

    if (NULL == SysTimePtr
            || NULL == SysTimezonePtr
            || NULL == SysDateTimeTypePtr
            || NULL == SysNtpServerInfoPtr)
    {
        SYS_CLIENT_ERROR("invalid param\n");
        return GMI_INVALID_PARAMETER;
    }

    memset(&SysTime, 0, sizeof(SysPkgSysTime));
    memcpy(&SysTime, SysTimePtr, sizeof(SysPkgSysTime));
    SysTime.s_Year     = htonl(SysTime.s_Year);
    SysTime.s_Month    = htonl(SysTime.s_Month);
    SysTime.s_Day      = htonl(SysTime.s_Day);
    SysTime.s_Hour     = htonl(SysTime.s_Hour);
    SysTime.s_Minute   = htonl(SysTime.s_Minute);
    SysTime.s_Second   = htonl(SysTime.s_Second);
    SysReqAttr[0].s_Type = TYPE_SYSTIME;
    SysReqAttr[0].s_Attr = (void_t*)&SysTime;
    SysReqAttr[0].s_AttrLength = sizeof(SysPkgSysTime);

    memset(&SysTimezone, 0, sizeof(SysPkgTimeZone));
    memcpy(&SysTimezone, SysTimezonePtr, sizeof(SysPkgTimeZone));

    for (i = 0; i < sizeof(l_TimezoneTable)/sizeof(l_TimezoneTable[0]); i++)
    {
        if (0 == strcmp(SysTimezone.s_TimzeZoneName, l_TimezoneTable[i].s_TimeZoneName1)
                || 0 == strcmp(SysTimezone.s_TimzeZoneName, l_TimezoneTable[i].s_TimeZoneName2))
        {
            SysTimezone.s_TimeZone = l_TimezoneTable[i].s_TimeZone;
            break;
        }
    }
    if (i >= sizeof(l_TimezoneTable)/sizeof(l_TimezoneTable[0]))
    {
        SYS_CLIENT_ERROR("%s not found int TimzoneTable\n", SysTimezone.s_TimzeZoneName);
        return GMI_FAIL;
    }

    SysTimezone.s_TimeZone = htonl(SysTimezone.s_TimeZone);
    SysReqAttr[1].s_Type = TYPE_TIMEZONE;
    SysReqAttr[1].s_Attr = (void_t*)&SysTimezone;
    SysReqAttr[1].s_AttrLength = sizeof(SysPkgTimeZone);

    memset(&SysDateTimeType, 0, sizeof(SysPkgDateTimeType));
    memcpy(&SysDateTimeType, SysDateTimeTypePtr, sizeof(SysPkgDateTimeType));
    SysDateTimeType.s_Type        = htonl(SysDateTimeType.s_Type);
    SysDateTimeType.s_NtpInterval = htonl(SysDateTimeType.s_NtpInterval);
    SysReqAttr[2].s_Type = TYPE_TIMETYPE;
    SysReqAttr[2].s_Attr = (void_t*)&SysDateTimeType;
    SysReqAttr[2].s_AttrLength = sizeof(SysPkgDateTimeType);

    memset(&SysNtpServerInfo, 0, sizeof(SysPkgNtpServerInfo));
    memcpy(&SysNtpServerInfo, SysNtpServerInfoPtr, sizeof(SysPkgNtpServerInfo));
    SysReqAttr[3].s_Type = TYPE_NTPSERVER;
    SysReqAttr[3].s_Attr = (void_t*)&SysNtpServerInfo;
    SysReqAttr[3].s_AttrLength = sizeof(SysPkgNtpServerInfo);

    Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_TIME_REQ, ReqAttrCnt, SysReqAttr);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("SysSetCmdExcute fail, Result = 0x%lx\n", Result);
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}


GMI_RESULT SysGetTimezone(uint16_t SessionId, uint32_t AuthValue, SysPkgTimeZone *SysTimezonePtr)
{
    if (NULL == SysTimezonePtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    FILE *Fp = NULL;
    int32_t Timezone;
    char_t Buffer[128];
    char_t *ToGetPtr = NULL;

    memset(Buffer, 0, sizeof(Buffer));
    Fp = popen("date -R", "r");
    if (NULL == Fp)
    {
        goto errExit;
    }

    if (NULL == fgets(Buffer, sizeof(Buffer), Fp))
    {
        goto  errExit;
    }

    memset(SysTimezonePtr, 0, sizeof(SysPkgTimeZone));
    if (NULL != (ToGetPtr = strchr(Buffer, '+')))
    {
        sscanf(ToGetPtr+1, "%d", &Timezone);
        sprintf(SysTimezonePtr->s_TimzeZoneName, "UTC+%d", Timezone);
        Timezone /= 100;
        SysTimezonePtr->s_TimeZone = Timezone;
    }
    else if (NULL != (ToGetPtr = strchr(Buffer, '-')))
    {
        sscanf(ToGetPtr+1, "%d", &Timezone);
        sprintf(SysTimezonePtr->s_TimzeZoneName, "UTC-%d", Timezone);
        Timezone /= 100;
        Timezone |= (1 << 31);
    }
    else
    {
        goto errExit;
    }

    SYS_CLIENT_INFO("[%s]: timezone=%d\n", __func__, Timezone);

    if (NULL != Fp)
    {
        pclose(Fp);
    }
    return GMI_SUCCESS;
errExit:
    if (NULL != Fp)
    {
        pclose(Fp);
    }

    return GMI_FAIL;
}


GMI_RESULT SysGetUsers(uint16_t SessionId, uint32_t AuthValue, SysPkgUserInfo *SysUserInfoPtr, uint32_t UserCnt, uint32_t *RealUserCnt)
{
    GMI_RESULT Result;
    boolean_t  Exist            = false;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;

    if (NULL == SysUserInfoPtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_USERINFO_REQ, &RspAttrCnt, &SysRspAttrPtr);
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

        //decrypt
        int32_t PlainLen = 0;
        char_t  Passwd[LEN_DES_PASSWORD];
        char_t  Key[8];
        char_t  Name[32];

        memset(Name, 0, sizeof(Name));
        sprintf(Name, "eth0");
        memset(Key, 0, sizeof(Key));
        if (0 > NetReadMacChar(Name, Key))
        {
            return GMI_FAIL;
        }

        SysRspAttrTmpPtr = SysRspAttrPtr;
        for (uint32_t i = 0; i < RspAttrCnt; i++)
        {
            if (SysRspAttrTmpPtr->s_Type == TYPE_USERINFOR
                    && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgUserInfo))
            {
                if (i >= UserCnt)
                {
                    break;
                }
                memcpy(&SysUserInfoPtr[i], SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);

                memset(Passwd, 0, sizeof(Passwd));
                memcpy(Passwd, SysUserInfoPtr[i].s_UserPass, sizeof(Passwd));
                if (0 > DES_Decrypt(Passwd, sizeof(Passwd), Key, SysUserInfoPtr[i].s_UserPass, &PlainLen))
                {
                    SYS_CLIENT_ERROR("DES_Decrypt %s fail\n", SysUserInfoPtr[i].s_UserName);
                    break;
                }
                SysUserInfoPtr[i].s_UserPass[LEN_DES_PASSWORD] = '\0';

                SYS_CLIENT_INFO("%s %s\n", SysUserInfoPtr[i].s_UserName, SysUserInfoPtr[i].s_UserPass);
                SysUserInfoPtr[i].s_UserFlag      = ntohs(SysUserInfoPtr[i].s_UserFlag);
                SysUserInfoPtr[i].s_UserLevel     = ntohs(SysUserInfoPtr[i].s_UserLevel);
                Exist = true;
            }
            SysRspAttrTmpPtr++;
        }

        *RealUserCnt = RspAttrCnt > UserCnt ? UserCnt : RspAttrCnt;

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


GMI_RESULT SysSetUsers(uint16_t SessionId, uint32_t AuthValue, SysPkgUserInfo *SysUserInfoPtr)
{
    GMI_RESULT   Result = GMI_SUCCESS;
    uint16_t     ReqAttrCnt = 1;
    SysAttr      SysReqAttr = {0};
    SysPkgUserInfo SysUserInfo;

    if (SysUserInfoPtr == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        //encrypt
        int32_t CiperLen = 0;
        char_t  Passwd[LEN_DES_PASSWORD];
        char_t  Key[8];
        char_t  Name[32];

        memset(&SysUserInfo, 0, sizeof(SysPkgUserInfo));
        memcpy(&SysUserInfo, SysUserInfoPtr, sizeof(SysPkgUserInfo));

        memset(Name, 0, sizeof(Name));
        sprintf(Name, "eth0");
        memset(Key, 0, sizeof(Key));
        if (0 > NetReadMacChar(Name, Key))
        {
            return GMI_FAIL;
        }

        memset(Passwd, 0, sizeof(Passwd));
        strcpy(Passwd, SysUserInfo.s_UserPass);
        if (0 > DES_Encrypt(Passwd, sizeof(Passwd), Key, SysUserInfo.s_UserPass, &CiperLen))
        {
            SYS_CLIENT_ERROR("DES_Encrypt %s fail\n", SysUserInfo.s_UserName);
            return GMI_FAIL;
        }

        SysUserInfo.s_UserFlag     = htons(SysUserInfo.s_UserFlag);
        SysUserInfo.s_UserLevel    = htons(SysUserInfo.s_UserLevel);

        SysReqAttr.s_Type       = TYPE_USERINFOR;
        SysReqAttr.s_Attr       = (void_t*)&SysUserInfo;
        SysReqAttr.s_AttrLength = sizeof(SysPkgUserInfo);

        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_USERINFO_REQ, ReqAttrCnt, &SysReqAttr);
        if (FAILED(Result))
        {
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return GMI_FAIL;
}


GMI_RESULT SysDelUsers(uint16_t SessionId, uint32_t AuthValue, SysPkgUserInfo *SysUserInfoPtr)
{
    GMI_RESULT   Result = GMI_SUCCESS;
    uint16_t     ReqAttrCnt = 1;
    SysAttr      SysReqAttr = {0};
    SysPkgUserInfo SysUserInfo;

    if (SysUserInfoPtr == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        memset(&SysUserInfo, 0, sizeof(SysPkgUserInfo));
        memcpy(&SysUserInfo, SysUserInfoPtr, sizeof(SysPkgUserInfo));
        SysUserInfo.s_UserFlag = htons(SysUserInfo.s_UserFlag);
        SysUserInfo.s_UserLevel= htons(SysUserInfo.s_UserLevel);

        SysReqAttr.s_Type       = TYPE_USERINFOR;
        SysReqAttr.s_Attr       = (void_t*)&SysUserInfo;
        SysReqAttr.s_AttrLength = sizeof(SysPkgUserInfo);

        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_DEL_USERINFO_REQ, ReqAttrCnt, &SysReqAttr);
        if (FAILED(Result))
        {
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return GMI_FAIL;
}


GMI_RESULT SysAddUsers(uint16_t SessionId, uint32_t AuthValue, SysPkgUserInfo *SysUserInfoPtr)
{
    uint32_t RealUserCnt;
    GMI_RESULT Result;
    SysPkgUserInfo *UserInfoPtr = NULL;

    UserInfoPtr = (SysPkgUserInfo*)malloc(MAX_USERS * sizeof(SysPkgUserInfo));
    if (NULL == UserInfoPtr)
    {
        SYS_CLIENT_ERROR("malloc SysUserInfoPtr fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    do
    {
        Result = SysGetUsers(SessionId, AuthValue, UserInfoPtr, MAX_USERS, &RealUserCnt);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SysGetUsers fail, Result = 0x%lx\n", Result);
            break;
        }

        uint32_t i;
        for (i = 0; i < RealUserCnt; i++)
        {
            if (0 == strcmp(UserInfoPtr[i].s_UserName, SysUserInfoPtr->s_UserName))
            {
                break;
            }
        }

        if (i < RealUserCnt)
        {
            SYS_CLIENT_ERROR("UserName %s have already exits\n", SysUserInfoPtr->s_UserName);
            if (NULL != UserInfoPtr)
            {
                free(UserInfoPtr);
                UserInfoPtr = NULL;
            }
            return GMI_INVALID_OPERATION;
        }

        Result = SysSetUsers(SessionId, AuthValue, SysUserInfoPtr);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SysSetUsers fail, Result = 0x%lx\n", Result);
            break;
        }

        if (NULL != UserInfoPtr)
        {
            free(UserInfoPtr);
            UserInfoPtr = NULL;
        }

        return GMI_SUCCESS;
    }
    while(0);

    if (NULL != UserInfoPtr)
    {
        free(UserInfoPtr);
        UserInfoPtr = NULL;
    }
    return Result;
}


GMI_RESULT SysModifyUsers(uint16_t SessionId, uint32_t AuthValue, SysPkgUserInfo *SysUserInfoPtr)
{
    uint32_t RealUserCnt;
    GMI_RESULT Result;
    SysPkgUserInfo *UserInfoPtr = NULL;

    UserInfoPtr = (SysPkgUserInfo*)malloc(MAX_USERS * sizeof(SysPkgUserInfo));
    if (NULL == UserInfoPtr)
    {
        SYS_CLIENT_ERROR("malloc SysUserInfoPtr fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    do
    {
        Result = SysGetUsers(SessionId, AuthValue, UserInfoPtr, MAX_USERS, &RealUserCnt);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SysGetUsers fail, Result = 0x%lx\n", Result);
            break;
        }

        uint32_t i;
        for (i = 0; i < RealUserCnt; i++)
        {
            if (0 == strcmp(UserInfoPtr[i].s_UserName, SysUserInfoPtr->s_UserName))
            {
                break;
            }
        }

        if (i >= RealUserCnt)
        {
            SYS_CLIENT_ERROR("UserName %s not exits\n", SysUserInfoPtr->s_UserName);
            if (NULL != UserInfoPtr)
            {
                free(UserInfoPtr);
                UserInfoPtr = NULL;
            }
            return GMI_INVALID_OPERATION;
        }

        Result = SysSetUsers(SessionId, AuthValue, SysUserInfoPtr);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SysSetUsers fail, Result = 0x%lx\n", Result);
            break;
        }

        if (NULL != UserInfoPtr)
        {
            free(UserInfoPtr);
            UserInfoPtr = NULL;
        }

        return GMI_SUCCESS;
    }
    while(0);

    if (NULL != UserInfoPtr)
    {
        free(UserInfoPtr);
        UserInfoPtr = NULL;
    }
    return Result;
}


GMI_RESULT SysGetNetworkPort(uint16_t SessionId, uint32_t AuthValue, SysPkgNetworkPort *SysNetworkPortPtr)
{
    GMI_RESULT Result;
    boolean_t  Exist            = false;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;

    if (NULL == SysNetworkPortPtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_NETWORK_PORT_REQ, &RspAttrCnt, &SysRspAttrPtr);
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
            if (SysRspAttrTmpPtr->s_Type == TYPE_NETWORK_PORT
                    && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgNetworkPort))
            {
                memcpy(SysNetworkPortPtr, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
                SysNetworkPortPtr->s_HTTP_Port = ntohl(SysNetworkPortPtr->s_HTTP_Port);
                SysNetworkPortPtr->s_RTSP_Port = ntohl(SysNetworkPortPtr->s_RTSP_Port);
                SysNetworkPortPtr->s_SDK_Port  = ntohl(SysNetworkPortPtr->s_SDK_Port);
                SysNetworkPortPtr->s_Upgrade_Port= ntohl(SysNetworkPortPtr->s_Upgrade_Port);
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


GMI_RESULT SysSetNetworkPort(uint16_t SessionId, uint32_t AuthValue, SysPkgNetworkPort *SysNetworkPortPtr)
{
    GMI_RESULT   Result = GMI_SUCCESS;
    uint16_t     ReqAttrCnt = 1;
    SysAttr      SysReqAttr = {0};
    SysPkgNetworkPort SysNetworkPort;

    if (SysNetworkPortPtr == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        memset(&SysNetworkPort, 0, sizeof(SysPkgNetworkPort));
        memcpy(&SysNetworkPort, SysNetworkPortPtr, sizeof(SysPkgNetworkPort));
        SysNetworkPort.s_HTTP_Port = htonl(SysNetworkPort.s_HTTP_Port);
        SysNetworkPort.s_RTSP_Port = htonl(SysNetworkPort.s_RTSP_Port);
        SysNetworkPort.s_SDK_Port  = htonl(SysNetworkPort.s_SDK_Port);
        SysNetworkPort.s_Upgrade_Port= htonl(SysNetworkPort.s_Upgrade_Port);

        SysReqAttr.s_Type       = TYPE_NETWORK_PORT;
        SysReqAttr.s_Attr       = (void_t*)&SysNetworkPort;
        SysReqAttr.s_AttrLength = sizeof(SysPkgNetworkPort);

        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_NETWORK_PORT_REQ, ReqAttrCnt, &SysReqAttr);
        if (FAILED(Result))
        {
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return GMI_FAIL;
}


GMI_RESULT SysGetCapabilities(uint16_t SessionId, uint32_t AuthValue, char_t* Message, int32_t MessageLength, SysPkgXml *SysCapabilities)
{
    GMI_RESULT Result;
    boolean_t  Exist            = false;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;

    do
    {
        Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_CAPABILITIES_REQ, &RspAttrCnt, &SysRspAttrPtr);
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
            if (SysRspAttrTmpPtr->s_Type == TYPE_XML
                    && SysRspAttrTmpPtr->s_AttrLength > sizeof(SysPkgXml))
            {
                memcpy(SysCapabilities, SysRspAttrTmpPtr->s_Attr, sizeof(SysPkgXml));
                memcpy(Message, (char_t*)SysRspAttrTmpPtr->s_Attr + sizeof(SysPkgXml), SysRspAttrTmpPtr->s_AttrLength - sizeof(SysPkgXml));
                SysCapabilities->s_ContentLength      = ntohl(SysCapabilities->s_ContentLength);
                SysCapabilities->s_Encrypt            = ntohl(SysCapabilities->s_Encrypt);
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


GMI_RESULT SysGetWorkState(uint16_t SessionId, uint32_t AuthValue, char_t* Message, int32_t MessageLength, SysPkgXml *SysWorkState)
{
    GMI_RESULT Result;
    boolean_t  Exist            = false;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;

    do
    {
        Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_DEVICE_WORK_STATE_REQ, &RspAttrCnt, &SysRspAttrPtr);
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
            if (SysRspAttrTmpPtr->s_Type == TYPE_XML
                    && SysRspAttrTmpPtr->s_AttrLength > sizeof(SysPkgXml))
            {
                memcpy(SysWorkState, SysRspAttrTmpPtr->s_Attr, sizeof(SysPkgXml));
                memcpy(Message, (char_t*)SysRspAttrTmpPtr->s_Attr + sizeof(SysPkgXml), SysRspAttrTmpPtr->s_AttrLength - sizeof(SysPkgXml));
                SysWorkState->s_ContentLength      = ntohl(SysWorkState->s_ContentLength);
                SysWorkState->s_Encrypt            = ntohl(SysWorkState->s_Encrypt);
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


GMI_RESULT SysGetDeviceStartedTime(SysPkgSysTime *DeviceStartedTimePtr)
{
    FILE  *Fp = NULL;
    char_t Time[64];

    if (NULL == DeviceStartedTimePtr)
    {
        SYS_CLIENT_ERROR("DeviceStartedTime is null");
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        Fp = fopen(LAST_START_TIME_FILE, "rb");
        if (NULL == Fp)
        {
            SYS_CLIENT_ERROR("fopen %s error\n", LAST_START_TIME_FILE);
            break;
        }

        if (0 == fgets(Time, sizeof(Time), Fp))
        {
            SYS_CLIENT_ERROR("fgets null\n");
            break;
        }

        char_t Year[5];
        sscanf(Time, "%4s", Year);
        DeviceStartedTimePtr->s_Year = atoi(Year);

        char_t Mon[3];
        sscanf(Time+4, "%2s", Mon);
        DeviceStartedTimePtr->s_Month = atoi(Mon);

        char_t Day[3];
        sscanf(Time+6, "%2s", Day);
        DeviceStartedTimePtr->s_Day = atoi(Day);

        char_t Hour[3];
        sscanf(Time+8, "%2s", Hour);
        DeviceStartedTimePtr->s_Hour = atoi(Hour);

        char_t Min[3];
        sscanf(Time+10, "%2s", Min);
        DeviceStartedTimePtr->s_Minute = atoi(Min);

        char_t Sec[3];
        sscanf(Time+12, "%2s", Sec);
        DeviceStartedTimePtr->s_Second = atoi(Sec);

        if (NULL != Fp)
        {
            fclose(Fp);
            Fp = NULL;
        }
        return GMI_SUCCESS;
    }
    while(0);

    if (NULL != Fp)
    {
        fclose(Fp);
        Fp = NULL;
    }
    return GMI_FAIL;
}


GMI_RESULT SysGetHostName(char_t HostName[128])
{
    if (NULL == HostName)
    {
        return GMI_INVALID_PARAMETER;
    }

    FILE *Fp = NULL;
    //int32_t Timezone;
    char_t Buffer[128];
    //char_t *ToGetPtr = NULL;

    do
    {
        memset(Buffer, 0, sizeof(Buffer));
        Fp = popen("hostname", "r");
        if (NULL == Fp)
        {
            break;
        }

        if (NULL == fgets(Buffer, sizeof(Buffer), Fp))
        {
            break;
        }

        strcpy(HostName, Buffer);

        if (NULL != Fp)
        {
            pclose(Fp);
        }
        return GMI_SUCCESS;
    }
    while (0);

    if (NULL != Fp)
    {
        pclose(Fp);
    }

    return GMI_FAIL;
}


GMI_RESULT SysSetHostName(char_t HostName[128])
{
    char_t CmdBuff[128];

    memset(CmdBuff, 0, sizeof(CmdBuff));
    sprintf(CmdBuff, "hostname %s", HostName);
    system(CmdBuff);

    return GMI_SUCCESS;
}


GMI_RESULT SysExecuteImportFile(uint16_t SessionId, uint32_t AuthValue, SysPkgConfigFileInfo *SysConfigFileInfoPtr)
{
    GMI_RESULT   Result = GMI_SUCCESS;
    uint16_t     ReqAttrCnt = 1;
    SysAttr      SysReqAttr = {0};
    SysPkgConfigFileInfo SysConfigFileInfo;

    if (SysConfigFileInfoPtr == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        memset(&SysConfigFileInfo, 0, sizeof(SysPkgConfigFileInfo));
        memcpy(&SysConfigFileInfo, SysConfigFileInfoPtr, sizeof(SysPkgConfigFileInfo));
        SysConfigFileInfo.s_Persit = htonl(SysConfigFileInfo.s_Persit);
        SysConfigFileInfo.s_Encrypt= htonl(SysConfigFileInfo.s_Encrypt);           

        SysReqAttr.s_Type       = TYPE_CONFIG_FILE;
        SysReqAttr.s_Attr       = (void_t*)&SysConfigFileInfo;
        SysReqAttr.s_AttrLength = sizeof(SysPkgConfigFileInfo);

        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_IMPORT_EXCUTE_REQ, ReqAttrCnt, &SysReqAttr);
        if (FAILED(Result))
        {
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return GMI_FAIL;
}


GMI_RESULT SysGetLogInfo(uint16_t SessionId, uint32_t AuthValue, SysPkgLogInfoSearch *SysLogInfoSearch, SysPkgLogInfoInt *SysLogInfoInt, SysPkgLogInfo *SysLogInfo)
{
    GMI_RESULT   Result = GMI_SUCCESS;  
    SysAttr      SysReqAttr = {0};
    SysPkgLogInfoSearch SysLogInfoSch;
    SysAttr      *SysRspAttrPtr    = NULL;
    SysAttr      *SysRspAttrTmpPtr = NULL;
    uint16_t     RspAttrCnt        = 0;
    uint16_t     ReqAttrCnt        = 1;
    boolean_t    Exist             = false;

    if (NULL == SysLogInfoInt
       || NULL == SysLogInfo)
    {
       return GMI_INVALID_PARAMETER;
    }

    do
    {
        memset(&SysLogInfoSch, 0, sizeof(SysPkgLogInfoSearch));
        memcpy(&SysLogInfoSch, SysLogInfoSearch, sizeof(SysPkgLogInfoSearch));
        SysLogInfoSch.s_SelectMode = htonl(SysLogInfoSch.s_SelectMode);
        SysLogInfoSch.s_MajorType  = htonl(SysLogInfoSch.s_MajorType);
        SysLogInfoSch.s_MinorType  = htonl(SysLogInfoSch.s_MinorType);
        SysLogInfoSch.s_Offset     = htonl(SysLogInfoSch.s_Offset);
        SysLogInfoSch.s_MaxNum     = htonl(SysLogInfoSch.s_MaxNum);       

        SysReqAttr.s_Type      = TYPE_LOGINFO_SEARCH;
        SysReqAttr.s_Attr      = (void_t*)&SysLogInfoSch;
        SysReqAttr.s_AttrLength = sizeof(SysPkgLogInfoSearch);
        Result = SysGetCmdExcuteWithAttrs(SessionId, AuthValue, SYSCODE_GET_LOGINFO_REQ, ReqAttrCnt, &SysReqAttr, &RspAttrCnt, &SysRspAttrPtr);
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
        if (SysRspAttrTmpPtr->s_Type == TYPE_LOGINFO_INT
            && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgLogInfoInt))
        {
            memcpy(SysLogInfoInt, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
            SysLogInfoInt->s_Total = ntohl(SysLogInfoInt->s_Total);
            SysLogInfoInt->s_Count = ntohl(SysLogInfoInt->s_Count);
        }

        if (SysLogInfoInt->s_Count != RspAttrCnt-1)
        {
            SYS_CLIENT_ERROR("loginfo count %d,but loginfo attr is %d\n", SysLogInfoInt->s_Count, RspAttrCnt-1);
            break;
        }

        SysRspAttrTmpPtr++;
        for (int32_t i = 0, j = 0; i < SysLogInfoInt->s_Count; i++)
        {
            if (SysRspAttrTmpPtr->s_Type == TYPE_LOGINFO
                && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgLogInfo))
            {
                memcpy(&SysLogInfo[j], SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
                SysLogInfo[j].s_LogId     = ntohll(SysLogInfo[j].s_LogId);
                SysLogInfo[j].s_MajorType = ntohl(SysLogInfo[j].s_MajorType);
                SysLogInfo[j].s_MinorType = ntohl(SysLogInfo[j].s_MinorType);
                j++;
                Exist = true;
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
    while (0);

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}