#pragma once

#include "base_thread.h"

class LinuxThread : public BaseThread
{
public:
    LinuxThread(void);
    virtual ~LinuxThread(void);

#if defined( __linux__ )
    virtual GMI_RESULT Create( const char_t *Name, size_t StackSize, TRHEAD_FUNCTION Function, void_t *Argument );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Start();
    virtual GMI_RESULT Pause();
    virtual GMI_RESULT Resume();
    virtual GMI_RESULT Stop();
    static  long_t     GetCurrentId();

private:
    pthread_t		    m_Thread;
    size_t              m_StackSize;
    TRHEAD_FUNCTION     m_ThreadFunction;
    void_t              *m_ThreadFunctionArgument;
#endif
};
