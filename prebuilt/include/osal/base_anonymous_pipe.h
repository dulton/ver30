#pragma once

#include "cross_platform_headers.h"

class BaseAnonymousPipe
{
protected:
    BaseAnonymousPipe(void);

public:
    virtual ~BaseAnonymousPipe(void);

    virtual GMI_RESULT Create()
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Destroy()
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Write( const uint8_t *Buffer, size_t Size, size_t *Transferred )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Read( uint8_t *Buffer, size_t Size, size_t *Transferred )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual FD_HANDLE  GetWriteHandle()
    {
        return NULL;
    }
    virtual FD_HANDLE  GetReadHandle()
    {
        return NULL;
    }
};
