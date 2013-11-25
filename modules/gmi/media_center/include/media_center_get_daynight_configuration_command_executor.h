#pragma once

#include "media_center_get_xxx_configuration_command_executor.h"

class MediaCenterGetDaynightConfigurationCommandExecutor : public MediaCenterGetXXXConfigurationCommandExecutor
{
public:
    MediaCenterGetDaynightConfigurationCommandExecutor(void);
    virtual ~MediaCenterGetDaynightConfigurationCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
