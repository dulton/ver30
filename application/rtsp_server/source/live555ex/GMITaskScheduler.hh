#ifndef _GMI_TASK_SCHEDULER_HH
#define _GMI_TASK_SCHEDULER_HH

#include <sys/select.h>

#include <UsageEnvironment.hh>

#include "task_queue.h"
#include "fd_listener.h"

class GMITaskScheduler : public TaskScheduler {
public:
    static GMITaskScheduler * createNew(unsigned maxSchedulerGranularity = 100000);

    virtual ~GMITaskScheduler();

    virtual TaskToken scheduleDelayedTask(int64_t microseconds, TaskFunc * proc, void * clientData);
    virtual void unscheduleDelayedTask(TaskToken & prevTask);
    virtual void rescheduleDelayedTask(TaskToken & task, int64_t microseconds, TaskFunc * proc, void * clientData);

    virtual void setBackgroundHandling(int socketNum, int conditionSet, BackgroundHandlerProc * handlerProc, void * clientData);
    virtual void moveSocketHandling(int oldSocketNum, int newSocketNum);

    virtual void doEventLoop(char * watchVariable = NULL);

    virtual EventTriggerId createEventTrigger(TaskFunc * eventHandlerProc);
    virtual void deleteEventTrigger(EventTriggerId eventTriggerId);
    virtual void triggerEvent(EventTriggerId eventTriggerId, void * clientData = NULL);

    virtual void internalError();

private:
    GMITaskScheduler(unsigned maxSchedulerGranularity);

    TaskQueue  fTaskQueue;
    FdListener fFdListener;
};

#endif
