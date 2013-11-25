#pragma once

#include "media_center_command_executor.h"

class MediaCenterOpenDevice1CommandExecutor : public MediaCenterCommandExecutor
{
protected:
    MediaCenterOpenDevice1CommandExecutor( uint32_t CommandId, enum CommandType Type );

public:
    virtual ~MediaCenterOpenDevice1CommandExecutor(void);

    virtual GMI_RESULT  Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();

protected:
    uint16_t   m_SensorId;
    uint16_t   m_ChannelId;
    FD_HANDLE  m_Handle;
};
