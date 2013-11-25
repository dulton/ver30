#pragma once

#include "media_center_operation_without_parameter_command_executor.h"

class MediaCenterStartAutoFocusCommandExecutor : public MediaCenterOperationWithoutParameterCommandExecutor
{
public:
    MediaCenterStartAutoFocusCommandExecutor();
    virtual ~MediaCenterStartAutoFocusCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
