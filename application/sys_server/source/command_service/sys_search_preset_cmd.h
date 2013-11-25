
#ifndef __SYS_SEARCH_PRESET_CMD_H__
#define __SYS_SEARCH_PRESET_CMD_H__

#include "sys_base_command_executor.h"


class SysSearchPresetCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysSearchPresetCommandExecutor(void);
    virtual ~SysSearchPresetCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};


#endif



