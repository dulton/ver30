#include "ipc_media_data_dispatch_handler.h"

#include "gmi_media_ctrl.h"
#include "ipc_fw_v3.x_resource.h"
#include "media_codec_parameter.h"
#include "share_memory_log_client.h"
#if defined( __linux__ )
#include "sys_info_readonly.h"
#endif

IpcMediaDataDispatchHandler::IpcMediaDataDispatchHandler(void)
    : m_MediaDataServer()
    , m_SourceId( 0 )
    , m_MediaId( 0 )
    , m_MediaType( 0 )
    , m_CodecType( 0 )
    , m_BitRate( 0 )
    , m_FrameRate( 0 )
{
}

IpcMediaDataDispatchHandler::~IpcMediaDataDispatchHandler(void)
{
}

IpcMediaDataDispatchHandler*  IpcMediaDataDispatchHandler::CreateNew()
{
    return BaseMemoryManager::Instance().New<IpcMediaDataDispatchHandler>();
}

GMI_RESULT IpcMediaDataDispatchHandler::Initialize( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::Initialize begin \n" );

    MediaSourceParameter *SourceParameter = reinterpret_cast<MediaSourceParameter *> (Argument);

    GMI_RESULT Result = StreamingMediaHandler::Initialize( FilterId, FilterName, FilterNameLength, Argument, ArgumentSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::Initialize, StreamingMediaHandler::Initialize failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    // input pin
    StreamingMediaInputPin *InputPin = StreamingMediaInputPin::CreateNew( false ,this, 0 );
    if ( NULL == InputPin )
    {
        StreamingMediaHandler::Deinitialize();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::Initialize, StreamingMediaInputPin::CreateNew failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    uint32_t ApplicationMediaType = MAKE_MEDIA(SourceParameter->s_MediaType, SourceParameter->s_CodecType);
    uint32_t ApplicationMediaId = (SourceParameter->s_SourceId<<8)+SourceParameter->s_MediaId;
    Result = InputPin->Initialize( ApplicationMediaType, ApplicationMediaId );
    if ( FAILED( Result ) )
    {
        InputPin->ReleaseRef();
        StreamingMediaHandler::Deinitialize();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::Initialize, InputPin::Initialize failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    m_SourceId  = SourceParameter->s_SourceId;
    m_MediaId   = SourceParameter->s_MediaId;
    m_MediaType = SourceParameter->s_MediaType;
    m_CodecType = SourceParameter->s_CodecType;
    m_BitRate   = SourceParameter->s_BitRate;
    m_FrameRate = SourceParameter->s_FrameRate;

    m_Inputs.push_back( InputPin );

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::Initialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT IpcMediaDataDispatchHandler::Deinitialize()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::Deinitialize begin \n" );
    if ( MFS_Uninit == Status() )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Warning, "IpcMediaDataDispatchHandler::Deinitialize, IpcMediaDataDispatchHandler is already deinitialized, funtion return %x \n", (uint32_t) GMI_SUCCESS );
        return GMI_SUCCESS;
    }

    GMI_RESULT Result = StreamingMediaHandler::Deinitialize();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Warning, "IpcMediaDataDispatchHandler::Deinitialize, StreamingMediaHandler::Deinitialize failed, funtion return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::Deinitialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT IpcMediaDataDispatchHandler::Play()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::Play begin \n" );

    uint16_t Server_UDP_Port = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1;
    GMI_RESULT Result = GetUDPPort( true, m_SourceId, m_MediaId, m_MediaType, m_CodecType, &Server_UDP_Port );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::Initialize, GetUDPPort failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    long_t   ShareMemoryKey = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1_SHARE_MEMORY_KEY;
    Result = GetShareMemoryKey( true, m_SourceId, m_MediaId, m_MediaType, m_CodecType, &ShareMemoryKey );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::Initialize, GetShareMemoryKey failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    long_t   IpcMutexKey = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1_IPC_MUTEX_KEY;
    Result = GetIpcMutexKey( true, m_SourceId, m_MediaId, m_MediaType, m_CodecType, &IpcMutexKey );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::Initialize, GetIpcMutexKey failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    size_t   ShareMemorySize = (m_BitRate*1024/8);//use 1024, instead of 1000, consider memory buffer overhead usage.
#define SHARE_MEMORY_PAGE_SIZE (1<<12)
    ShareMemorySize = (ShareMemorySize+(SHARE_MEMORY_PAGE_SIZE-1))&~(SHARE_MEMORY_PAGE_SIZE-1);//share memory generally is aligned to 4K bytes;

    size_t   MaxDataBlockSize = ShareMemorySize;
    size_t   MinDataBlockSize = ShareMemorySize/4;

    if ( MEDIA_FILTER_MEMORY_POOL_MINIMUM_MEMORY_BLOCK_NUMBER < m_FrameRate )
    {
        ShareMemorySize *= m_FrameRate;
    }
    else
    {
        ShareMemorySize *= MEDIA_FILTER_MEMORY_POOL_MINIMUM_MEMORY_BLOCK_NUMBER;
    }
    ShareMemorySize /= 2;//500ms buffer

#if defined( __linux__ )
    if ( LINUX_SHARE_MEMORY_MAX_SIZE < ShareMemorySize )
    {
        ShareMemorySize = LINUX_SHARE_MEMORY_MAX_SIZE;
    }
#endif

    uint32_t ApplicationMediaType = MAKE_MEDIA(m_MediaType, m_CodecType);
    uint32_t ApplicationMediaId = (m_SourceId<<8)+m_MediaId;

    Result = StreamingMediaHandler::Play();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::Initialize, StreamingMediaHandler.Play failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_MediaDataServer.Initialize( Server_UDP_Port, ApplicationMediaType, ApplicationMediaId, ShareMemoryKey, ShareMemorySize, MaxDataBlockSize, MinDataBlockSize, IpcMutexKey );
    if ( FAILED( Result ) )
    {
        StreamingMediaHandler::Stop();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::Initialize, MediaDataServer.Initialize failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::Play end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT IpcMediaDataDispatchHandler::Stop()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::Stop begin \n" );

    GMI_RESULT Result = StreamingMediaHandler::Stop();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Warning, "IpcMediaDataDispatchHandler::Stop, StreamingMediaHandler::Stop failed, funtion return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_MediaDataServer.Deinitialize();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Warning, "IpcMediaDataDispatchHandler::Stop, MediaDataServer::Deinitialize failed, funtion return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::Stop end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT IpcMediaDataDispatchHandler::Receive( int32_t InputPinIndex, const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength )
{
    GMI_RESULT Result = m_MediaDataServer.Write( Frame, FrameLength, FrameTS, ExtraData, ExtraDataLength );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::Receive, MediaDataServer.Write failed, function return %x, InputPinIndex=%d, FrameLength=%d, second=%d:millisecond=%d, ExtraDataLength=%d \n",
                   (uint32_t) Result, InputPinIndex, FrameLength, FrameTS->tv_sec, FrameTS->tv_usec, ExtraDataLength );
        return Result;
    }

    //DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Loop, "IpcMediaDataDispatchHandler::Receive, saw one frame, InputPinIndex=%d, FrameLength=%d, second=%d:millisecond=%d, ExtraDataLength=%d \n",
    //	InputPinIndex, FrameLength, FrameTS->tv_sec, FrameTS->tv_usec, ExtraDataLength );

    return GMI_SUCCESS;
}

GMI_RESULT IpcMediaDataDispatchHandler::GetUDPPort       ( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, uint16_t *UDP_Port )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::GetUDPPort begin, EncodeMode=%d, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d \n",
               EncodeMode, SourceId, MediaId, MediaType, CodecType );

    int32_t Default_UDP_Port = 0;
    int32_t Local_UDP_Port = 0;
    char_t  UDPPortKey[MAX_PATH_LENGTH];

    switch ( MediaType )
    {
    case MEDIA_VIDEO:
    {
        switch ( SourceId )
        {
        case 0:
            break;
        default:
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetUDPPort, Video source id(%d) is not supported, function return %x \n", SourceId, (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }

        switch ( MediaId )
        {
        case 0:
            Default_UDP_Port = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1;
            break;
        case 1:
            Default_UDP_Port = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO2;
            break;
        case 2:
            Default_UDP_Port = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO3;
            break;
        case 3:
            Default_UDP_Port = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO4;
            break;
        default:
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetUDPPort, Video media id(%d) is not supported, function return %x \n", MediaId, (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }
#if defined( __linux__ )
        sprintf( UDPPortKey, "video_%d_%s", MediaId, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SERVER_UDP_PORT );
#elif defined( _WIN32 )
        sprintf_s( UDPPortKey, MAX_PATH_LENGTH, "video_%d_%s", MediaId, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SERVER_UDP_PORT );
#endif
    }
    break;
    case MEDIA_AUDIO:
    {
        switch ( SourceId )
        {
        case 0:
            break;
        default:
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetUDPPort, Audio source id(%d) is not supported, function return %x \n", SourceId, (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }

        switch ( MediaId )
        {
        case 0:
            Default_UDP_Port = GMI_STREAMING_MEDIA_SERVER_ENCODE_AUDIO1;
            break;
        default:
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetUDPPort, Audio media id(%d) is not supported, function return %x \n", MediaId, (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }
#if defined( __linux__ )
        sprintf( UDPPortKey, "audio_%d_%s", MediaId, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SERVER_UDP_PORT );
#elif defined( _WIN32 )
        sprintf_s( UDPPortKey, MAX_PATH_LENGTH, "audio_%d_%s", MediaId, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SERVER_UDP_PORT );
#endif
    }
    break;
    default:
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetUDPPort, media type(%d) is not supported, function return %x \n", MediaType, (uint32_t) GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = SysInfoOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetUDPPort, SysInfoOpen failed, function return %x \n", MediaType, (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::GetUDPPort, Default_UDP_Port=%d \n", Default_UDP_Port );

    Result = SysInfoRead(Handle, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH, UDPPortKey, Default_UDP_Port, &Local_UDP_Port );
    if ( FAILED( Result ) )
    {
		SysInfoClose(Handle);
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetUDPPort, SysInfoRead failed, function return %x \n", MediaType, (uint32_t) Result );
        return Result;
    }

    Result = SysInfoClose(Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetUDPPort, SysInfoClose failed, function return %x \n", MediaType, (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::GetUDPPort, Default_UDP_Port=%d, Local_UDP_Port=%d \n", Default_UDP_Port, Local_UDP_Port );
#endif

    *UDP_Port = (uint16_t) Local_UDP_Port;
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::GetUDPPort end, function return %x, EncodeMode=%d, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d \n",
               GMI_SUCCESS, EncodeMode, SourceId, MediaId, MediaType, CodecType );
    return GMI_SUCCESS;
}

GMI_RESULT IpcMediaDataDispatchHandler::GetShareMemoryKey( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, long_t *ShareMemoryKey )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::GetShareMemoryKey begin, EncodeMode=%d, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d \n",
               EncodeMode, SourceId, MediaId, MediaType, CodecType );

    int32_t Default_ShareMemoryKey = 0;
    int32_t LocalShareMemoryKey = 0;
    char_t  ShareMemoryKeyName[MAX_PATH_LENGTH];

    switch ( MediaType )
    {
    case MEDIA_VIDEO:
    {
        switch ( SourceId )
        {
        case 0:
            break;
        default:
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetShareMemoryKey, Video source id(%d) is not supported, function return %x \n", SourceId, (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }

        switch ( MediaId )
        {
        case 0:
            Default_ShareMemoryKey = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1_SHARE_MEMORY_KEY;
            break;
        case 1:
            Default_ShareMemoryKey = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO2_SHARE_MEMORY_KEY;
            break;
        case 2:
            Default_ShareMemoryKey = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO3_SHARE_MEMORY_KEY;
            break;
        case 3:
            Default_ShareMemoryKey = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO4_SHARE_MEMORY_KEY;
            break;
        default:
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetShareMemoryKey, Video media id(%d) is not supported, function return %x \n", MediaId, (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }
#if defined( __linux__ )
        sprintf( ShareMemoryKeyName, "video_%d_%s", MediaId, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SHARE_MEMORY_KEY );
#elif defined( _WIN32 )
        sprintf_s( ShareMemoryKeyName, MAX_PATH_LENGTH, "video_%d_%s", MediaId, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SHARE_MEMORY_KEY );
#endif
    }
    break;
    case MEDIA_AUDIO:
    {
        switch ( SourceId )
        {
        case 0:
            break;
        default:
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetShareMemoryKey, Audio source id(%d) is not supported, function return %x \n", SourceId, (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }

        switch ( MediaId )
        {
        case 0:
            Default_ShareMemoryKey = GMI_STREAMING_MEDIA_SERVER_ENCODE_AUDIO1_SHARE_MEMORY_KEY;
            break;
        default:
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetShareMemoryKey, Audio media id(%d) is not supported, function return %x \n", MediaId, (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }
#if defined( __linux__ )
        sprintf( ShareMemoryKeyName, "audio_%d_%s", MediaId, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SHARE_MEMORY_KEY );
#elif defined( _WIN32 )
        sprintf_s( ShareMemoryKeyName, MAX_PATH_LENGTH, "audio_%d_%s", MediaId, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SHARE_MEMORY_KEY );
#endif
    }
    break;
    default:
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetShareMemoryKey, media type(%d) is not supported, function return %x \n", MediaType, (uint32_t) GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = SysInfoOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetShareMemoryKey, SysInfoOpen failed, function return %x \n", MediaType, (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::GetShareMemoryKey, Default_ShareMemoryKey=%d \n", Default_ShareMemoryKey );

    Result = SysInfoRead(Handle, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH, ShareMemoryKeyName, Default_ShareMemoryKey, &LocalShareMemoryKey );
    if ( FAILED( Result ) )
    {
		SysInfoClose(Handle);
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetShareMemoryKey, SysInfoRead failed, function return %x \n", MediaType, (uint32_t) Result );
        return Result;
    }

    Result = SysInfoClose(Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetShareMemoryKey, SysInfoClose failed, function return %x \n", MediaType, (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::GetShareMemoryKey, Default_ShareMemoryKey=%d, LocalShareMemoryKey=%d \n", Default_ShareMemoryKey, LocalShareMemoryKey );
#endif

    *ShareMemoryKey = (long_t) LocalShareMemoryKey;
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::GetShareMemoryKey end, function return %x, EncodeMode=%d, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d \n",
               GMI_SUCCESS, EncodeMode, SourceId, MediaId, MediaType, CodecType );
    return GMI_SUCCESS;
}

GMI_RESULT IpcMediaDataDispatchHandler::GetIpcMutexKey   ( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, long_t *IpcMutexKey )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::GetIpcMutexKey begin, EncodeMode=%d, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d \n",
               EncodeMode, SourceId, MediaId, MediaType, CodecType );

    int32_t Default_IpcMutexKey = 0;
    int32_t LocalIpcMutexKey = 0;
    char_t  IpcMutexKeyName[MAX_PATH_LENGTH];

    switch ( MediaType )
    {
    case MEDIA_VIDEO:
    {
        switch ( SourceId )
        {
        case 0:
            break;
        default:
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetIpcMutexKey, Video source id(%d) is not supported, function return %x \n", SourceId, (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }

        switch ( MediaId )
        {
        case 0:
            Default_IpcMutexKey = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1_IPC_MUTEX_KEY;
            break;
        case 1:
            Default_IpcMutexKey = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO2_IPC_MUTEX_KEY;
            break;
        case 2:
            Default_IpcMutexKey = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO3_IPC_MUTEX_KEY;
            break;
        case 3:
            Default_IpcMutexKey = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO4_IPC_MUTEX_KEY;
            break;
        default:
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetIpcMutexKey, Video media id(%d) is not supported, function return %x \n", MediaId, (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }
#if defined( __linux__ )
        sprintf( IpcMutexKeyName, "video_%d_%s", MediaId, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_IPC_MUTEX_KEY );
#elif defined( _WIN32 )
        sprintf_s( IpcMutexKeyName, MAX_PATH_LENGTH, "video_%d_%s", MediaId, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_IPC_MUTEX_KEY );
#endif
    }
    break;
    case MEDIA_AUDIO:
    {
        switch ( SourceId )
        {
        case 0:
            break;
        default:
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetIpcMutexKey, Audio source id(%d) is not supported, function return %x \n", SourceId, (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }

        switch ( MediaId )
        {
        case 0:
            Default_IpcMutexKey = GMI_STREAMING_MEDIA_SERVER_ENCODE_AUDIO1_IPC_MUTEX_KEY;
            break;
        default:
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetIpcMutexKey, Audio media id(%d) is not supported, function return %x \n", MediaId, (uint32_t) GMI_INVALID_PARAMETER );
            return GMI_INVALID_PARAMETER;
        }
#if defined( __linux__ )
        sprintf( IpcMutexKeyName, "audio_%d_%s", MediaId, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_IPC_MUTEX_KEY );
#elif defined( _WIN32 )
        sprintf_s( IpcMutexKeyName, MAX_PATH_LENGTH, "audio_%d_%s", MediaId, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_IPC_MUTEX_KEY );
#endif
    }
    break;
    default:
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetIpcMutexKey, media type(%d) is not supported, function return %x \n", MediaType, (uint32_t) GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = SysInfoOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetIpcMutexKey, SysInfoOpen  failed, function return %x \n", MediaType, (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::GetIpcMutexKey, Default_IpcMutexKey=%d \n", Default_IpcMutexKey );

    Result = SysInfoRead(Handle, MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH, IpcMutexKeyName, Default_IpcMutexKey, &LocalIpcMutexKey );
    if ( FAILED( Result ) )
    {
		SysInfoClose(Handle);
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetIpcMutexKey, SysInfoRead failed, function return %x \n", MediaType, (uint32_t) Result );
        return Result;
    }

    Result = SysInfoClose(Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "IpcMediaDataDispatchHandler::GetIpcMutexKey, SysInfoClose failed, function return %x \n", MediaType, (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::GetIpcMutexKey, Default_IpcMutexKey=%d, LocalIpcMutexKey=%d \n", Default_IpcMutexKey, LocalIpcMutexKey );
#endif

    *IpcMutexKey = (long_t) LocalIpcMutexKey;
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "IpcMediaDataDispatchHandler::GetIpcMutexKey end, function return %x, EncodeMode=%d, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d \n",
               GMI_SUCCESS, EncodeMode, SourceId, MediaId, MediaType, CodecType );
    return GMI_SUCCESS;
}
