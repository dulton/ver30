#pragma once

#include "event_processor.h"

class EventProcessCenter : public EventProcessor
{
public:
    EventProcessCenter(void);
    virtual ~EventProcessCenter(void);

    GMI_RESULT Initialize( const void_t *Parameter, size_t ParameterLength );
    GMI_RESULT Deinitialize();

    GMI_RESULT RegisterEventProcessor( ReferrencePtr<EventProcessor>& Processor, const void_t *Parameter, size_t ParameterLength );
    GMI_RESULT UnregisterEventProcessor( uint32_t ProcessorId );

    virtual GMI_RESULT Lock();
    virtual GMI_RESULT Unlock();
    virtual GMI_RESULT Notify( uint32_t EventId, enum EventType Type, void_t *Parameter, size_t ParameterLength );

private:
    GMI_Mutex                                     m_InstanceLock;
    GMI_Mutex                                     m_OperationLock;
    std::vector< ReferrencePtr<EventProcessor> >  m_EventProcessors;
};
