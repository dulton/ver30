
#if !defined( EVENT_PROCESS_HEADER )
#define EVENT_PROCESS_HEADER

#include "gmi_system_headers.h"

#define EVENT_PRIORITY_LOW      0
#define EVENT_PRIORITY_MEDIUM   1
#define EVENT_PRIORITY_HIGH     2

// event id macro
#define MAKE_DETECTOR_ID(p,i)   ((p<<16)+i)
#define QUERY_EVENT_PRIORITY(x) (((uint32_t)x)>>16)

enum EventDetectorType
{
    e_EventDetectorType_Active = 1,
    e_EventDetectorType_Passive,
};

#endif//EVENT_PROCESS_HEADER
