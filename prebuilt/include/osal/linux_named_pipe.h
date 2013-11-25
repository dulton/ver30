#pragma once

#include "base_named_pipe.h"
#include "safe_ptr.h"

class LinuxNamedPipe : public BaseNamedPipe
{
public:
    LinuxNamedPipe(void);
    virtual ~LinuxNamedPipe(void);

#if defined( __linux__ )
    virtual GMI_RESULT Create( const char_t *PipeName, long_t AccessMode );
    virtual GMI_RESULT Open( const char_t *PipeName, long_t AccessMode );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Write( const uint8_t *Buffer, size_t Size, size_t *Transferred );
    virtual GMI_RESULT Read( uint8_t *Buffer, size_t Size, size_t *Transferred );
    virtual FD_HANDLE  GetHandle();

private:
    static  GMI_RESULT GenerateName( const char_t *PipeName, char_t *NameBuffer, size_t BufferLength );
    long_t  GetSystemAccessMode( long_t AccessMode );

private:
    long_t	                                m_Pipe;
    boolean_t                                   m_IsCreator;
    SafePtr< char_t,DefaultObjectsDeleter >     m_FullPipeName;
#endif
};
