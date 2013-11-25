#include "media_center_set_vin_vout_configuration_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterSetVinVoutConfigurationCommandExecutor::MediaCenterSetVinVoutConfigurationCommandExecutor(void)
    : MediaCenterCommandExecutor( GMI_SET_VIN_VOUT_CONFIGURATION, CT_ShortTask )
    , m_Token( 0 )
    , m_VinParameter()
    , m_VinParameterLength( 0 )
    , m_VoutParameter()
    , m_VoutParameterLength( 0 )
{
}

MediaCenterSetVinVoutConfigurationCommandExecutor::~MediaCenterSetVinVoutConfigurationCommandExecutor(void)
{
}

GMI_RESULT MediaCenterSetVinVoutConfigurationCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterSetVinVoutConfigurationCommandExecutor> SetVinVoutonfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetVinVoutConfigurationCommandExecutor>() );
    if ( NULL == SetVinVoutonfigurationCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    SetVinVoutonfigurationCommandExecutor->m_Session = Packet->GetSession();
    SetVinVoutonfigurationCommandExecutor->m_Reply = Packet->Clone();
    SetVinVoutonfigurationCommandExecutor->SetParameter( m_MediaCenter );

    uint8_t *Offset = Packet->GetPacketPayloadBuffer();

    BIGENDIAN_TO_UINT( Offset, SetVinVoutonfigurationCommandExecutor->m_Token );              // Token
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_USHORT( Offset, SetVinVoutonfigurationCommandExecutor->m_VinParameterLength );   // Vin ParameterLength
    Offset += sizeof(uint16_t);

    SetVinVoutonfigurationCommandExecutor->m_VinParameter = BaseMemoryManager::Instance().News<uint8_t>( SetVinVoutonfigurationCommandExecutor->m_VinParameterLength );
    if ( NULL == SetVinVoutonfigurationCommandExecutor->m_VinParameter.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }
    memcpy( SetVinVoutonfigurationCommandExecutor->m_VinParameter.GetPtr(), Offset, SetVinVoutonfigurationCommandExecutor->m_VinParameterLength );	   // Vin Parameter
    Offset += SetVinVoutonfigurationCommandExecutor->m_VinParameterLength;

    BIGENDIAN_TO_USHORT( Offset, SetVinVoutonfigurationCommandExecutor->m_VoutParameterLength );  // Vout ParameterLength
    Offset += sizeof(uint16_t);

    SetVinVoutonfigurationCommandExecutor->m_VoutParameter = BaseMemoryManager::Instance().News<uint8_t>( SetVinVoutonfigurationCommandExecutor->m_VoutParameterLength );
    if ( NULL == SetVinVoutonfigurationCommandExecutor->m_VoutParameter.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }
    memcpy( SetVinVoutonfigurationCommandExecutor->m_VoutParameter.GetPtr(), Offset, SetVinVoutonfigurationCommandExecutor->m_VoutParameterLength );	// Vout Parameter
    Offset += SetVinVoutonfigurationCommandExecutor->m_VoutParameterLength;

    CommandExecutor = SetVinVoutonfigurationCommandExecutor;

    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterSetVinVoutConfigurationCommandExecutor::Execute()
{
    FD_HANDLE VinVoutHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->SetVinVoutConfig( VinVoutHandle, m_VinParameter.GetPtr(), m_VinParameterLength, m_VoutParameter.GetPtr(), m_VoutParameterLength );

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
