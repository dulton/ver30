#pragma once

#include "event_detector.h"

class EventDetectCenter
{
public:
    EventDetectCenter(void);
    ~EventDetectCenter(void);

    GMI_RESULT Initialize( ReferrencePtr<EventProcessCenter, DefaultObjectDeleter, MultipleThreadModel>& ProcessCenter, const void_t *Parameter, size_t ParameterLength );
    GMI_RESULT Deinitialize();

    GMI_RESULT RegisterEventDetector( SafePtr<EventDetector>& Detector );
    GMI_RESULT UnregisterEventDetector( uint32_t DectectorId );

    GMI_RESULT Start();
    GMI_RESULT Stop();

private:
    ReferrencePtr<EventProcessCenter, DefaultObjectDeleter, MultipleThreadModel>  m_ProcessCenter;
    std::vector< SafePtr<EventDetector> >                                         m_EventDetectors;
};
