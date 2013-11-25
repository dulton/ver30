#pragma once

#include "base_semaphore.h"

class LinuxSemaphore : public BaseSemaphore
{
public:
    LinuxSemaphore(void);
    virtual ~LinuxSemaphore(void);

#if defined( __linux__ )
    virtual GMI_RESULT Create( const char_t *Name, long_t InitCount );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Wait( long_t Timeout );
    virtual GMI_RESULT Post();

private:
    sem_t  m_Semaphore;
#endif
};
