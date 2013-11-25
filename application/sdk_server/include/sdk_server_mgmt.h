
#ifndef __SDK_SERVER_MGMT_H__
#define __SDK_SERVER_MGMT_H__

#define  MAX_IOV_LEN  4

#include <sdk_server_client.h>
#include <sdk_server_sys_req.h>
#include <sdk_server_sys_comm.h>
#include <sdk_client_login.h>
#include <sdk_client_daemon.h>
#include <sys_stream_info.h>
#include <sdk_audio_buffer.h>
#include <sdk_audio_dbuffer.h>

#define SDK_SERVER_MGMT_SOCK     0xffff

class SdkServerMgmt
{
public:
	SdkServerMgmt(int *pRunningBits=NULL,int maxclients=20,int maxsessions=10,int maxpacks=10);
	~SdkServerMgmt();
	void Stop();
	int Start();
	int RegisterSdkClient(SdkServerClient* pClient);
	int ChangeClientStream(SdkServerClient* pClient);
	int ChangeClientConf(SdkServerClient* pClient);
	void UnRegisterSdkClient(SdkServerClient* pClient);
	int StopAllStreams(void);
	int StartAllStreams(sys_stream_info_t* pStreamInfo);
	int ResumeAllStreams(sys_stream_info_t* pStreamInfo);
	int PauseAllStreams(void);
	int QueryStreamStarted(void);
	int GetClients();
	int SessionRenew(sessionid_t sesid,privledge_t& priv,int& expiretime,int& keeptime,int& err);
	int UserLoginSession(SdkServerClient* pClient,const char* username,const char* salt,const char* md5check);
	int UserLogoutSession(SdkServerClient* pClient);
	int UserLoginSessionCallBack(int sock,int reqnum,int err,sessionid_t sesid,privledge_t priv);
	int UserLogoutSessionCallBack(int sock,int reqnum,int err,sessionid_t sesid);
	int UserQueryLoginCallBack(int reqnum,std::vector<sessionid_t>& sesids,std::vector<privledge_t>& privs);
	int RegisterClientSession(sessionid_t sesid,SdkServerClient* pClient);

	/***********************************************
	*   this function just start the stream ,
	*    1 means all is started ,0 for not start all ,it must wait for the job
	*     negative error code
	***********************************************/
	int StartStreamId(int streamid,SdkServerClient *pClient);

	/***********************************************
	*   because ,when call UnRegisterClient will stop all stream id when
	*   the client exit , 
	*   but in the case of resume stream ,and it will stop stream id
	*   that is the same
	***********************************************/
	void StopStreamId(int streamid,SdkServerClient *pClient);

	/*this will change the streamid into the pending state into the right one*/
	int PauseStreamId(int streamid,SdkServerClient *pClient);

	int ResumeStreamId(int streamid,SdkServerClient *pClient);
	
	int GetStreamData(int sock,int streamid,struct iovec* pIoVec,int& iovlen,int& begin);
	/*if 1 it is forward at the end of the packet ,if 0 ,it is success ,negative error code*/
	int ForwardStreamData(int sock,int streamid,struct iovec* pIoVec,int iovlen,int forwardlen);
	/**/
	int PushSysReq(sdk_client_comm_t*& pComm);
	int PushSysResp(sdk_client_comm_t*& pComm);
	int GetStreamIdInfo(sys_stream_info_t* pSysQueryStreamids);

	void DebugStreamBufferBlock(int streamid,int sock);
	int InitDaemon();
	int IsNeedOpenAudio(int streamid);


	/*audio decode functions*/
	/*audio media data server start function,this function strongly recommended when before call RequestAudioStart*/
	int StartAudioDecoder(int sock,AudioDecParam* pAudioDec);
	/*audio media data server stop function ,this function strongly recommended when after call RequestAudioStop*/
	int StopAudioDecoder(int sock);
	/*this function call when notify the sys_server ,it has started audio decoder server and will give the return value*/
	int RequestAudioStart(SdkServerClient* pClient,AudioDecParam* pAudioDec);
	/*this function call when we want to stop audio ,this will */
	int RequestAudioStop(SdkServerClient* pClient);

	int HandleAudioStopCallBack(sdk_client_comm_t*& pComm);
	int HandleAudioStartCallBack(sdk_client_comm_t*& pComm);

	int PushAudioDecodeData(int sock,sdk_client_comm_t*& pComm);

private:
	void __StopRemoveStreamTimer();
	int __StartRemoveStreamTimer();
	void __StopRemoveConfTimer();
	int __StartRemoveConfTimer();
	void __StopReleaseDBufferTimer();
	int __StartReleaseDBufferTimer();
	void __StopPullTimer();
	int __StartPullTimer();
	int __ResetPullTimer();
	int __TryStartPullTimer();
	int __TryStopPullTimer();

	static void RemoveStreamTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg);
	int __RemoveStreamTimerImpl();
	static void RemoveConfTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg);
	int __RemoveConfTimerImpl();

	static void PullTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg);
	int __PullTimerImpl();
	static void ReleaseDBufferTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg);

	int __RemoveAllStreamAndBuffers();
	int __RemoveSpecStreamBuffer(SdkServerBuffer* pBuffer);
	int __RemoveSpecStreamClient(SdkServerClient* pClient);
	int __RemoveSpecAudioBuffer(SdkAudioBuffer* pAudio);
	int __RemoveSpecDBuffer(SdkAudioDBuffer* pDBuffer);

	void __ReleaseClientLogins();
	void __ReleaseClientConfs();
	void __ReleaseClientStreams();

	void __ReleaseAllSessions();

	void __ReleaseAllRemoveSessionIds();
	
	int __CopySysStreamInfo(sys_stream_info_t* pSysStreamInfo);

	void __RemoveClientVectors(SdkServerClient* pClient);
	void __RemoveClientSessions(SdkServerClient* pClient);

	int __IncReqNum();

	void __BreakOut();

	int __GarbageCollectSessions(int num);
	int __ResumeStreamIdVideo(int streamid,SdkServerClient *pClient);
	int __ResumeStreamIdAudio(int streamid,SdkServerClient *pClient);
	int __StartRequestReleaseDBuffer(int sock);
	void __DirectReleaseDBuffer();
	int __SendStartAudioDecodeRequest(int sock,AudioDecParam* pAudioDec);
	int __SendStopAudioDecodeRequest(int sock);
	int __HandleInnerSysReq(sdk_client_comm_t*& pComm);
	int __ResetLongTimeTimer();
#ifdef AUDIO_DUAL_FILE_EMULATE
	int __StartStartAudioDualFileTimer();
	void __StopStartAudioDualFileTimer();
	int __StartStopAudioDualFileTimer();
	void __StopStopAudioDualFileTimer();
	static void StartAudioDualFileTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg);
	int __StartAudioDualFileTimerImpl();
	static void StopAudioDualFileTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg);
	int __StopAudioDualFileTimerImpl();	
#endif
private:
	int *m_pRunningBits;
	int m_MaxClients;
	unsigned int m_MaxSessions;
	int m_MaxPacks;
	int m_ReqNum;
	int m_ExpireTime;
	int m_KeepTime;
	std::vector<SdkServerSession*>m_pSessions;
	/*client running in the login stage*/
	std::vector<SdkServerClient*>m_pClientLogins;
	/*client running for configuration*/
	std::vector<SdkServerClient*>m_pClientConfs;
	/*client running for stream transfer*/
	std::vector<SdkServerClient*>m_pClientStreams;
	std::vector<sessionid_t> m_WaitingRemoveSessionId;

	/*these are the error handle timers*/
	int m_InsertRemoveStreamsTimer;
	ev_timer m_EvRemoveStreamsTimer;
	int m_InsertRemoveConfTimer;
	ev_timer m_EvRemoveConfigTimer;
	int m_InsertReleaseDBufferTimer;
	ev_timer m_EvReleaseDBufferTimer;
	std::vector<SdkServerClient*> m_WillRemoveClients;
	std::vector<SdkServerBuffer*> m_WillRemoveBuffers;
	SdkAudioBuffer* m_pWillRemoveAudioBuffer;
	SdkAudioDBuffer* m_pWillRemoveDBuffer;
	

	SdkServerBuffer *m_pStreamBuffers[MAX_STREAM_IDS];
	SdkAudioBuffer *m_pAudioBuffer;
	SdkAudioDBuffer *m_pDBuffer;
	int m_DBufferEnding;
	AudioDecParam m_AudioDecParam;
	SdkServerSysReq *m_pSysReq;
	SdkServerSysComm *m_pSysComm;
	SdkClientLogin *m_pLoginHandler;
	SdkClientDaemon *m_pDaemon;
	sys_stream_info_t *m_pStreamInfo;
	int m_StreamStarted;

	int m_InsertPullTimer;
	ev_timer m_EvPullTimer;
#ifdef AUDIO_DUAL_FILE_EMULATE
	int m_InsertStartAudioDualFileTimer;
	ev_timer m_EvStartAudioDualFileTimer;
	int m_InsertStopAudioDualFileTimer;
	ev_timer m_EvStopAudioDualFileTimer;
	seqid_t m_StartSeqId;
	seqid_t m_StopSeqId;
#endif

};

#endif /*__SDK_SERVER_MGMT_H__*/

