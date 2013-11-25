#pragma once

#include "base_command_executor.h"
#include "media_center.h"

class MediaCenterCommandExecutor : public BaseCommandExecutor
{
protected:
    MediaCenterCommandExecutor( uint32_t CommandId, enum CommandType Type );

public:
    virtual ~MediaCenterCommandExecutor(void);

    virtual GMI_RESULT  Reply();

    void_t SetParameter( ReferrencePtr<MediaCenter>& Center );

protected:
    ReferrencePtr<MediaCenter> m_MediaCenter;
    GMI_RESULT                 m_Result;
};
