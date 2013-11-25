#include "media_center_set_xxx_configuration_command_executor.h"

#include "application_packet.h"

MediaCenterSetXXXConfigurationCommandExecutor::MediaCenterSetXXXConfigurationCommandExecutor( uint32_t CommandId, enum CommandType Type )
    : MediaCenterCommandExecutor( CommandId, Type )
    , m_Token( 0 )
    , m_Parameter()
    , m_ParameterLength( 0 )
{
}

MediaCenterSetXXXConfigurationCommandExecutor::~MediaCenterSetXXXConfigurationCommandExecutor(void)
{
}

GMI_RESULT MediaCenterSetXXXConfigurationCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    MediaCenterSetXXXConfigurationCommandExecutor *SetXXXConfigurationCommandExecutor = (MediaCenterSetXXXConfigurationCommandExecutor*) CommandExecutor.GetPtr();

    SetXXXConfigurationCommandExecutor->m_Session = Packet->GetSession();
    SetXXXConfigurationCommandExecutor->m_Reply = Packet->Clone();
    SetXXXConfigurationCommandExecutor->SetParameter( m_MediaCenter );

    uint8_t *Offset = Packet->GetPacketPayloadBuffer();

    BIGENDIAN_TO_UINT( Offset, SetXXXConfigurationCommandExecutor->m_Token );              // Token
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_USHORT( Offset, SetXXXConfigurationCommandExecutor->m_ParameterLength );  // Codec ParameterLength
    Offset += sizeof(uint16_t);

    SetXXXConfigurationCommandExecutor->m_Parameter = BaseMemoryManager::Instance().News<uint8_t>( SetXXXConfigurationCommandExecutor->m_ParameterLength );
    if ( NULL == SetXXXConfigurationCommandExecutor->m_Parameter.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }
    memcpy( SetXXXConfigurationCommandExecutor->m_Parameter.GetPtr(), Offset, SetXXXConfigurationCommandExecutor->m_ParameterLength );	// Codec Parameter
    Offset += SetXXXConfigurationCommandExecutor->m_ParameterLength;

    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterSetXXXConfigurationCommandExecutor::Execute()
{
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
