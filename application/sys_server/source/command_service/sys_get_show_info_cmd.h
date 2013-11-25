
#ifndef __SYS_GET_SHOW_INFO_CMD_H__
#define __SYS_GET_SHOW_INFO_CMD_H__

#include "sys_base_command_executor.h"


class SysGetShowInfoCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetShowInfoCommandExecutor(void);
    virtual ~SysGetShowInfoCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();

};

#endif

