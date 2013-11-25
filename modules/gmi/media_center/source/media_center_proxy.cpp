#include "media_center_proxy.h"

#include "log_client.h"
#include "media_center_packet.h"
#include "media_center_parameter.h"

#define MEDIA_CENTER_TIMEOUT 10000

MediaCenterProxy::MediaCenterProxy(void)
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    : m_OperationLock()
    , m_Client_UDP_IP( 0 )
#else
    : m_Client_UDP_IP( 0 )
#endif
    , m_Client_UDP_Port( 0 )
    , m_SessionBufferSize( 0 )
    , m_Server_UDP_IP( 0 )
    , m_Server_UDP_Port( 0 )
    , m_UDP_Socket()
    , m_UDP_Session()
    , m_SequenceNumber( 0 )
    , m_MediaCenterProxyRequestPacket()
    , m_MediaCenterProxyReplyPacket()
{
}

MediaCenterProxy::~MediaCenterProxy(void)
{
}

GMI_RESULT MediaCenterProxy::Initialize( long_t ClientIP, uint16_t ClientPort, size_t SessionBufferSize, long_t ServerIP, uint16_t ServerPort )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::Initialize begin, passed ClientIP=%x, ClientPort=%d, SessionBufferSize=%d, ServerIP=%x, ServerPort=%d \n", ntohl(ClientIP), ntohs(ClientPort), SessionBufferSize, ntohl(ServerIP), ntohs(ServerPort) );

    GMI_RESULT Result = m_MediaCenterProxyRequestPacket.Initialize( (const uint8_t *)GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::Initialize, MediaCenterProxyRequestPacket.Initialize failed, function return %x \n ", (uint32_t) Result );
        return Result;
    }

    Result = m_MediaCenterProxyReplyPacket.Initialize( (const uint8_t *)GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION );
    if ( FAILED( Result ) )
    {
        m_MediaCenterProxyRequestPacket.Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::Initialize, MediaCenterProxyReplyPacket.Initialize failed, function return %x \n ", (uint32_t) Result );
        return Result;
    }

    m_UDP_Socket = BaseMemoryManager::Instance().New<GMI_Socket>();
    if ( NULL == m_UDP_Socket.GetPtr() )
    {
        m_MediaCenterProxyReplyPacket.Deinitialize();
        m_MediaCenterProxyRequestPacket.Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::Initialize, GMI_Socket object allocation failed, function return %x \n ", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    Result = m_UDP_Socket->Create( SPF_INET, ST_DATAGRAM, SP_UDP );
    if ( FAILED( Result ) )
    {
        m_UDP_Socket = NULL;
        m_MediaCenterProxyReplyPacket.Deinitialize();
        m_MediaCenterProxyRequestPacket.Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::Initialize, GMI_Socket object creation failed, function return %x \n ", (uint32_t) Result );
        return Result;
    }

    struct sockaddr_in SocketAddress;
    SocketAddress.sin_family = (int16_t) GMI_GetSystemSocketProtocolFamily( SPF_INET );
    SocketAddress.sin_addr.s_addr = ClientIP;
    SocketAddress.sin_port = ClientPort;
    Result = m_UDP_Socket->Bind( (struct sockaddr *) &SocketAddress, sizeof(sockaddr_in) );
    if ( FAILED( Result ) )
    {
        m_UDP_Socket->Close();
        m_UDP_Socket = NULL;
        m_MediaCenterProxyReplyPacket.Deinitialize();
        m_MediaCenterProxyRequestPacket.Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::Initialize, GMI_Socket object bind failed, function return %x \n ", (uint32_t) Result );
        return Result;
    }

    m_UDP_Session = BaseMemoryManager::Instance().New<UDPSession>();
    if ( NULL == m_UDP_Session.GetPtr() )
    {
        m_UDP_Socket->Close();
        m_UDP_Socket = NULL;
        m_MediaCenterProxyReplyPacket.Deinitialize();
        m_MediaCenterProxyRequestPacket.Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::Initialize, UDPSession object allocation failed, function return %x \n ", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    Result = m_UDP_Session->Open( m_UDP_Socket, ServerIP, ServerPort, SessionBufferSize );
    if ( FAILED( Result ) )
    {
        m_UDP_Session = NULL;
        m_UDP_Socket->Close();
        m_UDP_Socket = NULL;
        m_MediaCenterProxyReplyPacket.Deinitialize();
        m_MediaCenterProxyRequestPacket.Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::Initialize, UDPSession object opening failed, function return %x \n ", (uint32_t) Result );
        return Result;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    Result = m_OperationLock.Create( NULL );
    if ( FAILED( Result ) )
    {
        m_UDP_Session->Close();
        m_UDP_Session = NULL;
        m_UDP_Socket->Close();
        m_UDP_Socket = NULL;
        m_MediaCenterProxyReplyPacket.Deinitialize();
        m_MediaCenterProxyRequestPacket.Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::Initialize, UDPSession object opening failed, function return %x \n ", (uint32_t) Result );
        return Result;
    }
#endif

    m_Client_UDP_IP     = ClientIP;
    m_Client_UDP_Port   = ClientPort;
    m_SessionBufferSize = SessionBufferSize;
    m_Server_UDP_IP     = ServerIP;
    m_Server_UDP_Port   = ServerPort;

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::Initialize end, function return %x \n", (uint32_t) GMI_SUCCESS );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterProxy::Deinitialize()
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::Deinitialize begin \n" );

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Destroy();
#endif

    if ( NULL != m_UDP_Session )
    {
        m_UDP_Session->Close();
        m_UDP_Session = NULL;
    }

    if ( NULL != m_UDP_Socket )
    {
        m_UDP_Socket->Close();
        m_UDP_Socket = NULL;
    }

    m_MediaCenterProxyReplyPacket.Deinitialize();
    m_MediaCenterProxyRequestPacket.Deinitialize();

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::Deinitialize end, function return %x \n", (uint32_t) GMI_SUCCESS );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterProxy::OpenVinVoutDevice( uint16_t SensorId, uint16_t ChannelId, FD_HANDLE *VinVoutHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::OpenVinVoutDevice begin \n" );
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenVinVoutDevice, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    *VinVoutHandle = GMI_VinVoutCreate( SensorId, ChannelId );
    if ( NULL == *VinVoutHandle )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenImageDevice, vin vout device opening failed, function return %x \n", (uint32_t) GMI_OPEN_DEVICE_FAIL );
        return GMI_OPEN_DEVICE_FAIL;
    }
    Result = MI_SUCCESS;
#else
    Result = OpenDevice1( GMI_OPEN_VIN_VOUT_DEVICE, SensorId, ChannelId, VinVoutHandle );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::OpenVinVoutDevice end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::CloseVinVoutDevice( FD_HANDLE& VinVoutHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::CloseVinVoutDevice begin \n" );
    if ( NULL == VinVoutHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CloseVinVoutDevice, vin vout device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CloseVinVoutDevice, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    GMI_VinVoutDestroy( VinVoutHandle );
    VinVoutHandle = NULL;
    Result = MI_SUCCESS;
#else
    Result = CloseDevice( VinVoutHandle, GMI_CLOSE_VIN_VOUT_DEVICE );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::CloseVinVoutDevice end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::GetVinVoutConfig( FD_HANDLE VinVoutHandle, void_t *VinParameter, size_t *VinParameterLength, void_t *VoutParameter, size_t *VoutParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetVinVoutConfig begin, VinParameterLength=%d, VoutParameterLength=%d \n", *VinParameterLength, *VoutParameterLength );
    if ( NULL == VinVoutHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetVinVoutConfig, vin vout device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetVinVoutConfig, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    if ( *VinParameterLength < sizeof(VideoInParam) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        return GMI_NOT_ENOUGH_SPACE;
    }

    if ( *VoutParameterLength < sizeof(VideoOutParam) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        return GMI_NOT_ENOUGH_SPACE;
    }

    Result = GMI_VinVoutGetConfig( VinVoutHandle, (VideoInParam *)VinParameter, (VideoOutParam *)VoutParameter );
#else
    size_t PayloadSize = sizeof(uint32_t)+sizeof(uint16_t)+sizeof(uint16_t);

    Result = m_MediaCenterProxyRequestPacket.AllocatePacketBuffer( 0, PayloadSize );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetVinVoutConfig, MediaCenterProxyRequestPacket.AllocatePacketBuffer return %x \n", (int32_t) Result );
        return Result;
    }

    Result = ApplicationPacket::FillPacketHeader( m_MediaCenterProxyRequestPacket.GetPacketHeaderBuffer(), (const uint8_t *) GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MESSAGE_FLAG_FIXED_CHECKSUM,
             GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION, ++m_SequenceNumber, GMI_MESSAGE_TYPE_REQUEST, GMI_GET_VIN_VOUT_CONFIGURATION );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetVinVoutConfig, ApplicationPacket::FillPacketHeader return %x \n", (int32_t) Result );
        return Result;
    }

    uint8_t *Offset = m_MediaCenterProxyRequestPacket.GetPacketPayloadBuffer();
    // application data

    uint32_t Token = (uint32_t)reinterpret_cast<size_t>(VinVoutHandle);
    UINT_TO_BIGENDIAN( Token, Offset );				                 // Token
    Offset += sizeof(uint32_t);

    USHORT_TO_BIGENDIAN( (uint16_t)(*VinParameterLength), Offset );  // VinParameterLength
    Offset += sizeof(uint16_t);

    USHORT_TO_BIGENDIAN( (uint16_t)(*VoutParameterLength), Offset ); // VoutParameterLength
    Offset += sizeof(uint16_t);

    Result =  m_MediaCenterProxyRequestPacket.CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetVinVoutConfig, MediaCenterProxyRequestPacket.CalculatePacketChecksum return %x \n", (int32_t) Result );
        return Result;
    }

    Result =  m_MediaCenterProxyRequestPacket.Submit( m_UDP_Session );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetVinVoutConfig, MediaCenterProxyRequestPacket.Submit return %x \n", (int32_t) Result );
        return Result;
    }

    Result = Receive( MEDIA_CENTER_TIMEOUT );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetVinVoutConfig, Receive() return %x \n", (int32_t) Result );
        return Result;
    }

    Result = m_MediaCenterProxyReplyPacket.Create( m_UDP_Session );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetVinVoutConfig, MediaCenterProxyReplyPacket.Create return %x \n", (int32_t) Result );
        return Result;
    }

    Offset = m_MediaCenterProxyReplyPacket.GetPacketPayloadBuffer();
    BIGENDIAN_TO_UINT( Offset, Result );                     // Result
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetVinVoutConfig, function return %x \n", (int32_t) Result );
        return Result;
    }
    Offset += sizeof(uint32_t);

    uint16_t Length = 0;
    BIGENDIAN_TO_USHORT( Offset, Length );                   // Vin ParameterLength
    Offset += sizeof(uint16_t);

    if ( *VinParameterLength < Length )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetVinVoutConfig, vin parameter buffer size is too small(%d), actually need %d bytes, return %x \n", *VinParameterLength, Length, (int32_t) GMI_NOT_ENOUGH_SPACE );
        return GMI_NOT_ENOUGH_SPACE;
    }

    *VinParameterLength = Length;

    memcpy( VinParameter, Offset, Length );
    Offset += Length;

    BIGENDIAN_TO_USHORT( Offset, Length );                   // Vout ParameterLength
    Offset += sizeof(uint16_t);

    if ( *VoutParameterLength < Length )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetVinVoutConfig, vout parameter buffer size is too small(%d), actually need %d bytes, return %x \n", *VoutParameterLength, Length, (int32_t) GMI_NOT_ENOUGH_SPACE );
        return GMI_NOT_ENOUGH_SPACE;
    }

    *VoutParameterLength = Length;

    memcpy( VoutParameter, Offset, Length );
    Offset += Length;
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetVinVoutConfig end, return VinParameterLength=%d, VoutParameterLength=%d, function return %x \n", *VinParameterLength, *VoutParameterLength, (uint32_t) Result );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterProxy::SetVinVoutConfig( FD_HANDLE VinVoutHandle, const void_t *VinParameter, size_t VinParameterLength, const void_t *VoutParameter, size_t VoutParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetVinVoutConfig begin, VinParameterLength=%d, VoutParameterLength=%d \n", VinParameterLength, VoutParameterLength );
    if ( NULL == VinVoutHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetVinVoutConfig, vin vout device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetVinVoutConfig, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    if ( VinParameterLength < sizeof(VideoInParam) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        return GMI_INVALID_OPERATION;
    }

    if ( VoutParameterLength < sizeof(VideoOutParam) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        return GMI_INVALID_OPERATION;
    }

    Result = GMI_VinVoutSetConfig( VinVoutHandle, (VideoInParam *)VinParameter, (VideoOutParam *)VoutParameter );
#else
    size_t PayloadSize = sizeof(uint32_t)+sizeof(uint16_t)+VinParameterLength+sizeof(uint16_t)+VoutParameterLength;

    Result = m_MediaCenterProxyRequestPacket.AllocatePacketBuffer( 0, PayloadSize );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetVinVoutConfig, MediaCenterProxyRequestPacket.AllocatePacketBuffer return %x \n", (int32_t) Result );
        return Result;
    }

    Result = ApplicationPacket::FillPacketHeader( m_MediaCenterProxyRequestPacket.GetPacketHeaderBuffer(), (const uint8_t *) GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MESSAGE_FLAG_FIXED_CHECKSUM,
             GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION, ++m_SequenceNumber, GMI_MESSAGE_TYPE_REQUEST, GMI_SET_VIN_VOUT_CONFIGURATION );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetVinVoutConfig, ApplicationPacket::FillPacketHeader return %x \n", (int32_t) Result );
        return Result;
    }

    uint8_t *Offset = m_MediaCenterProxyRequestPacket.GetPacketPayloadBuffer();
    // application data

    uint32_t Token = (uint32_t)reinterpret_cast<size_t>(VinVoutHandle);
    UINT_TO_BIGENDIAN( Token, Offset );				              // Token
    Offset += sizeof(uint32_t);

    USHORT_TO_BIGENDIAN( (uint16_t)VinParameterLength, Offset );  // Vin ParameterLength
    Offset += sizeof(uint16_t);

    memcpy( Offset, VinParameter, VinParameterLength );           // Vin Parameter
    Offset += VinParameterLength;

    USHORT_TO_BIGENDIAN( (uint16_t)VoutParameterLength, Offset ); // Vout ParameterLength
    Offset += sizeof(uint16_t);

    memcpy( Offset, VoutParameter, VoutParameterLength );         // Vout Parameter
    Offset += VoutParameterLength;

    Result =  m_MediaCenterProxyRequestPacket.CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetVinVoutConfig, MediaCenterProxyRequestPacket.CalculatePacketChecksum return %x \n", (int32_t) Result );
        return Result;
    }

    Result =  m_MediaCenterProxyRequestPacket.Submit( m_UDP_Session );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetVinVoutConfig, MediaCenterProxyRequestPacket.Submit return %x \n", (int32_t) Result );
        return Result;
    }

    Result = Receive( MEDIA_CENTER_TIMEOUT );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetVinVoutConfig, Receive() return %x \n", (int32_t) Result );
        return Result;
    }

    Result = m_MediaCenterProxyReplyPacket.Create( m_UDP_Session );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetVinVoutConfig, MediaCenterProxyReplyPacket.Create return %x \n", (int32_t) Result );
        return Result;
    }

    Offset = m_MediaCenterProxyReplyPacket.GetPacketPayloadBuffer();
    BIGENDIAN_TO_UINT( Offset, Result );                     // Result
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetVinVoutConfig, function return %x \n", (int32_t) Result );
        return Result;
    }
    Offset += sizeof(uint32_t);
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetVinVoutConfig end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::StartCodec( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, void_t *CodecParameter, size_t CodecParameterLength, FD_HANDLE *CodecHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::StartCodec begin, EncodeMode=%d, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d, CodecParameterLength=%d \n", EncodeMode, SourceId, MediaId, MediaType, CodecType, CodecParameterLength );

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StartCodec, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

    Result = CreateCodec( EncodeMode, SourceId, MediaId, MediaType, CodecType, CodecParameter, CodecParameterLength, CodecHandle );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StartCodec, CreateCodec return %x \n", (int32_t) Result );
        return Result;
    }

    Result = StartCodec2( *CodecHandle );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StartCodec, StartCodec2 return %x \n", (int32_t) Result );
        return Result;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::StartCodec end, CodecHandle=%p, function return %x \n", *CodecHandle, (int32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::StopCodec ( FD_HANDLE& CodecHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::StopCodec begin, CodecHandle=%p \n", CodecHandle );
    if ( NULL == CodecHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StopCodec, codec device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StopCodec, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

    Result = StopCodec2( CodecHandle );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StopCodec, StopCodec2 return %x \n", (int32_t) Result );
        return Result;
    }

    Result = DestroyCodec( CodecHandle );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StopCodec, DestroyCodec return %x \n", (int32_t) Result );
        return Result;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::StopCodec, function return %x \n", (int32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::CreateCodec( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, void_t *CodecParameter, size_t CodecParameterLength, FD_HANDLE *CodecHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::CreateCodec, EncodeMode=%d, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d, CodecParameterLength=%d \n", EncodeMode, SourceId, MediaId, MediaType, CodecType, CodecParameterLength );

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CreateCodec, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

    size_t PayloadSize = sizeof(uint8_t)*4+sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint16_t)+CodecParameterLength;

    Result = m_MediaCenterProxyRequestPacket.AllocatePacketBuffer( 0, PayloadSize );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CreateCodec, MediaCenterProxyRequestPacket.AllocatePacketBuffer return %x \n", (int32_t) Result );
        return Result;
    }

    Result = ApplicationPacket::FillPacketHeader( m_MediaCenterProxyRequestPacket.GetPacketHeaderBuffer(), (const uint8_t *) GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MESSAGE_FLAG_FIXED_CHECKSUM,
             GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION, ++m_SequenceNumber, GMI_MESSAGE_TYPE_REQUEST, GMI_CREATE_CODEC );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CreateCodec, ApplicationPacket::FillPacketHeader return %x \n", (int32_t) Result );
        return Result;
    }

    uint8_t *Offset = m_MediaCenterProxyRequestPacket.GetPacketPayloadBuffer();
    // application data
    *Offset++ = EncodeMode ? 1 : 0;                     // CodecMode
    *Offset++ = 0;
    *Offset++ = 0;
    *Offset++ = 0;

    UINT_TO_BIGENDIAN( SourceId, Offset );				// SourceId
    Offset += sizeof(uint32_t);

    UINT_TO_BIGENDIAN( MediaId, Offset );				// MediaId
    Offset += sizeof(uint32_t);

    UINT_TO_BIGENDIAN( MediaType, Offset );				// MediaType
    Offset += sizeof(uint32_t);

    UINT_TO_BIGENDIAN( CodecType, Offset );				// CodecType
    Offset += sizeof(uint32_t);

    USHORT_TO_BIGENDIAN( (uint16_t)CodecParameterLength, Offset );// Codec ParameterLength
    Offset += sizeof(uint16_t);

    memcpy( Offset, CodecParameter, CodecParameterLength );       // Codec Parameter
    Offset += CodecParameterLength;

    Result =  m_MediaCenterProxyRequestPacket.CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CreateCodec, MediaCenterProxyRequestPacket.CalculatePacketChecksum return %x \n", (int32_t) Result );
        return Result;
    }

    Result =  m_MediaCenterProxyRequestPacket.Submit( m_UDP_Session );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CreateCodec, MediaCenterProxyRequestPacket.Submit return %x \n", (int32_t) Result );
        return Result;
    }

    Result = Receive( MEDIA_CENTER_TIMEOUT );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CreateCodec, Receive() return %x \n", (int32_t) Result );
        return Result;
    }

    Result = m_MediaCenterProxyReplyPacket.Create( m_UDP_Session );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CreateCodec, MediaCenterProxyReplyPacket.Create return %x \n", (int32_t) Result );
        return Result;
    }

    Offset = m_MediaCenterProxyReplyPacket.GetPacketPayloadBuffer();
    BIGENDIAN_TO_UINT( Offset, Result );                     //Result
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CreateCodec, function return %x \n", (int32_t) Result );
        return Result;
    }
    Offset += sizeof(uint32_t);

    uint32_t Token = 0;
    BIGENDIAN_TO_UINT( Offset, Token );
    Offset += sizeof(uint32_t);

    *CodecHandle = reinterpret_cast<FD_HANDLE>((size_t)Token);
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::CreateCodec end, CodecHandle=%p, function return %x \n", *CodecHandle, (int32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::DestroyCodec( FD_HANDLE& CodecHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::DestroyCodec begin, CodecHandle=%p \n", CodecHandle );
    if ( NULL == CodecHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::DestroyCodec, codec device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::DestroyCodec, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

    Result = CloseDevice( CodecHandle, GMI_DESTROY_CODEC );
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::DestroyCodec, function return %x \n", (int32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::StartCodec2( FD_HANDLE CodecHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::StartCodec2 begin, passed CodecHandle=%p \n", CodecHandle );
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StartCodec2, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

    Result = OperationWithoutParameter( CodecHandle, GMI_START_CODEC );
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::StartCodec2 end, returned OperationWithoutParameter return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::StopCodec2 ( FD_HANDLE CodecHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::StopCodec2 begin, passed CodecHandle=%p \n", CodecHandle );
    if ( NULL == CodecHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StopCodec2, codec device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StopCodec2, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

    Result = OperationWithoutParameter( CodecHandle, GMI_STOP_CODEC );
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::StopCodec2 end, returned OperationWithoutParameter return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::GetCodecConfig( FD_HANDLE CodecHandle, void_t *CodecParameter, size_t *CodecParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetCodecConfig begin, passed CodecParameterLength=%d \n", *CodecParameterLength );
    if ( NULL == CodecHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetCodecConfig, codec device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetCodecConfig, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

    Result = GetXXXConfig( CodecHandle, GMI_GET_CODEC_CONFIGURARTION, CodecParameter, CodecParameterLength );
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetCodecConfig end, returned CodecParameterLength=%d, GetXXXConfig return %x \n", *CodecParameterLength, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::SetCodecConfig( FD_HANDLE CodecHandle, const void_t *CodecParameter, size_t CodecParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetCodecConfig begin, passed CodecParameterLength=%d \n", CodecParameterLength );
    if ( NULL == CodecHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetCodecConfig, codec device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetCodecConfig, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

    Result = SetXXXConfig( CodecHandle, GMI_SET_CODEC_CONFIGURARTION, CodecParameter, CodecParameterLength );
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetCodecConfig end, SetXXXConfig return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::GetOsdConfig( FD_HANDLE CodecHandle, void_t *OsdParameter, size_t *OsdParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetOsdConfig begin, passed OsdParameterLength=%d \n", *OsdParameterLength );
    if ( NULL == CodecHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetOsdConfig, codec device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetOsdConfig, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

    Result = GetXXXConfig( CodecHandle, GMI_GET_OSD_CONFIGURARTION, OsdParameter, OsdParameterLength );
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetOsdConfig end, returned OsdParameterLength=%d, GetXXXConfig return %x \n", *OsdParameterLength, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::SetOsdConfig( FD_HANDLE CodecHandle, const void_t *OsdParameter, size_t OsdParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetOsdConfig begin, passed OsdParameterLength=%d \n", OsdParameterLength );
    if ( NULL == CodecHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetOsdConfig, codec device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetOsdConfig, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

    Result = SetXXXConfig( CodecHandle, GMI_SET_OSD_CONFIGURARTION, OsdParameter, OsdParameterLength );
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetOsdConfig end, SetXXXConfig return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::ForceGenerateIdrFrame( FD_HANDLE CodecHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::ForceGenerateIdrFrame begin, passed CodecHandle=%p \n", CodecHandle );
    if ( NULL == CodecHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::ForceGenerateIdrFrame, codec device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::ForceGenerateIdrFrame, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

    Result = OperationWithoutParameter( CodecHandle, GMI_FORCE_GENERATE_IDR_FRAME );
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::ForceGenerateIdrFrame, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::OpenImageDevice( uint16_t SensorId, uint16_t ChannelId, FD_HANDLE *ImageHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::OpenImageDevice begin, passed SensorId=%d, ChannelId=%d \n", SensorId, ChannelId );
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenImageDevice, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    *ImageHandle = GMI_ImageOperateCreate( SensorId, ChannelId );
    if ( NULL == *ImageHandle )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenImageDevice, image device opening failed, function return %x \n", (uint32_t) GMI_OPEN_DEVICE_FAIL );
        return GMI_OPEN_DEVICE_FAIL;
    }
    GMI_RESULT Result = GMI_SUCCESS;
#else
    Result = OpenDevice1( GMI_OPEN_IMAGE_DEVICE, SensorId, ChannelId, ImageHandle );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::OpenImageDevice end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::CloseImageDevice( FD_HANDLE& ImageHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::CloseImageDevice begin, passed ImageHandle=%p \n", ImageHandle );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CloseImageDevice, image device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CloseImageDevice, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    GMI_ImageOperateDestroy( ImageHandle );
    ImageHandle = NULL;
    Result = GMI_SUCCESS;
#else
    Result = CloseDevice( ImageHandle, GMI_CLOSE_IMAGE_DEVICE );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::CloseImageDevice end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::GetBaseImageConfig( FD_HANDLE ImageHandle, void_t *ImageParameter, size_t *ImageParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetBaseImageConfig begin, passed ImageParameterLength=%p \n", *ImageParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetBaseImageConfig, image device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetBaseImageConfig, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    if ( *ImageParameterLength < sizeof(ImageBaseParam) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        return GMI_NOT_ENOUGH_SPACE;
    }

    Result = GMI_ImageGetBaseConfig( ImageHandle, (ImageBaseParam *) ImageParameter );
#else
    Result = GetXXXConfig( ImageHandle, GMI_GET_BASE_IMAGE_CONFIGURARTION, ImageParameter, ImageParameterLength );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetBaseImageConfig end, returned ImageParameterLength=%d, function return %x \n", *ImageParameterLength, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::SetBaseImageConfig( FD_HANDLE ImageHandle, const void_t *ImageParameter, size_t ImageParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetBaseImageConfig begin, passed ImageParameterLength=%p \n", ImageParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetBaseImageConfig, image device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetBaseImageConfig, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    if ( ImageParameterLength < sizeof(ImageBaseParam) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        return GMI_INVALID_OPERATION;
    }

    Result = GMI_ImageSetBaseConfig( ImageHandle, (ImageBaseParam *) ImageParameter );
#else
    Result = SetXXXConfig( ImageHandle, GMI_SET_BASE_IMAGE_CONFIGURARTION, ImageParameter, ImageParameterLength );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetBaseImageConfig end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::GetAdvancedImageConfig( FD_HANDLE ImageHandle, void_t *ImageParameter, size_t *ImageParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetAdvancedImageConfig begin, passed ImageParameterLength=%p \n", *ImageParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetAdvancedImageConfig, image device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetAdvancedImageConfig, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    if ( *ImageParameterLength < sizeof(ImageAdvanceParam) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        return GMI_NOT_ENOUGH_SPACE;
    }

    Result = GMI_ImageGetAdvanceConfig( ImageHandle, (ImageAdvanceParam *) ImageParameter );
#else
    Result = GetXXXConfig( ImageHandle, GMI_GET_ADVANCED_IMAGE_CONFIGURARTION, ImageParameter, ImageParameterLength );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetAdvancedImageConfig end, returned ImageParameterLength=%d, function return %x \n", *ImageParameterLength, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::SetAdvancedImageConfig( FD_HANDLE ImageHandle, const void_t *ImageParameter, size_t ImageParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetAdvancedImageConfig begin, passed ImageParameterLength=%p \n", ImageParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetAdvancedImageConfig, image device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetAdvancedImageConfig, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    if ( ImageParameterLength < sizeof(ImageAdvanceParam) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        return GMI_INVALID_PARAMETER;
    }

    Result = GMI_ImageSetAdvanceConfig( ImageHandle, (ImageAdvanceParam *) ImageParameter );
#else
    Result = SetXXXConfig( ImageHandle, GMI_SET_ADVANCED_IMAGE_CONFIGURARTION, ImageParameter, ImageParameterLength );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetAdvancedImageConfig end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::GetAutoFocusConfig( FD_HANDLE ImageHandle, void_t *AutoFocusParameter, size_t *AutoFocusParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetAutoFocusConfig begin, passed AutoFocusParameterLength=%p \n", *AutoFocusParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetAutoFocusConfig, image device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetAutoFocusConfig, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    if ( *AutoFocusParameterLength < sizeof(ImageAfParam) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        return GMI_NOT_ENOUGH_SPACE;
    }

    Result = GMI_ImageGetAfConfig( ImageHandle, (ImageAfParam *) AutoFocusParameter );
#else
    Result = GetXXXConfig( ImageHandle, GMI_GET_AUTO_FOCUS_CONFIGURARTION, AutoFocusParameter, AutoFocusParameterLength );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetAutoFocusConfig end, returned AutoFocusParameterLength=%d, function return %x \n", *AutoFocusParameterLength, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::SetAutoFocusConfig( FD_HANDLE ImageHandle, const void_t *AutoFocusParameter, size_t AutoFocusParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetAutoFocusConfig begin, passed AutoFocusParameterLength=%p \n", AutoFocusParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetAutoFocusConfig, image device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetAutoFocusConfig, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    if ( AutoFocusParameterLength < sizeof(ImageAfParam) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        return GMI_INVALID_PARAMETER;
    }

    Result = GMI_ImageSetAfConfig( ImageHandle, (ImageAfParam *) AutoFocusParameter );
#else
    Result = SetXXXConfig( ImageHandle, GMI_SET_AUTO_FOCUS_CONFIGURARTION, AutoFocusParameter, AutoFocusParameterLength );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetAutoFocusConfig end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::GetDaynightConfig( FD_HANDLE ImageHandle, void_t *DaynightParameter, size_t *DaynightParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetDaynightConfig begin, passed DaynightParameterLength=%p \n", *DaynightParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetDaynightConfig, image device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetDaynightConfig, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    if ( *DaynightParameterLength < sizeof(ImageDnParam) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        return GMI_NOT_ENOUGH_SPACE;
    }

    Result = GMI_ImageGetDnConfig( ImageHandle, (ImageDnParam *) DaynightParameter );
#else
    Result = GetXXXConfig( ImageHandle, GMI_GET_DAYNIGHT_CONFIGURARTION, DaynightParameter, DaynightParameterLength );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetDaynightConfig end, returned DaynightParameterLength=%d, function return %x \n", *DaynightParameterLength, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::SetDaynightConfig( FD_HANDLE ImageHandle, const void_t *DaynightParameter, size_t DaynightParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetDaynightConfig begin, passed DaynightParameterLength=%p \n", DaynightParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetDaynightConfig, image device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetDaynightConfig, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    if ( DaynightParameterLength < sizeof(ImageDnParam) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        return GMI_INVALID_PARAMETER;
    }

    Result = GMI_ImageSetDnConfig( ImageHandle, (ImageDnParam *) DaynightParameter );
#else
    Result = SetXXXConfig( ImageHandle, GMI_SET_DAYNIGHT_CONFIGURARTION, DaynightParameter, DaynightParameterLength );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetDaynightConfig end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::GetWhiteBalanceConfig( FD_HANDLE ImageHandle, void_t *WhiteBalanceParameter, size_t *WhiteBalanceParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetWhiteBalanceConfig begin, passed WhiteBalanceParameterLength=%p \n", *WhiteBalanceParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetWhiteBalanceConfig, image device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetWhiteBalanceConfig, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    if ( *WhiteBalanceParameterLength < sizeof(ImageWbParam) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        return GMI_NOT_ENOUGH_SPACE;
    }

    Result = GMI_ImageGetWbConfig( ImageHandle, (ImageWbParam *) WhiteBalanceParameter );
#else
    Result = GetXXXConfig( ImageHandle, GMI_GET_WHITE_BALANCE_CONFIGURARTION, WhiteBalanceParameter, WhiteBalanceParameterLength );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetWhiteBalanceConfig end, returned WhiteBalanceParameterLength=%d, function return %x \n", *WhiteBalanceParameterLength, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::SetWhiteBalanceConfig( FD_HANDLE ImageHandle, const void_t *WhiteBalanceParameter, size_t WhiteBalanceParameterLength )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetWhiteBalanceConfig begin, passed WhiteBalanceParameterLength=%p \n", WhiteBalanceParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetWhiteBalanceConfig, image device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetWhiteBalanceConfig, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    if ( WhiteBalanceParameterLength < sizeof(ImageWbParam) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        return GMI_INVALID_PARAMETER;
    }

    Result = GMI_ImageSetWbConfig( ImageHandle, (ImageWbParam *) WhiteBalanceParameter );
#else
    Result = SetXXXConfig( ImageHandle, GMI_SET_WHITE_BALANCE_CONFIGURARTION, WhiteBalanceParameter, WhiteBalanceParameterLength );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetWhiteBalanceConfig end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::OpenAutoFocusDevice( uint16_t SensorId, FD_HANDLE *AutoFocusHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::OpenAutoFocusDevice begin, passed SensorId=%d \n", SensorId );
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenAutoFocusDevice, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    *AutoFocusHandle = GMI_AutoFocusCreate( SensorId );
    if ( NULL == *AutoFocusHandle )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenAutoFocusDevice, auto focus device opening failed, function return %x \n", (uint32_t) GMI_OPEN_DEVICE_FAIL );
        return GMI_OPEN_DEVICE_FAIL;
    }
    Result = GMI_SUCCESS;
#else
    Result = OpenDevice2( GMI_OPEN_AUTO_FOCUS_DEVICE, SensorId, AutoFocusHandle );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::OpenAutoFocusDevice end, AutoFocusHandle=%p, function return %x \n", *AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::CloseAutoFocusDevice( FD_HANDLE& AutoFocusHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::CloseAutoFocusDevice begin, passed AutoFocusHandle=%p \n", AutoFocusHandle );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CloseAutoFocusDevice, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CloseAutoFocusDevice, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    GMI_AutoFocusDestroy( AutoFocusHandle );
    AutoFocusHandle = NULL;
    Result = GMI_SUCCESS;
#else
    Result = CloseDevice( AutoFocusHandle, GMI_CLOSE_AUTO_FOCUS_DEVICE );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::CloseAutoFocusDevice end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::StartAutoFocus( FD_HANDLE AutoFocusHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::StartAutoFocus begin, passed AutoFocusHandle=%p \n", AutoFocusHandle );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StartAutoFocus, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StartAutoFocus, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_AutoFocusStart( AutoFocusHandle );
#else
    Result = OperationWithoutParameter( AutoFocusHandle, GMI_START_AUTO_FOCUS );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::StartAutoFocus end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::StopAutoFocus( FD_HANDLE AutoFocusHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::StopAutoFocus begin, passed AutoFocusHandle=%p \n", AutoFocusHandle );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StopAutoFocus, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StopAutoFocus, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_AutoFocusStop( AutoFocusHandle );
#else
    Result = OperationWithoutParameter( AutoFocusHandle, GMI_STOP_AUTO_FOCUS );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::StopAutoFocus end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::PauseAutoFocus( FD_HANDLE AutoFocusHandle, int8_t ControlStatus )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::PauseAutoFocus begin, passed AutoFocusHandle=%p \n", AutoFocusHandle );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::PauseAutoFocus, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::PauseAutoFocus, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_AutoFocusPause( AutoFocusHandle, ControlStatus )
#else
    Result = SetOperationWithOneParameter( AutoFocusHandle, GMI_PAUSE_AUTO_FOCUS, ControlStatus );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
             m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::PauseAutoFocus end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::AutoFocusGlobalScan( FD_HANDLE AutoFocusHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::AutoFocusGlobalScan begin, passed AutoFocusHandle=%p \n", AutoFocusHandle );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::AutoFocusGlobalScan, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::AutoFocusGlobalScan, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_AutoFocusGlobalScan( AutoFocusHandle );
#else
    Result = OperationWithoutParameter( AutoFocusHandle, GMI_AUTO_FOCUS_GLOBAL_SCAN );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::AutoFocusGlobalScan end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::SetAutoFocusMode( FD_HANDLE AutoFocusHandle, int32_t AFMode )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetAutoFocusMode begin, passed AutoFocusHandle=%p, AFMode=%d \n", AutoFocusHandle, AFMode );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetAutoFocusMode, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetAutoFocusMode, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_AutoFocusSetMode( AutoFocusHandle, AFMode );
#else
    Result = SetOperationWithOneParameter( AutoFocusHandle, GMI_SET_AUTO_FOCUS_MODE, AFMode );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetAutoFocusMode end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::NotifyAutoFocus( FD_HANDLE	AutoFocusHandle, int32_t EventType, uint8_t *ExtData, uint32_t Length )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::NotifyAutoFocusDevice begin, passed AutoFocusHandle=%p \n", AutoFocusHandle );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::NotifyAutoFocusDevice, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::NotifyAutoFocus, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_AFEventNotify( AutoFocusHandle, EventType, ExtData, Length );
#else
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::NotifyAutoFocusDevice begin, passed AutoFocusHandle=%p, EventType=%d, Length=%d \n", AutoFocusHandle, EventType, Length );
    size_t PayloadSize = sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint32_t)+Length;

    Result = m_MediaCenterProxyRequestPacket.AllocatePacketBuffer( 0, PayloadSize );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::NotifyAutoFocusDevice, MediaCenterProxyRequestPacket.AllocatePacketBuffer return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = ApplicationPacket::FillPacketHeader( m_MediaCenterProxyRequestPacket.GetPacketHeaderBuffer(), (const uint8_t *) GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MESSAGE_FLAG_FIXED_CHECKSUM,
             GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION, ++m_SequenceNumber, GMI_MESSAGE_TYPE_REQUEST, GMI_NOTIFY_AUTO_FOCUS );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::NotifyAutoFocusDevice, ApplicationPacket::FillPacketHeader return %x \n", (uint32_t) Result );
        return Result;
    }

    uint8_t *Offset = m_MediaCenterProxyRequestPacket.GetPacketPayloadBuffer();
    // application data

    uint32_t Token = (uint32_t)reinterpret_cast<size_t>(AutoFocusHandle);
    UINT_TO_BIGENDIAN( Token, Offset );				    // Token
    Offset += sizeof(uint32_t);

    UINT_TO_BIGENDIAN( EventType, Offset );				// EventType
    Offset += sizeof(uint32_t);

    UINT_TO_BIGENDIAN( Length, Offset );				// Length
    Offset += sizeof(uint32_t);

    memcpy( Offset, ExtData, Length );                  // ExtData
    Offset += Length;

    Result =  m_MediaCenterProxyRequestPacket.CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::NotifyAutoFocusDevice, MediaCenterProxyRequestPacket.CalculatePacketChecksum return %x \n", (uint32_t) Result );
        return Result;
    }

    Result =  m_MediaCenterProxyRequestPacket.Submit( m_UDP_Session );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::NotifyAutoFocusDevice, MediaCenterProxyRequestPacket.Submit return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = Receive( MEDIA_CENTER_TIMEOUT );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::NotifyAutoFocusDevice, Receive() return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_MediaCenterProxyReplyPacket.Create( m_UDP_Session );
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::NotifyAutoFocusDevice, MediaCenterProxyReplyPacket.Create return %x \n", (uint32_t) Result );
        return Result;
    }

    Offset = m_MediaCenterProxyReplyPacket.GetPacketPayloadBuffer();
    BIGENDIAN_TO_UINT( Offset, Result );                     //Result
    if ( FAILED( Result ) )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::NotifyAutoFocusDevice, function return %x \n", (uint32_t) Result );
        return Result;
    }
    Offset += sizeof(uint32_t);
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::NotifyAutoFocusDevice end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::GetFocusPosition( FD_HANDLE AutoFocusHandle, int32_t *FocusPos )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetFocusPosition begin, passed AutoFocusHandle=%p \n", AutoFocusHandle );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetFocusPosition, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetFocusPosition, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_FocusPositionGet( AutoFocusHandle, FocusPos );
#else
    Result = GetOperationWithOneParameter( AutoFocusHandle, GMI_GET_FOCUS_POSITION, FocusPos );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetFocusPosition end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::SetFocusPosition( FD_HANDLE AutoFocusHandle, int32_t FocusPos )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetFocusPosition begin, passed AutoFocusHandle=%p, FocusPos=%d \n", AutoFocusHandle, FocusPos );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetFocusPosition, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetFocusPosition, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_FocusPositionSet( AutoFocusHandle, FocusPos );
#else
    Result = SetOperationWithOneParameter( AutoFocusHandle, GMI_SET_FOCUS_POSITION, FocusPos );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetFocusPosition end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::GetFocusRange( FD_HANDLE AutoFocusHandle, int32_t *MinFocusPos, int32_t *MaxFocusPos )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetFocusRange begin, passed AutoFocusHandle=%p \n", AutoFocusHandle );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetFocusRange, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetFocusRange, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_FocusRangeGet( AutoFocusHandle, MinFocusPos, MaxFocusPos );
#else
    Result = GetOperationWithTwoParameter( AutoFocusHandle, GMI_GET_FOCUS_RANGE, MinFocusPos, MaxFocusPos );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetFocusRange end, AutoFocusHandle=%p, MinFocusPos=%d, MaxFocusPos=%d, function return %x \n", AutoFocusHandle, *MinFocusPos, *MaxFocusPos, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::ResetFocusMotor( FD_HANDLE AutoFocusHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::ResetFocusMotor begin, passed AutoFocusHandle=%p \n", AutoFocusHandle );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::ResetFocusMotor, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::ResetFocusMotor, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_FocusMotorReset( AutoFocusHandle );
#else
    Result = OperationWithoutParameter( AutoFocusHandle, GMI_RESET_FOCUS_MOTOR );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetFocusRange end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::ControlAutoFocus( FD_HANDLE AutoFocusHandle, int8_t AfDirMode )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::ControlAutoFocus begin, passed AutoFocusHandle=%p, AfDirMode=%d \n", AutoFocusHandle, AfDirMode );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::ControlAutoFocus, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::ControlAutoFocus, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_AfCtrol( AutoFocusHandle, AfDirMode );
#else
    Result = SetOperationWithOneParameter( AutoFocusHandle, GMI_CONTROL_AUTO_FOCUS, AfDirMode );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::ControlAutoFocus end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::SetAutoFocusStep( FD_HANDLE AutoFocusHandle, int32_t AfStep )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetAutoFocusStep begin, passed AutoFocusHandle=%p, AfStep=%d \n", AutoFocusHandle, AfStep );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetAutoFocusStep, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetAutoFocusStep, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_AfStepSet( AutoFocusHandle, AfStep );
#else
    Result = SetOperationWithOneParameter( AutoFocusHandle, GMI_SET_AUTO_FOCUS_STEP, AfStep );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetAutoFocusStep end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::OpenZoomDevice( uint16_t SensorId, FD_HANDLE *ZoomHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::OpenZoomDevice begin, passed SensorId=%d \n", SensorId );

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenZoomDevice, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    *ZoomHandle = GMI_ZoomCreate( SensorId );
    if ( NULL == *ZoomHandle )
    {
#if MEDIA_CENTER_PROXY_THREADS_SYNC
        m_OperationLock.Unlock();
#endif
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenZoomDevice, zoom device opening failed, function return %x \n", (uint32_t) GMI_OPEN_DEVICE_FAIL );
        return GMI_OPEN_DEVICE_FAIL;
    }
    Result = GMI_SUCCESS;
#else
    Result = OpenDevice2( GMI_OPEN_ZOOM_DEVICE, SensorId, ZoomHandle );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::OpenZoomDevice end, ZoomHandle=%p, function return %x \n", *ZoomHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::CloseZoomDevice( FD_HANDLE& ZoomHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::CloseZoomDevice begin, passed ZoomHandle=%p \n", ZoomHandle );
    if ( NULL == ZoomHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CloseZoomDevice, zoom device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CloseZoomDevice, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    GMI_ZoomDestroy( ZoomHandle );
    ZoomHandle = NULL;
    Result = GMI_SUCCESS;
#else
    Result = CloseDevice( ZoomHandle, GMI_CLOSE_ZOOM_DEVICE );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::CloseZoomDevice end, ZoomHandle=%p, function return %x \n", ZoomHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::GetZoomPosition( FD_HANDLE ZoomHandle, int32_t *ZoomPos )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetZoomPosition begin, passed ZoomHandle=%p \n", ZoomHandle );
    if ( NULL == ZoomHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetZoomPosition, zoom device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetZoomPosition, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_ZoomPositionGet( ZoomHandle, ZoomPos );
#else
    Result = GetOperationWithOneParameter( ZoomHandle, GMI_GET_ZOOM_POSITION, ZoomPos );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetZoomPosition end, ZoomHandle=%p, ZoomPos=%d, function return %x \n", ZoomHandle, *ZoomPos, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::SetZoomPosition( FD_HANDLE ZoomHandle, int32_t ZoomPos )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetZoomPosition begin, passed ZoomHandle=%p, ZoomPos=%d \n", ZoomHandle, ZoomPos );
    if ( NULL == ZoomHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetZoomPosition, zoom device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetZoomPosition, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_ZoomPositionSet( ZoomHandle, ZoomPos );
#else
    Result = SetOperationWithOneParameter( ZoomHandle, GMI_SET_ZOOM_POSITION, ZoomPos );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetZoomPosition end, ZoomHandle=%p, ZoomPos=%d, function return %x \n", ZoomHandle, ZoomPos, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::GetZoomRange( FD_HANDLE ZoomHandle, int32_t *MinZoomPos, int32_t *MaxZoomPos )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetZoomRange begin, passed ZoomHandle=%p \n", ZoomHandle );
    if ( NULL == ZoomHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetZoomRange, zoom device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetZoomRange, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_ZoomRangeGet( ZoomHandle, MinZoomPos, MaxZoomPos );
#else
    Result = GetOperationWithTwoParameter( ZoomHandle, GMI_GET_ZOOM_RANGE, MinZoomPos, MaxZoomPos );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetZoomRange end, ZoomHandle=%p, MinZoomPos=%d, MaxZoomPos=%d, function return %x \n", ZoomHandle, *MinZoomPos, *MaxZoomPos, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::ResetZoomMotor( FD_HANDLE ZoomHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::ResetZoomMotor begin, passed ZoomHandle=%p \n", ZoomHandle );
    if ( NULL == ZoomHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::ResetZoomMotor, zoom device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::ResetZoomMotor, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_ZoomMotorReset( ZoomHandle );
#else
    Result = OperationWithoutParameter( ZoomHandle, GMI_RESET_ZOOM_MOTOR );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::ResetZoomMotor end, ZoomHandle=%p, function return %x \n", ZoomHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::ControlZoom( FD_HANDLE ZoomHandle, int8_t ZoomMode )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::ControlZoom begin, passed ZoomHandle=%p, ZoomMode=%d \n", ZoomHandle, ZoomMode );
    if ( NULL == ZoomHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::ControlZoom, zoom device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::ControlZoom, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_ZoomCtrol( ZoomHandle, ZoomMode );
#else
    Result = SetOperationWithOneParameter( ZoomHandle, GMI_CONTROL_ZOOM, ZoomMode );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::ControlZoom end, ZoomHandle=%p, function return %x \n", ZoomHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::SetZoomStep( FD_HANDLE ZoomHandle, int32_t ZoomStep )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetZoomStep begin, passed ZoomHandle=%p, ZoomStep=%d \n", ZoomHandle, ZoomStep );
    if ( NULL == ZoomHandle )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetZoomStep, zoom device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::ControlZoom, OperationLock.Lock failed, return %x \n", (uint32_t) Result );
        return Result;
    }
#else
    GMI_RESULT Result = GMI_FAIL;
#endif

#if 0
    Result = GMI_ZoomStepSet( ZoomHandle, ZoomStep );
#else
    Result = SetOperationWithOneParameter( ZoomHandle, GMI_SET_ZOOM_STEP, ZoomStep );
#endif
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    m_OperationLock.Unlock();
#endif
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetZoomStep end, ZoomHandle=%p, function return %x \n", ZoomHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenterProxy::StartZoom( FD_HANDLE AutoFocusHandle, int8_t ControlStatus, FD_HANDLE ZoomHandle, int8_t ZoomMode )
{
#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::StartZoom begin, passed AutoFocusHandle=%p, ControlStatus=%d, ZoomHandle=%p, ZoomMode=%d \n", AutoFocusHandle, ControlStatus, ZoomHandle, ZoomMode );
#endif

    size_t PayloadSize = sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint32_t);

    GMI_RESULT Result = m_MediaCenterProxyRequestPacket.AllocatePacketBuffer( 0, PayloadSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StartZoom, MediaCenterProxyRequestPacket.AllocatePacketBuffer return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = ApplicationPacket::FillPacketHeader( m_MediaCenterProxyRequestPacket.GetPacketHeaderBuffer(), (const uint8_t *) GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MESSAGE_FLAG_FIXED_CHECKSUM,
             GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION, ++m_SequenceNumber, GMI_MESSAGE_TYPE_REQUEST, GMI_START_ZOOM );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StartZoom, ApplicationPacket::FillPacketHeader return %x \n", (uint32_t) Result );
        return Result;
    }

    uint8_t *Offset = m_MediaCenterProxyRequestPacket.GetPacketPayloadBuffer();
    // application data

    uint32_t Token = (uint32_t)reinterpret_cast<size_t>(AutoFocusHandle);
    UINT_TO_BIGENDIAN( Token, Offset );				        // AutoFocusHandle
    Offset += sizeof(uint32_t);

    int32_t Parameter2 = ControlStatus;
    UINT_TO_BIGENDIAN( Parameter2, Offset );				// AutoFocusControlStatus
    Offset += sizeof(uint32_t);

    Token = (uint32_t)reinterpret_cast<size_t>(ZoomHandle);
    UINT_TO_BIGENDIAN( Token, Offset );				        // ZoomHandle
    Offset += sizeof(uint32_t);

    int32_t Parameter4 = ZoomMode;
    UINT_TO_BIGENDIAN( Parameter4, Offset );				// ZoomMode
    Offset += sizeof(uint32_t);

    Result =  m_MediaCenterProxyRequestPacket.CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StartZoom, MediaCenterProxyRequestPacket.CalculatePacketChecksum return %x \n", (uint32_t) Result );
        return Result;
    }

    Result =  m_MediaCenterProxyRequestPacket.Submit( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StartZoom, MediaCenterProxyRequestPacket.Submit return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = Receive( MEDIA_CENTER_TIMEOUT );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StartZoom, Receive() return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_MediaCenterProxyReplyPacket.Create( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StartZoom, MediaCenterProxyReplyPacket.Create return %x \n", (uint32_t) Result );
        return Result;
    }

    Offset = m_MediaCenterProxyReplyPacket.GetPacketPayloadBuffer();
    BIGENDIAN_TO_UINT( Offset, Result );                     //Result
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StartZoom, function return %x \n", (uint32_t) Result );
        return Result;
    }
    Offset += sizeof(uint32_t);

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::StartZoom end, passed AutoFocusHandle=%p, ControlStatus=%d, ZoomHandle=%p, ZoomMode=%d, function return %x \n",
               AutoFocusHandle, ControlStatus, ZoomHandle, ZoomMode, (uint32_t) Result );
#endif
    return Result;
}

GMI_RESULT MediaCenterProxy::StopZoom( FD_HANDLE AutoFocusHandle, int8_t ControlStatus, FD_HANDLE ZoomHandle, int8_t ZoomMode, int32_t *ZoomPos )
{
#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::StopZoom begin, passed AutoFocusHandle=%p, ControlStatus=%d, ZoomHandle=%p, ZoomMode=%d \n", AutoFocusHandle, ControlStatus, ZoomHandle, ZoomMode );
#endif

    size_t PayloadSize = sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint32_t);

    GMI_RESULT Result = m_MediaCenterProxyRequestPacket.AllocatePacketBuffer( 0, PayloadSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StopZoom, MediaCenterProxyRequestPacket.AllocatePacketBuffer return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = ApplicationPacket::FillPacketHeader( m_MediaCenterProxyRequestPacket.GetPacketHeaderBuffer(), (const uint8_t *) GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MESSAGE_FLAG_FIXED_CHECKSUM,
             GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION, ++m_SequenceNumber, GMI_MESSAGE_TYPE_REQUEST, GMI_STOP_ZOOM );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StopZoom, ApplicationPacket::FillPacketHeader return %x \n", (uint32_t) Result );
        return Result;
    }

    uint8_t *Offset = m_MediaCenterProxyRequestPacket.GetPacketPayloadBuffer();
    // application data

    uint32_t Token = (uint32_t)reinterpret_cast<size_t>(AutoFocusHandle);
    UINT_TO_BIGENDIAN( Token, Offset );				        // AutoFocusHandle
    Offset += sizeof(uint32_t);

    int32_t Parameter2 = ControlStatus;
    UINT_TO_BIGENDIAN( Parameter2, Offset );				// AutoFocusControlStatus
    Offset += sizeof(uint32_t);

    Token = (uint32_t)reinterpret_cast<size_t>(ZoomHandle);
    UINT_TO_BIGENDIAN( Token, Offset );				        // ZoomHandle
    Offset += sizeof(uint32_t);

    int32_t Parameter4 = ZoomMode;
    UINT_TO_BIGENDIAN( Parameter4, Offset );				// ZoomMode
    Offset += sizeof(uint32_t);

    Result =  m_MediaCenterProxyRequestPacket.CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StopZoom, MediaCenterProxyRequestPacket.CalculatePacketChecksum return %x \n", (uint32_t) Result );
        return Result;
    }

    Result =  m_MediaCenterProxyRequestPacket.Submit( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StopZoom, MediaCenterProxyRequestPacket.Submit return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = Receive( MEDIA_CENTER_TIMEOUT );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StopZoom, Receive() return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_MediaCenterProxyReplyPacket.Create( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StopZoom, MediaCenterProxyReplyPacket.Create return %x \n", (uint32_t) Result );
        return Result;
    }

    Offset = m_MediaCenterProxyReplyPacket.GetPacketPayloadBuffer();
    BIGENDIAN_TO_UINT( Offset, Result );                     //Result
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::StopZoom, function return %x \n", (uint32_t) Result );
        return Result;
    }
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_UINT( Offset, *ZoomPos );
    Offset += sizeof(uint32_t);

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::StopZoom end, passed AutoFocusHandle=%p, ControlStatus=%d, ZoomHandle=%p, ZoomMode=%d, function return %x \n",
               AutoFocusHandle, ControlStatus, ZoomHandle, ZoomMode, (uint32_t) Result );
#endif
    return Result;
}

GMI_RESULT MediaCenterProxy::Receive( long_t Timeout )
{
#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Loop, "MediaCenterProxy::Receive begin, passed Timeout=%d \n", Timeout );
#endif
    GMI_RESULT Result = GMI_FAIL;
    do
    {
        Result = m_UDP_Session->Receive();
        if ( SUCCEEDED( Result ) )
        {
#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Loop, "MediaCenterProxy::Receive end, function return %x \n", (uint32_t) Result );
#endif
            return Result;
        }
    }
    while ( --Timeout );

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Warning, "MediaCenterProxy::Receive end, function return %x \n", (uint32_t) GMI_WAIT_TIMEOUT );
    return GMI_WAIT_TIMEOUT;
}

GMI_RESULT MediaCenterProxy::OpenDevice1( uint16_t MessageId, uint16_t SensorId, uint16_t ChannelId, FD_HANDLE *Handle )
{
#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::OpenDevice1 begin, passed MessageId=%d, SensorId=%d, ChannelId=%d \n", MessageId, SensorId, ChannelId );
#endif

    size_t PayloadSize = sizeof(uint16_t)+sizeof(uint16_t);

    GMI_RESULT Result = m_MediaCenterProxyRequestPacket.AllocatePacketBuffer( 0, PayloadSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenDevice1, MediaCenterProxyRequestPacket.AllocatePacketBuffer return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = ApplicationPacket::FillPacketHeader( m_MediaCenterProxyRequestPacket.GetPacketHeaderBuffer(), (const uint8_t *) GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MESSAGE_FLAG_FIXED_CHECKSUM,
             GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION, ++m_SequenceNumber, GMI_MESSAGE_TYPE_REQUEST, MessageId );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenDevice1, ApplicationPacket::FillPacketHeader return %x \n", (uint32_t) Result );
        return Result;
    }

    uint8_t *Offset = m_MediaCenterProxyRequestPacket.GetPacketPayloadBuffer();
    // application data

    USHORT_TO_BIGENDIAN( SensorId, Offset );				// SensorId
    Offset += sizeof(uint16_t);

    USHORT_TO_BIGENDIAN( ChannelId, Offset );				// ChannelId
    Offset += sizeof(uint16_t);

    Result =  m_MediaCenterProxyRequestPacket.CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenDevice1, MediaCenterProxyRequestPacket.CalculatePacketChecksum return %x \n", (uint32_t) Result );
        return Result;
    }

    Result =  m_MediaCenterProxyRequestPacket.Submit( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenDevice1, MediaCenterProxyRequestPacket.Submit return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = Receive( MEDIA_CENTER_TIMEOUT );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenDevice1, Receive() return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_MediaCenterProxyReplyPacket.Create( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenDevice1, MediaCenterProxyReplyPacket.Create return %x \n", (uint32_t) Result );
        return Result;
    }

    Offset = m_MediaCenterProxyReplyPacket.GetPacketPayloadBuffer();
    BIGENDIAN_TO_UINT( Offset, Result );                     //Result
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenDevice1, function return %x \n", (uint32_t) Result );
        return Result;
    }
    Offset += sizeof(uint32_t);

    uint32_t Token = 0;
    BIGENDIAN_TO_UINT( Offset, Token );
    Offset += sizeof(uint32_t);

    *Handle = reinterpret_cast<FD_HANDLE>((size_t)Token);

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::OpenDevice1 end, passed MessageId=%d, SensorId=%d, ChannelId=%d, Handle=%p, function return %x \n", MessageId, SensorId, ChannelId, *Handle, (uint32_t) Result );
#endif
    return Result;
}

GMI_RESULT MediaCenterProxy::OpenDevice2( uint16_t MessageId, uint16_t SensorId, FD_HANDLE *Handle )
{
#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::OpenDevice2 begin, passed MessageId=%d, SensorId=%d \n", MessageId, SensorId );
#endif

    size_t PayloadSize = sizeof(uint16_t);

    GMI_RESULT Result = m_MediaCenterProxyRequestPacket.AllocatePacketBuffer( 0, PayloadSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenDevice2, MediaCenterProxyRequestPacket.AllocatePacketBuffer return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = ApplicationPacket::FillPacketHeader( m_MediaCenterProxyRequestPacket.GetPacketHeaderBuffer(), (const uint8_t *) GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MESSAGE_FLAG_FIXED_CHECKSUM,
             GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION, ++m_SequenceNumber, GMI_MESSAGE_TYPE_REQUEST, MessageId );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenDevice2, ApplicationPacket::FillPacketHeader return %x \n", (uint32_t) Result );
        return Result;
    }

    uint8_t *Offset = m_MediaCenterProxyRequestPacket.GetPacketPayloadBuffer();
    // application data

    USHORT_TO_BIGENDIAN( SensorId, Offset );				// SensorId
    Offset += sizeof(uint16_t);

    Result =  m_MediaCenterProxyRequestPacket.CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenDevice2, MediaCenterProxyRequestPacket.CalculatePacketChecksum return %x \n", (uint32_t) Result );
        return Result;
    }

    Result =  m_MediaCenterProxyRequestPacket.Submit( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenDevice2, MediaCenterProxyRequestPacket.Submit return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = Receive( MEDIA_CENTER_TIMEOUT );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenDevice2, Receive() return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_MediaCenterProxyReplyPacket.Create( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenDevice2, MediaCenterProxyReplyPacket.Create return %x \n", (uint32_t) Result );
        return Result;
    }

    Offset = m_MediaCenterProxyReplyPacket.GetPacketPayloadBuffer();
    BIGENDIAN_TO_UINT( Offset, Result );                     //Result
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OpenDevice2, function return %x \n", (uint32_t) Result );
        return Result;
    }
    Offset += sizeof(uint32_t);

    uint32_t Token = 0;
    BIGENDIAN_TO_UINT( Offset, Token );
    Offset += sizeof(uint32_t);

    *Handle = reinterpret_cast<FD_HANDLE>((size_t)Token);

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::OpenDevice2 end, passed MessageId=%d, SensorId=%d, Handle=%p, function return %x \n", MessageId, SensorId, *Handle, (uint32_t) Result );
#endif
    return Result;
}

GMI_RESULT MediaCenterProxy::CloseDevice( FD_HANDLE& Handle, uint16_t MessageId )
{
#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::CloseDevice begin, passed Handle=%p, MessageId=%d \n", Handle, MessageId );
#endif

    GMI_RESULT Result = OperationWithoutParameter( Handle, MessageId );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::CloseDevice, OperationWithoutParameter return %x \n", (uint32_t) Result );
        return Result;
    }

    Handle = NULL;

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::CloseDevice end, passed MessageId=%d, Handle=%p, function return %x \n", MessageId, Handle, (uint32_t) Result );
#endif
    return Result;
}

GMI_RESULT MediaCenterProxy::GetXXXConfig( FD_HANDLE Handle, uint16_t MessageId, void_t *Parameter, size_t *ParameterLength )
{
#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetXXXConfig begin, passed Handle=%p, MessageId=%d, ParameterLength=%d \n", Handle, MessageId, *ParameterLength );
#endif

    size_t PayloadSize = sizeof(uint32_t)+sizeof(uint16_t);

    GMI_RESULT Result = m_MediaCenterProxyRequestPacket.AllocatePacketBuffer( 0, PayloadSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetXXXConfig, MediaCenterProxyRequestPacket.AllocatePacketBuffer return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = ApplicationPacket::FillPacketHeader( m_MediaCenterProxyRequestPacket.GetPacketHeaderBuffer(), (const uint8_t *) GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MESSAGE_FLAG_FIXED_CHECKSUM,
             GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION, ++m_SequenceNumber, GMI_MESSAGE_TYPE_REQUEST, MessageId );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetXXXConfig, ApplicationPacket::FillPacketHeader return %x \n", (uint32_t) Result );
        return Result;
    }

    uint8_t *Offset = m_MediaCenterProxyRequestPacket.GetPacketPayloadBuffer();
    // application data

    uint32_t Token = (uint32_t)reinterpret_cast<size_t>(Handle);
    UINT_TO_BIGENDIAN( Token, Offset );				                 // Token
    Offset += sizeof(uint32_t);

    USHORT_TO_BIGENDIAN( (uint16_t)(*ParameterLength), Offset );     // ParameterLength
    Offset += sizeof(uint16_t);

    Result =  m_MediaCenterProxyRequestPacket.CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetXXXConfig, MediaCenterProxyRequestPacket.CalculatePacketChecksum return %x \n", (uint32_t) Result );
        return Result;
    }

    Result =  m_MediaCenterProxyRequestPacket.Submit( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetXXXConfig, MediaCenterProxyRequestPacket.Submit return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = Receive( MEDIA_CENTER_TIMEOUT );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetXXXConfig, Receive() return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_MediaCenterProxyReplyPacket.Create( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetXXXConfig, MediaCenterProxyReplyPacket.Create return %x \n", (uint32_t) Result );
        return Result;
    }

    Offset = m_MediaCenterProxyReplyPacket.GetPacketPayloadBuffer();
    BIGENDIAN_TO_UINT( Offset, Result );                     // Result
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetXXXConfig, function return %x \n", (uint32_t) Result );
        return Result;
    }
    Offset += sizeof(uint32_t);

    uint16_t Length = 0;
    BIGENDIAN_TO_USHORT( Offset, Length );                   // ParameterLength
    Offset += sizeof(uint16_t);

    if ( *ParameterLength < Length )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetXXXConfig, not enough space to return parameter(%d), actually need %d bytes, function return %x \n", *ParameterLength, Length, (uint32_t) GMI_NOT_ENOUGH_SPACE );
        return GMI_NOT_ENOUGH_SPACE;
    }

    *ParameterLength = Length;

    memcpy( Parameter, Offset, Length );
    Offset += Length;

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetXXXConfig end, returned ParameterLength=%d, function return %x \n", *ParameterLength, (uint32_t) Result );
#endif
    return Result;
}

GMI_RESULT MediaCenterProxy::SetXXXConfig( FD_HANDLE Handle, uint16_t MessageId, const void_t *Parameter, size_t ParameterLength )
{
#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetXXXConfig begin, passed Handle=%p, MessageId=%d, ParameterLength=%d \n", Handle, MessageId, ParameterLength );
#endif

    size_t PayloadSize = sizeof(uint32_t)+sizeof(uint16_t)+ParameterLength;

    GMI_RESULT Result = m_MediaCenterProxyRequestPacket.AllocatePacketBuffer( 0, PayloadSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetXXXConfig, MediaCenterProxyRequestPacket.AllocatePacketBuffer return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = ApplicationPacket::FillPacketHeader( m_MediaCenterProxyRequestPacket.GetPacketHeaderBuffer(), (const uint8_t *) GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MESSAGE_FLAG_FIXED_CHECKSUM,
             GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION, ++m_SequenceNumber, GMI_MESSAGE_TYPE_REQUEST, MessageId );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetXXXConfig, ApplicationPacket::FillPacketHeader return %x \n", (uint32_t) Result );
        return Result;
    }

    uint8_t *Offset = m_MediaCenterProxyRequestPacket.GetPacketPayloadBuffer();
    // application data

    uint32_t Token = (uint32_t)reinterpret_cast<size_t>(Handle);
    UINT_TO_BIGENDIAN( Token, Offset );				              // Token
    Offset += sizeof(uint32_t);

    USHORT_TO_BIGENDIAN( (uint16_t)ParameterLength, Offset );     // ParameterLength
    Offset += sizeof(uint16_t);

    memcpy( Offset, Parameter, ParameterLength );                 // Parameter
    Offset += ParameterLength;

    Result =  m_MediaCenterProxyRequestPacket.CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetXXXConfig, MediaCenterProxyRequestPacket.CalculatePacketChecksum return %x \n", (uint32_t) Result );
        return Result;
    }

    Result =  m_MediaCenterProxyRequestPacket.Submit( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetXXXConfig, MediaCenterProxyRequestPacket.Submit return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = Receive( MEDIA_CENTER_TIMEOUT );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetXXXConfig, Receive() return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_MediaCenterProxyReplyPacket.Create( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetXXXConfig, MediaCenterProxyReplyPacket.Create return %x \n", (uint32_t) Result );
        return Result;
    }

    Offset = m_MediaCenterProxyReplyPacket.GetPacketPayloadBuffer();
    BIGENDIAN_TO_UINT( Offset, Result );                     // Result
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetXXXConfig, function return %x \n", (uint32_t) Result );
        return Result;
    }
    Offset += sizeof(uint32_t);

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetXXXConfig end, function return %x \n", (uint32_t) Result );
#endif
    return Result;
}

GMI_RESULT MediaCenterProxy::OperationWithoutParameter    ( FD_HANDLE Handle, uint16_t MessageId )
{
#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::OperationWithoutParameter begin, passed Handle=%p, MessageId=%d \n", Handle, MessageId );
#endif

    size_t PayloadSize = sizeof(uint32_t);

    GMI_RESULT Result = m_MediaCenterProxyRequestPacket.AllocatePacketBuffer( 0, PayloadSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OperationWithoutParameter, MediaCenterProxyRequestPacket.AllocatePacketBuffer return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = ApplicationPacket::FillPacketHeader( m_MediaCenterProxyRequestPacket.GetPacketHeaderBuffer(), (const uint8_t *) GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MESSAGE_FLAG_FIXED_CHECKSUM,
             GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION, ++m_SequenceNumber, GMI_MESSAGE_TYPE_REQUEST, MessageId );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OperationWithoutParameter, ApplicationPacket::FillPacketHeader return %x \n", (uint32_t) Result );
        return Result;
    }

    uint8_t *Offset = m_MediaCenterProxyRequestPacket.GetPacketPayloadBuffer();
    // application data

    uint32_t Token = (uint32_t) reinterpret_cast<size_t>(Handle);
    UINT_TO_BIGENDIAN( Token, Offset );				// Token
    Offset += sizeof(uint32_t);

    Result =  m_MediaCenterProxyRequestPacket.CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OperationWithoutParameter, MediaCenterProxyRequestPacket.CalculatePacketChecksum return %x \n", (uint32_t) Result );
        return Result;
    }

    Result =  m_MediaCenterProxyRequestPacket.Submit( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OperationWithoutParameter, MediaCenterProxyRequestPacket.Submit return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = Receive( MEDIA_CENTER_TIMEOUT );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OperationWithoutParameter, Receive() return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_MediaCenterProxyReplyPacket.Create( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OperationWithoutParameter, MediaCenterProxyReplyPacket.Create return %x \n", (uint32_t) Result );
        return Result;
    }

    Offset = m_MediaCenterProxyReplyPacket.GetPacketPayloadBuffer();
    BIGENDIAN_TO_UINT( Offset, Result );                     //Result
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::OperationWithoutParameter, function return %x \n", (uint32_t) Result );
        return Result;
    }
    Offset += sizeof(uint32_t);

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::OperationWithoutParameter end, passed MessageId=%d, Handle=%p, function return %x \n", MessageId, Handle, (uint32_t) Result );
#endif
    return Result;
}

GMI_RESULT MediaCenterProxy::GetOperationWithOneParameter ( FD_HANDLE Handle, uint16_t MessageId, int32_t *Parameter1 )
{
#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetOperationWithOneParameter begin, passed Handle=%p, MessageId=%d, \n", Handle, MessageId );
#endif

    size_t PayloadSize = sizeof(uint32_t);

    GMI_RESULT Result = m_MediaCenterProxyRequestPacket.AllocatePacketBuffer( 0, PayloadSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetOperationWithOneParameter, MediaCenterProxyRequestPacket.AllocatePacketBuffer return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = ApplicationPacket::FillPacketHeader( m_MediaCenterProxyRequestPacket.GetPacketHeaderBuffer(), (const uint8_t *) GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MESSAGE_FLAG_FIXED_CHECKSUM,
             GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION, ++m_SequenceNumber, GMI_MESSAGE_TYPE_REQUEST, MessageId );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetOperationWithOneParameter, ApplicationPacket::FillPacketHeader return %x \n", (uint32_t) Result );
        return Result;
    }

    uint8_t *Offset = m_MediaCenterProxyRequestPacket.GetPacketPayloadBuffer();
    // application data

    uint32_t Token = (uint32_t)reinterpret_cast<size_t>(Handle);
    UINT_TO_BIGENDIAN( Token, Offset );				                 // Token
    Offset += sizeof(uint32_t);

    Result =  m_MediaCenterProxyRequestPacket.CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetOperationWithOneParameter, MediaCenterProxyRequestPacket.CalculatePacketChecksum return %x \n", (uint32_t) Result );
        return Result;
    }

    Result =  m_MediaCenterProxyRequestPacket.Submit( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetOperationWithOneParameter, MediaCenterProxyRequestPacket.Submit return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = Receive( MEDIA_CENTER_TIMEOUT );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetOperationWithOneParameter, Receive() return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_MediaCenterProxyReplyPacket.Create( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetOperationWithOneParameter, MediaCenterProxyReplyPacket.Create return %x \n", (uint32_t) Result );
        return Result;
    }

    Offset = m_MediaCenterProxyReplyPacket.GetPacketPayloadBuffer();
    BIGENDIAN_TO_UINT( Offset, Result );                     // Result
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetOperationWithOneParameter, function return %x \n", (uint32_t) Result );
        return Result;
    }
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_UINT( Offset, *Parameter1 );              // Parameter1
    Offset += sizeof(uint32_t);

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetOperationWithOneParameter end, returned Parameter1=%d, function return %x \n", *Parameter1, (uint32_t) Result );
#endif
    return Result;
}

GMI_RESULT MediaCenterProxy::GetOperationWithTwoParameter ( FD_HANDLE Handle, uint16_t MessageId, int32_t *Parameter1, int32_t *Parameter2 )
{
#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetOperationWithTwoParameter begin, passed Handle=%p, MessageId=%d, \n", Handle, MessageId );
#endif

    size_t PayloadSize = sizeof(uint32_t);

    GMI_RESULT Result = m_MediaCenterProxyRequestPacket.AllocatePacketBuffer( 0, PayloadSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetOperationWithTwoParameter, MediaCenterProxyRequestPacket.AllocatePacketBuffer return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = ApplicationPacket::FillPacketHeader( m_MediaCenterProxyRequestPacket.GetPacketHeaderBuffer(), (const uint8_t *) GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MESSAGE_FLAG_FIXED_CHECKSUM,
             GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION, ++m_SequenceNumber, GMI_MESSAGE_TYPE_REQUEST, MessageId );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetOperationWithTwoParameter, ApplicationPacket::FillPacketHeader return %x \n", (uint32_t) Result );
        return Result;
    }

    uint8_t *Offset = m_MediaCenterProxyRequestPacket.GetPacketPayloadBuffer();
    // application data

    uint32_t Token = (uint32_t)reinterpret_cast<size_t>(Handle);
    UINT_TO_BIGENDIAN( Token, Offset );				                 // Token
    Offset += sizeof(uint32_t);

    Result =  m_MediaCenterProxyRequestPacket.CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetOperationWithTwoParameter, MediaCenterProxyRequestPacket.CalculatePacketChecksum return %x \n", (uint32_t) Result );
        return Result;
    }

    Result =  m_MediaCenterProxyRequestPacket.Submit( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetOperationWithTwoParameter, MediaCenterProxyRequestPacket.Submit return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = Receive( MEDIA_CENTER_TIMEOUT );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetOperationWithTwoParameter, Receive() return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_MediaCenterProxyReplyPacket.Create( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetOperationWithTwoParameter, MediaCenterProxyReplyPacket.Create return %x \n", (uint32_t) Result );
        return Result;
    }

    Offset = m_MediaCenterProxyReplyPacket.GetPacketPayloadBuffer();
    BIGENDIAN_TO_UINT( Offset, Result );                     // Result
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetOperationWithTwoParameter, function return %x \n", (uint32_t) Result );
        return Result;
    }
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_UINT( Offset, *Parameter1 );              // Parameter1
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_UINT( Offset, *Parameter2 );              // Parameter2
    Offset += sizeof(uint32_t);

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::GetOperationWithTwoParameter end, returned Parameter1=%d, Parameter2=%d, function return %x \n", *Parameter1, *Parameter2, (uint32_t) Result );
#endif
    return Result;
}

GMI_RESULT MediaCenterProxy::SetOperationWithOneParameter ( FD_HANDLE Handle, uint16_t MessageId, int32_t Parameter1 )
{
#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetOperationWithOneParameter begin, passed Handle=%p, MessageId=%d, Parameter1=%d \n", Handle, MessageId, Parameter1 );
#endif

    size_t PayloadSize = sizeof(uint32_t)+sizeof(uint32_t);

    GMI_RESULT Result = m_MediaCenterProxyRequestPacket.AllocatePacketBuffer( 0, PayloadSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetOperationWithOneParameter, MediaCenterProxyRequestPacket.AllocatePacketBuffer return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = ApplicationPacket::FillPacketHeader( m_MediaCenterProxyRequestPacket.GetPacketHeaderBuffer(), (const uint8_t *) GMI_MEDIA_CENTER_MESSAGE_TAG, GMI_MESSAGE_FLAG_FIXED_CHECKSUM,
             GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION, GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION, ++m_SequenceNumber, GMI_MESSAGE_TYPE_REQUEST, MessageId );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetOperationWithOneParameter, ApplicationPacket::FillPacketHeader return %x \n", (uint32_t) Result );
        return Result;
    }

    uint8_t *Offset = m_MediaCenterProxyRequestPacket.GetPacketPayloadBuffer();
    // application data

    uint32_t Token = (uint32_t)reinterpret_cast<size_t>(Handle);
    UINT_TO_BIGENDIAN( Token, Offset );				              // Token
    Offset += sizeof(uint32_t);

    UINT_TO_BIGENDIAN( Parameter1, Offset );                    // Parameter1
    Offset += sizeof(uint32_t);

    Result =  m_MediaCenterProxyRequestPacket.CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetOperationWithOneParameter, MediaCenterProxyRequestPacket.CalculatePacketChecksum return %x \n", (uint32_t) Result );
        return Result;
    }

    Result =  m_MediaCenterProxyRequestPacket.Submit( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetOperationWithOneParameter, MediaCenterProxyRequestPacket.Submit return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = Receive( MEDIA_CENTER_TIMEOUT );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetOperationWithOneParameter, Receive() return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_MediaCenterProxyReplyPacket.Create( m_UDP_Session );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetOperationWithOneParameter, MediaCenterProxyReplyPacket.Create return %x \n", (uint32_t) Result );
        return Result;
    }

    Offset = m_MediaCenterProxyReplyPacket.GetPacketPayloadBuffer();
    BIGENDIAN_TO_UINT( Offset, Result );                     // Result
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::SetOperationWithOneParameter, function return %x \n", (uint32_t) Result );
        return Result;
    }
    Offset += sizeof(uint32_t);

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenterProxy::SetOperationWithOneParameter end, function return %x \n", (uint32_t) Result );
#endif
    return Result;
}
