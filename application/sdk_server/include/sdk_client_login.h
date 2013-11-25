
#ifndef __SDK_CLIENT_LOGIN_H__
#define __SDK_CLIENT_LOGIN_H__

#include <sdk_client_comm.h>
#include <sdk_client_sock.h>
#include <sdk_server_client.h>

#define MIDDLE_STRING_SIZE      64

#define LOGIN_CLIENT_REQUEST           0x1
#define LOGIN_CLIENT_RESPONSE          0x2
#define LOGOUT_CLIENT_REQUEST          0x3
#define LOGOUT_CLIENT_RESPONSE         0x4

#define  TYPE_AUTH_CLEAR_ALL_USER      0x3
#define  TYPE_AUTH_QUERY_USERS         0x4

#define  DES_KEY_LENGTH                8
#define  DES_PASSWORD_LENGTH           32

typedef struct
{
	int m_Sock;
	int m_ReqNum;
	int m_BodyLength;
	int m_OpCode;
}sdk_client_login_header_t;

typedef struct
{
	sdk_client_login_header_t m_Header;
	unsigned char m_Username[MIDDLE_STRING_SIZE];
	unsigned char m_Password[MIDDLE_STRING_SIZE];
	unsigned char m_Salt[MIDDLE_STRING_SIZE];	
}sdk_client_login_request_t;

typedef struct
{
	sdk_client_login_request_t m_Request;
	sessionid_t m_SesId;
	privledge_t m_Priv;
}sdk_client_login_response_t;

typedef struct
{
	sdk_client_login_header_t m_Header;
	sessionid_t m_SesId;
	privledge_t m_Priv;
}sdk_client_logout_request_t;

typedef struct
{
	sdk_client_login_header_t m_Header;
	uint32_t m_Result;
} sdk_client_logout_response_t;

class SdkServerMgmt;

class SdkClientLogin
{
public:
	SdkClientLogin(SdkServerMgmt* pSvrMgmt,int maxclients = 20,int *pRunningBits=NULL);
	~SdkClientLogin();
	int AddUserLoginRequest(SdkServerClient* pClient,
		         int reqnum,
		         const char * username,
		         const char * salt,
		         const char * md5check);

	int AddUserLogoutRequest(SdkServerClient* pClient,
			int reqnum,sessionid_t sesid,privledge_t priv);
	int UserLogoutClear(int reqnum,int timeout=3);
	int AddUserQueryLogins(int reqnum);
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
	void __StopWaitTimer();
	int __StartWaitTimer();

	void __ReleaseSendRequests();
	
    static void ReadTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);

	static void WriteTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);
	static void WaitTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);
	int __WaitTimerImpl();

	static void ReadIoCallBack(EV_P_ ev_io *w, int revents,void* arg);
	int __ReadIoImpl();
	
	static void WriteIoCallBack(EV_P_ ev_io *w, int revents,void* arg);
	int __WriteIoImpl();

	void __BreakOut();
	int __FormatSendingRequest();

	int __HandleLoginResp(sdk_client_comm_t* pComm);
	int __HandleLogoutResp(sdk_client_comm_t* pComm);

	int __WriteCommandSync(time_t etime);
	int __ReadCommandSync(time_t etime);

private:
	SdkServerMgmt* m_pSvrMgmt;
	int *m_pRunningBits;
	int m_MaxClients;
	SdkClientSock *m_pSock;
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
	int m_InsertWaitTimer;
	ev_timer m_EvWaitTimer;

	sdk_client_login_header_t* m_pSendingRequest;
	std::vector<sdk_client_login_header_t*> m_pSendRequest;
};

#endif /*__SDK_CLIENT_LOGIN_H__*/

