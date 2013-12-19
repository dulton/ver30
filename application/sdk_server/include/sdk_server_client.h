
#ifndef __SDK_SERVER_CLIENT_H__
#define __SDK_SERVER_CLIENT_H__

#include <sys_stream_info.h>
#include <sdk_server_buffer.h>

typedef enum
{
    sdk_client_close_state     = 0,
    sdk_client_login_state     = 1,
    sdk_client_login2_state    = 2,
    sdk_client_login_succ_state= 3,
    sdk_client_command_state   = 4,
    sdk_client_message_state   = 5,
    sdk_client_stream_state    = 6,
    /*this means that we should do the audio dual communication*/
    sdk_client_stream_audio_dual_start_state = 7,
    sdk_client_stream_audio_dual_state  = 8,
    /*
    	this state means that we should do the audio communication pending ,
    	because the notify communication call us,discard all the income audio*/
    sdk_client_stream_audio_dual_pending_state=9,
    /*this state ,is when change config ,we should restart the decode audio data server */
    sdk_client_stream_audio_dual_pending_start_state=10,
    sdk_client_upgrade_state   = 11,
    sdk_client_logout_state    = 13,
} sdk_client_state_t ;




#define   LOGIN_REQUEST      0x1
#define   LOGIN_RESPONSE     0x2

#define   LOGIN_AUTH_NONE    0x1
#define   LOGIN_AUTH_MD5     0x2
#define   LOGIN_AUTH_CONSULT 0x3
#define   LOGIN_AUTH_DES     0x4

#define   LOGIN_RESP_SUCC             0x0
#define   LOGIN_RESP_UNATHORIZED      0x1
#define   LOGIN_RESP_AUTH_FAILED      0x2

#define   LOGIN_ENCDATA_LEN  64

#define   SDK_NOTIFY_SEQID   0



typedef struct
{
    uint32_t m_ReqId;
    uint16_t m_SesId;
    uint16_t m_EncType;
    uint8_t  m_UserName[64];
    uint8_t  m_Password[LOGIN_ENCDATA_LEN];
    uint32_t m_ExpireTime;
    uint32_t m_HeartBeatTime;
} login_request_t;

typedef struct
{
    uint32_t m_RespId;
    uint32_t m_Result;
    uint16_t m_Sesid;
    uint16_t m_EncType;
	uint32_t m_KeepTimeMS;
    uint8_t  m_EncData[LOGIN_ENCDATA_LEN];
} login_response_t;









class SdkServerMgmt;

class SdkServerClient
{
public:
    SdkServerClient(int sock,SdkServerMgmt* pServerMgmt,int *pRunningBits=NULL);
    ~SdkServerClient();
    int CloseSocket();
    int GetSessionId(sessionid_t& sesid,privledge_t& priv);
    int Start();
    void Stop();
    int ResumeStream(sys_stream_info_t * pStreamInfo);
    int PauseStream();
	int GetSocket();

	int PushSysResp(sdk_client_comm_t*& pComm);
	int LoginCallBack(int err,int reqnum,sessionid_t sesid,privledge_t priv,int expiretime,int keepalivetime);
	int NotifyClient();
	int StartAudioDecodeCallBack(sdk_client_comm_t*& pComm);
	int StopAudioDecodeCallBack(sdk_client_comm_t*& pComm);
	int ResetLongTimeTimer();
	int PushAlarmComm(sdk_client_comm_t* pComm);

private:
    void __StopReadIo();
    void __StopWriteIo();
    void __StopReadTimer();
    void __StopWriteTimer();
	void __StopFailTimer();


    int __StartReadIo();
    int __StartWriteIo();
    int __StartReadTimer();
	int __ResetReadTimer();
    int __StartWriteTimer();
    int __ResetWriteTimer();
	int __StartFailTimer();

    int __ReadLoginIo();
    int __ReadLogin2Io();
    int __ReadLoginSuccIo();
    int __ReadStreamIo();
	int __ReadStreamAudioDualStartIo();
	int __ReadStreamAudioDualIo();
	int __ReadStreamAudioDualPendingIo();
	int __ReadStreamAudioDualPendingStartIo();
    int __ReadMessageIo();
    int __ReadCommandIo();

    int __WriteLoginIo();
    int __WriteLogin2Io();
    int __WriteLoginSuccIo();
    int __WriteStreamIo();
	int __WriteStreamAudioDualStartIo();
	int __WriteStreamAudioDualIo();
	int __WriteStreamAudioDualPendingIo();
	int __WriteStreamAudioDualPendingStartIo();
    int __WriteMessageIo();
    int __WriteCommandIo();
	int __WriteUpgradeIo();

    static void ReadIoCallBack(EV_P_ ev_io *w, int revents,void* arg);
    int __ReadIoImpl();

    static void WriteIoCallBack(EV_P_ ev_io *w, int revents,void* arg);
    int __WriteIoImpl();

    static void ReadTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);
    int __ReadTimerImpl();

    static void WriteTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);
    int __WriteTimerImpl();

	static void FailTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);
	static void WaitSysTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);

    void __ClearStreamIds();
	void __ClearOpenIds();
	void __ClearRespVecs();
	void __ClearFragVecs();

    int __LoginFailResponse(sdk_client_comm_t*& pComm,sdk_client_comm_t*& pRetComm,int err);
    int __LoginSuccResponse(sdk_client_comm_t*& pComm,sdk_client_comm_t*& pRetComm,sessionid_t sesid);
    int __HandleLoginMessage(sdk_client_comm_t*& pComm,sdk_client_comm_t*& pRetComm);
    int __HandleLogin2Message(sdk_client_comm_t*& pComm,sdk_client_comm_t*&pRetComm);

    int __HandleCommandRead(sdk_client_comm_t*& pComm);
    int __HandleUpgradeRead(sdk_client_comm_t*& pComm);
    int __HandleStreamRead(sdk_client_comm_t*& pComm);
    int __HandleMessageRead(sdk_client_comm_t*& pComm);
    int __HandleStreamVideoOpen(sdk_client_comm_t*& pComm);
    int __HandleStreamAudioOpen(sdk_client_comm_t*& pComm);
	int __HandleStreamAudioDualOpen(sdk_client_comm_t*& pComm);
	


    sdk_client_state_t __SetState(sdk_client_state_t state);
    sdk_client_state_t __GetState();

    int __StartStream(sys_stream_info_t* pStreamInfo);
    int __PrepareSendStreamInfo(sys_stream_info_t* pStreamInfo,std::vector<int>& succstreamids);
    int __CopyVideoInfo(sdk_video_info_t* pVInfo,SysPkgEncodeCfg *pSysCfg);
	int __CopyAudioInfo(sdk_audio_info_t* pAInfo,SysPkgAudioEncodeCfg *pAudioCfg);

	int __PushSysReqVec(sdk_client_comm_t*& pComm);
	int __PushSysRespVec(sdk_client_comm_t*& pComm);
	
    int __StartHandleRequest();
    /*ret == 1 ,write all the response , ret == 0 not write all ,but pending ,negative for error code*/
    int __WriteResponse();

    int __WriteStreamData(int streamid,struct iovec* iov,int iovlen);
    int __ChangePendings();
	int __LogoutHandle(sdk_client_comm_t* pComm);
	
	void __BreakOut();

	
	int __ReassembleFragResp(sdk_client_comm_t*& pInsertComm,sdk_client_comm_t** ppReassembleComm);
	int __IsOverlapDataOffset(unsigned int offa,unsigned int lena,unsigned int offb,unsigned int lenb);
	int __AudioFailedSend(void);
	int __StartWaitSysTimer(void);
	void __StopWaitSysTimer(void);
	int __InsertAudioDualNotify(uint32_t result,int seqid);
	int __HandleSdkServerInnerResp(sdk_client_comm_t*& pComm);

private:
    int m_Sock;
    SdkServerMgmt* m_pSvrMgmt;
    int *m_pRunningBits;
    uint8_t m_EncData[LOGIN_ENCDATA_LEN];
    sdk_client_state_t m_State;
	int m_ReqNum;
    int m_Ended;
	int m_Fail;
    sessionid_t m_SessionId;
    privledge_t m_Priv;
	int m_ExpireTime;
	int m_KeepAliveTime;

    /*this is the stream started handled information*/
    int m_StreamStarted;
    int m_InsertReadIo;
    ev_io m_EvReadIo;
    int m_InsertReadTimer;
    ev_timer m_EvReadTimer;
    int m_InsertWriteIo;
    ev_io m_EvWriteIo;
    int m_InsertWriteTimer;
    ev_timer m_EvWriteTimer;
	int m_InsertFailTimer;
	ev_timer m_EvFailTimer;
	int m_InsertWaitSysTimer;
	ev_timer m_EvWaitSysTimer;
	uint16_t m_AudioDualSeqId;

    SdkServerSock* m_pSock;
    std::vector<sdk_client_comm_t*> m_RespVec;
	std::vector<sdk_client_comm_t*> m_FragResp;
	sdk_client_comm_t *m_pLoginComm;


    /*streamids ,that is handled ,for stream used*/
    std::vector<int> m_StreamIds;
	std::vector<int> m_OpenIds;
    unsigned int m_CurGetStreamIds;
	uint64_t m_LastSendMills;
	uint64_t m_StartSendMills;
};

#endif /*__SDK_SERVER_CLIENT_H__*/

