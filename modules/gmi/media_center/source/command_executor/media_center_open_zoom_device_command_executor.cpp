#include "media_center_open_zoom_device_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterOpenZoomDeviceCommandExecutor::MediaCenterOpenZoomDeviceCommandExecutor()
    : MediaCenterOpenDevice2CommandExecutor( GMI_OPEN_ZOOM_DEVICE, CT_ShortTask )
{
}

MediaCenterOpenZoomDeviceCommandExecutor::~MediaCenterOpenZoomDeviceCommandExecutor(void)
{
}

GMI_RESULT MediaCenterOpenZoomDeviceCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterOpenZoomDeviceCommandExecutor> OpenZoomDeviceCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterOpenZoomDeviceCommandExecutor>() );
    if ( NULL == OpenZoomDeviceCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = OpenZoomDeviceCommandExecutor;
    return MediaCenterOpenDevice2CommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterOpenZoomDeviceCommandExecutor::Execute()
{
    m_Result = m_MediaCenter->OpenZoomDevice( m_SensorId, &m_Handle );

    return MediaCenterOpenDevice2CommandExecutor::Execute();
}
