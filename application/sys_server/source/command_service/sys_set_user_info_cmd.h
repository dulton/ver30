#ifndef __SYS_SET_USER_INFO_CMD_H_
#define __SYS_SET_USER_INFO_CMD_H_

#include "sys_base_command_executor.h"


class SysSetUserInfoCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysSetUserInfoCommandExecutor(void);
    virtual ~SysSetUserInfoCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();

};


#endif





