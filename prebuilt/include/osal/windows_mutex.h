#pragma once

#include "base_mutex.h"

class WindowsMutex : public BaseMutex
{
public:
    WindowsMutex(void);
    virtual ~WindowsMutex(void);

#if defined( _WIN32 )
    virtual GMI_RESULT Create( const char_t *Name );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Lock( long_t Timeout );
    virtual GMI_RESULT Unlock();
private:
    HANDLE	m_Mutex;
#endif
};
