#pragma once

#include "base_event.h"

class WindowsEvent : public BaseEvent
{
public:
    WindowsEvent(void);
    virtual ~WindowsEvent(void);

#if defined( _WIN32 )
    virtual GMI_RESULT Create( const char_t *Name );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Wait( long_t Timeout );
    virtual GMI_RESULT Signal();

private:
    HANDLE	m_Event;
#endif
};
