#include "media_center_set_advanced_image_configuration_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterSetAdvancedImageConfigurationCommandExecutor::MediaCenterSetAdvancedImageConfigurationCommandExecutor(void)
    : MediaCenterSetXXXConfigurationCommandExecutor( GMI_SET_ADVANCED_IMAGE_CONFIGURARTION, CT_ShortTask )
{
}

MediaCenterSetAdvancedImageConfigurationCommandExecutor::~MediaCenterSetAdvancedImageConfigurationCommandExecutor(void)
{
}

GMI_RESULT MediaCenterSetAdvancedImageConfigurationCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterSetAdvancedImageConfigurationCommandExecutor> SetAdvancedImageConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetAdvancedImageConfigurationCommandExecutor>() );
    if ( NULL == SetAdvancedImageConfigurationCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = SetAdvancedImageConfigurationCommandExecutor;
    return MediaCenterSetXXXConfigurationCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterSetAdvancedImageConfigurationCommandExecutor::Execute()
{
    FD_HANDLE ImageHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->SetAdvancedImageConfig( ImageHandle, m_Parameter.GetPtr(), m_ParameterLength );

    return MediaCenterSetXXXConfigurationCommandExecutor::Execute();
}
