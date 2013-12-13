#pragma once

#include "gmi_system_headers.h"

class EventDetector;
class EventDetectCenter;
class EventProcessor;
class EventProcessCenter;

class EventCenter
{
public:
    EventCenter(void);
    ~EventCenter(void);

    GMI_RESULT Initialize( const void_t *Parameter, size_t ParameterLength );
    GMI_RESULT Deinitialize();

    GMI_RESULT RegisterEventDetector( SafePtr<EventDetector>& Detector );
    GMI_RESULT UnregisterEventDetector( uint32_t DectectorId );

    GMI_RESULT RegisterEventProcessor( SafePtr<EventProcessor>& Processor );
    GMI_RESULT UnregisterEventProcessor( uint32_t ProcessorId );

    GMI_RESULT Start();
    GMI_RESULT Stop();

private:
    SafePtr<EventDetectCenter>                                                    m_DetectCenter;
    ReferrencePtr<EventProcessCenter, DefaultObjectDeleter, MultipleThreadModel>  m_ProcessCenter;
};
