#pragma once

#include "base_command_header.h"
#include "base_packet.h"
#include "base_session.h"
#include "gmi_system_headers.h"

class BaseCommandRequester
{
protected:
    BaseCommandRequester( uint32_t CommandId, enum CommandType Type );
public:
    virtual ~BaseCommandRequester(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BaseSession,DefaultObjectDeleter,MultipleThreadModel> Session, long_t Timeout );
    virtual GMI_RESULT  Destroy();
    virtual GMI_RESULT	Package() = 0;
    virtual GMI_RESULT  Submit();
    virtual GMI_RESULT  Wait() = 0;
    virtual GMI_RESULT  Signal( ReferrencePtr<BasePacket,DefaultObjectDeleter,MultipleThreadModel>& Packet ) = 0;
    virtual GMI_RESULT  Lock();
    virtual GMI_RESULT  Unlock();

    inline  uint32_t    GetCommandId() const
    {
        return m_CommandId;
    }
    inline  CommandType IsLongTask() const
    {
        return m_CommandType;
    }
    inline  long_t      GetTimeout() const
    {
        return m_Timeout;
    }

protected:
    ReferrencePtr<BaseSession,DefaultObjectDeleter,MultipleThreadModel>  m_Session;
    ReferrencePtr<BasePacket>                                            m_Packet;
    long_t                                                               m_Timeout;
    GMI_Mutex                                                            m_InstanceMutex;

private:
    uint32_t                                                             m_CommandId;
    CommandType	                                                         m_CommandType;
};
