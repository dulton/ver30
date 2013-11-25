#pragma once

#include "media_center_command_executor.h"

class MediaCenterForceGenerateIdrFrameCommandExecutor : public MediaCenterCommandExecutor
{
public:
    MediaCenterForceGenerateIdrFrameCommandExecutor(void);
    virtual ~MediaCenterForceGenerateIdrFrameCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();

private:
    uint32_t  m_Token;
};
