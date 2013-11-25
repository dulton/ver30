
#ifndef  __SDK_CLIENT_DAEMON_H__
#define  __SDK_CLIENT_DAEMON_H__

#include <sdk_client_comm.h>
#include <libev/ev.h>
#include <sdk_client_sock.h>
typedef enum
{
	cd_logout_state = 0,
	cd_login_state = 1,
} cd_enum_state_t;

class SdkServerMgmt;

class SdkClientDaemon
{
public:
	SdkClientDaemon(SdkServerMgmt* pSvrMgmt,int timeout=5,int *pRunningBits=NULL);
	~SdkClientDaemon();
	int Start();
	void Stop();
	int ResetLongTimeTimer();
private:
	int __StartReadIo();
	void __StopReadIo();
	int __StartReadTimer();
	void __StopReadTimer();
	int __StartWriteIo();
	void __StopWriteIo();
	int __StartWriteTimer();
	void __StopWriteTimer();
	int __ResetWriteTimer();
	int __StartSendTimer();
	void __StopSendTimer();
	int __ResetSendTimer();
	int __StartRegisterTimer();
	void __StopRegisterTimer();
	
	void __ClearClientComms();

	int __BindSocket();
    int __SetSocketUnBlock(int sock);
	int __GetLPort();
	int __GetRPort();
	int __IncReqNum();
	sdk_client_comm_t* __GetRegisterComm();
	sdk_client_comm_t* __GetReportComm();
	

	static void ReadTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);
	static void WriteTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);
	static void RegisterTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);
	static void ReadIoCallBack(EV_P_ ev_io *w, int revents,void* arg);
	int __ReadIoImpl();
	
	static void WriteIoCallBack(EV_P_ ev_io *w, int revents,void* arg);
	int __WriteIoImpl();
	static void SendTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);
	int __SendTimerImpl();
	void __BreakOut();

	cd_enum_state_t __GetState();
	cd_enum_state_t __SetState(cd_enum_state_t state);
	seqid_t __FindSeqId(sdk_client_comm_t* pComm);


private:
	SdkServerMgmt *m_pSvrMgmt;
	int m_SendTimeout;
	int *m_pRunningBits;
	int m_ReqNum;
	cd_enum_state_t m_State;
	int m_InsertWriteIo;
	ev_io m_EvWriteIo;
	int m_InsertWriteTimer;
	ev_timer m_EvWriteTimer;	
	int m_InsertReadIo;
	ev_io m_EvReadIo;
	int m_InsertReadTimer;
	ev_timer m_EvReadTimer;
	int m_InsertSendTimer;
	ev_timer m_EvSendTimer;
	int m_InsertRegisterTimer;
	ev_timer m_EvRegisterTimer;
	int m_RPort;
	int m_LPort;
	SdkClientSock *m_pSock;
	
	std::vector<sdk_client_comm_t*> m_pSendingComms;
	std::vector<seqid_t> m_SendSeqIds;
};


#endif /*__SDK_CLIENT_DAEMON_H__*/


