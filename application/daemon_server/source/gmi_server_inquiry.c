/******************************************************************************
modules 	:	 network manage
name		:	 gmi_server_inquiry.c
function	:	 Deal with server inquiry function.return message to other server.
author	:	 minchao.wang
version 	:	 1.0.0.0.0
---------------------------------------------------------------
modification	:
	date		version 	 author 	modification
	7/10/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
#include "gmi_struct.h"
#include "gmi_config.h"
#include "gmi_daemon_thread.h"
#include "gmi_system_headers.h"
#include "gmi_common.h"
#include "gmi_debug.h"
#include "gmi_rudp_api.h"
#include "gmi_config_api.h"
#include "gmi_server_inquiry.h"
#include "ipc_fw_v3.x_setting.h"
#include "gmi_brdwrapper.h"
#include "ipc_fw_v3.x_resource.h"
#include "gmi_heartbeat.h"

static FD_HANDLE l_InquirySock;

/*=======================================================
name				:	GMI_ServerStatusInquiry
function			:  Inquiry System Server status
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	7/10/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_InquiryMassge(FD_HANDLE SockFd)
{
    int32_t Ret = GMI_FAIL;
    uint8_t *Buffer = NULL;
    int32_t Tmp = 5;

    DaemonMessage  *l_DaemonMessage;

    PkgRudpSendInput RudpSendInput;
    PkgRudpSendOutput RudpSendOutput;
    PkgRudpRecvInput RudpRecvInput;
    PkgRudpRecvOutput RudpRecvOutput;

    Buffer = (uint8_t*)malloc(sizeof(DaemonMessage));
    memset(Buffer, 0, sizeof(DaemonMessage));

    RudpRecvInput.s_TimeoutMS = GMI_RUDP_TIMEOUT;
    RudpRecvOutput.s_Buffer = Buffer;
    RudpRecvOutput.s_BufferLength = sizeof(DaemonMessage);

    Ret = GMI_RudpRecv(SockFd, &RudpRecvInput, &RudpRecvOutput);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_InquiryMassge %d\n",Ret);
        if (NULL != Buffer)
        {
            free(Buffer);
        }
    }
    else if (SUCCEEDED(Ret))
    {
        l_DaemonMessage = (DaemonMessage  *)RudpRecvOutput.s_Buffer;

        DAEMON_PRINT_LOG(INFO,"l_DaemonMessage	 -> s_AppId  = %d !!\n",ntohs(l_DaemonMessage->s_AppId));
        DAEMON_PRINT_LOG(INFO,"l_DaemonMessage	 -> s_LocalServerId  = %d !!\n",ntohs(l_DaemonMessage->s_LocalServerId));

        if (GMI_DAEMON_APPLICATION_STATUS_QUIRY == ntohs(l_DaemonMessage->s_DaemonHearder.s_MessageId))
        {
            switch (ntohs(l_DaemonMessage->s_AppId))
            {
            case LOG_SERVER_ID:
                l_DaemonMessage->s_AppId = htons(LOG_SERVER_ID);
                if (g_ApplicationOpenFlags->s_LogServerOpenFlags && g_ApplicationOnlineFlag->s_LogServerOnlineFlag)
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_ONLINE);
                else
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_OFFLINE);
                break;
            case MEDIA_SERVER_ID:
                l_DaemonMessage->s_AppId = htons(MEDIA_SERVER_ID);
                if (g_ApplicationOpenFlags->s_MediaServerOpenFlags && g_ApplicationOnlineFlag->s_MediaOnlineFlag)
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_ONLINE);
                else
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_OFFLINE);
                break;
            case TRANSPORT_SERVER_ID:
                l_DaemonMessage->s_AppId = htons(TRANSPORT_SERVER_ID);
                if (g_ApplicationOpenFlags->s_TransportServerOpenFlags && g_ApplicationOnlineFlag->s_TransPortOnlineFlag)
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_ONLINE);
                else
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_OFFLINE);
                break;
            case CONTROL_SERVER_ID:
                l_DaemonMessage->s_AppId = htons(CONTROL_SERVER_ID);
                if (g_ApplicationOpenFlags->s_ControlServerOpenFlags && g_ApplicationOnlineFlag->s_ControlOnlineFlag)
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_ONLINE);
                else
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_OFFLINE);
                break;
            case GB28181_SERVER_ID:
                l_DaemonMessage->s_AppId = htons(GB28181_SERVER_ID);
                if (g_ApplicationOpenFlags->s_Gb28181ServerOpenFlags && g_ApplicationOnlineFlag->s_GbOnlineFlag)
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_ONLINE);
                else
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_OFFLINE);
                break;
            case ONVIF_SERVER_ID:
                l_DaemonMessage->s_AppId = htons(ONVIF_SERVER_ID);
                if (g_ApplicationOpenFlags->s_OnVifServerOpenFlags && g_ApplicationOnlineFlag->s_OnvifOnlineFlag)
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_ONLINE);
                else
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_OFFLINE);
                break;
            case SDK_SERVER_ID:
                l_DaemonMessage->s_AppId = htons(SDK_SERVER_ID);
                if (g_ApplicationOpenFlags->s_SdkServerOpenFlags && g_ApplicationOnlineFlag->s_SdkOnlineFlag)
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_ONLINE);
                else
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_OFFLINE);
                break;
            case AUTH_SERVER_ID:
                l_DaemonMessage->s_AppId = htons(AUTH_SERVER_ID);
                if (g_ApplicationOpenFlags->s_AuthServerOpenFlags && g_ApplicationOnlineFlag->s_AuthOnlineFlag)
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_ONLINE);
                else
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_OFFLINE);
                break;
            case CONFIG_TOOL_SERVER_ID:
                l_DaemonMessage->s_AppId = htons(CONFIG_TOOL_SERVER_ID);
                if (g_ApplicationOpenFlags->s_ConfigToolOpenFlags && g_ApplicationOnlineFlag->s_ConfigToolOnlineFlag)
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_ONLINE);
                else
                    l_DaemonMessage->s_AppStatus = htons(APPLICATION_STATUS_OFFLINE);
                break;

            default:
                break;
            }

            DAEMON_PRINT_LOG(INFO,"[%s][%d] s_LocalServerId = [%d]   s_AppId = [%d]  s_AppStatus = [%d]  ! ",__func__,__LINE__,  \
                             ntohs(l_DaemonMessage->s_LocalServerId),ntohs(l_DaemonMessage->s_AppId),ntohs(l_DaemonMessage->s_AppStatus));

            memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
            switch (ntohs(l_DaemonMessage->s_LocalServerId))
            {
            case LOG_SERVER_ID:
                RudpSendInput.s_RemotePort = GMI_DAEMON_HEARDBEAT_LOG;
                RudpSendInput.s_Buffer = (uint8_t*) l_DaemonMessage;
                RudpSendInput.s_SendLength = sizeof(DaemonMessage);
                RudpSendInput.s_TimeoutMS = GMI_RUDP_TIMEOUT;
                LOCK(&g_LockRudpLogPort);
                Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
                if (FAILED(Ret))
                {
                    UNLOCK(&g_LockRudpLogPort);
                    DAEMON_PRINT_LOG(ERROR,"GMI_RudpSend %d\n",Ret);
                    if (NULL != Buffer)
                        free(Buffer);
                    return Ret;
                }
                UNLOCK(&g_LockRudpLogPort);

                break;
            case MEDIA_SERVER_ID:
                RudpSendInput.s_RemotePort = GMI_DAEMON_HEARDBEAT_MEDIA;
                RudpSendInput.s_Buffer = (uint8_t*) l_DaemonMessage;
                RudpSendInput.s_SendLength = sizeof(DaemonMessage);
                RudpSendInput.s_TimeoutMS = GMI_RUDP_TIMEOUT;
                LOCK(&g_LockRudpMediaPort);
                Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
                if (FAILED(Ret))
                {
                    UNLOCK(&g_LockRudpMediaPort);
                    DAEMON_PRINT_LOG(ERROR,"GMI_RudpSend %d\n",Ret);
                    if (NULL != Buffer)
                        free(Buffer);
                    return Ret;
                }
                UNLOCK(&g_LockRudpMediaPort);

                break;
            case TRANSPORT_SERVER_ID:
                RudpSendInput.s_RemotePort = GMI_DAEMON_HEARDBEAT_TRANSPORT;
                RudpSendInput.s_Buffer = (uint8_t*) l_DaemonMessage;
                RudpSendInput.s_SendLength = sizeof(DaemonMessage);
                RudpSendInput.s_TimeoutMS = GMI_RUDP_TIMEOUT;
                LOCK(&g_LockRudpTransportPort);
                Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
                if (FAILED(Ret))
                {
                    UNLOCK(&g_LockRudpTransportPort);
                    DAEMON_PRINT_LOG(ERROR,"GMI_RudpSend %d\n",Ret);
                    if (NULL != Buffer)
                        free(Buffer);
                    return Ret;
                }
                UNLOCK(&g_LockRudpTransportPort);

                break;
            case CONTROL_SERVER_ID:
                RudpSendInput.s_RemotePort = GMI_DAEMON_HEARDBEAT_CONTROL;
                RudpSendInput.s_Buffer = (uint8_t*) l_DaemonMessage;
                RudpSendInput.s_SendLength = sizeof(DaemonMessage);
                RudpSendInput.s_TimeoutMS = GMI_RUDP_TIMEOUT;

                LOCK(&g_LockRudpSystemPort);
                Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
                if (FAILED(Ret))
                {
                    UNLOCK(&g_LockRudpSystemPort);
                    DAEMON_PRINT_LOG(ERROR,"GMI_RudpSend %d\n",Ret);
                    if (NULL != Buffer)
                        free(Buffer);
                    return Ret;
                }
                UNLOCK(&g_LockRudpSystemPort);

                break;
            case WEB_SERVER_ID:
                RudpSendInput.s_RemotePort = GMI_DAEMON_HEARDBEAT_WEB;
                RudpSendInput.s_Buffer = (uint8_t*) l_DaemonMessage;
                RudpSendInput.s_SendLength = sizeof(DaemonMessage);
                RudpSendInput.s_TimeoutMS = GMI_RUDP_TIMEOUT;

                LOCK(&g_LockRudpWebPort);
                Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
                if (FAILED(Ret))
                {
                    UNLOCK(&g_LockRudpWebPort);
                    DAEMON_PRINT_LOG(ERROR,"GMI_RudpSend %d\n",Ret);
                    if (NULL != Buffer)
                        free(Buffer);
                    return Ret;
                }
                UNLOCK(&g_LockRudpWebPort);

                break;
            case GB28181_SERVER_ID:
                RudpSendInput.s_RemotePort = GMI_DAEMON_HEARDBEAT_GB28181;
                RudpSendInput.s_Buffer = (uint8_t*) l_DaemonMessage;
                RudpSendInput.s_SendLength = sizeof(DaemonMessage);
                RudpSendInput.s_TimeoutMS = GMI_RUDP_TIMEOUT;

                LOCK(&g_LockRudpGBPort);
                Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
                if (FAILED(Ret))
                {
                    UNLOCK(&g_LockRudpGBPort);
                    DAEMON_PRINT_LOG(ERROR,"GMI_RudpSend %d\n",Ret);
                    if (NULL != Buffer)
                        free(Buffer);
                    return Ret;
                }
                UNLOCK(&g_LockRudpGBPort);

                break;
            case ONVIF_SERVER_ID:
                RudpSendInput.s_RemotePort = GMI_DAEMON_HEARDBEAT_ONVIF;
                RudpSendInput.s_Buffer = (uint8_t*) l_DaemonMessage;
                RudpSendInput.s_SendLength = sizeof(DaemonMessage);
                RudpSendInput.s_TimeoutMS = GMI_RUDP_TIMEOUT;

                LOCK(&g_LockRudpOnvifPort);
                Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
                if (FAILED(Ret))
                {
                    UNLOCK(&g_LockRudpOnvifPort);
                    DAEMON_PRINT_LOG(ERROR,"GMI_RudpSend %d\n",Ret);
                    if (NULL != Buffer)
                        free(Buffer);
                    return Ret;
                }
                UNLOCK(&g_LockRudpOnvifPort);

                break;
            case SDK_SERVER_ID:
                RudpSendInput.s_RemotePort = GMI_DAEMON_HEARTBEAT_SDK;
                RudpSendInput.s_Buffer = (uint8_t*) l_DaemonMessage;
                RudpSendInput.s_SendLength = sizeof(DaemonMessage);
                RudpSendInput.s_TimeoutMS = GMI_RUDP_TIMEOUT;

                LOCK(&g_LockRudpSdkPort);
                Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
                if (FAILED(Ret))
                {
                    UNLOCK(&g_LockRudpSdkPort);
                    DAEMON_PRINT_LOG(ERROR,"GMI_RudpSend %d\n",Ret);
                    if (NULL != Buffer)
                        free(Buffer);
                    return Ret;
                }
                UNLOCK(&g_LockRudpSdkPort);

                break;
            case AUTH_SERVER_ID:
                RudpSendInput.s_RemotePort = GMI_DAEMON_HEARTBEAT_AUTH;
                RudpSendInput.s_Buffer = (uint8_t*) l_DaemonMessage;
                RudpSendInput.s_SendLength = sizeof(DaemonMessage);
                RudpSendInput.s_TimeoutMS = GMI_RUDP_TIMEOUT;

                LOCK(&g_LockRudpAuthPort);
                Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
                if (FAILED(Ret))
                {
                    UNLOCK(&g_LockRudpAuthPort);
                    DAEMON_PRINT_LOG(ERROR,"GMI_RudpSend %d\n",Ret);
                    if (NULL != Buffer)
                        free(Buffer);
                    return Ret;
                }
                UNLOCK(&g_LockRudpAuthPort);

                break;
            case CONFIG_TOOL_SERVER_ID:
                RudpSendInput.s_RemotePort = GMI_DAEMON_HEARTBEAT_CONFIG_TOOL;
                RudpSendInput.s_Buffer = (uint8_t*) l_DaemonMessage;
                RudpSendInput.s_SendLength = sizeof(DaemonMessage);
                RudpSendInput.s_TimeoutMS = GMI_RUDP_TIMEOUT;

                LOCK(&g_LockRudpConfigToolPort);
                Ret = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
                UNLOCK(&g_LockRudpConfigToolPort);

                if (FAILED(Ret))
                {
                    DAEMON_PRINT_LOG(ERROR,"GMI_RudpSend %d\n",Ret);
                    if (NULL != Buffer)
                        free(Buffer);
                    return Ret;
                }

                break;
            default:
                break;
            }

            if (NULL != Buffer)
                free(Buffer);
            return Ret;

        }
        else if (GMI_DAEMON_APPLICATION_IP_CHANGE == ntohs(l_DaemonMessage->s_DaemonHearder.s_MessageId))
        {
            GMI_DeBugPrint("[%s][%d] AppId = %d Serer Send IP Change Cmd ! ",__func__,__LINE__,ntohs(l_DaemonMessage->s_LocalServerId));

            g_ApplicationIPChangeFlags->s_LogServerIPChangeFlags = true;
            g_ApplicationIPChangeFlags->s_AuthServerIPChangeFlags = true;
            g_ApplicationIPChangeFlags->s_MediaServerIPChangeFlags = true;
            g_ApplicationIPChangeFlags->s_ControlServerIPChangeFlags = true;
            g_ApplicationIPChangeFlags->s_TransportServerIPChangeFlags = true;
            g_ApplicationIPChangeFlags->s_OnVifServerIPChangeFlags = true;
            g_ApplicationIPChangeFlags->s_SdkServerIPChangeFlags = true;
            //  g_ApplicationIPChangeFlags->s_ConfigToolIPChangeFlags = true;
            g_ApplicationIPChangeFlags->s_Gb28181ServerIPChangeFlags = true;

            char_t CmdBuffer[MAX_BUFFER_LENGTH];
            memset(CmdBuffer, 0,  sizeof(CmdBuffer));
            snprintf(CmdBuffer, 255,CONFIG_TOOL_SERVER_APPLICATION_KILL);
            if (system(CmdBuffer) < 0)
            {
                DAEMON_PRINT_LOG(ERROR,"Kill config_tool System exe Error! ! ");
                GMI_DeBugPrint("[%s][%d]Kill Transport System exe Error",__func__,__LINE__);
            }

            memset(CmdBuffer, 0,	sizeof(CmdBuffer));
            snprintf(CmdBuffer, 255, CONFIG_TOOL_SERVER_APPLICATION);
            if (system(CmdBuffer) < 0)
            {
                DAEMON_PRINT_LOG(ERROR,"Runnig config_tool  exe Error! ! ");
                GMI_DeBugPrint("[%s][%d]Runnig Transport System exe Error",__func__,__LINE__);
            }

            if (NULL != Buffer)
                free(Buffer);
            return Ret;

        }
        else if (GMI_DAEMON_APPLICATION_SYSTEM_REBOOT == ntohs(l_DaemonMessage->s_DaemonHearder.s_MessageId))
        {
            GMI_DeBugPrint("[%s][%d] Reboot Cmd from server  = %d ! ",__func__,__LINE__,ntohs(l_DaemonMessage->s_LocalServerId));

            if(g_ApplicationOpenFlags->s_Gb28181ServerOpenFlags)
            {
                g_ApplicationQuitFlag->s_GbQuitFlag = true;
                while(g_ApplicationQuitFlag->s_GbQuitFlag)
                {
                    if(0 == Tmp)
                    {
                        break;
                    }
                    sleep(1);
                    --Tmp;
                }
            }

            Tmp=5;
            if(g_ApplicationOpenFlags->s_SdkServerOpenFlags)
            {
                g_ApplicationQuitFlag->s_SdkQuitFlag = true;
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

            Tmp=5;
            if(g_ApplicationOpenFlags->s_OnVifServerOpenFlags)
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

            Tmp=5;
            if(g_ApplicationOpenFlags->s_ControlServerOpenFlags)
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

            Tmp=5;
            if(g_ApplicationOpenFlags->s_MediaServerOpenFlags)
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

            Tmp=5;
            if(g_ApplicationOpenFlags->s_LogServerOpenFlags)
            {
                g_ApplicationQuitFlag->s_LogServerQuitFlag = true;
                while (g_ApplicationQuitFlag->s_LogServerQuitFlag)
                {
                    if(0 == Tmp)
                    {
                        break;
                    }
                    sleep(1);
                    --Tmp;
                }
            }

            Tmp=5;
            if(g_ApplicationOpenFlags->s_AuthServerOpenFlags)
            {
                g_ApplicationQuitFlag->s_AuthQuitFlag = true;
                while (g_ApplicationQuitFlag->s_AuthQuitFlag)
                {
                    if(0 == Tmp)
                    {
                        break;
                    }
                    sleep(1);
                    --Tmp;
                }
            }

            if (NULL != Buffer)
                free(Buffer);

            Ret = GMI_SysHwWatchDogEnable();
            if (FAILED(Ret))
            {
                DAEMON_PRINT_LOG(ERROR,"GMI_Reboot->TestHwWatchDogDisable fail!!!\n");
            }

            Ret = GMI_BrdHwReset();
            if (FAILED(Ret))
            {
                DAEMON_PRINT_LOG(ERROR,"GMI_Reboot! !!!");
            }

        }
        if (NULL != Buffer)
            free(Buffer);
        return Ret;

    }

    return GMI_SUCCESS;
}

/*=============================================================================
name				:	GMI_ServerStatusInquiry
function			:  Inquiry System Server status
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	7/10/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
static void* GMI_ServerStatusInquiry(void *param)
{

    pthread_detach(pthread_self());

    for (;;)
    {
        GMI_InquiryMassge(l_InquirySock);
    }

    pthread_exit(NULL);
    return (void *)0;

}

/*=============================================================================
name				:	GMI_UnInitSystemInquiry
function			: free system Inquiry port resource
algorithm implementation	:	no
global variable			:	no
parameter declaration		:
return				:   no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	7/10/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
void GMI_UnInitSystemInquiryPort(FD_HANDLE SockFd)
{
    GMI_RudpSocketClose(SockFd);
}


/*=============================================================================
name				:	GMI_InitSystemInquiry
function			:  Get Rudp Socket value
algorithm implementation	:	no
global variable			:	no
parameter declaration		:
return				:   no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	7/10/2013	1.0.0.0.0     minchao.wang         establish

******************************************************************************/
FD_HANDLE GMI_InitSystemInquiryPort()
{
    DAEMON_PRINT_LOG(INFO,"GMI_InitSystemInquiry is Error ! ! ");

    long_t LocalPort = GMI_DAEMON_HEARTBEAT_SDK_SERVER;
    FD_HANDLE sock;

    sock = GMI_RudpSocket(LocalPort);
    if (!sock)
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_InitHeartbeat is Error ! ! ");
    }

    return sock;
}

/*=======================================================
name				:	GMI_InitServerQuiry
function			:  Init ServerQuiry resource thread
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
GMI_RESULT GMI_InitServerQuiry(void)
{

    l_InquirySock = GMI_InitSystemInquiryPort();

    DAEMON_PRINT_LOG(INFO,"GMI_ServerStatusInquiry is Start ! ! ");
    GMI_RESULT Status = GMI_FAIL;
    Status =  GMI_CreateTask(190, 8192, (fTaskEntryPoint)GMI_ServerStatusInquiry, 0, 0, 0, 0, 0, 0, 0);
    if (Status < 0)
    {
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}

/*=======================================================
name				:	GMI_UnInitServerQuiry
function			:    UnInit ServerQuiry resource
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
void GMI_UnInitServerQuiry(void)
{
    GMI_UnInitSystemInquiryPort(l_InquirySock);
}



