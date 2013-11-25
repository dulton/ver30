/******************************************************************************
modules		:    Daemon
name		:    gmi_daemon_thread.c
function	:    deal with sotf watch dog function, if timeout ,server will be restart
author		:    minchao.wang
version		:    1.0.0.0.0
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/29/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
#include "gmi_includes.h"
#include "gmi_common.h"
#include "gmi_daemon_thread.h"
#include "gmi_heartbeat.h"
#include "gmi_debug.h"
#include "gmi_system.h"
#include "gmi_struct.h"
#include "gmi_config.h"
#include "gmi_update.h"


static FD_HANDLE l_sock;

/*=======================================================
name				:	GMI_DaemonMessageServer
function			:  Init daemon SysLog Server thread
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/29/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/

static void* GMI_DaemonMessageServer(void *param)
{

    pthread_detach(pthread_self());

    for (;;)
    {
        GMI_QueryHeartbeatMassge(l_sock);
    }

    pthread_exit(NULL);

    return (void *)0;
}

GMI_RESULT SystemRestartServer(const int32_t AppId, const int32_t MaxTimer, int32_t *DaemonTime, boolean_t *OnlineFlag)
{

    int32_t TimeNumber=0;
    TimeNumber = *DaemonTime;

    if(TimeNumber < 0)
    {
        static char_t CmdBuffer[MAX_BUFFER_LENGTH];
        static char_t CmdBufferStart[MAX_BUFFER_LENGTH];

        memset(CmdBuffer,0,sizeof(CmdBuffer));
        memset(CmdBufferStart,0,sizeof(CmdBufferStart));

        switch (AppId)
        {
        case LOG_SERVER_ID:
            strcpy(CmdBuffer, LOG_SERVER_APPLICATION_KILL);
            strcpy(CmdBufferStart,LOG_SERVER_APPLICATION);
            break;
        case MEDIA_SERVER_ID:
            strcpy(CmdBuffer, MEDIA_SERVER_APPLICATION_KILL);
            strcpy(CmdBufferStart,MEDIA_SERVER_APPLICATION);
            break;
        case TRANSPORT_SERVER_ID:
            strcpy(CmdBuffer, TRANSPORT_SERVER_APPLICATION_KILL);
            strcpy(CmdBufferStart,TRANSPORT_SERVER_APPLICATION);
            break;
        case CONTROL_SERVER_ID:
            strcpy(CmdBuffer, CONTROL_SERVER_APPLICATION_KILL);
            strcpy(CmdBufferStart,CONTROL_SERVER_APPLICATION);
            break;
        case GB28181_SERVER_ID:
            strcpy(CmdBuffer, GB28181_SERVER_APPLICATION_KILL);
            strcpy(CmdBufferStart,GB28181_SERVER_APPLICATION);
            break;
        case ONVIF_SERVER_ID:
            strcpy(CmdBuffer, ONVIF_SERVER_APPLICATON_KILL);
            strcpy(CmdBufferStart,ONVIF_SERVER_APPLICATON);
            break;
        case SDK_SERVER_ID:
            strcpy(CmdBuffer, SDK_SERVER_APPLICATION_KILL);
            strcpy(CmdBufferStart,SDK_SERVER_APPLICATION);
            break;
        case AUTH_SERVER_ID:
            strcpy(CmdBuffer, AUTH_SERVER_APPLICATION_KILL);
            strcpy(CmdBufferStart,AUTH_SERVER_APPLICATION);
            break;
        case CONFIG_TOOL_SERVER_ID:
            strcpy(CmdBuffer, CONFIG_TOOL_SERVER_APPLICATION_KILL);
            strcpy(CmdBufferStart,CONFIG_TOOL_SERVER_APPLICATION);
            break;
        default:
            return GMI_NOT_SUPPORT;
        }

        *OnlineFlag = false;
        *DaemonTime = MaxTimer;

        GMI_DeBugPrint("[%s][%d]  %s  ", __func__, __LINE__, CmdBuffer);
        if (system(CmdBuffer) <  0)
        {
            GMI_DeBugPrint("[%s][%d]  %s  Error ", __func__, __LINE__, CmdBuffer);
        }

        GMI_DeBugPrint("[%s][%d]  %s Restart ", __func__, __LINE__, CmdBufferStart);
        if (system(CmdBufferStart) <  0)
        {
            GMI_DeBugPrint("[%s][%d] Restart  %s Error ", __func__, __LINE__, CmdBufferStart);
        }
        return GMI_SUCCESS;
    }

    TimeNumber--;
    *DaemonTime = TimeNumber;

    return GMI_SUCCESS;
}

static void* GMI_DaemonServer(void *param)
{
    pthread_detach(pthread_self());

    for (;;)
    {
        if ( g_ApplicationOpenFlags->s_LogServerOpenFlags  &&  g_ApplicationRegisterFlags->s_LogRegisterFlag)
        {
            SystemRestartServer(LOG_SERVER_ID, g_pSystemApplicationCfg->s_LogServerTimer, &(g_ApplicationDaemonTime->s_LogDaeminTime), &(g_ApplicationOnlineFlag->s_LogServerOnlineFlag));
        }

        if ( g_ApplicationOpenFlags->s_MediaServerOpenFlags  &&  g_ApplicationRegisterFlags->s_MediaRegisterFlag)
        {
            SystemRestartServer(MEDIA_SERVER_ID, g_pSystemApplicationCfg->s_MediaServerTimer, &(g_ApplicationDaemonTime->s_MediaDaeminTime), &(g_ApplicationOnlineFlag->s_MediaOnlineFlag));
        }

        if ( g_ApplicationOpenFlags->s_TransportServerOpenFlags  &&  g_ApplicationRegisterFlags->s_TransPortRegisterFlag)
        {
            SystemRestartServer(TRANSPORT_SERVER_ID, g_pSystemApplicationCfg->s_TransportServerTimer, &(g_ApplicationDaemonTime->s_TransPortDaeminTime), &(g_ApplicationOnlineFlag->s_TransPortOnlineFlag));
        }

        if ( g_ApplicationOpenFlags->s_ControlServerOpenFlags && g_ApplicationRegisterFlags->s_ControlRegisterFlag)
        {
            SystemRestartServer(CONTROL_SERVER_ID, g_pSystemApplicationCfg->s_ControlServerTimer, &(g_ApplicationDaemonTime->s_ControlDaeminTime), &(g_ApplicationOnlineFlag->s_ControlOnlineFlag));
        }

        if ( g_ApplicationOpenFlags->s_Gb28181ServerOpenFlags && g_ApplicationRegisterFlags->s_GbRegisterFlag)
        {
            SystemRestartServer(GB28181_SERVER_ID, g_pSystemApplicationCfg->s_GbServerTimer, &(g_ApplicationDaemonTime->s_GbDaeminTime), &(g_ApplicationOnlineFlag->s_GbOnlineFlag));
        }

        if ( g_ApplicationOpenFlags->s_OnVifServerOpenFlags && g_ApplicationRegisterFlags->s_OnvifRegisterFlag)
        {
            SystemRestartServer(ONVIF_SERVER_ID, g_pSystemApplicationCfg->s_OnvifServerTimer, &(g_ApplicationDaemonTime->s_OnvifDaeminTime), &(g_ApplicationOnlineFlag->s_OnvifOnlineFlag));
        }

        if ( g_ApplicationOpenFlags->s_SdkServerOpenFlags && g_ApplicationRegisterFlags->s_SdkRegisterFlag)
        {
            SystemRestartServer(SDK_SERVER_ID, g_pSystemApplicationCfg->s_SdkServerTimer, &(g_ApplicationDaemonTime->s_SdkDaeminTime), &(g_ApplicationOnlineFlag->s_SdkOnlineFlag));
        }

        if ( g_ApplicationOpenFlags->s_AuthServerOpenFlags && g_ApplicationRegisterFlags->s_AuthRegisterFlag)
        {
            SystemRestartServer(AUTH_SERVER_ID, g_pSystemApplicationCfg->s_AuthServerTimer, &(g_ApplicationDaemonTime->s_AuthDaeminTime), &(g_ApplicationOnlineFlag->s_AuthOnlineFlag));
        }

        if ( g_ApplicationOpenFlags->s_ConfigToolOpenFlags && g_ApplicationRegisterFlags->s_ConfigToolRegisterFlag)
        {
            SystemRestartServer(CONFIG_TOOL_SERVER_ID, g_pSystemApplicationCfg->s_ConfigToolServertimer, &(g_ApplicationDaemonTime->s_ConfigToolDaeminTime), &(g_ApplicationOnlineFlag->s_ConfigToolOnlineFlag));
        }

        GMI_TimeDelay(2, 0);
    }

    pthread_exit(NULL);
    return (void *)0;
}

/*=======================================================
name				:	GMI_DaemonInit
function			:  Init Daemon thread
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    FAIL:  -1
                                     SUCCESS : 0
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/29/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_DaemonInit(void)
{
    int32_t status = -1;

    l_sock = GMI_InitSystemHeard();

    DAEMON_PRINT_LOG(INFO,"GMI_DaemonMessageServer is Start ! ! ");

    status = GMI_CreateTask(190, 40960, (fTaskEntryPoint)GMI_DaemonMessageServer, 0, 0, 0, 0, 0, 0, 0);
    if (status < 0)
    {
        return GMI_FAIL;
    }
    DAEMON_PRINT_LOG(INFO,"GMI_DaemonServer is Start ! ! ");

    status = GMI_CreateTask(190, 8192, (fTaskEntryPoint)GMI_DaemonServer, 0, 0, 0, 0, 0, 0, 0);
    if (status < 0)
    {
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}

/*=======================================================
name				:	GMI_DaemonUnInit
function			:  UnInit Daemon thread,free resource
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    FAIL:  -1
                                     SUCCESS : 0
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	7/10/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
void GMI_DaemonUnInit(void)
{
    GMI_UnInitHeartbeat(l_sock);
}

