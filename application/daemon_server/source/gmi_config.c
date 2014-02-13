/******************************************************************************
modules		:    Daemon
name		:    gmi_config.c
function	:    System configs, malloc
author		:    minchao.wang
version		:    1.0.0.0.0
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/28/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
#include "gmi_includes.h"
#include "gmi_debug.h"
#include "gmi_system.h"
#include "gmi_struct.h"
#include "gmi_config.h"
#include "gmi_daemon_thread.h"
#include "gmi_update.h"
#include "gmi_hardware_monitor.h"
#include "gmi_config_api.h"
#include "ipc_fw_v3.x_setting.h"

//System hardware monitor config
HardwareConfig                          *g_Hardware = NULL;
//System update struct data
SystemUpdateData                     *g_UpMessage = NULL;
//System daemon time
ApplicationDaemonTime             *g_ApplicationDaemonTime = NULL;

/*==============================================================
name				:	WriteApplicationCfg
function			:  Set Daemon configs to DefaultCfg
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    FAIL:  -1
                                    SUCCESS : 0
---------------------------------------------------------------
modification	:
   date		version		 author 	modification
   1/28/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT WriteDefaultApplicationCfg(const char_t * FileName, const char_t *ItemPath, SystemApplicationCfg_t* DefaultApplicationCfg, SystemApplicationCfg_t* SystemApplicationCfg)
{

    if(NULL == FileName || NULL == ItemPath || NULL == DefaultApplicationCfg)
    {
        return GMI_INVALID_PARAMETER;
    }
    DAEMON_PRINT_LOG(INFO,"WriteDefaultApplicationCfg  Start!! ");

    GMI_RESULT Ret = GMI_FAIL;
    FD_HANDLE  Handle = NULL;

    Ret = GMI_XmlOpen(FileName, &Handle);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_XmlOpen xml file Error!");
        return Ret;
    }

    char_t	Key[MAX_BUFFER_LENGTH]= {"0"};
    int32_t  Value;

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "log_server_run");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_LogServer , &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_LogServer = Value;
    }
    else
    {
        SystemApplicationCfg->s_LogServer = DefaultApplicationCfg->s_LogServer ;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "tranport_server_run");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_TransportServer, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_TransportServer = Value;
    }
    else
    {
        SystemApplicationCfg->s_TransportServer = DefaultApplicationCfg->s_TransportServer ;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "system_server_run");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_ControlServer, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_ControlServer = Value;
    }
    else
    {
        SystemApplicationCfg->s_ControlServer = DefaultApplicationCfg->s_ControlServer ;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "onvif_run");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_OnVifServer, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_OnVifServer = Value;
    }
    else
    {
        SystemApplicationCfg->s_OnVifServer = DefaultApplicationCfg->s_OnVifServer ;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "gb_server_run");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_Gb28181Server, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_Gb28181Server = Value;
    }
    else
    {
        SystemApplicationCfg->s_Gb28181Server = DefaultApplicationCfg->s_Gb28181Server ;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "sdk_server_run");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_SdkServer, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_SdkServer = Value;
    }
    else
    {
        SystemApplicationCfg->s_SdkServer = DefaultApplicationCfg->s_SdkServer ;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "auth_server_run");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_AuthServer, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_AuthServer = Value;
    }
    else
    {
        SystemApplicationCfg->s_AuthServer = DefaultApplicationCfg->s_AuthServer ;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "config_tool_run");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_ConfigToolServer, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_ConfigToolServer = Value;
    }
    else
    {
        SystemApplicationCfg->s_ConfigToolServer = DefaultApplicationCfg->s_ConfigToolServer ;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "log_server_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_LogServerTimer, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_LogServerTimer= Value;
    }
    else
    {
        SystemApplicationCfg->s_LogServerTimer = DefaultApplicationCfg->s_LogServerTimer ;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "media_center_server_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_MediaServerTimer, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_MediaServerTimer= Value;
    }
    else
    {
        SystemApplicationCfg->s_MediaServerTimer = DefaultApplicationCfg->s_MediaServerTimer ;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "tranport_server_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_TransportServerTimer, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_TransportServerTimer= Value;
    }
    else
    {
        SystemApplicationCfg->s_TransportServerTimer = DefaultApplicationCfg->s_TransportServerTimer ;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "system_server_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_ControlServerTimer, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_ControlServerTimer= Value;
    }
    else
    {
        SystemApplicationCfg->s_ControlServerTimer = DefaultApplicationCfg->s_ControlServerTimer ;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "onvif_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_OnvifServerTimer, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_OnvifServerTimer = Value;
    }
    else
    {
        SystemApplicationCfg->s_OnvifServerTimer = DefaultApplicationCfg->s_OnvifServerTimer ;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "gb_server_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_GbServerTimer, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_GbServerTimer = Value;
    }
    else
    {
        SystemApplicationCfg->s_GbServerTimer = DefaultApplicationCfg->s_GbServerTimer ;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "sdk_server_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_SdkServerTimer, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_SdkServerTimer = Value;
    }
    else
    {
        SystemApplicationCfg->s_SdkServerTimer = DefaultApplicationCfg->s_SdkServerTimer ;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "auth_server_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_AuthServerTimer, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_AuthServerTimer = Value;
    }
    else
    {
        SystemApplicationCfg->s_AuthServerTimer = DefaultApplicationCfg->s_AuthServerTimer ;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "config_tool_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, DefaultApplicationCfg->s_ConfigToolServertimer, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
    {
        SystemApplicationCfg->s_ConfigToolServertimer = Value;
    }
    else
    {
        SystemApplicationCfg->s_ConfigToolServertimer = DefaultApplicationCfg->s_ConfigToolServertimer ;
    }

    GMI_DeBugPrint("[%s][%d]s_ConfigToolServertimer = %d ",__func__,__LINE__,SystemApplicationCfg->s_ConfigToolServertimer);

    Ret = GMI_XmlFileSave(Handle);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_XmlFileSave xml file Error!!");
        return Ret;
    }

    return Ret;

}

/*==============================================================
name				:	ReadApplicationCfg
function			:  read application config
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    FAIL:  -1
                                    SUCCESS : 0
---------------------------------------------------------------
modification	:
   date		version		 author 	modification
   1/28/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT ReadApplicationCfg(const char_t * FileName, const char_t *ItemPath, SystemApplicationCfg_t* SystemApplicationCfg)
{

    if(NULL == FileName || NULL== ItemPath)
    {
        return GMI_INVALID_PARAMETER;
    }

    DAEMON_PRINT_LOG(INFO,"ReadApplicationCfg  Start!! ");

    GMI_RESULT Ret = GMI_FAIL;
    FD_HANDLE  Handle = NULL;

    Ret = GMI_XmlOpen(FileName, &Handle);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_XmlOpen xml file Error!");
        return Ret;
    }

    char_t  Key[MAX_BUFFER_LENGTH] = {"0"};
    int32_t  Value = 0;

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "log_server_run");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, 2, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
        SystemApplicationCfg->s_LogServer = Value;
    else
        return Ret;

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "tranport_server_run");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, 5, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
        SystemApplicationCfg->s_TransportServer = Value;
    else
        return Ret;

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "system_server_run");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, 4, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
        SystemApplicationCfg->s_ControlServer = Value;
    else
        return Ret;


    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "onvif_run");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, 6, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
        SystemApplicationCfg->s_OnVifServer = Value;
    else
        return Ret;

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "gb_server_run");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, 0, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
        SystemApplicationCfg->s_Gb28181Server = Value;
    else
        return Ret;

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "sdk_server_run");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, 7, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
        SystemApplicationCfg->s_SdkServer = Value;
    else
        return Ret;

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "auth_server_run");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, 3, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
        SystemApplicationCfg->s_AuthServer = Value;
    else
        return Ret;

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "config_tool_run");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, 1, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
        SystemApplicationCfg->s_ConfigToolServer = Value;
    else
        return Ret;

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "log_server_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, MAX_RETURN_VALUE , &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
        SystemApplicationCfg->s_LogServerTimer= Value;
    else
        return Ret;

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "tranport_server_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, MAX_RETURN_VALUE, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
        SystemApplicationCfg->s_TransportServerTimer= Value;
    else
        return Ret;


    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "system_server_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, MAX_RETURN_VALUE, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
        SystemApplicationCfg->s_ControlServerTimer= Value;
    else
        return Ret;


    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "onvif_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, MAX_RETURN_VALUE, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
        SystemApplicationCfg->s_OnvifServerTimer = Value;
    else
        return Ret;

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "gb_server_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, MAX_RETURN_VALUE, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
        SystemApplicationCfg->s_GbServerTimer = Value;
    else
        return Ret;

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "sdk_server_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, MAX_RETURN_VALUE, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
        SystemApplicationCfg->s_SdkServerTimer = Value;
    else
        return Ret;

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "auth_server_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, MAX_RETURN_VALUE, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
        SystemApplicationCfg->s_AuthServerTimer = Value;
    else
        return Ret;

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "config_tool_timer");
    Ret = GMI_XmlRead(Handle, ItemPath, Key, MAX_RETURN_VALUE, &Value,GMI_CONFIG_READ_WRITE);
    if (SUCCEEDED(Ret))
        SystemApplicationCfg->s_ConfigToolServertimer = Value;
    else
        return Ret;

    Ret = GMI_XmlFileSave(Handle);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_XmlFileSave xml file Error!!");
        return Ret;
    }

    return Ret;
}

/*==============================================================
name				:	GMI_LoadCfg
function			:  load daemon application config
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    FAIL:  -1
                                    SUCCESS : 0
---------------------------------------------------------------
modification	:
   date		version		 author 	modification
   1/28/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_LoadApplicationCfg(void)
{
    DAEMON_PRINT_LOG(INFO,"GMI_LoadApplicationCfg  Start!! ");

    SystemApplicationCfg_t  SystemApplicationCfg = {0};
    SystemApplicationCfg.s_LogServer                                            = 2;    //must be running
    SystemApplicationCfg.s_MediaServer                                         = 0; //must be running
    SystemApplicationCfg.s_TransportServer                                   = 5; //now this application is not write
    SystemApplicationCfg.s_ControlServer                                       = 4; //must be running
    SystemApplicationCfg.s_OnVifServer                                          = 6;   //must be running
    SystemApplicationCfg.s_Gb28181Server                                    = 0;  //Gb server is choice
    SystemApplicationCfg.s_WebServer                                           = 0;  //web server not running
    SystemApplicationCfg.s_SdkServer                                             = 7;  //sdk server not running
    SystemApplicationCfg.s_AuthServer                                           = 3;  //auth server not running
    SystemApplicationCfg.s_ConfigToolServer                                  = 1;  //auth server not running

    SystemApplicationCfg.s_LogServerTimer                                    = MAX_RETURN_VALUE;
    SystemApplicationCfg.s_MediaServerTimer                                 = MAX_RETURN_VALUE;
    SystemApplicationCfg.s_TransportServerTimer                           = MAX_RETURN_VALUE;
    SystemApplicationCfg.s_ControlServerTimer                               = MAX_RETURN_VALUE;
    SystemApplicationCfg.s_OnvifServerTimer                                  = MAX_RETURN_VALUE;
    SystemApplicationCfg.s_GbServerTimer                                      = MAX_RETURN_VALUE;
    SystemApplicationCfg.s_WebServerTimer                                   = MAX_RETURN_VALUE;
    SystemApplicationCfg.s_SdkServerTimer                                    = MAX_RETURN_VALUE;
    SystemApplicationCfg.s_AuthServerTimer                                   = MAX_RETURN_VALUE;
    SystemApplicationCfg.s_ConfigToolServertimer                          = MAX_RETURN_VALUE;

    GMI_RESULT Ret = GMI_FAIL;
    Ret = GMI_FileExists(GMI_DAEMON_CONFIG_FILE);
    if (FAILED(Ret))
    {
        //System first rnning is application running is not exists, System will be write file.
        Ret = WriteDefaultApplicationCfg(GMI_DAEMON_CONFIG_FILE, GMI_DAEMON_PATH, &SystemApplicationCfg,g_pSystemApplicationCfg);
        if (FAILED(Ret))
        {
            DAEMON_PRINT_LOG(ERROR,"WriteDefaultApplicationCfg Error! ! ");
            return Ret;
        }
    }
    else if (SUCCEEDED(Ret))
    {
        //System running will be read applicatoion file.
        Ret = ReadApplicationCfg(GMI_DAEMON_CONFIG_FILE, GMI_DAEMON_PATH, g_pSystemApplicationCfg);
        if (FAILED(Ret))
        {
            DAEMON_PRINT_LOG(ERROR,"ReadApplicationCfg Error! ! ");
            WriteDefaultApplicationCfg(GMI_DAEMON_CONFIG_FILE, GMI_DAEMON_PATH, &SystemApplicationCfg,g_pSystemApplicationCfg);
        }

        DAEMON_PRINT_LOG(INFO,"ReadApplicationCfg  SUCCESS!! ");
    }

    g_ApplicationDaemonTime->s_LogDaeminTime            = g_pSystemApplicationCfg->s_LogServerTimer;
    g_ApplicationDaemonTime->s_MediaDaeminTime         = g_pSystemApplicationCfg->s_MediaServerTimer;
    g_ApplicationDaemonTime->s_TransPortDaeminTime   = g_pSystemApplicationCfg->s_TransportServerTimer;
    g_ApplicationDaemonTime->s_ControlDaeminTime       = g_pSystemApplicationCfg->s_ControlServerTimer;
    g_ApplicationDaemonTime->s_OnvifDaeminTime          = g_pSystemApplicationCfg->s_OnvifServerTimer;
    g_ApplicationDaemonTime->s_GbDaeminTime              = g_pSystemApplicationCfg->s_GbServerTimer;
    g_ApplicationDaemonTime->s_SdkDaeminTime             = g_pSystemApplicationCfg->s_SdkServerTimer;
    g_ApplicationDaemonTime->s_AuthDaeminTime           = g_pSystemApplicationCfg->s_AuthServerTimer;
    g_ApplicationDaemonTime->s_ConfigToolDaeminTime  = g_pSystemApplicationCfg->s_ConfigToolServertimer;


    return Ret;
}

/*==============================================================
name				:	GMI_MemoryAllocForLoadCFG
function			:  System Memory alloc
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    FAIL:  -1
                                     SUCCESS : 0
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/28/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_MemoryAllocForLoadCFG(void)
{

    DAEMON_PRINT_LOG(INFO,"GMI_MemoryAllocForLoadCFG  Start!! ");
    g_pSystemApplicationCfg = (SystemApplicationCfg_t*)malloc(sizeof(SystemApplicationCfg_t));
    if (NULL == g_pSystemApplicationCfg)
    {
        DAEMON_PRINT_LOG(ERROR,"Malloc  g_pSystemApplicationCfg Error!! ");
        goto Exit;
    }
    memset(g_pSystemApplicationCfg, 0, sizeof(g_pSystemApplicationCfg));

    g_UpMessage = (SystemUpdateData*)malloc(sizeof(SystemUpdateData));
    if (NULL == g_UpMessage)
    {
        DAEMON_PRINT_LOG(ERROR,"Malloc  g_UpMessage Error!! ");
        goto Exit;
    }
    memset(g_UpMessage, 0, sizeof(g_UpMessage));

    g_Hardware = (HardwareConfig*) malloc(sizeof(HardwareConfig));
    if (NULL == g_Hardware)
    {
        DAEMON_PRINT_LOG(ERROR,"Malloc  g_Hardware Error!! ");

        goto Exit;
    }
    memset(g_Hardware, 0, sizeof(g_Hardware));

    g_ApplicationOpenFlags = (ApplicationOpenFlags*) malloc(sizeof(ApplicationOpenFlags));
    if (NULL == g_ApplicationOpenFlags)
    {
        DAEMON_PRINT_LOG(ERROR,"Malloc  g_ApplicationOpenFlags Error!! ");

        goto Exit;
    }
    g_ApplicationOpenFlags->s_LogServerOpenFlags                 = false;
    g_ApplicationOpenFlags->s_MediaServerOpenFlags              = false;
    g_ApplicationOpenFlags->s_ControlServerOpenFlags            = false;
    g_ApplicationOpenFlags->s_TransportServerOpenFlags        = false;
    g_ApplicationOpenFlags->s_OnVifServerOpenFlags               = false;
    g_ApplicationOpenFlags->s_Gb28181ServerOpenFlags         = false;
    g_ApplicationOpenFlags->s_SdkServerOpenFlags                  = false;
    g_ApplicationOpenFlags->s_AuthServerOpenFlags                = false;
    g_ApplicationOpenFlags->s_ConfigToolOpenFlags                 = false;

    g_ApplicationRegisterFlags = (ApplicationRegisterFlags*) malloc(sizeof(ApplicationRegisterFlags));
    if (NULL ==  g_ApplicationRegisterFlags)
    {
        DAEMON_PRINT_LOG(ERROR,"Malloc  g_ApplicationRegisterFlags Error!! ");

        goto Exit;
    }
    g_ApplicationRegisterFlags->s_LogRegisterFlag                     = false;
    g_ApplicationRegisterFlags->s_MediaRegisterFlag                  = false;
    g_ApplicationRegisterFlags->s_ControlRegisterFlag               = false;
    g_ApplicationRegisterFlags->s_TransPortRegisterFlag           = false;
    g_ApplicationRegisterFlags->s_OnvifRegisterFlag                   = false;
    g_ApplicationRegisterFlags->s_GbRegisterFlag                      = false;
    g_ApplicationRegisterFlags->s_SdkRegisterFlag                     = false;
    g_ApplicationRegisterFlags->s_AuthRegisterFlag                   = false;
    g_ApplicationRegisterFlags->s_ConfigToolRegisterFlag          = false;

    g_ApplicationDaemonTime = (ApplicationDaemonTime*)malloc(sizeof(ApplicationDaemonTime));
    if (NULL == g_ApplicationDaemonTime)
    {
        DAEMON_PRINT_LOG(ERROR,"Malloc  g_ApplicationDaemonTime Error!! ");
        goto Exit;
    }
    memset(g_ApplicationDaemonTime, 0, sizeof(g_ApplicationDaemonTime));

    //Application Quit Flag
    g_ApplicationQuitFlag= (ApplicationQuitFlag*)malloc(sizeof(ApplicationQuitFlag));
    if (NULL == g_ApplicationQuitFlag)
    {
        DAEMON_PRINT_LOG(ERROR,"Malloc  g_ApplicationQuitFlag Error!! ");
        goto Exit;
    }
    g_ApplicationQuitFlag->s_AllAppQuitFlag 		= false;
    g_ApplicationQuitFlag->s_LogServerQuitFlag 	= false;
    g_ApplicationQuitFlag->s_MediaQuitFlag		= false;
    g_ApplicationQuitFlag->s_TransPortQuitFlag	= false;
    g_ApplicationQuitFlag->s_ControlQuitFlag	= false;
    g_ApplicationQuitFlag->s_GbQuitFlag		= false;
    g_ApplicationQuitFlag->s_OnvifQuitFlag		= false;
    g_ApplicationQuitFlag->s_SdkQuitFlag		= false;
    g_ApplicationQuitFlag->s_AuthQuitFlag		= false;
    g_ApplicationQuitFlag->s_ConfigToolQuitFlag   = false;


    g_ApplicationOnlineFlag = (ApplicationOnlineFlag*)malloc(sizeof(ApplicationOnlineFlag));
    if (NULL == g_ApplicationOnlineFlag)
    {
        DAEMON_PRINT_LOG(ERROR,"Malloc  g_ApplicationOnlineFlag Error!! ");
        goto Exit;
    }
    g_ApplicationOnlineFlag->s_AuthOnlineFlag            = false;
    g_ApplicationOnlineFlag->s_LogServerOnlineFlag    = false;
    g_ApplicationOnlineFlag->s_MediaOnlineFlag           = false;
    g_ApplicationOnlineFlag->s_ControlOnlineFlag        = false;
    g_ApplicationOnlineFlag->s_OnvifOnlineFlag            = false;
    g_ApplicationOnlineFlag->s_GbOnlineFlag                = false;
    g_ApplicationOnlineFlag->s_SdkOnlineFlag               = false;
    g_ApplicationOnlineFlag->s_TransPortOnlineFlag     = false;
    g_ApplicationOnlineFlag->s_ControlOnlineFlag         = false;

    g_IpInfo = (IpInfo*)malloc(sizeof(IpInfo));
    if(NULL== g_IpInfo)
    {
        DAEMON_PRINT_LOG(ERROR,"Malloc  g_IpInfo Error!! ");
        goto Exit;
    }

    g_ApplicationIPChangeFlags = (ApplicationIPChangeFlags*)malloc(sizeof(ApplicationIPChangeFlags));
    if (NULL == g_ApplicationIPChangeFlags)
    {
        DAEMON_PRINT_LOG(ERROR,"Malloc  g_ApplicationIPChangeFlags Error!! ");
        goto Exit;
    }
    memset(g_ApplicationIPChangeFlags, 0, sizeof(ApplicationIPChangeFlags));
    g_ApplicationIPChangeFlags->s_LogServerIPChangeFlags = false;
    g_ApplicationIPChangeFlags->s_AuthServerIPChangeFlags = false;
    g_ApplicationIPChangeFlags->s_MediaServerIPChangeFlags = false;
    g_ApplicationIPChangeFlags->s_ControlServerIPChangeFlags = false;
    g_ApplicationIPChangeFlags->s_TransportServerIPChangeFlags = false;
    g_ApplicationIPChangeFlags->s_OnVifServerIPChangeFlags = false;
    g_ApplicationIPChangeFlags->s_SdkServerIPChangeFlags = false;
    g_ApplicationIPChangeFlags->s_ConfigToolIPChangeFlags = false;
    g_ApplicationIPChangeFlags->s_Gb28181ServerIPChangeFlags = false;


    DAEMON_PRINT_LOG(INFO,"GMI_MemoryAllocForLoadCFG  End!! ");

    return GMI_SUCCESS;

Exit:
    DAEMON_PRINT_LOG(ERROR,"GMI_MemoryAllocForLoadCFG  Error!! ");
    MemoryFreeForLoadCFG();
    return GMI_FAIL;
}

/*==============================================================
name				:	MemoryFreeForLoadCFG
function			:  System Free Memory
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:
---------------------------------------------------------------
modification	:
   date		version		 author 	modification
   1/28/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
void MemoryFreeForLoadCFG(void)
{
    if (NULL != g_pSystemApplicationCfg)
    {
        free(g_pSystemApplicationCfg);
        g_pSystemApplicationCfg = NULL;
    }

    if (NULL != g_UpMessage)
    {
        free(g_UpMessage);
        g_UpMessage = NULL;
    }

    if (NULL != g_Hardware)
    {
        free(g_Hardware);
        g_Hardware = NULL;
    }

    if (NULL != g_ApplicationOpenFlags)
    {
        free(g_ApplicationOpenFlags);
        g_ApplicationOpenFlags = NULL;
    }

    if (NULL != g_ApplicationRegisterFlags)
    {
        free(g_ApplicationRegisterFlags);
        g_ApplicationRegisterFlags = NULL;
    }

    if (NULL != g_ApplicationQuitFlag)
    {
        free(g_ApplicationQuitFlag);
        g_ApplicationQuitFlag = NULL;
    }

    if(NULL != g_ApplicationOnlineFlag)
    {
        free(g_ApplicationOnlineFlag);
        g_ApplicationOnlineFlag = NULL;
    }

    if(NULL !=  g_IpInfo)
    {
        free(g_IpInfo);
        g_IpInfo = NULL;
    }

    if (NULL != g_ApplicationIPChangeFlags)
    {
        free(g_ApplicationIPChangeFlags);
        g_ApplicationIPChangeFlags = NULL;
    }

    DAEMON_PRINT_LOG(INFO,"MemoryFreeForLoadCFG  EXIT!! ");
    return;
}

