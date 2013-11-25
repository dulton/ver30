#pragma once

#include "cross_platform_headers.h"

class BaseMutex
{
protected:
    BaseMutex(void);

public:
    virtual ~BaseMutex(void);

    virtual GMI_RESULT Create( const char_t *Name )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Destroy()
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Lock( long_t Timeout )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Unlock()
    {
        return GMI_NOT_IMPLEMENT;
    }
};
