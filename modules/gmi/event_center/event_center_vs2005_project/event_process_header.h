
#if !defined( EVENT_PROCESS_HEADER )
#define EVENT_PROCESS_HEADER

#include "gmi_system_headers.h"

#define EVENT_PRIORITY_LOW      0
#define EVENT_PRIORITY_MEDIUM   1
#define EVENT_PRIORITY_HIGH     2

#define MAKE_DETECTOR_ID(p,i)   ((p<<16)+i)
#define QUERY_EVENT_PRIORITY(x) (((uint32_t)x)>>16)

// priority need to set according to configuration, which improve flexibility.
enum EventCenterDetectorId
{
	e_EventCenterDetectorId_GPIO = 0,
};

enum EventCenterProcessorId
{
	e_EventCenterProcessorId_ProcessCenter = 0,
};

#endif//EVENT_PROCESS_HEADER
