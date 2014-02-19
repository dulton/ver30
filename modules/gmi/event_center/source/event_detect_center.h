#pragma once

#include "event_detector.h"

class EventDetectCenter
{
public:
    EventDetectCenter(void);
    ~EventDetectCenter(void);

    GMI_RESULT Initialize( ReferrencePtr<EventProcessor, DefaultObjectDeleter, MultipleThreadModel> ProcessCenter, const void_t *Parameter, size_t ParameterLength );
    GMI_RESULT Deinitialize();

    GMI_RESULT RegisterEventDetector( ReferrencePtr<EventDetector>& Detector, const void_t *Parameter, size_t ParameterLength );
    GMI_RESULT UnregisterEventDetector( uint32_t DectectorId, uint32_t Index );

    GMI_RESULT Start();
    GMI_RESULT Stop();

private:
    ReferrencePtr<EventProcessor, DefaultObjectDeleter, MultipleThreadModel>  m_ProcessCenter;
    std::vector< ReferrencePtr<EventDetector> >                               m_EventDetectors;
};
