#pragma once

#include "base_semaphore.h"

class WindowsSemaphore : public BaseSemaphore
{
public:
    WindowsSemaphore(void);
    virtual ~WindowsSemaphore(void);

#if defined( _WIN32 )
    virtual GMI_RESULT Create( const char_t *Name, long_t InitCount );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Wait( long_t Timeout );
    virtual GMI_RESULT Post();

private:
    HANDLE	m_Semaphore;
#endif
};
