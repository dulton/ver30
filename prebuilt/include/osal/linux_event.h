#pragma once

#include "base_event.h"

class LinuxEvent : public BaseEvent
{
public:
    LinuxEvent(void);
    virtual ~LinuxEvent(void);

#if defined( __linux__ )
    virtual GMI_RESULT Create( const char_t *Name );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Wait( long_t Timeout );
    virtual GMI_RESULT Signal();

private:
    GMI_RESULT WaitMutex( long_t Timeout );

private:
    pthread_mutex_t    m_Mutex;
    pthread_cond_t     m_ConditionVariable;
#endif
};
