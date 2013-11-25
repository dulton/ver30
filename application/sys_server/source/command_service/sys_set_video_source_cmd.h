#ifndef __SYS_SET_VIDEO_SOURCE_CMD_H__
#define __SYS_SET_VIDEO_SOURCE_CMD_H__

#include "sys_base_command_executor.h"

class SysSetVideoSourceCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysSetVideoSourceCommandExecutor();
    virtual ~SysSetVideoSourceCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};


#endif


