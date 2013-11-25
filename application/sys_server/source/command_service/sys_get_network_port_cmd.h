#ifndef __SYS_GET_NETWORK_PORT_CMD_H__
#define __SYS_GET_NETWORK_PORT_CMD_H__

#include "sys_base_command_executor.h"


class SysGetNetworkPortCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetNetworkPortCommandExecutor(void);
    virtual ~SysGetNetworkPortCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif



