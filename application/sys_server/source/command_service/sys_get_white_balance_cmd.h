#ifndef __SYS_GET_WHITE_BALANCE_CMD_H__
#define __SYS_GET_WHITE_BALANCE_CMD_H__

#include "sys_base_command_executor.h"


class SysGetWhiteBalanceCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetWhiteBalanceCommandExecutor();
    virtual ~SysGetWhiteBalanceCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif




