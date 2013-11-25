#pragma once

#include "cross_platform_headers.h"

class BaseShareMemory
{
protected:
    BaseShareMemory(void);

public:
    virtual ~BaseShareMemory(void);

    virtual GMI_RESULT Create( long_t Key, size_t Size )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Open( long_t Key )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Destroy( boolean_t ForceReleaseShareMemorySpace )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Map( void_t *Address, size_t Offset, size_t MapSize )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Unmap()
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Write( size_t Offset, const uint8_t *Buffer, size_t Size, size_t *Transferred )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Read( size_t Offset, uint8_t *Buffer, size_t Size, size_t *Transferred )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual void_t*    GetAddress()
    {
        return NULL;
    }
    virtual size_t     GetSize()
    {
        return 0;
    }
};
