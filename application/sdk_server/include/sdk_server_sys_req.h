
#ifndef __SDK_SERVER_SYS_REQ_H__
#define __SDK_SERVER_SYS_REQ_H__

#include <sdk_client_sock.h>
#include <libev/ev.h>

class SdkServerMgmt;

class SdkServerSysReq
{
public:
    SdkServerSysReq(SdkServerMgmt* pSvrMgmt,int* pRunningBits=NULL);
    ~SdkServerSysReq();
    int Start();
    void Stop();
    int AddRequest(sdk_client_comm_t*& pComm);
	int ResetLongTimeTimer(void);

private:
    void __StopReadIo();
    int __StartReadIo();
    void __StopReadTimer();
    int __StartReadTimer();
    void __StopWriteIo();
    int __StartWriteIo();
    void __StopWriteTimer();
    int __StartWriteTimer();
	int __ResetWriteTimer();
	void __BreakOut();

    static void ReadTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);

	static void WriteTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);

	static void ReadIoCallBack(EV_P_ ev_io *w, int revents,void* arg);
	int __ReadIoImpl();
	
	static void WriteIoCallBack(EV_P_ ev_io *w, int revents,void* arg);
	int __WriteIoImpl();

    int __GetLPort();
    int __GetRPort();
    int __SetSocketUnBlock(int sock);
    int __BindPort();

    void __FreeSendComms();
	int __SplitFragPackets(sdk_client_comm_t*& pComm);

private:
    SdkServerMgmt* m_pSvrMgmt;
    int *m_pRunningBits;
    SdkClientSock* m_pSock;
    int m_LPort;
    int m_RPort;
    int m_InsertReadIo;
    ev_io m_EvReadIo;
    int m_InsertReadTimer;
    ev_timer m_EvReadTimer;
    int m_InsertWriteIo;
    ev_io m_EvWriteIo;
    int m_InsertWriteTimer;
	ev_timer m_EvWriteTimer;
    std::vector<sdk_client_comm_t*> m_pSendComm;

};


#endif /*__SDK_SERVER_SYS_REQ_H__*/


