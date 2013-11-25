#include "media_center_open_vin_vout_device_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterOpenVinVoutDeviceCommandExecutor::MediaCenterOpenVinVoutDeviceCommandExecutor()
    : MediaCenterOpenDevice1CommandExecutor( GMI_OPEN_VIN_VOUT_DEVICE, CT_ShortTask )
{
}

MediaCenterOpenVinVoutDeviceCommandExecutor::~MediaCenterOpenVinVoutDeviceCommandExecutor(void)
{
}

GMI_RESULT MediaCenterOpenVinVoutDeviceCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterOpenVinVoutDeviceCommandExecutor> OpenVinVoutDeviceCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterOpenVinVoutDeviceCommandExecutor>() );
    if ( NULL == OpenVinVoutDeviceCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = OpenVinVoutDeviceCommandExecutor;
    return MediaCenterOpenDevice1CommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterOpenVinVoutDeviceCommandExecutor::Execute()
{
    m_Result = m_MediaCenter->OpenVinVoutDevice( m_SensorId, m_ChannelId, &m_Handle );

    return MediaCenterOpenDevice1CommandExecutor::Execute();
}
