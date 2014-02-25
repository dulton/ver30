#ifndef __SYS_GET_ALARM_CONFIG_CMD_H_
#define __SYS_GET_ALARM_CONFIG_CMD_H_

#include "sys_base_command_executor.h"


class SysGetAlarmConfigCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetAlarmConfigCommandExecutor(void);
    virtual ~SysGetAlarmConfigCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();

};


#endif
