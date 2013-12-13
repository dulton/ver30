
#include "event_process_center.h"

EventProcessCenter::EventProcessCenter(void)
    : EventProcessor( 0 )
    , m_InstanceLock()
    , m_OperationLock()
    , m_EventProcessors()
{
}

EventProcessCenter::~EventProcessCenter(void)
{
}

GMI_RESULT EventProcessCenter::Initialize( const void_t *Parameter, size_t ParameterLength )
{
    GMI_RESULT Result = m_InstanceLock.Create( NULL );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    Result = m_OperationLock.Create( NULL );
    if ( FAILED( Result ) )
    {
        m_InstanceLock.Destroy();
        return Result;
    }

    return Result;
}

GMI_RESULT EventProcessCenter::Deinitialize()
{
    GMI_RESULT Result = m_InstanceLock.Destroy();
    if ( FAILED( Result ) )
    {
        return Result;
    }

    Result = m_OperationLock.Destroy();
    if ( FAILED( Result ) )
    {
        return Result;
    }

    return Result;
}

GMI_RESULT EventProcessCenter::RegisterEventProcessor( SafePtr<EventProcessor>& Processor )
{
    std::vector< SafePtr<EventProcessor> >::iterator ProcessorIt = m_EventProcessors.begin(), ProcessorEnd = m_EventProcessors.end();
    for ( ; ProcessorIt != ProcessorEnd; ++ProcessorIt )
    {
        if ( Processor->GetId() == (*ProcessorIt)->GetId() )
        {
            return GMI_SUCCESS;
        }
    }

    Processor->Start( NULL, 0 );
    m_EventProcessors.push_back( Processor );
    return GMI_SUCCESS;
}

GMI_RESULT EventProcessCenter::UnregisterEventProcessor( uint32_t ProcessorId )
{
    std::vector< SafePtr<EventProcessor> >::iterator ProcessorIt = m_EventProcessors.begin(), ProcessorEnd = m_EventProcessors.end();
    for ( ; ProcessorIt != ProcessorEnd; ++ProcessorIt )
    {
        if ( ProcessorId == (*ProcessorIt)->GetId() )
        {
            (*ProcessorIt)->Stop();
            m_EventProcessors.erase( ProcessorIt );
            return GMI_SUCCESS;
        }
    }

    return GMI_INVALID_PARAMETER;
}

GMI_RESULT EventProcessCenter::Notify( uint32_t EventId, void_t *Parameter, size_t ParameterLength )
{
    GMI_RESULT Result = m_OperationLock.Lock( TIMEOUT_INFINITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    std::vector< SafePtr<EventProcessor> >::iterator ProcessorIt = m_EventProcessors.begin(), ProcessorEnd = m_EventProcessors.end();
    for ( ; ProcessorIt != ProcessorEnd; ++ProcessorIt )
    {
        (*ProcessorIt)->Notify( EventId, Parameter, ParameterLength );
    }

    m_OperationLock.Unlock();
    return Result;
}

GMI_RESULT EventProcessCenter::Lock()
{
    return m_InstanceLock.Lock( TIMEOUT_INFINITE );
}

GMI_RESULT EventProcessCenter::Unlock()
{
    return m_InstanceLock.Unlock();
}
