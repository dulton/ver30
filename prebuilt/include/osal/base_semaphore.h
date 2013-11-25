#pragma once

#include "cross_platform_headers.h"

class BaseSemaphore
{
protected:
    BaseSemaphore(void);

public:
    virtual ~BaseSemaphore(void);

    virtual GMI_RESULT Create( const char_t *Name, long_t InitCount )
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
    virtual GMI_RESULT Post()
    {
        return GMI_NOT_IMPLEMENT;
    }
};
