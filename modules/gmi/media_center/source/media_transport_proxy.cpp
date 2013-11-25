#include "media_transport_proxy.h"

#include "log_client.h"
#include "rtsp_server.h"

MediaTransportProxy::MediaTransportProxy(void)
    : m_RtspServer( NULL )
{
}

MediaTransportProxy::~MediaTransportProxy(void)
{
}

GMI_RESULT MediaTransportProxy::Initialize( uint16_t ServerPort )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaTransportProxy::Initialize begin, passed ServerPort=%p \n", ntohs(ServerPort) );

    m_RtspServer = BaseMemoryManager::Instance().New<GMI_RtspServer>();
    if ( NULL == m_RtspServer.GetPtr() )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaTransportProxy::Initialize, GMI_RtspServer object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    GMI_RESULT Result = m_RtspServer->Initialize( ServerPort );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaTransportProxy::Initialize, GMI_RtspServer object initialization failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_RtspServer->Start();
    if ( FAILED( Result ) )
    {
        m_RtspServer->Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaTransportProxy::Initialize, GMI_RtspServer object start failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaTransportProxy::Initialize end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaTransportProxy::Deinitialize()
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaTransportProxy::Deinitialize begin \n" );

    if ( NULL == m_RtspServer )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaTransportProxy::Deinitialize, object is already deinitialized, function return %x \n", (uint32_t) GMI_SUCCESS );
        return GMI_SUCCESS;
    }

    GMI_RESULT Result = m_RtspServer->Stop();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaTransportProxy::Deinitialize, RtspServer.Stop return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = m_RtspServer->Deinitialize();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaTransportProxy::Deinitialize, RtspServer.Deinitialize return %x \n", (uint32_t) Result );
        return Result;
    }

    m_RtspServer = NULL;
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaTransportProxy::Deinitialize end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaTransportProxy::Start( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, void_t *CodecParameter, size_t CodecParameterLength, FD_HANDLE *TransportHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaTransportProxy::Start begin, passed EncodeMode=%d, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d, CodecParameterLength=%d \n", EncodeMode, SourceId, MediaId, MediaType, CodecType, CodecParameterLength );

    GMI_RESULT Result = m_RtspServer->Add( EncodeMode, SourceId, MediaId, MediaType, CodecType, CodecParameter, CodecParameterLength, TransportHandle );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaTransportProxy::Start, RtspServer.Add return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaTransportProxy::Start end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaTransportProxy::Stop( FD_HANDLE TransportHandle )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaTransportProxy::Stop begin, passed TransportHandle=%p \n", TransportHandle );

    GMI_RESULT Result = m_RtspServer->Remove( TransportHandle );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaTransportProxy::Stop, RtspServer.Remove return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaTransportProxy::Stop end, function return %x \n", (uint32_t) Result );
    return Result;
}

void_t MediaTransportProxy::SetCallback( RTSP_EVENT_CALLBACK Callback, void_t *UserData )
{
    if ( NULL != m_RtspServer.GetPtr() )
    {
        m_RtspServer->SetCallback( Callback, UserData );
    }
}
