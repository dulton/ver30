
#ifndef __SDK_SERVER_SESSION_H__
#define __SDK_SERVER_SESSION_H__

#include <sdk_client_comm.h>
#include <libev/ev.h>


#define  SESSION_EXPIRE_TIME   1
#undef   SESSION_EXPIRE_TIME

class SdkServerClient;

class SdkServerSession
{
public:
	SdkServerSession(sessionid_t sesid,privledge_t priv,int maxclient=10,int expiretime=900,int keepalivetime=10);
	~SdkServerSession();
	int Start();
	
	/*****************************
	*
	*  return number of the client in the session 
	*  if pClient == NULL ,just return the number of the session
	*****************************/
	int UnRegisterServerSession(SdkServerClient* pClient);
	int RegisterServerSession(SdkServerClient* pClient);
	int GetSessionId(sessionid_t& sesid,privledge_t& priv);
	/*
	        to renew session ,
		0 for not valid session now
		1 for valid and renewed
		negative is the other error code
	*/
	int SessionRenew(sessionid_t sesid,privledge_t& priv,int& expiretime,int& keeptime,int& err);
	int Clients();
	int IsValid(){return m_Valid;};

private:
#ifdef SESSION_EXPIRE_TIME	
	void __StopExpireTimer();
	int __StartExpireTimer();
	static void ExpireTimerFunc(EV_P_ ev_timer *w, int revents,void* arg);
#endif	
	void __StopKeepAliveTimer();
	int __StartKeepAliveTimer();
	int __ResetKeepAliveTimer();
	void __ReleaseResource();
 
 	static void KeepAliveTimerFunc(EV_P_ ev_timer *w, int revents,void* arg);
private:
	std::vector<SdkServerClient*> m_pClients;
	sessionid_t m_SesId;
	privledge_t m_Priv;
	int m_Valid;
	unsigned int m_MaxClients;
	int m_KeepAliveTime;

#ifdef SESSION_EXPIRE_TIME
	int m_ExpireTime;
	int m_InsertExpireTimer;
	ev_timer m_ExpireTimer;
#endif /*SESSION_EXPIRE_TIME*/	
	int m_InsertKeepAliveTimer;
	ev_timer m_KeepAliveTimer;
};

#endif /*__SDK_SERVER_SESSION_H__*/


