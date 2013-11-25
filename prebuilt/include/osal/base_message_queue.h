#pragma once

#include "cross_platform_headers.h"

// to windows,no limit for message length;
// to linux, MSGMAX(Ubuntu is 8192) is max length of single message;
// to unify max length and consider cross-platform, we define 4096 to use
#define MAX_MESSAGE_QUEUE_MESSAGE_LENGTH 4096

class BaseMessageQueue
{
protected:
    BaseMessageQueue(void);

public:
    virtual ~BaseMessageQueue(void);

    virtual GMI_RESULT Create( long_t Key )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Open( long_t Key )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Destroy()
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Send( const uint8_t *Buffer, size_t Size, boolean_t NoWait, size_t *Transferred )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Receive( uint8_t *Buffer, size_t Size, boolean_t NoWait, size_t *Transferred )
    {
        return GMI_NOT_IMPLEMENT;
    }
};
