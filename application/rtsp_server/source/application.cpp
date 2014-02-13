#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "application.h"
#include "heartbeat.h"
#include "media_capture.h"
#include "configure.h"

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
    : m_StopFlag(1)
    , m_RtspService(NULL)
    , m_StreamCtrl()
{
    // TODO: Parse the parameter from command line

    // Initialize log module
    LOG_INITIALIZE("/opt/log/rtsp_server.log", (128 << 10));
    LOG_SET_WRITE_LEVEL(VERBOSE);
    LOG_SET_DISPLAY_LEVEL(DEBUG);

    // Set priority
    int32_t RetVal = setpriority(PRIO_PROCESS, getpid(), -20);
    PRINT_LOG(INFO, "RetVal = %d", RetVal);

    // Register singal handler
    signal(SIGTERM, Application::SignalStop);
    signal(SIGQUIT, Application::SignalStop);
    signal(SIGINT,  Application::SignalStop);

    signal(SIGPIPE, Application::SignalPipe);
}

Application::~Application()
{
    // Destroy all media frame captures
    MediaFrameCapture::DestroyAllMediaFrameCaptures();
}

GMI_RESULT Application::Start()
{
    if (IsRunning())
    {
        return GMI_ALREADY_OPERATED;
    }

    GMI_RESULT RetVal = GMI_SUCCESS;

    do
    {
        RetVal = Configure::GetInstance().Initialize();
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to initialize configure");
            break;
        }

        m_RtspService = RtspService::CreateInstance();
        if (NULL == m_RtspService)
        {
            PRINT_LOG(ERROR, "Failed to create RtspService");
            RetVal = GMI_OUT_OF_MEMORY;
            break;
        }

        RetVal = m_RtspService->Initialize();
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to initialize RtspService");
            break;
        }

        RetVal = m_StreamCtrl.Initialize(m_RtspService);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to initialize StreamCtrl");
            break;
        }

        // Initialize heart beat task
        RetVal = HeartBeat::GetInstance().Initialize();
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to initialize heart beat task");
            break;
        }

        // Start heart beat
        m_HearbeatTask = m_RtspService->AddDelayedTask(Application::HeartBeatTask, NULL, 3000000);

/********************************************************************************************************
        // Test code, used to start stream without stream control module
        const SubStreamInfo StreamInfo1[] = {
            {MEDIA_VIDEO_H264,  0, 25, 4000000}, //  H264 encode with 25fps and  4Mbps on video stream 0
            {MEDIA_AUDIO_G711A, 0, 50, 64000  }, // G711A encode with 16fps and 64Kbps on audio stream 0
        };

        const SubStreamInfo StreamInfo2[] = {
            {MEDIA_VIDEO_H264,  1, 25, 2000000}, //  H264 encode with 25fps and  2Mbps on video stream 1
            {MEDIA_AUDIO_G711A, 0, 50, 64000  }, // G711A encode with 16fps and 64Kbps on audio stream 0
        };

        m_RtspService->StartStream(1, COUNT_OF(StreamInfo1), StreamInfo1);
        m_RtspService->StartStream(2, COUNT_OF(StreamInfo2), StreamInfo2);
 ********************************************************************************************************/

        // Remove the stop flag
        m_StopFlag = 0;

        RetVal = Run();
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(WARNING, "Error happens while running application");
        }

    } while (0);

    HeartBeat::GetInstance().Uninitialize();

    if (m_StreamCtrl.Initialized())
    {
        m_StreamCtrl.Uninitialize();
    }

    if (m_RtspService != NULL)
    {
        delete m_RtspService;
        m_RtspService = NULL;
    }

    Configure::GetInstance().Uninitialize();

    return RetVal;
}

GMI_RESULT Application::Stop()
{
    if (!IsRunning())
    {
        return GMI_ALREADY_OPERATED;
    }

    // Add the stop flag
    m_StopFlag = 1;
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
        PRINT_LOG(WARNING, "Process is stopping ...");
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
    Application & App      = Application::GetSingleton();
    boolean_t     NeedQuit = false;

    GMI_RESULT RetVal = HeartBeat::GetInstance().Report(NeedQuit);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(WARNING, "Failed to report to daemon");
    }

    if (NeedQuit)
    {
        PRINT_LOG(INFO, "Daemon notified the quit message");
        Application & App = Application::GetSingleton();
        if (App.IsRunning())
        {
            PRINT_LOG(INFO, "Try to stop process ...");
            App.Stop();
        }
        else
        {
            PRINT_LOG(WARNING, "Process is stopping ...");
        }
    }
    else
    {
        // Add schedule task
        App.m_HearbeatTask = App.m_RtspService->AddDelayedTask(Application::HeartBeatTask, NULL, 3000000);
    }
}

GMI_RESULT Application::Run()
{
    // Start RTSP service main loop
    m_RtspService->DoEventLoop(&m_StopFlag);

    return GMI_SUCCESS;
}

