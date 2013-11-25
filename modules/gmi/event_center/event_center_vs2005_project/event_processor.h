#pragma once

#include "event_process_header.h"

class EventProcessor
{
protected:
	EventProcessor( enum EventCenterProcessorId Id ) : m_ProcessorId( Id ) {}

public:
	virtual ~EventProcessor(void) {}

	virtual GMI_RESULT Notify( uint32_t EventType, void_t *Param, size_t ParamLength ) = 0;
	enum EventCenterProcessorId GetProcessorId() const { return m_ProcessorId; }

private:
	enum EventCenterProcessorId  m_ProcessorId;
};
