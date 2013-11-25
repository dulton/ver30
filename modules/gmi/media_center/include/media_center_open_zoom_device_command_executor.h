#pragma once

#include "media_center_open_device2_command_executor.h"

class MediaCenterOpenZoomDeviceCommandExecutor : public MediaCenterOpenDevice2CommandExecutor
{
public:
    MediaCenterOpenZoomDeviceCommandExecutor();
    virtual ~MediaCenterOpenZoomDeviceCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
