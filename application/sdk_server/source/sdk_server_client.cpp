
#include <sdk_server_client.h>
#include <sdk_server_mgmt.h>
#include <sdk_server_debug.h>

SdkServerClient::SdkServerClient(int sock,SdkServerMgmt * pServerMgmt,int * pRunningBits)
    :
    m_Sock(sock),
    m_pSvrMgmt(pServerMgmt),
    m_pRunningBits(pRunningBits)
{
    DEBUG_INFO("create client[%d]\n",sock);
    memset(m_EncData,0,sizeof(m_EncData));
    /*we assume ,that is loggin state*/
    m_State = sdk_client_login_state;
    m_ReqNum = -1;
    m_Ended = 0;
    m_Fail = 0;
    m_SessionId = 0;
    m_Priv = 0;

    /*for session time out is 900 seconds ,and the keep alive time is 10 seconds this is default*/
    m_ExpireTime = 900;
    m_KeepAliveTime = 15;
    /*we assume that stream is not started*/
    m_StreamStarted = 0;

    /*we assume that is */
    m_InsertReadIo = 0;
    m_InsertReadTimer = 0;
    m_InsertWriteIo = 0;
    m_InsertWriteTimer = 0 ;
    m_InsertFailTimer = 0;
    m_InsertWaitSysTimer = 0;
    m_AudioDualSeqId = 0;
    m_pSock = NULL;
    SDK_ASSERT(m_RespVec.size() == 0);
    SDK_ASSERT(m_FragResp.size() == 0);
    m_pLoginComm = NULL;
    SDK_ASSERT(m_StreamIds.size() == 0);

    /*default is from index of 0*/
    m_CurGetStreamIds = 0;
    m_LastSendMills = (uint64_t)0;
    m_StartSendMills = (uint64_t)0;
}


void SdkServerClient::__StopReadIo()
{
    if(this->m_InsertReadIo)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvReadIo));
    }
    this->m_InsertReadIo = 0;
    return ;
}

int SdkServerClient::__StartReadIo()
{
    SDK_ASSERT(this->m_pSock);
    SDK_ASSERT(this->m_InsertReadIo == 0);
    ev_io_init(&(this->m_EvReadIo),SdkServerClient::ReadIoCallBack,this->m_pSock->GetSocket(),EV_READ,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvReadIo));
    this->m_InsertReadIo = 1;
    return 0;
}

void SdkServerClient::__StopReadTimer()
{
    if(this->m_InsertReadTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvReadTimer));
    }
    this->m_InsertReadTimer = 0;
    return;
}

int SdkServerClient::__StartReadTimer()
{
    SDK_ASSERT(this->m_pSock);
    SDK_ASSERT(this->m_InsertReadTimer == 0);
    ev_timer_init(&(this->m_EvReadTimer),SdkServerClient::ReadTimerCallBack,10.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvReadTimer));
    this->m_InsertReadTimer = 1;
    return 0;
}

void SdkServerClient::__StopWriteIo()
{
    //BACK_TRACE();
    if(this->m_InsertWriteIo)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvWriteIo));
    }
    this->m_InsertWriteIo = 0;
    return ;
}

int SdkServerClient::__StartWriteIo()
{
    //BACK_TRACE();
    SDK_ASSERT(this->m_pSock);
    SDK_ASSERT(this->m_InsertWriteIo == 0);
    ev_io_init(&(this->m_EvWriteIo),SdkServerClient::WriteIoCallBack,this->m_pSock->GetSocket(),EV_WRITE,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvWriteIo));
    this->m_InsertWriteIo = 1;
    return 0;
}

void SdkServerClient::__StopWriteTimer()
{
    if(this->m_InsertWriteTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvWriteTimer));
    }
    this->m_InsertWriteTimer = 0;
    return;
}

int SdkServerClient::__StartWriteTimer()
{
    SDK_ASSERT(this->m_InsertWriteTimer == 0);
    ev_timer_init(&(this->m_EvWriteTimer),SdkServerClient::WriteTimerCallBack,15.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvWriteTimer));
    this->m_InsertWriteTimer = 1;
    return 0;
}


int SdkServerClient::__ResetWriteTimer()
{
    //DEBUG_INFO("[%d] timer reset\n",this->GetSocket());
    this->__StopWriteTimer();
    return this->__StartWriteTimer();
}

int SdkServerClient::__ResetReadTimer()
{
    this->__StopReadTimer();
    return this->__StartReadTimer();
}

void SdkServerClient::__StopFailTimer()
{
    if(this->m_InsertFailTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvFailTimer));
    }
    this->m_InsertFailTimer = 0;
    this->m_Fail = 0;
    return;
}

int SdkServerClient::__StartFailTimer()
{
    this->m_Fail = 1;
    if(this->m_InsertFailTimer == 0)
    {
        ev_timer_init(&(this->m_EvFailTimer),SdkServerClient::FailTimerCallBack,0.1,0.0,this);
        ev_timer_start(EV_DEFAULT,&(this->m_EvFailTimer));
        this->m_InsertFailTimer = 1;
    }
    return 0;
}

int SdkServerClient::__StartWaitSysTimer(void)
{
    SDK_ASSERT(this->m_InsertWaitSysTimer == 0);
    ev_timer_init(&(this->m_EvWaitSysTimer),SdkServerClient::WaitSysTimerCallBack,5.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvWaitSysTimer));
    this->m_InsertWaitSysTimer = 1;
    return 0;
}

void SdkServerClient::__StopWaitSysTimer(void)
{
    if(this->m_InsertWaitSysTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvWaitSysTimer));
    }
    this->m_InsertWaitSysTimer = 0;
    return ;
}

void SdkServerClient::WaitSysTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkServerClient* pThis= (SdkServerClient*)arg;
    if(pThis)
    {
        ERROR_INFO("[%d]wait sys timeout\n",pThis->GetSocket());
        delete pThis;
    }
    pThis = NULL;
    return;
}

void SdkServerClient::__ClearStreamIds()
{
    this->m_StreamIds.clear();
    return ;
}


void SdkServerClient::__ClearRespVecs()
{
    while(this->m_RespVec.size() > 0)
    {
        sdk_client_comm_t* pComm=NULL;
        pComm = this->m_RespVec[0];
        this->m_RespVec.erase(this->m_RespVec.begin());
        free(pComm);
        pComm = NULL;
    }
    return ;
}

void SdkServerClient::__ClearFragVecs()
{
    while(this->m_FragResp.size() > 0)
    {
        sdk_client_comm_t *pComm =NULL;
        pComm = this->m_FragResp[0];
        this->m_FragResp.erase(this->m_FragResp.begin());
        FreeComm(pComm);
    }
    return ;
}


#define STOP_ASSERT()   \
do\
{\
	SDK_ASSERT(this->m_State == sdk_client_login_state);\
	SDK_ASSERT(this->m_Ended == 0);\
	SDK_ASSERT(this->m_Fail == 0);\
	SDK_ASSERT(this->m_StreamStarted == 0);\
	SDK_ASSERT(this->m_InsertReadIo == 0);\
	SDK_ASSERT(this->m_InsertReadTimer == 0);\
	SDK_ASSERT(this->m_InsertWriteIo == 0);\
	SDK_ASSERT(this->m_InsertWriteTimer == 0);\
	SDK_ASSERT(this->m_InsertFailTimer == 0);\
	SDK_ASSERT(this->m_InsertWaitSysTimer == 0);\
	SDK_ASSERT(this->m_AudioDualSeqId == 0);\
	SDK_ASSERT(this->m_RespVec.size() == 0);\
	SDK_ASSERT(this->m_FragResp.size() == 0);\
	SDK_ASSERT(this->m_StreamIds.size() == 0);\
	SDK_ASSERT(this->m_CurGetStreamIds == 0);\
}while(0)

void SdkServerClient::Stop()
{
    m_LastSendMills = (uint64_t)0;
    m_StartSendMills = (uint64_t)0;
    if(this->m_pSvrMgmt)
    {
        this->m_pSvrMgmt->UnRegisterSdkClient(this);
    }



    /*to reset for the first one*/
    this->m_CurGetStreamIds = 0;


    this->__ClearStreamIds();

    if(this->m_pLoginComm)
    {
        free(this->m_pLoginComm);
    }
    this->m_pLoginComm = NULL;

    this->__ClearRespVecs();
    this->__ClearFragVecs();

    this->m_AudioDualSeqId = 0;
    this->__StopWaitSysTimer();
    this->__StopFailTimer();
    this->__StopWriteTimer();
    this->__StopWriteIo();
    this->__StopReadTimer();
    this->__StopReadIo();
    this->m_StreamStarted = 0;


    this->m_KeepAliveTime = 15;
    this->m_ExpireTime = 900;
    this->m_Priv = 0;
    this->m_SessionId = 0;
    this->m_Fail = 0;
    this->m_Ended = 0;
    this->m_ReqNum = -1;
    this->__SetState(sdk_client_login_state);

    STOP_ASSERT();
    return ;

}

int SdkServerClient::Start()
{
    int ret;
    this->Stop();

    if(this->m_pSvrMgmt == NULL)
    {
        ERROR_INFO("\n");
        this->Stop();
        return -EINVAL;
    }

    if(this->m_Sock >= 0)
    {
        SDK_ASSERT(this->m_pSock == NULL);
        this->m_pSock = new SdkServerSock(this->m_Sock);
        this->m_Sock = -1;
    }

    SDK_ASSERT(this->m_pSock);

    ret = this->m_pSvrMgmt->RegisterSdkClient(this);
    if(ret < 0)
    {
        ERROR_INFO("\n");
        this->Stop();
        return ret;
    }

    SDK_ASSERT(this->__GetState()== sdk_client_login_state);
    /*we should start to read io for login state*/


    ret = this->__StartReadIo();
    if(ret < 0)
    {
        ERROR_INFO("\n");
        this->Stop();
        return ret;
    }

    ret = this->__StartReadTimer();
    if(ret < 0)
    {
        ERROR_INFO("\n");
        this->Stop();
        return ret;
    }

    /*this is ok ,so do return*/
    return 0;
}


SdkServerClient::~SdkServerClient()
{
    BACK_TRACE();

    this->Stop();
    if(this->m_Sock >= 0)
    {
        DEBUG_INFO("delete client[%d] (%p)\n",this->m_Sock,this);
        SDK_ASSERT(this->m_pSock == NULL);
        close(this->m_Sock);
        this->m_Sock = -1;
    }

    if(this->m_pSock)
    {
        DEBUG_INFO("delete client[%d] (%p)\n",this->m_pSock->GetSocket(),this);
        SDK_ASSERT(this->m_Sock == -1);
        delete this->m_pSock;
        this->m_pSock = NULL;
    }
    this->m_pSvrMgmt = NULL;
    this->m_pRunningBits = NULL;
}


int SdkServerClient::__WriteLoginIo()
{
    int ret;

    ret= this->m_pSock->IsWriteSth();
    if(ret == 0)
    {
        /*if we have nothing to write ,we just set it to the sdk_client_login2_state*/
        this->__SetState(sdk_client_login2_state);
        this->m_pSock->ClearWrite();
        this->__StopWriteIo();
        this->__StopWriteTimer();
        return 0;
    }

    ret = this->m_pSock->Write();
    if(ret < 0)
    {
        return ret;
    }
    else if(ret == 0)
    {
        return 0;
    }

    /*ok ,we should get the job*/
    this->m_pSock->ClearWrite();
    /*we should change to login2 state*/
    this->__SetState(sdk_client_login2_state);
    this->__StopWriteIo();
    this->__StopWriteTimer();

    this->__StartReadIo();
    this->__StartReadTimer();

    return 1;
}




int SdkServerClient::__WriteLogin2Io()
{
    int ret;

    ret= this->m_pSock->IsWriteSth();
    if(ret == 0)
    {
        this->__SetState(sdk_client_login_succ_state);
        this->__StopWriteIo();
        this->__StopWriteTimer();
        /*we trust the connection ,so we do not set timerout*/
        DEBUG_INFO("\n");
		/*we should stop read io ,as it will make heart beat for handling*/
		this->__StopReadIo();
        ret = this->__StartReadIo();
        DEBUG_INFO("\n");
        if(ret < 0)
        {
            return ret;
        }
        return 0;
    }


    ret = this->m_pSock->Write();
    if(ret < 0)
    {
        return ret;
    }

    else if(ret == 0)
    {
        return 0;
    }

    /*ok ,we should get the job*/
    this->m_pSock->ClearWrite();


    /*we should set logined ,for next comming to change the state when the next coming*/
    //DEBUG_INFO("\n");
    this->__SetState(sdk_client_login_succ_state);
    //DEBUG_INFO("\n");
    this->__StopWriteIo();
    this->__StopWriteTimer();
    this->__StopReadTimer();
    /*we trust the connection ,so we do not set timerout*/
    //DEBUG_INFO("\n");
	/*we should stop read io ,as it will make heart beat for handling*/
	this->__StopReadIo();
    ret = this->__StartReadIo();
    //DEBUG_INFO("\n");
    if(ret < 0)
    {
        return ret;
    }
    return 1;
}


int SdkServerClient::__WriteIoImpl()
{
    int ret;

    /*if failed happened ,so we close socket*/
    if(this->m_Fail)
    {
        return -EFAULT;
    }

    switch(this->__GetState())
    {
    case sdk_client_login_state:
        ret = this->__WriteLoginIo();
        break;
    case sdk_client_login2_state:
    case sdk_client_login_succ_state:
        /*we put these all the same*/
        ret = this->__WriteLogin2Io();
        break;
    case sdk_client_stream_state:
        ret = this->__WriteStreamIo();
        break;
    case sdk_client_stream_audio_dual_state:
        ret = this->__WriteStreamAudioDualIo();
        break;
    case sdk_client_stream_audio_dual_pending_state:
        ret = this->__WriteStreamAudioDualPendingIo();
        break;
    case sdk_client_stream_audio_dual_pending_start_state:
        ret = this->__WriteStreamAudioDualPendingStartIo();
        break;
    case sdk_client_stream_audio_dual_start_state:
        ret = this->__WriteStreamAudioDualStartIo();
        break;
    case sdk_client_command_state:
        ret = this->__WriteCommandIo();
        break;
    case sdk_client_message_state:
        ret = this->__WriteMessageIo();
        break;
    case sdk_client_upgrade_state:
        ret = this->__WriteUpgradeIo();
        break;
    case sdk_client_logout_state:
    case sdk_client_close_state:
        /*we delete this*/
        ret = -1;
        break;

    default:
        /*we could not for this one*/
        SDK_ASSERT(0!=0);
        ret = -ENOTSUP;
        break;

    }

    if(this->m_Ended)
    {
        /*if we are the end of the socket communication ,when send the writing_message close socket*/
        return -EIO;
    }
    return ret;
}


int SdkServerClient::__HandleLoginMessage(sdk_client_comm_t*& pComm,sdk_client_comm_t*& pRetComm)
{
    login_request_t* preq=(login_request_t*)pComm->m_Data;
    uint32_t h32;
    uint16_t h16;
    login_response_t* presp=(login_response_t*)pRetComm->m_Data;
    //static char st_Md5Chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    time_t curt;
    unsigned int i;

    /*now we should fill the message for login response*/

    if(pComm->m_DataLen < sizeof(*preq))
    {
        ERROR_INFO("\n");
        return -EINVAL;
    }

    h32 = PROTO_TO_HOST32(preq->m_ReqId);
    if(h32 != LOGIN_REQUEST)
    {
        ERROR_INFO("\n");
        return -EINVAL;
    }

    h16 = PROTO_TO_HOST16(preq->m_SesId);
    if(h16 != 0)
    {
        ERROR_INFO("\n");
        return -EINVAL;
    }

    h16 = PROTO_TO_HOST16(preq->m_EncType);
    if(h16 != LOGIN_AUTH_CONSULT &&
            h16 != LOGIN_AUTH_DES)
    {
        ERROR_INFO("enc %d\n",h16);
        return -EINVAL;
    }

    h32 = HOST_TO_PROTO32(LOGIN_RESPONSE);
    presp->m_RespId = h32;

    h32 = HOST_TO_PROTO32(LOGIN_RESP_UNATHORIZED);
    presp->m_Result = h32;

    h16 = HOST_TO_PROTO16(0);
    presp->m_Sesid = h16;

    h16 = HOST_TO_PROTO16(LOGIN_AUTH_DES);
    presp->m_EncType= h16;

    /*now to make sure for the random char*/
    curt = time(NULL);
    srand(curt);
    memset(presp->m_EncData,0,sizeof(presp->m_EncData));
    for(i=0; i<sizeof(presp->m_EncData)-1; i++)
    {
        unsigned int rnd;
try_again_rnd:
        rnd = rand();
        rnd %= 256;
        if(rnd == 0)
        {
            goto try_again_rnd;
        }
        presp->m_EncData[i] = rnd;
    }
    presp->m_EncData[sizeof(presp->m_EncData)-1] = '\0';

    memcpy(this->m_EncData,presp->m_EncData,sizeof(this->m_EncData));
    //DEBUG_INFO("encdata %s (%s)\n",presp->m_EncData,this->m_EncData);
    pRetComm->m_SeqId = 0;
    pRetComm->m_Priv = 0;
    pRetComm->m_ServerPort = 0;
    pRetComm->m_SeqId = pComm->m_SeqId;
    pRetComm->m_Type = GMIS_PROTOCOL_TYPE_LOGGIN;
    pRetComm->m_Frag = 0;
    pRetComm->m_DataId = 0;
    pRetComm->m_Offset = 0;
    pRetComm->m_Totalsize = 0;
    pRetComm->m_DataLen = sizeof(*presp);

    /*we need to start writer*/
    return 1;
}


int SdkServerClient::__LoginFailResponse(sdk_client_comm_t*& pComm,sdk_client_comm_t*& pRetComm,int err)
{
    login_response_t* presp = (login_response_t*)pRetComm->m_Data;
    uint32_t n32;
    uint16_t n16;

    pRetComm->m_SesId = pComm->m_SesId;
    pRetComm->m_Priv = pComm->m_Priv;
    pRetComm->m_ServerPort = 0;
    pRetComm->m_SeqId = pComm->m_SeqId;
    pRetComm->m_Type = GMIS_PROTOCOL_TYPE_LOGGIN;
    pRetComm->m_Frag = 0;
    pRetComm->m_DataId = 0;
    pRetComm->m_Offset = 0;
    pRetComm->m_Totalsize = 0;
    pRetComm->m_DataLen = sizeof(*presp);

    n32 = HOST_TO_PROTO32(LOGIN_RESPONSE);
    presp->m_RespId = n32;
    n32 = HOST_TO_PROTO32(err);
    presp->m_Result = n32;
    n16 = HOST_TO_PROTO16(0);
    presp->m_Sesid = n16;

    /*to send this message and close socket*/
    BACK_TRACE_FMT("error sesid %d priv %d seqid %d err %d\n",pComm->m_SesId,pComm->m_Priv,pComm->m_SeqId,err);
    this->m_Ended = 1;
    return 0;
}

int SdkServerClient::__LoginSuccResponse(sdk_client_comm_t *& pComm,sdk_client_comm_t *& pRetComm,sessionid_t sesid)
{
    login_response_t* presp = (login_response_t*)pRetComm->m_Data;
    uint32_t n32,keeptimems;
    uint16_t n16;

    pRetComm->m_SesId = pComm->m_SesId;
    pRetComm->m_Priv = pComm->m_Priv;
    pRetComm->m_ServerPort = 0;
    pRetComm->m_SeqId = pComm->m_SeqId;
    //DEBUG_INFO("seqid %d %p\n",pComm->m_SeqId,pComm);
    pRetComm->m_Type = GMIS_PROTOCOL_TYPE_LOGGIN;
    pRetComm->m_FHB = pComm->m_FHB;
    pRetComm->m_Frag = 0;
    pRetComm->m_DataId = 0;
    pRetComm->m_Offset = 0;
    pRetComm->m_Totalsize = 0;
    pRetComm->m_DataLen = sizeof(*presp);

    n32 = HOST_TO_PROTO32(LOGIN_RESPONSE);
    presp->m_RespId = n32;
    n32 = HOST_TO_PROTO32(LOGIN_RESP_SUCC);
    presp->m_Result = n32;
    n16 = HOST_TO_PROTO16(sesid);
    presp->m_Sesid = n16;
    keeptimems = this->m_KeepAliveTime * 1000;
    presp->m_KeepTimeMS = HOST_TO_PROTO32(keeptimems);
    //DEBUG_INFO("sessionid %d\n",pRetComm->m_SesId);
    return 0;
}



int SdkServerClient::__ReadLoginIo()
{
    int ret,err=0;
    sessionid_t sesid;
    privledge_t priv;
    sdk_client_comm_t *pComm=NULL,*pRetComm=NULL;
    int expiretime,keeptime;


    SDK_ASSERT(this->m_pSock);
    ret = this->m_pSock->Read();
    if(ret < 0)
    {
        /*error ,or not read all ,so just return*/
        int realret=ret;
        ret = this->m_pSock->PutDirectData((unsigned char*)"GMI0300",7);
        if(ret < 0)
        {
            return ret;
        }



        /*stop read and we will exit for the job*/
        this->__StopReadIo();
        this->__StopReadTimer();
        /*now to start write time*/
        DEBUG_INFO("read error start io sock %d (realret %d)\n",this->m_pSock->GetSocket(),realret);
        ret = this->__StartWriteIo();
        if(ret < 0)
        {
            return ret;
        }

        ret = this->__StartWriteTimer();
        if(ret < 0)
        {
            return ret;
        }
        /*we put the end of data and we should close just after the sending data*/
        this->m_Ended = 1;
        ret = 0;
        ERROR_INFO("\n");
        return ret;
    }
    else if(ret == 0)
    {
        return ret;
    }

    /*it is read all so get it all*/
    SDK_ASSERT(ret > 0);
    pComm = this->m_pSock->GetRead();
    SDK_ASSERT(pComm);
    this->m_pSock->ClearRead();
    pRetComm = (sdk_client_comm_t*)calloc(sizeof(*pRetComm),1);
    if(pRetComm == NULL)
    {
        ERROR_INFO("\n");
        ret = -ENOMEM;
        goto fail;
    }

    if(pComm->m_Frag)
    {
        ERROR_INFO("\n");
        ret = -EINVAL;
        goto fail;
    }

    DEBUG_INFO("[%d]session id %d heartbeat %d seqnum %d\n",this->GetSocket(),pComm->m_SesId,pComm->m_FHB,pComm->m_SeqId);

    /*we should not */
    {
        /*now first we get the data*/
        if(pComm->m_SesId != 0)
        {
            sesid = pComm->m_SesId;
            ret = this->m_pSvrMgmt->SessionRenew(sesid,priv,expiretime,keeptime,err);
            if(ret <= 0)
            {
                /**/
                ret = this->__LoginFailResponse(pComm,pRetComm,LOGIN_RESP_AUTH_FAILED);
                if(ret < 0)
                {
                    ERROR_INFO("\n");
                    goto fail;
                }

                /*now to stop the readio read timer ,and start write io write timer*/
                this->__StopReadIo();
                this->__StopReadTimer();

                /*in the __PushSysRespVec it will restart the write io and write timer */
                /*this will give the session to insert return and it will give ok it will ok for it
                  if (this->m_SessionId != pRetComm->m_SesId) will ASSERT failed ,and not insert into the job*/
                this->m_SessionId = pRetComm->m_SesId;
                ret =this->__PushSysRespVec(pRetComm);
                if(ret < 0)
                {
                    ERROR_INFO("\n");
                    goto fail;
                }

                /*success ,the pRetComm is transfer into the other buffer control*/
                SDK_ASSERT(pRetComm == NULL);


                /*now to free pComm and this will comm*/
                SDK_ASSERT(pComm);
                FreeComm(pComm);
                /*we close socket failed*/
                ERROR_INFO("\n");
                this->m_Ended = 1;
                return 0;

            }
            else
            {
                /*it is ok ,so we should do this for sessionid and priv set ok*/
                this->m_SessionId = sesid;
                this->m_Priv = priv;
                this->m_ExpireTime = expiretime;
                this->m_KeepAliveTime = keeptime;
                pComm->m_SesId = sesid;
                pComm->m_Priv = priv;
                /*register session ok*/
                ret = this->m_pSvrMgmt->RegisterClientSession(sesid,this);
                if(ret < 0)
                {
                    goto fail;
                }
                switch(pComm->m_Type)
                {
                case GMIS_PROTOCOL_TYPE_LOGGIN:
                    ret = this->__LoginSuccResponse(pComm,pRetComm,sesid);
                    if(ret >=0)
                    {
                        /*this is success ,so we should do this ok*/
                        DEBUG_INFO("[%d]pRetComm FHB %d\n",this->GetSocket(),pRetComm->m_FHB);
                        this->__SetState(sdk_client_login_succ_state);
                        DEBUG_INFO("\n");
                        this->__StopReadIo();
                        this->__StopReadTimer();
                        ret = this->__PushSysRespVec(pRetComm);
                        if(ret < 0)
                        {
                            goto fail;
                        }
                        /*this is taken control by sending comm*/
                        SDK_ASSERT(pRetComm == NULL);
                        SDK_ASSERT(pComm);

                    }
                    break;
                case GMIS_PROTOCOL_TYPE_CONF:
                    this->__SetState(sdk_client_command_state);
                    ret = this->m_pSvrMgmt->ChangeClientConf(this);
                    if(ret < 0)
                    {
                        ERROR_INFO("\n");
                        goto fail;
                    }
                    /*we rely on the connection*/
                    this->__StopReadTimer();
                    /*if this is the handle message*/
                    ret = this->__HandleCommandRead(pComm);
                    if(ret >= 0)
                    {
                        /*if we success ,we put the structure into the SdkServerClientSysReq control,so it must be set NULL*/
                        SDK_ASSERT(pComm == NULL);
                    }
                    break;
                case GMIS_PROTOCOL_TYPE_UPGRADE:
                    this->__SetState(sdk_client_upgrade_state);
                    ret = this->__HandleUpgradeRead(pComm);
                    break;
                case GMIS_PROTOCOL_TYPE_MEDIA_CTRL:
                    this->__SetState(sdk_client_stream_state);
                    ret = this->m_pSvrMgmt->ChangeClientStream(this);
                    if(ret < 0)
                    {
                        goto fail;
                    }
                    /*now ,it is before the client handle stream*/
                    this->__StopReadTimer();
                    ret = this->__HandleStreamRead(pComm);
                    break;
                case GMIS_PROTOCOL_TYPE_LOG:
                case GMIS_PROTOCOL_TYPE_WARNING:
					DEBUG_INFO("Warning logging session(%d)\n",pComm->m_SesId);
                    this->__SetState(sdk_client_message_state);
                    ret = this->m_pSvrMgmt->ChangeClientAlarm(this);
                    if(ret < 0)
                    {
                        goto fail;
                    }
                    ret = this->__HandleMessageRead(pComm);
                    break;
                default:
                    ret = -EINVAL;
                    goto fail;
                }

                if(ret < 0)
                {
                    goto fail;
                }

            }
        }
        else
        {
            //DEBUG_INFO("\n");
            /*this is login we should test*/
            if(pComm->m_Type != GMIS_PROTOCOL_TYPE_LOGGIN)
            {
                ret = -EINVAL;
                ERROR_INFO("\n");
                goto fail;
            }

            //DEBUG_INFO("\n");
            /*now to send for the handle login*/
            ret = this->__HandleLoginMessage(pComm,pRetComm);
            if(ret < 0)
            {
                ERROR_INFO("\n");
                goto fail;
            }
            /**************************
            		*  we should give the stop read io
            		*  and not let the read timer ,this will ok
            		**************************/
            /*this is the bug to set for login2 ,if we just write the response packet ,and set it to the login2 state*/
            //this->__SetState(sdk_client_login2_state);
            this->__StopReadIo();
            this->__StopReadTimer();
            ret = this->__PushSysRespVec(pRetComm);
            if(ret < 0)
            {
                goto fail;
            }
            /*this is handled by the sys resp vec*/
            SDK_ASSERT(pRetComm == NULL);

        }

        if(ret < 0)
        {
            goto fail;
        }
    }


    if(pComm)
    {
        free(pComm);
    }
    pComm = NULL;
    if(pRetComm)
    {
        free(pRetComm);
    }
    pRetComm = NULL;



    return 0;
fail:
    if(pComm)
    {
        free(pComm);
    }
    pComm = NULL;
    if(pRetComm)
    {
        free(pRetComm);
    }
    pRetComm = NULL;
    return ret;
}

int SdkServerClient::__HandleLogin2Message(sdk_client_comm_t*& pComm,sdk_client_comm_t*&pRetComm)
{
    int ret;
    uint32_t h32;
    uint16_t h16;
    login_request_t* preq = (login_request_t*)pComm->m_Data;

    if(this->m_pLoginComm)
    {
        ERROR_INFO("\n");
        return -EPERM;
    }

    /*now to get the information*/
    if(pComm->m_DataLen< sizeof(*preq))
    {
        ERROR_INFO("\n");
        return -EINVAL;
    }

    h32 = PROTO_TO_HOST32(preq->m_ReqId);
    if(h32 != LOGIN_REQUEST)
    {
        ERROR_INFO("\n");
        return -EINVAL;
    }

    h16 = PROTO_TO_HOST16(preq->m_SesId);
    if(h16 != 0)
    {
        ERROR_INFO("session id 0x%04x\n",preq->m_SesId);
        return -EINVAL;
    }

    /**/
    h16 = PROTO_TO_HOST16(preq->m_EncType);
    if(h16 != LOGIN_AUTH_DES)
    {
        ERROR_INFO("\n");
        return -EINVAL;
    }

    /*if we have not set the encryption salt data set ,it will not set ok*/
    if(strlen((const char*)this->m_EncData) == 0)
    {
        ERROR_INFO("\n");
        return -EPERM;
    }

    h32 = PROTO_TO_HOST32(preq->m_ExpireTime);
    this->m_ExpireTime = h32;
    h32 = PROTO_TO_HOST32(preq->m_HeartBeatTime);
    h32 /= 1000000;
    this->m_KeepAliveTime = h32;
    //DEBUG_BUFFER_FMT(preq,sizeof(*preq),"GetKeepAliveTime %d heartbeattime 0x%08x",this->m_KeepAliveTime,preq->m_HeartBeatTime);

    //DEBUG_INFO("username %s encdata %s passwordmd5 %s\n",
    //           preq->m_UserName,
    //           this->m_EncData,
    //           preq->m_Password);
    ret = this->m_pSvrMgmt->UserLoginSession(this,(const char*)preq->m_UserName,(const char*)this->m_EncData,(const char*)preq->m_Password);
    if(ret < 0)
    {
        this->__StopReadIo();
        this->__StopReadTimer();
        ret = this->__LoginFailResponse(pComm,pRetComm,-ret);
        if(ret < 0)
        {
            return ret;
        }
        ret = this->__PushSysRespVec(pRetComm);
        if(ret < 0)
        {
            return ret;
        }
        return 0;
    }
    //DEBUG_INFO("seqid %d %p\n",pComm->m_SeqId,pComm);

    this->m_pLoginComm = pComm;
    pComm = NULL;

    /*now to stop read and stop write ,and start read timer to let it ok*/
    this->m_ReqNum = ret;
    return 1;
}


int SdkServerClient::__ReadLogin2Io()
{
    int ret;
    sdk_client_comm_t *pComm=NULL,*pRetComm=NULL;

    ret = this->m_pSock->Read();
    if(ret < 0)
    {
        ERROR_INFO("\n");
        return -EINVAL;
    }
    else if(ret == 0)
    {
        ERROR_INFO("\n");
        return 0;
    }

    pComm = this->m_pSock->GetRead();
    SDK_ASSERT(pComm);


    if(pComm->m_SesId != 0)
    {
        ERROR_INFO("\n");
        return -EINVAL;
    }
    if(pComm->m_Type != GMIS_PROTOCOL_TYPE_LOGGIN)
    {
        ERROR_INFO("\n");
        return -EINVAL;
    }

    pRetComm = (sdk_client_comm_t*)calloc(sizeof(*pRetComm),1);
    if(pRetComm == NULL)
    {
        SDK_ASSERT(pComm);
        free(pComm);
        pComm =NULL;
        ERROR_INFO("\n");
        return -ENOMEM;
    }

    /*now to give the handle message*/
    ret = this->__HandleLogin2Message(pComm,pRetComm);
    if(ret < 0)
    {
        SDK_ASSERT(pComm);
        SDK_ASSERT(pRetComm);
        free(pComm);
        free(pRetComm);
        pComm = NULL;
        pRetComm = NULL;
        ERROR_INFO("\n");
        return ret;
    }
    else if(ret == 0)
    {
        /*this means failed login ,so we should */
        SDK_ASSERT(pRetComm == NULL);
        SDK_ASSERT(pComm);
        free(pComm);
        pComm = NULL;
        ERROR_INFO("\n");
        return 0;
    }

    /*we have put pComm into the pLoginComm ,so it would be NULL*/
    SDK_ASSERT(pComm==NULL);
    SDK_ASSERT(pRetComm);
    free(pRetComm);
    pComm = NULL;
    pRetComm = NULL;
    /*
    	stop read io and write io ,because ,we are in the login state ,we should keep this state ok
    	but we do not stop readio timer and write timer ,because ,we should give this ok
        */
    this->__StopReadIo();
    this->__StopWriteIo();
    this->__StopWriteTimer();
    this->__StopReadTimer();
    ret = this->__StartReadTimer();
    if(ret < 0)
    {
        return ret;
    }

    return 0;
}


int SdkServerClient::__ReadLoginSuccIo()
{
    int ret;
    sdk_client_comm_t  *pComm=NULL,*pRetComm=NULL;
    int err;

    ret = this->m_pSock->Read();
    if(ret < 0)
    {
        return ret;
    }
    else if(ret < 0)
    {
        /*if we have not start timer for read ,so make sure 1 packet in time*/
        if(this->m_InsertReadTimer == 0)
        {
            ret = this->__StartReadTimer();
            if(ret < 0)
            {
                return ret;
            }
        }
        return 0;
    }

    /*we read one packet ,so we should stop read timer*/
    this->__StopReadTimer();

    pComm = this->m_pSock->GetRead();
    SDK_ASSERT(pComm);
    if(pComm->m_SesId != this->m_SessionId)
    {
        ret=  -EINVAL;
        goto fail;
    }

    pRetComm = (sdk_client_comm_t*)calloc(sizeof(*pRetComm),1);
    if(pRetComm == NULL)
    {
        ret = -ENOMEM;
        goto fail;
    }

	DEBUG_INFO("pComm Type (%d)\n",pComm->m_Type);
    switch(pComm->m_Type)
    {
    case GMIS_PROTOCOL_TYPE_LOGGIN:
        ret = this->m_pSvrMgmt->SessionRenew(this->m_SessionId,this->m_Priv,this->m_ExpireTime,this->m_KeepAliveTime,err);
        if(ret <= 0)
        {
            ret = this->__LoginFailResponse(pComm,pRetComm ,LOGIN_RESP_AUTH_FAILED);
            if(ret < 0)
            {
                goto fail;
            }

            ret = this->__PushSysRespVec(pRetComm);
            if(ret <0)
            {
                goto fail;
            }

            SDK_ASSERT(pRetComm == NULL);
            SDK_ASSERT(pComm);
            free(pComm);
            pComm = NULL;

            /*this would stop read timer and we should start read io*/
            this->__StopReadTimer();

            return 0;
        }
        else
        {
            ret = this->__LoginSuccResponse(pComm,pRetComm,pComm->m_SesId);
            if(ret < 0)
            {
                goto fail;
            }
            ret = this->__PushSysRespVec(pRetComm);
            if(ret <0)
            {
                goto fail;
            }

            SDK_ASSERT(pRetComm == NULL);
            SDK_ASSERT(pComm);
            free(pComm);
            pComm = NULL;

            this->__StopReadIo();
            this->__StopReadTimer();

            return 0;
        }
        break;
    case GMIS_PROTOCOL_TYPE_CONF:
        this->__SetState(sdk_client_command_state);
        ret = this->m_pSvrMgmt->ChangeClientConf(this);
        if(ret < 0)
        {
            goto fail;
        }
        ret = this->__HandleCommandRead(pComm);
        if(ret < 0)
        {
            goto fail;
        }
        /*we would not set the write timer or write io*/
        break;
    case GMIS_PROTOCOL_TYPE_UPGRADE:
        this->__SetState(sdk_client_upgrade_state);
        ret = this->__HandleUpgradeRead(pComm);
        if(ret < 0)
        {
            goto fail;
        }
        break;
    case GMIS_PROTOCOL_TYPE_MEDIA_CTRL:
        this->__SetState(sdk_client_stream_state);
        ret = this->m_pSvrMgmt->ChangeClientStream(this);
        if(ret < 0)
        {
            goto fail;
        }
        ret = this->__HandleStreamRead(pComm);
        if(ret < 0)
        {
            goto fail;
        }
        break;
    case GMIS_PROTOCOL_TYPE_LOG:
    case GMIS_PROTOCOL_TYPE_WARNING:
		DEBUG_INFO("warning change\n");
        this->__SetState(sdk_client_message_state);
        ret = this->m_pSvrMgmt->ChangeClientAlarm(this);
        if(ret < 0)
        {
            goto fail;
        }
        ret = this->__HandleMessageRead(pComm);
        if(ret < 0)
        {
            goto fail;
        }
        break;
    default:
        ret = -EINVAL;
        goto fail;
    }


    if(ret < 0)
    {
        goto fail;
    }

    if(pComm)
    {
        free(pComm);
    }
    pComm = NULL;
    if(pRetComm)
    {
        free(pRetComm);
    }
    pRetComm = NULL;
    return 0;

fail:
    FreeComm(pComm);
    FreeComm(pRetComm);
    return ret;
}



int SdkServerClient::__ReadIoImpl()
{
    int ret;

    if(this->m_Fail)
    {
        return -EFAULT;
    }

    /*we at the end ,so we should not read any more*/
    if(this->m_Ended)
    {
        return 0;
    }
    //DEBUG_INFO("state %d\n",this->__GetState());

    switch(this->__GetState())
    {
    case sdk_client_login_state:
        ret = this->__ReadLoginIo();
        break;
    case sdk_client_login2_state:
        ret = this->__ReadLogin2Io();
        break;
    case sdk_client_login_succ_state:
        ret = this->__ReadLoginSuccIo();
        break;
    case sdk_client_stream_state:
        ret = this->__ReadStreamIo();
        break;
    case sdk_client_stream_audio_dual_start_state:
        ret = this->__ReadStreamAudioDualStartIo();
        break;
    case sdk_client_stream_audio_dual_state:
        ret = this->__ReadStreamAudioDualIo();
        break;
    case sdk_client_stream_audio_dual_pending_state:
        ret = this->__ReadStreamAudioDualPendingIo();
        break;
    case sdk_client_stream_audio_dual_pending_start_state:
        ret = this->__ReadStreamAudioDualPendingStartIo();
        break;
    case sdk_client_command_state:
        ret = this->__ReadCommandIo();
        break;
    case sdk_client_message_state:
        ret = this->__ReadMessageIo();
        break;
    case sdk_client_logout_state:
    case sdk_client_close_state:
        ret = -1;
        break;

    default:
        /*we could not for this one*/
        SDK_ASSERT(0!=0);
        ret = -1;
        break;
    }

    return ret;
}

void SdkServerClient::ReadIoCallBack(EV_P_ ev_io *w, int revents,void* arg)
{
    int ret;
    SdkServerClient* pThis = (SdkServerClient*)arg;
    ret = pThis->__ReadIoImpl();
    if(ret < 0)
    {
        ERROR_INFO("delete [%d]\n",pThis->GetSocket());
        delete pThis;
    }
    return ;
}


void SdkServerClient::WriteIoCallBack(EV_P_ ev_io *w, int revents,void* arg)
{
    int ret;
    SdkServerClient* pThis = (SdkServerClient*)arg;
    ret = pThis->__WriteIoImpl();
    if(ret < 0)
    {
        DEBUG_INFO("ret %d\n",ret);
        delete pThis;
    }
    return ;
}

int SdkServerClient::__ReadTimerImpl()
{
    return -1;
}


void SdkServerClient::ReadTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg)
{
    int ret;
    SdkServerClient* pThis = (SdkServerClient*)arg;
    DEBUG_INFO("pThis %p\n",pThis);
    ret = pThis->__ReadTimerImpl();
    if(ret < 0)
    {

        DEBUG_INFO("pThis %p (%d)\n",pThis,pThis->GetSocket());
        delete pThis;
    }
    DEBUG_INFO("pThis %p\n",pThis);
    return ;
}


int SdkServerClient::__WriteTimerImpl()
{
    return -1;
}

void SdkServerClient::WriteTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg)
{
    int ret;
    SdkServerClient* pThis = (SdkServerClient*)arg;

    ret = pThis->__WriteTimerImpl();
    if(ret < 0)
    {
        delete pThis;
    }
    return ;
}

void SdkServerClient::__BreakOut()
{
    BACK_TRACE();
    ev_break(EV_DEFAULT,EVBREAK_ONE);
    return ;
}


int SdkServerClient::LoginCallBack(int err,int reqnum,sessionid_t sesid,privledge_t priv,int expiretime,int keepalivetime)
{
    int ret,res;
    sdk_client_comm_t *pRetComm=NULL;
    if(this->__GetState()!= sdk_client_login2_state ||
            this->m_ReqNum != reqnum || this->m_pLoginComm == NULL)
    {
        /*we are not in the right state ,so it is not the client wait for the return value*/
        if(this->m_pLoginComm)
        {
            free(this->m_pLoginComm);
            this->m_pLoginComm = NULL;
        }
        return -EINVAL;
    }

    if(this->m_Fail)
    {
        return -EPERM;
    }
    //DEBUG_INFO("login[%d] with sessionid %d err(%d)\n",this->m_pSock->GetSocket(),sesid,err);

    /*now it response ,so we can do the next handle*/
    this->__StopReadTimer();
    this->__StopWriteTimer();
    pRetComm = (sdk_client_comm_t*)calloc(sizeof(*pRetComm),1);
    if(pRetComm == NULL)
    {
        ret=  -ENOMEM;
        goto fail;
    }

    if(err)
    {
        ret = this->__LoginFailResponse(this->m_pLoginComm,pRetComm,LOGIN_RESP_AUTH_FAILED);
        if(ret < 0)
        {
            /*the client will close when call next write*/
            goto fail;
        }

        ret = this->__PushSysRespVec(pRetComm);
        if(ret < 0)
        {
            goto fail;
        }

        SDK_ASSERT(this->m_pLoginComm);
        SDK_ASSERT(pRetComm == NULL);
        free(this->m_pLoginComm);
        this->m_pLoginComm = NULL;
        return 0;
    }


    /*it is ok*/
    //DEBUG_INFO("sesid %d\n",sesid);
    this->m_SessionId = sesid;
    this->m_Priv = priv;
    DEBUG_INFO("keepalivetime %d\n",this->m_KeepAliveTime);
    if(this->m_ExpireTime > expiretime || this->m_ExpireTime == 0)
    {
        this->m_ExpireTime = expiretime;
    }

    if(this->m_KeepAliveTime > keepalivetime || this->m_KeepAliveTime == 0)
    {
        this->m_KeepAliveTime = keepalivetime;
    }
    DEBUG_INFO("keepalivetime %d\n",this->m_KeepAliveTime);

    /*we should change into the new session id to set*/
    this->m_pLoginComm->m_SesId = this->m_SessionId;
    this->m_pLoginComm->m_Priv = this->m_Priv;

    /*register client session ,this will give it ok*/
    ret = this->m_pSvrMgmt->RegisterClientSession(sesid,this);
    if(ret < 0)
    {
        goto fail;
    }

    ret = this->__LoginSuccResponse(this->m_pLoginComm,pRetComm,sesid);
    if(ret < 0)
    {
        goto fail;
    }

    ret = this->__PushSysRespVec(pRetComm);
    if(ret < 0)
    {
        goto fail;
    }

    SDK_ASSERT(pRetComm == NULL);
    SDK_ASSERT(this->m_pLoginComm);
    free(this->m_pLoginComm);
    this->m_pLoginComm= NULL;


    return 0;

fail:
    if(this->m_pLoginComm)
    {
        free(this->m_pLoginComm);
        this->m_pLoginComm = NULL;
    }
    if(pRetComm)
    {
        free(pRetComm);
    }
    pRetComm =NULL;

    res = this->__StartFailTimer();
    if(res < 0)
    {
        ERROR_INFO("could not start fail timer in client [%d]\n",this->m_pSock->GetSocket());
        this->__BreakOut();
    }
    return ret;
}


int SdkServerClient::GetSocket()
{
    if(this->m_pSock)
    {
        return this->m_pSock->GetSocket();
    }
    else
    {
        return this->m_Sock;
    }
}


void SdkServerClient::FailTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg)
{
    SdkServerClient* pThis = (SdkServerClient*)arg;
    delete pThis;
    return;
}

sdk_client_state_t SdkServerClient::__SetState(sdk_client_state_t state)
{
    sdk_client_state_t oldstate=this->m_State;
    DEBUG_INFO("[%d]change state(%d) => state(%d)\n",this->m_pSock ? this->m_pSock->GetSocket() : -1,oldstate,state);
    this->m_State = state;
    return oldstate;
}

sdk_client_state_t SdkServerClient::__GetState()
{
    return this->m_State;
}

int SdkServerClient::GetSessionId(sessionid_t & sesid,privledge_t & priv)
{
    sesid = this->m_SessionId ;
    priv = this->m_Priv;
    return 0;
}


int SdkServerClient::__IsOverlapDataOffset(unsigned int offa,unsigned int lena,unsigned int offb,unsigned int lenb)
{
    /*it is not overlap for b < a*/
    if((offb)<offa && (offb + lenb) <= offa)
    {
        return 0;
    }

    /*it is not overlap for a < b*/
    if((offa) <(offb) && (offa + lena) <= offb)
    {
        return 0;
    }

    return 1;
}

/******************************************
*  insert the fragmentation sdk_client_comm into the buffer and
*  return 1 : for assemble all the clients   *ppReassembleComm != NULL
*  return 0 : for insert clients ,and the *ppReassembleComm == NULL
*  negative error code
******************************************/
int SdkServerClient::__ReassembleFragResp(sdk_client_comm_t * & pInsertComm,sdk_client_comm_t * * ppReassembleComm)
{
    int ret;
    unsigned int i;
    sdk_client_comm_t *pCurComm=NULL;
    int insertidx=0;
    sdk_client_comm_t *pReassembleComm=NULL;
    int copiedlen=0;

    if(pInsertComm->m_Totalsize > MAX_CLIENT_COMM_SIZE)
    {
        /*it is a too big packets to reassemble*/
        return 0;
    }

    if(this->m_FragResp.size() > 0)
    {
        pCurComm = this->m_FragResp[0];
        /*now to check if this insert comm is the same as for the comm*/
        if(pInsertComm->m_SeqId != pCurComm->m_SeqId ||
                pInsertComm->m_SesId != pCurComm->m_SesId ||
                pInsertComm->m_Totalsize != pCurComm->m_Totalsize)
        {
            /*this is not the valid one so discard this insert comm*/
            ERROR_INFO("insert [sesid(%d):seqid(%d)totalsize(%d)] => list[sesid(%d):seqid(%d)totalsize(%d)]\n",
                       pInsertComm->m_SesId,pInsertComm->m_SeqId,pInsertComm->m_Totalsize,
                       pCurComm->m_SesId,pCurComm->m_SeqId,pCurComm->m_Totalsize);
            return 0;
        }
    }

    for(i=0; i<this->m_FragResp.size() ; i++)
    {
        pCurComm  = this->m_FragResp[i];
        ret = this->__IsOverlapDataOffset(pCurComm->m_Offset,pCurComm->m_DataLen,pInsertComm->m_Offset,pInsertComm->m_DataLen);
        /*now compare the size*/
        if(ret)
        {
            ERROR_INFO("[%d] %d + %d overlap insert %d + %d\n",
                       i,pCurComm->m_Offset,pCurComm->m_DataLen,
                       pInsertComm->m_Offset,pInsertComm->m_DataLen);
            return 0;
        }
    }

    /*now to see where to insert*/
    for(i=0; i<this->m_FragResp.size() ; i++)
    {
        pCurComm = this->m_FragResp[i];
        if(pCurComm->m_DataId == pInsertComm->m_DataId)
        {
            ERROR_INFO("[%d] dataid %d == insert %d\n",i,pCurComm->m_DataId,pInsertComm->m_DataId);
            return 0;
        }

        if(pCurComm->m_DataId < pInsertComm->m_DataId &&
                pCurComm->m_Offset > pInsertComm->m_Offset)
        {
            ERROR_INFO("[%d] dataid[%d]offset[%d] Conflict with Insert dataid[%d][%d]\n",
                       i,pCurComm->m_DataId,pCurComm->m_Offset,
                       pInsertComm->m_DataId,pInsertComm->m_Offset);
            return 0;
        }

        if(pCurComm->m_DataId > pInsertComm->m_DataId &&
                pCurComm->m_Offset < pInsertComm->m_Offset)
        {
            ERROR_INFO("[%d] dataid[%d]offset[%d] Conflict with Insert dataid[%d][%d]\n",
                       i,pCurComm->m_DataId,pCurComm->m_Offset,
                       pInsertComm->m_DataId,pInsertComm->m_Offset);
            return 0;
        }
    }

    insertidx = -1;
    for(i=0; i<this->m_FragResp.size() ; i++)
    {
        pCurComm = this->m_FragResp[i];
        if(pCurComm->m_DataId > pInsertComm->m_DataId)
        {
            /*now insert for the new one*/
            insertidx = i;
            break;
        }
    }


    /*now it is ok ,so we should insert into the vector*/
    if(insertidx < 0)
    {
        this->m_FragResp.push_back(pInsertComm);
    }
    else
    {
        std::vector<sdk_client_comm_t*>::iterator biter = this->m_FragResp.begin();
        this->m_FragResp.insert(biter,insertidx,pInsertComm);
    }
    pInsertComm = NULL;

    for(i = 0; i<this->m_FragResp.size() ; i++)
    {
        pCurComm = this->m_FragResp[i];
        /*now to check for it is all the things ok*/
        if((i+1)<this->m_FragResp.size())
        {
            sdk_client_comm_t* pCur2 = this->m_FragResp[i+1];
            if(pCur2->m_Offset!= (pCurComm->m_Offset+pCurComm->m_DataLen))
            {
                /*has some whole in the buffer ,so do not reassemble*/
                return 0;
            }

            if(i == 0 && pCurComm->m_Offset != 0)
            {
                /*it is not from the begin*/
                return 0;
            }
        }
        else
        {
            /*it is the last one ,so check for the total size*/
            if((pCurComm->m_Offset + pCurComm->m_DataLen) !=
                    pCurComm->m_Totalsize)
            {
                return 0;
            }
        }
    }

    /*ok ,we should reassemble all the buffer*/
    pReassembleComm = (sdk_client_comm_t*)AllocateComm(pCurComm->m_Totalsize);
    if(pReassembleComm == NULL)
    {
        ret = -errno ? -errno : -1;
        goto fail;
    }

    /*first to set for the header */
    pCurComm = this->m_FragResp[0];
    pReassembleComm->m_SesId = pCurComm->m_SesId;
    pReassembleComm->m_Priv = pCurComm->m_Priv;
    pReassembleComm->m_ServerPort = 0;
    pReassembleComm->m_LocalPort = 0;
    pReassembleComm->m_SeqId = pCurComm->m_SeqId;
    /*this is the configuration for this*/
    pReassembleComm->m_Type = GMIS_PROTOCOL_TYPE_CONF;
    pReassembleComm->m_FHB = 0;
    pReassembleComm->m_Frag = 0;
    pReassembleComm->m_DataId = 0;
    pReassembleComm->m_Offset = 0;
    pReassembleComm->m_Totalsize = 0;
    pReassembleComm->m_DataLen = pCurComm->m_Totalsize;

    copiedlen = 0;
    for(i=0; i<this->m_FragResp.size(); i++)
    {
        pCurComm = this->m_FragResp[i];
        memcpy(pReassembleComm->m_Data + copiedlen ,pCurComm->m_Data,pCurComm->m_DataLen);
        copiedlen += pCurComm->m_DataLen;
    }

    SDK_ASSERT(copiedlen == pCurComm->m_Totalsize);

    this->__ClearFragVecs();
    *ppReassembleComm = pReassembleComm;

    return 1;
fail:
    FreeComm(pReassembleComm);
    return ret;
}


int SdkServerClient::ResetLongTimeTimer()
{
    int ret;
    if(this->m_InsertReadTimer > 0)
    {
        this->__StopReadTimer();
        ret = this->__StartReadTimer();
        if(ret < 0)
        {
            return ret;
        }
    }

    if(this->m_InsertWriteTimer > 0)
    {
        this->__StopWriteTimer();
        ret = this->__StartWriteTimer();
        if(ret < 0)
        {
            return ret;
        }
    }

    if(this->m_InsertWaitSysTimer > 0)
    {
        this->__StopWaitSysTimer();
        ret = this->__StartWaitSysTimer();
        if(ret < 0)
        {
            return ret;
        }
    }

    return 0;
}

