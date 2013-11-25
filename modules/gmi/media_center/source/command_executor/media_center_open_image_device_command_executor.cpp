#include "media_center_open_image_device_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterOpenImageDeviceCommandExecutor::MediaCenterOpenImageDeviceCommandExecutor()
    : MediaCenterOpenDevice1CommandExecutor( GMI_OPEN_IMAGE_DEVICE, CT_ShortTask )
{
}

MediaCenterOpenImageDeviceCommandExecutor::~MediaCenterOpenImageDeviceCommandExecutor(void)
{
}

GMI_RESULT MediaCenterOpenImageDeviceCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterOpenImageDeviceCommandExecutor> OpenImageDeviceCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterOpenImageDeviceCommandExecutor>() );
    if ( NULL == OpenImageDeviceCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = OpenImageDeviceCommandExecutor;
    return MediaCenterOpenDevice1CommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterOpenImageDeviceCommandExecutor::Execute()
{
    m_Result = m_MediaCenter->OpenImageDevice( m_SensorId, m_ChannelId, &m_Handle );

    return MediaCenterOpenDevice1CommandExecutor::Execute();
}
