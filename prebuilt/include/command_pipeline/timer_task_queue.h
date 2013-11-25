#pragma once

#include "base_delay_task.h"
#include "gmi_system_headers.h"

#if !defined( DELAY_TASK_FUNCTION1_PROTOTYPE )
#define DELAY_TASK_FUNCTION1_PROTOTYPE
typedef void_t (*DELAY_TASK_FUNCTION1)( void_t *Argument );
#endif//DELAY_TASK_FUNCTION1_PROTOTYPE

#if !defined( DELAY_TASK_FUNCTION2_PROTOTYPE )
#define DELAY_TASK_FUNCTION2_PROTOTYPE
typedef void_t (*DELAY_TASK_FUNCTION2)( void_t *Argument1, void_t *Argument2 );
#endif//DELAY_TASK_FUNCTION2_PROTOTYPE

typedef void_t* TASK_HANDLE;

class TimerTaskQueue
{
public:
    TimerTaskQueue(void);
    ~TimerTaskQueue(void);

    GMI_RESULT  Initialize();
    GMI_RESULT  Deinitialize();

    TASK_HANDLE Schedule1( uint64_t RelativeTime, DELAY_TASK_FUNCTION1 TaskFunction, DELAY_TASK_FUNCTION1 ResourceReleaseFunction, void_t *Argument );
    TASK_HANDLE Schedule1( uint64_t RelativeTime, DELAY_TASK_FUNCTION2 TaskFunction, DELAY_TASK_FUNCTION2 ResourceReleaseFunction, void_t *Argument1, void_t *Argument2 );
    TASK_HANDLE Schedule2( uint64_t SystemTime,   DELAY_TASK_FUNCTION1 TaskFunction, DELAY_TASK_FUNCTION1 ResourceReleaseFunction, void_t *Argument );
    TASK_HANDLE Schedule2( uint64_t SystemTime,   DELAY_TASK_FUNCTION2 TaskFunction, DELAY_TASK_FUNCTION2 ResourceReleaseFunction, void_t *Argument1, void_t *Argument2 );
    GMI_RESULT  Unschedule ( TASK_HANDLE& Task );
    GMI_RESULT  Reschedule1( TASK_HANDLE& Task, uint64_t RelativeTime );
    GMI_RESULT  Reschedule2( TASK_HANDLE& Task, uint64_t SystemTime );
    GMI_RESULT  Synchronize( uint64_t SystemTime );

private:
    GMI_RESULT  InsertTask ( SafePtr<BaseDelayTask>& Task );
    GMI_RESULT  CleanupTask();

private:
    std::list<SafePtr<BaseDelayTask> > m_TaskQueue;
    GMI_Mutex                          m_TaskQueueMutex;
};

#if defined( __linux__ )
#define gettimeofday1 gettimeofday
extern int32_t gettimeofday_monotonic( struct timeval *TimeValue, int32_t *TimeZone );
#elif defined( _WIN32 )
extern int32_t gettimeofday1( struct timeval *TimeValue, int32_t *TimeZone );
#define gettimeofday_monotonic gettimeofday1
#endif
extern uint64_t TimevalToUInt64( struct timeval *TimeValue );
