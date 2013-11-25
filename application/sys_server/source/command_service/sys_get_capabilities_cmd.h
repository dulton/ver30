


#ifndef __SYS_GET_CAPABILITIES_CMD_H__
#define __SYS_GET_CAPABILITIES_CMD_H__

#include "sys_base_command_executor.h"


class SysGetCapabilitiesCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetCapabilitiesCommandExecutor(void);
    virtual ~SysGetCapabilitiesCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif

