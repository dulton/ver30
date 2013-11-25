#include "media_center_open_device2_command_executor.h"

#include "application_packet.h"

MediaCenterOpenDevice2CommandExecutor::MediaCenterOpenDevice2CommandExecutor( uint32_t CommandId, enum CommandType Type )
    : MediaCenterCommandExecutor( CommandId, Type )
    , m_SensorId( 0 )
    , m_Handle( NULL )
{
}

MediaCenterOpenDevice2CommandExecutor::~MediaCenterOpenDevice2CommandExecutor(void)
{
}

GMI_RESULT MediaCenterOpenDevice2CommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    MediaCenterOpenDevice2CommandExecutor *OpenDevice1CommandExecutor = (MediaCenterOpenDevice2CommandExecutor*) CommandExecutor.GetPtr();

    OpenDevice1CommandExecutor->m_Session = Packet->GetSession();
    OpenDevice1CommandExecutor->m_Reply = Packet->Clone();
    OpenDevice1CommandExecutor->SetParameter( m_MediaCenter );

    uint8_t *Offset = Packet->GetPacketPayloadBuffer();

    BIGENDIAN_TO_USHORT( Offset, OpenDevice1CommandExecutor->m_SensorId );  // SensorId
    Offset += sizeof(uint16_t);

    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterOpenDevice2CommandExecutor::Execute()
{
    uint16_t PayloadSize = sizeof(uint32_t)+sizeof(uint32_t);
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

    UINT_TO_BIGENDIAN( m_Result, Offset );				    // Result
    Offset += sizeof(uint32_t);

    uint32_t Token = (uint32_t)reinterpret_cast<size_t> (m_Handle);
    UINT_TO_BIGENDIAN( Token, Offset );				        // Token
    Offset += sizeof(uint32_t);

    Result =  Reply->CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        return Result;
    }

    m_Reply = Reply;

    return GMI_SUCCESS;
}
