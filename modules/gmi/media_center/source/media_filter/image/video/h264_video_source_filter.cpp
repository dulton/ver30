#include "h264_video_source_filter.h"

#include "gmi_config_api.h"
#include "media_codec_parameter.h"
#include "ipc_fw_v3.x_setting.h"
#include "share_memory_log_client.h"
#include "timer_task_queue.h"

H264_VideoSourceFilter::H264_VideoSourceFilter(void)
    : m_HardwareSource( NULL )
#if SAVE_H264_FILE
    , m_VideoRecordFile( NULL )
#endif
    , m_VideoMonitorEnable( GMI_H264_VIDEO_MONITOR_CONFIG_ENABLE_VALUE )
    , m_VideoFrameCheckInterval( GMI_H264_VIDEO_MONITOR_CONFIG_FRAME_CHECK_INTERVAL_VALUE )
    , m_FrameNumber( 0 )
    , m_IFrameNumber( 0 )
#if MONITOR_H264_VIDEO_FRAME_INTERVAL
    , m_TotalFrameInterval( 0 )
    , m_MaxFrameInterval( 0 )
    , m_MinFrameInterval( 1000000 )
#endif
#if USE_SIMULATOR_VIDEO_SOUCRE
    , m_SimulatedDataSourceThread()
    , m_ThreadWorking( false )
    , m_ThreadExitFlag( false )
    , m_VideoSourceFile( NULL )
    , m_VideoSourceFileLength( 0 )
    , m_VideoData( NULL )
    , m_FrameRate( 0.0f )
#endif
{
    m_FirstFrameTime.tv_sec  = 0;
    m_FirstFrameTime.tv_usec = 0;
#if MONITOR_H264_VIDEO_FRAME_INTERVAL
    m_LastFrameTimeStamp.tv_sec  = 0;
    m_LastFrameTimeStamp.tv_usec = 0;
#endif
}

H264_VideoSourceFilter::~H264_VideoSourceFilter(void)
{
}

H264_VideoSourceFilter*  H264_VideoSourceFilter::CreateNew()
{
    return BaseMemoryManager::Instance().New<H264_VideoSourceFilter>();
}

GMI_RESULT	H264_VideoSourceFilter::Initialize( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::Initialize begin \n" );

    GMI_RESULT Result = GetVideoMonitorEnableConfig( &m_VideoMonitorEnable );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Initialize, GetVideoMonitorEnableConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    if ( m_VideoMonitorEnable )
    {
        Result = GetVideoFrameCheckInterval( &m_VideoFrameCheckInterval );
        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Initialize, GetVideoFrameCheckInterval failed, function return %x \n", (uint32_t) Result );
            return Result;
        }
    }

    MediaSourceParameter *SourceParameter = reinterpret_cast<MediaSourceParameter *> (Argument);
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::Initialize, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d\n", SourceParameter->s_SourceId, SourceParameter->s_MediaId, SourceParameter->s_MediaType, SourceParameter->s_CodecType );
    if ( CODEC_H264 != SourceParameter->s_CodecType )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Initialize, EncodeParameter->CodecType=%d, not CODEC_H264(%d), function return %x \n", SourceParameter->s_CodecType, CODEC_H264, GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

    Result = VideoSourceFilter::Initialize( FilterId, FilterName, FilterNameLength, Argument, ArgumentSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Initialize, VideoSourceFilter::Initialize return %x \n", (uint32_t) Result );
        return Result;
    }

    VideoEncodeParam *EncParam = (VideoEncodeParam *) SourceParameter->s_InternalParamter;
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::Initialize, EncodeType=%d,FrameRate=%d,EncodeWidth=%d,EncodeHeight=%d,BitRateType=%d,BitRateAverage=%d,BitRateUp=%d,BitRateDown=%d,FrameInterval=%d,EncodeQulity=%d,Rotate=%d\n",
               EncParam->s_EncodeType, EncParam->s_FrameRate, EncParam->s_EncodeWidth, EncParam->s_EncodeHeight, EncParam->s_BitRateType, EncParam->s_BitRateAverage, EncParam->s_BitRateUp, EncParam->s_BitRateDown, EncParam->s_FrameInterval, EncParam->s_EncodeQulity, EncParam->s_Rotate );

    m_HardwareSource = GMI_VideoEncCreate( SourceParameter->s_SourceId, SourceParameter->s_MediaId, (VideoEncodeParam *) SourceParameter->s_InternalParamter );
    if ( NULL == m_HardwareSource )
    {
        VideoSourceFilter::Deinitialize();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Initialize, GMI_VideoEncCreate failed, function return %x \n", (uint32_t) GMI_OPEN_DEVICE_FAIL );
        return GMI_OPEN_DEVICE_FAIL;
    }

    Result = GMI_VideoEncSetCB( m_HardwareSource, MediaEncCallBack, this );
    if ( FAILED( Result ) )
    {
        VideoSourceFilter::Deinitialize();
        GMI_VideoEncDestroy( m_HardwareSource );
        m_HardwareSource = NULL;
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Initialize, GMI_VideoEncSetCB failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

#if USE_SIMULATOR_VIDEO_SOUCRE
    m_FrameRate = EncParam->s_FrameRate;
#endif

#if SAVE_H264_FILE
    char_t FileName[MAX_PATH_LENGTH];
#if defined( __linux__ )
    //sprintf( FileName, "/mnt/nfs/%p_H264.264", this );
    sprintf( FileName, "%p_H264.264", this );
#elif defined( _WIN32 )
    sprintf_s( FileName, MAX_PATH_LENGTH, "%p_H264.264", this );
#endif
    printf( "File Name : %s\n", FileName );
#if defined( __linux__ )
    m_VideoRecordFile = fopen( FileName, "wb" );
#elif defined( _WIN32 )
    fopen_s( &m_VideoRecordFile, FileName, "wb" );
#endif
#endif

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::Initialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT  H264_VideoSourceFilter::Deinitialize()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::Deinitialize begin \n" );
#if SAVE_H264_FILE
    if ( NULL != m_VideoRecordFile )
    {
        fclose( m_VideoRecordFile );
        m_VideoRecordFile = NULL;
    }
#endif

    GMI_RESULT Result = VideoSourceFilter::Deinitialize();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Deinitialize, VideoSourceFilter::Deinitialize failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    if ( NULL != m_HardwareSource )
    {
        GMI_VideoEncDestroy( m_HardwareSource );
        m_HardwareSource = NULL;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::Deinitialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT  H264_VideoSourceFilter::Play()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::Play begin \n" );
    if ( NULL == m_HardwareSource )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Play, video encode device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = VideoSourceFilter::Play();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Play, VideoSourceFilter::Play failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

#if USE_SIMULATOR_VIDEO_SOUCRE

    printf( " test code, we will open file: media_video.h264 \n" );
    m_VideoSourceFile = fopen( "media_video.h264", "r" );
    if ( NULL == m_VideoSourceFile )
    {
        VideoSourceFilter::Stop();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Play, fopen failed, errno=%d, function return %x \n", errno, (uint32_t) GMI_OPEN_DEVICE_FAIL );
        return GMI_OPEN_DEVICE_FAIL;
    }

    fseek(m_VideoSourceFile, 0, SEEK_END);
    m_VideoSourceFileLength = ftell(m_VideoSourceFile);
    m_VideoData = BaseMemoryManager::Instance().News<uint8_t>(m_VideoSourceFileLength);
    if ( NULL == m_VideoData )
    {
        VideoSourceFilter::Stop();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Play, allocating memory failed, function return %x \n", errno, (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }
    fseek(m_VideoSourceFile, 0, SEEK_SET);

    m_ThreadWorking = false;
    m_ThreadExitFlag = false;

    Result = m_SimulatedDataSourceThread.Create( NULL, 0, H264_VideoSourceFilter::SimulatedDataSourceThread, this );
    if ( FAILED( Result ) )
    {
        VideoSourceFilter::Stop();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Play, SimulatedDataSourceThread.Create failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_SimulatedDataSourceThread.Start();
    if ( FAILED( Result ) )
    {
        m_SimulatedDataSourceThread.Destroy();
        StreamingMediaSource::Stop();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Play, SimulatedDataSourceThread.Start failed, function return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    Result = GMI_VideoEncStart( m_HardwareSource );
    if ( FAILED( Result ) )
    {
        VideoSourceFilter::Stop();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Play, GMI_VideoEncStart failed, function return %x \n", (uint32_t) Result );
        return Result;
    }
#endif

    gettimeofday1( &m_FirstFrameTime, NULL );

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::Play end \n" );
    return Result;
}

GMI_RESULT  H264_VideoSourceFilter::Stop()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::Stop begin \n" );
    if ( NULL == m_HardwareSource )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Stop, video encode device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = VideoSourceFilter::Stop();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Play, VideoSourceFilter::Stop failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

#if USE_SIMULATOR_VIDEO_SOUCRE
    m_ThreadExitFlag = true;
    while ( m_ThreadWorking ) GMI_Sleep( 10 );
    m_SimulatedDataSourceThread.Destroy();

    BaseMemoryManager::Instance().Deletes( m_VideoData );
    m_VideoData = NULL;

    fclose( m_VideoSourceFile );
    m_VideoSourceFile = NULL;
#else
    Result = GMI_VideoEncStop( m_HardwareSource );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::Play, GMI_VideoEncStop failed, function return %x \n", (uint32_t) Result );
        return Result;
    }
#endif

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::Stop end \n" );
    return Result;
}

GMI_RESULT H264_VideoSourceFilter::GetEncodeConfig( void_t *EncodeParameter, size_t *EncodeParameterLength )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetEncodeConfig begin, passed EncodeParameterLength=%d \n", *EncodeParameterLength );
    if ( NULL == m_HardwareSource )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::GetEncodeConfig, video encode device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(VideoEncodeParam) > *EncodeParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::GetEncodeConfig, provided buffer space is not enough, function return %x \n", (uint32_t) GMI_NOT_ENOUGH_SPACE );
        return GMI_NOT_ENOUGH_SPACE;
    }

    GMI_RESULT Result = GMI_VideoEncGetConfig( m_HardwareSource, (VideoEncodeParam*)EncodeParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::GetEncodeConfig, GMI_VideoEncGetConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetEncodeConfig, StreamId       = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_StreamId );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetEncodeConfig, EncodeType     = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_EncodeType );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetEncodeConfig, FrameRate      = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_FrameRate );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetEncodeConfig, EncodeWidth    = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_EncodeWidth );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetEncodeConfig, EncodeHeight   = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_EncodeHeight );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetEncodeConfig, BitRateType    = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_BitRateType );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetEncodeConfig, BitRateAverage = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_BitRateAverage );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetEncodeConfig, BitRateUp      = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_BitRateUp );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetEncodeConfig, BitRateDown    = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_BitRateDown );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetEncodeConfig, FrameInterval  = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_FrameInterval );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetEncodeConfig, EncodeQulity   = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_EncodeQulity );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetEncodeConfig, Rotate         = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_Rotate );
#endif

    *EncodeParameterLength = sizeof(VideoEncodeParam);
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetEncodeConfig end, returned EncodeParameterLength=%d \n", *EncodeParameterLength );
    return GMI_SUCCESS;
}

GMI_RESULT H264_VideoSourceFilter::SetEncodeConfig( const void_t *EncodeParameter, size_t EncodeParameterLength )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetEncodeConfig begin, passed EncodeParameterLength=%d \n", EncodeParameterLength );
    if ( NULL == m_HardwareSource )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::SetEncodeConfig, video encode device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(VideoEncodeParam) > EncodeParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::GetEncodeConfig, provided buffer space is not enough, function return %x \n", (uint32_t) GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetEncodeConfig, StreamId       = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_StreamId );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetEncodeConfig, EncodeType     = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_EncodeType );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetEncodeConfig, FrameRate      = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_FrameRate );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetEncodeConfig, EncodeWidth    = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_EncodeWidth );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetEncodeConfig, EncodeHeight   = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_EncodeHeight );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetEncodeConfig, BitRateType    = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_BitRateType );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetEncodeConfig, BitRateAverage = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_BitRateAverage );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetEncodeConfig, BitRateUp      = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_BitRateUp );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetEncodeConfig, BitRateDown    = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_BitRateDown );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetEncodeConfig, FrameInterval  = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_FrameInterval );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetEncodeConfig, EncodeQulity   = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_EncodeQulity );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetEncodeConfig, Rotate         = %d\n", ((VideoEncodeParam*)EncodeParameter)->s_Rotate );
#endif

    GMI_RESULT Result = GMI_VideoEncSetConfig( m_HardwareSource, (VideoEncodeParam*)EncodeParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::SetEncodeConfig, GMI_VideoEncSetConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetEncodeConfig end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT H264_VideoSourceFilter::GetOsdConfig( void_t *OsdParameter, size_t *OsdParameterLength )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig begin, passed OsdParameterLength=%d \n", *OsdParameterLength );
    if ( NULL == m_HardwareSource )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::GetOsdConfig, video encode device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(VideoOSDParam) > *OsdParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::GetOsdConfig, provided buffer space is not enough, function return %x \n", (uint32_t) GMI_NOT_ENOUGH_SPACE );
        return GMI_NOT_ENOUGH_SPACE;
    }

    GMI_RESULT Result = GMI_VideoOsdGetConfig( m_HardwareSource, (VideoOSDParam*)OsdParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::GetOsdConfig, GMI_VideoOsdGetConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, StreamId     = %d\n", ((VideoOSDParam*)OsdParameter)->s_StreamId );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, DisplayType  = %d\n", ((VideoOSDParam*)OsdParameter)->s_DisplayType );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, Flag         = %d\n", ((VideoOSDParam*)OsdParameter)->s_Flag );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, Language     = %d\n", ((VideoOSDParam*)OsdParameter)->s_Language );

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TimeDisplay.Flag       = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_Flag );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TimeDisplay.DateStyle  = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_DateStyle );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TimeDisplay.TimeStyle  = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_TimeStyle );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TimeDisplay.FontColor  = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_FontColor );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TimeDisplay.FontStyle  = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_FontStyle );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TimeDisplay.FontBlod   = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_FontBlod );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TimeDisplay.Rotate     = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_Rotate );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TimeDisplay.Italic     = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_Italic );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TimeDisplay.Outline    = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_Outline );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TimeDisplay.DisplayX   = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_DisplayX );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TimeDisplay.DisplayY   = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_DisplayY );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TimeDisplay.DisplayH   = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_DisplayH );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TimeDisplay.DisplayW   = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_DisplayW );

    for ( int32_t i = 0; i < OSD_TEXT_NUM; ++i )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TextDisplay[%d].Flag           = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_Flag );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TextDisplay[%d].FontColor      = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_FontColor );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TextDisplay[%d].FontStyle      = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_FontStyle );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TextDisplay[%d].FontBlod       = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_FontBlod );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TextDisplay[%d].Rotate         = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_Rotate );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TextDisplay[%d].Italic         = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_Italic );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TextDisplay[%d].Outline        = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_Outline );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TextDisplay[%d].TextContentLen = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_TextContentLen );
        if ( 0 < ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_TextContentLen )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TextDisplay[%d].TextContent= %s\n", i, (char_t *)((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_TextContent );
        }
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TextDisplay[%d].DisplayX       = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_DisplayX );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TextDisplay[%d].DisplayY       = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_DisplayY );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TextDisplay[%d].DisplayH       = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_DisplayH );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig, TextDisplay[%d].DisplayW       = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_DisplayW );
    }
#endif

    *OsdParameterLength = sizeof(VideoOSDParam);
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetOsdConfig end, returned OsdParameterLength=%d \n", *OsdParameterLength );
    return GMI_SUCCESS;
}

GMI_RESULT H264_VideoSourceFilter::SetOsdConfig( const void_t *OsdParameter, size_t OsdParameterLength )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig begin, passed OsdParameterLength=%d \n", OsdParameterLength );
    if ( NULL == m_HardwareSource )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::SetOsdConfig, video encode device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(VideoOSDParam) > OsdParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::SetOsdConfig, provided buffer space is not enough, function return %x \n", (uint32_t) GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, StreamId     = %d\n", ((VideoOSDParam*)OsdParameter)->s_StreamId );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, DisplayType  = %d\n", ((VideoOSDParam*)OsdParameter)->s_DisplayType );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, Flag         = %d\n", ((VideoOSDParam*)OsdParameter)->s_Flag );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, Language     = %d\n", ((VideoOSDParam*)OsdParameter)->s_Language );

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TimeDisplay.Flag       = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_Flag );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TimeDisplay.DateStyle  = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_DateStyle );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TimeDisplay.TimeStyle  = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_TimeStyle );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TimeDisplay.FontColor  = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_FontColor );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TimeDisplay.FontStyle  = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_FontStyle );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TimeDisplay.FontBlod   = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_FontBlod );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TimeDisplay.Rotate     = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_Rotate );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TimeDisplay.Italic     = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_Italic );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TimeDisplay.Outline    = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_Outline );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TimeDisplay.DisplayX   = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_DisplayX );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TimeDisplay.DisplayY   = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_DisplayY );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TimeDisplay.DisplayH   = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_DisplayH );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TimeDisplay.DisplayW   = %d\n", ((VideoOSDParam*)OsdParameter)->s_TimeDisplay.s_DisplayW );

    for ( int32_t i = 0; i < OSD_TEXT_NUM; ++i )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TextDisplay[%d].Flag           = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_Flag );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TextDisplay[%d].FontColor      = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_FontColor );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TextDisplay[%d].FontStyle      = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_FontStyle );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TextDisplay[%d].FontBlod       = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_FontBlod );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TextDisplay[%d].Rotate         = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_Rotate );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TextDisplay[%d].Italic         = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_Italic );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TextDisplay[%d].Outline        = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_Outline );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TextDisplay[%d].TextContentLen = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_TextContentLen );
        if ( 0 < ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_TextContentLen )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig end, TextDisplay[%d].TextContent= %s\n", i, (char_t *)((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_TextContent );
        }
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TextDisplay[%d].DisplayX       = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_DisplayX );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TextDisplay[%d].DisplayY       = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_DisplayY );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TextDisplay[%d].DisplayH       = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_DisplayH );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig, TextDisplay[%d].DisplayW       = %d\n", i, ((VideoOSDParam*)OsdParameter)->s_TextDisplay[i].s_DisplayW );
    }
#endif

    GMI_RESULT Result = GMI_VideoOsdSetConfig( m_HardwareSource, (VideoOSDParam*)OsdParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::SetOsdConfig, GMI_VideoOsdSetConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SetOsdConfig end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT H264_VideoSourceFilter::ForceGenerateIdrFrame()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::ForceGenerateIdrFrame begin \n" );
    if ( NULL == m_HardwareSource )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::ForceGenerateIdrFrame, video encode device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_ForceSetIdrFrame( m_HardwareSource );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::ForceGenerateIdrFrame, GMI_ForceSetIdrFrame failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::ForceGenerateIdrFrame end \n" );
    return GMI_SUCCESS;
}

void_t H264_VideoSourceFilter::MediaEncCallBack( void_t *UserDataPtr, MediaEncInfo *EncInfo, ExtMediaEncInfo *ExtEncInfo )
{
    H264_VideoSourceFilter *VideoSourceFilter = (H264_VideoSourceFilter *) UserDataPtr;

#if SAVE_H264_FILE
    if ( NULL != VideoSourceFilter->m_VideoRecordFile )
    {
        fwrite( EncInfo->s_StreamAddr, 1, EncInfo->s_StreamSize, VideoSourceFilter->m_VideoRecordFile );
    }
#endif

    size_t ExtensionSize = 0;
    if ( NULL != ExtEncInfo )
    {
        if ( VIDEO_IDR_FRAME == ExtEncInfo->s_FrameType )
        {
            ++VideoSourceFilter->m_IFrameNumber;
            //DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Loop, "H264_VideoSourceFilter::MediaEncCallBack, saw I frame, Media=%d, Port=%d, MediaId=%d, StreamSize=%d, FrameType=%d, FrameNum=%d, second=%d:millisecond=%d, ExtraDataLength=%d \n",
            //           EncInfo->s_Media, EncInfo->s_Port, EncInfo->s_MediaId, EncInfo->s_StreamSize, ExtEncInfo->s_FrameType, ExtEncInfo->s_FrameNum, (int32_t) EncInfo->s_PTS.tv_sec, (int32_t) EncInfo->s_PTS.tv_usec, ExtEncInfo->s_Length );
        }

        ExtensionSize = sizeof(uint32_t)+sizeof(uint32_t)+sizeof(size_t)+ExtEncInfo->s_Length;
    }

    ++VideoSourceFilter->m_FrameNumber;
    if ( VideoSourceFilter->m_VideoMonitorEnable )
    {
        // the following code facilitate unitest check execution, uncomment it to enable print info
        //printf( "H264_VideoSourceFilter::MediaEncCallBack, FrameNumber=%lld, VideoFrameCheckInterval=%d \n", VideoSourceFilter->m_FrameNumber, VideoSourceFilter->m_VideoFrameCheckInterval );

        if ( 0 == VideoSourceFilter->m_FrameNumber%VideoSourceFilter->m_VideoFrameCheckInterval )
        {
            struct timeval CurrentTime;
            gettimeofday1( &CurrentTime, NULL );

            uint64_t Duration = (CurrentTime.tv_sec - VideoSourceFilter->m_FirstFrameTime.tv_sec)*1000 + CurrentTime.tv_usec/1000 - VideoSourceFilter->m_FirstFrameTime.tv_usec/1000;

            double FrameRate = 1000.0f * VideoSourceFilter->m_FrameNumber / Duration;
            double IFrameRate = 1000.0f * VideoSourceFilter->m_IFrameNumber / Duration;

            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Loop, "H264_VideoSourceFilter::MediaEncCallBack, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d, FrameNumber=%lld, IFrameNumber=%lld, FrameRate=%f, IFrameRate=%f, second=%d:millisecond=%d \n",
                       VideoSourceFilter->GetSourceId(), VideoSourceFilter->GetMediaId(), VideoSourceFilter->GetMediaType(), VideoSourceFilter->GetCodecType(),
                       VideoSourceFilter->m_FrameNumber, VideoSourceFilter->m_IFrameNumber, FrameRate, IFrameRate, (int32_t) EncInfo->s_PTS.tv_sec, (int32_t) EncInfo->s_PTS.tv_usec );
            // the following code facilitate unitest check execution, uncomment it to enable print info
            //printf( "H264_VideoSourceFilter::MediaEncCallBack, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d, FrameNumber=%lld, IFrameNumber=%lld, FrameRate=%f, IFrameRate=%f, second=%d:millisecond=%d \n",
            //           VideoSourceFilter->GetSourceId(), VideoSourceFilter->GetMediaId(), VideoSourceFilter->GetMediaType(), VideoSourceFilter->GetCodecType(),
            //		   VideoSourceFilter->m_FrameNumber, VideoSourceFilter->m_IFrameNumber, FrameRate, IFrameRate, (int32_t) EncInfo->s_PTS.tv_sec, (int32_t) EncInfo->s_PTS.tv_usec );
        }
    }

#if MONITOR_H264_VIDEO_FRAME_INTERVAL
    if ( 0 == VideoSourceFilter->m_LastFrameTimeStamp.tv_sec && 0 == VideoSourceFilter->m_LastFrameTimeStamp.tv_usec )
    {
        VideoSourceFilter->m_LastFrameTimeStamp = EncInfo->s_PTS;
    }

    uint64_t Interval = ((uint64_t)EncInfo->s_PTS.tv_sec - (uint64_t)VideoSourceFilter->m_LastFrameTimeStamp.tv_sec)*1000000 + (uint64_t)EncInfo->s_PTS.tv_usec - (uint64_t)VideoSourceFilter->m_LastFrameTimeStamp.tv_usec;
    if ( Interval > VideoSourceFilter->m_MaxFrameInterval )
    {
        VideoSourceFilter->m_MaxFrameInterval = Interval;
    }
    else if ( Interval < VideoSourceFilter->m_MinFrameInterval && 0 != Interval )
    {
        VideoSourceFilter->m_MinFrameInterval = Interval;
    }
    VideoSourceFilter->m_TotalFrameInterval += Interval;
    VideoSourceFilter->m_LastFrameTimeStamp = EncInfo->s_PTS;

    if ( 0 == VideoSourceFilter->m_FrameNumber%MONITOR_H264_VIDEO_FRAME_INTERVAL_NUMBER )
    {
        uint64_t AverageFrameInterval = VideoSourceFilter->m_TotalFrameInterval/(VideoSourceFilter->m_FrameNumber-1);
        printf( "SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d, FrameCount=%llu, TotalFrameInterval=%llu, Interval=%lld, MaxFrameInterval=%llu, MinFrameInterval=%llu, AverageFrameInterval=%llu, (uint:us) \n",
                VideoSourceFilter->GetSourceId(), VideoSourceFilter->GetMediaId(), VideoSourceFilter->GetMediaType(), VideoSourceFilter->GetCodecType(),
                VideoSourceFilter->m_FrameNumber, VideoSourceFilter->m_TotalFrameInterval, Interval, VideoSourceFilter->m_MaxFrameInterval, VideoSourceFilter->m_MinFrameInterval, AverageFrameInterval );
    }
#endif

#if 1 // we can swith off the following code when testing video frame interval
    // notify base class of new data coming
    GMI_RESULT Result = VideoSourceFilter->Receive( 0, EncInfo->s_StreamAddr, EncInfo->s_StreamSize, &EncInfo->s_PTS, ExtEncInfo, ExtensionSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::MediaEncCallBack, VideoSourceFilter::Receive failed and return %x , Media=%d, Port=%d, MediaId=%d, StreamSize=%d, FrameType=%d, FrameNum=%d, second=%d:millisecond=%d, ExtraDataLength=%d\n",
                   (uint32_t)Result, EncInfo->s_Media, EncInfo->s_Port, EncInfo->s_MediaId, EncInfo->s_StreamSize, ExtEncInfo->s_FrameType, ExtEncInfo->s_FrameNum, (int32_t) EncInfo->s_PTS.tv_sec, (int32_t) EncInfo->s_PTS.tv_usec, ExtEncInfo->s_Length );
        return;
    }

    Result = VideoSourceFilter->m_Outputs[0]->Receive( EncInfo->s_StreamAddr, EncInfo->s_StreamSize, &EncInfo->s_PTS, ExtEncInfo, ExtensionSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::MediaEncCallBack, output pin Receive failed and return %x , Media=%d, Port=%d, MediaId=%d, StreamSize=%d, FrameType=%d, FrameNum=%d, second=%d:millisecond=%d, ExtraDataLength=%d\n",
                   (uint32_t)Result, EncInfo->s_Media, EncInfo->s_Port, EncInfo->s_MediaId, EncInfo->s_StreamSize, ExtEncInfo->s_FrameType, ExtEncInfo->s_FrameNum, (int32_t) EncInfo->s_PTS.tv_sec, (int32_t) EncInfo->s_PTS.tv_usec, ExtEncInfo->s_Length );
        return;
    }
#endif
}

#if USE_SIMULATOR_VIDEO_SOUCRE
void_t* H264_VideoSourceFilter::SimulatedDataSourceThread( void_t *Argument )
{
    H264_VideoSourceFilter *SimulatedDataSource = reinterpret_cast<H264_VideoSourceFilter*> ( Argument );
    void_t *Return = SimulatedDataSource->SimulatedDataSourceEntry();
    return Return;
}

// Definition of nalu unit type
#define NALU_TYPE_NONIDR            1
#define NALU_TYPE_IDR               5
#define NALU_TYPE_SEI               6
#define NALU_TYPE_SPS               7
#define NALU_TYPE_PPS               8
#define NALU_TYPE_AUD               9

struct VideoFrameInfo
{
    uint8_t *s_Frame;
    size_t  s_FrameLength;
    VideoFrameInfo( uint8_t *Frame, size_t FrameLength ) : s_Frame( Frame ), s_FrameLength( FrameLength ) {}
};

void_t* H264_VideoSourceFilter::SimulatedDataSourceEntry()
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SimulatedDataSourceEntry begin \n" );
    m_ThreadWorking        = true;
    GMI_RESULT Result      = GMI_FAIL;

    // read video file
    uint8_t  * From = m_VideoData;
    uint32_t   Left = m_VideoSourceFileLength;
    while ( !feof(m_VideoSourceFile) )
    {
        uint32_t ReadBytes = fread(From, 1, Left, m_VideoSourceFile);
        if (ReadBytes < 0)
        {
            printf( "H264_VideoSourceFilter::SimulatedDataSourceEntry, read file error \n" );
            return (void_t *) size_t(GMI_FAIL);
        }
        else if (ReadBytes == 0)
        {
            break;
        }

        printf( "H264_VideoSourceFilter::SimulatedDataSourceEntry, read %d bytes \n", ReadBytes );

        From += ReadBytes;
        Left -= ReadBytes;
    }

    m_VideoSourceFileLength -= Left;

    From = m_VideoData;
    Left = m_VideoSourceFileLength;

    // parse video frame
    boolean_t   LastNaluIsVCL = false;

    std::vector< VideoFrameInfo> VideoFrames;
    while (Left > 0)
    {
        uint8_t   * p    = From;
        uint32_t    Size = 0;

        if (Left < 5)
        {
            break;
        }

        while (Left > 0)
        {
            uint32_t Next4Bytes = ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
            if (Next4Bytes == 0x00000001)
            {
                if (Left > 4)
                {
                    uint8_t NaluType = (p[4] & 0x1F);
                    if (NaluType == NALU_TYPE_NONIDR || NaluType == NALU_TYPE_IDR)
                    {
                        LastNaluIsVCL = true;
                    }
                    else if (LastNaluIsVCL)
                    {
                        LastNaluIsVCL = false;
                        break;
                    }
                }
                else
                {
                    break;
                }

                p    += 4;
                Left -= 4;
                Size += 4;
            }

            uint32_t SkipBytes = 1;
            if (p[3] > 0)
            {
                SkipBytes = 4;
            }
            else if (p[2] > 0)
            {
                SkipBytes = 3;
            }
            else if (p[1] > 0)
            {
                SkipBytes = 2;
            }

            if (SkipBytes > Left)
            {
                SkipBytes = Left;
            }

            p    += SkipBytes;
            Left -= SkipBytes;
            Size += SkipBytes;
        }

        VideoFrames.push_back( VideoFrameInfo( From, Size ) );
        From += Size;
    }
    // parsing end

    if ( 0 == VideoFrames.size() )
    {
        printf( "H264_VideoSourceFilter::SimulatedDataSourceEntry, no video frame \n" );
        return (void_t *) size_t(GMI_NO_AVAILABLE_DATA);
    }

    uint32_t index = 0;
    uint32_t SleepTime = (uint32_t)(1000/m_FrameRate);
    while( !m_ThreadExitFlag )
    {
        struct timeval CurrentTime;
        gettimeofday1( &CurrentTime, NULL );

        // notify base class of new data coming
        GMI_RESULT Result = Receive( 0, VideoFrames[index].s_Frame, VideoFrames[index].s_FrameLength, &CurrentTime, NULL, 0 );
        Result = m_Outputs[0]->Receive( VideoFrames[index].s_Frame, VideoFrames[index].s_FrameLength, &CurrentTime, NULL, 0 );
        READ_MYSELF(Result);

        if ( ++index == VideoFrames.size() )
        {
            index = 0;
        }
        GMI_Sleep( SleepTime );
    }

    VideoFrames.clear();
    m_ThreadWorking = false;
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::SimulatedDataSourceThread end, function return %x \n", (uint32_t) Result );
    return (void_t *) size_t(Result);
}
#endif

GMI_RESULT H264_VideoSourceFilter::GetVideoMonitorEnableConfig( boolean_t *Enable )
{
    int32_t DefaultEnable = GMI_H264_VIDEO_MONITOR_CONFIG_ENABLE_VALUE;
    int32_t IntEnable = GMI_H264_VIDEO_MONITOR_CONFIG_ENABLE_VALUE;

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::GetVideoMonitorEnableConfig, GMI_XmlOpen failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetVideoMonitorEnableConfig, DefaultEnable=%d \n", DefaultEnable );

    Result = GMI_XmlRead(Handle, GMI_H264_VIDEO_MONITOR_CONFIG_PATH, GMI_H264_VIDEO_MONITOR_CONFIG_ENABLE_KEY_NAME, DefaultEnable, &IntEnable, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::GetVideoMonitorEnableConfig, GMI_XmlRead failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::GetVideoMonitorEnableConfig, GMI_XmlFileSave failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetVideoMonitorEnableConfig, DefaultEnable=%d, IntEnable=%d \n", DefaultEnable, IntEnable );
#endif

    // the following code facilitate unitest check execution, uncomment it to enable print info
    //printf( "H264_VideoSourceFilter::GetVideoMonitorEnableConfig, DefaultEnable=%d, IntEnable=%d \n", DefaultEnable, IntEnable );

    *Enable = (IntEnable == 0) ? false : true;
    return GMI_SUCCESS;
}

GMI_RESULT H264_VideoSourceFilter::GetVideoFrameCheckInterval( uint32_t *FrameNumber )
{
    int32_t DefaultFrameCheckInterval = GMI_H264_VIDEO_MONITOR_CONFIG_FRAME_CHECK_INTERVAL_VALUE;
    int32_t FrameCheckInterval = GMI_H264_VIDEO_MONITOR_CONFIG_FRAME_CHECK_INTERVAL_VALUE;

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::GetVideoFrameCheckInterval, GMI_XmlOpen failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetVideoFrameCheckInterval, DefaultFrameCheckInterval=%d \n", DefaultFrameCheckInterval );

    Result = GMI_XmlRead(Handle, GMI_H264_VIDEO_MONITOR_CONFIG_PATH, GMI_H264_VIDEO_MONITOR_CONFIG_FRAME_CHECK_INTERVAL_KEY_NAME, DefaultFrameCheckInterval, &FrameCheckInterval, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::GetVideoFrameCheckInterval, GMI_XmlRead failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "H264_VideoSourceFilter::GetVideoFrameCheckInterval, GMI_XmlFileSave failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "H264_VideoSourceFilter::GetVideoFrameCheckInterval, DefaultFrameCheckInterval=%d, FrameCheckInterval=%d \n", DefaultFrameCheckInterval, FrameCheckInterval );
#endif

    // the following code facilitate unitest check execution, uncomment it to enable print info
    //printf( "H264_VideoSourceFilter::GetVideoFrameCheckInterval, DefaultFrameCheckInterval=%d, FrameCheckInterval=%d \n", DefaultFrameCheckInterval, FrameCheckInterval );

    *FrameNumber = FrameCheckInterval;
    return GMI_SUCCESS;
}
