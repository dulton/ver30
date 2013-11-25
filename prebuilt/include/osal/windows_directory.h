#pragma once

#include "base_directory.h"

class WindowsDirectory : public BaseDirectory
{
public:
    WindowsDirectory(void);
    virtual ~WindowsDirectory(void);

    static GMI_RESULT MakeDirectory( const char_t *Path );
    static GMI_RESULT MakeDirectory( const wchar_t *Path );
    static GMI_RESULT RemoveDirectory( const char_t *Path );
    static GMI_RESULT RemoveDirectory( const wchar_t *Path );
};
