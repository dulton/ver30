
#include "gmi_system_headers.h"
#include "thread_utils_headers.h"
#include "ipc_fw_v3.x_resource.h"
#include "gmi_daemon_heartbeat_api.h"
#include "daemon.h"
#include "log.h"

static DAEMON_DATA_CFG l_DaemonHandler;
static boolean_t  l_ThreadExitFlag = false;
static GMI_Thread l_TimerRebootThread;
static uint16_t   l_DelayTime;
static boolean_t  l_TimerRebootEnable = false;

static uint16_t   l_NotifyIpChanged_DelayTime;
static boolean_t  l_NotifyIpChanged_TimerEnable = false;
static int32_t    l_NotifyIpChanged_EthId;

static void_t* TimerRebootThread(void*)
{
    GMI_RESULT Result;

    while (!l_ThreadExitFlag)
    {
        if (l_TimerRebootEnable)
        {
            if (l_DelayTime <= 0)
            {
                Result = DaemonReboot();
                if (FAILED(Result))
                {
                    DaemonReboot();
                }
            }
            l_DelayTime--;
        }

        if (l_NotifyIpChanged_TimerEnable)
        {
            if (l_NotifyIpChanged_DelayTime <= 0)
            {
                l_NotifyIpChanged_TimerEnable = false;

                char_t CmdBuff[64];
                memset(CmdBuff, 0, sizeof(CmdBuff));
                sprintf(CmdBuff, "ifdown eth%d", l_NotifyIpChanged_EthId);
                system(CmdBuff);

                memset(CmdBuff, 0, sizeof(CmdBuff));
                sprintf(CmdBuff, "ifup eth%d", l_NotifyIpChanged_EthId);
                system(CmdBuff);

                DaemonReportIpChanged();
            }

            l_NotifyIpChanged_DelayTime--;
        }

        sleep(1);
    }

    l_ThreadExitFlag = false;

    return (void *) GMI_SUCCESS;
}

static GMI_RESULT TimerRebootCreate()
{
    l_ThreadExitFlag  = false;
    GMI_RESULT Result = l_TimerRebootThread.Create(NULL, 0, TimerRebootThread, NULL);
    if (FAILED(Result))
    {
        SYS_ERROR("l_TimerRebootThread.Create fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "l_TimerRebootThread.Create fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = l_TimerRebootThread.Start();
    if (FAILED(Result))
    {
        l_TimerRebootThread.Destroy();
        SYS_ERROR("l_TimerRebootThread.Start fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "l_TimerRebootThread.Start fail, Result = 0x%lx\n", Result);
        return Result;
    }

    return GMI_SUCCESS;
}

/*
static GMI_RESULT TimerRebootDelete()
{
	l_ThreadExitFlag = true;
    while (l_ThreadExitFlag)
    {
        GMI_Sleep(100);
    }

    GMI_RESULT Result = l_TimerRebootThread.Destroy();
    if (FAILED(Result))
    {
        SYS_ERROR("l_TimerRebootThread.Destroy() fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "l_TimerRebootThread.Destroy() fail, Result = 0x%lx\n", Result);
        return Result;
    }

    return GMI_SUCCESS;
}
*/


GMI_RESULT DaemonRegister(void)
{
    uint8_t RegisterNum = 10;
    GMI_RESULT Result = GMI_SUCCESS;

    SYS_INFO("%s in.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in.........\n", __func__);
    Result = GMI_DaemonInit(&l_DaemonHandler, CONTROL_SERVER_ID, GMI_DAEMON_HEARDBEAT_SERVER, GMI_DAEMON_HEARDBEAT_CONTROL);
    if (FAILED(Result))
    {
        SYS_ERROR("DaemonInit error, Result = 0x%lx\n", Result);
        return Result;
    }

    do
    {
        Result = GMI_DaemonRegister(&l_DaemonHandler);
        if (SUCCEEDED(Result))
        {
            SYS_INFO("DaemonRegister OK\n");
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "DaemonRegister OK\n");
            break;
        }
        sleep(1);
    }
    while (RegisterNum--);

    Result = TimerRebootCreate();
    if (FAILED(Result))
    {
        SYS_ERROR("TimerRebootCreate fail, Result = 0x%lx\n", Result);
        return Result;
    }
    SYS_INFO("%s normal out.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out.........\n", __func__);
    return Result;
}


GMI_RESULT DaemonKeepAlive(uint32_t *BootFlag)
{
    GMI_RESULT Result;

    if (NULL == BootFlag)
    {
        return GMI_INVALID_PARAMETER;
    }

    Result = GMI_DaemonReport(&l_DaemonHandler, BootFlag);
    if (FAILED(Result))
    {
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}


void DaemonUnregister(void)
{
    GMI_DaemonUnInit(&l_DaemonHandler);
    return;
}


GMI_RESULT DaemonQueryServerStatus(int32_t ServerId, uint16_t *StatusPtr)
{
    uint16_t StatusTmp;

    if (ServerId < LOG_SERVER_ID
            || ServerId > AUTH_SERVER_ID)
    {
        SYS_ERROR("ServerId %d error\n", ServerId);
        return GMI_INVALID_PARAMETER;
    }

    if (NULL == StatusPtr)
    {
        SYS_ERROR("StatusPtr is null\n");
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Result = GMI_InquiryServerStatus(&l_DaemonHandler, GMI_MONITOR_TO_SDK_PORT_DEFAULT, GMI_DAEMON_APPLICATION_STATUS_QUIRY, ServerId, &StatusTmp);
    if (FAILED(Result))
    {
        SYS_ERROR("InquiryServerStatus fail, Result = 0x%lx\n", Result);
        return Result;
    }

    *StatusPtr = StatusTmp;

    return GMI_SUCCESS;
}


GMI_RESULT DaemonReboot(void)
{
    GMI_RESULT Result = GMI_SystemReboot(&l_DaemonHandler, GMI_MONITOR_TO_SDK_PORT_DEFAULT);
    if (FAILED(Result))
    {
        SYS_ERROR("SystemReboot fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "l_TimerRebootThread.Start fail, Result = 0x%lx\n", Result);
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT DaemonReboot(uint16_t DelayTime)
{
    l_DelayTime = DelayTime;
    l_TimerRebootEnable = true;

    return GMI_SUCCESS;
}


GMI_RESULT DaemonReportIpChanged(void)
{
    GMI_RESULT Result = GMI_SystemIPChangeReport(&l_DaemonHandler, GMI_MONITOR_TO_SDK_PORT_DEFAULT);
    if (FAILED(Result))
    {
        GMI_SystemIPChangeReport(&l_DaemonHandler, GMI_MONITOR_TO_SDK_PORT_DEFAULT);
        return Result;
    }

    return Result;
}

GMI_RESULT DaemonReportIpChanged(int32_t EthId, uint16_t DelayTime)
{
    l_NotifyIpChanged_DelayTime = DelayTime;
    l_NotifyIpChanged_TimerEnable = true;
    l_NotifyIpChanged_EthId = EthId;
    return GMI_SUCCESS;
}

