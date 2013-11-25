#pragma once

#include "cross_platform_headers.h"

#define GMI_FILE_ACCESS_READ    1
#define GMI_FILE_ACCESS_WRITE   2
#define GMI_FILE_ACCESS_EXECUTE 4

#define GMI_FILE_SHARE_NONE     0
#define GMI_FILE_SHARE_READ     1
#define GMI_FILE_SHARE_WRITE    2
#define GMI_FILE_SHARE_EXECUTE  4
#define GMI_FILE_SHARE_DELETE   8

#define GMI_FILE_SEEK_BEGIN     0
#define GMI_FILE_SEEK_CURRENT   1
#define GMI_FILE_SEEK_END       2

class BaseFile
{
protected:
    BaseFile(void);
public:
    virtual ~BaseFile(void);

    virtual GMI_RESULT Create( const char_t *Path, long_t AccessMode, long_t ShareMode, long_t ExtentArgument )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Open( const char_t *Path, long_t AccessMode, long_t ShareMode, long_t ExtentArgument )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Close()
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Read( uint8_t *Buffer, size_t Size, size_t *Transferred )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Write( const uint8_t *Buffer, size_t Size, size_t *Transferred )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Seek( long_t SeekMethod, int64_t Offset )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual uint64_t   Size() const
    {
        return 0;
    }
};
