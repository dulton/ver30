#pragma once

#include "media_center_set_xxx_configuration_command_executor.h"

class MediaCenterSetBaseImageConfigurationCommandExecutor : public MediaCenterSetXXXConfigurationCommandExecutor
{
public:
    MediaCenterSetBaseImageConfigurationCommandExecutor(void);
    virtual ~MediaCenterSetBaseImageConfigurationCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
