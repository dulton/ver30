#include "heart_beat.h"
#include "configure_service.h"

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

GMI_RESULT HeartBeat::Uninitialize()
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

GMI_RESULT HeartBeat::Report(boolean_t & NeedQuit)
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

GMI_RESULT HeartBeat::RebootSystem()
{
    if (!Initialized())
    {
        return GMI_INVALID_OPERATION;
    }

    GMI_RESULT RetVal = GMI_SystemReboot(&m_DaemonData, ConfigureService::GetInstance().GetDaemonRebootPort());
    if (GMI_SUCCESS != RetVal)
    {
        PRINT_LOG(WARNING, "Failed to reboot system");
    }

    return RetVal;
}

