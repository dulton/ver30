#ifndef __SYS_GET_WORK_STATE_CMD_H__
#define __SYS_GET_WORK_STATE_CMD_H__

#include "sys_base_command_executor.h"


class SysGetWorkStateCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetWorkStateCommandExecutor(void);
    virtual ~SysGetWorkStateCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif




