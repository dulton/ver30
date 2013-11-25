#ifndef __SYS_STOP_AUDIO_DECODE_CMD_H__
#define __SYS_STOP_AUDIO_DECODE_CMD_H__

#include "sys_base_command_executor.h"


class SysStopAudioDecodeCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysStopAudioDecodeCommandExecutor(void);
    virtual ~SysStopAudioDecodeCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
};

#endif


