#pragma once

#include "gmi_system_headers.h"

class EventDetectCenter;
class EventProcessCenter;

class EventCenter
{
public:
	EventCenter(void);
	~EventCenter(void);

	GMI_RESULT Initialize();
	GMI_RESULT Deinitialize();

private:
	SafePtr<EventDetectCenter>   m_DetectCenter;
	SafePtr<EventProcessCenter>  m_ProcessCenter;
};
