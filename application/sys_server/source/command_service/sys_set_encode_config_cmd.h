#ifndef __SYS_SET_ENCODE_CONFIG_CMD_H__
#define __SYS_SET_ENCODE_CONFIG_CMD_H__

#include "sys_base_command_executor.h"


class SysSetEncodeConfigCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysSetEncodeConfigCommandExecutor();
    virtual ~SysSetEncodeConfigCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif


