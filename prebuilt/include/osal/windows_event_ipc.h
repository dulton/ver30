#pragma once

#include "base_event_ipc.h"

class WindowsEventIPC : public BaseEventIPC
{
public:
    WindowsEventIPC(void);
    virtual ~WindowsEventIPC(void);

#if defined( _WIN32 )
    virtual GMI_RESULT Create( long_t Key );
    virtual GMI_RESULT Open( long_t Key );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Wait( long_t Timeout );
    virtual GMI_RESULT Signal();

private:
    static  GMI_RESULT GenerateName( long_t Key, char_t *NameBuffer, size_t BufferLength );

private:
    long_t      m_Key;
    HANDLE		m_EventIPC;
#endif
};
