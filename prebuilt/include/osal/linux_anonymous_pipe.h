#pragma once

#include "base_anonymous_pipe.h"

class LinuxAnonymousPipe : public BaseAnonymousPipe
{
public:
    LinuxAnonymousPipe(void);
    virtual ~LinuxAnonymousPipe(void);

#if defined( __linux__ )
    virtual GMI_RESULT Create();
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Write( const uint8_t *Buffer, size_t Size, size_t *Transferred );
    virtual GMI_RESULT Read( uint8_t *Buffer, size_t Size, size_t *Transferred );
    virtual FD_HANDLE  GetWriteHandle();
    virtual FD_HANDLE  GetReadHandle();

private:
    long_t	m_ReadPort;
    long_t	m_WritePort;
#endif
};
