#pragma once

#include "event_detector.h"
#include "event_transaction_header.h"

struct HumanDetectInfo
{
	uint32_t          s_CheckTime;
};


class HumanDetect : public EventDetector
{
public:
    HumanDetect( enum EventDetectorType Type, uint32_t EventDetectorId, uint32_t Index );
    virtual ~HumanDetect(void);

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
    static void_t* HumanDetectThread( void_t *Argument );
    void_t* DetectEntry();

private:
    uint32_t                                m_CheckTime;
    std::vector<ScheduleTimeInfo>           m_ScheduleTimes;

    GMI_Thread             m_DetectThread;
    boolean_t              m_ThreadWorking;
    boolean_t              m_ThreadExitFlag;
};

