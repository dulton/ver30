#ifndef __SYS_GET_ALARM_DEPLOY_CMD_H__
#define __SYS_GET_ALARM_DEPLOY_CMD_H__
#include "sys_base_command_executor.h"


class SysGetAlarmDeployCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetAlarmDeployCommandExecutor();
    virtual ~SysGetAlarmDeployCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif