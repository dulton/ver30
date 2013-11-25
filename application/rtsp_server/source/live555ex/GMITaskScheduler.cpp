#include "GMITaskScheduler.hh"

#include "debug.h"

GMITaskScheduler * GMITaskScheduler::createNew(unsigned maxSchedulerGranularity) {
    GMITaskScheduler * scheduler = NULL;

    do {
        scheduler = new GMITaskScheduler(maxSchedulerGranularity);
        if (NULL == scheduler) {
            PRINT_LOG(ERROR, "Not enough memory");
            break;
        }

        if (GMI_SUCCESS != scheduler->fTaskQueue.Initialize()) {
            PRINT_LOG(ERROR, "Failed to initialize TaskQueue");
            break;
        }

        if (GMI_SUCCESS != scheduler->fFdListener.Initialize()) {
            PRINT_LOG(ERROR, "Failed to initialize FdListener");
            break;
        }

        return scheduler;
    } while (0);

    if (scheduler != NULL) {
        delete scheduler;
    }

    return NULL;
}

GMITaskScheduler::GMITaskScheduler(unsigned maxSchedulerGranularity)
    : TaskScheduler()
    , fTaskQueue(maxSchedulerGranularity)
    , fFdListener(fTaskQueue, maxSchedulerGranularity) {
}

GMITaskScheduler::~GMITaskScheduler() {
    if (fFdListener.Initialized()) {
        fFdListener.Uninitialize();
    }

    if (fTaskQueue.Initialized()) {
        fTaskQueue.Uninitialize();
    }
}

TaskToken GMITaskScheduler::scheduleDelayedTask(int64_t microseconds, TaskFunc * proc, void * clientData) {
    return fTaskQueue.AddTask(microseconds, proc, clientData);
}

void GMITaskScheduler::unscheduleDelayedTask(TaskToken & prevTask) {
    fTaskQueue.RemoveTask(prevTask);
}

void GMITaskScheduler::rescheduleDelayedTask(TaskToken & task, int64_t microseconds, TaskFunc * proc, void * clientData) {
    fTaskQueue.RemoveTask(task);
    task = fTaskQueue.AddTask(microseconds, proc, clientData);
}

void GMITaskScheduler::setBackgroundHandling(int socketNum, int conditionSet, BackgroundHandlerProc * handlerProc, void * clientData) {
    if (0 == conditionSet || NULL == handlerProc)
    {
        fFdListener.UnregisterFdHandler(socketNum);
    }
    else
    {
        fFdListener.RegisterFdHandler(socketNum, conditionSet, handlerProc, clientData);
    }
}

void GMITaskScheduler::moveSocketHandling(int oldSocketNum, int newSocketNum) {
    fFdListener.MoveFdHandler(oldSocketNum, newSocketNum);
}

void GMITaskScheduler::doEventLoop(char * watchVariable) {
    fTaskQueue.DoEventLoop((uint8_t *)watchVariable);
}

EventTriggerId GMITaskScheduler::createEventTrigger(TaskFunc * eventHandlerProc) {
    PRINT_LOG(WARNING, "Not implement yet");
    return 0;
}

void GMITaskScheduler::deleteEventTrigger(EventTriggerId eventTriggerId) {
    PRINT_LOG(WARNING, "Not implement yet");
}

void GMITaskScheduler::triggerEvent(EventTriggerId eventTriggerId, void * clientData) {
    PRINT_LOG(WARNING, "Not implement yet");
}

void GMITaskScheduler::internalError() {
    PRINT_LOG(ERROR, "Internal error happened");
    TaskScheduler::internalError();
}

