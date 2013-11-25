
#ifndef __SDK_SERVER_MAIN_H__
#define __SDK_SERVER_MAIN_H__

#include <sdk_server_mgmt.h>


class SdkServerMain
{
public:
	SdkServerMain(int port,int*pRunningBits=NULL,int maxclients=16);
	~SdkServerMain();
	int RunLoop();

private:
	int __Start();
	void __Stop();
	static void FreqTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);
	int __FreqTimerImpl();

	static void IoAccCallBack(EV_P_ ev_io *w, int revents,void* arg);
	int __IoAccImpl();
	
	void __StopFreqTimer();
	int __StartFreqTimer();
	int __ResetFreqTimer();

	void __StopIoAcc();
	int __StartIoAcc();
	int __BindPort();
	void __BreakRunLoop();
	int __SetSocketUnBlock(int sock);
	int __CanInsertSock(void);
	int __SetKeepAlive(int sock);
	int __TryBindAndStartAcc(void);
private:
	int m_Port;
	int *m_pRunningBits;
	unsigned int m_MaxClients;
	int m_TryBindWaits;
	int m_AccSock;
	ev_io m_EvIoAcc;
 	int m_HasInsertIoAcc;
 	ev_timer m_EvFreqTimer;
 	int m_HasInsertFreqTimer;
 	SdkServerMgmt* m_pServerMgmt;
};


#endif  /*__SDK_SERVER_MAIN_H__*/


