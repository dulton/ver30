#pragma once

#include "media_center_command_executor.h"

class MediaCenterCloseDeviceCommandExecutor : public MediaCenterCommandExecutor
{
protected:
    MediaCenterCloseDeviceCommandExecutor( uint32_t CommandId, enum CommandType Type );

public:
    virtual ~MediaCenterCloseDeviceCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();

protected:
    uint32_t  m_Token;
};
