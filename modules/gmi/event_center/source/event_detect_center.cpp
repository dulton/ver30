
#include "event_detect_center.h"

EventDetectCenter::EventDetectCenter(void)
    : m_ProcessCenter()
    , m_EventDetectors()
{
}

EventDetectCenter::~EventDetectCenter(void)
{
}

GMI_RESULT EventDetectCenter::Initialize( ReferrencePtr<EventProcessor, DefaultObjectDeleter, MultipleThreadModel> ProcessCenter, const void_t *Parameter, size_t ParameterLength )
{
    m_ProcessCenter = ProcessCenter;
    return GMI_SUCCESS;
}

GMI_RESULT EventDetectCenter::Deinitialize()
{
    return GMI_SUCCESS;
}

GMI_RESULT EventDetectCenter::RegisterEventDetector( ReferrencePtr<EventDetector>& Detector, const void_t *Parameter, size_t ParameterLength )
{
    std::vector< ReferrencePtr<EventDetector> >::iterator DetectorIt = m_EventDetectors.begin(), DetectorEnd = m_EventDetectors.end();
    for ( ; DetectorIt != DetectorEnd; ++DetectorIt )
    {
        if ( Detector->GetId() == (*DetectorIt)->GetId() )
        {
            return GMI_SUCCESS;
        }
    }

    GMI_RESULT Result = GMI_SUCCESS;
    if ( e_EventDetectorType_Passive == Detector->GetType() )
    {
        Detector->SetEventProcessor( m_ProcessCenter );
        Result = Detector->Start( Parameter, ParameterLength );
    }
    m_EventDetectors.push_back( Detector );
    return Result;
}

GMI_RESULT EventDetectCenter::UnregisterEventDetector( uint32_t DectectorId )
{
    std::vector< ReferrencePtr<EventDetector> >::iterator DetectorIt = m_EventDetectors.begin(), DetectorEnd = m_EventDetectors.end();
    for ( ; DetectorIt != DetectorEnd; ++DetectorIt )
    {
        if ( DectectorId == (*DetectorIt)->GetId() )
        {
            GMI_RESULT Result = GMI_SUCCESS;
            if ( e_EventDetectorType_Passive == (*DetectorIt)->GetType() )
            {
                Result = (*DetectorIt)->Stop();
            }
            m_EventDetectors.erase( DetectorIt );
            return Result;
        }
    }

    return GMI_INVALID_PARAMETER;
}

GMI_RESULT EventDetectCenter::Start()
{
    return GMI_SUCCESS;
}

GMI_RESULT EventDetectCenter::Stop()
{
    return GMI_SUCCESS;
}
