#pragma once

#include "event_detector.h"

class EventDetectCenter
{
public:
	EventDetectCenter(void);
	~EventDetectCenter(void);

	GMI_RESULT Initialize();
	GMI_RESULT Deinitialize();
};
