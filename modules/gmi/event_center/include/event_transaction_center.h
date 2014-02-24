#pragma once

#include "gmi_system_headers.h"
#include "event_center.h"
#include "event_process_header.h"
#include "event_transaction_header.h"

class EventTransactionCenter
{
public:
    EventTransactionCenter();
    ~EventTransactionCenter(void);

    GMI_RESULT Start( const void_t *Parameter, size_t ParameterLength, EventCallback Callback, void_t *UserData );
    GMI_RESULT Stop();

    GMI_RESULT ConfigureGPIOAlarmInput( const void_t *Parameter, size_t ParameterLength );
    GMI_RESULT ConfigureGPIOAlarmOutput( const void_t *Parameter, size_t ParameterLength );
	GMI_RESULT ConfigureAlarmEvent(const enum AlarmEventType EventType, const void *Parameter, size_t ParamterLength);
	GMI_RESULT ConfigureAlarmScheduleTime(size_t ScheduleId, const void_t *Parameter, size_t Parameterlength);

private:
	GMI_RESULT StartGPIOAlarmInputEx(const void *Parameter, size_t ParameterLength);
    GMI_RESULT StopGPIOAlarmInputEx(size_t InIoIndex);
    GMI_RESULT StartGPIOAlarmOutputEx(const void *Parameter, size_t ParameterLength);
    GMI_RESULT StopGPIOAlarmOutputEx(size_t OutIoIndex);
	GMI_RESULT StartAlarmInfoRecord();
    GMI_RESULT StopAlarmInfoRecord(); 
	GMI_RESULT StartHumanDetect();
    GMI_RESULT StopHumanDetect();

private:
    ReferrencePtr<EventCenter>  m_Center;
    EventCallback               m_Callback;
    void_t                      *m_UserData;
	static size_t               m_IsStartHumanDetect;
	static size_t               m_IsStartInfoRecord;
	static size_t               m_IsStartGPIOInputEx[MAX_NUM_GPIO_IN];
	static size_t               m_IsStartGPIOOutputEx[MAX_NUM_GPIO_OUT];
	
};
