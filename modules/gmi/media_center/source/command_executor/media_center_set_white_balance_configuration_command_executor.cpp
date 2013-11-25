#include "media_center_set_white_balance_configuration_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterSetWhiteBalanceConfigurationCommandExecutor::MediaCenterSetWhiteBalanceConfigurationCommandExecutor(void)
    : MediaCenterSetXXXConfigurationCommandExecutor( GMI_SET_WHITE_BALANCE_CONFIGURARTION, CT_ShortTask )
{
}

MediaCenterSetWhiteBalanceConfigurationCommandExecutor::~MediaCenterSetWhiteBalanceConfigurationCommandExecutor(void)
{
}

GMI_RESULT MediaCenterSetWhiteBalanceConfigurationCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterSetWhiteBalanceConfigurationCommandExecutor> SetWhiteBalanceConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetWhiteBalanceConfigurationCommandExecutor>() );
    if ( NULL == SetWhiteBalanceConfigurationCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = SetWhiteBalanceConfigurationCommandExecutor;
    return MediaCenterSetXXXConfigurationCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterSetWhiteBalanceConfigurationCommandExecutor::Execute()
{
    FD_HANDLE ImageHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->SetWhiteBalanceConfig( ImageHandle, m_Parameter.GetPtr(), m_ParameterLength );

    return MediaCenterSetXXXConfigurationCommandExecutor::Execute();
}
