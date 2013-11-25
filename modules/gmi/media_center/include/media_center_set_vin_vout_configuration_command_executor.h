#pragma once

#include "media_center_command_executor.h"

class MediaCenterSetVinVoutConfigurationCommandExecutor : public MediaCenterCommandExecutor
{
public:
    MediaCenterSetVinVoutConfigurationCommandExecutor(void);
    virtual ~MediaCenterSetVinVoutConfigurationCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();

private:
    uint32_t                                 m_Token;
    SafePtr<uint8_t, DefaultObjectsDeleter > m_VinParameter;
    uint16_t                                 m_VinParameterLength;
    SafePtr<uint8_t, DefaultObjectsDeleter > m_VoutParameter;
    uint16_t                                 m_VoutParameterLength;
};
