#pragma once

#include "media_center_command_executor.h"

class MediaCenterGetVinVoutConfigurationCommandExecutor : public MediaCenterCommandExecutor
{
public:
    MediaCenterGetVinVoutConfigurationCommandExecutor(void);
    virtual ~MediaCenterGetVinVoutConfigurationCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();

private:
    uint32_t m_Token;
    uint16_t m_VinParameterLength;
    uint16_t m_VoutParameterLength;
};
