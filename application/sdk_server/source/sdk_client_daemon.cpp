
#include <sdk_client_daemon.h>
#include <sdk_server_debug.h>
#include <sdk_server_mgmt.h>

#define  DAEMON_SIZE 32
#define  MAX_SEQIDS_WAIT      5
#define  SDK_SERVER_REGISTER_MSG         "SdkServer"
#define  SDK_SERVER_LINK_MSG             "SdkServer Link"
#define  SDK_SERVER_QUIT_MSG             "QUIT"


SdkClientDaemon::SdkClientDaemon(SdkServerMgmt * pSvrMgmt,int timeout,int * pRunningBits )
    :m_pSvrMgmt(pSvrMgmt),
     m_SendTimeout(timeout),
     m_pRunningBits(pRunningBits),
     m_ReqNum(1),
     m_State(cd_logout_state)
{
    m_InsertWriteIo = 0;
    m_InsertWriteTimer = 0;
    m_InsertReadIo = 0;
    m_InsertReadTimer = 0;
    m_InsertSendTimer = 0;
    m_InsertRegisterTimer = 0;
    m_RPort = -1;
    m_LPort = -1;
    m_pSock = NULL;

    SDK_ASSERT(m_pSendingComms.size() == 0);
    SDK_ASSERT(m_SendSeqIds.size() == 0);
}

void SdkClientDaemon::__ClearClientComms()
{
    sdk_client_comm_t *pComm=NULL;

    while(this->m_pSendingComms.size() > 0)
    {
        SDK_ASSERT(pComm == NULL);
        pComm = this->m_pSendingComms[0];
        this->m_pSendingComms.erase(this->m_pSendingComms.begin());
        free(pComm);
        pComm = NULL;
    }
    return ;
}

#define  ASSERT_STOP()\
do\
{\
	SDK_ASSERT(this->__GetState() == cd_logout_state);\
	SDK_ASSERT(this->m_InsertWriteIo == 0);\
	SDK_ASSERT(this->m_InsertWriteTimer == 0);\
	SDK_ASSERT(this->m_InsertReadIo == 0);\
	SDK_ASSERT(this->m_InsertReadTimer == 0);\
	SDK_ASSERT(this->m_InsertSendTimer == 0);\
	SDK_ASSERT(this->m_InsertRegisterTimer == 0);\
	SDK_ASSERT(this->m_LPort < 0);\
	SDK_ASSERT(this->m_RPort < 0); \
	SDK_ASSERT(this->m_pSock == NULL);\
	SDK_ASSERT(this->m_pSendingComms.size() == 0);\
	SDK_ASSERT(this->m_SendSeqIds.size() == 0);\
}while(0)

void SdkClientDaemon::Stop()
{
    /*stop all timers*/
    this->__SetState(cd_logout_state);
    this->__StopWriteIo();
    this->__StopWriteTimer();
    this->__StopReadIo();
    this->__StopReadTimer();
    this->__StopSendTimer();
    this->__StopRegisterTimer();

    this->m_LPort = -1;
    this->m_RPort = -1;
    if (this->m_pSock)
    {
        delete this->m_pSock;
        this->m_pSock = NULL;
    }
    this->__ClearClientComms();
    this->m_SendSeqIds.clear();

    ASSERT_STOP();
    return ;
}


SdkClientDaemon::~SdkClientDaemon()
{
    this->Stop();
    this->m_pRunningBits = NULL;
    this->m_pSvrMgmt = NULL;
    this->m_SendTimeout = -1;
    this->m_ReqNum = -1;
}


int SdkClientDaemon::__GetLPort()
{
    return 56788;
}

int SdkClientDaemon::__GetRPort()
{
    return 56780;
}

int SdkClientDaemon::__SetSocketUnBlock(int sock)
{
    int ret;
    int flags;

    errno = 0;
    flags = fcntl(sock,F_GETFL);
    if (flags == -1 && errno)
    {
        ret = -errno;
        return ret;
    }

    ret = fcntl(sock,F_SETFL,flags | O_NONBLOCK);
    if (ret < 0)
    {
        ret = -errno ?   -errno :  -1;
        return ret;
    }
    return 0;
}


int SdkClientDaemon::__BindSocket()
{
    int ret;
    int sock=-1;
    struct sockaddr_in saddr;
    socklen_t socklen;


    sock = socket(AF_INET,SOCK_DGRAM,0);
    if (sock < 0)
    {
        ret = -errno ? -errno : -1;
        goto fail;
    }

    ret  = this->__GetLPort();
    if (ret < 0)
    {
        goto fail;
    }
    this->m_LPort = ret;

    ret = this->__GetRPort();
    if (ret < 0)
    {
        goto fail;
    }
    this->m_RPort = ret;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    saddr.sin_port = htons(this->m_LPort);
    DEBUG_INFO("LPORT %d\n",this->m_LPort);

    socklen = sizeof(saddr);
    ret = bind(sock,(struct sockaddr*)&saddr,socklen);
    if (ret < 0)
    {
        goto fail;
    }

    ret = this->__SetSocketUnBlock(sock);
    if (ret < 0)
    {
        goto fail;
    }

    SDK_ASSERT(this->m_pSock==NULL);
    this->m_pSock = new SdkClientSock(sock);
    sock = -1;

    return 0;

fail:
    if (sock >= 0)
    {
        close(sock);
    }
    sock = -1;
    return ret;
}


int SdkClientDaemon::Start()
{
    int ret;
    sdk_client_comm_t *pComm=NULL;
    this->Stop();

    ret = this->__BindSocket();
    if (ret < 0)
    {
        this->Stop();
        return ret;
    }

    SDK_ASSERT(this->m_pSock && this->m_pSock->IsWriteSth() == 0);
    pComm = this->__GetRegisterComm();
    if (pComm == NULL)
    {
        ret = -errno ? -errno : -1;
        goto fail;
    }

    ret = this->m_pSock->PushData(pComm);
    if (ret < 0)
    {
        goto fail;
    }

    /*now to push the seqid into the waiting seqid*/
    this->m_SendSeqIds.push_back(pComm->m_SeqId);
    pComm = NULL;

    ret = this->__StartReadIo();
    if (ret < 0)
    {
        goto fail;
    }

    ret = this->__StartWriteIo();
    if (ret < 0)
    {
        goto fail;
    }

    ret = this->__StartWriteTimer();
    if (ret < 0)
    {
        goto fail;
    }

    ret = this->__StartRegisterTimer();
    if (ret < 0)
    {
        goto fail;
    }

    /*we do not start send timer ,because ,this when we received send state*/

    SDK_ASSERT(this->m_pSock->IsWriteSth() > 0);
    SDK_ASSERT(pComm == NULL);

    return 0;

fail:
    if (pComm)
    {
        free(pComm);
    }
    pComm = NULL;
    this->Stop();
    return ret;
}

int SdkClientDaemon::__IncReqNum()
{
    this->m_ReqNum += 1;
    if (this->m_ReqNum > 0xffff)
    {
        this->m_ReqNum = 0;
    }
    return this->m_ReqNum ;
}


sdk_client_comm_t* SdkClientDaemon::__GetRegisterComm()
{
    sdk_client_comm_t* pComm=NULL;
    char* pDst;

    pComm = (sdk_client_comm_t*)calloc(sizeof(*pComm),1);
    if (pComm == NULL)
    {
        return NULL;
    }

    pComm->m_SesId = 0;
    pComm->m_Priv = 0;
    pComm->m_ServerPort = this->m_RPort;
    pComm->m_LocalPort = this->m_LPort;
    pComm->m_SeqId = this->__IncReqNum();
    pComm->m_Type = GMIS_PROTOCOL_TYPE_SDK_TO_SYS;
    pComm->m_FHB = 0;
    pComm->m_Frag = 0;
    pComm->m_DataId = 0;
    pComm->m_Offset = 0;
    pComm->m_Totalsize = 0;
    pComm->m_DataLen = DAEMON_SIZE;
    pDst = (char*) pComm->m_Data;
    strncpy(pDst,SDK_SERVER_REGISTER_MSG,DAEMON_SIZE-1);

    return pComm;
}

sdk_client_comm_t* SdkClientDaemon::__GetReportComm()
{
    sdk_client_comm_t* pComm=NULL;
    char* pDst;

    pComm = AllocateComm(DAEMON_SIZE);
    if (pComm == NULL)
    {
        return NULL;
    }

    pComm->m_SesId = 0;
    pComm->m_Priv = 0;
    pComm->m_ServerPort = this->m_RPort;
    pComm->m_LocalPort = this->m_LPort;
    pComm->m_SeqId = this->__IncReqNum();
    pComm->m_Type = GMIS_PROTOCOL_TYPE_SDK_TO_SYS;
    pComm->m_FHB = 0;
    pComm->m_Frag = 0;
    pComm->m_DataId = 0;
    pComm->m_Offset = 0;
    pComm->m_Totalsize = 0;
    pComm->m_DataLen = DAEMON_SIZE;
    pDst = (char*) pComm->m_Data;
    strncpy(pDst,SDK_SERVER_LINK_MSG,DAEMON_SIZE-1);

    return pComm;
}


int SdkClientDaemon::__StartReadIo()
{
    SDK_ASSERT(this->m_InsertReadIo == 0);
    SDK_ASSERT(this->m_pSock);
    ev_io_init(&(this->m_EvReadIo),SdkClientDaemon::ReadIoCallBack,this->m_pSock->Socket(),EV_READ,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvReadIo));
    this->m_InsertReadIo = 1;
    return 0;
}

void SdkClientDaemon::__StopReadIo()
{
    if (this->m_InsertReadIo)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvReadIo));
    }
    this->m_InsertReadIo = 0;
    return;
}

int SdkClientDaemon::__StartReadTimer()
{
    SDK_ASSERT(this->m_InsertReadTimer == 0);
    ev_timer_init(&(this->m_EvReadTimer),SdkClientDaemon::ReadTimerCallBack,3.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvReadTimer));
    this->m_InsertReadTimer = 1;
    return 0;
}

void SdkClientDaemon::__StopReadTimer()
{
    if (this->m_InsertReadTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvReadTimer));
    }
    this->m_InsertReadTimer = 0;
    return;
}



int SdkClientDaemon::__StartWriteIo()
{
    SDK_ASSERT(this->m_InsertWriteIo== 0);
    SDK_ASSERT(this->m_pSock);
    ev_io_init(&(this->m_EvWriteIo),SdkClientDaemon::WriteIoCallBack,this->m_pSock->Socket(),EV_WRITE,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvWriteIo));
    this->m_InsertWriteIo= 1;
    return 0;
}

void SdkClientDaemon::__StopWriteIo()
{
    if (this->m_InsertWriteIo)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvWriteIo));
    }
    this->m_InsertWriteIo = 0;
    return;
}

int SdkClientDaemon::__StartWriteTimer()
{
    SDK_ASSERT(this->m_InsertWriteTimer == 0);
    ev_timer_init(&(this->m_EvWriteTimer),SdkClientDaemon::WriteTimerCallBack,3.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvWriteTimer));
    this->m_InsertWriteTimer = 1;
    return 0;
}

void SdkClientDaemon::__StopWriteTimer()
{
    if (this->m_InsertWriteTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvWriteTimer));
    }
    this->m_InsertWriteTimer = 0;
    return;
}

int SdkClientDaemon::__ResetWriteTimer()
{
    this->__StopWriteTimer();
    return this->__StartWriteTimer();
}


int SdkClientDaemon::__StartSendTimer()
{
    float ftime = (float)this->m_SendTimeout;
    SDK_ASSERT(this->m_InsertSendTimer == 0);
    ev_timer_init(&(this->m_EvSendTimer),SdkClientDaemon::SendTimerCallBack,ftime,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvSendTimer));
    this->m_InsertSendTimer = 1;
    return 0;
}

void SdkClientDaemon::__StopSendTimer()
{
    if (this->m_InsertSendTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvSendTimer));
    }
    this->m_InsertSendTimer = 0;
    return ;
}

int SdkClientDaemon::__ResetSendTimer()
{
    this->__StopSendTimer();
    return this->__StartSendTimer();
}

int SdkClientDaemon::__StartRegisterTimer()
{
	DEBUG_INFO("\n");
    SDK_ASSERT(this->m_InsertRegisterTimer == 0);
    ev_timer_init(&(this->m_EvRegisterTimer),SdkClientDaemon::RegisterTimerCallBack,3.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvRegisterTimer));
    this->m_InsertRegisterTimer = 1;
    return 0;
}

void SdkClientDaemon::__StopRegisterTimer()
{
    if (this->m_InsertRegisterTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvRegisterTimer));
    }
	this->m_InsertRegisterTimer = 0;
	return;
}

void SdkClientDaemon::__BreakOut()
{
	BACK_TRACE();
    ev_break (EV_DEFAULT,EVBREAK_ONE);
}

int SdkClientDaemon::__SendTimerImpl()
{
    int ret;
    sdk_client_comm_t *pComm = NULL;
    sdk_client_comm_t* pGetComm=NULL;

    if (this->m_pSendingComms.size() > MAX_SEQIDS_WAIT)
    {
        /*yes this is really a bug ,when we can not send to the server*/
        return -EIO;
    }

    pComm = this->__GetReportComm();
    if (pComm == NULL)
    {
        return -ENOMEM;
    }

    if (this->m_pSock->IsWriteSth() > 0)
    {

        this->m_pSendingComms.push_back(pComm);
        pComm = NULL;
        return 0;
    }
    else
    {
        if (this->m_pSendingComms.size() >0)
        {
            pGetComm = this->m_pSendingComms[0];
            this->m_pSendingComms.erase(this->m_pSendingComms.begin());
            ret = this->m_pSock->PushData(pGetComm);
            if (ret < 0)
            {
                goto fail;
            }
            this->m_SendSeqIds.push_back(pGetComm->m_SeqId);
            pGetComm = NULL;
            this->m_pSendingComms.push_back(pComm);
            pComm = NULL;
        }
        else
        {
            ret = this->m_pSock->PushData(pComm);
            if (ret < 0)
            {
                goto fail;
            }
            this->m_SendSeqIds.push_back(pComm->m_SeqId);

            /*we start write io and timer ,for it is the first to send*/
            pComm = NULL;
            ret = this->__StartWriteIo();
            if (ret < 0)
            {
                goto fail;
            }

            ret = this->__StartWriteTimer();
            if (ret < 0)
            {
                goto fail;
            }
        }

    }

    if (this->m_SendSeqIds.size() > MAX_SEQIDS_WAIT)
    {
        ret = -EFAULT;
        ERROR_INFO("send seqids %d\n",this->m_SendSeqIds.size());
        goto fail;
    }

    SDK_ASSERT(pComm == NULL);
    SDK_ASSERT(pGetComm == NULL);
    return 0;

fail:
    if (pComm)
    {
        free(pComm);
    }
    pComm = NULL;
    if (pGetComm)
    {
        free(pGetComm);
    }
    pGetComm = NULL;
    return ret;
}


void SdkClientDaemon::SendTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkClientDaemon* pThis = (SdkClientDaemon*)arg;
    int ret;

    ret = pThis->__SendTimerImpl();
    if (ret < 0)
    {
        pThis->__BreakOut();
    }
    else
    {
        ret = pThis->__ResetSendTimer();
        if (ret < 0)
        {
            pThis->__BreakOut();
        }
    }
    return ;
}


seqid_t SdkClientDaemon::__FindSeqId(sdk_client_comm_t * pComm)
{
    unsigned int i;
    int findidx=-1;

    for(i=0; i<this->m_SendSeqIds.size(); i++)
    {
        if (this->m_SendSeqIds[i] == pComm->m_SeqId)
        {
            findidx = i;
            break;
        }
    }

    if (findidx <0)
    {
        ERROR_INFO("not find %d seqid\n",pComm->m_SeqId);
        return -1;
    }

    //DEBUG_INFO("Get Response for seq (%d)\n",pComm->m_SeqId);
    this->m_SendSeqIds.erase(this->m_SendSeqIds.begin() + findidx);
    return pComm->m_SeqId;

}


int SdkClientDaemon::__ReadIoImpl()
{
    int ret,exited = 0;
    sdk_client_comm_t *pComm=NULL;
    char* pChar;

    ret =this->m_pSock->Read();
    if (ret < 0)
    {
        return ret;
    }
    else if (ret == 0)
    {
        if (this->m_InsertReadTimer == 0)
        {
            ret = this->__StartReadTimer();
            if (ret < 0)
            {
                return ret;
            }
        }
        return 0;
    }

    /*we have read one packet ,so just stop the timer*/
    this->__StopReadTimer();
    pComm = this->m_pSock->GetRead();
    SDK_ASSERT(pComm);

    this->__FindSeqId(pComm);
    /*now we should test if it is the stop */
    pChar = (char*)pComm->m_Data;
    if (pComm->m_DataLen >= DAEMON_SIZE)
    {
        if (strcmp(pChar,SDK_SERVER_QUIT_MSG)==0)
        {
            exited = 1;
        }
        else if (strcmp(pChar,SDK_SERVER_REGISTER_MSG)==0)
        {
            cd_enum_state_t oldstate;
            oldstate = this->__SetState(cd_login_state);
            if (oldstate == cd_logout_state)
            {
                /*we have registered*/
                this->__StopRegisterTimer();
                /*now to start for the send timer*/
                ret = this->__StartSendTimer();
                if (ret < 0)
                {
                    goto fail;
                }
            }
            else
            {
                DEBUG_INFO("receive (%s) in state(%d)\n",pChar,oldstate);
            }
        }
    }

    free(pComm);
    pComm = NULL;

    if (exited >  0)
    {
        return -EFAULT;
    }
    return 0;
fail:
    if (pComm)
    {
        free(pComm);
    }
    pComm = NULL;
    return ret;
}

void SdkClientDaemon::ReadIoCallBack(EV_P_ ev_io * w,int revents,void * arg)\
{
    SdkClientDaemon* pThis = (SdkClientDaemon*)arg;
    int ret;

    ret = pThis->__ReadIoImpl();
    if (ret < 0)
    {
        pThis->__BreakOut();
    }

    return ;
}

int SdkClientDaemon::__WriteIoImpl()
{
    int ret;
    sdk_client_comm_t *pComm=NULL;

    ret = this->m_pSock->IsWriteSth();
    if (ret > 0)
    {
        ret = this->m_pSock->Write();
        if (ret < 0)
        {
			ERROR_INFO("\n");
            return ret;
        }
        else if (ret == 0)
        {
            return 0;
        }
        this->__ResetWriteTimer();
    }

	SDK_ASSERT(this->m_SendSeqIds.size() > 0);
	//DEBUG_INFO("write seqnum (%d) succ\n",this->m_SendSeqIds[this->m_SendSeqIds.size() - 1]);

    /*ok to find out whether it is something to write*/
    if (this->m_pSendingComms.size() == 0)
    {
        this->m_pSock->ClearWrite();
        this->__StopWriteIo();
        this->__StopWriteTimer();
        return 0;
    }

    pComm = this->m_pSendingComms[0];
    this->m_pSendingComms.erase(this->m_pSendingComms.begin());
    SDK_ASSERT(pComm);

    ret = this->m_pSock->PushData(pComm);
    if (ret < 0)
    {
		FreeComm(pComm);
		ERROR_INFO("\n");
        return ret;
    }
    this->m_SendSeqIds.push_back(pComm->m_SeqId);
    /*we have reset the timer ,so do not anything ok*/
    pComm = NULL;

    if (this->m_SendSeqIds.size() > MAX_SEQIDS_WAIT)
    {
        ERROR_INFO("seqids (%d)\n",this->m_SendSeqIds.size());
        return -EFAULT;
    }
    return 0;

}


void SdkClientDaemon::WriteIoCallBack(EV_P_ ev_io * w,int revents,void * arg)
{
    SdkClientDaemon* pThis = (SdkClientDaemon*)arg;
    int ret;

    ret = pThis->__WriteIoImpl();
    if (ret < 0)
    {
        pThis->__BreakOut();
    }

    return ;
}


cd_enum_state_t  SdkClientDaemon::__GetState()
{
    return this->m_State;
}


cd_enum_state_t SdkClientDaemon::__SetState(cd_enum_state_t state)
{
    cd_enum_state_t oldstate;
    oldstate = this->m_State;
    this->m_State = state;
    return oldstate;
}

void SdkClientDaemon::ReadTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkClientDaemon* pThis = (SdkClientDaemon*)arg;
	ERROR_INFO("timeout\n");
    pThis->__BreakOut();
    return;
}


void SdkClientDaemon::WriteTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkClientDaemon* pThis = (SdkClientDaemon*)arg;
	ERROR_INFO("timeout\n");
    pThis->__BreakOut();
    return;
}

void SdkClientDaemon::RegisterTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkClientDaemon* pThis = (SdkClientDaemon*)arg;
	ERROR_INFO("timeout\n");
    pThis->__BreakOut();
    return;
}

int SdkClientDaemon::ResetLongTimeTimer()
{
	int ret;

	if (this->m_InsertWriteTimer > 0)
	{
		this->__StopWriteTimer();
		ret = this->__StartWriteTimer();
		if (ret < 0)
		{
			return ret;
		}
	}

	if (this->m_InsertReadTimer > 0)
	{
		this->__StopReadTimer();
		ret = this->__StartReadTimer();
		if (ret < 0)
		{
			return ret;
		}
	}

	if (this->m_InsertRegisterTimer > 0)
	{
		this->__StopRegisterTimer();
		ret = this->__StartRegisterTimer();
		if (ret < 0)
		{
			return ret;
		}
	}

	return 0;
}

