#include "daemon_service.h"
#include "configure_service.h"

DaemonService::DaemonService()
    : m_Initialized(false)
{
}

DaemonService::~DaemonService()
{
    if (Initialized())
    {
        Uninitialize();
    }
}

GMI_RESULT DaemonService::Initialize()
{
    if (Initialized())
    {
        return GMI_ALREADY_OPERATED;
    }

    ConfigureService & Svr = ConfigureService::GetInstance();

    GMI_RESULT RetVal = GMI_DaemonInit(&m_DaemonData, Svr.GetDaemonLocalModuleId(), Svr.GetDaemonRemotePort(), Svr.GetDaemonLocalPort());
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

GMI_RESULT DaemonService::Uninitialize()
{
    if (Initialized())
    {
        GMI_DaemonUnRegister(&m_DaemonData);
        GMI_DaemonUnInit(&m_DaemonData);

        m_Initialized = false;
        return GMI_SUCCESS;
    }

    return GMI_INVALID_OPERATION;
}

GMI_RESULT DaemonService::Report(boolean_t & NeedQuit)
{
    if (!Initialized())
    {
        return GMI_INVALID_OPERATION;
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

GMI_RESULT DaemonService::RebootSystem()
{
    if (!Initialized())
    {
        return GMI_INVALID_OPERATION;
    }

    GMI_RESULT RetVal = GMI_SystemReboot(&m_DaemonData, ConfigureService::GetInstance().GetDaemonServerPort());
    if (GMI_SUCCESS != RetVal)
    {
        PRINT_LOG(WARNING, "Failed to reboot system");
    }

    return RetVal;
}

GMI_RESULT DaemonService::QuerySystemStatus(SystemStatus & Status)
{
    if (!Initialized())
    {
        return GMI_INVALID_OPERATION;
    }

    uint16_t   AppStatus = APPLICATION_STATUS_OFFLINE;
    GMI_RESULT RetVal    = GMI_InquiryServerStatus(&m_DaemonData, ConfigureService::GetInstance().GetDaemonServerPort(), GMI_DAEMON_APPLICATION_STATUS_QUIRY, SDK_SERVER_ID, &AppStatus);
    if (GMI_SUCCESS != RetVal)
    {
        PRINT_LOG(WARNING, "Failed to query system status");
        return RetVal;
    }

    switch (AppStatus)
    {
    case APPLICATION_STATUS_REBOOT:
    case APPLICATION_STATUS_OFFLINE:
        Status = ePreparing;
        break;

    case APPLICATION_STATUS_ONLINE:
        Status = eOnLine;
        break;

    case APPLICATION_STATUS_ERROR:
    default:
        Status = eUnexpect;
        break;
    }

    return RetVal;
}

