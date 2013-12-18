
#ifndef __SYS_GET_LOG_INFO_H__
#define __SYS_GET_LOG_INFO_H__

#include "sys_base_command_executor.h"


class SysGetLogInfoCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetLogInfoCommandExecutor(void);
    virtual ~SysGetLogInfoCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif


