#include "g726_audio_encode_source_filter.h"

#include "log_client.h"
#include "media_codec_parameter.h"

G726_AudioEncodeSourceFilter::G726_AudioEncodeSourceFilter(void)
    : m_HardwareSource( NULL )
#if SAVE_G726_AUDIO_ENCODE_FILE
    , m_AudioFile( NULL )
#endif
{
}

G726_AudioEncodeSourceFilter::~G726_AudioEncodeSourceFilter(void)
{
}

G726_AudioEncodeSourceFilter*  G726_AudioEncodeSourceFilter::CreateNew()
{
    return BaseMemoryManager::Instance().New<G726_AudioEncodeSourceFilter>();
}

GMI_RESULT	G726_AudioEncodeSourceFilter::Initialize( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::Initialize begin \n" );

    MediaSourceParameter *SourceParameter = reinterpret_cast<MediaSourceParameter *> (Argument);
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::Initialize, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d\n", SourceParameter->s_SourceId, SourceParameter->s_MediaId, SourceParameter->s_MediaType, SourceParameter->s_CodecType );
    if ( CODEC_G711A != SourceParameter->s_CodecType )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::Initialize, EncodeParameter->CodecType=%d, not CODEC_G711A(%d), function return %x \n", SourceParameter->s_CodecType, CODEC_G711A, GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Result = AudioSourceFilter::Initialize( FilterId, FilterName, FilterNameLength, Argument, ArgumentSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::Initialize, AudioSourceFilter::Initialize return %x \n", (uint32_t) Result );
        return Result;
    }

    m_HardwareSource = GMI_AudioEncCreate( SourceParameter->s_SourceId, (AudioEncParam *) SourceParameter->s_InternalParamter );
    if ( NULL == m_HardwareSource )
    {
        AudioSourceFilter::Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::Initialize, GMI_AudioEncCreate failed, function return %x \n", (uint32_t) GMI_OPEN_DEVICE_FAIL );
        return GMI_OPEN_DEVICE_FAIL;
    }

    Result = GMI_AudioEncSetCB( m_HardwareSource, MediaEncCallBack, this );
    if ( FAILED( Result ) )
    {
        GMI_AudioEncDestroy( m_HardwareSource );
        m_HardwareSource = NULL;
        AudioSourceFilter::Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::Initialize, GMI_AudioEncSetCB failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

#if SAVE_G726_AUDIO_ENCODE_FILE
    char_t FileName[MAX_PATH_LENGTH];
#if defined( __linux__ )
    sprintf( FileName, "%p_g726_encode_audio.726", this );
#elif defined( _WIN32 )
    sprintf_s( FileName, MAX_PATH_LENGTH, "%p_g726_encode_audio.726", this );
#endif
    printf( "File Name : %s\n", FileName );
#if defined( __linux__ )
    m_AudioFile = fopen( FileName, "wb" );
#elif defined( _WIN32 )
    fopen_s( &m_AudioFile, FileName, "wb" );
#endif
#endif

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::Initialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT	G726_AudioEncodeSourceFilter::Deinitialize()
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::Deinitialize begin \n" );
#if SAVE_G726_AUDIO_ENCODE_FILE
    if ( NULL != m_AudioFile )
    {
        fclose( m_AudioFile );
        m_AudioFile = NULL;
    }
#endif

    GMI_RESULT Result = AudioSourceFilter::Deinitialize();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::Deinitialize, AudioSourceFilter::Deinitialize failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    if ( NULL != m_HardwareSource )
    {
        GMI_AudioEncDestroy( m_HardwareSource );
        m_HardwareSource = NULL;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::Deinitialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT	G726_AudioEncodeSourceFilter::Play()
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::Play begin \n" );
    if ( NULL == m_HardwareSource )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::Play, video encode device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = AudioSourceFilter::Play();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::Play, AudioSourceFilter::Play failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = GMI_AudioEncStart( m_HardwareSource );
    if ( FAILED( Result ) )
    {
        AudioSourceFilter::Stop();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::Play, GMI_VideoEncStart failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::Play end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT	G726_AudioEncodeSourceFilter::Stop()
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::Stop begin \n" );
    if ( NULL == m_HardwareSource )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::Stop, video encode device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = AudioSourceFilter::Stop();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::Play, AudioSourceFilter::Stop failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = GMI_AudioEncStop( m_HardwareSource );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::Play, GMI_AudioEncStop failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::Stop end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT  G726_AudioEncodeSourceFilter::GetEncodeConfig( void_t *EncodeParameter, size_t *EncodeParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::GetEncodeConfig begin, passed EncodeParameterLength=%d \n", *EncodeParameterLength );
    if ( NULL == m_HardwareSource )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::GetEncodeConfig, video encode device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(AudioEncParam) > *EncodeParameterLength )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::GetEncodeConfig, provided buffer space is not enough, function return %x \n", (uint32_t) GMI_NOT_ENOUGH_SPACE );
        return GMI_NOT_ENOUGH_SPACE;
    }

    GMI_RESULT Result = GMI_AudioGetEncConfig( m_HardwareSource, (AudioEncParam*)EncodeParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::GetEncodeConfig, GMI_AudioGetEncConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::GetEncodeConfig, AudioId      = %d\n", ((AudioEncParam*)EncodeParameter)->s_AudioId );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::GetEncodeConfig, Codec        = %d\n", ((AudioEncParam*)EncodeParameter)->s_Codec );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::GetEncodeConfig, SampleFreq   = %d\n", ((AudioEncParam*)EncodeParameter)->s_SampleFreq );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::GetEncodeConfig, BitWidth     = %d\n", ((AudioEncParam*)EncodeParameter)->s_BitWidth );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::GetEncodeConfig, FrameRate    = %d\n", ((AudioEncParam*)EncodeParameter)->s_FrameRate );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::GetEncodeConfig, BitRate      = %d\n", ((AudioEncParam*)EncodeParameter)->s_BitRate );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::GetEncodeConfig, ChannelNum   = %d\n", ((AudioEncParam*)EncodeParameter)->s_ChannelNum );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::GetEncodeConfig, Volume       = %d\n", ((AudioEncParam*)EncodeParameter)->s_Volume );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::GetEncodeConfig, AecFlag      = %d\n", ((AudioEncParam*)EncodeParameter)->s_AecFlag );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::GetEncodeConfig, AecDelayTime = %d\n", ((AudioEncParam*)EncodeParameter)->s_AecDelayTime );
#endif

    *EncodeParameterLength = sizeof(AudioEncParam);
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::GetEncodeConfig end, returned EncodeParameterLength=%d \n", *EncodeParameterLength );
    return GMI_SUCCESS;
}

GMI_RESULT  G726_AudioEncodeSourceFilter::SetEncodeConfig( const void_t *EncodeParameter, size_t EncodeParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::SetEncodeConfig begin, passed EncodeParameterLength=%d \n", EncodeParameterLength );
    if ( NULL == m_HardwareSource )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::SetEncodeConfig, audio encode device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(AudioEncParam) > EncodeParameterLength )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::GetEncodeConfig, provided buffer space is not enough, function return %x \n", (uint32_t) GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::SetEncodeConfig, AudioId      = %d\n", ((AudioEncParam*)EncodeParameter)->s_AudioId );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::SetEncodeConfig, Codec        = %d\n", ((AudioEncParam*)EncodeParameter)->s_Codec );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::SetEncodeConfig, SampleFreq   = %d\n", ((AudioEncParam*)EncodeParameter)->s_SampleFreq );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::SetEncodeConfig, BitWidth     = %d\n", ((AudioEncParam*)EncodeParameter)->s_BitWidth );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::SetEncodeConfig, FrameRate    = %d\n", ((AudioEncParam*)EncodeParameter)->s_FrameRate );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::SetEncodeConfig, BitRate      = %d\n", ((AudioEncParam*)EncodeParameter)->s_BitRate );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::SetEncodeConfig, ChannelNum   = %d\n", ((AudioEncParam*)EncodeParameter)->s_ChannelNum );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::SetEncodeConfig, Volume       = %d\n", ((AudioEncParam*)EncodeParameter)->s_Volume );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::SetEncodeConfig, AecFlag      = %d\n", ((AudioEncParam*)EncodeParameter)->s_AecFlag );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::SetEncodeConfig, AecDelayTime = %d\n", ((AudioEncParam*)EncodeParameter)->s_AecDelayTime );
#endif

    GMI_RESULT Result = GMI_AudioSetEncConfig( m_HardwareSource, (AudioEncParam*)EncodeParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::SetEncodeConfig, GMI_AudioSetEncConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "G726_AudioEncodeSourceFilter::SetEncodeConfig end \n" );
    return GMI_SUCCESS;
}

void_t G726_AudioEncodeSourceFilter::MediaEncCallBack( void_t *UserDataPtr, MediaEncInfo *EncInfo, ExtMediaEncInfo *ExtEncInfo )
{
    G726_AudioEncodeSourceFilter *AudioSourceFilter = (G726_AudioEncodeSourceFilter *) UserDataPtr;

#if SAVE_G726_AUDIO_ENCODE_FILE
    if ( NULL != AudioSourceFilter->m_AudioFile )
    {
        fwrite( EncInfo->s_StreamAddr, 1, EncInfo->s_StreamSize, AudioSourceFilter->m_AudioFile );
    }
#endif

    size_t ExtensionSize = 0;
    if ( NULL != ExtEncInfo )
    {
        ExtensionSize = sizeof(uint32_t)+sizeof(uint32_t)+sizeof(size_t)+ExtEncInfo->s_Length;
    }

    GMI_RESULT Result = AudioSourceFilter->m_Outputs[0]->Receive( EncInfo->s_StreamAddr, EncInfo->s_StreamSize, &EncInfo->s_PTS, ExtEncInfo, ExtensionSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "G726_AudioEncodeSourceFilter::MediaEncCallBack, output pin Receive failed and return %x , Media=%d, Port=%d, MediaId=%d, StreamSize=%d, FrameType=%d, FrameNum=%d, second=%d:millisecond=%d, ExtraDataLength=%d\n",
                   (uint32_t)Result, EncInfo->s_Media, EncInfo->s_Port, EncInfo->s_MediaId, EncInfo->s_StreamSize, ExtEncInfo->s_FrameType, ExtEncInfo->s_FrameNum, (int32_t) EncInfo->s_PTS.tv_sec, (int32_t) EncInfo->s_PTS.tv_usec, ExtEncInfo->s_Length );
        return;
    }
}
