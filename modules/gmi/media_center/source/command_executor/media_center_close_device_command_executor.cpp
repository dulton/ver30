#include "media_center_close_device_command_executor.h"

#include "application_packet.h"

MediaCenterCloseDeviceCommandExecutor::MediaCenterCloseDeviceCommandExecutor( uint32_t CommandId, enum CommandType Type )
    : MediaCenterCommandExecutor( CommandId, Type )
    , m_Token( 0 )
{
}

MediaCenterCloseDeviceCommandExecutor::~MediaCenterCloseDeviceCommandExecutor(void)
{
}

GMI_RESULT MediaCenterCloseDeviceCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    MediaCenterCloseDeviceCommandExecutor *CloseDeviceCommandExecutor = (MediaCenterCloseDeviceCommandExecutor*) CommandExecutor.GetPtr();

    CloseDeviceCommandExecutor->m_Session = Packet->GetSession();
    CloseDeviceCommandExecutor->m_Reply = Packet->Clone();
    CloseDeviceCommandExecutor->SetParameter( m_MediaCenter );

    uint8_t *Offset = Packet->GetPacketPayloadBuffer();

    BIGENDIAN_TO_UINT( Offset, CloseDeviceCommandExecutor->m_Token );               // Token
    Offset += sizeof(uint32_t);

    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterCloseDeviceCommandExecutor::Execute()
{
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
