#pragma once

#include "event_processor.h"

typedef void_t (*EventNotifyCallback)( void_t *UserData, uint32_t EventType, void_t *Param, size_t ParamLength );

class EventProcessCenter : public EventProcessor
{
public:
	EventProcessCenter(void);
	virtual ~EventProcessCenter(void);

	virtual GMI_RESULT Notify( uint32_t EventType, void_t *Param, size_t ParamLength );

	GMI_RESULT Initialize();
	GMI_RESULT Deinitialize();
	GMI_RESULT RegisterEventProcessor( SafePtr<EventProcessor> Processor );
	GMI_RESULT UnregisterEventProcessor( enum ProcessorId Id );

private:
	GMI_Mutex  m_OperationLock;
};
