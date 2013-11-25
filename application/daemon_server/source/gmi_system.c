/******************************************************************************
modules		:    Daemon
name		:    System.c
function	:    System init
author		:    minchao.wang
version		:    1.0.0.0.0
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/28/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
#include "gmi_includes.h"
#include "gmi_common.h"
#include "gmi_update.h"
#include "gmi_system.h"
#include "gmi_debug.h"
#include "gmi_hardware_monitor.h"
#include "gmi_struct.h"
#include "gmi_config.h"
#include "gmi_daemon_thread.h"
#include "gmi_heartbeat.h"
#include "gmi_server_inquiry.h"
#include "gmi_netconfig_api.h"
#include "gmi_net_manage.h"
#include "ipc_fw_v3.x_setting.h"
#include "ipc_fw_v3.x_resource.h"

//System Update Port
int32_t g_UpdatePort = GMI_DAEMON_UPDATE_SERVER_PORT;
//System application configs struct
SystemApplicationCfg_t           *g_pSystemApplicationCfg   = NULL;
//Application Open Flags
ApplicationOpenFlags               *g_ApplicationOpenFlags    = NULL;
//Application Daemon Register Flage
ApplicationRegisterFlags          *g_ApplicationRegisterFlags = NULL;
//Application Quit Flag
ApplicationQuitFlag                  *g_ApplicationQuitFlag        = NULL;
//Application Online Flag
ApplicationOnlineFlag               *g_ApplicationOnlineFlag   = NULL;
//System Ip addr info
IpInfo                                       *g_IpInfo = NULL;
//System IP change report flags
ApplicationIPChangeFlags        *g_ApplicationIPChangeFlags = NULL;


/*=======================================================
name				:	GMI_ApplicationQuit
function			:  Application Daemon exit funciton
algorithm implementation	:	no
global variable 		:	no
parameter declaration		:	  no
return				:	 no
---------------------------------------------------------------
modification	:
  date		version		 author 	modification
  7/12/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
void GMI_ApplicationQuit(int32_t AppId)
{
    switch (AppId)
    {
    case ALL_SERVER_ID:
        g_ApplicationQuitFlag->s_AllAppQuitFlag = true;
        break;
    case LOG_SERVER_ID:
        g_ApplicationQuitFlag->s_LogServerQuitFlag =  true;
        break;
    case MEDIA_SERVER_ID:
        g_ApplicationQuitFlag->s_MediaQuitFlag = true;
        break;
    case TRANSPORT_SERVER_ID:
        g_ApplicationQuitFlag->s_TransPortQuitFlag = true;
        break;
    case CONTROL_SERVER_ID:
        g_ApplicationQuitFlag->s_ControlQuitFlag = true;
        break;
    case GB28181_SERVER_ID:
        g_ApplicationQuitFlag->s_GbQuitFlag = true;
        break;
    case ONVIF_SERVER_ID:
        g_ApplicationQuitFlag->s_OnvifQuitFlag = true;
        break;
    case SDK_SERVER_ID:
        g_ApplicationQuitFlag->s_SdkQuitFlag = true;
        break;
    case AUTH_SERVER_ID:
        g_ApplicationQuitFlag->s_AuthQuitFlag = true;
        break;
    default:
        break;
    }
}

/*=======================================================
name				:	GMI_ApplicationUpdateQuit
function			:  Application Daemon exit funciton
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/29/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
void GMI_ApplicationUpdateQuit(void)
{
    int32_t Tmp=5;

    GMI_DeBugPrint("[%s][%d]GMI_ApplicationUpdateQuit Start Running ! ",__func__,__LINE__);
    if(g_ApplicationRegisterFlags->s_OnvifRegisterFlag)
    {
        g_ApplicationQuitFlag->s_OnvifQuitFlag = true;
        while (g_ApplicationQuitFlag->s_OnvifQuitFlag)
        {
            if(0 == Tmp)
            {
                break;
            }
            sleep(1);
            --Tmp;
        }
    }

    Tmp = 5;
    if(g_ApplicationRegisterFlags->s_ControlRegisterFlag)
    {
        g_ApplicationQuitFlag->s_ControlQuitFlag = true;
        while (g_ApplicationQuitFlag->s_ControlQuitFlag)
        {
            if(0 == Tmp)
            {
                break;
            }
            sleep(1);
            --Tmp;
        }
    }

    Tmp = 5;
    if(g_ApplicationRegisterFlags->s_MediaRegisterFlag)
    {
        g_ApplicationQuitFlag->s_MediaQuitFlag = true;
        while (g_ApplicationQuitFlag->s_MediaQuitFlag)
        {
            if(0 == Tmp)
            {
                break;
            }
            sleep(1);
            --Tmp;
        }
    }
}


GMI_RESULT GMI_StartUpSequenceInquiry(const SystemApplicationCfg_t *SystemApplicationCfg, int32_t SequenceId, int32_t *AppId)
{
    if(NULL == SystemApplicationCfg || SequenceId <= 0)
    {
        *AppId = 0;
        return GMI_INVALID_PARAMETER;
    }

    if(SequenceId == SystemApplicationCfg->s_LogServer)
    {
        *AppId = LOG_SERVER_ID;
        return GMI_SUCCESS;
    }

    if(SequenceId == SystemApplicationCfg->s_MediaServer)
    {
        *AppId = MEDIA_SERVER_ID;
        return GMI_SUCCESS;
    }


    if(SequenceId == SystemApplicationCfg->s_TransportServer)
    {
        *AppId = TRANSPORT_SERVER_ID;
        return GMI_SUCCESS;
    }

    if(SequenceId == SystemApplicationCfg->s_WebServer)
    {
        *AppId = WEB_SERVER_ID;
        return GMI_SUCCESS;
    }

    if(SequenceId == SystemApplicationCfg->s_ControlServer)
    {
        *AppId = CONTROL_SERVER_ID;
        return GMI_SUCCESS;
    }


    if(SequenceId == SystemApplicationCfg->s_Gb28181Server)
    {
        *AppId = GB28181_SERVER_ID;
        return GMI_SUCCESS;
    }

    if(SequenceId == SystemApplicationCfg->s_OnVifServer)
    {
        *AppId = ONVIF_SERVER_ID;
        return GMI_SUCCESS;
    }

    if(SequenceId == SystemApplicationCfg->s_SdkServer)
    {
        *AppId = SDK_SERVER_ID;
        return GMI_SUCCESS;
    }

    if(SequenceId == SystemApplicationCfg->s_AuthServer)
    {
        *AppId = AUTH_SERVER_ID;
        return GMI_SUCCESS;
    }

    if(SequenceId == SystemApplicationCfg->s_ConfigToolServer)
    {
        *AppId = CONFIG_TOOL_SERVER_ID;
        return GMI_SUCCESS;
    }

    return GMI_FAIL;

}

GMI_RESULT GMI_RunningServer(int32_t AppId, boolean_t *OpenFlags)
{
    char_t CmdBuffer[MAX_BUFFER_LENGTH];
    memset(CmdBuffer, 0, sizeof(CmdBuffer));

    switch (AppId)
    {
    case LOG_SERVER_ID:
        strcpy(CmdBuffer,LOG_SERVER_APPLICATION);
        break;
    case MEDIA_SERVER_ID:
        strcpy(CmdBuffer,MEDIA_SERVER_APPLICATION);
        break;
    case TRANSPORT_SERVER_ID:
        strcpy(CmdBuffer,TRANSPORT_SERVER_APPLICATION);
        break;
    case CONTROL_SERVER_ID:
        strcpy(CmdBuffer,CONTROL_SERVER_APPLICATION);
        break;
    case GB28181_SERVER_ID:
        strcpy(CmdBuffer,GB28181_SERVER_APPLICATION);
        break;
    case ONVIF_SERVER_ID:
        strcpy(CmdBuffer,ONVIF_SERVER_APPLICATON);
        break;
    case SDK_SERVER_ID:
        strcpy(CmdBuffer,SDK_SERVER_APPLICATION);
        break;
    case AUTH_SERVER_ID:
        strcpy(CmdBuffer,AUTH_SERVER_APPLICATION);
        break;
    case CONFIG_TOOL_SERVER_ID:
        strcpy(CmdBuffer,CONFIG_TOOL_SERVER_APPLICATION);
        break;
    default:
        return GMI_NOT_SUPPORT;
    }

    if (system(CmdBuffer) < 0)
    {
        *OpenFlags = false;
        GMI_DeBugPrint("[%s][%d]CmdBuffer  Start fail ! ",CmdBuffer,__func__,__LINE__);
        return GMI_FAIL;
    }
    else
    {
        *OpenFlags = true;
    }

    return GMI_SUCCESS;
}

/*==============================================================
name				:	GMI_StartUpSystemService
function			:  System uninit function
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    FAIL:  ERROR
                                     SUCCESS : OK
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/28/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_StartUpSystemService(const SystemApplicationCfg_t *SystemApplicationCfg, int32_t SequenceId)
{
    if(NULL == SystemApplicationCfg)
    {
        return GMI_INVALID_PARAMETER;
    }

    int32_t AppId =-1;
    GMI_RESULT Result = GMI_FAIL;

    Result = GMI_StartUpSequenceInquiry(SystemApplicationCfg, SequenceId, &AppId);
    if(FAILED(Result))
    {
        GMI_DeBugPrint("[%s][%d] GMI_StartUpSequenceInquiry Fail ! ",__func__,__LINE__);
    }

    switch (AppId)
    {
    case LOG_SERVER_ID:
        Result = GMI_RunningServer(LOG_SERVER_ID, &(g_ApplicationOpenFlags->s_LogServerOpenFlags));
        if(SUCCEEDED(Result))
        {
            GMI_DeBugPrint("[%s][%d]logserver  Start Running ! ",__func__,__LINE__);
        }
        break;
    case MEDIA_SERVER_ID:
        Result = GMI_RunningServer(MEDIA_SERVER_ID, &(g_ApplicationOpenFlags->s_MediaServerOpenFlags));
        if(SUCCEEDED(Result))
        {
            GMI_DeBugPrint("[%s][%d]Media Server  Start Running ! ",__func__,__LINE__);
        }
        break;
    case TRANSPORT_SERVER_ID:
        Result = GMI_RunningServer(TRANSPORT_SERVER_ID, &(g_ApplicationOpenFlags->s_TransportServerOpenFlags));
        if(SUCCEEDED(Result))
        {
            GMI_DeBugPrint("[%s][%d]Transport Server  Start Running ! ",__func__,__LINE__);
        }
        break;
    case CONTROL_SERVER_ID:
        Result = GMI_RunningServer(CONTROL_SERVER_ID, &(g_ApplicationOpenFlags->s_ControlServerOpenFlags));
        if(SUCCEEDED(Result))
        {
            GMI_DeBugPrint("[%s][%d]Control  Server Start Running ! ",__func__,__LINE__);
        }
        break;
    case GB28181_SERVER_ID:
        Result = GMI_RunningServer(GB28181_SERVER_ID, &(g_ApplicationOpenFlags->s_Gb28181ServerOpenFlags));
        if(SUCCEEDED(Result))
        {
            GMI_DeBugPrint("[%s][%d]Gb28181  Server Start Running ! ",__func__,__LINE__);
        }
        break;
    case ONVIF_SERVER_ID:
        Result = GMI_RunningServer(ONVIF_SERVER_ID, &(g_ApplicationOpenFlags->s_OnVifServerOpenFlags));
        if(SUCCEEDED(Result))
        {
            GMI_DeBugPrint("[%s][%d]OnVif  Server Start Running ! ",__func__,__LINE__);
        }
        break;
    case SDK_SERVER_ID:
        Result = GMI_RunningServer(SDK_SERVER_ID, &(g_ApplicationOpenFlags->s_SdkServerOpenFlags));
        if(SUCCEEDED(Result))
        {
            GMI_DeBugPrint("[%s][%d]Sdk  Server Start Running ! ",__func__,__LINE__);
        }
        break;
    case AUTH_SERVER_ID:
        Result = GMI_RunningServer(AUTH_SERVER_ID, &(g_ApplicationOpenFlags->s_AuthServerOpenFlags));
        if(SUCCEEDED(Result))
        {
            GMI_DeBugPrint("[%s][%d]auth  Server Start Running ! ",__func__,__LINE__);
        }
        break;
    case CONFIG_TOOL_SERVER_ID:
        Result = GMI_RunningServer(CONFIG_TOOL_SERVER_ID, &(g_ApplicationOpenFlags->s_ConfigToolOpenFlags));
        if(SUCCEEDED(Result))
        {
            GMI_DeBugPrint("[%s][%d]Config Tool  Server Start Running ! ",__func__,__LINE__);
        }
        break;
    default:
        break;
    }

    return GMI_SUCCESS;
}

/*=======================================================
name				:	GMI_SystemInitial
function			:  System initial function
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    FAIL:  ERROR
                                     SUCCESS : OK
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/28/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_SystemInitial(void)
{
    GMI_RESULT Ret = GMI_FAIL;

    //Init Debug log File
    GMI_DebugLog2FileInitial();
    DAEMON_PRINT_LOG(INFO,"GMI_MemoryAllocForLoadCFG  Start!! ");

    //memory alloc for config
    Ret = GMI_MemoryAllocForLoadCFG();
    if (FAILED(Ret))
    {
        GMI_DeBugPrint("[%s][%d]GMI_MemoryAllocForLoadCFG Error! ",__func__,__LINE__);
        return Ret;
    }

    // detect hardware load sensor driver
    DAEMON_PRINT_LOG(INFO,"GMI_HardwareInit Start!! ");
    Ret = GMI_HardwareInit();
    if (FAILED(Ret))
    {
        GMI_DeBugPrint("[%s][%d]GMI_HardwareInit Start Error ! ",__func__,__LINE__);
        return Ret;
    }

    GMI_DeBugPrint("[%s]GMI_SystemInitial Start Running  Daemon_server SVN = [%d]! ",__func__,SVN);

    Ret = GMI_NetManageInit();
    if (FAILED(Ret))
    {
        GMI_DeBugPrint("[%s][%d]GMI_NetManageInit Start Error ! ",__func__,__LINE__);
        return Ret;
    }

    //load application deamon running config
    DAEMON_PRINT_LOG(INFO,"GMI_LoadApplicationCfg Start!! ");
    Ret = GMI_LoadApplicationCfg();
    if (FAILED(Ret))
    {
        GMI_DeBugPrint("[%s][%d]GMI_LoadApplicationCfg  Error ! ",__func__,__LINE__);
        return Ret;
    }

    // init daemon thread for application
    DAEMON_PRINT_LOG(INFO,"GMI_DaemonInit Start!! ");
    Ret = GMI_DaemonInit();
    if (FAILED(Ret))
    {
        GMI_DeBugPrint("[%s][%d]GMI_DaemonInit	Error ! ",__func__,__LINE__);
        return Ret;
    }

    //Start up system server
    DAEMON_PRINT_LOG(INFO,"GMI_StartUpSystemService Start!! ");
    Ret = GMI_SystemAppRunning(g_pSystemApplicationCfg, g_ApplicationRegisterFlags, 1);
    if (FAILED(Ret))
    {
        GMI_DeBugPrint("[%s][%d]GMI_StartUpSystemService	Error ! ",__func__,__LINE__);
    }

    //Create Task for system update
    DAEMON_PRINT_LOG(INFO,"GMI_SystemUpdateServerInit Start!! ");
    Ret =GMI_SystemUpdateServerInit();
    if (FAILED(Ret))
    {
        GMI_DeBugPrint("[%s][%d] GMI_SystemUpdateServerInit	 Error ! ",__func__,__LINE__);
    }

    //SDK Inquiry system server status thread
    DAEMON_PRINT_LOG(INFO,"GMI_CreateTask GMI_InitServerQuiry Start!! ");
    Ret = GMI_InitServerQuiry();
    if (FAILED(Ret))
    {
        GMI_DeBugPrint("[%s][%d] GMI_InitServerQuiry 	Error ! ",__func__,__LINE__);
    }

    return GMI_SUCCESS;
}

/*==============================================================
name				:	GMI_SystemUninitial
function			:  System uninit function
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    FAIL:  ERROR
                                     SUCCESS : OK
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/28/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
void GMI_SystemUninitial(void)
{
    GMI_DaemonUnInit();
    GMI_UnInitServerQuiry();
    MemoryFreeForLoadCFG();
    return ;
}

