#include "media_center_close_image_device_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterCloseImageDeviceCommandExecutor::MediaCenterCloseImageDeviceCommandExecutor()
    : MediaCenterCloseDeviceCommandExecutor( GMI_CLOSE_IMAGE_DEVICE, CT_ShortTask )
{
}

MediaCenterCloseImageDeviceCommandExecutor::~MediaCenterCloseImageDeviceCommandExecutor(void)
{
}

GMI_RESULT MediaCenterCloseImageDeviceCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterCloseImageDeviceCommandExecutor> CloseImageDeviceCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterCloseImageDeviceCommandExecutor>() );
    if ( NULL == CloseImageDeviceCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = CloseImageDeviceCommandExecutor;
    return MediaCenterCloseDeviceCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterCloseImageDeviceCommandExecutor::Execute()
{
    FD_HANDLE ImageHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->CloseImageDevice( ImageHandle );

    return MediaCenterCloseDeviceCommandExecutor::Execute();
}
