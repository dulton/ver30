

#ifndef __SDK_CLIENT_MONITOR_H__
#define __SDK_CLIENT_MONITOR_H__



/***************************
*
*  we wait monitor sys sta
***************************/

class SdkClientMonitor
{
public:
	SdkClientMonitor(SdkServerMgmt* pSvrMgmt,int* pRunningBits=NULL);
	~SdkClientMonitor();
	int MonitorSysStart();
	void MonitorSysEnd();

private:
	int m_InsertSysWait;
	ev_timer m_EvSysWaitTimer;
	int m_INsertSysTimeout;
	ev_timer m_EvSysTimeoutTimer;
	int 
};

#endif /*__SDK_CLIENT_MONITOR_H__*/


