#ifndef __ALARM_H__
#define __ALARM_H__
#include "sys_alarm_info_report.h"
#include "event_transaction_center.h"
#include "event_transaction_header.h"
#include "gmi_system_headers.h"
#include "sys_env_types.h"

class Alarm
{
public:
	Alarm();
	~Alarm();
	GMI_RESULT Initialize();
    GMI_RESULT Deinitialize(); 
    GMI_RESULT Config(int32_t AlarmId, int32_t Index, const void_t *Parameter, size_t ParameterLength);
    GMI_RESULT Schedule(int32_t ScheduleId, int32_t Index, const void_t *Parameter, size_t ParameterLength);
private:	
   	static void EventProcessCallBack(void_t *UserData, uint32_t EventId, enum EventType Type, void_t *Parameter, size_t ParameterLength);
   	void_t EventProcess(uint32_t EventId, enum EventType Type, void_t *Parameter, size_t ParameterLength);
	GMI_RESULT CheckConfig(int32_t AlarmId, int32_t Index, const void_t *Parameter, size_t ParameterLength);
	GMI_RESULT CheckSchedule(int32_t ScheduleId, int32_t Index, const void_t *Parameter, size_t ParameterLength);
private:
	ReferrencePtr<SysAlarmInfoReport>    m_AlarmInfoReport;
	EventTransactionCenter         m_EventCenter;		
};


#endif

