#ifndef __SYS_STOP_3A_CMD_H__
#define __SYS_STOP_3A_CMD_H__

#include "sys_base_command_executor.h"


class SysStop3ACommandExecutor : public SysBaseCommandExecutor
{
public:
    SysStop3ACommandExecutor(void);
    virtual ~SysStop3ACommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif





