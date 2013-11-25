#include "media_center_set_osd_configuration_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterSetOsdConfigurationCommandExecutor::MediaCenterSetOsdConfigurationCommandExecutor(void)
    : MediaCenterCommandExecutor( GMI_SET_OSD_CONFIGURARTION, CT_ShortTask )
    , m_Token( 0 )
    , m_Parameter()
    , m_ParameterLength( 0 )
{
}

MediaCenterSetOsdConfigurationCommandExecutor::~MediaCenterSetOsdConfigurationCommandExecutor(void)
{
}

GMI_RESULT MediaCenterSetOsdConfigurationCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterSetOsdConfigurationCommandExecutor> SetOsdConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetOsdConfigurationCommandExecutor>() );
    if ( NULL == SetOsdConfigurationCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    SetOsdConfigurationCommandExecutor->m_Session = Packet->GetSession();
    SetOsdConfigurationCommandExecutor->m_Reply = Packet->Clone();
    SetOsdConfigurationCommandExecutor->SetParameter( m_MediaCenter );

    uint8_t *Offset = Packet->GetPacketPayloadBuffer();

    BIGENDIAN_TO_UINT( Offset, SetOsdConfigurationCommandExecutor->m_Token );              // Token
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_USHORT( Offset, SetOsdConfigurationCommandExecutor->m_ParameterLength );  // Osd ParameterLength
    Offset += sizeof(uint16_t);

    SetOsdConfigurationCommandExecutor->m_Parameter = BaseMemoryManager::Instance().News<uint8_t>( SetOsdConfigurationCommandExecutor->m_ParameterLength );
    if ( NULL == SetOsdConfigurationCommandExecutor->m_Parameter.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }
    memcpy( SetOsdConfigurationCommandExecutor->m_Parameter.GetPtr(), Offset, SetOsdConfigurationCommandExecutor->m_ParameterLength );	// Osd Parameter
    Offset += SetOsdConfigurationCommandExecutor->m_ParameterLength;

    CommandExecutor = SetOsdConfigurationCommandExecutor;

    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterSetOsdConfigurationCommandExecutor::Execute()
{
    FD_HANDLE CodecHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->SetOsdConfig( CodecHandle, m_Parameter.GetPtr(), m_ParameterLength );

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
