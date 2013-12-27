#include "alarm_output.h"

AlarmOutput::AlarmOutput( uint32_t EventProcessorId )
    : EventProcessor( EventProcessorId )
{
}

AlarmOutput::~AlarmOutput(void)
{
}

GMI_RESULT  AlarmOutput::SetName( const char_t *Name )
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

GMI_RESULT  AlarmOutput::AddScheduleTime( const ScheduleTimeInfo *Schedule )
{
    m_ScheduleTimes.push_back( *Schedule );
    return GMI_SUCCESS;
}

GMI_RESULT  AlarmOutput::ListScheduleTime( uint32_t *ItemNumber, ScheduleTimeInfo *Schedule )
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

GMI_RESULT  AlarmOutput::Notify( uint32_t EventId, enum EventType Type, void_t *Parameter, size_t ParameterLength )
{
    return GMI_SUCCESS;
}

GMI_RESULT  AlarmOutput::Start( const void_t *Parameter, size_t ParameterLength )
{
    const struct AlarmOutputInfo *Info = (const struct AlarmOutputInfo *) Parameter;
    SetOutputNumber( Info->s_OutputNumber );
    SetName( Info->s_Name );
    SetWorkMode( Info->s_WorkMode );
    SetDelayTime( Info->s_DelayTime );
    for ( uint32_t i = 0; i < Info->s_ScheduleTimeNumber; i+=2 )
    {
        AddScheduleTime( &(Info->s_ScheduleTime[i]) );
    }

    return GMI_SUCCESS;
}

GMI_RESULT  AlarmOutput::Stop()
{
    return GMI_SUCCESS;
}
