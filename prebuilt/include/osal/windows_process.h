#pragma once

#include "base_process.h"

class WindowsProcess : public BaseProcess
{
public:
    WindowsProcess(void);
    virtual ~WindowsProcess(void);

#if defined( _WIN32 )
    static  long_t     GetCurrentId();
#endif
};
