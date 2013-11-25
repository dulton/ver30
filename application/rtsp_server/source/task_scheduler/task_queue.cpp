#include "task_queue.h"

static inline struct timespec & operator += (struct timespec & Time, int64_t USec)
{
    // Convert to nano second
    int64_t NSec = USec * 1000;

    NSec += Time.tv_nsec;
    Time.tv_sec += NSec / 1000000000;
    Time.tv_nsec = NSec % 1000000000;
    return Time;
}

static inline uint32_t operator - (const struct timespec & Time1, const struct timespec & Time2)
{
    int64_t Sec  = Time1.tv_sec  - Time2.tv_sec;
    int64_t NSec = Time1.tv_nsec - Time2.tv_nsec;
    int32_t USec = (Sec * 1000000) + (NSec / 1000);

    return USec > 0 ? USec : 0;
}

static inline boolean_t operator < (const struct timespec & Time1, const struct timespec & Time2)
{
    if (Time1.tv_sec < Time2.tv_sec)
    {
        return true;
    }
    else if (Time1.tv_sec == Time2.tv_sec)
    {
        return (Time1.tv_nsec < Time2.tv_nsec);
    }

    return false;
}

static inline boolean_t operator <= (const struct timespec & Time1, const struct timespec & Time2)
{
    if (Time1.tv_sec < Time2.tv_sec)
    {
        return true;
    }
    else if (Time1.tv_sec == Time2.tv_sec)
    {
        return (Time1.tv_nsec <= Time2.tv_nsec);
    }

    return false;
}

// Task token 0 is used for TaskQueue
uint32_t TaskEntry::ms_TaskCount = 0;

TaskEntry::TaskEntry()
    : Entry()
    , m_TaskFunc(NULL)
    , m_ClientData(NULL)
    , m_TaskToken(NULL)
{
}

TaskEntry::TaskEntry(const struct timespec & ExeTime, TaskFunc * Proc, void_t * ClientData)
    : Entry()
    , m_ExeTime(ExeTime)
    , m_TaskFunc(Proc)
    , m_ClientData(ClientData)
    , m_TaskToken((TaskToken)++ ms_TaskCount)
{
}

TaskEntry::~TaskEntry()
{
}

TaskQueue::TaskQueue(uint32_t MaxSchedulerGranularity)
    : TaskEntry()
    , m_MaxSchedulerGranularity(MaxSchedulerGranularity)
    , m_Initialized(false)
{
}

TaskQueue::~TaskQueue()
{
    if (Initialized())
    {
        Uninitialize();
    }

    for (Entry * Task = Next(); Task != this; Task = Next())
    {
         Task->Detach();
         delete Task;
    }
}

GMI_RESULT TaskQueue::Initialize()
{
    if (Initialized())
    {
        PRINT_LOG(WARNING, "TaskQueue is already initialized");
        return GMI_ALREADY_OPERATED;
    }

    m_Initialized = true;

    return GMI_SUCCESS;
}

GMI_RESULT TaskQueue::Uninitialize()
{
    if (!Initialized())
    {
        PRINT_LOG(WARNING, "TaskQueue is not initialized yet");
        return GMI_ALREADY_OPERATED;
    }

    m_Initialized = false;

    return GMI_SUCCESS;
}

TaskToken TaskQueue::AddTask(int64_t DelayUSec, TaskFunc * Proc, void_t * ClientData)
{
    if (!Initialized())
    {
        PRINT_LOG(WARNING, "TaskQueue is not initialized yet");
        return NULL;
    }

    if (NULL == Proc)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return NULL;
    }

    struct timespec ExeTime = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &ExeTime);
    if (DelayUSec > 0)
    {
        ExeTime += DelayUSec;
    }

    TaskEntry * NewTask = new TaskEntry(ExeTime, Proc, ClientData);
    if (NewTask == NULL)
    {
        PRINT_LOG(ERROR, "Not enough memory");
        return NULL;
    }

    TaskEntry * Task = dynamic_cast<TaskEntry *>(Next());
    while (Task != this && Task->ExecuteTime() <= ExeTime)
    {
        Task = dynamic_cast<TaskEntry *>(Task->Next());
    }

    NewTask->InsertBefore(Task);
    return NewTask->ThisTask();
}

void_t TaskQueue::RemoveTask(const TaskToken & Token)
{
    if (!Initialized())
    {
        PRINT_LOG(WARNING, "TaskQueue is not initialized yet");
        return;
    }

    TaskEntry * Task = FindTask(Token);
    if (Task != NULL)
    {
        Task->Detach();
        delete Task;
    }
}

void_t TaskQueue::DoEventLoop(uint8_t * StopFlag)
{
    if (!Initialized())
    {
        PRINT_LOG(WARNING, "TaskQueue is not initialized yet");
        return;
    }

    PRINT_LOG(VERBOSE, "TaskQueue::DoEventLoop start");

    struct timespec Time      = {0, 0};
    uint32_t        SleepTime = m_MaxSchedulerGranularity;

    while (NULL == StopFlag || 0 == *StopFlag)
    {
        clock_gettime(CLOCK_MONOTONIC, &Time);

        TaskEntry * Task = dynamic_cast<TaskEntry *>(Next());
        while (Task != this && Task->ExecuteTime() <= Time)
        {
            Task->Detach();

            Task->Execute();
            delete Task;

            clock_gettime(CLOCK_MONOTONIC, &Time);
            Task = dynamic_cast<TaskEntry *>(Next());
        }

        if (Task != this)
        {
            SleepTime = Task->ExecuteTime() - Time;
            if (SleepTime > m_MaxSchedulerGranularity)
            {
                SleepTime = m_MaxSchedulerGranularity;
            }
        }

        // Wait until time up
        if (SleepTime > 0)
        {
            usleep(SleepTime);
        }
    }

    PRINT_LOG(VERBOSE, "TaskQueue::DoEventLoop end");
}

TaskEntry * TaskQueue::FindTask(const TaskToken & Token) const
{
    TaskEntry * Task = dynamic_cast<TaskEntry *>(Next());
    while (Task != this)
    {
         if (Task->IsThisTask(Token))
         {
             return Task;
         }

         Task = dynamic_cast<TaskEntry *>(Task->Next());
    }

    return NULL;
}
