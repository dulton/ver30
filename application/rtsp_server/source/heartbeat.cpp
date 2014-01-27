#include <ipc_fw_v3.x_resource.h>

#include "heartbeat.h"

HeartBeat::HeartBeat()
    : m_Initialized(false)
{
}

HeartBeat::~HeartBeat()
{
    if (Initialized())
    {
        Uninitialize();
    }
}

GMI_RESULT HeartBeat::Initialize()
{
    if (Initialized())
    {
        return GMI_ALREADY_OPERATED;
    }

    GMI_RESULT RetVal = GMI_DaemonInit(&m_DaemonData, TRANSPORT_SERVER_ID, GMI_DAEMON_HEARDBEAT_SERVER, GMI_DAEMON_HEARDBEAT_TRANSPORT);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to initialize");
        return RetVal;
    }

    RetVal = GMI_DaemonRegister(&m_DaemonData);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to register");
        GMI_DaemonUnInit(&m_DaemonData);
        return RetVal;
    }

    m_Initialized = true;
    return GMI_SUCCESS;
}

GMI_RESULT HeartBeat::Uninitialize()
{
    if (Initialized())
    {
        GMI_DaemonUnRegister(&m_DaemonData);
        GMI_DaemonUnInit(&m_DaemonData);

        m_Initialized = false;
    }

    return GMI_SUCCESS;
}

GMI_RESULT HeartBeat::Report(boolean_t & NeedQuit)
{
    if (!Initialized())
    {
        return GMI_FAIL;
    }

    uint32_t   Flags  = 0;
    GMI_RESULT RetVal = GMI_DaemonReport(&m_DaemonData, &Flags);
    if (GMI_SUCCESS != RetVal)
    {
        PRINT_LOG(WARNING, "Failed to report to daemon");
    }
    else
    {
        NeedQuit = (Flags == APPLICATION_QUIT);
    }

    return RetVal;
}

GMI_RESULT HeartBeat::RebootSystem()
{
    if (!Initialized())
    {
        return GMI_FAIL;
    }

    GMI_RESULT RetVal = GMI_SystemReboot(&m_DaemonData, GMI_DAEMON_HEARTBEAT_STATUS_QUERY);
    if (GMI_SUCCESS != RetVal)
    {
        PRINT_LOG(WARNING, "Failed to reboot system");
    }

    return RetVal;
}

