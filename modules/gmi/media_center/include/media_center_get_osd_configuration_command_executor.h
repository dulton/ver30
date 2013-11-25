#pragma once

#include "media_center_command_executor.h"

class MediaCenterGetOsdConfigurationCommandExecutor : public MediaCenterCommandExecutor
{
public:
    MediaCenterGetOsdConfigurationCommandExecutor(void);
    virtual ~MediaCenterGetOsdConfigurationCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();

private:
    uint32_t m_Token;
    uint16_t m_ParameterLength;
};
