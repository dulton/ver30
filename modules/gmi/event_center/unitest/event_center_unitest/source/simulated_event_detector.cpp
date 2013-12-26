
#include "simulated_event_detector.h"

SimulatedEventDetector::SimulatedEventDetector(void)
    : EventDetector( e_EventDetectorType_Passive, 0 )
    , m_DetectThread()
    , m_ThreadWorking( false )
    , m_ThreadExitFlag( false )
{
}

SimulatedEventDetector::~SimulatedEventDetector(void)
{
}

GMI_RESULT SimulatedEventDetector::Start( const void_t *Parameter, size_t ParameterLength )
{
    GMI_RESULT Result = EventDetector::Start( Parameter, ParameterLength );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    m_ThreadWorking  = false;
    m_ThreadExitFlag = false;

    Result = m_DetectThread.Create( NULL, 0, EventDetectThread, this );
    if ( FAILED( Result ) )
    {
        EventDetector::Stop();
        return Result;
    }

    Result = m_DetectThread.Start();
    if ( FAILED( Result ) )
    {
        m_DetectThread.Destroy();
        EventDetector::Stop();
        return Result;
    }

    return Result;
}

GMI_RESULT SimulatedEventDetector::Stop()
{
    m_ThreadExitFlag = true;
    while ( m_ThreadWorking ) GMI_Sleep( 10 );
    m_DetectThread.Destroy();

    return EventDetector::Stop();
}

void_t* SimulatedEventDetector::EventDetectThread( void_t *Argument )
{
    SimulatedEventDetector *Detecter = reinterpret_cast<SimulatedEventDetector*> ( Argument );
    void_t *Return = Detecter->DetectEntry();
    return Return;
}

void_t* SimulatedEventDetector::DetectEntry()
{
    GMI_RESULT Result = GMI_FAIL;
    m_ThreadWorking   = true;
    while( !m_ThreadExitFlag )
    {
        m_ProcessCenter->Notify( SIMULATED_EVENT_ID, NULL, 0 );
        GMI_Sleep( 1000 );
    }
    m_ThreadWorking   = false;
    return (void_t *) size_t(Result);
}
