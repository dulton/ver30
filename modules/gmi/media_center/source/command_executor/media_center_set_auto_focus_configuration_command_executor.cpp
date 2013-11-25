#include "media_center_set_auto_focus_configuration_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterSetAutoFocusConfigurationCommandExecutor::MediaCenterSetAutoFocusConfigurationCommandExecutor(void)
    : MediaCenterSetXXXConfigurationCommandExecutor( GMI_SET_AUTO_FOCUS_CONFIGURARTION, CT_ShortTask )
{
}

MediaCenterSetAutoFocusConfigurationCommandExecutor::~MediaCenterSetAutoFocusConfigurationCommandExecutor(void)
{
}

GMI_RESULT MediaCenterSetAutoFocusConfigurationCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterSetAutoFocusConfigurationCommandExecutor> SetAutoFocusConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetAutoFocusConfigurationCommandExecutor>() );
    if ( NULL == SetAutoFocusConfigurationCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = SetAutoFocusConfigurationCommandExecutor;
    return MediaCenterSetXXXConfigurationCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterSetAutoFocusConfigurationCommandExecutor::Execute()
{
    FD_HANDLE ImageHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->SetAutoFocusConfig( ImageHandle, m_Parameter.GetPtr(), m_ParameterLength );

    return MediaCenterSetXXXConfigurationCommandExecutor::Execute();
}
