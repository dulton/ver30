#pragma once

#include "base_semaphore_ipc.h"

class WindowsSemaphoreIPC : public BaseSemaphoreIPC
{
public:
    WindowsSemaphoreIPC(void);
    virtual ~WindowsSemaphoreIPC(void);

#if defined( _WIN32 )
    virtual GMI_RESULT Create( long_t Key, long_t InitCount );
    virtual GMI_RESULT Open( long_t Key );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Wait( long_t Timeout );
    virtual GMI_RESULT Post();

private:
    static  GMI_RESULT GenerateName( long_t Key, char_t *NameBuffer, size_t BufferLength );

private:
    long_t      m_Key;
    HANDLE		m_SemaphoreIPC;
#endif
};
