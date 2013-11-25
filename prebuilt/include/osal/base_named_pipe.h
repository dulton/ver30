#pragma once

#include "cross_platform_headers.h"

#define GMI_NAMED_PIPE_ACCESS_READ	            1
#define GMI_NAMED_PIPE_ACCESS_WRITE	            2
#define GMI_NAMED_PIPE_ACCESS_NONBLOCK	        4

#define GMI_NAMED_PIPE_MAX_INPUT_BUFFER_SIZE    4096
#define GMI_NAMED_PIPE_MAX_OUTPUT_BUFFER_SIZE   4096

class BaseNamedPipe
{
protected:
    BaseNamedPipe(void);

public:
    virtual ~BaseNamedPipe(void);

    virtual GMI_RESULT Create( const char_t *PipeName, long_t AccessMode )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Open( const char_t *PipeName, long_t AccessMode )
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
    virtual FD_HANDLE  GetHandle()
    {
        return NULL;
    }
};
