#pragma once

#include "gmi_system_headers.h"

class BaseDelayTask
{
protected:
    BaseDelayTask( uint64_t ScheduleTime );
public:
    virtual ~BaseDelayTask(void);

    virtual GMI_RESULT Execute() = 0;
    virtual GMI_RESULT ReleaseResource() = 0;

    GMI_RESULT Synchronize( uint64_t SystemTime );

    boolean_t operator >  ( const BaseDelayTask& Task );
    boolean_t operator <  ( const BaseDelayTask& Task );
    boolean_t operator == ( const BaseDelayTask& Task );
    boolean_t operator >= ( const BaseDelayTask& Task );
    boolean_t operator <= ( const BaseDelayTask& Task );

    inline uint64_t GetScheduleTime() const
    {
        return m_ScheduleTime;
    }
    inline void_t   SetScheduleTime( uint64_t ScheduleTime )
    {
        m_ScheduleTime = ScheduleTime;
    }

protected:
    uint64_t m_ScheduleTime;
};
