#include "media_center_get_xxx_configuration_command_executor.h"

#include "application_packet.h"

MediaCenterGetXXXConfigurationCommandExecutor::MediaCenterGetXXXConfigurationCommandExecutor( uint32_t CommandId, enum CommandType Type )
    : MediaCenterCommandExecutor( CommandId, Type )
    , m_Token( 0 )
    , m_ClientParameterLength( 0 )
    , m_ReplyParameterLength( 0 )
    , m_Parameter()
{
}

MediaCenterGetXXXConfigurationCommandExecutor::~MediaCenterGetXXXConfigurationCommandExecutor(void)
{
}

GMI_RESULT MediaCenterGetXXXConfigurationCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    MediaCenterGetXXXConfigurationCommandExecutor *GetXXXConfigurationCommandExecutor = (MediaCenterGetXXXConfigurationCommandExecutor*) CommandExecutor.GetPtr();

    GetXXXConfigurationCommandExecutor->m_Session = Packet->GetSession();
    GetXXXConfigurationCommandExecutor->m_Reply = Packet->Clone();
    GetXXXConfigurationCommandExecutor->SetParameter( m_MediaCenter );

    uint8_t *Offset = Packet->GetPacketPayloadBuffer();

    BIGENDIAN_TO_UINT( Offset, GetXXXConfigurationCommandExecutor->m_Token );                     // Token
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_USHORT( Offset, GetXXXConfigurationCommandExecutor->m_ClientParameterLength );   // ParameterLength
    Offset += sizeof(uint16_t);

    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterGetXXXConfigurationCommandExecutor::Execute()
{
    size_t PayloadSize = sizeof(uint32_t)+sizeof(uint16_t)+m_ReplyParameterLength;
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

    USHORT_TO_BIGENDIAN( (uint16_t) m_ReplyParameterLength, Offset );
    Offset += sizeof(uint16_t);

    if ( 0 < m_ReplyParameterLength )
    {
        memcpy( Offset, m_Parameter.GetPtr(), m_ReplyParameterLength );
        Offset += m_ReplyParameterLength;
    }

    Result =  Reply->CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        return Result;
    }

    m_Reply = Reply;

    return GMI_SUCCESS;
}
