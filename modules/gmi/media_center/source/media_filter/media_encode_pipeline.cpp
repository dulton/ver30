#include "media_encode_pipeline.h"

#include "base_streaming_pipeline_manager.h"
#include "g711a_audio_encode_source_filter.h"
#include "g711u_audio_encode_source_filter.h"
#include "g726_audio_encode_source_filter.h"
#include "h264_video_source_filter.h"
#include "log_client.h"
#include "ipc_media_data_dispatch_handler.h"
#include "media_codec_parameter.h"
#include "mjpeg_image_source_filter.h"
#include "mpeg4_video_source_filter.h"

MediaEncodePipeline::MediaEncodePipeline(void)
    : m_StreamingPipelineManager( NULL )
    , m_SourceId( 0 )
    , m_MediaId( 0 )
    , m_MediaType( 0 )
    , m_CodecType( 0 )
    , m_MediaSource( NULL )
    , m_MediaDataDispatchHandler( NULL )
{
}

MediaEncodePipeline::~MediaEncodePipeline(void)
{
}

GMI_RESULT MediaEncodePipeline::Initialize( uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, void_t *EncodeParameter, size_t EncodeParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::Initialize begin \n" );

    MediaSourceParameter Parameter;
    Parameter.s_SourceId         = SourceId;
    Parameter.s_MediaId          = MediaId;
    Parameter.s_MediaType        = MediaType;
    Parameter.s_CodecType        = CodecType;
    Parameter.s_BitRate          = 0;
    Parameter.s_FrameRate        = 0;
    Parameter.s_InternalParamter = EncodeParameter;

    GMI_RESULT Result = GMI_FAIL;
    switch ( MAKE_MEDIA(MediaType,CodecType) )
    {
    case MEDIA_VIDEO_H264:
        m_MediaSource = H264_VideoSourceFilter::CreateNew();
        if ( NULL == m_MediaSource )
        {
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, H264_VideoSourceFilter object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
            return GMI_OUT_OF_MEMORY;
        }

        Parameter.s_BitRate   = (( VideoEncodeParam * )EncodeParameter)->s_BitRateAverage;
        Parameter.s_FrameRate = (( VideoEncodeParam * )EncodeParameter)->s_FrameRate;

        if ( 0 == Parameter.s_BitRate || 0 == Parameter.s_FrameRate )
        {
            m_MediaSource->ReleaseRef();
            m_MediaSource = NULL;
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, H264EncodeSource parameter error, function return %x \n", (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }

        Result = m_MediaSource->Initialize( (SourceId<<24)+(MediaId<<16)+(MediaType<<8)+CodecType, "H264EncodeSource", 17, &Parameter, sizeof(Parameter) );
        if ( FAILED( Result ) )
        {
            m_MediaSource->ReleaseRef();
            m_MediaSource = NULL;
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, H264EncodeSource initialization failed, function return %x \n", (uint32_t) Result );
            return Result;
        }
        break;

#if 0 // MEDIA_VIDEO_MJPEG and MEDIA_VIDEO_MPEG4 are not supported for now
    case MEDIA_VIDEO_MJPEG:
        m_MediaSource = MJPEG_ImageSourceFilter::CreateNew();
        if ( NULL == m_MediaSource )
        {
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, MJPEG_ImageSourceFilter object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
            return GMI_OUT_OF_MEMORY;
        }
        break;

    case MEDIA_VIDEO_MPEG4:
        m_MediaSource = Mpeg4VideoSourceFilter::CreateNew();
        if ( NULL == m_MediaSource )
        {
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, Mpeg4VideoSourceFilter object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
            return GMI_OUT_OF_MEMORY;
        }
        break;
#endif

    case MEDIA_AUDIO_G711A:
        m_MediaSource = G711A_AudioEncodeSourceFilter::CreateNew();
        if ( NULL == m_MediaSource )
        {
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, G711A_AudioEncodeSourceFilter object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
            return GMI_OUT_OF_MEMORY;
        }

        Parameter.s_BitRate   = (( AudioEncParam * )EncodeParameter)->s_BitRate;
        Parameter.s_FrameRate = (( AudioEncParam * )EncodeParameter)->s_FrameRate;

        if ( 0 == Parameter.s_BitRate || 0 == Parameter.s_FrameRate )
        {
            m_MediaSource->ReleaseRef();
            m_MediaSource = NULL;
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, G711A_AudioEncodeSource parameter error, function return %x \n", (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }

        Result = m_MediaSource->Initialize( (SourceId<<24)+(MediaId<<16)+(MediaType<<8)+CodecType, "G711A_AudioSource", 18, &Parameter, sizeof(Parameter) );
        if ( FAILED( Result ) )
        {
            m_MediaSource->ReleaseRef();
            m_MediaSource = NULL;
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, G711A_AudioEncodeSourceFilter initialization failed, function return %x \n", (uint32_t) Result );
            return Result;
        }
        break;

    case MEDIA_AUDIO_G711U:
        m_MediaSource = G711U_AudioEncodeSourceFilter::CreateNew();
        if ( NULL == m_MediaSource )
        {
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, G711U_AudioEncodeSourceFilter object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
            return GMI_OUT_OF_MEMORY;
        }

        Parameter.s_BitRate   = (( AudioEncParam * )EncodeParameter)->s_BitRate;
        Parameter.s_FrameRate = (( AudioEncParam * )EncodeParameter)->s_FrameRate;

        if ( 0 == Parameter.s_BitRate || 0 == Parameter.s_FrameRate )
        {
            m_MediaSource->ReleaseRef();
            m_MediaSource = NULL;
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, G711U_AudioEncodeSource parameter error, function return %x \n", (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }

        Result = m_MediaSource->Initialize( (SourceId<<24)+(MediaId<<16)+(MediaType<<8)+CodecType, "G711U_AudioSource", 18, &Parameter, sizeof(Parameter) );
        if ( FAILED( Result ) )
        {
            m_MediaSource->ReleaseRef();
            m_MediaSource = NULL;
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, G711U_AudioSource initialization failed, function return %x \n", (uint32_t) Result );
            return Result;
        }
        break;

#if 0 // no G726 definition
    case MEDIA_AUDIO_G726:
        m_MediaSource = G726_AudioEncodeSourceFilter::CreateNew();
        if ( NULL == m_MediaSource )
        {
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, G726_AudioEncodeSourceFilter object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
            return GMI_OUT_OF_MEMORY;
        }

        Parameter.s_BitRate   = (( AudioEncParam * )EncodeParameter)->s_BitRate;
        Parameter.s_FrameRate = (( AudioEncParam * )EncodeParameter)->s_FrameRate;

        if ( 0 == Parameter.s_BitRate || 0 == Parameter.s_FrameRate )
        {
            m_MediaSource->ReleaseRef();
            m_MediaSource = NULL;
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, G726_AudioEncodeSource parameter error, function return %x \n", (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }

        Result = m_MediaSource->Initialize( (SourceId<<24)+(MediaId<<16)+(MediaType<<8)+CodecType, "G726_AudioSource", 17, &Parameter, sizeof(Parameter) );
        if ( FAILED( Result ) )
        {
            m_MediaSource->ReleaseRef();
            m_MediaSource = NULL;
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, G726_AudioSource initialization failed, function return %x \n", (uint32_t) Result );
            return Result;
        }
        break;
#endif

    default:
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, unsupported media type(%x), function return %x \n", MAKE_MEDIA(MediaType,CodecType), (uint32_t) GMI_NOT_SUPPORT );
        return GMI_NOT_SUPPORT;
    }

    m_MediaDataDispatchHandler = IpcMediaDataDispatchHandler::CreateNew();
    if ( NULL == m_MediaDataDispatchHandler )
    {
        m_MediaSource->Deinitialize();
        m_MediaSource->ReleaseRef();
        m_MediaSource = NULL;
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, IpcMediaDataDispatchHandler object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    Result = m_MediaDataDispatchHandler->Initialize( (SourceId<<24)+(MediaId<<16)+(MediaType<<8)+CodecType, "MediaDataDispatch", 17, &Parameter, sizeof(Parameter) );
    if ( FAILED( Result ) )
    {
        m_MediaDataDispatchHandler->ReleaseRef();
        m_MediaDataDispatchHandler = NULL;
        m_MediaSource->Deinitialize();
        m_MediaSource->ReleaseRef();
        m_MediaSource = NULL;
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, MediaDataDispatchHandler object initializaztion failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    m_StreamingPipelineManager = BaseStreamingPipelineManager::CreateNew( MFM_Push );
    if ( NULL == m_StreamingPipelineManager )
    {
        m_MediaDataDispatchHandler->Deinitialize();
        m_MediaDataDispatchHandler->ReleaseRef();
        m_MediaDataDispatchHandler = NULL;
        m_MediaSource->Deinitialize();
        m_MediaSource->ReleaseRef();
        m_MediaSource = NULL;
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, BaseStreamingPipelineManager object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    Result = m_StreamingPipelineManager->Initialize();
    if ( FAILED( Result ) )
    {
        m_StreamingPipelineManager->ReleaseRef();
        m_StreamingPipelineManager = NULL;
        m_MediaDataDispatchHandler->Deinitialize();
        m_MediaDataDispatchHandler->ReleaseRef();
        m_MediaDataDispatchHandler = NULL;
        m_MediaSource->Deinitialize();
        m_MediaSource->ReleaseRef();
        m_MediaSource = NULL;
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, BaseStreamingPipelineManager object initializaztion failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_StreamingPipelineManager->AddFilter( m_MediaSource );
    if ( FAILED( Result ) )
    {
        m_StreamingPipelineManager->Deinitialize();
        m_StreamingPipelineManager->ReleaseRef();
        m_StreamingPipelineManager = NULL;
        m_MediaDataDispatchHandler->Deinitialize();
        m_MediaDataDispatchHandler->ReleaseRef();
        m_MediaDataDispatchHandler = NULL;
        m_MediaSource->Deinitialize();
        m_MediaSource->ReleaseRef();
        m_MediaSource = NULL;
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, BaseStreamingPipelineManager add source filter failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_StreamingPipelineManager->AddFilter( m_MediaDataDispatchHandler );
    if ( FAILED( Result ) )
    {
        m_StreamingPipelineManager->Deinitialize();
        m_StreamingPipelineManager->ReleaseRef();
        m_StreamingPipelineManager = NULL;
        m_MediaDataDispatchHandler->Deinitialize();
        m_MediaDataDispatchHandler->ReleaseRef();
        m_MediaDataDispatchHandler = NULL;
        m_MediaSource->Deinitialize();
        m_MediaSource->ReleaseRef();
        m_MediaSource = NULL;
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, BaseStreamingPipelineManager add dispatch handller filter failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = BaseStreamingPipelineManager::ConnectFilter( m_MediaSource, 0, m_MediaDataDispatchHandler, 0 );
    if ( FAILED( Result ) )
    {
        m_StreamingPipelineManager->Deinitialize();
        m_StreamingPipelineManager->ReleaseRef();
        m_StreamingPipelineManager = NULL;
        m_MediaDataDispatchHandler->Deinitialize();
        m_MediaDataDispatchHandler->ReleaseRef();
        m_MediaDataDispatchHandler = NULL;
        m_MediaSource->Deinitialize();
        m_MediaSource->ReleaseRef();
        m_MediaSource = NULL;
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Initialize, BaseStreamingPipelineManager ConnectFilter failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    m_SourceId  = SourceId;
    m_MediaId   = MediaId;
    m_MediaType = MediaType;
    m_CodecType = CodecType;

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::Initialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT MediaEncodePipeline::Deinitialize()
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::Deinitialize begin \n" );
    Stop();

    GMI_RESULT Result = BaseStreamingPipelineManager::DisconnectFilter( m_MediaSource, 0, m_MediaDataDispatchHandler, 0 );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Deinitialize, BaseStreamingPipelineManager DisconnectFilter failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    if ( NULL != m_MediaSource )
    {
        Result = m_MediaSource->Deinitialize();
        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Deinitialize, MediaSource Deinitialize failed, function return %x \n", (uint32_t) Result );
            return Result;
        }

        m_MediaSource->ReleaseRef();
        m_MediaSource = NULL;
    }

    if ( NULL != m_MediaDataDispatchHandler )
    {
        Result = m_MediaDataDispatchHandler->Deinitialize();
        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Deinitialize, MediaDataDispatchHandler Deinitialize failed, function return %x \n", (uint32_t) Result );
            return Result;
        }

        m_MediaDataDispatchHandler->ReleaseRef();
        m_MediaDataDispatchHandler = NULL;
    }

    if ( NULL != m_StreamingPipelineManager )
    {
        Result = m_StreamingPipelineManager->Deinitialize();
        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Deinitialize, StreamingPipelineManager Deinitialize failed, function return %x \n", (uint32_t) Result );
            return Result;
        }

        m_StreamingPipelineManager->ReleaseRef();
        m_StreamingPipelineManager = NULL;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::Deinitialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT MediaEncodePipeline::Play()
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::Play begin \n" );
    if ( NULL == m_StreamingPipelineManager )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Play, device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = m_StreamingPipelineManager->Play();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Play, StreamingPipelineManager->Play failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::Play end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaEncodePipeline::Stop()
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::Stop begin \n" );
    if ( NULL == m_StreamingPipelineManager )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Stop, device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = m_StreamingPipelineManager->Stop();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::Stop, StreamingPipelineManager->Stop failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::Stop end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaEncodePipeline::GetEncodeConfig( void_t *EncodeParameter, size_t *EncodeParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::GetEncodeConfig begin, passed EncodeParameterLength=%d \n", *EncodeParameterLength );
    if ( NULL == m_MediaSource )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::GetEncodeConfig, device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    // later, we'd better let audio_source_filer or image_source_filter(or video_source_filter) abstract GetEncodeConfig interface
    GMI_RESULT Result = GMI_FAIL;
    uint32_t FinalMediaType = MAKE_MEDIA(m_MediaType,m_CodecType);
    switch( FinalMediaType )
    {
    case MEDIA_VIDEO_H264:
        Result = ((H264_VideoSourceFilter *)m_MediaSource)->GetEncodeConfig( EncodeParameter, EncodeParameterLength );
        break;

    case MEDIA_AUDIO_G711A:
        Result = ((G711A_AudioEncodeSourceFilter *)m_MediaSource)->GetEncodeConfig( EncodeParameter, EncodeParameterLength );
        break;

    case MEDIA_AUDIO_G711U:
        Result = ((G711U_AudioEncodeSourceFilter *)m_MediaSource)->GetEncodeConfig( EncodeParameter, EncodeParameterLength );
        break;

    default:
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::GetEncodeConfig, unsupported media type(%x), function return %x \n", FinalMediaType, (uint32_t) GMI_NOT_SUPPORT );
        return GMI_NOT_SUPPORT;
    }

    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::GetEncodeConfig, MediaSource::GetEncodeConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::GetEncodeConfig end and return %x, returned EncodeParameterLength=%d \n", (uint32_t) Result, *EncodeParameterLength );
    return Result;
}

GMI_RESULT MediaEncodePipeline::SetEncodeConfig( const void_t *EncodeParameter, size_t EncodeParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::SetEncodeConfig begin, passed EncodeParameterLength=%d \n", EncodeParameterLength );
    if ( NULL == m_MediaSource )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::SetEncodeConfig, device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    // later, we'd better let audio_source_filer or image_source_filter(or video_source_filter) abstract SetEncodeConfig interface
    GMI_RESULT Result = GMI_FAIL;
    uint32_t FinalMediaType = MAKE_MEDIA(m_MediaType,m_CodecType);
    switch( FinalMediaType )
    {
    case MEDIA_VIDEO_H264:
        Result = ((H264_VideoSourceFilter *)m_MediaSource)->SetEncodeConfig( EncodeParameter, EncodeParameterLength );
        break;

    case MEDIA_AUDIO_G711A:
        Result = ((G711A_AudioEncodeSourceFilter *)m_MediaSource)->SetEncodeConfig( EncodeParameter, EncodeParameterLength );
        break;

    case MEDIA_AUDIO_G711U:
        Result = ((G711U_AudioEncodeSourceFilter *)m_MediaSource)->SetEncodeConfig( EncodeParameter, EncodeParameterLength );
        break;

    default:
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::SetEncodeConfig, unsupported media type(%x), function return %x \n", FinalMediaType, (uint32_t) GMI_NOT_SUPPORT );
        return GMI_NOT_SUPPORT;
    }

    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::SetEncodeConfig, MediaSource::SetEncodeConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::SetEncodeConfig end and return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaEncodePipeline::GetOsdConfig( void_t *OsdParameter, size_t *OsdParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::GetOsdConfig begin, passed OsdParameterLength=%d \n", *OsdParameterLength );
    if ( NULL == m_MediaSource )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::GetOsdConfig, device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    // later, we'd better let image_source_filter(or video_source_filter) abstract GetOsdConfig interface
    GMI_RESULT Result = GMI_FAIL;
    uint32_t FinalMediaType = MAKE_MEDIA(m_MediaType,m_CodecType);
    switch( FinalMediaType )
    {
    case MEDIA_VIDEO_H264:
        Result = ((H264_VideoSourceFilter *)m_MediaSource)->GetOsdConfig( OsdParameter, OsdParameterLength );
        break;

    default:
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::GetOsdConfig, unsupported media type(%x), function return %x \n", FinalMediaType, (uint32_t) GMI_NOT_SUPPORT );
        return GMI_NOT_SUPPORT;
    }

    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::GetOsdConfig, MediaSource::GetOsdConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::GetOsdConfig end and return %x, returned EncodeParameterLength=%d \n", (uint32_t) Result, *OsdParameterLength );
    return Result;
}

GMI_RESULT MediaEncodePipeline::SetOsdConfig( const void_t *OsdParameter, size_t OsdParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::SetOsdConfig begin, passed OsdParameterLength=%d \n", OsdParameterLength );
    if ( NULL == m_MediaSource )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::SetOsdConfig, device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    // later, we'd better let image_source_filter(or video_source_filter) abstract SetOsdConfig interface
    GMI_RESULT Result = GMI_FAIL;
    uint32_t FinalMediaType = MAKE_MEDIA(m_MediaType,m_CodecType);
    switch( FinalMediaType )
    {
    case MEDIA_VIDEO_H264:
        Result = ((H264_VideoSourceFilter *)m_MediaSource)->SetOsdConfig( OsdParameter, OsdParameterLength );
        break;

    default:
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::SetOsdConfig, unsupported media type(%x), function return %x \n", FinalMediaType, (uint32_t) GMI_NOT_SUPPORT );
        return GMI_NOT_SUPPORT;
    }

    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::SetOsdConfig, MediaSource::SetOsdConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::SetOsdConfig end and return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaEncodePipeline::ForceGenerateIdrFrame()
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::ForceGenerateIdrFrame begin\n" );
    if ( NULL == m_MediaSource )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::ForceGenerateIdrFrame, device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    // later, we'd better let image_source_filter(or video_source_filter) abstract ForceGenerateIdrFrame interface
    GMI_RESULT Result = GMI_FAIL;
    uint32_t FinalMediaType = MAKE_MEDIA(m_MediaType,m_CodecType);
    switch( FinalMediaType )
    {
    case MEDIA_VIDEO_H264:
        Result = ((H264_VideoSourceFilter *)m_MediaSource)->ForceGenerateIdrFrame();
        break;

    default:
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::ForceGenerateIdrFrame, unsupported media type(%x), function return %x \n", FinalMediaType, (uint32_t) GMI_NOT_SUPPORT );
        return GMI_NOT_SUPPORT;
    }

    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaEncodePipeline::ForceGenerateIdrFrame, MediaSource::ForceGenerateIdrFrame failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaEncodePipeline::ForceGenerateIdrFrame end and return %x \n", (uint32_t) Result );
    return Result;
}
