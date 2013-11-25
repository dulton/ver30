#pragma once

#include "cross_platform_headers.h"

class BaseEvent
{
protected:
    BaseEvent(void);

public:
    virtual ~BaseEvent(void);

    virtual GMI_RESULT Create( const char_t *Name )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Destroy()
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Wait( long_t Timeout )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Signal()
    {
        return GMI_NOT_IMPLEMENT;
    }
};
