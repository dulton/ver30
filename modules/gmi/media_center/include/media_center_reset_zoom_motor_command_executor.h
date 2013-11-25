#pragma once

#include "media_center_operation_without_parameter_command_executor.h"

class MediaCenterResetZoomMotorCommandExecutor : public MediaCenterOperationWithoutParameterCommandExecutor
{
public:
    MediaCenterResetZoomMotorCommandExecutor();
    virtual ~MediaCenterResetZoomMotorCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
