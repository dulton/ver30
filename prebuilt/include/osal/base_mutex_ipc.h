#pragma once

#include "cross_platform_headers.h"

class BaseMutexIPC
{
protected:
    BaseMutexIPC(void);

public:
    virtual ~BaseMutexIPC(void);

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
    virtual GMI_RESULT Lock( long_t Timeout )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Unlock()
    {
        return GMI_NOT_IMPLEMENT;
    }
};
