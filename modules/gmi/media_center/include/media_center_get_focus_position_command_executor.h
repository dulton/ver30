#pragma once

#include "media_center_get_operation_with_one_parameter_command_executor.h"

class MediaCenterGetFocusPositionCommandExecutor : public MediaCenterGetOperationWithOneParameterCommandExecutor
{
public:
    MediaCenterGetFocusPositionCommandExecutor();
    virtual ~MediaCenterGetFocusPositionCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
