#pragma once

#include "media_center_command_executor.h"

class MediaCenterOperationWithoutParameterCommandExecutor : public MediaCenterCommandExecutor
{
protected:
    MediaCenterOperationWithoutParameterCommandExecutor( uint32_t CommandId, enum CommandType Type );

public:
    virtual ~MediaCenterOperationWithoutParameterCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();

protected:
    uint32_t  m_Token;
};
