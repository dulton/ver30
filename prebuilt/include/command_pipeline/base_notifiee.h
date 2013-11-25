#pragma once

#include "base_packet.h"
#include "gmi_system_headers.h"

class BaseNotifiee
{
protected:
    BaseNotifiee( uint32_t NotifyId );
public:
    virtual ~BaseNotifiee(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseNotifiee>& Notifiee ) = 0;
    virtual GMI_RESULT  Process() = 0;

    inline  uint32_t    GetNotifyId() const
    {
        return m_NotifyId;
    }

private:
    uint32_t  m_NotifyId;
};
