#pragma once

#include "media_center_open_device1_command_executor.h"

class MediaCenterOpenVinVoutDeviceCommandExecutor : public MediaCenterOpenDevice1CommandExecutor
{
public:
    MediaCenterOpenVinVoutDeviceCommandExecutor();
    virtual ~MediaCenterOpenVinVoutDeviceCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
