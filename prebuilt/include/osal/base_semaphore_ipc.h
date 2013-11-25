#pragma once

#include "cross_platform_headers.h"

class BaseSemaphoreIPC
{
protected:
    BaseSemaphoreIPC(void);

public:
    virtual ~BaseSemaphoreIPC(void);

    virtual GMI_RESULT Create( long_t Key, long_t InitCount )
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
    virtual GMI_RESULT Wait( long_t Timeout )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Post()
    {
        return GMI_NOT_IMPLEMENT;
    }
};
