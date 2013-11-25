#pragma once

#include "media_center_set_xxx_configuration_command_executor.h"

class MediaCenterSetAutoFocusConfigurationCommandExecutor : public MediaCenterSetXXXConfigurationCommandExecutor
{
public:
    MediaCenterSetAutoFocusConfigurationCommandExecutor(void);
    virtual ~MediaCenterSetAutoFocusConfigurationCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
