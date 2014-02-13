
#include "log.h"
#include "rtsp_stream_control.h"

#define COUNT_OF(array) (sizeof(array) / sizeof(array[0]))

static const SubStreamInfo StreamInfo1[] =
{
    {MEDIA_VIDEO_H264,  0, 25, 4000000},
    {MEDIA_AUDIO_G711A, 0, 20, 64000  },
};

static const SubStreamInfo StreamInfo2[] =
{
    {MEDIA_VIDEO_H264,  1, 25, 2000000},
    {MEDIA_AUDIO_G711A, 0, 20, 64000  },
};

typedef struct tagStreamInfo
{
    uint32_t              s_SubStreamNum;
    SubStreamInfo         *s_SubStreamInfo;
} StreamInfo;

static StreamInfo StreamTable[16];


RtspStreamControl::RtspStreamControl()
{
}

RtspStreamControl::~RtspStreamControl()
{
}


GMI_RESULT RtspStreamControl::Initialize()
{
    GMI_RESULT Result = m_Mutex.Create(NULL);
    if (FAILED(Result))
    {
        SYS_ERROR("m_Mutex.Create error\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_Mutex.Create error\n");
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT RtspStreamControl::Deinitialize()
{
    m_Mutex.Destroy();
    return GMI_SUCCESS;
}


GMI_RESULT RtspStreamControl::Start(SysPkgEncodeCfg *SysEncodeCfg, int32_t VideoCount, SysPkgAudioEncodeCfg *SysAudioCfg, int32_t AudioCount, int32_t Timeout)
{
    SYS_INFO("%s in.........\n", __func__);
    if (NULL == SysEncodeCfg
            || VideoCount < 0
            || NULL == SysAudioCfg
            || AudioCount < 0)
    {
        return GMI_INVALID_PARAMETER;
    }

    Lock();
    ReferrencePtr<SysPkgEncodeCfg, DefaultObjectsDeleter>SysVideoEncodeCfgPtr(BaseMemoryManager::Instance().News<SysPkgEncodeCfg>(VideoCount));
    if (NULL == SysVideoEncodeCfgPtr.GetPtr())
    {
        Unlock();
        SYS_ERROR("SysVideoEncodeCfgPtr news fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysVideoEncodeCfgPtr news fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    memset(SysVideoEncodeCfgPtr.GetPtr(), 0, sizeof(SysPkgEncodeCfg)*VideoCount);
    memcpy(SysVideoEncodeCfgPtr.GetPtr(), SysEncodeCfg, sizeof(SysPkgEncodeCfg)*VideoCount);

    ReferrencePtr<SysPkgAudioEncodeCfg, DefaultObjectsDeleter>SysAudioEncodeCfgPtr(BaseMemoryManager::Instance().News<SysPkgAudioEncodeCfg>(AudioCount));
    if (NULL == SysVideoEncodeCfgPtr.GetPtr())
    {
        Unlock();
        SYS_ERROR("SysVideoEncodeCfgPtr news fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysVideoEncodeCfgPtr news fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    memset(SysAudioEncodeCfgPtr.GetPtr(), 0, sizeof(SysPkgAudioEncodeCfg)*AudioCount);
    memcpy(SysAudioEncodeCfgPtr.GetPtr(), SysAudioCfg, sizeof(SysPkgAudioEncodeCfg)*AudioCount);


    //StreamTable
    for (int32_t i = 0; i < VideoCount; i++)
    {
        //audio and video
        if (SYS_AUDIO_VIDEO_STREAM == SysVideoEncodeCfgPtr.GetPtr()[i].s_StreamType)
        {
            StreamTable[i].s_SubStreamNum  = 2;
            StreamTable[i].s_SubStreamInfo = BaseMemoryManager::Instance().News<SubStreamInfo>(StreamTable[i].s_SubStreamNum);
            StreamTable[i].s_SubStreamInfo[0].s_EncodeType = MEDIA_VIDEO_H264;
            StreamTable[i].s_SubStreamInfo[0].s_StreamId   = i;
            StreamTable[i].s_SubStreamInfo[0].s_FrameRate  = SysVideoEncodeCfgPtr.GetPtr()[i].s_FPS;
            if (SYS_BRC_CBR == SysVideoEncodeCfgPtr.GetPtr()->s_BitrateCtrl)
            {
                StreamTable[i].s_SubStreamInfo[0].s_MaxBitRate = SysVideoEncodeCfgPtr.GetPtr()[i].s_BitRateAverage*1024;
            }
            else
            {
                StreamTable[i].s_SubStreamInfo[0].s_MaxBitRate = SysVideoEncodeCfgPtr.GetPtr()[i].s_BitRateUp*1024;
            }
            StreamTable[i].s_SubStreamInfo[1].s_EncodeType = MEDIA_AUDIO_G711A;
            StreamTable[i].s_SubStreamInfo[1].s_StreamId   = 0;
            StreamTable[i].s_SubStreamInfo[1].s_FrameRate  = SysAudioEncodeCfgPtr.GetPtr()->s_FrameRate;
            StreamTable[i].s_SubStreamInfo[1].s_MaxBitRate = SysAudioEncodeCfgPtr.GetPtr()->s_BitRate*1024;
        }
        else //just video
        {
            StreamTable[i].s_SubStreamNum = 1;
            StreamTable[i].s_SubStreamInfo = BaseMemoryManager::Instance().News<SubStreamInfo>(StreamTable[i].s_SubStreamNum);
            StreamTable[i].s_SubStreamInfo[0].s_EncodeType = MEDIA_VIDEO_H264;
            StreamTable[i].s_SubStreamInfo[0].s_StreamId   = i;
            StreamTable[i].s_SubStreamInfo[0].s_FrameRate  = SysVideoEncodeCfgPtr.GetPtr()[i].s_FPS;
            if (SYS_BRC_CBR == SysVideoEncodeCfgPtr.GetPtr()[i].s_BitrateCtrl)
            {
                StreamTable[i].s_SubStreamInfo[0].s_MaxBitRate = SysVideoEncodeCfgPtr.GetPtr()[i].s_BitRateAverage*1024;
            }
            else
            {
                StreamTable[i].s_SubStreamInfo[0].s_MaxBitRate = SysVideoEncodeCfgPtr.GetPtr()[i].s_BitRateUp*1024;
            }
        }
    }

    for (int32_t i = 0; i < MAX_VIDEO_STREAM_NUM; i++)
    {
        GMI_RESULT Result = RtspCloseStream(i+1);
        if (FAILED(Result))
        {
            for (int32_t i = 0; i < VideoCount; i++)
            {
                BaseMemoryManager::Instance().Deletes<SubStreamInfo>(StreamTable[i].s_SubStreamInfo);
            }
            Unlock();
            SYS_ERROR("RtspCloseStream fail, Result = 0x%lx\n", Result);
            return Result;
        }
    }

    for (int32_t i = 0; i < VideoCount; i++)
    {
        GMI_RESULT Result = RtspOpenStream(i+1, StreamTable[i].s_SubStreamNum, StreamTable[i].s_SubStreamInfo);
        if (FAILED(Result))
        {
            for (int32_t i = 0; i < VideoCount; i++)
            {
                BaseMemoryManager::Instance().Deletes<SubStreamInfo>(StreamTable[i].s_SubStreamInfo);
            }
            Unlock();
            SYS_ERROR("RtspOpenStream fail, Result = 0x%lx\n", Result);
            return Result;
        }
    }
    m_VideoCount = VideoCount;
    Unlock();

    //release resource
    for (int32_t i = 0; i < VideoCount; i++)
    {
        BaseMemoryManager::Instance().Deletes<SubStreamInfo>(StreamTable[i].s_SubStreamInfo);
    }
    SysVideoEncodeCfgPtr = NULL;
    SysAudioEncodeCfgPtr = NULL;
    SYS_INFO("%s normal out.........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT RtspStreamControl::Stop(int32_t Timeout)
{
    Lock();
    for (int32_t i = 0; i < MAX_VIDEO_STREAM_NUM; i++)
    {
        GMI_RESULT Result = RtspCloseStream(i+1);
        if (FAILED(Result))
        {
            SYS_ERROR("RtspCloseStream%d fail, Result = 0x%lx\n", i+1, Result);
            Unlock();
            return Result;
        }
    }
    Unlock();
    return GMI_SUCCESS;
}


GMI_RESULT RtspStreamControl::Query(int32_t Timeout, int32_t VideoCount, int32_t *Started)
{
    uint32_t StreamStatus;

    Lock();
    for (int32_t i = 0; i < VideoCount; i++)
    {
        GMI_RESULT Result = RtspQueryStreamStatus(i+1, &StreamStatus);
        if (FAILED(Result))
        {
            Unlock();
            return Result;
        }

        if (eStreamClosed == StreamStatus)
        {
            *Started = RTSP_STREAM_STATE_STOP;
            Unlock();
            return GMI_SUCCESS;
        }
    }
    *Started = RTSP_STREAM_STATE_START;
    Unlock();
    return GMI_SUCCESS;
}


GMI_RESULT RtspStreamControl::Lock()
{
    m_Mutex.Lock(TIMEOUT_INFINITE);
    return GMI_SUCCESS;
}


GMI_RESULT RtspStreamControl::Unlock()
{
    m_Mutex.Unlock();
    return GMI_SUCCESS;
}


