#ifndef __SYS_PTZ_CTRL_CMD_H__
#define __SYS_PTZ_CTRL_CMD_H__

#include "sys_base_command_executor.h"

class SysPtzCtrlCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysPtzCtrlCommandExecutor();
    virtual ~SysPtzCtrlCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};


#endif

