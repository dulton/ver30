/******************************************************************************
modules		:    Daemon
name		:    heardbeat.c
function	:    heardbeat modules
author		:    minchao.wang
version		:    1.0.0.0.0
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/31/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
#include "gmi_includes.h"
#include "gmi_common.h"
#include "gmi_heartbeat.h"
#include "gmi_debug.h"
#include "gmi_daemon_thread.h"
#include "gmi_rudp_api.h"
#include "gmi_system.h"
#include "gmi_struct.h"
#include "gmi_config.h"
#include "gmi_update.h"
#include "ipc_fw_v3.x_resource.h"
#include "gmi_system_headers.h"

//System Start flag
boolean_t l_SystemStartFlag = false;

//rudp port lock
pthread_mutex_t g_LockRudpAuthPort   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_LockRudpLogPort   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_LockRudpMediaPort   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_LockRudpSystemPort   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_LockRudpOnvifPort   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_LockRudpGBPort   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_LockRudpSdkPort   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_LockRudpWebPort   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_LockRudpTransportPort   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_LockRudpConfigToolPort   = PTHREAD_MUTEX_INITIALIZER;

void GMI_UnInitHeartbeat(FD_HANDLE SockFd)
{
    GMI_RudpSocketClose(SockFd);
}

GMI_RESULT GMI_ApplicationRegister(const SystemApplicationCfg_t *SystemApplicationCfg, ApplicationRegisterFlags *AppRegisterFlags, int32_t SequenceId)
{
    if (NULL == SystemApplicationCfg)
    {
        return GMI_INVALID_PARAMETER;
    }

    if (SequenceId == SystemApplicationCfg->s_LogServer)
    {
        if (!AppRegisterFlags->s_LogRegisterFlag)
        {
            AppRegisterFlags->s_LogRegisterFlag = true;
            return GMI_SUCCESS;
        }
        return GMI_FAIL;
    }

    if (SequenceId == SystemApplicationCfg->s_MediaServer)
    {
        if (!AppRegisterFlags->s_MediaRegisterFlag)
        {
            AppRegisterFlags->s_MediaRegisterFlag = true;
            return GMI_SUCCESS;
        }
        return GMI_FAIL;
    }

    if(SequenceId == SystemApplicationCfg->s_TransportServer)
    {
        if (!AppRegisterFlags->s_TransPortRegisterFlag)
        {
            AppRegisterFlags->s_TransPortRegisterFlag = true;
            return GMI_SUCCESS;
        }
        return GMI_FAIL;
    }

    if(SequenceId == SystemApplicationCfg->s_WebServer)
    {
        return GMI_FAIL;
    }

    if(SequenceId == SystemApplicationCfg->s_ControlServer)
    {
        if (!AppRegisterFlags->s_ControlRegisterFlag)
        {
            AppRegisterFlags->s_ControlRegisterFlag = true;
            return GMI_SUCCESS;
        }
        return GMI_FAIL;
    }


    if(SequenceId == SystemApplicationCfg->s_Gb28181Server)
    {
        if (!AppRegisterFlags->s_GbRegisterFlag)
        {
            AppRegisterFlags->s_GbRegisterFlag = true;
            return GMI_SUCCESS;
        }
        return GMI_FAIL;
    }

    if(SequenceId == SystemApplicationCfg->s_OnVifServer)
    {
        if (!AppRegisterFlags->s_OnvifRegisterFlag)
        {
            AppRegisterFlags->s_OnvifRegisterFlag = true;
            return GMI_SUCCESS;
        }
        return GMI_FAIL;
    }

    if(SequenceId == SystemApplicationCfg->s_SdkServer)
    {
        if (!AppRegisterFlags->s_SdkRegisterFlag)
        {
            AppRegisterFlags->s_SdkRegisterFlag = true;
            return GMI_SUCCESS;
        }
        return GMI_FAIL;
    }

    if(SequenceId == SystemApplicationCfg->s_AuthServer)
    {
        if (!AppRegisterFlags->s_AuthRegisterFlag)
        {
            AppRegisterFlags->s_AuthRegisterFlag = true;
            return GMI_SUCCESS;
        }
        return GMI_FAIL;
    }

    if(SequenceId == SystemApplicationCfg->s_ConfigToolServer)
    {
        if (!AppRegisterFlags->s_ConfigToolRegisterFlag)
        {
            AppRegisterFlags->s_ConfigToolRegisterFlag = true;
            return GMI_SUCCESS;
        }
        return GMI_FAIL;
    }

    return GMI_FAIL;
}

void GMI_ApplicationUnRegister(boolean_t *RegisterFlag, boolean_t *OnlineFlag)
{
    *RegisterFlag= true;
    *OnlineFlag = false;
}

GMI_RESULT GMI_SystemAppRunning(const SystemApplicationCfg_t *SystemApplicationCfg, ApplicationRegisterFlags *AppRegisterFlags, int32_t SequenceId)
{
    GMI_RESULT Result=GMI_FAIL;
    Result = GMI_ApplicationRegister(SystemApplicationCfg, AppRegisterFlags, SequenceId);
    if (FAILED(Result))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_ApplicationRegister   Error    ! ! ");
    }
    else if (SUCCEEDED(Result))
    {
        Result = GMI_StartUpSystemService(SystemApplicationCfg, SequenceId);
        if (FAILED(Result))
            DAEMON_PRINT_LOG(ERROR,"GMI_StartUpSystemService Running  log  Error	! ! ");
    }
    return Result;
}

GMI_RESULT GMI_RudpPackFill(PkgRudpSendInput *RudpSendInput, int32_t AppId,  uint32_t RemotePort)
{

    switch (AppId)
    {
    case LOG_SERVER_ID:
        strcpy((char *)(RudpSendInput->s_Buffer), LOG_SERVER_REGISTER_MESSAGE);
        break;
    case MEDIA_SERVER_ID:
        strcpy((char *)(RudpSendInput->s_Buffer), MEDIA_SERVER_REGISTER_MESSAGE);
        break;
    case TRANSPORT_SERVER_ID:
        strcpy((char *)(RudpSendInput->s_Buffer), TRANSPORT_SERVER_REGISTER_MESSAGE);
        break;
    case CONTROL_SERVER_ID:
        strcpy((char *)(RudpSendInput->s_Buffer), CONTROL_SERVER_REGISTER_MESSAGE);
        break;
    case WEB_SERVER_ID:
        strcpy((char *)(RudpSendInput->s_Buffer), WEB_SERVER_REGISTER_MESSAGE);
        break;
    case GB28181_SERVER_ID:
        strcpy((char *)(RudpSendInput->s_Buffer), GB28181_SERVER_REGISTER_MESSAGE);
        break;
    case ONVIF_SERVER_ID:
        strcpy((char *)(RudpSendInput->s_Buffer), ONVIF_SERVER_REGISTER_MESSAGE);
        break;
    case SDK_SERVER_ID:
        strcpy((char *)(RudpSendInput->s_Buffer), SDK_SERVER_REGISTER_MESSAGE);
        break;
    case AUTH_SERVER_ID:
        strcpy((char *)(RudpSendInput->s_Buffer), AUTH_SERVER_REGISTER_MESSAGE);
        break;
    case CONFIG_TOOL_SERVER_ID:
        strcpy((char *)(RudpSendInput->s_Buffer), CONFIG_TOOL_SERVER_REGISTER_MESSAGE);
        break;
    default:
        return GMI_NOT_SUPPORT;
    }

    RudpSendInput->s_RemotePort = RemotePort;
    RudpSendInput->s_SendLength = GMI_RUDP_BUFFER_LENGTH;
    RudpSendInput->s_TimeoutMS = GMI_RUDP_TIMEOUT;

    return GMI_SUCCESS;
}

void GMI_MessageLinkPackFill( uint32_t RemotePort, boolean_t *QuitFlag,  boolean_t *OpenFlags, boolean_t *IPChangeFlags, PkgRudpSendInput *RudpSendInput)
{
    if (*QuitFlag)
    {
        *QuitFlag = false;
        *OpenFlags = false;
        strcpy((char *)(RudpSendInput->s_Buffer), APPLICATION_QUIT_FLAGS);
        GMI_DeBugPrint("[%s][%d]  Server  will be quit",__func__,__LINE__);
    }
    else if (*IPChangeFlags)
    {
        *IPChangeFlags = false;
        strcpy((char *)(RudpSendInput->s_Buffer), APPLICATION_IPCHANGE_FLAGS);
    }
    else
    {
        strcpy((char *)(RudpSendInput->s_Buffer), APPLICATION_LINK_FLAGS);
    }

    RudpSendInput->s_RemotePort = RemotePort;
    RudpSendInput->s_SendLength = GMI_RUDP_BUFFER_LENGTH;
    RudpSendInput->s_TimeoutMS = GMI_RUDP_TIMEOUT;

}

void GMI_SystemRunningApp(void)
{
    char_t CmdBuffer[MAX_BUFFER_LENGTH];

    memset(CmdBuffer, 0, sizeof(CmdBuffer));
    snprintf(CmdBuffer, 255, SYSTEM_START_TIME_RECORD);
    if (system(CmdBuffer) < 0)
    {
        GMI_DeBugPrint("[%s][%d]	System Start time record fail",__func__,__LINE__);
    }

    memset(CmdBuffer, 0, sizeof(CmdBuffer));
    snprintf(CmdBuffer, 255, "/opt/bin/snmp &");
    if (system(CmdBuffer) < 0)
    {
        GMI_DeBugPrint("[%s][%d]	snmp running fail",__func__,__LINE__);
    }

    memset(CmdBuffer, 0, sizeof(CmdBuffer));
    snprintf(CmdBuffer, 255, "/opt/bin/system_resource_detect &");
    if (system(CmdBuffer) < 0)
    {
        GMI_DeBugPrint("[%s][%d]	system_resource_detect	running fail",__func__,__LINE__);
    }
}

GMI_RESULT GMI_QueryHeartbeatMassge(FD_HANDLE SockFd)
{
    PkgRudpSendInput RudpSendInput = {0};
    PkgRudpSendOutput RudpSendOutput = {0};
    PkgRudpRecvInput RudpRecvInput = {0};
    PkgRudpRecvOutput RudpRecvOutput = {0};

    char_t buffer[GMI_RUDP_BUFFER_LENGTH];
    memset(buffer, 0, sizeof(buffer));

    RudpRecvInput.s_TimeoutMS = GMI_RUDP_TIMEOUT;
    RudpRecvOutput.s_Buffer = (uint8_t*)buffer;
    RudpRecvOutput.s_BufferLength = sizeof(buffer);

    GMI_RESULT Ret = GMI_FAIL;
    int32_t TmpId=-1;

    Ret = GMI_RudpRecv(SockFd, &RudpRecvInput, &RudpRecvOutput);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_RudpRecv.buffer %ld\n",Ret);
    }
    else if (SUCCEEDED(Ret))
    {
        DAEMON_PRINT_LOG(INFO,"[%s] RudpRecvOutput.buffer %s\n",__func__, RudpRecvOutput.s_Buffer);
        if (strcmp(LOG_SERVER_REGISTER_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_RudpPackFill(&RudpSendInput, LOG_SERVER_ID,  GMI_DAEMON_HEARDBEAT_LOG);

            LOCK(&g_LockRudpLogPort);
            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            if (SUCCEEDED(Ret))
            {
                UNLOCK(&g_LockRudpLogPort);
                GMI_DeBugPrint("[%s][%d] LogServer  is Register OK  ",__func__,__LINE__);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                UNLOCK(&g_LockRudpLogPort);
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error	! ! ");
            }
        }
        else if (strcmp(AUTH_SERVER_REGISTER_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_RudpPackFill(&RudpSendInput, AUTH_SERVER_ID,  GMI_DAEMON_HEARTBEAT_AUTH);

            LOCK(&g_LockRudpAuthPort);
            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            if (SUCCEEDED(Ret))
            {
                UNLOCK(&g_LockRudpAuthPort);
                GMI_DeBugPrint("[%s][%d] AuthServer	is Register OK	",__func__,__LINE__);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                UNLOCK(&g_LockRudpAuthPort);
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error ! ! ");
            }
        }
        else if (strcmp(MEDIA_SERVER_REGISTER_MESSAGE,(char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            DAEMON_PRINT_LOG(INFO,"Receive MeidaServer Register message	! ! ");
            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_RudpPackFill(&RudpSendInput, MEDIA_SERVER_ID,  GMI_DAEMON_HEARDBEAT_MEDIA);

            LOCK(&g_LockRudpMediaPort);
            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            if (SUCCEEDED(Ret))
            {
                UNLOCK(&g_LockRudpMediaPort);
                GMI_DeBugPrint("[%s][%d] MeidaServer  is Register OK	",__func__,__LINE__);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                UNLOCK(&g_LockRudpMediaPort);
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error! ! ");
            }
        }
        else if (strcmp(TRANSPORT_SERVER_REGISTER_MESSAGE,(char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_RudpPackFill(&RudpSendInput, TRANSPORT_SERVER_ID,  GMI_DAEMON_HEARDBEAT_TRANSPORT);

            LOCK(&g_LockRudpTransportPort);
            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            if (SUCCEEDED(Ret))
            {
                UNLOCK(&g_LockRudpTransportPort);
                GMI_DeBugPrint("[%s][%d] TransportServer  is Register OK	",__func__,__LINE__);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                UNLOCK(&g_LockRudpTransportPort);
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error	! ! ");
            }
        }
        else if (strcmp(CONTROL_SERVER_REGISTER_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_RudpPackFill(&RudpSendInput, CONTROL_SERVER_ID,  GMI_DAEMON_HEARDBEAT_CONTROL);

            LOCK(&g_LockRudpSystemPort);
            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            if (SUCCEEDED(Ret))
            {
                UNLOCK(&g_LockRudpSystemPort);
                GMI_DeBugPrint("[%s][%d] ControlServer  is Register OK	",__func__,__LINE__);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                UNLOCK(&g_LockRudpSystemPort);
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error	! ! ");
            }
        }
        else if (strcmp(WEB_SERVER_REGISTER_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_RudpPackFill(&RudpSendInput, WEB_SERVER_ID,  GMI_DAEMON_HEARDBEAT_WEB);

            LOCK(&g_LockRudpWebPort);
            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            if (SUCCEEDED(Ret))
            {
                UNLOCK(&g_LockRudpWebPort);
                GMI_DeBugPrint("[%s][%d] Web  is Register OK	",__func__,__LINE__);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                UNLOCK(&g_LockRudpWebPort);
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error	! ! ");
            }
        }
        else if (strcmp(GB28181_SERVER_REGISTER_MESSAGE,(char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_RudpPackFill(&RudpSendInput, GB28181_SERVER_ID,  GMI_DAEMON_HEARDBEAT_GB28181);

            LOCK(&g_LockRudpGBPort);
            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            if (SUCCEEDED(Ret))
            {
                UNLOCK(&g_LockRudpGBPort);
                GMI_DeBugPrint("[%s][%d] Gb28181Server	is Register OK	",__func__,__LINE__);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                UNLOCK(&g_LockRudpGBPort);
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error	! ! ");
            }
        }
        else if (strcmp(ONVIF_SERVER_REGISTER_MESSAGE,(char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_RudpPackFill(&RudpSendInput, ONVIF_SERVER_ID,  GMI_DAEMON_HEARDBEAT_ONVIF);

            LOCK(&g_LockRudpOnvifPort);
            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            if (SUCCEEDED(Ret))
            {
                UNLOCK(&g_LockRudpOnvifPort);
                GMI_DeBugPrint("[%s][%d] Onvif Server is Register OK	",__func__,__LINE__);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                UNLOCK(&g_LockRudpOnvifPort);
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error	! ! ");
            }
        }
        else if (strcmp(SDK_SERVER_REGISTER_MESSAGE,(char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            strcpy(buffer, SDK_SERVER_REGISTER_MESSAGE);
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_RemotePort = GMI_DAEMON_HEARTBEAT_SDK;
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            RudpSendInput.s_SendLength = sizeof(buffer);
            RudpSendInput.s_TimeoutMS = GMI_RUDP_TIMEOUT;
            RudpSendInput.s_SequenceNum = RudpRecvOutput.s_SequenceNum;

            LOCK(&g_LockRudpSdkPort);
            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            if (SUCCEEDED(Ret))
            {
                UNLOCK(&g_LockRudpSdkPort);
                GMI_DeBugPrint("[%s][%d] SDK Server is Register OK    ",__func__,__LINE__);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                UNLOCK(&g_LockRudpSdkPort);
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error ! ! ");
            }
        }
        else if (strcmp(CONFIG_TOOL_SERVER_REGISTER_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_RudpPackFill(&RudpSendInput, CONFIG_TOOL_SERVER_ID,  GMI_DAEMON_HEARTBEAT_CONFIG_TOOL);

            LOCK(&g_LockRudpConfigToolPort);
            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            if (SUCCEEDED(Ret))
            {
                UNLOCK(&g_LockRudpConfigToolPort);
                GMI_DeBugPrint("[%s][%d] Config  is Register OK	",__func__,__LINE__);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                UNLOCK(&g_LockRudpConfigToolPort);
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error	! ! ");
            }
        }
        else if (strcmp(LOG_SERVER_HEARTBEAT_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            g_ApplicationOnlineFlag->s_LogServerOnlineFlag = true;

            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_MessageLinkPackFill( GMI_DAEMON_HEARDBEAT_LOG, &(g_ApplicationQuitFlag->s_LogServerQuitFlag), &(g_ApplicationOpenFlags->s_LogServerOpenFlags), &(g_ApplicationIPChangeFlags->s_LogServerIPChangeFlags), &RudpSendInput);
            LOCK(&g_LockRudpLogPort);

            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            UNLOCK(&g_LockRudpLogPort);
            if (SUCCEEDED(Ret))
            {
                g_ApplicationDaemonTime->s_LogDaeminTime = g_pSystemApplicationCfg->s_LogServerTimer;
                TmpId = g_pSystemApplicationCfg->s_LogServer + 1;
                Ret = GMI_SystemAppRunning(g_pSystemApplicationCfg, g_ApplicationRegisterFlags, TmpId);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error	! ! ");
                return Ret;
            }
        }
        else if (strcmp(AUTH_SERVER_HEARTBEAT_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            g_ApplicationOnlineFlag->s_AuthOnlineFlag =  true;

            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_MessageLinkPackFill( GMI_DAEMON_HEARTBEAT_AUTH, &(g_ApplicationQuitFlag->s_AuthQuitFlag), &(g_ApplicationOpenFlags->s_AuthServerOpenFlags), &(g_ApplicationIPChangeFlags->s_AuthServerIPChangeFlags), &RudpSendInput);
            LOCK(&g_LockRudpAuthPort);

            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            UNLOCK(&g_LockRudpAuthPort);

            if (SUCCEEDED(Ret))
            {
                g_ApplicationDaemonTime->s_AuthDaeminTime = g_pSystemApplicationCfg->s_AuthServerTimer;
                TmpId = g_pSystemApplicationCfg->s_AuthServer + 1;
                Ret = GMI_SystemAppRunning(g_pSystemApplicationCfg, g_ApplicationRegisterFlags, TmpId);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error ! ! ");
                return Ret;
            }
        }
        else if (strcmp(MEDIA_SERVER_HEARTBEAT_MESSAGE,(char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            g_ApplicationOnlineFlag->s_MediaOnlineFlag = true;

            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_MessageLinkPackFill( GMI_DAEMON_HEARDBEAT_MEDIA, &(g_ApplicationQuitFlag->s_MediaQuitFlag), &(g_ApplicationOpenFlags->s_MediaServerOpenFlags), &(g_ApplicationIPChangeFlags->s_MediaServerIPChangeFlags), &RudpSendInput);
            LOCK(&g_LockRudpMediaPort);
            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            UNLOCK(&g_LockRudpMediaPort);
            if (SUCCEEDED(Ret))
            {
                g_ApplicationDaemonTime->s_MediaDaeminTime = g_pSystemApplicationCfg->s_MediaServerTimer;
                TmpId = g_pSystemApplicationCfg->s_MediaServer + 1;
                Ret = GMI_SystemAppRunning(g_pSystemApplicationCfg, g_ApplicationRegisterFlags, TmpId);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error	! ! ");
                return Ret;
            }
        }
        else if (strcmp(TRANSPORT_SERVER_HEARTBEAT_MESSAGE,(char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            g_ApplicationOnlineFlag->s_TransPortOnlineFlag = true;
            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_MessageLinkPackFill( GMI_DAEMON_HEARDBEAT_TRANSPORT, &(g_ApplicationQuitFlag->s_TransPortQuitFlag), &(g_ApplicationOpenFlags->s_TransportServerOpenFlags), &(g_ApplicationIPChangeFlags->s_TransportServerIPChangeFlags), &RudpSendInput);

            LOCK(&g_LockRudpTransportPort);
            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            UNLOCK(&g_LockRudpTransportPort);
            if (SUCCEEDED(Ret))
            {
                g_ApplicationDaemonTime->s_TransPortDaeminTime = g_pSystemApplicationCfg->s_TransportServerTimer;
                TmpId = g_pSystemApplicationCfg->s_TransportServer + 1;
                Ret = GMI_SystemAppRunning(g_pSystemApplicationCfg, g_ApplicationRegisterFlags, TmpId);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error	! ! ");
                return Ret;
            }
        }
        else if (strcmp(CONTROL_SERVER_HEARTBEAT_MESSAGE,(char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            g_ApplicationOnlineFlag->s_ControlOnlineFlag = true;

            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_MessageLinkPackFill( GMI_DAEMON_HEARDBEAT_CONTROL, &(g_ApplicationQuitFlag->s_ControlQuitFlag), &(g_ApplicationOpenFlags->s_ControlServerOpenFlags), &(g_ApplicationIPChangeFlags->s_ControlServerIPChangeFlags), &RudpSendInput);

            LOCK(&g_LockRudpSystemPort);
            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            UNLOCK(&g_LockRudpSystemPort);
            if (SUCCEEDED(Ret))
            {
                g_ApplicationDaemonTime->s_ControlDaeminTime = g_pSystemApplicationCfg->s_ControlServerTimer;
                TmpId = g_pSystemApplicationCfg->s_ControlServer + 1;
                Ret = GMI_SystemAppRunning(g_pSystemApplicationCfg, g_ApplicationRegisterFlags, TmpId);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                return Ret;
            }
        }
        else if (strcmp(GB28181_SERVER_HEARTBEAT_MESSAGE,(char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            g_ApplicationOnlineFlag->s_GbOnlineFlag = true;

            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_MessageLinkPackFill( GMI_DAEMON_HEARDBEAT_GB28181, &(g_ApplicationQuitFlag->s_GbQuitFlag), &(g_ApplicationOpenFlags->s_Gb28181ServerOpenFlags), &(g_ApplicationIPChangeFlags->s_Gb28181ServerIPChangeFlags), &RudpSendInput);
            LOCK(&g_LockRudpGBPort);

            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            UNLOCK(&g_LockRudpGBPort);
            if (SUCCEEDED(Ret))
            {
                g_ApplicationDaemonTime->s_GbDaeminTime = g_pSystemApplicationCfg->s_GbServerTimer;
                TmpId = g_pSystemApplicationCfg->s_Gb28181Server + 1;
                Ret = GMI_SystemAppRunning(g_pSystemApplicationCfg, g_ApplicationRegisterFlags, TmpId);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error	! ! ");
                return Ret;
            }
        }
        else if (strcmp(ONVIF_SERVER_HEARTBEAT_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            g_ApplicationOnlineFlag->s_OnvifOnlineFlag = true;

            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_MessageLinkPackFill( GMI_DAEMON_HEARDBEAT_ONVIF, &(g_ApplicationQuitFlag->s_OnvifQuitFlag), &(g_ApplicationOpenFlags->s_OnVifServerOpenFlags), &(g_ApplicationIPChangeFlags->s_OnVifServerIPChangeFlags), &RudpSendInput);
            LOCK(&g_LockRudpOnvifPort);

            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            UNLOCK(&g_LockRudpOnvifPort);
            if (SUCCEEDED(Ret))
            {
                g_ApplicationDaemonTime->s_OnvifDaeminTime = g_pSystemApplicationCfg->s_OnvifServerTimer;
                TmpId = g_pSystemApplicationCfg->s_OnVifServer + 1;
                Ret = GMI_SystemAppRunning(g_pSystemApplicationCfg, g_ApplicationRegisterFlags, TmpId);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error	! ! ");
                return Ret;
            }
        }
        else if (strcmp(SDK_SERVER_HEARTBEAT_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            g_ApplicationOnlineFlag->s_SdkOnlineFlag = true;

            if(!l_SystemStartFlag)
            {
                l_SystemStartFlag = true;
                GMI_SystemRunningApp();
            }

            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            RudpSendInput.s_SequenceNum = RudpRecvOutput.s_SequenceNum;
            GMI_MessageLinkPackFill( GMI_DAEMON_HEARTBEAT_SDK, &(g_ApplicationQuitFlag->s_SdkQuitFlag), &(g_ApplicationOpenFlags->s_SdkServerOpenFlags), &(g_ApplicationIPChangeFlags->s_SdkServerIPChangeFlags), &RudpSendInput);

            LOCK(&g_LockRudpSdkPort);
            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            UNLOCK(&g_LockRudpSdkPort);
            if (SUCCEEDED(Ret))
            {
                g_ApplicationDaemonTime->s_SdkDaeminTime = g_pSystemApplicationCfg->s_SdkServerTimer;
                TmpId = g_pSystemApplicationCfg->s_SdkServer+ 1;
                Ret = GMI_SystemAppRunning(g_pSystemApplicationCfg, g_ApplicationRegisterFlags, TmpId);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                DAEMON_PRINT_LOG(ERROR,"GMI_QueryHeardbeatRegister is Error ! ! ");
                return Ret;
            }
        }

        else if (strcmp(CONFIG_TOOL_SERVER_HEARTBEAT_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            g_ApplicationOnlineFlag->s_ConfigToolOnlineFlag = true;

            memset(buffer,0,sizeof(buffer));
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            GMI_MessageLinkPackFill( GMI_DAEMON_HEARTBEAT_CONFIG_TOOL, &(g_ApplicationQuitFlag->s_ConfigToolQuitFlag), &(g_ApplicationOpenFlags->s_ConfigToolOpenFlags), &(g_ApplicationIPChangeFlags->s_ConfigToolIPChangeFlags), &RudpSendInput);

            LOCK(&g_LockRudpConfigToolPort);
            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            UNLOCK(&g_LockRudpConfigToolPort);
            if (SUCCEEDED(Ret))
            {
                g_ApplicationDaemonTime->s_ConfigToolDaeminTime = g_pSystemApplicationCfg->s_ConfigToolServertimer;
                TmpId = g_pSystemApplicationCfg->s_ConfigToolServer + 1;
                Ret = GMI_SystemAppRunning(g_pSystemApplicationCfg, g_ApplicationRegisterFlags, TmpId);
                return Ret;
            }
            else if (FAILED(Ret))
            {
                GMI_DeBugPrint("[%s][%d] GMI_RudpSend return Fail is %d",__func__,__LINE__,Ret);
                return Ret;
            }
        }

        else if (strcmp(WEB_SERVER_HEARTBEAT_MESSAGE,(char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            memset(buffer,0,sizeof(buffer));
            strcpy(buffer, APPLICATION_LINK_FLAGS);
            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            RudpSendInput.s_RemotePort = GMI_DAEMON_HEARDBEAT_WEB;
            RudpSendInput.s_Buffer = (uint8_t*)buffer;
            RudpSendInput.s_SendLength = sizeof(buffer);
            RudpSendInput.s_TimeoutMS = GMI_RUDP_TIMEOUT;

            LOCK(&g_LockRudpWebPort);
            Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
            UNLOCK(&g_LockRudpWebPort);
            if (SUCCEEDED(Ret))
            {
                return Ret;
            }
            else if (FAILED(Ret))
            {
                return Ret;
            }
        }
        else if (strcmp(LOG_SERVER_UNREGISTER_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            GMI_ApplicationUnRegister(&(g_ApplicationRegisterFlags->s_LogRegisterFlag), &(g_ApplicationOnlineFlag->s_LogServerOnlineFlag));
            GMI_DeBugPrint("[%s][%d] log	Server Will be unregister ",__func__,__LINE__);
        }
        else if (strcmp(MEDIA_SERVER_UNREGISTER_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            GMI_ApplicationUnRegister(&(g_ApplicationRegisterFlags->s_MediaRegisterFlag), &(g_ApplicationOnlineFlag->s_MediaOnlineFlag));
            GMI_DeBugPrint("[%s][%d] Media	Server Will be unregister ",__func__,__LINE__);
        }
        else if (strcmp(TRANSPORT_SERVER_UNREGISTER_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            GMI_ApplicationUnRegister(&(g_ApplicationRegisterFlags->s_TransPortRegisterFlag), &(g_ApplicationOnlineFlag->s_TransPortOnlineFlag));
            GMI_DeBugPrint("[%s][%d] Transport	Server Will be unregister ",__func__,__LINE__);
        }
        else if (strcmp(CONTROL_SERVER_UNREGISTER_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            GMI_ApplicationUnRegister(&(g_ApplicationRegisterFlags->s_ControlRegisterFlag), &(g_ApplicationOnlineFlag->s_ControlOnlineFlag));
            GMI_DeBugPrint("[%s][%d] Control Server Will be unregister ",__func__,__LINE__);
        }
        else if (strcmp(GB28181_SERVER_UNREGISTER_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            GMI_ApplicationUnRegister(&(g_ApplicationRegisterFlags->s_GbRegisterFlag), &(g_ApplicationOnlineFlag->s_GbOnlineFlag));
            GMI_DeBugPrint("[%s][%d] GB	Server Will be unregister ",__func__,__LINE__);
        }
        else if (strcmp(ONVIF_SERVER_UNREGISTER_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            GMI_ApplicationUnRegister(&(g_ApplicationRegisterFlags->s_OnvifRegisterFlag), &(g_ApplicationOnlineFlag->s_OnvifOnlineFlag));
            GMI_DeBugPrint("[%s][%d] Onvif	Server Will be unregister ",__func__,__LINE__);
        }
        else if (strcmp(SDK_SERVER_UNREGISTER_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            GMI_ApplicationUnRegister(&(g_ApplicationRegisterFlags->s_SdkRegisterFlag), &(g_ApplicationOnlineFlag->s_SdkOnlineFlag));
            GMI_DeBugPrint("[%s][%d] Sdk Server Will be unregister ",__func__,__LINE__);
        }
        else if (strcmp(AUTH_SERVER_UNREGISTER_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            GMI_ApplicationUnRegister(&(g_ApplicationRegisterFlags->s_AuthRegisterFlag), &(g_ApplicationOnlineFlag->s_AuthOnlineFlag));
            GMI_DeBugPrint("[%s][%d] Auth Server Will be unregister ",__func__,__LINE__);
        }
        else if (strcmp(CONFIG_TOOL_SERVER_UNREGISTER_MESSAGE, (char_t*)RudpRecvOutput.s_Buffer) == 0)
        {
            GMI_ApplicationUnRegister(&(g_ApplicationRegisterFlags->s_ConfigToolRegisterFlag), &(g_ApplicationOnlineFlag->s_ConfigToolOnlineFlag));
            GMI_DeBugPrint("[%s][%d] Config Tool Will be unregister ",__func__,__LINE__);
        }
        else
        {

        }
    }

    return Ret;
}

/*=======================================================
name				:	GMI_InitHeartbeat
function			:  Get Rudp Socket value
algorithm implementation	:	no
global variable			:	no
parameter declaration		:
return				:   no
---------------------------------------------------------------
modification	:
   date		version		 author 	modification
   1/31/2013	1.0.0.0.0     minchao.wang         establish

******************************************************************************/
FD_HANDLE GMI_InitHeartbeat()
{
    DAEMON_PRINT_LOG(INFO,"GMI_InitHeartbeat is Error ! ! ");
    long_t LocalPort = GMI_DAEMON_HEARDBEAT_SERVER;
    FD_HANDLE sock;

    sock = GMI_RudpSocket(LocalPort);
    if (!sock)
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_InitHeartbeat is Error ! ! ");
    }

    return sock;
}

/*=======================================================
name				:	GMI_InitSystemHeard
function			:  Get Message key Id
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     tSystemApplicationCfg : System Application server
                                             tSystemHeardbeatMessage: get Message send and read id
return				:   no
---------------------------------------------------------------
modification	:
   date		version		 author 	modification
   1/31/2013	1.0.0.0.0     minchao.wang         establish

******************************************************************************/
FD_HANDLE GMI_InitSystemHeard()
{
    return GMI_InitHeartbeat();
}

