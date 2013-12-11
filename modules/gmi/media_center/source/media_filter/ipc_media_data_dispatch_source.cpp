#include "ipc_media_data_dispatch_source.h"

#include "gmi_config_api.h"
#include "gmi_media_ctrl.h"
#include "ipc_fw_v3.x_resource.h"
#include "ipc_fw_v3.x_setting.h"
#include "ipc_media_data_dispatch.h"
#include "media_codec_parameter.h"
#include "share_memory_log_client.h"

IpcMediaDataDispatchSource::IpcMediaDataDispatchSource(void)
    : m_MediaDataClient()
    , m_Buffer( NULL )
    , m_BufferSize( 0 )
    , m_DataPool( 0 )
    , m_FetchThread()
    , m_ThreadWorking( false )
    , m_ThreadExitFlag( false )
    , m_SourceId( 0 )
    , m_MediaId( 0 )
    , m_MediaType( 0 )
    , m_CodecType( 0 )
    , m_FrameRate( 0 )
    , m_DecodeSourceMonitorEnable( GMI_DECODE_SOURCE_MONITOR_CONFIG_ENABLE_VALUE )
    , m_DecodeSourceFrameCheckInterval( GMI_DECODE_SOURCE_MONITOR_CONFIG_FRAME_CHECK_INTERVAL_VALUE )
    , m_FrameNumber( 0 )
{
    m_FirstFrameTime.tv_sec  = 0;
    m_FirstFrameTime.tv_usec = 0;
}

IpcMediaDataDispatchSource::~IpcMediaDataDispatchSource(void)
{
}

IpcMediaDataDispatchSource*  IpcMediaDataDispatchSource::CreateNew()
{
    return BaseMemoryManager::Instance().New<IpcMediaDataDispatchSource>();
}

GMI_RESULT IpcMediaDataDispatchSource::Initialize( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::Initialize begin \n" );

    GMI_RESULT Result = GetDecodeSourceMonitorEnableConfig( &m_DecodeSourceMonitorEnable );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Initialize, GetDecodeSourceMonitorEnableConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    if ( m_DecodeSourceMonitorEnable )
    {
        Result = GetDecodeSourceFrameCheckInterval( &m_DecodeSourceFrameCheckInterval );
        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Initialize, GetVideoFrameCheckInterval failed, function return %x \n", (uint32_t) Result );
            return Result;
        }
    }

    MediaSourceParameter *SourceParameter = reinterpret_cast<MediaSourceParameter *> (Argument);

    Result = StreamingMediaSource::Initialize( FilterId, FilterName, FilterNameLength, Argument, ArgumentSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Initialize, StreamingMediaSource::Initialize failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    size_t   MemorySize = 0;
    size_t   MaxDataBlockSize = 0;
    size_t   MinDataBlockSize = 0;

    MemorySize = (SourceParameter->s_BitRate*1024/8);//use 1024, instead of 1000, consider memory buffer overhead usage.

    MaxDataBlockSize = MemorySize;
    MinDataBlockSize = MemorySize/4;

    if ( MEDIA_FILTER_MEMORY_POOL_MINIMUM_MEMORY_BLOCK_NUMBER < SourceParameter->s_FrameRate )
    {
        MemorySize *= SourceParameter->s_FrameRate;
    }
    else
    {
        MemorySize *= MEDIA_FILTER_MEMORY_POOL_MINIMUM_MEMORY_BLOCK_NUMBER;
    }
    MemorySize /= 4;//250ms buffer

    m_FrameBufferSize = MaxDataBlockSize;
    m_FrameBuffer = BaseMemoryManager::Instance().News<uint8_t>( m_FrameBufferSize );
    if ( NULL == m_FrameBuffer )
    {
        StreamingMediaSource::Deinitialize();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Initialize, memory used by frame allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    m_ExtraDataSize = MinDataBlockSize;
    m_ExtraData = BaseMemoryManager::Instance().News<uint8_t>( m_ExtraDataSize );
    if ( NULL == m_ExtraData )
    {
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_FrameBuffer );
        m_FrameBuffer = NULL;
        StreamingMediaSource::Deinitialize();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Initialize, memory used by extra data allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    // buffer
    m_Buffer = BaseMemoryManager::Instance().News<uint8_t>( MemorySize );
    if ( NULL == m_Buffer )
    {
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_ExtraData );
        m_ExtraData = NULL;
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_FrameBuffer );
        m_FrameBuffer = NULL;
        StreamingMediaSource::Deinitialize();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Initialize, memory used by memory pool allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    // memory pool
    m_DataPool = BaseMemoryManager::Instance().New<MemoryPoolParallelConsumers>();
    if ( NULL == m_DataPool )
    {
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_Buffer );
        m_Buffer = NULL;
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_ExtraData );
        m_ExtraData = NULL;
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_FrameBuffer );
        m_FrameBuffer = NULL;
        StreamingMediaSource::Deinitialize();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Initialize, memory pool object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    uint32_t ApplicationMediaType = MAKE_MEDIA(SourceParameter->s_MediaType, SourceParameter->s_CodecType);
    uint32_t ApplicationMediaId = (SourceParameter->s_SourceId<<8)+SourceParameter->s_MediaId;
    Result = m_DataPool->Initialize( ApplicationMediaType, ApplicationMediaId, m_Buffer, MemorySize, MaxDataBlockSize, MinDataBlockSize );
    if ( FAILED( Result ) )
    {
        BaseMemoryManager::Instance().Delete( m_DataPool );
        m_DataPool = NULL;
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_Buffer );
        m_Buffer = NULL;
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_ExtraData );
        m_ExtraData = NULL;
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_FrameBuffer );
        m_FrameBuffer = NULL;
        StreamingMediaSource::Deinitialize();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Initialize, memory pool initialization failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    // output pin
    StreamingMediaOutputPin *OutputPin = StreamingMediaOutputPin::CreateNew( false ,this, 0 );
    if ( NULL == OutputPin )
    {
        m_DataPool->Deinitialize();
        BaseMemoryManager::Instance().Delete( m_DataPool );
        m_DataPool = NULL;
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_Buffer );
        m_Buffer = NULL;
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_ExtraData );
        m_ExtraData = NULL;
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_FrameBuffer );
        m_FrameBuffer = NULL;
        StreamingMediaSource::Deinitialize();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Initialize, output pin object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    Result = OutputPin->Initialize( ApplicationMediaType, ApplicationMediaId, m_DataPool );
    if ( FAILED( Result ) )
    {
        OutputPin->ReleaseRef();
        m_DataPool->Deinitialize();
        BaseMemoryManager::Instance().Delete( m_DataPool );
        m_DataPool = NULL;
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_Buffer );
        m_Buffer = NULL;
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_ExtraData );
        m_ExtraData = NULL;
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_FrameBuffer );
        m_FrameBuffer = NULL;
        StreamingMediaSource::Deinitialize();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Initialize, output pin initialization failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    m_SourceId        = SourceParameter->s_SourceId;
    m_MediaId         = SourceParameter->s_MediaId;
    m_MediaType       = SourceParameter->s_MediaType;
    m_CodecType       = SourceParameter->s_CodecType;
    m_FrameRate       = SourceParameter->s_FrameRate;

    m_Outputs.push_back( OutputPin );

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::Initialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT IpcMediaDataDispatchSource::Deinitialize()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::Deinitialize begin \n" );
    if ( MFS_Uninit == Status() )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Warning, "IpcMediaDataDispatchSource::Deinitialize, IpcMediaDataDispatchSource is already deinitialized, funtion return %x \n", (uint32_t) GMI_SUCCESS );
        return GMI_SUCCESS;
    }

    GMI_RESULT Result = StreamingMediaSource::Deinitialize();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Warning, "IpcMediaDataDispatchSource::Deinitialize, StreamingMediaSource::Deinitialize failed, funtion return %x \n", (uint32_t) Result );
        return Result;
    }

    if ( NULL != m_DataPool )
    {
        Result = m_DataPool->Deinitialize();
        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Deinitialize, memory pool deinitialize failed, function return %x \n", (uint32_t) Result );
            return Result;
        }
    }

    BaseMemoryManager::Instance().Delete( m_DataPool );
    m_DataPool = NULL;

    BaseMemoryManager::Instance().Deletes( (uint8_t *)m_Buffer );
    m_Buffer = NULL;

    BaseMemoryManager::Instance().Deletes( (uint8_t *)m_ExtraData );
    m_ExtraData = NULL;

    BaseMemoryManager::Instance().Deletes( (uint8_t *)m_FrameBuffer );
    m_FrameBuffer = NULL;

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::Deinitialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT IpcMediaDataDispatchSource::Play()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::Play begin \n" );

    GMI_RESULT Result = StreamingMediaSource::Play();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Play, GetClientUDPPort failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    uint16_t Client_UDP_Port = GMI_STREAMING_MEDIA_SERVER_DECODE_AUDIO1;
    Result = GetClientUDPPort( true, m_SourceId, m_MediaId, m_MediaType, m_CodecType, &Client_UDP_Port );
    if ( FAILED( Result ) )
    {
        StreamingMediaSource::Stop();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Play, GetClientUDPPort failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    // for now, GM_STREAM_APPLICATION_ID is being used
    uint16_t Server_UDP_Port = GMI_STREAMING_MEDIA_SDK_DECODE_AUDIO1;
    Result = GetServerUDPPort( true, m_SourceId, GM_STREAM_APPLICATION_ID, m_MediaType, m_CodecType, &Server_UDP_Port );
    if ( FAILED( Result ) )
    {
        StreamingMediaSource::Stop();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Play, GetServerUDPPort failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_MediaDataClient.Initialize( Client_UDP_Port, GM_STREAM_APPLICATION_ID );
    if ( FAILED( Result ) )
    {
        StreamingMediaSource::Stop();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Play, MediaDataClient.Initialize failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    uint32_t ApplicationMediaType = MAKE_MEDIA(m_MediaType, m_CodecType);
    uint32_t ApplicationMediaId = (m_SourceId<<8)+m_MediaId;
    Result = m_MediaDataClient.Register( Server_UDP_Port, ApplicationMediaType, ApplicationMediaId, true, NULL, 0 );
    if ( FAILED( Result ) )
    {
        m_MediaDataClient.Deinitialize();
        StreamingMediaSource::Stop();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Play, MediaDataClient.Register failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    m_ThreadWorking = false;
    m_ThreadExitFlag = false;

    Result = m_FetchThread.Create( NULL, 0, IpcMediaDataDispatchSource::FetchThread, this );
    if ( FAILED( Result ) )
    {
        m_MediaDataClient.Unregister();
        m_MediaDataClient.Deinitialize();
        StreamingMediaSource::Stop();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Play, FetchThread.Create failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    gettimeofday1( &m_FirstFrameTime, NULL );

    Result = m_FetchThread.Start();
    if ( FAILED( Result ) )
    {
        m_FetchThread.Destroy();
        m_MediaDataClient.Unregister();
        m_MediaDataClient.Deinitialize();
        StreamingMediaSource::Stop();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::Play, FetchThread.Start failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::Play end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT IpcMediaDataDispatchSource::Stop()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::Stop begin \n" );

    GMI_RESULT Result = StreamingMediaSource::Stop();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Warning, "IpcMediaDataDispatchSource::Stop, StreamingMediaSource::Stop failed, funtion return %x \n", (uint32_t) Result );
        return Result;
    }

    m_ThreadExitFlag = true;
    while ( m_ThreadWorking ) GMI_Sleep( 10 );
    m_FetchThread.Destroy();

    Result = m_MediaDataClient.Unregister();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Warning, "IpcMediaDataDispatchSource::Stop, MediaDataClient::Unregister failed, funtion return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_MediaDataClient.Deinitialize();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Warning, "IpcMediaDataDispatchSource::Stop, MediaDataClient::Deinitialize failed, funtion return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::Stop end, function return %x \n", (uint32_t) Result );
    return Result;
}

void_t* IpcMediaDataDispatchSource::FetchThread( void_t *Argument )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::FetchThread begin, Argument=%p \n", Argument );
    IpcMediaDataDispatchSource *Fetcher = reinterpret_cast<IpcMediaDataDispatchSource*> ( Argument );
    void_t *Return = Fetcher->FetchEntry();
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::FetchThread end, Return=%p \n", Return );
    return Return;
}

void_t* IpcMediaDataDispatchSource::FetchEntry()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::FetchEntry begin \n" );
    m_ThreadWorking        = true;

    GMI_RESULT Result      = GMI_FAIL;
    size_t FrameBufferSize = 0;
    struct timeval FrameTS;
    size_t ExtraDataSize   = 0;

    while( !m_ThreadExitFlag )
    {
        FrameBufferSize = m_FrameBufferSize;
        ExtraDataSize   = m_ExtraDataSize;

        Result = m_MediaDataClient.Read( m_FrameBuffer, &FrameBufferSize, &FrameTS, m_ExtraData, &ExtraDataSize, UDP_SESSION_IO_WAITTIME*2 );
        if ( SUCCEEDED( Result ) )
        {
            if ( m_DecodeSourceMonitorEnable )
            {
                ++m_FrameNumber;
                // the following code facilitate unitest check execution, uncomment it to enable print info
                //printf( "IpcMediaDataDispatchSource::FetchEntry, FrameNumber=%lld, DecodeSourceFrameCheckInterval=%d \n", m_FrameNumber, m_DecodeSourceFrameCheckInterval );

                if ( 0 == m_FrameNumber%m_DecodeSourceFrameCheckInterval )
                {
                    struct timeval CurrentTime;
                    gettimeofday1( &CurrentTime, NULL );

                    uint64_t Duration = (CurrentTime.tv_sec - m_FirstFrameTime.tv_sec)*1000 + CurrentTime.tv_usec/1000 - m_FirstFrameTime.tv_usec/1000;

                    double FrameRate = 1000.0f * m_FrameNumber / Duration;

                    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Loop, "IpcMediaDataDispatchSource::FetchEntry, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d, FrameNumber=%lld, FrameRate=%f, second=%d:millisecond=%d \n",
                               GetSourceId(), GetMediaId(), GetMediaType(), GetCodecType(), m_FrameNumber, FrameRate, (int32_t) FrameTS.tv_sec, (int32_t) FrameTS.tv_usec );
                    // the following code facilitate unitest check execution, uncomment it to enable print info
                    //printf( "IpcMediaDataDispatchSource::FetchEntry, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d, FrameNumber=%lld, FrameRate=%f, second=%d:millisecond=%d \n",
                    //         GetSourceId(), GetMediaId(), GetMediaType(), GetCodecType(), m_FrameNumber, FrameRate, (int32_t) FrameTS.tv_sec, (int32_t) FrameTS.tv_usec );
                }
            }

            Result = m_Outputs[0]->Receive( (const uint8_t *) m_FrameBuffer, FrameBufferSize, &FrameTS, m_ExtraData, ExtraDataSize );
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::FetchEntry, output pin Receive failed and return %x , SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d, FrameBufferSize=%d, second=%d:millisecond=%d, ExtraDataSize=%d\n",
                           (uint32_t)Result, m_SourceId, m_MediaId, m_MediaType, m_CodecType, FrameBufferSize,(int32_t) FrameTS.tv_sec, (int32_t) FrameTS.tv_usec, ExtraDataSize );
            }
        }
        else
        {
            if ( GMI_TRY_AGAIN_ERROR != Result )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::FetchEntry, MediaDataClient::Read return %x \n", (uint32_t)Result );
                break;
            }
        }
    }

    m_ThreadWorking = false;
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::FetchEntry end, function return %x \n", (uint32_t) Result );
    return (void_t *) size_t(Result);
}

GMI_RESULT IpcMediaDataDispatchSource::GetServerUDPPort( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, uint16_t *UDP_Port )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::GetServerUDPPort begin, EncodeMode=%d, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d \n",
               EncodeMode, SourceId, MediaId, MediaType, CodecType );

    const char_t *ConfigPath = NULL;
    const char_t *KeyName = NULL;
    int32_t Default_UDP_Port = 0;
    int32_t Local_UDP_Port = 0;

    switch ( MediaType )
    {
    case MEDIA_AUDIO:
    {
        switch ( SourceId )
        {
        case 0:
            break;
        default:
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetServerUDPPort, Audio source id(%d) is not supported, function return %x \n", SourceId, (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }

        switch ( MediaId )
        {
        case GM_STREAM_APPLICATION_ID:
            ConfigPath = SDK_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH;
            KeyName = SDK_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SERVER_UDP_PORT;
            Default_UDP_Port = GMI_STREAMING_MEDIA_SDK_DECODE_AUDIO1;
            break;
        case ONVIF_STREAM_APPLICATION_ID:
            ConfigPath = ONVIF_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH;
            KeyName = ONVIF_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SERVER_UDP_PORT;
            Default_UDP_Port = GMI_STREAMING_MEDIA_ONVIF_DECODE_AUDIO1;
            break;
        case GB_28181_STREAM_APPLICATION_ID:
            ConfigPath = GB_MEDIA_CLIENT_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH;
            KeyName = GB_MEDIA_CLIENT_IPC_MEDIA_DATA_DISPATCH_CONFIG_SERVER_UDP_PORT;
            Default_UDP_Port = GMI_STREAMING_MEDIA_GB28181_DECODE_AUDIO1;
            break;
        default:
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetServerUDPPort, Audio media id(%d) is not supported, function return %x \n", MediaId, (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }
    }
    break;
    default:
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetServerUDPPort, media type(%d) is not supported, function return %x \n", MediaType, (uint32_t) GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetServerUDPPort, GMI_XmlOpen failed, function return %x \n", MediaType, (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::GetServerUDPPort, Default_UDP_Port=%d \n", Default_UDP_Port );

    Result = GMI_XmlRead(Handle, ConfigPath, KeyName, Default_UDP_Port, &Local_UDP_Port, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetServerUDPPort, GMI_XmlRead failed, function return %x \n", MediaType, (uint32_t) Result );
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetServerUDPPort, GMI_XmlFileSave failed, function return %x \n", MediaType, (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::GetServerUDPPort, Default_UDP_Port=%d, Local_UDP_Port=%d \n", Default_UDP_Port, Local_UDP_Port );
#endif

    *UDP_Port = (uint16_t) Local_UDP_Port;
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::GetServerUDPPort end, function return %x, EncodeMode=%d, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d \n",
               GMI_SUCCESS, EncodeMode, SourceId, MediaId, MediaType, CodecType );
    return GMI_SUCCESS;
}

GMI_RESULT IpcMediaDataDispatchSource::GetClientUDPPort( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, uint16_t *UDP_Port )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::GetClientUDPPort begin, EncodeMode=%d, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d \n",
               EncodeMode, SourceId, MediaId, MediaType, CodecType );

    const char_t *ConfigPath = NULL;
    const char_t *KeyName = NULL;
    int32_t Default_UDP_Port = 0;
    int32_t Local_UDP_Port = 0;

    switch ( MediaType )
    {
    case MEDIA_AUDIO:
    {
        switch ( SourceId )
        {
        case 0:
            break;
        default:
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetClientUDPPort, Audio source id(%d) is not supported, function return %x \n", SourceId, (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }

        switch ( MediaId )
        {
        case 0:
            ConfigPath = MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH;
            KeyName = MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_CLIENT_UDP_PORT;
            Default_UDP_Port = GMI_STREAMING_MEDIA_SERVER_DECODE_AUDIO1;
            break;
        default:
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetClientUDPPort, Audio media id(%d) is not supported, function return %x \n", MediaId, (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }
    }
    break;
    default:
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetClientUDPPort, media type(%d) is not supported, function return %x \n", MediaType, (uint32_t) GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetClientUDPPort, GMI_XmlOpen failed, function return %x \n", MediaType, (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::GetClientUDPPort, Default_UDP_Port=%d \n", Default_UDP_Port );

    Result = GMI_XmlRead(Handle, ConfigPath, KeyName, Default_UDP_Port, &Local_UDP_Port, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetClientUDPPort, GMI_XmlRead failed, function return %x \n", MediaType, (uint32_t) Result );
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetClientUDPPort, GMI_XmlFileSave failed, function return %x \n", MediaType, (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::GetClientUDPPort, Default_UDP_Port=%d, Local_UDP_Port=%d \n", Default_UDP_Port, Local_UDP_Port );
#endif

    *UDP_Port = (uint16_t) Local_UDP_Port;
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::GetClientUDPPort end, function return %x, EncodeMode=%d, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d \n",
               GMI_SUCCESS, EncodeMode, SourceId, MediaId, MediaType, CodecType );
    return GMI_SUCCESS;
}

GMI_RESULT IpcMediaDataDispatchSource::GetDecodeSourceMonitorEnableConfig( boolean_t *Enable )
{
    int32_t DefaultEnable = GMI_DECODE_SOURCE_MONITOR_CONFIG_ENABLE_VALUE;
    int32_t IntEnable = GMI_DECODE_SOURCE_MONITOR_CONFIG_ENABLE_VALUE;

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetDecodeSourceMonitorEnableConfig, GMI_XmlOpen failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::GetDecodeSourceMonitorEnableConfig, DefaultEnable=%d \n", DefaultEnable );

    Result = GMI_XmlRead(Handle, GMI_DECODE_SOURCE_MONITOR_CONFIG_PATH, GMI_DECODE_SOURCE_MONITOR_CONFIG_ENABLE_KEY_NAME, DefaultEnable, &IntEnable, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetDecodeSourceMonitorEnableConfig, GMI_XmlRead failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetDecodeSourceMonitorEnableConfig, GMI_XmlFileSave failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::GetDecodeSourceMonitorEnableConfig, DefaultEnable=%d, IntEnable=%d \n", DefaultEnable, IntEnable );
#endif

    // the following code facilitate unitest check execution, uncomment it to enable print info
    //printf( "IpcMediaDataDispatchSource::GetDecodeSourceMonitorEnableConfig, DefaultEnable=%d, IntEnable=%d \n", DefaultEnable, IntEnable );

    *Enable = (IntEnable == 0) ? false : true;
    return GMI_SUCCESS;
}

GMI_RESULT IpcMediaDataDispatchSource::GetDecodeSourceFrameCheckInterval( uint32_t *FrameNumber )
{
    int32_t DefaultFrameCheckInterval = GMI_DECODE_SOURCE_MONITOR_CONFIG_FRAME_CHECK_INTERVAL_VALUE;
    int32_t FrameCheckInterval = GMI_DECODE_SOURCE_MONITOR_CONFIG_FRAME_CHECK_INTERVAL_VALUE;

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetDecodeSourceFrameCheckInterval, GMI_XmlOpen failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::GetDecodeSourceFrameCheckInterval, DefaultFrameCheckInterval=%d \n", DefaultFrameCheckInterval );

    Result = GMI_XmlRead(Handle, GMI_DECODE_SOURCE_MONITOR_CONFIG_PATH, GMI_DECODE_SOURCE_MONITOR_CONFIG_FRAME_CHECK_INTERVAL_KEY_NAME, DefaultFrameCheckInterval, &FrameCheckInterval, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetDecodeSourceFrameCheckInterval, GMI_XmlRead failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchSource::GetDecodeSourceFrameCheckInterval, GMI_XmlFileSave failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchSource::GetDecodeSourceFrameCheckInterval, DefaultFrameCheckInterval=%d, FrameCheckInterval=%d \n", DefaultFrameCheckInterval, FrameCheckInterval );
#endif

    // the following code facilitate unitest check execution, uncomment it to enable print info
    //printf( "IpcMediaDataDispatchSource::GetDecodeSourceFrameCheckInterval, DefaultFrameCheckInterval=%d, FrameCheckInterval=%d \n", DefaultFrameCheckInterval, FrameCheckInterval );

    *FrameNumber = FrameCheckInterval;
    return GMI_SUCCESS;
}
