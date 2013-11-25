#include "media_center_open_auto_focus_device_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterOpenAutoFocusDeviceCommandExecutor::MediaCenterOpenAutoFocusDeviceCommandExecutor()
    : MediaCenterOpenDevice2CommandExecutor( GMI_OPEN_AUTO_FOCUS_DEVICE, CT_ShortTask )
{
}

MediaCenterOpenAutoFocusDeviceCommandExecutor::~MediaCenterOpenAutoFocusDeviceCommandExecutor(void)
{
}

GMI_RESULT MediaCenterOpenAutoFocusDeviceCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterOpenAutoFocusDeviceCommandExecutor> OpenAutoFocusDeviceCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterOpenAutoFocusDeviceCommandExecutor>() );
    if ( NULL == OpenAutoFocusDeviceCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = OpenAutoFocusDeviceCommandExecutor;
    return MediaCenterOpenDevice2CommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterOpenAutoFocusDeviceCommandExecutor::Execute()
{
    m_Result = m_MediaCenter->OpenAutoFocusDevice( m_SensorId, &m_Handle );

    return MediaCenterOpenDevice2CommandExecutor::Execute();
}
