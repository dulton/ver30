#pragma once

#include "media_center_command_executor.h"

class MediaCenterSetXXXConfigurationCommandExecutor : public MediaCenterCommandExecutor
{
protected:
    MediaCenterSetXXXConfigurationCommandExecutor( uint32_t CommandId, enum CommandType Type );

public:
    virtual ~MediaCenterSetXXXConfigurationCommandExecutor(void);

    virtual GMI_RESULT  Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();

protected:
    uint32_t                                 m_Token;
    SafePtr<uint8_t, DefaultObjectsDeleter > m_Parameter;
    uint16_t                                 m_ParameterLength;
};
