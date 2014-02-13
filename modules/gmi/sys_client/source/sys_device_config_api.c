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

	Result = GMI_SystemReboot(&DaemonData, GMI_DAEMON_HEARTBEAT_STATUS_QUERY);
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

	Result = GMI_SystemReboot(&DaemonData, GMI_DAEMON_HEARTBEAT_STATUS_QUERY);
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


