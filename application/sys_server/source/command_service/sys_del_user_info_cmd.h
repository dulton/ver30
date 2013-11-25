#ifndef __SYS_DEL_USER_INFO_CMD_H_
#define __SYS_DEL_USER_INFO_CMD_H_

#include "sys_base_command_executor.h"


class SysDelUserInfoCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysDelUserInfoCommandExecutor(void);
    virtual ~SysDelUserInfoCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();

};

#endif





