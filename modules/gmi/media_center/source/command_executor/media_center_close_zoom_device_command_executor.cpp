#include "media_center_close_zoom_device_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterCloseZoomDeviceCommandExecutor::MediaCenterCloseZoomDeviceCommandExecutor()
    : MediaCenterCloseDeviceCommandExecutor( GMI_CLOSE_ZOOM_DEVICE, CT_ShortTask )
{
}

MediaCenterCloseZoomDeviceCommandExecutor::~MediaCenterCloseZoomDeviceCommandExecutor(void)
{
}

GMI_RESULT MediaCenterCloseZoomDeviceCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterCloseZoomDeviceCommandExecutor> CloseZoomDeviceCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterCloseZoomDeviceCommandExecutor>() );
    if ( NULL == CloseZoomDeviceCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = CloseZoomDeviceCommandExecutor;
    return MediaCenterCloseDeviceCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterCloseZoomDeviceCommandExecutor::Execute()
{
    FD_HANDLE ZoomHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->CloseZoomDevice( ZoomHandle );

    return MediaCenterCloseDeviceCommandExecutor::Execute();
}
