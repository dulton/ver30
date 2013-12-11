#include "g711a_audio_decode_handler_filter.h"

#include "media_codec_parameter.h"
#include "share_memory_log_client.h"

G711A_AudioDecodeHandlerFilter::G711A_AudioDecodeHandlerFilter(void)
    : m_HardwareSource( NULL )
#if SAVE_G711A_AUDIO_DECODE_FILE
    , m_AudioFile( NULL )
#endif
{
}

G711A_AudioDecodeHandlerFilter::~G711A_AudioDecodeHandlerFilter(void)
{
}

G711A_AudioDecodeHandlerFilter*  G711A_AudioDecodeHandlerFilter::CreateNew()
{
    return BaseMemoryManager::Instance().New<G711A_AudioDecodeHandlerFilter>();
}

GMI_RESULT	G711A_AudioDecodeHandlerFilter::Initialize( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::Initialize begin \n" );

    MediaSourceParameter *SourceParameter = reinterpret_cast<MediaSourceParameter *> (Argument);
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::Initialize, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d\n", SourceParameter->s_SourceId, SourceParameter->s_MediaId, SourceParameter->s_MediaType, SourceParameter->s_CodecType );
    if ( CODEC_G711A != SourceParameter->s_CodecType )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "G711A_AudioDecodeHandlerFilter::Initialize, EncodeParameter->CodecType=%d, not CODEC_G711A(%d), function return %x \n", SourceParameter->s_CodecType, CODEC_G711A, GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Result = AudioHandlerFilter::Initialize( FilterId, FilterName, FilterNameLength, Argument, ArgumentSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "G711A_AudioDecodeHandlerFilter::Initialize, AudioHandlerFilter::Initialize return %x \n", (uint32_t) Result );
        return Result;
    }

    m_HardwareSource = GMI_AudioDecCreate( SourceParameter->s_SourceId, (AudioDecParam *) SourceParameter->s_InternalParamter );
    if ( NULL == m_HardwareSource )
    {
        AudioHandlerFilter::Deinitialize();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "G711A_AudioDecodeHandlerFilter::Initialize, GMI_AudioDecCreate failed, function return %x \n", (uint32_t) GMI_OPEN_DEVICE_FAIL );
        return GMI_OPEN_DEVICE_FAIL;
    }

#if SAVE_G711A_AUDIO_DECODE_FILE
    char_t FileName[MAX_PATH_LENGTH];
#if defined( __linux__ )
    sprintf( FileName, "%p_g711a_decode_audio.711", this );
#elif defined( _WIN32 )
    sprintf_s( FileName, MAX_PATH_LENGTH, "%p_g711a_decode_audio.711", this );
#endif
    printf( "File Name : %s\n", FileName );
#if defined( __linux__ )
    m_AudioFile = fopen( FileName, "wb" );
#elif defined( _WIN32 )
    fopen_s( &m_AudioFile, FileName, "wb" );
#endif
#endif

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::Initialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT	G711A_AudioDecodeHandlerFilter::Deinitialize()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::Deinitialize begin \n" );
#if SAVE_G711A_AUDIO_DECODE_FILE
    if ( NULL != m_AudioFile )
    {
        fclose( m_AudioFile );
        m_AudioFile = NULL;
    }
#endif

    GMI_RESULT Result = AudioHandlerFilter::Deinitialize();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "G711A_AudioDecodeHandlerFilter::Deinitialize, AudioHandlerFilter::Deinitialize failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    if ( NULL != m_HardwareSource )
    {
        GMI_AudioDecDestroy( m_HardwareSource );
        m_HardwareSource = NULL;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::Deinitialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT	G711A_AudioDecodeHandlerFilter::Play()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::Play begin \n" );
    if ( NULL == m_HardwareSource )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "G711A_AudioDecodeHandlerFilter::Play, audio decode device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = AudioHandlerFilter::Play();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "G711A_AudioDecodeHandlerFilter::Play, AudioHandlerFilter::Play failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::Play end \n" );
    return Result;
}

GMI_RESULT	G711A_AudioDecodeHandlerFilter::Stop()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::Stop begin \n" );
    if ( NULL == m_HardwareSource )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "G711A_AudioDecodeHandlerFilter::Stop, audio decode device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = AudioHandlerFilter::Stop();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "G711A_AudioDecodeHandlerFilter::Play, AudioHandlerFilter::Stop failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::Stop end \n" );
    return Result;
}

GMI_RESULT  G711A_AudioDecodeHandlerFilter::GetDecodeConfig( void_t *DecodeParameter, size_t *DecodeParameterLength )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::GetDecodeConfig begin, passed DecodeParameterLength=%d \n", *DecodeParameterLength );
    if ( NULL == m_HardwareSource )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "G711A_AudioDecodeHandlerFilter::GetDecodeConfig, audio decode device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(AudioDecParam) > *DecodeParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "G711A_AudioDecodeHandlerFilter::GetDecodeConfig, provided buffer space is not enough, function return %x \n", (uint32_t) GMI_NOT_ENOUGH_SPACE );
        return GMI_NOT_ENOUGH_SPACE;
    }

    GMI_RESULT Result = GMI_AudioGetDecConfig( m_HardwareSource, (AudioDecParam*)DecodeParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "G711A_AudioDecodeHandlerFilter::GetDecodeConfig, GMI_AudioGetEncConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::GetDecodeConfig, AudioId      = %d\n", ((AudioDecParam*)DecodeParameter)->s_AudioId );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::GetDecodeConfig, Codec        = %d\n", ((AudioDecParam*)DecodeParameter)->s_Codec );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::GetDecodeConfig, SampleFreq   = %d\n", ((AudioDecParam*)DecodeParameter)->s_SampleFreq );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::GetDecodeConfig, BitWidth     = %d\n", ((AudioDecParam*)DecodeParameter)->s_BitWidth );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::GetDecodeConfig, ChannelNum   = %d\n", ((AudioDecParam*)DecodeParameter)->s_ChannelNum );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::GetDecodeConfig, FrameRate    = %d\n", ((AudioDecParam*)DecodeParameter)->s_FrameRate );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::GetDecodeConfig, BitRate      = %d\n", ((AudioDecParam*)DecodeParameter)->s_BitRate );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::GetDecodeConfig, ChannelNum   = %d\n", ((AudioDecParam*)DecodeParameter)->s_ChannelNum );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::GetDecodeConfig, Volume       = %d\n", ((AudioDecParam*)DecodeParameter)->s_Volume );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::GetDecodeConfig, AecFlag      = %d\n", ((AudioDecParam*)DecodeParameter)->s_AecFlag );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::GetDecodeConfig, AecDelayTime = %d\n", ((AudioDecParam*)DecodeParameter)->s_AecDelayTime );
#endif

    *DecodeParameterLength = sizeof(AudioDecParam);
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::GetEncodeConfig end, returned DecodeParameterLength=%d \n", *DecodeParameterLength );
    return GMI_SUCCESS;
}

GMI_RESULT  G711A_AudioDecodeHandlerFilter::SetDecodeConfig( const void_t *DecodeParameter, size_t DecodeParameterLength )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::SetDecodeConfig begin, passed DecodeParameterLength=%d \n", DecodeParameterLength );
    if ( NULL == m_HardwareSource )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "G711A_AudioDecodeHandlerFilter::SetDecodeConfig, audio decode device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(AudioDecParam) > DecodeParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "G711A_AudioDecodeHandlerFilter::SetDecodeConfig, provided buffer space is not enough, function return %x \n", (uint32_t) GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::SetDecodeConfig, AudioId      = %d\n", ((AudioEncParam*)DecodeParameter)->s_AudioId );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::SetDecodeConfig, Codec        = %d\n", ((AudioEncParam*)DecodeParameter)->s_Codec );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::SetDecodeConfig, SampleFreq   = %d\n", ((AudioEncParam*)DecodeParameter)->s_SampleFreq );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::SetDecodeConfig, BitWidth     = %d\n", ((AudioEncParam*)DecodeParameter)->s_BitWidth );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::SetDecodeConfig, ChannelNum   = %d\n", ((AudioDecParam*)DecodeParameter)->s_ChannelNum );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::SetDecodeConfig, FrameRate    = %d\n", ((AudioEncParam*)DecodeParameter)->s_FrameRate );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::SetDecodeConfig, BitRate      = %d\n", ((AudioEncParam*)DecodeParameter)->s_BitRate );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::SetDecodeConfig, ChannelNum   = %d\n", ((AudioEncParam*)DecodeParameter)->s_ChannelNum );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::SetDecodeConfig, Volume       = %d\n", ((AudioEncParam*)DecodeParameter)->s_Volume );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::SetDecodeConfig, AecFlag      = %d\n", ((AudioEncParam*)DecodeParameter)->s_AecFlag );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::SetDecodeConfig, AecDelayTime = %d\n", ((AudioEncParam*)DecodeParameter)->s_AecDelayTime );
#endif

    GMI_RESULT Result = GMI_AudioSetDecConfig( m_HardwareSource, (AudioDecParam*)DecodeParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "G711A_AudioDecodeHandlerFilter::SetEncodeConfig, GMI_AudioSetEncConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "G711A_AudioDecodeHandlerFilter::SetEncodeConfig end \n" );
    return GMI_SUCCESS;
}

#include "timer_task_queue.h"

GMI_RESULT  G711A_AudioDecodeHandlerFilter::Receive( int32_t InputPinIndex, const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength )
{
#if SAVE_G711A_AUDIO_DECODE_FILE
    if ( NULL != AudioSourceFilter->m_AudioFile )
    {
        fwrite( Frame, 1, FrameLength, m_AudioFile );
    }
#endif

#if 1

    GMI_RESULT Result = GMI_AudioDecOneFrame( m_HardwareSource, const_cast<uint8_t *>(Frame), (uint32_t) FrameLength );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "G711A_AudioDecodeHandlerFilter::Receive, GMI_AudioDecOneFrame failed, function return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    struct timeval begin_time;
    gettimeofday1( &begin_time, NULL );

    printf( "G711A_AudioDecodeHandlerFilter::Receive, InputPinIndex=%d, Frame=%p, FrameLength=%d, time=%d:%6d, ExtraData=%p, ExtraDataLength=%d \n",
            InputPinIndex, Frame, FrameLength, (uint32_t) FrameTS->tv_sec, (uint32_t) FrameTS->tv_usec, ExtraData, ExtraDataLength );

    struct timeval end_time;
    gettimeofday1( &end_time, NULL );

    uint32_t spent_time = 1000*(end_time.tv_sec - begin_time.tv_sec) + end_time.tv_usec/1000 - begin_time.tv_usec/1000;

    printf( "G711A_AudioDecodeHandlerFilter::Receive, GMI_AudioDecOneFrame spent %dms \n", spent_time );
#endif

    return GMI_SUCCESS;
}
