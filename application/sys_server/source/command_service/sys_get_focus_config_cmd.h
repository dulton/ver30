
#ifndef __SYS_GET_FOCUS_CONFIG_CMD_H__
#define __SYS_GET_FOCUS_CONFIG_CMD_H__

#include "sys_base_command_executor.h"


class SysGetFocusConfigCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetFocusConfigCommandExecutor(void);
    virtual ~SysGetFocusConfigCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif

