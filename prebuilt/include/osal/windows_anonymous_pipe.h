#pragma once

#include "base_anonymous_pipe.h"

class WindowsAnonymousPipe : public BaseAnonymousPipe
{
public:
    WindowsAnonymousPipe(void);
    virtual ~WindowsAnonymousPipe(void);

#if defined( _WIN32 )
    virtual GMI_RESULT Create();
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Write( const uint8_t *Buffer, size_t Size, size_t *Transferred );
    virtual GMI_RESULT Read( uint8_t *Buffer, size_t Size, size_t *Transferred );
    virtual FD_HANDLE  GetWriteHandle();
    virtual FD_HANDLE  GetReadHandle();

private:
    HANDLE	m_ReadPort;
    HANDLE	m_WritePort;
#endif
};
