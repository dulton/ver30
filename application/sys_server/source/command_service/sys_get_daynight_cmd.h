#ifndef __SYS_GET_DAY_NIGHT_CMD_H__
#define __SYS_GET_DAY_NIGHT_CMD_H__

#include "sys_base_command_executor.h"


class SysGetDaynightCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetDaynightCommandExecutor();
    virtual ~SysGetDaynightCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif



