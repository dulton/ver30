#ifndef __TASK_QUEUE_H__
#define __TASK_QUEUE_H__

#include <UsageEnvironment.hh>

#include "common_def.h"

class TaskEntry : public Entry
{
public:
    TaskEntry(const struct timespec & ExeTime, TaskFunc * Proc, void_t * ClientData);
    virtual ~TaskEntry();

    inline const struct timespec & ExecuteTime() const { return m_ExeTime; }
    inline void_t Execute() { m_TaskFunc(m_ClientData); }
    inline TaskToken ThisTask() const { return m_TaskToken; }

    inline boolean_t IsThisTask(TaskToken Token) const { return Token == m_TaskToken; }

protected:
    TaskEntry();

private:
    struct timespec   m_ExeTime;
    TaskFunc        * m_TaskFunc;
    void_t          * m_ClientData;
    TaskToken         m_TaskToken;

    // Used to generate task token
    static uint32_t   ms_TaskCount;
};

class TaskQueue : protected TaskEntry
{
public:
    TaskQueue(uint32_t MaxSchedulerGranularity);
    ~TaskQueue();

    GMI_RESULT Initialize();
    GMI_RESULT Uninitialize();

    inline boolean_t Initialized() const { return m_Initialized; }

    TaskToken AddTask(int64_t DelayUSec, TaskFunc * Proc, void_t * ClientData);
    void_t RemoveTask(const TaskToken & Token);

    void_t DoEventLoop(uint8_t * StopFlag = NULL);

private:
    TaskEntry * FindTask(const TaskToken & Token) const;

    uint32_t           m_MaxSchedulerGranularity;
    boolean_t          m_Initialized;
};

#endif // __TASK_QUEUE_H__
