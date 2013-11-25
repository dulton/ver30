#pragma once

#include "base_process.h"

class LinuxProcess : public BaseProcess
{
public:
    LinuxProcess(void);
    virtual ~LinuxProcess(void);

#if defined( __linux__ )
    static  long_t     GetCurrentId();
#endif
};
