#include "media_center_set_base_image_configuration_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterSetBaseImageConfigurationCommandExecutor::MediaCenterSetBaseImageConfigurationCommandExecutor(void)
    : MediaCenterSetXXXConfigurationCommandExecutor( GMI_SET_BASE_IMAGE_CONFIGURARTION, CT_ShortTask )
{
}

MediaCenterSetBaseImageConfigurationCommandExecutor::~MediaCenterSetBaseImageConfigurationCommandExecutor(void)
{
}

GMI_RESULT MediaCenterSetBaseImageConfigurationCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterSetBaseImageConfigurationCommandExecutor> SetBaseImageConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetBaseImageConfigurationCommandExecutor>() );
    if ( NULL == SetBaseImageConfigurationCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = SetBaseImageConfigurationCommandExecutor;
    return MediaCenterSetXXXConfigurationCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterSetBaseImageConfigurationCommandExecutor::Execute()
{
    FD_HANDLE ImageHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->SetBaseImageConfig( ImageHandle, m_Parameter.GetPtr(), m_ParameterLength );

    return MediaCenterSetXXXConfigurationCommandExecutor::Execute();
}
