#include "media_center_create_codec_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterCreateCodecCommandExecutor::MediaCenterCreateCodecCommandExecutor(void)
    : MediaCenterCommandExecutor( GMI_CREATE_CODEC, CT_ShortTask )
    , m_CodecMode( 0 )
    , m_SourceId( 0 )
    , m_MediaId( 0 )
    , m_MediaType( 0 )
    , m_CodecType( 0 )
    , m_CodecParameter()
    , m_CodecParameterLength( 0 )
{
}

MediaCenterCreateCodecCommandExecutor::~MediaCenterCreateCodecCommandExecutor(void)
{
}

GMI_RESULT MediaCenterCreateCodecCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterCreateCodecCommandExecutor> CreateCodecCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterCreateCodecCommandExecutor>() );
    if ( NULL == CreateCodecCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CreateCodecCommandExecutor->m_Session = Packet->GetSession();
    CreateCodecCommandExecutor->m_Reply = Packet->Clone();
    CreateCodecCommandExecutor->SetParameter( m_MediaCenter );

    uint8_t *Offset = Packet->GetPacketPayloadBuffer();

    CreateCodecCommandExecutor->m_CodecMode = *Offset++;

    ++Offset;
    ++Offset;
    ++Offset;

    BIGENDIAN_TO_UINT( Offset, CreateCodecCommandExecutor->m_SourceId );               // SourceId
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_UINT( Offset, CreateCodecCommandExecutor->m_MediaId );                // MediaId
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_UINT( Offset, CreateCodecCommandExecutor->m_MediaType );              // MediaType
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_UINT( Offset, CreateCodecCommandExecutor->m_CodecType );              // CodecType
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_USHORT( Offset, CreateCodecCommandExecutor->m_CodecParameterLength ); // CodecParameterLength
    Offset += sizeof(uint16_t);

    CreateCodecCommandExecutor->m_CodecParameter = BaseMemoryManager::Instance().News<uint8_t>( CreateCodecCommandExecutor->m_CodecParameterLength );
    if ( NULL == CreateCodecCommandExecutor->m_CodecParameter.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }
    memcpy( CreateCodecCommandExecutor->m_CodecParameter.GetPtr(), Offset, CreateCodecCommandExecutor->m_CodecParameterLength );	// CodecParameter
    Offset += CreateCodecCommandExecutor->m_CodecParameterLength;

    CommandExecutor = CreateCodecCommandExecutor;

    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterCreateCodecCommandExecutor::Execute()
{
    FD_HANDLE CodecHandle = NULL;
    m_Result = m_MediaCenter->CreateCodec( (( 1 == m_CodecMode ) ? true : false), m_SourceId, m_MediaId, m_MediaType, m_CodecType, m_CodecParameter.GetPtr(), m_CodecParameterLength, &CodecHandle );

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

    UINT_TO_BIGENDIAN( m_Result, Offset );				        // Result
    Offset += sizeof(uint32_t);

    uint32_t Token = (uint32_t)reinterpret_cast<size_t> (CodecHandle);
    UINT_TO_BIGENDIAN( Token, Offset );				            // Token
    Offset += sizeof(uint32_t);

    Result =  Reply->CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        return Result;
    }

    m_Reply = Reply;

    return GMI_SUCCESS;
}
