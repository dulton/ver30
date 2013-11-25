#pragma once

#include "media_center_command_executor.h"

class MediaCenterSetOsdConfigurationCommandExecutor : public MediaCenterCommandExecutor
{
public:
    MediaCenterSetOsdConfigurationCommandExecutor(void);
    virtual ~MediaCenterSetOsdConfigurationCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();

private:
    uint32_t                                 m_Token;
    SafePtr<uint8_t, DefaultObjectsDeleter > m_Parameter;
    uint16_t                                 m_ParameterLength;
};
