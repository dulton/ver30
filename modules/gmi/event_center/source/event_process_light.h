#pragma once

#include "event_processor.h"
#include "event_transaction_header.h"
#include "event_common_header.h"

class EventProcessLight : public EventProcessor
{
public:
    EventProcessLight( uint32_t EventProcessorId, uint32_t Index );
    virtual ~EventProcessLight(void);

    inline void_t SetDelayTime( uint32_t Time )
    {
        m_DelayTime = Time;
    }
    inline uint32_t GetDelayTime() const
    {
        return m_DelayTime;
    }

	inline void_t SetTriggedTime( uint32_t Time)
    {
        m_TriggedTime = Time;
    }
    inline uint32_t GetTriggedTime() const
    {
        	return m_TriggedTime;
    }
	
    virtual GMI_RESULT Notify( uint32_t EventId, uint32_t Index, enum EventType Type, void_t *Parameter, size_t ParameterLength );

protected:
    virtual GMI_RESULT Start( const void_t *Parameter, size_t ParameterLength );
    virtual GMI_RESULT Stop();

private:
	static void_t* TimerThread( void_t *Argument );
	void_t* TimerEntry();


private:
    uint32_t                                m_DelayTime;
	GMI_Thread             m_TimerThread;
    boolean_t              m_ThreadWorking;
    boolean_t              m_ThreadExitFlag;
	uint32_t               m_TriggedTime;
	GMI_Mutex              m_OperationLock;
};

