#ifndef __SYS_GET_ADVANCED_IMAGING_CMD_H__
#define __SYS_GET_ADVANCED_IMAGING_CMD_H__

#include "sys_base_command_executor.h"


class SysGetAdvancedImagingCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetAdvancedImagingCommandExecutor();
    virtual ~SysGetAdvancedImagingCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif

