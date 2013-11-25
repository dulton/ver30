#ifndef __SYS_SYSTEM_CTRL_CMD_H__
#define __SYS_SYSTEM_CTRL_CMD_H__

#include "base_packet.h"
#include "sys_base_command_executor.h"


class SysSystemCtrlCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysSystemCtrlCommandExecutor(void);
    virtual ~SysSystemCtrlCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();

private:
    boolean_t m_Reboot;
};


#endif

