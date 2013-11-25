#pragma once

#include "base_message_queue.h"

class WindowsMessageQueue : public BaseMessageQueue
{
public:
    WindowsMessageQueue(void);
    virtual ~WindowsMessageQueue(void);

#if defined( _WIN32 )
    virtual GMI_RESULT Create( long_t Key );
    virtual GMI_RESULT Open( long_t Key );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Send( const uint8_t *Buffer, size_t Size, boolean_t NoWait, size_t *Transferred );
    virtual GMI_RESULT Receive( uint8_t *Buffer, size_t Size, boolean_t NoWait, size_t *Transferred );

private:
    static  GMI_RESULT GenerateName( long_t Key, char_t *NameBuffer, size_t BufferLength );

private:
    long_t      m_Key;
    HANDLE	    m_MessageQueue;
    boolean_t   m_IsCreator;
#endif
};
