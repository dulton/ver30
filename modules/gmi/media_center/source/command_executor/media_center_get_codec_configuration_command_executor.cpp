#include "media_center_get_codec_configuration_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterGetCodecConfigurationCommandExecutor::MediaCenterGetCodecConfigurationCommandExecutor(void)
    : MediaCenterCommandExecutor( GMI_GET_CODEC_CONFIGURARTION, CT_ShortTask )
    , m_Token( 0 )
    , m_ParameterLength( 0 )
{
}

MediaCenterGetCodecConfigurationCommandExecutor::~MediaCenterGetCodecConfigurationCommandExecutor(void)
{
}

GMI_RESULT MediaCenterGetCodecConfigurationCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterGetCodecConfigurationCommandExecutor> GetCodecConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetCodecConfigurationCommandExecutor>() );
    if ( NULL == GetCodecConfigurationCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetCodecConfigurationCommandExecutor->m_Session = Packet->GetSession();
    GetCodecConfigurationCommandExecutor->m_Reply = Packet->Clone();
    GetCodecConfigurationCommandExecutor->SetParameter( m_MediaCenter );

    uint8_t *Offset = Packet->GetPacketPayloadBuffer();

    BIGENDIAN_TO_UINT( Offset, GetCodecConfigurationCommandExecutor->m_Token );               // Token
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_USHORT( Offset, GetCodecConfigurationCommandExecutor->m_ParameterLength );   // ParameterLength
    Offset += sizeof(uint16_t);

    CommandExecutor = GetCodecConfigurationCommandExecutor;

    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterGetCodecConfigurationCommandExecutor::Execute()
{
    SafePtr< uint8_t, DefaultObjectsDeleter> Confurataion( BaseMemoryManager::Instance().News<uint8_t>( m_ParameterLength ) );
    if ( NULL == Confurataion.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    size_t  ConfurationLength = m_ParameterLength;
    memset( Confurataion.GetPtr(), 0, ConfurationLength );

    FD_HANDLE CodecHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->GetCodecConfig( CodecHandle, Confurataion.GetPtr(), &ConfurationLength );
    if ( FAILED( m_Result ) )
    {
        ConfurationLength = 0;
    }

    size_t PayloadSize = sizeof(uint32_t)+sizeof(uint16_t)+ConfurationLength;
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

    USHORT_TO_BIGENDIAN( (uint16_t) ConfurationLength, Offset );
    Offset += sizeof(uint16_t);

    if ( 0 < ConfurationLength )
    {
        memcpy( Offset, Confurataion.GetPtr(), ConfurationLength );
        Offset += ConfurationLength;
    }

    Result =  Reply->CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        return Result;
    }

    m_Reply = Reply;

    return GMI_SUCCESS;
}
