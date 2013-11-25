#ifndef __SYS_SET_ENCSTREAM_COMBINE_CMD_H__
#define __SYS_SET_ENCSTREAM_COMBINE_CMD_H__

#include "sys_base_command_executor.h"

class SysSetEncStreamCombineCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysSetEncStreamCombineCommandExecutor();
    virtual ~SysSetEncStreamCombineCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};


#endif




