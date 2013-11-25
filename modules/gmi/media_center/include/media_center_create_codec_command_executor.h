#pragma once

#include "media_center_command_executor.h"

class MediaCenterCreateCodecCommandExecutor : public MediaCenterCommandExecutor
{
public:
    MediaCenterCreateCodecCommandExecutor(void);
    virtual ~MediaCenterCreateCodecCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();

private:
    uint8_t                                  m_CodecMode;
    uint32_t                                 m_SourceId;
    uint32_t                                 m_MediaId;
    uint32_t                                 m_MediaType;
    uint32_t                                 m_CodecType;
    SafePtr<uint8_t, DefaultObjectsDeleter > m_CodecParameter;
    uint16_t                                 m_CodecParameterLength;
};
