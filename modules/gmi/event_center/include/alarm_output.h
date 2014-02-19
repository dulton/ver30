#pragma once

#include "event_processor.h"
#include "event_transaction_header.h"

class AlarmOutput : public EventProcessor
{
public:
    AlarmOutput( uint32_t EventProcessorId, uint32_t Index );
    virtual ~AlarmOutput(void);

    inline void_t  SetOutputNumber( uint32_t Number )
    {
        m_OutputNumber = Number;
    }
    inline uint32_t GetOutputNumber() const
    {
        return m_OutputNumber;
    }

    GMI_RESULT     SetName( const char_t *Name );
    inline const char_t*  GetName() const
    {
        return m_Name.GetPtr();
    }

    inline void_t SetWorkMode( enum AlarmOutputWorkMode Mode )
    {
        m_WorkMode  = Mode;
    }
    inline enum AlarmOutputWorkMode GetWorkMode() const
    {
        return m_WorkMode;
    }

    inline void_t SetDelayTime( uint32_t Time )
    {
        m_DelayTime = Time;
    }
    inline uint32_t GetDelayTime() const
    {
        return m_DelayTime;
    }

    GMI_RESULT  AddScheduleTime( const ScheduleTimeInfo *Schedule );
    GMI_RESULT  ListScheduleTime( uint32_t *ItemNumber, ScheduleTimeInfo *Schedule );

    virtual GMI_RESULT Notify( uint32_t EventId, uint32_t Index, enum EventType Type, void_t *Parameter, size_t ParameterLength );

protected:
    virtual GMI_RESULT Start( const void_t *Parameter, size_t ParameterLength );
    virtual GMI_RESULT Stop();

private:
    uint32_t                                m_OutputNumber;
    SafePtr<char_t, DefaultObjectsDeleter>  m_Name;
    enum AlarmOutputWorkMode                m_WorkMode;
    uint32_t                                m_DelayTime;
    std::vector<ScheduleTimeInfo>           m_ScheduleTimes;
};
