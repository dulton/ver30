#include "media_center_force_generate_idr_frame_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterForceGenerateIdrFrameCommandExecutor::MediaCenterForceGenerateIdrFrameCommandExecutor(void)
    : MediaCenterCommandExecutor( GMI_FORCE_GENERATE_IDR_FRAME, CT_ShortTask )
    , m_Token( 0 )
{
}

MediaCenterForceGenerateIdrFrameCommandExecutor::~MediaCenterForceGenerateIdrFrameCommandExecutor(void)
{
}

GMI_RESULT MediaCenterForceGenerateIdrFrameCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterForceGenerateIdrFrameCommandExecutor> ForceGenerateIdrFrameCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterForceGenerateIdrFrameCommandExecutor>() );
    if ( NULL == ForceGenerateIdrFrameCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    ForceGenerateIdrFrameCommandExecutor->m_Session = Packet->GetSession();
    ForceGenerateIdrFrameCommandExecutor->m_Reply = Packet->Clone();
    ForceGenerateIdrFrameCommandExecutor->SetParameter( m_MediaCenter );

    uint8_t *Offset = Packet->GetPacketPayloadBuffer();

    BIGENDIAN_TO_UINT( Offset, ForceGenerateIdrFrameCommandExecutor->m_Token );                   // Token
    Offset += sizeof(uint32_t);

    CommandExecutor = ForceGenerateIdrFrameCommandExecutor;

    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterForceGenerateIdrFrameCommandExecutor::Execute()
{
    FD_HANDLE CodecHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->ForceGenerateIdrFrame( CodecHandle );

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
