

#ifndef  __SDK_SERVER_SYS_COMM_H__
#define  __SDK_SERVER_SYS_COMM_H__

#include <sys_stream_info.h>
#include <libev/ev.h>
#include <sdk_client_comm.h>
#include <sdk_client_sock.h>



class SdkServerMgmt;

class SdkServerSysComm
{
public:
    SdkServerSysComm(SdkServerMgmt*pSvrMgmt,int *pRunningBits=NULL);
    ~SdkServerSysComm();
    int Start();
    void Stop();
	int ResetLongTimeTimer();

private:
    int __BindSocket();
    int __GetRPort();
    int __GetLPort();
    int __SetSocketUnBlock(int sock);

    void __StopWriteIo();
    int __StartWriteIo();
    void __StopWriteTimer();
    int __StartWriteTimer();
    void __StopReadIo();
    int __StartReadIo();
    void __StopReadTimer();
    int __StartReadTimer();

    int __HandleStartAllStream(uint32_t opcode,sys_stream_request_t* pRequest,int datalen);
    int __HandleStopAllStream(uint32_t opcode,sys_stream_request_t* pRequest,int datalen);
    int __HandlePauseAllStream(uint32_t opcode,sys_stream_request_t* pRequest,int datalen);
    int __HandleResumeAllStream(uint32_t opcode,sys_stream_request_t* pRequest,int datalen);
    int __HandleStreamRequest(sdk_client_comm_t*& pComm);
    /*success ,return value is the state ,otherwise ,negative error code*/
    int __HandleQueryStreamState(uint32_t opcode,sys_stream_request_t* pRequest,int datalen);

    int __ResponseCode(uint32_t opcode,int err,sdk_client_comm_t* pReqComm);
    int __InsertRequestComm(sdk_client_comm_t*& pComm);
    int __ProtoToHostRequest(sys_stream_request_t* pRequest,int datalen);

    static void ReadTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);

    static void WriteTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);

    static void ReadIoCallBack(EV_P_ ev_io *w, int revents,void* arg);
    int __ReadIoImpl();

    static void WriteIoCallBack(EV_P_ ev_io *w, int revents,void* arg);
    int __WriteIoImpl();

    void __BreakOut();


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
    std::vector<sdk_client_comm_t*> m_pResponse;
};


#endif  /*__SDK_SERVER_SYS_COMM_H__*/



