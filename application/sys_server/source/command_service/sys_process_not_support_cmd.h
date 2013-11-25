#ifndef __SYS_PROCESS_NOT_SUPPORT_CMD_H_
#define __SYS_PROCESS_NOT_SUPPORT_CMD_H_

#include "sys_base_command_executor.h"


class SysProcessNotSupportCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysProcessNotSupportCommandExecutor(void);
    virtual ~SysProcessNotSupportCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();

};


#endif




