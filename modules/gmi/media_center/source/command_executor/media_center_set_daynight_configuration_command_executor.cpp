#include "media_center_set_daynight_configuration_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterSetDaynightConfigurationCommandExecutor::MediaCenterSetDaynightConfigurationCommandExecutor(void)
    : MediaCenterSetXXXConfigurationCommandExecutor( GMI_SET_DAYNIGHT_CONFIGURARTION, CT_ShortTask )
{
}

MediaCenterSetDaynightConfigurationCommandExecutor::~MediaCenterSetDaynightConfigurationCommandExecutor(void)
{
}

GMI_RESULT MediaCenterSetDaynightConfigurationCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterSetDaynightConfigurationCommandExecutor> SetDaynightConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetDaynightConfigurationCommandExecutor>() );
    if ( NULL == SetDaynightConfigurationCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = SetDaynightConfigurationCommandExecutor;
    return MediaCenterSetXXXConfigurationCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterSetDaynightConfigurationCommandExecutor::Execute()
{
    FD_HANDLE ImageHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->SetDaynightConfig( ImageHandle, m_Parameter.GetPtr(), m_ParameterLength );

    return MediaCenterSetXXXConfigurationCommandExecutor::Execute();
}
