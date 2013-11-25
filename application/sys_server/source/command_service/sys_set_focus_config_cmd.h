#ifndef __SYS_SET_FOCUS_CONFIG_CMD_H__
#define __SYS_SET_FOCUS_CONFIG_CMD_H__

#include "sys_base_command_executor.h"

class SysSetFocusConfigCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysSetFocusConfigCommandExecutor();
    virtual ~SysSetFocusConfigCommandExecutor();
    virtual GMI_RESULT	Create(ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor);
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};
#endif


