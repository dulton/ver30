#pragma once

#include "media_center_set_xxx_configuration_command_executor.h"

class MediaCenterSetDaynightConfigurationCommandExecutor : public MediaCenterSetXXXConfigurationCommandExecutor
{
public:
    MediaCenterSetDaynightConfigurationCommandExecutor(void);
    virtual ~MediaCenterSetDaynightConfigurationCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
