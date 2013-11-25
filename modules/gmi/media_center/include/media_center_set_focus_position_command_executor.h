#pragma once

#include "media_center_set_operation_with_one_parameter_command_executor.h"

class MediaCenterSetFocusPositionCommandExecutor : public MediaCenterSetOperationWithOneParameterCommandExecutor
{
public:
    MediaCenterSetFocusPositionCommandExecutor();
    virtual ~MediaCenterSetFocusPositionCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
