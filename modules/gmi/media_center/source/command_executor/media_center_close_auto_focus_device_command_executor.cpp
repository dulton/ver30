#include "media_center_close_auto_focus_device_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterCloseAutoFocusDeviceCommandExecutor::MediaCenterCloseAutoFocusDeviceCommandExecutor()
    : MediaCenterCloseDeviceCommandExecutor( GMI_CLOSE_AUTO_FOCUS_DEVICE, CT_ShortTask )
{
}

MediaCenterCloseAutoFocusDeviceCommandExecutor::~MediaCenterCloseAutoFocusDeviceCommandExecutor(void)
{
}

GMI_RESULT MediaCenterCloseAutoFocusDeviceCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterCloseAutoFocusDeviceCommandExecutor> CloseAutoFocusDeviceCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterCloseAutoFocusDeviceCommandExecutor>() );
    if ( NULL == CloseAutoFocusDeviceCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = CloseAutoFocusDeviceCommandExecutor;
    return MediaCenterCloseDeviceCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterCloseAutoFocusDeviceCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->CloseAutoFocusDevice( AutoFocusHandle );

    return MediaCenterCloseDeviceCommandExecutor::Execute();
}
