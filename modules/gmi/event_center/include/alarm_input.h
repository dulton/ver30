#pragma once

#include "event_detector.h"
#include "event_transaction_header.h"
#include "event_common_header.h"

class AlarmInput : public EventDetector
{
public:
    AlarmInput( enum EventDetectorType Type, uint32_t EventDetectorId, uint32_t Index );
    virtual ~AlarmInput(void);

    inline void_t  SetInputNumber( uint32_t Number )
    {
        m_InputNumber = Number;
    }
    inline uint32_t GetInputNumber() const
    {
        return m_InputNumber;
    }

    GMI_RESULT     SetName( const char_t *Name );
    inline const char_t*  GetName() const
    {
        return m_Name.GetPtr();
    }

    inline void_t   SetCheckTime( uint32_t Millisecond )
    {
        m_CheckTime = Millisecond;
    }
    inline uint32_t GetCheckTime() const
    {
        return m_CheckTime;
    }

    GMI_RESULT  AddScheduleTime( const ScheduleTimeInfo *Schedule );
    GMI_RESULT  ListScheduleTime( uint32_t *ItemNumber, ScheduleTimeInfo *Schedule );

protected:
    virtual GMI_RESULT Start( const void_t *Parameter, size_t ParameterLength );
    virtual GMI_RESULT Stop();

private:
    static void_t* AlarmInputDetectThread( void_t *Argument );
    void_t* DetectEntry();

private:
    uint32_t                                m_InputNumber;
    SafePtr<char_t, DefaultObjectsDeleter>  m_Name;
    uint32_t                                m_CheckTime;
    std::vector<ScheduleTimeInfo>           m_ScheduleTimes;

    GMI_Thread             m_DetectThread;
    boolean_t              m_ThreadWorking;
    boolean_t              m_ThreadExitFlag;
    enum AlarmInputStatus  m_GPIOInputStatus;
};
