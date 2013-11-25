#pragma once

#include "media_center_close_device_command_executor.h"

class MediaCenterDestroyCodecCommandExecutor : public MediaCenterCloseDeviceCommandExecutor
{
public:
    MediaCenterDestroyCodecCommandExecutor(void);
    virtual ~MediaCenterDestroyCodecCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
