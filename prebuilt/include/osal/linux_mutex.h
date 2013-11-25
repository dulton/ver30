#pragma once

#include "base_mutex.h"

class LinuxMutex : public BaseMutex
{
public:
    LinuxMutex(void);
    virtual ~LinuxMutex(void);

#if defined( __linux__ )
    virtual GMI_RESULT Create( const char_t *Name );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Lock( long_t Timeout );
    virtual GMI_RESULT Unlock();

private:
    pthread_mutex_t   m_Mutex;
#endif
};
