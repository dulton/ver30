#pragma once

#include "cross_platform_headers.h"

class BaseDirectory
{
protected:
    BaseDirectory(void);
public:
    virtual ~BaseDirectory(void);

    static GMI_RESULT MakeDirectory( const char_t *Path );
    static GMI_RESULT MakeDirectory( const wchar_t *Path );
    static GMI_RESULT RemoveDirectory( const char_t *Path );
    static GMI_RESULT RemoveDirectory( const wchar_t *Path );
};
