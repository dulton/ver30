
#include "simple_event_processor.h"

#include "simulated_event_detector.h"

SimpleEventProcessor::SimpleEventProcessor(void)
    : EventProcessor( 0 )
{
}

SimpleEventProcessor::~SimpleEventProcessor(void)
{
}

GMI_RESULT SimpleEventProcessor::Notify( uint32_t EventId, enum EventType Type, void_t *Parameter, size_t ParameterLength )
{
    std::vector<uint32_t>::iterator DetectorIdIt = m_DetectorIds.begin(), DetectorIdEnd = m_DetectorIds.end();
    for ( ; DetectorIdIt != DetectorIdEnd ; ++DetectorIdIt )
    {
        if ( *DetectorIdIt == EventId )
        {
            if ( NULL != m_Callback )
            {
                m_Callback( m_UserData, EventId, Type, Parameter, ParameterLength );
            }
        }
    }

    return GMI_SUCCESS;
}
