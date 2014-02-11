#pragma once

#include "event_processor.h"
#include "event_transaction_header.h"

#define FILE_PATH_EVENT_LOG  "/opt/log/event_trigger_log"
#define LENGTH_EVENT_LOG_MAX                500*1024

class EventProcessInfoRecord : public EventProcessor
{
public:
    EventProcessInfoRecord( uint32_t EventProcessorId );
    virtual ~EventProcessInfoRecord(void);

    virtual GMI_RESULT Notify( uint32_t EventId, enum EventType Type, void_t *Parameter, size_t ParameterLength );

protected:
    virtual GMI_RESULT Start( const void_t *Parameter, size_t ParameterLength );
    virtual GMI_RESULT Stop();
};

