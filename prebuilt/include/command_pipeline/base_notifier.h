#pragma once

#include "base_packet.h"
#include "base_session.h"
#include "gmi_system_headers.h"

class BaseNotifier
{
protected:
    BaseNotifier( uint32_t NotifyId );
public:
    virtual ~BaseNotifier(void);

    virtual GMI_RESULT	Package() = 0;
    virtual GMI_RESULT  Dispatch();

    inline  uint32_t    GetNotifyId() const
    {
        return m_NotifyId;
    }

    GMI_RESULT AddSession( ReferrencePtr<BaseSession,DefaultObjectDeleter,MultipleThreadModel> Session );
    GMI_RESULT RemoveSession( ReferrencePtr<BaseSession,DefaultObjectDeleter,MultipleThreadModel> Session );

protected:
    SafePtr<BasePacket>                                                               m_NotifierPacket;
    std::list< ReferrencePtr<BaseSession,DefaultObjectDeleter,MultipleThreadModel> >  m_Sessions;

private:
    uint32_t  m_NotifyId;
};
