#include "media_decode_pipeline.h"

#include "base_streaming_pipeline_manager.h"
#include "g711a_audio_decode_handler_filter.h"
#include "g711u_audio_decode_handler_filter.h"
#include "ipc_media_data_dispatch_source.h"
#include "media_codec_parameter.h"
#include "share_memory_log_client.h"

MediaDecodePipeline::MediaDecodePipeline(void)
    : m_StreamingPipelineManager( NULL )
    , m_SourceId( 0 )
    , m_MediaId( 0 )
    , m_MediaType( 0 )
    , m_CodecType( 0 )
    , m_MediaDataDispatchSource( NULL )
    , m_MediaHandler( NULL )
{
}

MediaDecodePipeline::~MediaDecodePipeline(void)
{
}

GMI_RESULT MediaDecodePipeline::Initialize( uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, void_t *DecodeParameter, size_t DecodeParameterLength )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaDecodePipeline::Initialize begin, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d, DecodeParameterLength=%d \n",
               SourceId, MediaId, MediaType, CodecType, DecodeParameterLength );

    MediaSourceParameter Parameter;
    Parameter.s_SourceId         = SourceId;
    Parameter.s_MediaId          = MediaId;
    Parameter.s_MediaType        = MediaType;
    Parameter.s_CodecType        = CodecType;
    Parameter.s_BitRate          = 0;
    Parameter.s_FrameRate        = 0;
    Parameter.s_InternalParamter = DecodeParameter;

    GMI_RESULT Result = GMI_FAIL;
    switch ( MAKE_MEDIA(MediaType,CodecType) )
    {
    case MEDIA_AUDIO_G711A:
        m_MediaHandler = G711A_AudioDecodeHandlerFilter::CreateNew();
        if ( NULL == m_MediaHandler )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Initialize, G711A_AudioDecodeHandlerFilter object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
            return GMI_OUT_OF_MEMORY;
        }

        Parameter.s_BitRate   = (( AudioDecParam * )DecodeParameter)->s_BitRate;
        Parameter.s_FrameRate = (( AudioDecParam * )DecodeParameter)->s_FrameRate;

        if ( 0 == Parameter.s_BitRate || 0 == Parameter.s_FrameRate )
        {
            m_MediaHandler->ReleaseRef();
            m_MediaHandler = NULL;
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Initialize, G711A_AudioDecodeHandler parameter error, function return %x \n", (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }

        Result = m_MediaHandler->Initialize( (SourceId<<24)+(MediaId<<16)+(MediaType<<8)+CodecType, "G711A_AudioHandler", 19, &Parameter, sizeof(Parameter) );
        if ( FAILED( Result ) )
        {
            m_MediaHandler->ReleaseRef();
            m_MediaHandler = NULL;
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Initialize, G711A_AudioDecodeHandlerFilter initialization failed, function return %x \n", (uint32_t) Result );
            return Result;
        }
        break;

    case MEDIA_AUDIO_G711U:
        m_MediaHandler = G711U_AudioDecodeHandlerFilter::CreateNew();
        if ( NULL == m_MediaHandler )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Initialize, G711U_AudioDecodeHandlerFilter object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
            return GMI_OUT_OF_MEMORY;
        }

        Parameter.s_BitRate   = (( AudioDecParam * )DecodeParameter)->s_BitRate;
        Parameter.s_FrameRate = (( AudioDecParam * )DecodeParameter)->s_FrameRate;

        if ( 0 == Parameter.s_BitRate || 0 == Parameter.s_FrameRate )
        {
            m_MediaHandler->ReleaseRef();
            m_MediaHandler = NULL;
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Initialize, G711U_AudioDecodeHandler parameter error, function return %x \n", (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }

        Result = m_MediaHandler->Initialize( (SourceId<<24)+(MediaId<<16)+(MediaType<<8)+CodecType, "G711U_AudioHandler", 19, &Parameter, sizeof(Parameter) );
        if ( FAILED( Result ) )
        {
            m_MediaHandler->ReleaseRef();
            m_MediaHandler = NULL;
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Initialize, G711U_AudioDecodeHandlerFilter initialization failed, function return %x \n", (uint32_t) Result );
            return Result;
        }
        break;

    default:
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Initialize, unsupported media type(%x), function return %x \n", MAKE_MEDIA(MediaType,CodecType), (uint32_t) GMI_NOT_SUPPORT );
        return GMI_NOT_SUPPORT;
    }

    m_MediaDataDispatchSource = IpcMediaDataDispatchSource::CreateNew();
    if ( NULL == m_MediaDataDispatchSource )
    {
        m_MediaHandler->Deinitialize();
        m_MediaHandler->ReleaseRef();
        m_MediaHandler = NULL;
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Initialize, IpcMediaDataDispatchSource object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    Result = m_MediaDataDispatchSource->Initialize( (SourceId<<24)+(MediaId<<16)+(MediaType<<8)+CodecType, "MediaDataDispatch", 17, &Parameter, sizeof(Parameter) );
    if ( FAILED( Result ) )
    {
        m_MediaDataDispatchSource->ReleaseRef();
        m_MediaDataDispatchSource = NULL;
        m_MediaHandler->Deinitialize();
        m_MediaHandler->ReleaseRef();
        m_MediaHandler = NULL;
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Initialize, MediaDataDispatchSource object initializaztion failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    m_StreamingPipelineManager = BaseStreamingPipelineManager::CreateNew( MFM_Push );
    if ( NULL == m_StreamingPipelineManager )
    {
        m_MediaDataDispatchSource->Deinitialize();
        m_MediaDataDispatchSource->ReleaseRef();
        m_MediaDataDispatchSource = NULL;
        m_MediaHandler->Deinitialize();
        m_MediaHandler->ReleaseRef();
        m_MediaHandler = NULL;
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Initialize, BaseStreamingPipelineManager object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    Result = m_StreamingPipelineManager->Initialize();
    if ( FAILED( Result ) )
    {
        m_StreamingPipelineManager->ReleaseRef();
        m_StreamingPipelineManager = NULL;
        m_MediaDataDispatchSource->Deinitialize();
        m_MediaDataDispatchSource->ReleaseRef();
        m_MediaDataDispatchSource = NULL;
        m_MediaHandler->Deinitialize();
        m_MediaHandler->ReleaseRef();
        m_MediaHandler = NULL;
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Initialize, BaseStreamingPipelineManager object initializaztion failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_StreamingPipelineManager->AddFilter( m_MediaDataDispatchSource );
    if ( FAILED( Result ) )
    {
        m_StreamingPipelineManager->Deinitialize();
        m_StreamingPipelineManager->ReleaseRef();
        m_StreamingPipelineManager = NULL;
        m_MediaDataDispatchSource->Deinitialize();
        m_MediaDataDispatchSource->ReleaseRef();
        m_MediaDataDispatchSource = NULL;
        m_MediaHandler->Deinitialize();
        m_MediaHandler->ReleaseRef();
        m_MediaHandler = NULL;
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Initialize, BaseStreamingPipelineManager add source filter failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_StreamingPipelineManager->AddFilter( m_MediaHandler );
    if ( FAILED( Result ) )
    {
        m_StreamingPipelineManager->Deinitialize();
        m_StreamingPipelineManager->ReleaseRef();
        m_StreamingPipelineManager = NULL;
        m_MediaDataDispatchSource->Deinitialize();
        m_MediaDataDispatchSource->ReleaseRef();
        m_MediaDataDispatchSource = NULL;
        m_MediaHandler->Deinitialize();
        m_MediaHandler->ReleaseRef();
        m_MediaHandler = NULL;
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Initialize, BaseStreamingPipelineManager add handller filter failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = BaseStreamingPipelineManager::ConnectFilter( m_MediaDataDispatchSource, 0, m_MediaHandler, 0 );
    if ( FAILED( Result ) )
    {
        m_StreamingPipelineManager->Deinitialize();
        m_StreamingPipelineManager->ReleaseRef();
        m_StreamingPipelineManager = NULL;
        m_MediaDataDispatchSource->Deinitialize();
        m_MediaDataDispatchSource->ReleaseRef();
        m_MediaDataDispatchSource = NULL;
        m_MediaHandler->Deinitialize();
        m_MediaHandler->ReleaseRef();
        m_MediaHandler = NULL;
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Initialize, BaseStreamingPipelineManager ConnectFilter failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    m_SourceId  = SourceId;
    m_MediaId   = MediaId;
    m_MediaType = MediaType;
    m_CodecType = CodecType;

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaDecodePipeline::Initialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT MediaDecodePipeline::Deinitialize()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaDecodePipeline::Deinitialize begin \n" );
    Stop();

    GMI_RESULT Result = BaseStreamingPipelineManager::DisconnectFilter( m_MediaDataDispatchSource, 0, m_MediaHandler, 0 );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Deinitialize, BaseStreamingPipelineManager DisconnectFilter failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    if ( NULL != m_MediaDataDispatchSource )
    {
        Result = m_MediaDataDispatchSource->Deinitialize();
        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Deinitialize, MediaSource Deinitialize failed, function return %x \n", (uint32_t) Result );
            return Result;
        }

        m_MediaDataDispatchSource->ReleaseRef();
        m_MediaDataDispatchSource = NULL;
    }

    if ( NULL != m_MediaHandler )
    {
        Result = m_MediaHandler->Deinitialize();
        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Deinitialize, MediaDataDispatchHandler Deinitialize failed, function return %x \n", (uint32_t) Result );
            return Result;
        }

        m_MediaHandler->ReleaseRef();
        m_MediaHandler = NULL;
    }

    if ( NULL != m_StreamingPipelineManager )
    {
        Result = m_StreamingPipelineManager->Deinitialize();
        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Deinitialize, StreamingPipelineManager Deinitialize failed, function return %x \n", (uint32_t) Result );
            return Result;
        }

        m_StreamingPipelineManager->ReleaseRef();
        m_StreamingPipelineManager = NULL;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaDecodePipeline::Deinitialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT MediaDecodePipeline::Play()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaDecodePipeline::Play begin \n" );
    if ( NULL == m_StreamingPipelineManager )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Play, device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = m_StreamingPipelineManager->Play();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Play, StreamingPipelineManager->Play failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaDecodePipeline::Play end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaDecodePipeline::Stop()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaDecodePipeline::Stop begin \n" );
    if ( NULL == m_StreamingPipelineManager )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Stop, device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = m_StreamingPipelineManager->Stop();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::Stop, StreamingPipelineManager->Stop failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaDecodePipeline::Stop end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaDecodePipeline::GetDecodeConfig( void_t *DecodeParameter, size_t *DecodeParameterLength )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaDecodePipeline::GetDecodeConfig begin, passed DecodeParameterLength=%d \n", *DecodeParameterLength );
    if ( NULL == m_MediaHandler )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::GetDecodeConfig, device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    // later, we'd better let audio_source_filer or image_source_filter(or video_source_filter) abstract GetEncodeConfig interface
    GMI_RESULT Result = GMI_FAIL;
    uint32_t FinalMediaType = MAKE_MEDIA(m_MediaType,m_CodecType);
    switch( FinalMediaType )
    {
    case MEDIA_AUDIO_G711A:
        Result = ((G711A_AudioDecodeHandlerFilter *)m_MediaHandler)->GetDecodeConfig( DecodeParameter, DecodeParameterLength );
        break;

    case MEDIA_AUDIO_G711U:
        Result = ((G711U_AudioDecodeHandlerFilter *)m_MediaHandler)->GetDecodeConfig( DecodeParameter, DecodeParameterLength );
        break;

    default:
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::GetDecodeConfig, unsupported media type(%x), function return %x \n", FinalMediaType, (uint32_t) GMI_NOT_SUPPORT );
        return GMI_NOT_SUPPORT;
    }

    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::GetDecodeConfig, MediaSource::GetEncodeConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaDecodePipeline::GetDecodeConfig end and return %x, returned DecodeParameterLength=%d \n", (uint32_t) Result, *DecodeParameterLength );
    return Result;
}

GMI_RESULT MediaDecodePipeline::SetDecodeConfig( const void_t *DecodeParameter, size_t DecodeParameterLength )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaDecodePipeline::SetDecodeConfig begin, passed DecodeParameterLength=%d \n", DecodeParameterLength );
    if ( NULL == m_MediaHandler )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::SetDecodeConfig, device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    // later, we'd better let audio_source_filer or image_source_filter(or video_source_filter) abstract SetEncodeConfig interface
    GMI_RESULT Result = GMI_FAIL;
    uint32_t FinalMediaType = MAKE_MEDIA(m_MediaType,m_CodecType);
    switch( FinalMediaType )
    {
    case MEDIA_AUDIO_G711A:
        Result = ((G711A_AudioDecodeHandlerFilter *)m_MediaHandler)->SetDecodeConfig( DecodeParameter, DecodeParameterLength );
        break;

    case MEDIA_AUDIO_G711U:
        Result = ((G711U_AudioDecodeHandlerFilter *)m_MediaHandler)->SetDecodeConfig( DecodeParameter, DecodeParameterLength );
        break;

    default:
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::SetDecodeConfig, unsupported media type(%x), function return %x \n", FinalMediaType, (uint32_t) GMI_NOT_SUPPORT );
        return GMI_NOT_SUPPORT;
    }

    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaDecodePipeline::SetDecodeConfig, MediaSource::SetEncodeConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaDecodePipeline::SetDecodeConfig end and return %x \n", (uint32_t) Result );
    return Result;
}
