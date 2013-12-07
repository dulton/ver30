#ifndef __SDK_SERVER_ALARM_H__
#define __SDK_SERVER_ALARM_H__

#include <sdk_client_comm.h>
#include <sdk_client_sock.h>
#include <libev/ev.h>

class SdkServerMgmt;

class SdkServerAlarm
{
public:
	SdkServerAlarm(SdkServerMgmt* pSvrMgmt);
	~SdkServerAlarm();
	int Start();
	void Stop();
	int ResetLongTimeTimer();
private:
	int __WriteIoImpl();
    static void WriteIoCallBack(EV_P_ ev_io *w, int revents,void* arg);
	int __ReadIoImpl();
    static void ReadIoCallBack(EV_P_ ev_io *w, int revents,void* arg);
	static void ReadTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);
	static void WriteTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);
	int __StartReadIo();
	void __StopReadIo();
	int __StartReadTimer();
	void __StopReadTimer();
	int __StartWriteIo();
	void __StopWriteIo();
	int __StartWriteTimer();
	void __StopWriteTimer();
	void __ClearComms();
	int __BindSocket();
	int __SetSocketUnBlock(int sock);
	void __BreakOut();
	int __GetRPort();
	int __GetLPort();
	int __PrepareReqAlarm(sdk_client_comm_t* pComm);
private:
	SdkServerMgmt* m_pSvrMgmt;
	SdkClientSock* m_pSock;	
	int m_LPort;
	int m_RPort;
	std::vector<sdk_client_comm_t*> m_pReqCommVecs;
	int m_InsertReadIo;
	ev_io m_EvReadIo;
	int m_InsertReadTimer;
	ev_timer m_EvReadTimer;
	int m_InsertWriteIo;
	ev_io m_EvWriteIo;
	int m_InsertWriteTimer;
	ev_timer m_EvWriteTimer;
};


#endif /*__SDK_SERVER_ALARM_H__*/
