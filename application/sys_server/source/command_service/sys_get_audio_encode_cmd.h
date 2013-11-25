#ifndef __SYS_GET_AUDIO_ENCODE_CMD_H__
#define __SYS_GET_AUDIO_ENCODE_CMD_H__

#include "sys_base_command_executor.h"


class SysGetAudioEncodeCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetAudioEncodeCommandExecutor(void);
    virtual ~SysGetAudioEncodeCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif



