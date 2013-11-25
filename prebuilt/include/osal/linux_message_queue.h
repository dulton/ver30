#pragma once

#include "base_message_queue.h"

class LinuxMessageQueue : public BaseMessageQueue
{
public:
    LinuxMessageQueue(void);
    virtual ~LinuxMessageQueue(void);

#if defined( __linux__ )
    virtual GMI_RESULT Create( long_t Key );
    virtual GMI_RESULT Open( long_t Key );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Send( const uint8_t *Buffer, size_t Size, boolean_t NoWait, size_t *Transferred );
    virtual GMI_RESULT Receive( uint8_t *Buffer, size_t Size, boolean_t NoWait, size_t *Transferred );

private:
    struct GMI_Message
    {
        long_t MessageType;
        long_t MessageLength;
        char_t MessageData[MAX_MESSAGE_QUEUE_MESSAGE_LENGTH];
    };
private:
    key_t       m_Key;
    long_t	    m_MessageQueueId;
    boolean_t   m_IsCreator;
#endif
};
