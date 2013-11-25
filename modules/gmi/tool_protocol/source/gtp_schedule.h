#ifndef __GMI_SCHEDULE_H__
#define __GMI_SCHEDULE_H__

#include <gmi_type_definitions.h>
#include <gmi_errors.h>

#include <tool_protocol.h>

#include "gtp_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Initialize schedule task list
#define GtpScheduleListInit(list) GTP_LIST_INIT(list)

// Destroy schedule task list
GMI_RESULT GtpScheduleListDestroy(GtpList * ScheduledList);

// Add task
GMI_RESULT GtpScheduleAddDelayTask(GtpList * ScheduledList, CbOnSchedule CbFunc,
    void_t * CbData, uint32_t Sec, uint32_t USec);

// Update task, if not found, add it
GMI_RESULT GtpScheduleUpdateDelayTask(GtpList * ScheduledList, CbOnSchedule CbFunc,
    void_t * CbData, uint32_t Sec, uint32_t USec);

// Remove task
GMI_RESULT GtpScheduleCancelDelayTask(GtpList * ScheduledList, CbOnSchedule CbFunc,
    void_t * CbData);

// Schedule task
GMI_RESULT GtpScheduleTask(GtpList * ScheduledList);

#ifdef __cplusplus
}
#endif

#endif // __GMI_SCHEDULE_H__
