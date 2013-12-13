
#include "event_detect_center.h"

EventDetectCenter::EventDetectCenter(void)
    : m_ProcessCenter()
    , m_EventDetectors()
{
}

EventDetectCenter::~EventDetectCenter(void)
{
}

GMI_RESULT EventDetectCenter::Initialize( ReferrencePtr<EventProcessCenter, DefaultObjectDeleter, MultipleThreadModel>& ProcessCenter, const void_t *Parameter, size_t ParameterLength )
{
    m_ProcessCenter = ProcessCenter;
    return GMI_SUCCESS;
}

GMI_RESULT EventDetectCenter::Deinitialize()
{
    return GMI_SUCCESS;
}

GMI_RESULT EventDetectCenter::RegisterEventDetector( SafePtr<EventDetector>& Detector )
{
    std::vector< SafePtr<EventDetector> >::iterator DetectorIt = m_EventDetectors.begin(), DetectorEnd = m_EventDetectors.end();
    for ( ; DetectorIt != DetectorEnd; ++DetectorIt )
    {
        if ( Detector->GetId() == (*DetectorIt)->GetId() )
        {
            return GMI_SUCCESS;
        }
    }

    if ( e_EventDetectorType_Passive == Detector->GetType() )
    {
        Detector->Start( m_ProcessCenter, NULL, 0 );
    }
    m_EventDetectors.push_back( Detector );
    return GMI_SUCCESS;
}

GMI_RESULT EventDetectCenter::UnregisterEventDetector( uint32_t DectectorId )
{
    std::vector< SafePtr<EventDetector> >::iterator DetectorIt = m_EventDetectors.begin(), DetectorEnd = m_EventDetectors.end();
    for ( ; DetectorIt != DetectorEnd; ++DetectorIt )
    {
        if ( DectectorId == (*DetectorIt)->GetId() )
        {
            if ( e_EventDetectorType_Passive == (*DetectorIt)->GetType() )
            {
                (*DetectorIt)->Stop();
            }
            m_EventDetectors.erase( DetectorIt );
            return GMI_SUCCESS;
        }
    }

    return GMI_INVALID_PARAMETER;
}

GMI_RESULT EventDetectCenter::Start()
{
    return GMI_NOT_IMPLEMENT;
}

GMI_RESULT EventDetectCenter::Stop()
{
    return GMI_NOT_IMPLEMENT;
}
