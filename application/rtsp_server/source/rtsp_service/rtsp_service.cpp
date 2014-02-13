#include <BasicUsageEnvironment.hh>
#include <RTSPServer.hh>
#include <H264RealTimeStreamServerMediaSubsession.hh>
#include <G711RealTimeStreamServerMediaSubsession.hh>

#include "GMITaskScheduler.hh"

#include "configure.h"
#include "rtsp_service.h"

// Max size of one frame is 1M bytes
#define MAX_BYTES_PER_FRAME     (1 << 20)

// Max size of steam name string
#define MAX_STREAM_NAME_LENGTH  128

static const char_t l_DescriptionString[] = "Session streamed by \"GMI RTSP Server\"";

class RtspServiceImpl : public RtspService
{
public:
    RtspServiceImpl();
    virtual ~RtspServiceImpl();

    virtual GMI_RESULT Initialize();
    virtual GMI_RESULT Uninitiialize();

    virtual GMI_RESULT StartStream(uint32_t StreamId, uint32_t SubStreamNum, const SubStreamInfo * SubStreams);
    virtual GMI_RESULT StopStream(uint32_t StreamId);
    virtual boolean_t IsStreamStarted(uint32_t StreamId);

    virtual void_t DoEventLoop(uint8_t * StopFlag = NULL);

    virtual TaskToken AddDelayedTask(TaskFunc Func, void_t * Data, uint32_t USec);
    virtual void_t RemoveDelayedTask(TaskToken TaskId);

    virtual void_t AddFdListener(int32_t Fd, FdListener Listener, void_t * Data);
    virtual void_t RemoveFdListener(int32_t Fd);

protected:
    boolean_t                    m_Initialized;
    TaskScheduler              * m_TaskScheduler;
    UsageEnvironment           * m_UsageEnvironment;
    UserAuthenticationDatabase * m_UserAuthDatabase;
    RTSPServer                 * m_RTSPServer;
};

RtspService::RtspService()
{
}

RtspService::~RtspService()
{
}

RtspService * RtspService::CreateInstance()
{
    return new RtspServiceImpl();
}

/*
 * Implementation of RTSP service
 */
RtspServiceImpl::RtspServiceImpl()
    : RtspService()
    , m_Initialized(false)
    , m_TaskScheduler(NULL)
    , m_UsageEnvironment(NULL)
    , m_UserAuthDatabase(NULL)
    , m_RTSPServer(NULL)
{
    OutPacketBuffer::maxSize = MAX_BYTES_PER_FRAME;
}

RtspServiceImpl::~RtspServiceImpl()
{
    if (m_Initialized)
    {
        // Uninitialize instance
        Uninitiialize();
    }
}

GMI_RESULT RtspServiceImpl::Initialize()
{
    if (m_Initialized)
    {
        PRINT_LOG(WARNING, "Aleady initialized");
        return GMI_ALREADY_OPERATED;
    }

    GMI_RESULT RetVal = GMI_SUCCESS;

    do
    {
        // Create task scheduler
        m_TaskScheduler = GMITaskScheduler::createNew();
        if (NULL == m_TaskScheduler)
        {
            PRINT_LOG(ERROR, "Failed to create GMITaskScheduler");
            RetVal = GMI_OUT_OF_MEMORY;
            break;
        }

        // Create usage environment
        m_UsageEnvironment = BasicUsageEnvironment::createNew(*m_TaskScheduler);
        if (NULL == m_UsageEnvironment)
        {
            PRINT_LOG(ERROR, "Failed to create BasicUsageEnvironment");
            RetVal = GMI_OUT_OF_MEMORY;
            break;
        }

        // TODO: Create user auth database
        m_UserAuthDatabase = NULL;

        // Create RTSP server
        m_RTSPServer = RTSPServer::createNew(*m_UsageEnvironment, Configure::GetInstance().GetRtspServicePort(), m_UserAuthDatabase, 0);
        if (NULL == m_RTSPServer)
        {
            PRINT_LOG(ERROR, "Failed to create RTSPServer");
            RetVal = GMI_OUT_OF_MEMORY;
            break;
        }

        m_Initialized = true;
        return GMI_SUCCESS;
    } while (0);

    if (m_RTSPServer != NULL)
    {
        Medium::close(m_RTSPServer);
        m_RTSPServer = NULL;
    }

    if (m_UsageEnvironment != NULL)
    {
        m_UsageEnvironment->reclaim();
        m_UsageEnvironment = NULL;
    }

    if (m_TaskScheduler != NULL)
    {
        delete m_TaskScheduler;
        m_TaskScheduler = NULL;
    }

    return RetVal;
}

GMI_RESULT RtspServiceImpl::Uninitiialize()
{
    if (!m_Initialized)
    {
        PRINT_LOG(WARNING, "Not initialized yet");
        return GMI_ALREADY_OPERATED;
    }

    Medium::close(m_RTSPServer);
    m_RTSPServer = NULL;

    m_UsageEnvironment->reclaim();
    m_UsageEnvironment = NULL;

    delete m_TaskScheduler;
    m_TaskScheduler = NULL;

    m_Initialized = false;
    return GMI_SUCCESS;
}

// Help the generate stream name using stream id based from 1
static inline const char_t * GenerateStreamName(uint32_t StreamId)
{
    static char_t StreamName[MAX_STREAM_NAME_LENGTH];
    snprintf(StreamName, sizeof(StreamName), "stream%u", StreamId);
    return StreamName;
}

// Start rtsp stream using sub stream information
GMI_RESULT RtspServiceImpl::StartStream(uint32_t StreamId, uint32_t SubStreamNum, const SubStreamInfo * SubStreams)
{
    if (!m_Initialized)
    {
        PRINT_LOG(ERROR, "Instance is not initialized");
        return GMI_FAIL;
    }

    // Must have at least 1 sub stream
    if (0 == SubStreamNum || NULL == SubStreams)
    {
        PRINT_LOG(ERROR, "Parameter failed");
        return GMI_INVALID_PARAMETER;
    }

    const char_t          * StreamName = GenerateStreamName(StreamId);
    ServerMediaSubsession * SubSession = NULL;
    GMI_RESULT              RetVal     = GMI_SUCCESS;
    ServerMediaSession    * Session    = m_RTSPServer->lookupServerMediaSession(StreamName);
    if (Session != NULL)
    {
        PRINT_LOG(WARNING, "Stream '%s' is already started", StreamName);
        return GMI_ALREADY_OPERATED;
    }

    Session = ServerMediaSession::createNew(*m_UsageEnvironment, StreamName, StreamName, l_DescriptionString, False);
    if (NULL == Session)
    {
        PRINT_LOG(ERROR, "Failed to create ServerMediaSession");
        return GMI_OUT_OF_MEMORY;
    }

    for (uint32_t i = 0; i < SubStreamNum; ++ i)
    {
        switch (SubStreams[i].s_EncodeType)
        {
            case MEDIA_VIDEO_H264:
                SubSession = H264RealTimeStreamServerMediaSubsession::createNew(*m_UsageEnvironment, SubStreams[i], True);
                if (NULL == SubSession)
                {
                    PRINT_LOG(ERROR, "Failed to create H264RealTimeStreamServerMediaSubsession");
                    RetVal = GMI_OUT_OF_MEMORY;
                }
                break;

            case MEDIA_AUDIO_G711A:
#if 0
                SubSession = G711RealTimeStreamServerMediaSubsession::createNew(*m_UsageEnvironment, SubStreams[i], True);
                if (NULL == SubSession)
                {
                    PRINT_LOG(ERROR, "Failed to create G711RealTimeStreamServerMediaSubsession");
                    RetVal = GMI_OUT_OF_MEMORY;
                }
#endif
                break;

            case MEDIA_VIDEO_MJPEG:
            case MEDIA_VIDEO_MPEG4:
            case MEDIA_AUDIO_G711U:
            case MEDIA_AUDIO_G726:
                PRINT_LOG(ERROR, "Unsupported encode type");
                RetVal = GMI_NOT_SUPPORT;
                break;

            default:
                PRINT_LOG(ERROR, "Unknown encode type");
                RetVal = GMI_INVALID_PARAMETER;
                break;
        }

        if (RetVal == GMI_SUCCESS && SubSession != NULL)
        {
            Session->addSubsession(SubSession);
        }
        else
        {
            PRINT_LOG(ERROR, "Failed to create ServerMediaSubsession");
            break;
        }
    }

    if (RetVal == GMI_SUCCESS)
    {
        m_RTSPServer->addServerMediaSession(Session);

        const char * URL = m_RTSPServer->rtspURL(Session);
        PRINT_LOG(INFO, "Play this stream using the URL '%s'", URL);
        delete URL;
    }
    else
    {
        Medium::close(Session);
    }

    return RetVal;
}

GMI_RESULT RtspServiceImpl::StopStream(uint32_t StreamId)
{
    if (!m_Initialized)
    {
        PRINT_LOG(ERROR, "Instance is not initialized");
        return GMI_FAIL;
    }

    m_RTSPServer->deleteServerMediaSession(GenerateStreamName(StreamId));
    return GMI_SUCCESS;
}

boolean_t RtspServiceImpl::IsStreamStarted(uint32_t StreamId)
{
    if (!m_Initialized)
    {
        PRINT_LOG(ERROR, "Instance is not initialized");
        return false;
    }

    return m_RTSPServer->lookupServerMediaSession(GenerateStreamName(StreamId)) != NULL;
}

void_t RtspServiceImpl::DoEventLoop(uint8_t * StopFlag)
{
    if (!m_Initialized)
    {
        PRINT_LOG(ERROR, "Instance is not initialized");
        return;
    }

    m_TaskScheduler->doEventLoop((char *)StopFlag);
}

RtspService::TaskToken RtspServiceImpl::AddDelayedTask(TaskFunc Func, void_t * Data, uint32_t USec)
{
    if (!m_Initialized)
    {
        PRINT_LOG(ERROR, "Instance is not initialized");
        return NULL;
    }

    return m_TaskScheduler->scheduleDelayedTask(USec, Func, Data);
}

void_t RtspServiceImpl::RemoveDelayedTask(RtspService::TaskToken TaskId)
{
    if (!m_Initialized)
    {
        PRINT_LOG(ERROR, "Instance is not initialized");
        return;
    }

    m_TaskScheduler->unscheduleDelayedTask(TaskId);
}

void_t RtspServiceImpl::AddFdListener(int32_t Fd, FdListener Listener, void_t * Data)
{
    if (!m_Initialized)
    {
        PRINT_LOG(ERROR, "Instance is not initialized");
        return;
    }

    m_TaskScheduler->turnOnBackgroundReadHandling(Fd, Listener, Data);
}

void_t RtspServiceImpl::RemoveFdListener(int32_t Fd)
{
    if (!m_Initialized)
    {
        PRINT_LOG(ERROR, "Instance is not initialized");
        return;
    }

    m_TaskScheduler->turnOffBackgroundReadHandling(Fd);
}

