#ifndef __SYS_SET_PRESET_INFO_CMD_H__
#define __SYS_SET_PRESET_INFO_CMD_H__

#include "sys_base_command_executor.h"

class SysSetPresetInfoCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysSetPresetInfoCommandExecutor();
    virtual ~SysSetPresetInfoCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};


#endif



