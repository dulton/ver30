#include "media_center_get_advanced_image_configuration_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterGetAdvancedImageConfigurationCommandExecutor::MediaCenterGetAdvancedImageConfigurationCommandExecutor(void)
    : MediaCenterGetXXXConfigurationCommandExecutor( GMI_GET_ADVANCED_IMAGE_CONFIGURARTION, CT_ShortTask )
{
}

MediaCenterGetAdvancedImageConfigurationCommandExecutor::~MediaCenterGetAdvancedImageConfigurationCommandExecutor(void)
{
}

GMI_RESULT MediaCenterGetAdvancedImageConfigurationCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterGetAdvancedImageConfigurationCommandExecutor> GetAdvancedImageConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetAdvancedImageConfigurationCommandExecutor>() );
    if ( NULL == GetAdvancedImageConfigurationCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = GetAdvancedImageConfigurationCommandExecutor;
    return MediaCenterGetXXXConfigurationCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterGetAdvancedImageConfigurationCommandExecutor::Execute()
{
    m_Parameter = BaseMemoryManager::Instance().News<uint8_t>( m_ClientParameterLength );
    if ( NULL == m_Parameter.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    size_t  ConfurationLength = m_ClientParameterLength;
    memset( m_Parameter.GetPtr(), 0, ConfurationLength );

    FD_HANDLE ImageHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->GetAdvancedImageConfig( ImageHandle, m_Parameter.GetPtr(), &ConfurationLength );
    if ( FAILED( m_Result ) )
    {
        ConfurationLength = 0;
    }

    m_ReplyParameterLength = (uint16_t) ConfurationLength;

    return MediaCenterGetXXXConfigurationCommandExecutor::Execute();
}
