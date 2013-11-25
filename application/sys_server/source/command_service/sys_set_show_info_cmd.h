#ifndef __SYS_SET_SHOW_INFO_CMD_H__
#define __SYS_SET_SHOW_INFO_CMD_H__

#include "sys_base_command_executor.h"


class SysSetShowInfoCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysSetShowInfoCommandExecutor();
    virtual ~SysSetShowInfoCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};


#endif

