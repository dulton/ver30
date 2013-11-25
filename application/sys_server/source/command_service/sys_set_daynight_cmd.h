#ifndef __SYS_SET_DAY_NIGHT_CMD_H__
#define __SYS_SET_DAY_NIGHT_CMD_H__

#include "sys_base_command_executor.h"


class SysSetDaynightCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysSetDaynightCommandExecutor();
    virtual ~SysSetDaynightCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif




