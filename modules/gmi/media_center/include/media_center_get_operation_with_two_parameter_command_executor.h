#pragma once

#include "media_center_command_executor.h"

class MediaCenterGetOperationWithTwoParameterCommandExecutor : public MediaCenterCommandExecutor
{
protected:
    MediaCenterGetOperationWithTwoParameterCommandExecutor( uint32_t CommandId, enum CommandType Type );

public:
    virtual ~MediaCenterGetOperationWithTwoParameterCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();

protected:
    uint32_t  m_Token;
    uint32_t  m_Parameter1;
    uint32_t  m_Parameter2;
};
