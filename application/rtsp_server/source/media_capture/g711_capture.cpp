#include "g711_capture.h"
#include "configure.h"

G711FrameCapture::G711FrameCapture(const SubStreamInfo & Info, uint32_t MaxFrameCount)
    : MediaFrameCapture(Info)
    , m_MediaDataClient()
    , m_Thread()
    , m_Mutex()
    , m_Working(false)
    , m_WorkDone(true)
    , m_MaxFrameCount(MaxFrameCount)
    , m_UsingFrameCount(0)
    , m_FrameBuf(NULL)
    , m_G711FrameArray(NULL)
    , m_Head()
{
}

G711FrameCapture::~G711FrameCapture()
{
    if (IsCapturing())
    {
        PRINT_LOG(WARNING, "G711 frame capture is still working, shutdown");
        StopCaptureImpl();
    }
}

MediaFrame * G711FrameCapture::GetNextFrame(MediaFrame * PrevFrame)
{
    GMI_AUTO_LOCK(m_Mutex);

    if (m_Head.Prev() == &m_Head || (PrevFrame != NULL && !PrevFrame->IsG711Frame()))
    {
        return NULL;
    }

    MediaFrame * Frame = NULL;

    if (NULL == PrevFrame)
    {
        Frame = dynamic_cast<MediaFrame *>(m_Head.Prev());
    }
    else if (PrevFrame->Next() == &m_Head)
    {
        return NULL;
    }
    else
    {
        Frame = dynamic_cast<MediaFrame *>(PrevFrame->Next());
        PrevFrame->Release();
    }

    Frame->AddRef();
    return Frame;
}

void_t G711FrameCapture::RecycleLastFrame(MediaFrame * LastFrame)
{
    if (NULL == LastFrame || !LastFrame->IsG711Frame())
    {
        return;
    }

    GMI_AUTO_LOCK(m_Mutex);
    LastFrame->Release();
}

GMI_RESULT G711FrameCapture::StartCaptureImpl()
{
    if (m_Working)
    {
        return GMI_ALREADY_OPERATED;
    }

    GMI_RESULT RetVal       = GMI_SUCCESS;
    uint16_t   LocalPort    = Configure::GetInstance().GetAudioStreamLocalPort(m_SubStreamInfo.s_StreamId);
    uint16_t   RemotePort   = Configure::GetInstance().GetAudioStreamRemotePort(m_SubStreamInfo.s_StreamId);
    uint32_t   MaxFrameSize = m_SubStreamInfo.s_MaxBitRate / (m_SubStreamInfo.s_FrameRate * BITS_PER_BYTE);
    MaxFrameSize += (m_SubStreamInfo.s_MaxBitRate % (m_SubStreamInfo.s_FrameRate * BITS_PER_BYTE)) > 0 ? 1 : 0;

    do
    {
        m_Working = true;
        m_WorkDone = false;

        m_FrameBuf = new uint8_t [MaxFrameSize * m_MaxFrameCount];
        if (NULL == m_FrameBuf)
        {
            PRINT_LOG(ERROR, "Not enough memory");
            RetVal = GMI_OUT_OF_MEMORY;
            break;
        }

        m_G711FrameArray = new G711Frame [m_MaxFrameCount];
        if (NULL == m_G711FrameArray)
        {
            PRINT_LOG(ERROR, "Not enough memory");
            RetVal = GMI_OUT_OF_MEMORY;
            break;
        }

        for (uint32_t i = 0; i < m_MaxFrameCount; ++ i)
        {
            m_G711FrameArray[i].Assign(m_FrameBuf + (MaxFrameSize * i), MaxFrameSize);
        }

        RetVal = m_MediaDataClient.Initialize(LocalPort, Configure::GetInstance().GetStreamApplicationId());
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to initialize media data client");
            break;
        }

        RetVal = m_MediaDataClient.Register(RemotePort, m_SubStreamInfo.s_EncodeType, m_SubStreamInfo.s_StreamId, true, NULL, NULL);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to register media data client");
            break;
        }

        RetVal = m_Mutex.Create("G711FrameCapture");
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to create mutex");
            break;
        }
#ifndef USE_OSAL_THREAD
        if (pthread_create(&m_Thread, NULL, G711FrameCapture::ThreadEntry, static_cast<void_t *>(this)) != 0)
        {
            PRINT_LOG(ERROR, "Failed to create thread");
            RetVal = GMI_FAIL;
            break;
        }
#else
        RetVal = m_Thread.Create("G711FrameCapture", 0, G711FrameCapture::ThreadEntry, static_cast<void_t *>(this));
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to create thread");
            break;
        }

        RetVal = m_Thread.Start();
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to start thread");
            break;
        }
#endif
        return GMI_SUCCESS;

    } while (0);

    // Destory all resouces
#ifdef USE_OSAL_THREAD
    m_Thread.Stop();
    m_Thread.Destroy();
#endif
    m_Mutex.Destroy();

    m_MediaDataClient.Unregister();
    m_MediaDataClient.Deinitialize();

    if (m_G711FrameArray)
    {
        delete [] m_G711FrameArray;
        m_G711FrameArray = NULL;
    }

    if (m_FrameBuf)
    {
        delete [] m_FrameBuf;
        m_FrameBuf = NULL;
    }

    m_Working = false;
    m_WorkDone = true;
    return RetVal;
}

GMI_RESULT G711FrameCapture::StopCaptureImpl()
{
    if (m_WorkDone)
    {
        return GMI_ALREADY_OPERATED;
    }

    PRINT_LOG(VERBOSE, "Try to stop G711 frame capture ...");

    m_Working = false;

    // Destory all resouces
#ifndef USE_OSAL_THREAD
    pthread_join(m_Thread, NULL);
#else
    while (m_WorkDone)
    {
        usleep(10000);
    }

    m_Thread.Stop();
    m_Thread.Destroy();
#endif
    m_Mutex.Destroy();

    m_MediaDataClient.Unregister();
    m_MediaDataClient.Deinitialize();

    m_Head.Detach();
    delete [] m_G711FrameArray;
    m_G711FrameArray = NULL;
    delete [] m_FrameBuf;
    m_FrameBuf = NULL;

    m_UsingFrameCount = 0;

    return GMI_SUCCESS;
}

void_t * G711FrameCapture::ThreadEntry(void_t * Data)
{
    ASSERT(Data != NULL, "Data MUST NOT be non-pointer");

    G711FrameCapture * This = static_cast<G711FrameCapture *>(Data);
    return reinterpret_cast<void_t *>(This->ThreadEntryImpl());
}

uint32_t G711FrameCapture::ThreadEntryImpl()
{
    PRINT_LOG(VERBOSE, "Start to capture G711 frames");

    G711Frame * NextFrame = NULL;
    while (m_Working)
    {
        if (NULL == NextFrame)
        {
            NextFrame = FreeFrame();
            if (NULL == NextFrame)
            {
                usleep(10000);
                continue;
            }
        }

        G711Frame & Frame = *NextFrame;

        // Reset this frame
        Frame.Reset();

        uint8_t        * Addr             = Frame.Addr();
        uint32_t         Size             = Frame.MaxLength();
        struct timeval   presentationTime = {0, 0};
        GMI_RESULT       RetVal           = GMI_SUCCESS;

        // Read one frame data
        RetVal = m_MediaDataClient.Read(Addr, &Size, &presentationTime, NULL, NULL, m_Sleep);
        if (RetVal != GMI_SUCCESS)
        {
            continue;
        }

        Frame.SetLength(Size);
        Frame.SetTimeStamp(presentationTime);

        // Use frame rate to calculate
        Frame.SetDuration(0); //m_FrameDuration);

        RetVal = Frame.Parse();
        if (GMI_SUCCESS == RetVal)
        {
            EnQueue(Frame);
            NextFrame = NULL;
        }
    }

    m_WorkDone = true;

    PRINT_LOG(VERBOSE, "G711 frame capture stopped");

    return 0;
}

G711Frame * G711FrameCapture::FreeFrame()
{
    if (m_UsingFrameCount < m_MaxFrameCount)
    {
        return &m_G711FrameArray[m_UsingFrameCount];
    }
    else
    {
        GMI_AUTO_LOCK(m_Mutex);
        for (Entry * Node = m_Head.Next(); Node != &m_Head; Node = Node->Next())
        {
            G711Frame * Frame = dynamic_cast<G711Frame *>(Node);
            if (Frame->RefCount() == 0)
            {
                Frame->Detach();
                return Frame;
            }

            PRINT_LOG(WARNING, "Drop one G711 frame");
        }
    }

    return NULL;
}

void_t G711FrameCapture::EnQueue(G711Frame & NewFrame)
{
    if (m_UsingFrameCount < m_MaxFrameCount)
    {
        m_UsingFrameCount ++;
    }
    GMI_AUTO_LOCK(m_Mutex);
    NewFrame.InsertBefore(&m_Head);
}

