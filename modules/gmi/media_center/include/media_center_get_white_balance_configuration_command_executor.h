#pragma once

#include "media_center_get_xxx_configuration_command_executor.h"

class MediaCenterGetWhiteBalanceConfigurationCommandExecutor : public MediaCenterGetXXXConfigurationCommandExecutor
{
public:
    MediaCenterGetWhiteBalanceConfigurationCommandExecutor(void);
    virtual ~MediaCenterGetWhiteBalanceConfigurationCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
