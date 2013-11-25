#pragma once

#include "base_named_pipe.h"

class WindowsNamedPipe : public BaseNamedPipe
{
public:
    WindowsNamedPipe(void);
    virtual ~WindowsNamedPipe(void);

#if defined( _WIN32 )
    virtual GMI_RESULT Create( const char_t *PipeName, long_t AccessMode );
    virtual GMI_RESULT Open( const char_t *PipeName, long_t AccessMode );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Write( const uint8_t *Buffer, size_t Size, size_t *Transferred );
    virtual GMI_RESULT Read( uint8_t *Buffer, size_t Size, size_t *Transferred );
    virtual FD_HANDLE  GetHandle();

private:
    static  GMI_RESULT GenerateName( const char_t *PipeName, char_t *NameBuffer, size_t BufferLength );
    long_t  GetSystemPipeAccessMode( long_t AccessMode );
    long_t  GetSystemFileAccessMode( long_t AccessMode );

private:
    HANDLE      m_Pipe;
    boolean_t   m_IsCreator;
#endif
};
