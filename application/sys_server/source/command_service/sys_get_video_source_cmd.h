#ifndef __SYS_VIDEO_SOURCE_CMD_H__
#define __SYS_VIDEO_SOURCE_CMD_H__

#include "sys_base_command_executor.h"


class SysGetVideoSourceCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetVideoSourceCommandExecutor();
    virtual ~SysGetVideoSourceCommandExecutor();
    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif



