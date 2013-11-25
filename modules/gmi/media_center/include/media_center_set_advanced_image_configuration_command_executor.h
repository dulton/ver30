#pragma once

#include "media_center_set_xxx_configuration_command_executor.h"

class MediaCenterSetAdvancedImageConfigurationCommandExecutor : public MediaCenterSetXXXConfigurationCommandExecutor
{
public:
    MediaCenterSetAdvancedImageConfigurationCommandExecutor(void);
    virtual ~MediaCenterSetAdvancedImageConfigurationCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
