#pragma once

#include "cross_platform_headers.h"

class BaseProcess
{
protected:
    BaseProcess(void);

public:
    virtual ~BaseProcess(void);

    virtual GMI_RESULT Create( const char_t *Path, char_t *CommandLine )
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
    virtual GMI_RESULT Stop()
    {
        return GMI_NOT_IMPLEMENT;
    }
    static  long_t     GetCurrentId();
};
