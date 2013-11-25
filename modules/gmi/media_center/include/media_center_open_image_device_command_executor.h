#pragma once

#include "media_center_open_device1_command_executor.h"

class MediaCenterOpenImageDeviceCommandExecutor : public MediaCenterOpenDevice1CommandExecutor
{
public:
    MediaCenterOpenImageDeviceCommandExecutor();
    virtual ~MediaCenterOpenImageDeviceCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
