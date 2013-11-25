#pragma once

#include "cross_platform_headers.h"

typedef void_t* (*TRHEAD_FUNCTION)( void_t *Argument );

class BaseThread
{
protected:
    BaseThread(void);

public:
    virtual ~BaseThread(void);

    virtual GMI_RESULT Create( const char_t *Name, size_t StackSize, TRHEAD_FUNCTION Function, void_t *Argument )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Destroy()
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Start()
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Pause()
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Resume()
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Stop()
    {
        return GMI_NOT_IMPLEMENT;
    }
    static  long_t     GetCurrentId();
};
