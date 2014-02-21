#ifndef __ALARM_H__
#define __ALARM_H__
#include "alarm_session.h"
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
private:
	GMI_RESULT Report(SysPkgAlarmInfor *SysAlarmInfor);
	static void EventProcess(void_t *UserData, uint32_t EventId, enum EventType Type, void_t *Parameter, size_t ParameterLength);
private:
	ReferrencePtr<AlarmSession>    m_AlarmSession;
	EventTransactionCenter         m_EventCenter;
};


#endif
