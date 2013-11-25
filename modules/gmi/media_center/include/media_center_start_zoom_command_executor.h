#pragma once

#include "media_center_command_executor.h"

class MediaCenterStartZoomCommandExecutor : public MediaCenterCommandExecutor
{
public:
    MediaCenterStartZoomCommandExecutor();
    virtual ~MediaCenterStartZoomCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();

private:
    uint32_t  m_AutoFocusHandle;
    uint32_t  m_AutoFocusControlStatus;
    uint32_t  m_ZoomHandle;
    uint32_t  m_ZoomMode;
};
