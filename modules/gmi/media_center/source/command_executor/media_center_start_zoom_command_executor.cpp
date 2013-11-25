#include "media_center_start_zoom_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterStartZoomCommandExecutor::MediaCenterStartZoomCommandExecutor()
    : MediaCenterCommandExecutor( GMI_START_ZOOM, CT_ShortTask )
    , m_AutoFocusHandle( 0 )
    , m_AutoFocusControlStatus( 0 )
    , m_ZoomHandle( 0 )
    , m_ZoomMode( 0 )
{
}

MediaCenterStartZoomCommandExecutor::~MediaCenterStartZoomCommandExecutor(void)
{
}

GMI_RESULT MediaCenterStartZoomCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterStartZoomCommandExecutor> StartZoomCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterStartZoomCommandExecutor>() );
    if ( NULL == StartZoomCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    StartZoomCommandExecutor->m_Session = Packet->GetSession();
    StartZoomCommandExecutor->m_Reply = Packet->Clone();
    StartZoomCommandExecutor->SetParameter( m_MediaCenter );

    uint8_t *Offset = Packet->GetPacketPayloadBuffer();

    BIGENDIAN_TO_UINT( Offset, StartZoomCommandExecutor->m_AutoFocusHandle );          // AutoFocusHandle
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_UINT( Offset, StartZoomCommandExecutor->m_AutoFocusControlStatus );   // AutoFocusControlStatus
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_UINT( Offset, StartZoomCommandExecutor->m_ZoomHandle );               // ZoomHandle
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_UINT( Offset, StartZoomCommandExecutor->m_ZoomMode );                 // ZoomMode
    Offset += sizeof(uint32_t);

    CommandExecutor = StartZoomCommandExecutor;

    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterStartZoomCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_AutoFocusHandle);
    m_Result = m_MediaCenter->PauseAutoFocus( AutoFocusHandle, (int8_t) m_AutoFocusControlStatus );
    if ( SUCCEEDED( m_Result ) )
    {
        FD_HANDLE ZoomHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_ZoomHandle);
        m_Result = m_MediaCenter->ControlZoom( ZoomHandle, (int8_t) m_ZoomMode );
    }

    uint16_t PayloadSize = sizeof(uint32_t);
    ReferrencePtr<ApplicationPacket> Reply( BaseMemoryManager::Instance().New<ApplicationPacket>() );
    if ( NULL == Reply.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    GMI_RESULT Result = Reply->AllocatePacketBuffer( 0, PayloadSize );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    ApplicationPacket *CommandPacket = (ApplicationPacket*) m_Reply.GetPtr();

    uint8_t MessageType = GMI_MESSAGE_TYPE_REPLY;

    Result = ApplicationPacket::FillPacketHeader( Reply->GetPacketHeaderBuffer(), CommandPacket->GetMessageTag(), CommandPacket->GetHeaderFlags(),
             CommandPacket->GetMajorVersion(), CommandPacket->GetMinorVersion(), CommandPacket->GetSequenceNumber(),
             MessageType, CommandPacket->GetMessageId() );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    uint8_t *PayloadBuffer = Reply->GetPacketPayloadBuffer();
    uint8_t *Offset = PayloadBuffer;

    UINT_TO_BIGENDIAN( m_Result, Offset );				        // Result
    Offset += sizeof(uint32_t);

    Result =  Reply->CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        return Result;
    }

    m_Reply = Reply;

    return GMI_SUCCESS;
}
