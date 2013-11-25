#pragma once

#include "media_center_command_executor.h"

class MediaCenterOpenDevice2CommandExecutor : public MediaCenterCommandExecutor
{
protected:
    MediaCenterOpenDevice2CommandExecutor( uint32_t CommandId, enum CommandType Type );

public:
    virtual ~MediaCenterOpenDevice2CommandExecutor(void);

    virtual GMI_RESULT  Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();

protected:
    uint16_t   m_SensorId;
    FD_HANDLE  m_Handle;
};
