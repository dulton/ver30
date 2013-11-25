#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/time.h>
#endif

#include "gtp_schedule.h"

#include "debug.h"

#ifdef _WIN32
typedef union tagTimeStruct
{
	long long u_100NSec;
	FILETIME  u_FileTime;
} TimeStruct;

static void_t gettimeofday(struct timeval * TvPtr, void * TzPtr)
{
    TimeStruct NowTime;

    ASSERT(TvPtr != NULL, "TvPtr MUST NOT be non-pointer");
    ASSERT(TzPtr == NULL, "TzPtr MUST be non-pointer");

	GetSystemTimeAsFileTime (&NowTime.u_FileTime);
	TvPtr->tv_usec = (long)((NowTime.u_100NSec / 10LL) % 1000000LL);
	TvPtr->tv_sec = (long)((NowTime.u_100NSec - 116444736000000000LL) / 10000000LL);
}
#endif

typedef struct tagGtpScheduledTask
{
    CbOnSchedule     s_CbFunc;
    void_t         * s_CbData;
    struct timeval   s_Timeval;
    boolean_t        s_DeleteMark;

    GtpListNode      s_ListNode;
} GtpScheduledTask;

GMI_RESULT GtpScheduleListDestroy(GtpList * ScheduledList)
{
    GtpListNode * Node = NULL;

    if (NULL == ScheduledList)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    GTP_LIST_FOR_EACH(Node, ScheduledList)
    {
        GtpScheduledTask * Task = GTP_LIST_ENTRY(Node, GtpScheduledTask);
        Node = GTP_LIST_REMOVE(ScheduledList, Task);
        free(Task);
    }

    return GMI_SUCCESS;
}

GMI_RESULT GtpScheduleAddDelayTask(GtpList * ScheduledList, CbOnSchedule CbFunc,
    void_t * CbData, uint32_t Sec, uint32_t USec)
{
    GtpScheduledTask * Task = NULL;

    if (NULL == ScheduledList || NULL == CbFunc)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    Task = (GtpScheduledTask *)malloc(sizeof(GtpScheduledTask));
    if (NULL == Task)
    {
        PRINT_LOG(ERROR, "Not enough memory");
        return GMI_OUT_OF_MEMORY;
    }

    memset(Task, 0x00, sizeof(GtpScheduledTask));

    Task->s_CbFunc = CbFunc;
    Task->s_CbData = CbData;
    Task->s_DeleteMark = false;

    // Get current time
    gettimeofday(&Task->s_Timeval, NULL);

    // Set time up
    Task->s_Timeval.tv_sec += Sec;
    Task->s_Timeval.tv_usec += USec;
    while (Task->s_Timeval.tv_usec > 1000000)
    {
        Task->s_Timeval.tv_sec ++;
        Task->s_Timeval.tv_usec -= 1000000;
    }

    GTP_LIST_ADD(ScheduledList, Task);

    return GMI_SUCCESS;
}

static GtpScheduledTask * GtpScheduleFindTask(GtpList * ScheduledList,
    CbOnSchedule CbFunc, void_t * CbData)
{
    GtpListNode * Node = NULL;

    ASSERT(ScheduledList != NULL, "ScheduledList MUST NOT be non-pointer");
    ASSERT(CbFunc != NULL, "CbFunc MUST NOT be non-pointer");

    GTP_LIST_FOR_EACH(Node, ScheduledList)
    {
        GtpScheduledTask * Task = GTP_LIST_ENTRY(Node, GtpScheduledTask);
        if (Task->s_CbFunc == CbFunc && Task->s_CbData == CbData && Task->s_DeleteMark != true)
        {
            return Task;
        }
    }

    return NULL;
}

GMI_RESULT GtpScheduleUpdateDelayTask(GtpList * ScheduledList, CbOnSchedule CbFunc,
    void_t * CbData, uint32_t Sec, uint32_t USec)
{
    GtpScheduledTask * Task = NULL;

    if (NULL == ScheduledList || NULL == CbFunc)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    Task = GtpScheduleFindTask(ScheduledList, CbFunc, CbData);
    if (Task == NULL)
    {
        // PRINT_LOG(WARNING, "Could not find the task, add it");
        return GtpScheduleAddDelayTask(ScheduledList, CbFunc, CbData, Sec, USec);
    }

    // Get current time
    gettimeofday(&Task->s_Timeval, NULL);

    // Set time up
    Task->s_Timeval.tv_sec += Sec;
    Task->s_Timeval.tv_usec += USec;
    while (Task->s_Timeval.tv_usec > 1000000)
    {
        Task->s_Timeval.tv_sec ++;
        Task->s_Timeval.tv_usec -= 1000000;
    }

    return GMI_SUCCESS;
}

GMI_RESULT GtpScheduleCancelDelayTask(GtpList * ScheduledList, CbOnSchedule CbFunc,
    void_t * CbData)
{
    GtpScheduledTask * Task = NULL;

    if (NULL == ScheduledList || NULL == CbFunc)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    Task = GtpScheduleFindTask(ScheduledList, CbFunc, CbData);
    if (Task == NULL)
    {
        // PRINT_LOG(WARNING, "Could not find the task");
        return GMI_SUCCESS;
    }

    Task->s_DeleteMark = true;

    return GMI_SUCCESS;
}

GMI_RESULT GtpScheduleTask(GtpList * ScheduledList)
{
    GtpListNode    * Node = NULL;
    struct timeval   Now;

    if (NULL == ScheduledList)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    // Get current time
    gettimeofday(&Now, NULL);

    GTP_LIST_FOR_EACH(Node, ScheduledList)
    {
        GtpScheduledTask * Task = GTP_LIST_ENTRY(Node, GtpScheduledTask);

	if (Task->s_DeleteMark == true)
        {
            Node = GTP_LIST_REMOVE(ScheduledList, Task);
            free(Task);
            continue;
        }

        if (Now.tv_sec > Task->s_Timeval.tv_sec ||
            (Now.tv_sec == Task->s_Timeval.tv_sec &&
            Now.tv_usec > Task->s_Timeval.tv_usec))
        {
            Node = GTP_LIST_REMOVE(ScheduledList, Task);
            // Resume the task callback
            ASSERT(Task->s_CbFunc != NULL, "Task->s_CbFunc MUST NOT be non-pointer");

            Task->s_CbFunc(Task->s_CbData);

            free(Task);
        }
    }

    return GMI_SUCCESS;
}
