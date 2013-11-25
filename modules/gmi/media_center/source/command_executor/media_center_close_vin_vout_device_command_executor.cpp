#include "media_center_close_vin_vout_device_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterCloseVinVoutDeviceCommandExecutor::MediaCenterCloseVinVoutDeviceCommandExecutor()
    : MediaCenterCloseDeviceCommandExecutor( GMI_CLOSE_VIN_VOUT_DEVICE, CT_ShortTask )
{
}

MediaCenterCloseVinVoutDeviceCommandExecutor::~MediaCenterCloseVinVoutDeviceCommandExecutor(void)
{
}

GMI_RESULT MediaCenterCloseVinVoutDeviceCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterCloseVinVoutDeviceCommandExecutor> CloseVinVoutDeviceCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterCloseVinVoutDeviceCommandExecutor>() );
    if ( NULL == CloseVinVoutDeviceCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = CloseVinVoutDeviceCommandExecutor;
    return MediaCenterCloseDeviceCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterCloseVinVoutDeviceCommandExecutor::Execute()
{
    FD_HANDLE VinVoutHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->CloseVinVoutDevice( VinVoutHandle );

    return MediaCenterCloseDeviceCommandExecutor::Execute();
}
