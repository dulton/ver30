#pragma once

#include "event_processor.h"
#include "event_transaction_header.h"
#include "event_common_header.h"

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

	#if 0
    inline void_t SetWorkMode( enum AlarmOutputWorkMode Mode )
    {
        m_WorkMode  = Mode;
    }
    inline enum AlarmOutputWorkMode GetWorkMode() const
    {
        return m_WorkMode;
    }
	#endif

    inline void_t SetDelayTime( uint32_t Time )
    {
        m_DelayTime = Time;
    }
    inline uint32_t GetDelayTime() const
    {
        return m_DelayTime;
    }

	inline void_t SetTriggedTime( uint32_t Time, uint32_t EventType )
    {
    	if((EventType > 0) && (EventType <= MAX_NUM_EVENT_TYPE))
    	{
        	m_TriggedTime[EventType-1] = Time;
    	}
    }
    inline uint32_t GetTriggedTime(uint32_t EventType) const
    {
    	if((EventType > 0) && (EventType <= MAX_NUM_EVENT_TYPE))
    	{
        	return m_TriggedTime[EventType-1];
    	}
		else
		{
			return 0;
		}
    }

    GMI_RESULT  AddScheduleTime( const ScheduleTimeInfo *Schedule );
    GMI_RESULT  ListScheduleTime( uint32_t *ItemNumber, ScheduleTimeInfo *Schedule );

    virtual GMI_RESULT Notify( uint32_t EventId, uint32_t Index, enum EventType Type, void_t *Parameter, size_t ParameterLength );

protected:
    virtual GMI_RESULT Start( const void_t *Parameter, size_t ParameterLength );
    virtual GMI_RESULT Stop();

private:
	static void_t* TimerThread( void_t *Argument );
	void_t* TimerEntry();


private:
    uint32_t                                m_OutputNumber;
    SafePtr<char_t, DefaultObjectsDeleter>  m_Name;
    //enum AlarmOutputWorkMode                m_WorkMode;
    uint32_t                                m_DelayTime;
    std::vector<ScheduleTimeInfo>           m_ScheduleTimes;

	GMI_Thread             m_TimerThread;
    boolean_t              m_ThreadWorking;
    boolean_t              m_ThreadExitFlag;
	uint32_t               m_TriggedTime[MAX_NUM_EVENT_TYPE];
	GMI_Mutex              m_OperationLock;
};
