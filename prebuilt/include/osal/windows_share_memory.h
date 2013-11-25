#pragma once

#include "base_share_memory.h"

#define WINDOWS_SHARE_MEMORY_RESERVED_SPACE_SIZE 16
#define WINDOWS_SHARE_MEMORY_PAGE_UINT_SIZE      4096

class WindowsShareMemory : public BaseShareMemory
{
public:
    WindowsShareMemory(void);
    virtual ~WindowsShareMemory(void);

#if defined( _WIN32 )
    virtual GMI_RESULT Create( long_t Key, size_t Size );
    virtual GMI_RESULT Open( long_t Key );
    virtual GMI_RESULT Destroy( boolean_t ForceReleaseShareMemorySpace );
    virtual GMI_RESULT Map( void_t *Address, size_t Offset, size_t MapSize );
    virtual GMI_RESULT Unmap();
    virtual GMI_RESULT Write( size_t Offset, const uint8_t *Buffer, size_t Size, size_t *Transferred );
    virtual GMI_RESULT Read( size_t Offset, uint8_t *Buffer, size_t Size, size_t *Transferred );
    virtual void_t*    GetAddress();
    virtual size_t     GetSize();

private:
    static  GMI_RESULT GenerateName( long_t Key, char_t *NameBuffer, size_t BufferLength );

private:
    long_t      m_Key;
    HANDLE	    m_ShareMemory;
    boolean_t   m_IsCreator;
    size_t		m_Size;
    void_t		*m_Address;
#endif
};
