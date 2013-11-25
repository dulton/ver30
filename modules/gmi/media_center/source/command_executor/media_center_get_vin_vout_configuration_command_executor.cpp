#include "media_center_get_vin_vout_configuration_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterGetVinVoutConfigurationCommandExecutor::MediaCenterGetVinVoutConfigurationCommandExecutor(void)
    : MediaCenterCommandExecutor( GMI_GET_VIN_VOUT_CONFIGURATION, CT_ShortTask )
    , m_Token( 0 )
    , m_VinParameterLength( 0 )
    , m_VoutParameterLength( 0 )
{
}

MediaCenterGetVinVoutConfigurationCommandExecutor::~MediaCenterGetVinVoutConfigurationCommandExecutor(void)
{
}

GMI_RESULT MediaCenterGetVinVoutConfigurationCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterGetVinVoutConfigurationCommandExecutor> GetVinVoutConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetVinVoutConfigurationCommandExecutor>() );
    if ( NULL == GetVinVoutConfigurationCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetVinVoutConfigurationCommandExecutor->m_Session = Packet->GetSession();
    GetVinVoutConfigurationCommandExecutor->m_Reply = Packet->Clone();
    GetVinVoutConfigurationCommandExecutor->SetParameter( m_MediaCenter );

    uint8_t *Offset = Packet->GetPacketPayloadBuffer();

    BIGENDIAN_TO_UINT( Offset, GetVinVoutConfigurationCommandExecutor->m_Token );                  // Token
    Offset += sizeof(uint32_t);

    BIGENDIAN_TO_USHORT( Offset, GetVinVoutConfigurationCommandExecutor->m_VinParameterLength );   // VinParameterLength
    Offset += sizeof(uint16_t);

    BIGENDIAN_TO_USHORT( Offset, GetVinVoutConfigurationCommandExecutor->m_VoutParameterLength );  // VoutParameterLength
    Offset += sizeof(uint16_t);

    CommandExecutor = GetVinVoutConfigurationCommandExecutor;

    return GMI_SUCCESS;
}

GMI_RESULT MediaCenterGetVinVoutConfigurationCommandExecutor::Execute()
{
    SafePtr< uint8_t, DefaultObjectsDeleter> VinConfurataion( BaseMemoryManager::Instance().News<uint8_t>( m_VinParameterLength ) );
    if ( NULL == VinConfurataion.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    size_t  VinConfurationLength = m_VinParameterLength;
    memset( VinConfurataion.GetPtr(), 0, VinConfurationLength );

    SafePtr< uint8_t, DefaultObjectsDeleter> VoutConfurataion( BaseMemoryManager::Instance().News<uint8_t>( m_VoutParameterLength ) );
    if ( NULL == VoutConfurataion.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    size_t  VoutConfurationLength = m_VoutParameterLength;
    memset( VoutConfurataion.GetPtr(), 0, VoutConfurationLength );

    FD_HANDLE VinVoutHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->GetVinVoutConfig( VinVoutHandle, VinConfurataion.GetPtr(), &VinConfurationLength, VoutConfurataion.GetPtr(), &VoutConfurationLength );
    if ( FAILED( m_Result ) )
    {
        VinConfurationLength = 0;
        VoutConfurationLength = 0;
    }

    size_t PayloadSize = sizeof(uint32_t)+sizeof(uint16_t)+VinConfurationLength+sizeof(uint16_t)+VoutConfurationLength;
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

    USHORT_TO_BIGENDIAN( (uint16_t) VinConfurationLength, Offset );
    Offset += sizeof(uint16_t);

    if ( 0 < VinConfurationLength )
    {
        memcpy( Offset, VinConfurataion.GetPtr(), VinConfurationLength );
        Offset += VinConfurationLength;
    }

    USHORT_TO_BIGENDIAN( (uint16_t) VoutConfurationLength, Offset );
    Offset += sizeof(uint16_t);

    if ( 0 < VoutConfurationLength )
    {
        memcpy( Offset, VoutConfurataion.GetPtr(), VoutConfurationLength );
        Offset += VoutConfurationLength;
    }

    Result =  Reply->CalculatePacketChecksum();
    if ( FAILED( Result ) )
    {
        return Result;
    }

    m_Reply = Reply;

    return GMI_SUCCESS;
}
