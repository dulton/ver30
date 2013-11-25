#ifndef __SYS_GET_ENCODE_CONFIG_CMD_H__
#define __SYS_GET_ENCODE_CONFIG_CMD_H__

#include "sys_base_command_executor.h"


class SysGetEncodeConfigCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetEncodeConfigCommandExecutor();
    virtual ~SysGetEncodeConfigCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif


