#pragma once

#include "media_center_get_xxx_configuration_command_executor.h"

class MediaCenterGetBaseImageConfigurationCommandExecutor : public MediaCenterGetXXXConfigurationCommandExecutor
{
public:
    MediaCenterGetBaseImageConfigurationCommandExecutor(void);
    virtual ~MediaCenterGetBaseImageConfigurationCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
