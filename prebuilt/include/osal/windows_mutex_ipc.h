#pragma once

#include "base_mutex_ipc.h"

class WindowsMutexIPC : public BaseMutexIPC
{
public:
    WindowsMutexIPC(void);
    virtual ~WindowsMutexIPC(void);

#if defined( _WIN32 )
    virtual GMI_RESULT Create( long_t Key );
    virtual GMI_RESULT Open( long_t Key );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Lock( long_t Timeout );
    virtual GMI_RESULT Unlock();

private:
    static  GMI_RESULT GenerateName( long_t Key, char_t *NameBuffer, size_t BufferLength );

private:
    long_t      m_Key;
    HANDLE		m_MutexIPC;
#endif
};
