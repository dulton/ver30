#pragma once

#include "event_processor.h"
#include "event_transaction_header.h"

class AlarmOutput : public EventProcessor
{
public:
    AlarmOutput( uint32_t EventProcessorId );
    virtual ~AlarmOutput(void);

    void_t  SetOutputNumber( uint32_t Number )
    {
        m_OutputNumber = Number;
    }
    uint32_t GetOutputNumber() const
    {
        return m_OutputNumber;
    }

    GMI_RESULT     SetName( const char_t *Name );
    const char_t*  GetName() const
    {
        return m_Name.GetPtr();
    }

    void_t SetWorkMode( uint32_t Mode )
    {
        m_WorkMode  = Mode;
    }
    uint32_t GetWorkMode() const
    {
        return m_WorkMode;
    }

    void_t SetDelayTime( uint32_t Time )
    {
        m_DelayTime = Time;
    }
    uint32_t GetDelayTime() const
    {
        return m_DelayTime;
    }

    GMI_RESULT  AddScheduleTime( const ScheduleTimeInfo *Schedule );
    GMI_RESULT  ListScheduleTime( uint32_t *ItemNumber, ScheduleTimeInfo *Schedule );

    virtual GMI_RESULT Notify( uint32_t EventId, enum EventType Type, void_t *Parameter, size_t ParameterLength );

protected:
    virtual GMI_RESULT Start( const void_t *Parameter, size_t ParameterLength );
    virtual GMI_RESULT Stop();

private:
    uint32_t                                m_OutputNumber;
    SafePtr<char_t, DefaultObjectsDeleter>  m_Name;
    uint32_t                                m_WorkMode;
    uint32_t                                m_DelayTime;
    std::vector<ScheduleTimeInfo>           m_ScheduleTimes;
};
