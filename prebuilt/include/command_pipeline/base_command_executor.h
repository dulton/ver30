#pragma once

#include "base_command_header.h"
#include "base_packet.h"
#include "base_session.h"
#include "gmi_system_headers.h"

class BaseCommandExecutor
{
protected:
    BaseCommandExecutor( uint32_t CommandId, enum CommandType Type );
public:
    virtual ~BaseCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor ) = 0;
    virtual GMI_RESULT  Destroy();
    virtual GMI_RESULT  Execute() = 0;
    virtual GMI_RESULT  Reply() = 0;
    inline  uint32_t    GetCommandId() const
    {
        return m_CommandId;
    }
    inline  CommandType GetCommandType() const
    {
        return m_CommandType;
    }

protected:
    ReferrencePtr<BaseSession,DefaultObjectDeleter,MultipleThreadModel>  m_Session;
    ReferrencePtr<BasePacket>                                            m_Reply;

private:
    uint32_t                                                             m_CommandId;
    CommandType	                                                         m_CommandType;
};
