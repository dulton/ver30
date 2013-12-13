#pragma once

#include "event_processor.h"

//typedef void_t (*EventNotifyCallback)( void_t *UserData, uint32_t EventId, void_t *Parameter, size_t ParamLength );

class EventProcessCenter : public EventProcessor
{
public:
    EventProcessCenter(void);
    virtual ~EventProcessCenter(void);

    GMI_RESULT Initialize( const void_t *Parameter, size_t ParameterLength );
    GMI_RESULT Deinitialize();
    GMI_RESULT RegisterEventProcessor( SafePtr<EventProcessor>& Processor );
    GMI_RESULT UnregisterEventProcessor( uint32_t ProcessorId );
    GMI_RESULT Lock();
    GMI_RESULT Unlock();

    virtual GMI_RESULT Notify( uint32_t EventId, void_t *Parameter, size_t ParameterLength );

private:
    GMI_Mutex                               m_InstanceLock;
    GMI_Mutex                               m_OperationLock;
    std::vector< SafePtr<EventProcessor> >  m_EventProcessors;
};
