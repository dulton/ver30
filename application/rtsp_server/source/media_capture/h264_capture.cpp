#include "h264_capture.h"
#include "configure.h"

// Definition of nalu unit type
#define NALU_TYPE_NONIDR            1
#define NALU_TYPE_IDR               5
#define NALU_TYPE_SEI               6
#define NALU_TYPE_SPS               7
#define NALU_TYPE_PPS               8
#define NALU_TYPE_AUD               9

GMI_RESULT H264Frame::Parse()
{
    uint8_t  * From = Addr();
    uint32_t   Left = Length();
    NaluUnit   Nalu = {NULL, 0};

    if (Left < 5)
    {
        PRINT_LOG(ERROR, "Frame length is less than 5 bytes");
        return GMI_FAIL;
    }

    unsigned Next4Bytes = ((From[0] << 24) | (From[1] << 16) | (From[2] << 8) | From[3]);
    if (Next4Bytes != 0x00000001)
    {
        PRINT_LOG(ERROR, "First 4 bytes is not 0x00000001");
        return GMI_FAIL;
    }

    From += 4;
    Left -= 4;

    while (Left > 0)
    {
        uint8_t NaluType = From[0] & 0x1f;
        Nalu.s_Addr = From;
        Nalu.s_Length = 1;
        
        if (NaluType == NALU_TYPE_NONIDR || NaluType == NALU_TYPE_IDR)
        {
            // We only have one VCL nalu unit
            Nalu.s_Length = Left;
            Left = 0;
        }
        else
        {
            From ++;
            Left --;

            while (Left > 0)
            {
                Next4Bytes = ((From[0] << 24) | (From[1] << 16) | (From[2] << 8) | From[3]);
                if (Next4Bytes == 0x00000001)
                {
                    From += 4;
                    Left -= 4;
                    break;
                }

                uint32_t SkipBytes = 1;
                if (From[3] > 0)
                {
                    SkipBytes = 4;
                }
                else if (From[2] > 0)
                {
                    SkipBytes = 3;
                }
                else if (From[1] > 0)
                {
                    SkipBytes = 2;
                }

                if (SkipBytes > Left)
                {
                    SkipBytes = Left;
                }

                From += SkipBytes;
                Left -= SkipBytes;
                Nalu.s_Length += SkipBytes;
            }

            if (NaluType == NALU_TYPE_SEI)
            {
                // Skip SEI nalu unit
                continue;
            }
            else if (NaluType == NALU_TYPE_SPS)
            {
                m_KeyFrame = true;
            }
        }

        m_NaluUnits[m_TotalNalu ++] = Nalu;
        if (m_TotalNalu >= COUNT_OF(m_NaluUnits))
        {
            PRINT_LOG(WARNING, "Not enough nalu units");
            break;
        }
    }

    return GMI_SUCCESS;
}

H264FrameCapture::H264FrameCapture(const SubStreamInfo & Info, uint32_t MaxFrameCount)
    : MediaFrameCapture(Info)
    , m_MediaDataClient()
    , m_Thread()
    , m_Mutex()
    , m_Working(false)
    , m_WorkDone(true)
    , m_MaxFrameCount(MaxFrameCount)
    , m_UsingFrameCount(0)
    , m_FrameBuf(NULL)
    , m_H264FrameArray(NULL)
    , m_Head()
{
}

H264FrameCapture::~H264FrameCapture()
{
    if (IsCapturing())
    {
        PRINT_LOG(WARNING, "H264 frame capture is still working, shutdown");
        StopCaptureImpl();
    }
}

MediaFrame * H264FrameCapture::GetNextFrame(MediaFrame * PrevFrame)
{
    GMI_AUTO_LOCK(m_Mutex);

    if (m_Head.Prev() == &m_Head || (PrevFrame != NULL && !PrevFrame->IsH264Frame()))
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

void_t H264FrameCapture::RecycleLastFrame(MediaFrame * LastFrame)
{
    if (NULL == LastFrame || !LastFrame->IsH264Frame())
    {
        return;
    }

    GMI_AUTO_LOCK(m_Mutex);
    LastFrame->Release();
}

GMI_RESULT H264FrameCapture::StartCaptureImpl()
{
    if (m_Working)
    {
        return GMI_ALREADY_OPERATED;
    }

    GMI_RESULT RetVal     = GMI_SUCCESS;
    uint16_t   LocalPort  = Configure::GetInstance().GetVideoStreamLocalPort(m_SubStreamInfo.s_StreamId);
    uint16_t   RemotePort = Configure::GetInstance().GetVideoStreamRemotePort(m_SubStreamInfo.s_StreamId);
    uint32_t   MaxFrameSize = m_SubStreamInfo.s_MaxBitRate / BITS_PER_BYTE;
    MaxFrameSize += (m_SubStreamInfo.s_MaxBitRate % BITS_PER_BYTE) > 0 ? 1 : 0;

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

        m_H264FrameArray = new H264Frame [m_MaxFrameCount];
        if (NULL == m_H264FrameArray)
        {
            PRINT_LOG(ERROR, "Not enough memory");
            RetVal = GMI_OUT_OF_MEMORY;
            break;
        }

        for (uint32_t i = 0; i < m_MaxFrameCount; ++ i)
        {
            m_H264FrameArray[i].Assign(m_FrameBuf + (MaxFrameSize * i), MaxFrameSize);
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

        RetVal = m_Mutex.Create("H264FrameCapture");
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to create mutex");
            break;
        }
#ifndef USE_OSAL_THREAD
        if (pthread_create(&m_Thread, NULL, H264FrameCapture::ThreadEntry, static_cast<void_t *>(this)) != 0)
        {
            PRINT_LOG(ERROR, "Failed to create thread");
            RetVal = GMI_FAIL;
            break;
        }
#else
        RetVal = m_Thread.Create("H264FrameCapture", 0, H264FrameCapture::ThreadEntry, static_cast<void_t *>(this));
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

    if (m_H264FrameArray)
    {
        delete [] m_H264FrameArray;
        m_H264FrameArray = NULL;
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

GMI_RESULT H264FrameCapture::StopCaptureImpl()
{
    if (m_WorkDone)
    {
        return GMI_ALREADY_OPERATED;
    }

    PRINT_LOG(VERBOSE, "Try to stop H264 frame capture ...");

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
    delete [] m_H264FrameArray;
    m_H264FrameArray = NULL;
    delete [] m_FrameBuf;
    m_FrameBuf = NULL;

    m_UsingFrameCount = 0;

    return GMI_SUCCESS;
}

void_t * H264FrameCapture::ThreadEntry(void_t * Data)
{
    ASSERT(Data != NULL, "Data MUST NOT be non-pointer");

    H264FrameCapture * This = static_cast<H264FrameCapture *>(Data);
    return reinterpret_cast<void_t *>(This->ThreadEntryImpl());
}

uint32_t H264FrameCapture::ThreadEntryImpl()
{
    PRINT_LOG(VERBOSE, "Start to capture H264 frames");
	struct timeval   TimeStamp = {0, 0};
    H264Frame      * NextFrame = NULL;
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

        H264Frame & Frame = *NextFrame;

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

        if (0 == m_UsingFrameCount)
        {
		    gettimeofday(&TimeStamp, NULL);
        }
        else
        {
            TimeStamp.tv_usec += m_FrameDuration;
            if (TimeStamp.tv_usec > 1000000)
            {
                TimeStamp.tv_usec -= 1000000;
                TimeStamp.tv_sec ++;
            }
        }

        presentationTime = TimeStamp;

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

    PRINT_LOG(VERBOSE, "H264 frame capture stopped");

    return 0;
}

H264Frame * H264FrameCapture::FreeFrame()
{
    if (m_UsingFrameCount < m_MaxFrameCount)
    {
        return &m_H264FrameArray[m_UsingFrameCount];
    }
    else
    {
        GMI_AUTO_LOCK(m_Mutex);
        for (Entry * Node = m_Head.Next(); Node != &m_Head; Node = Node->Next())
        {
            H264Frame * Frame = dynamic_cast<H264Frame *>(Node);
            if (Frame->RefCount() == 0)
            {
                Frame->Detach();
                return Frame;
            }

            PRINT_LOG(WARNING, "Drop one H264 frame");
        }
    }

    return NULL;
}

void_t H264FrameCapture::EnQueue(H264Frame & NewFrame)
{
    if (m_UsingFrameCount < m_MaxFrameCount)
    {
        m_UsingFrameCount ++;
    }
    GMI_AUTO_LOCK(m_Mutex);
    NewFrame.InsertBefore(&m_Head);
}

