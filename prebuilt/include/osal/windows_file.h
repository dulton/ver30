#pragma once

#include "base_file.h"

class WindowsFile : public BaseFile
{
public:
    WindowsFile(void);
    virtual ~WindowsFile(void);

#if defined( _WIN32 )
    virtual GMI_RESULT Create( const char_t *Path, long_t AccessMode, long_t ShareMode, long_t ExtentArgument );
    virtual GMI_RESULT Open( const char_t *Path, long_t AccessMode, long_t ShareMode, long_t ExtentArgument );
    virtual GMI_RESULT Close();
    virtual GMI_RESULT Read( uint8_t *Buffer, size_t Size, size_t *Transferred );
    virtual GMI_RESULT Write( const uint8_t *Buffer, size_t Size, size_t *Transferred );
    virtual GMI_RESULT Seek( long_t SeekMethod, int64_t Offset );
    virtual uint64_t   Size() const;

private:
    long_t GetSystemAccessMode( long_t AccessMode );
    long_t GetSystemShareMode( long_t ShareMode );
    long_t GetSystemExtentArgument( long_t ExtentArgument );
    long_t GetSystemSeekMethod( long_t SeekMethod );

private:
    HANDLE m_File;
#endif
};
