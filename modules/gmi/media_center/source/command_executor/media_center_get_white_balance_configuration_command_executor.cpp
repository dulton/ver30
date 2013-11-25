#include "media_center_get_white_balance_configuration_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterGetWhiteBalanceConfigurationCommandExecutor::MediaCenterGetWhiteBalanceConfigurationCommandExecutor(void)
    : MediaCenterGetXXXConfigurationCommandExecutor( GMI_GET_WHITE_BALANCE_CONFIGURARTION, CT_ShortTask )
{
}

MediaCenterGetWhiteBalanceConfigurationCommandExecutor::~MediaCenterGetWhiteBalanceConfigurationCommandExecutor(void)
{
}

GMI_RESULT MediaCenterGetWhiteBalanceConfigurationCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterGetWhiteBalanceConfigurationCommandExecutor> GetWhiteBalanceConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetWhiteBalanceConfigurationCommandExecutor>() );
    if ( NULL == GetWhiteBalanceConfigurationCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = GetWhiteBalanceConfigurationCommandExecutor;
    return MediaCenterGetXXXConfigurationCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterGetWhiteBalanceConfigurationCommandExecutor::Execute()
{
    m_Parameter = BaseMemoryManager::Instance().News<uint8_t>( m_ClientParameterLength );
    if ( NULL == m_Parameter.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    size_t  ConfurationLength = m_ClientParameterLength;
    memset( m_Parameter.GetPtr(), 0, ConfurationLength );

    FD_HANDLE ImageHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->GetWhiteBalanceConfig( ImageHandle, m_Parameter.GetPtr(), &ConfurationLength );
    if ( FAILED( m_Result ) )
    {
        ConfurationLength = 0;
    }

    m_ReplyParameterLength = (uint16_t) ConfurationLength;

    return MediaCenterGetXXXConfigurationCommandExecutor::Execute();
}
