#pragma once

#include "media_center_get_operation_with_two_parameter_command_executor.h"

class MediaCenterGetZoomRangeCommandExecutor : public MediaCenterGetOperationWithTwoParameterCommandExecutor
{
public:
    MediaCenterGetZoomRangeCommandExecutor();
    virtual ~MediaCenterGetZoomRangeCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
};
