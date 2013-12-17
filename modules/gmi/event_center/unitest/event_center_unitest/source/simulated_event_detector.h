#pragma once

#include "event_detector.h"

#define SIMULATED_EVENT_ID MAKE_DETECTOR_ID(EVENT_PRIORITY_MEDIUM,100)

class SimulatedEventDetector : public EventDetector
{
public:
    SimulatedEventDetector(void);
    virtual ~SimulatedEventDetector(void);

protected:
    virtual GMI_RESULT Start( const void_t *Parameter, size_t ParameterLength );
    virtual GMI_RESULT Stop();

private:
    static void_t* EventDetectThread( void_t *Argument );
    void_t* DetectEntry();

private:
    GMI_Thread  m_FetchThread;
    boolean_t   m_ThreadWorking;
    boolean_t   m_ThreadExitFlag;
};
