#pragma once

#include "media_center_get_xxx_configuration_command_executor.h"

class MediaCenterGetAdvancedImageConfigurationCommandExecutor : public MediaCenterGetXXXConfigurationCommandExecutor
{
public:
    MediaCenterGetAdvancedImageConfigurationCommandExecutor(void);
    virtual ~MediaCenterGetAdvancedImageConfigurationCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
