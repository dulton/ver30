#pragma once

#include "base_thread.h"

class WindowsThread : public BaseThread
{
public:
    WindowsThread(void);
    virtual ~WindowsThread(void);

#if defined( _WIN32 )
    virtual GMI_RESULT Create( const char_t *Name, size_t StackSize, TRHEAD_FUNCTION Function, void_t *Argument );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Start();
    virtual GMI_RESULT Pause();
    virtual GMI_RESULT Resume();
    virtual GMI_RESULT Stop();
    static  long_t     GetCurrentId();

private:
    static DWORD WINAPI ThreadProc( LPVOID lpParameter )
    {
        WindowsThread *Thread = reinterpret_cast<WindowsThread*>(lpParameter);
        void_t *ReturnValue = Thread->m_ThreadFunction( Thread->m_ThreadFunctionArgument );
        size_t ReturnInteger = (size_t) ReturnValue;
        return (DWORD) ReturnInteger;
    }

private:
    HANDLE				m_Thread;
    size_t              m_StackSize;
    TRHEAD_FUNCTION		m_ThreadFunction;
    void_t				*m_ThreadFunctionArgument;
#endif
};
