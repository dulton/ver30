#pragma once

#include "media_center_close_device_command_executor.h"

class MediaCenterCloseAutoFocusDeviceCommandExecutor : public MediaCenterCloseDeviceCommandExecutor
{
public:
    MediaCenterCloseAutoFocusDeviceCommandExecutor();
    virtual ~MediaCenterCloseAutoFocusDeviceCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
