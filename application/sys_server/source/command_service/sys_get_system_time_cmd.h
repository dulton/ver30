#ifndef __SYS_GET_SYSTEM_TIME_CMD_H__
#define __SYS_GET_SYSTEM_TIME_CMD_H__

#include "sys_base_command_executor.h"


class SysGetTimeCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetTimeCommandExecutor();
    virtual ~SysGetTimeCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif


