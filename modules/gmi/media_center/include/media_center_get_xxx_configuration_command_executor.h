#pragma once

#include "media_center_command_executor.h"

class MediaCenterGetXXXConfigurationCommandExecutor : public MediaCenterCommandExecutor
{
protected:
    MediaCenterGetXXXConfigurationCommandExecutor( uint32_t CommandId, enum CommandType Type );

public:
    virtual ~MediaCenterGetXXXConfigurationCommandExecutor(void);

    virtual GMI_RESULT  Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();

protected:
    uint32_t                                 m_Token;
    uint16_t                                 m_ClientParameterLength;
    uint16_t                                 m_ReplyParameterLength;
    SafePtr<uint8_t, DefaultObjectsDeleter > m_Parameter;
};
