#ifndef __SYS_PTZ_3DCTRL_CMD_H__
#define __SYS_PTZ_3DCTRL_CMD_H__

#include "sys_base_command_executor.h"


class SysPtz3DCtrlCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysPtz3DCtrlCommandExecutor(void);
    virtual ~SysPtz3DCtrlCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif

