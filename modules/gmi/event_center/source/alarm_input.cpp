#include "alarm_input.h"

AlarmInput::AlarmInput( enum EventDetectorType Type, uint32_t EventDetectorId )
    : EventDetector( Type, EventDetectorId )
{
}

AlarmInput::~AlarmInput(void)
{
}

GMI_RESULT  AlarmInput::SetName( const char_t *Name )
{
    if ( NULL == Name )
    {
        m_Name = NULL;
    }
    else
    {
        size_t NameLength = strlen( Name );
        m_Name = BaseMemoryManager::Instance().News<char_t>(NameLength+1);
        if ( NULL == m_Name.GetPtr() )
        {
            return GMI_OUT_OF_MEMORY;
        }
        if ( 0 < NameLength )
        {
            memcpy( m_Name.GetPtr(), Name, NameLength );
        }
        (m_Name.GetPtr())[NameLength] = 0;
    }

    return GMI_SUCCESS;
}

GMI_RESULT  AlarmInput::AddScheduleTime( const ScheduleTimeInfo *Schedule )
{
    m_ScheduleTimes.push_back( *Schedule );
    return GMI_SUCCESS;
}

GMI_RESULT  AlarmInput::ListScheduleTime( uint32_t *ItemNumber, ScheduleTimeInfo *Schedule )
{
    if ( *ItemNumber < m_ScheduleTimes.size() )
    {
        return GMI_NOT_ENOUGH_SPACE;
    }

    *ItemNumber = (uint32_t) m_ScheduleTimes.size();
    if ( 0 < *ItemNumber )
    {
        for ( uint32_t i = 0; i < *ItemNumber; ++i )
        {
            *Schedule = m_ScheduleTimes[i];
        }
    }
    return GMI_SUCCESS;
}

GMI_RESULT  AlarmInput::Start( const void_t *Parameter, size_t ParameterLength )
{
    GMI_RESULT Result = EventDetector::Start( Parameter, ParameterLength );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    m_ThreadWorking  = false;
    m_ThreadExitFlag = false;

    Result = m_DetectThread.Create( NULL, 0, AlarmInputDetectThread, this );
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

    const struct AlarmInputInfo *Info = (const struct AlarmInputInfo *) Parameter;
    SetInputNumber( Info->s_InputNumber );
    SetName( Info->s_Name );
    SetTriggerType( (enum AlarmInputTriggerType) Info->s_TriggerType );
    for ( uint32_t i = 0; i < Info->s_ScheduleTimeNumber; i+=2 )
    {
        AddScheduleTime( &(Info->s_ScheduleTime[i]) );
    }

    return Result;
}

GMI_RESULT  AlarmInput::Stop()
{
    m_ThreadExitFlag = true;
    while ( m_ThreadWorking ) GMI_Sleep( 10 );
    m_DetectThread.Destroy();

    return EventDetector::Stop();
}

void_t* AlarmInput::AlarmInputDetectThread( void_t *Argument )
{
    AlarmInput *Detecter = reinterpret_cast<AlarmInput*> ( Argument );
    void_t *Return = Detecter->DetectEntry();
    return Return;
}

void_t* AlarmInput::DetectEntry()
{
    GMI_RESULT Result = GMI_FAIL;
    m_ThreadWorking   = true;
    while( !m_ThreadExitFlag )
    {
#if defined( __linux__ )
#elif defined( _WIN32 ) // only test used
        m_ProcessCenter->Notify( GetId(), e_EventType_Start, NULL, 0 );
        GMI_Sleep( 1000 );
        m_ProcessCenter->Notify( GetId(), e_EventType_End, NULL, 0 );
#endif
    }
    m_ThreadWorking   = false;
    return (void_t *) size_t(Result);
}
