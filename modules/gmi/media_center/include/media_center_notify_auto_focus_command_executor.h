#pragma once

#include "media_center_command_executor.h"

class MediaCenterNotifyAutoFocusCommandExecutor : public MediaCenterCommandExecutor
{
public:
    MediaCenterNotifyAutoFocusCommandExecutor();
    virtual ~MediaCenterNotifyAutoFocusCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();

private:
    uint32_t                                m_Token;
    int32_t                                 m_EventType;
    uint32_t                                m_ExtDataLength;
    SafePtr<uint8_t, DefaultObjectsDeleter> m_ExtData;
};
