#ifndef __SYS_SET_ALARM_DEPLOY_CMD_H__
#define __SYS_SET_ALARM_DEPLOY_CMD_H__
#include "sys_base_command_executor.h"


class SysSetAlarmDeployCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysSetAlarmDeployCommandExecutor();
    virtual ~SysSetAlarmDeployCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};
#endif



