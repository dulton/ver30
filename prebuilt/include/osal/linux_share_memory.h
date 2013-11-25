#pragma once

#include "base_share_memory.h"

class LinuxShareMemory : public BaseShareMemory
{
public:
    LinuxShareMemory(void);
    virtual ~LinuxShareMemory(void);

#if defined( __linux__ )
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
    key_t       m_Key;
    long_t	    m_ShareMemoryId;
    boolean_t   m_IsCreator;
    size_t		m_Size;
    void_t		*m_Address;
#endif
};
