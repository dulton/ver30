#ifndef __SYS_START_AUDIO_DECODE_CMD_H__
#define __SYS_START_AUDIO_DECODE_CMD_H__

#include "sys_base_command_executor.h"


class SysStartAudioDecodeCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysStartAudioDecodeCommandExecutor(void);
    virtual ~SysStartAudioDecodeCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif



