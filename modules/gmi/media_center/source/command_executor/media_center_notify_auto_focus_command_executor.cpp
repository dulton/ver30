#include "media_center_notify_auto_focus_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterNotifyAutoFocusCommandExecutor::MediaCenterNotifyAutoFocusCommandExecutor()
    : MediaCenterCommandExecutor( GMI_NOTIFY_AUTO_FOCUS, CT_ShortTask )
    , m_Token( 0 )
    , m_EventType( 0 )
    , m_ExtDataLength( 0 )
    , m_ExtData( NULL )
{
}

MediaCenterNotifyAutoFocusCommandExecutor::~MediaCenterNotifyAutoFocusCommandExecutor(void)
{
}

GMI_RESULT MediaCenterNotifyAutoFocusCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterNotifyAutoFocusCommandExecutor> NotifyAutoFocusCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterNotifyAutoFocusCommandExecutor>() );
    if ( NULL == NotifyAutoFocusCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    NotifyAutoFocusCommandExecutor->m_Session = Packet->GetSession();
    NotifyAutoFocusCommandExecutor->m_Reply = Packet->Clone();
    NotifyAutoFocusCommandExecutor->SetParameter( m_MediaCenter );

    uint8_t *Offset = Packet->GetPacketPayloadBuffer();

    BIGENDIAN_TO_UINT( Offset, NotifyAutoFocusCommandExecutor->m_Token );         // Token
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_UINT( Offset, NotifyAutoFocusCommandExecutor->m_EventType );     // EventType
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_UINT( Offset, NotifyAutoFocusCommandExecutor->m_ExtDataLength ); // Length
    Offset += sizeof(uint32_t);

    NotifyAutoFocusCommandExecutor->m_ExtData = BaseMemoryManager::Instance().News<uint8_t>( NotifyAutoFocusCommandExecutor->m_ExtDataLength );
    if ( NULL == NotifyAutoFocusCommandExecutor->m_ExtData.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    memcpy( NotifyAutoFocusCommandExecutor->m_ExtData.GetPtr(), Offset, NotifyAutoFocusCommandExecutor->m_ExtDataLength );
    Offset += NotifyAutoFocusCommandExecutor->m_ExtDataLength;

    CommandExecutor = NotifyAutoFocusCommandExecutor;

    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterNotifyAutoFocusCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->NotifyAutoFocus( AutoFocusHandle, m_EventType, m_ExtData.GetPtr(), m_ExtDataLength );

    size_t PayloadSize = sizeof(uint32_t);
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

    UINT_TO_BIGENDIAN( m_Result, Offset );		// Result
    Offset += sizeof(uint32_t);

    Result =  Reply->CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        return Result;
    }

    m_Reply = Reply;

    return GMI_SUCCESS;
}
