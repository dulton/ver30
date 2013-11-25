#include "media_center_set_codec_configuration_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterSetCodecConfigurationCommandExecutor::MediaCenterSetCodecConfigurationCommandExecutor(void)
    : MediaCenterCommandExecutor( GMI_SET_CODEC_CONFIGURARTION, CT_ShortTask )
    , m_Token( 0 )
    , m_Parameter()
    , m_ParameterLength( 0 )
{
}

MediaCenterSetCodecConfigurationCommandExecutor::~MediaCenterSetCodecConfigurationCommandExecutor(void)
{
}

GMI_RESULT MediaCenterSetCodecConfigurationCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterSetCodecConfigurationCommandExecutor> SetCodecConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetCodecConfigurationCommandExecutor>() );
    if ( NULL == SetCodecConfigurationCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    SetCodecConfigurationCommandExecutor->m_Session = Packet->GetSession();
    SetCodecConfigurationCommandExecutor->m_Reply = Packet->Clone();
    SetCodecConfigurationCommandExecutor->SetParameter( m_MediaCenter );

    uint8_t *Offset = Packet->GetPacketPayloadBuffer();

    BIGENDIAN_TO_UINT( Offset, SetCodecConfigurationCommandExecutor->m_Token );              // Token
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_USHORT( Offset, SetCodecConfigurationCommandExecutor->m_ParameterLength );  // Codec ParameterLength
    Offset += sizeof(uint16_t);

    SetCodecConfigurationCommandExecutor->m_Parameter = BaseMemoryManager::Instance().News<uint8_t>( SetCodecConfigurationCommandExecutor->m_ParameterLength );
    if ( NULL == SetCodecConfigurationCommandExecutor->m_Parameter.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }
    memcpy( SetCodecConfigurationCommandExecutor->m_Parameter.GetPtr(), Offset, SetCodecConfigurationCommandExecutor->m_ParameterLength );	// Codec Parameter
    Offset += SetCodecConfigurationCommandExecutor->m_ParameterLength;

    CommandExecutor = SetCodecConfigurationCommandExecutor;

    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterSetCodecConfigurationCommandExecutor::Execute()
{
    FD_HANDLE CodecHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->SetCodecConfig( CodecHandle, m_Parameter.GetPtr(), m_ParameterLength );

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
