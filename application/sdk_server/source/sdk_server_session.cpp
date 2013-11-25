
#include <sdk_server_session.h>
#include <sdk_server_debug.h>
#include <sdk_server_client.h>

SdkServerSession::SdkServerSession(sessionid_t sesid,
                                   privledge_t priv,
                                   int maxclient ,
                                   int expiretime ,
                                   int keepalivetime )
    : m_SesId(sesid),
      m_Priv(priv),
      m_MaxClients(maxclient),
#ifdef  SESSION_EXPIRE_TIME
      m_ExpireTime(expiretime),
#endif /*SESSION_EXPIRE_TIME*/
      m_KeepAliveTime(keepalivetime)
{
    SDK_ASSERT(m_pClients.size() == 0);
    /*we make sure not valid */
    m_Valid = 0;
#ifdef SESSION_EXPIRE_TIME	
    m_InsertExpireTimer = 0;
#endif /*SESSION_EXPIRE_TIME*/
    m_InsertKeepAliveTimer = 0;
}

void SdkServerSession::__ReleaseResource()
{
    /*we can not release when session has in the vector*/
    SDK_ASSERT(this->m_pClients.size() == 0);
    this->__StopKeepAliveTimer();
#ifdef SESSION_EXPIRE_TIME	
    this->__StopExpireTimer();
#endif /*SESSION_EXPIRE_TIME*/
    this->m_Valid = 0;
}


SdkServerSession::~SdkServerSession()
{
	BACK_TRACE_FMT("Delete Session %d",this->m_SesId);
    this->__ReleaseResource();
    this->m_MaxClients = 0;
    this->m_KeepAliveTime = 0;
#ifdef SESSION_EXPIRE_TIME	
    this->m_ExpireTime = 0;
#endif /*SESSION_EXPIRE_TIME*/
}

int SdkServerSession::Start()
{
    int ret;
    this->__ReleaseResource();

    this->m_Valid = 1;

#ifdef SESSION_EXPIRE_TIME
    ret = this->__StartExpireTimer();
    if (ret <0)
    {
        this->__ReleaseResource();
        return ret;
    }
#endif /*SESSION_EXPIRE_TIME*/

    ret = this->__StartKeepAliveTimer();
    if (ret < 0)
    {
        this->__ReleaseResource();
        return ret;
    }

    return 0;
}

int SdkServerSession::SessionRenew(sessionid_t sesid,privledge_t & priv,int& expiretime,int& keeptime,int & err)
{
    int ret;
    if (sesid != this->m_SesId)
    {
        return 0;
    }

    if (this->m_Valid == 0)
    {
        err = ETIMEDOUT;
        return -ETIMEDOUT;
    }

    ret = this->__ResetKeepAliveTimer();
    if (ret < 0)
    {
        err = ETIMEDOUT;
        return -EFAULT;
    }

#ifdef SESSION_EXPIRE_TIME
	expiretime = this->m_ExpireTime;
#else /*SESSION_EXPIRE_TIME*/
	expiretime = 0;
#endif /*SESSION_EXPIRE_TIME*/
	keeptime = this->m_KeepAliveTime;
	DEBUG_INFO("KEEPTIME %d\n",keeptime);
    priv = this->m_Priv;
    return 1;
}


int SdkServerSession::GetSessionId(sessionid_t & sesid,privledge_t & priv)
{
    sesid = this->m_SesId;
    priv = this->m_Priv;
    return 0;
}


int SdkServerSession::RegisterServerSession(SdkServerClient * pClient)
{
    unsigned int i;
    if (this->m_pClients.size() >= this->m_MaxClients && this->m_MaxClients)
    {
        return -ENOSPC;
    }

    for (i=0; i<this->m_pClients.size(); i++)
    {
        if (this->m_pClients[i] == pClient)
        {
            return -EEXIST;
        }
    }

	DEBUG_INFO("session[%d] client %d register\n",this->m_SesId,pClient->GetSocket());
    this->m_pClients.push_back(pClient);
    return 0;
}

int SdkServerSession::UnRegisterServerSession(SdkServerClient * pClient)
{
    unsigned int i;

    for (i=0; i<this->m_pClients.size(); i++)
    {
        if (this->m_pClients[i] == pClient)
        {
            this->m_pClients.erase(this->m_pClients.begin() + i);
			DEBUG_INFO("session[%d] client %d unregister\n",this->m_SesId,pClient->GetSocket());
            return 1;
        }
    }
    return 0;
}

#ifdef SESSION_EXPIRE_TIME
void SdkServerSession::__StopExpireTimer()
{
    if (this->m_InsertExpireTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_ExpireTimer));
    }
    this->m_InsertExpireTimer = 0;
    return ;
}

int SdkServerSession::__StartExpireTimer()
{
    float ftime = this->m_ExpireTime;
    SDK_ASSERT(this->m_InsertExpireTimer == 0);
    ev_timer_init(&(this->m_ExpireTimer),SdkServerSession::ExpireTimerFunc,ftime,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_ExpireTimer));
    this->m_InsertExpireTimer = 1;
    return 0;
}

void SdkServerSession::ExpireTimerFunc(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkServerSession* pThis= (SdkServerSession*)arg;
    pThis->m_Valid = 0;
    pThis->__StopExpireTimer();
    return ;
}
#endif /*SESSION_EXPIRE_TIME*/

void SdkServerSession::__StopKeepAliveTimer()
{
    if (this->m_InsertKeepAliveTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_KeepAliveTimer));
    }
    this->m_InsertKeepAliveTimer = 0;
    return ;
}


int SdkServerSession::__StartKeepAliveTimer()
{
    float ftime = (float)this->m_KeepAliveTime;
    SDK_ASSERT(this->m_InsertKeepAliveTimer == 0);
    ev_timer_init(&(this->m_KeepAliveTimer),SdkServerSession::KeepAliveTimerFunc,ftime,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_KeepAliveTimer));
    this->m_InsertKeepAliveTimer = 1;
    return 0;
}

void SdkServerSession::KeepAliveTimerFunc(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkServerSession* pThis= (SdkServerSession*)arg;
    pThis->m_Valid = 0;
    pThis->__StopKeepAliveTimer();
    return ;
}

int SdkServerSession::__ResetKeepAliveTimer()
{
    if(this->m_Valid)
    {
        this->__StopKeepAliveTimer();
        return this->__StartKeepAliveTimer();
    }
    return 0;
}

int SdkServerSession::Clients()
{
    return this->m_pClients.size();
}


