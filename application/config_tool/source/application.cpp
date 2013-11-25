#include <signal.h>

#include "application.h"
#include "heart_beat.h"
#include "configure_service.h"
#include "service_dispatch.h"

#define DEFAULT_NETWORK_INTERFACE "eth0"
#define DEFAULT_PCAP_TIMEDOUT     500

static const uint8_t l_OffLineNotification[] = "<Notify operation=\"OffLine\" />";

inline static const char_t * Signal2String(int32_t SignalNumber)
{
    switch (SignalNumber)
    {
#define CASE_SIGNAL(sig) case sig: return #sig

        CASE_SIGNAL(SIGTERM);
        CASE_SIGNAL(SIGQUIT);
        CASE_SIGNAL(SIGINT);
        CASE_SIGNAL(SIGPIPE);
        CASE_SIGNAL(SIGCHLD);

#undef CASE_SIGNAL
    }

    return "Unknown";
}

template <>
Application * Singleton<Application>::ms_InstancePtr = NULL;

Application::Application(int32_t argc, const char_t * argv [])
    : m_Running(false)
    , m_Interface(DEFAULT_NETWORK_INTERFACE)
    , m_TimedOut(DEFAULT_PCAP_TIMEDOUT)
    , m_GtpHandle(GTP_INVALID_HANDLE)
{
    // TODO: Parse the parameter from command line

    // Initialized log module
    LOG_INITIALIZE("/opt/log/config_tool.log", (128 << 10));
    LOG_SET_DISPLAY_LEVEL(DEBUG);
    LOG_SET_WRITE_LEVEL(DEBUG);

    PRINT_LOG(INFO, "Process is starting, pid = %d", getpid());

    // Register singal handler
    signal(SIGTERM, Application::SignalStop);
    signal(SIGQUIT, Application::SignalStop);
    signal(SIGINT,  Application::SignalStop);

    signal(SIGPIPE, Application::SignalPipe);
}

Application::~Application()
{
    PRINT_LOG(INFO, "Process is stopped");
}

GMI_RESULT Application::Start()
{
    if (IsRunning())
    {
        return GMI_ALREADY_OPERATED;
    }

    GMI_RESULT        RetVal   = GMI_SUCCESS;
    PcapSessionHandle PcapHnd  = PCAP_SESSION_INVALID_HANDLE;

    do
    {
        // Initialize configure service
        RetVal = ConfigureService::GetInstance().Initialize();
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to initialize configure service");
            break;
        }

        // Initialize heart beat
        RetVal = HeartBeat::GetInstance().Initialize();
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to initialize heart beat");
            break;
        }

        // Create pcap handle
        PcapHnd = PcapSessionOpen(m_Interface, m_TimedOut);
        if (PCAP_SESSION_INVALID_HANDLE == PcapHnd)
        {
            PRINT_LOG(ERROR, "Failed to open interface %s", m_Interface);
            RetVal = GMI_FAIL;
            break;
        }

        // Create tool protocol handle
        m_GtpHandle = GtpCreateServerHandle(PcapHnd);
        if (GTP_INVALID_HANDLE == m_GtpHandle)
        {
            PRINT_LOG(ERROR, "Failed to create GTP handle");
            RetVal = GMI_FAIL;
            break;
        }

        // Start heart beat task
        RetVal = GtpAddDelayTask(m_GtpHandle, Application::HeartBeatTask, this, 0, 0);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to add heart beat task");
            break;
        }

        // Set event proc
        GtpSetSentCallback(m_GtpHandle, ServiceDispatch::OnSentProc, ServiceDispatch::GetInstancePtr());
        GtpSetRecvCallback(m_GtpHandle, ServiceDispatch::OnRecvProc, ServiceDispatch::GetInstancePtr());
        GtpSetTransKilledCallback(m_GtpHandle, ServiceDispatch::OnTransactionKilledProc, ServiceDispatch::GetInstancePtr());

        // Set the loop flag
        m_Running = true;

        // Start main loop
        RetVal = Run();
    } while (0);

    // Release the resource
    if (m_GtpHandle != GTP_INVALID_HANDLE)
    {
        GtpDestroyHandle(m_GtpHandle);
        m_GtpHandle = GTP_INVALID_HANDLE;
    }

    if (PcapHnd != PCAP_SESSION_INVALID_HANDLE)
    {
        PcapSessionClose(PcapHnd);
    }

    HeartBeat::GetInstance().Uninitialize();
    ConfigureService::GetInstance().Uninitialize();

    return RetVal;
}

GMI_RESULT Application::Stop()
{
    if (!IsRunning())
    {
        return GMI_INVALID_OPERATION;
    }

    // Remove the loop flag
    m_Running = false;

    return GMI_SUCCESS;
}

// Handle SIGTERM, SIGQUIT and SIGINT
void_t Application::SignalStop(int32_t SignalNumber)
{
    PRINT_LOG(INFO, "Application received signal %s ...", Signal2String(SignalNumber));

    Application & App = Application::GetSingleton();
    if (App.IsRunning())
    {
        PRINT_LOG(INFO, "Try to stop process ...");
        App.Stop();
    }
    else
    {
        PRINT_LOG(WARNING, "Process is already stopping ...");
    }
}

// Handle SIGPIPE
void_t Application::SignalPipe(int32_t SignalNumber)
{
    PRINT_LOG(INFO, "Application received signal %s ...", Signal2String(SignalNumber));
    PRINT_LOG(VERBOSE, "Nothing to do.");
}

void_t Application::HeartBeatTask(void_t * Data)
{
    ASSERT(Data != NULL, "Data MUST NOT be non-pointer");

    Application * App      = static_cast<Application *>(Data);
    boolean_t     NeedQuit = false;
    GMI_RESULT    RetVal   = GMI_SUCCESS;

    // PRINT_LOG(VERBOSE, "Try to report to daemon process");

    RetVal = HeartBeat::GetInstance().Report(NeedQuit);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(WARNING, "Failed to report to daemon");
    }

    // DUMP_VARIABLE(NeedQuit);

    if (NeedQuit)
    {
        PRINT_LOG(INFO, "Daemon notified the quit message");
        if (App->IsRunning())
        {
            PRINT_LOG(INFO, "Try to stop process ...");
            App->Stop();
        }
        else
        {
            PRINT_LOG(WARNING, "Process is stopping ...");
        }
    }
    else
    {
        RetVal = GtpAddDelayTask(App->m_GtpHandle, Application::HeartBeatTask, Data, 3, 0);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to add heart beat task");
        }
    }
}

GMI_RESULT Application::Run()
{
    GMI_RESULT     RetVal   = GMI_SUCCESS;
    GtpTransHandle GtpTrans = GTP_INVALID_HANDLE;

    // Main loop
    while (IsRunning())
    {
        // Single step
        RetVal = GtpDoEvent(m_GtpHandle);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to do GTP event");
        }
    }

    do
    {
        // Broadcast the offline notification
        GtpTrans = GtpCreateBroadcastTransHandle(m_GtpHandle);
        if (GTP_INVALID_HANDLE == GtpTrans)
        {
            PRINT_LOG(ERROR, "Failed to create GTP broadcast transaction handle");
            break;
        }

        if (GtpTransSendData(GtpTrans, l_OffLineNotification,
            sizeof(l_OffLineNotification), GTP_DATA_TYPE_UTF8_XML) != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to send off line notification");
            break;
        }

    } while (0);

    // Release the resource
    if (GtpTrans != GTP_INVALID_HANDLE)
    {
        GtpDestroyTransHandle(GtpTrans);
    }

    return RetVal;
}

