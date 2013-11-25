#ifndef __SYS_EXCUTE_IMPORT_CMD_H__
#define __SYS_EXCUTE_IMPORT_CMD_H__

#include "sys_base_command_executor.h"

class SysExcuteImportConfigFileCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysExcuteImportConfigFileCommandExecutor();
    virtual ~SysExcuteImportConfigFileCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor);
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};


#endif




