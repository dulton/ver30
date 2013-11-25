#pragma once

#include "cross_platform_headers.h"

class BaseCommunication
{
protected:
    BaseCommunication(void);
public:
    virtual ~BaseCommunication(void);

    virtual GMI_RESULT Close()
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Send( const uint8_t *Buffer, size_t BufferSize, long_t Flags, size_t *Transferred )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Receive( uint8_t *Buffer, size_t BufferSize, long_t Flags, size_t *Transferred )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT SendTo( const uint8_t *Buffer, size_t BufferSize, long_t Flags, const struct sockaddr* To, long_t ToLength, size_t *Transferred )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT ReceiveFrom( uint8_t *Buffer, size_t BufferSize, long_t Flags, struct sockaddr *From, long_t *FromLength, size_t *Transferred )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual FD_HANDLE  GetFDHandle()
    {
        return NULL;
    }
};
