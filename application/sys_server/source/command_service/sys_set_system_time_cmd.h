#ifndef __SYS_SET_SYSTEM_TIME_CMD_H__
#define __SYS_SET_SYSTEM_TIME_CMD_H__

#include "sys_base_command_executor.h"


class SysSetTimeCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysSetTimeCommandExecutor();
    virtual ~SysSetTimeCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif





