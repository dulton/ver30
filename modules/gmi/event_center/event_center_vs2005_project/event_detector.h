#pragma once

#include "event_process_header.h"

class EventDetector
{
protected:
	EventDetector( enum EventCenterDetectorId Id ) : m_DetectorId( Id ), m_Handle( NULL ) {}

public:
	virtual ~EventDetector(void) {}

	enum EventCenterDetectorId GetDetectorId() const { return m_DetectorId; }
	FD_HANDLE GetFDHandle() const { return m_Handle; }

private:
	enum EventCenterDetectorId  m_DetectorId;
protected:
	FD_HANDLE                   m_Handle;
};
