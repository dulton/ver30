
#include <sys_env_types.h>
#include <sdk_server_alarm.h>
#include <sdk_server_debug.h>
#include <sdk_proto_pack.h>
#include <sdk_server_mgmt.h>
#include <memory>

#define TYPE_WARNING_INFO      122
#define SYSCODE_GET_ALARM_REQ  1229
#define SYSCODE_GET_ALARM_RSP  1230


SdkServerAlarm::SdkServerAlarm(SdkServerMgmt * pSvrMgmt) : m_pSvrMgmt(pSvrMgmt)
{
    m_pSock = NULL;
    m_LPort = 0;
    m_RPort = 0;
    SDK_ASSERT(m_pReqCommVecs.size() == 0);
    m_InsertReadIo = 0;
    m_InsertReadTimer = 0;
    m_InsertWriteIo = 0;
    m_InsertWriteTimer = 0;
}

void SdkServerAlarm::__ClearComms()
{
    sdk_client_comm_t* pComm = NULL;
    while(this->m_pReqCommVecs.size() > 0)
    {
        SDK_ASSERT(pComm == NULL);
        pComm = this->m_pReqCommVecs[0];
        this->m_pReqCommVecs.erase(this->m_pReqCommVecs.begin());
        FreeComm(pComm);
    }
    return ;
}

void SdkServerAlarm::Stop()
{
    this->__StopReadIo();
    this->__StopReadTimer();
    this->__StopWriteIo();
    this->__StopWriteTimer();
    this->__ClearComms();
    if(this->m_pSock)
    {
        delete this->m_pSock;
    }
    this->m_pSock = NULL;
    this->m_LPort = 0;
    this->m_RPort = 0;
    return ;
}

SdkServerAlarm::~SdkServerAlarm()
{
    this->Stop();
    this->m_pSvrMgmt = NULL;
}

int SdkServerAlarm::__StartReadIo()
{
    SDK_ASSERT(this->m_InsertReadIo == 0);
    SDK_ASSERT(this->m_pSock);
    ev_io_init(&(this->m_EvReadIo),SdkServerAlarm::ReadIoCallBack,this->m_pSock->Socket(),EV_READ,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvReadIo));
    this->m_InsertReadIo = 1;
    return 0;
}

void SdkServerAlarm::__StopReadIo()
{
    if(this->m_InsertReadIo)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvReadIo));
    }
    this->m_InsertReadIo = 0;
    return ;
}

int SdkServerAlarm::__StartReadTimer()
{
    SDK_ASSERT(this->m_InsertReadTimer == 0);
    SDK_ASSERT(this->m_pSock);
    ev_timer_init(&(this->m_EvReadTimer),SdkServerAlarm::ReadTimerCallBack,3.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvReadTimer));
    this->m_InsertReadTimer = 1;
    return 0;
}

void SdkServerAlarm::__StopReadTimer()
{
    if(this->m_InsertReadTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvReadTimer));
    }
    this->m_InsertReadTimer = 0;
    return;
}

int SdkServerAlarm::__StartWriteIo()
{
    SDK_ASSERT(this->m_InsertWriteIo == 0);
    SDK_ASSERT(this->m_pSock);
    ev_io_init(&(this->m_EvWriteIo),SdkServerAlarm::WriteIoCallBack,this->m_pSock->Socket(),EV_WRITE,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvWriteIo));
    this->m_InsertWriteIo = 1;
    return 0;
}

void SdkServerAlarm::__StopWriteIo()
{
    if(this->m_InsertWriteIo)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvWriteIo));
    }
    this->m_InsertWriteIo = 0;
    return ;
}

int SdkServerAlarm::__StartWriteTimer()
{
    SDK_ASSERT(this->m_InsertWriteTimer == 0);
    SDK_ASSERT(this->m_pSock);
    ev_timer_init(&(this->m_EvWriteTimer),SdkServerAlarm::WriteTimerCallBack,3.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvWriteTimer));
    this->m_InsertWriteTimer = 1;
    return 0;
}

void SdkServerAlarm::__StopWriteTimer()
{
    if(this->m_InsertWriteTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvWriteTimer));
    }
    this->m_InsertWriteTimer = 0;
    return;
}

int SdkServerAlarm::__GetLPort()
{
    return GMI_SDK_S_WARING_PORT;
}

int SdkServerAlarm::__GetRPort()
{
    return GMI_SYS_SERVER_C_WARING_PORT;
}


int SdkServerAlarm::__SetSocketUnBlock(int sock)
{
    int ret;
    int flags;

    errno = 0;
    flags = fcntl(sock,F_GETFL);
    if(flags == -1 && errno)
    {
        ret = GETERRNO();
        SETERRNO(ret);
        return -ret;
    }

    ret = fcntl(sock,F_SETFL,flags | O_NONBLOCK);
    if(ret < 0)
    {
        ret = GETERRNO();
        SETERRNO(ret);
        return -ret;
    }
    return 0;
}


int SdkServerAlarm::__BindSocket()
{
    int ret;
    int sock=-1;
    struct sockaddr_in saddr;
    socklen_t socklen;

    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0)
    {
        ret = GETERRNO();
        goto fail;
    }

    ret  = this->__GetLPort();
    if(ret < 0)
    {
        ret = GETERRNO();
        goto fail;
    }
    this->m_LPort = ret;

    ret = this->__GetRPort();
    if(ret < 0)
    {
        ret = GETERRNO();
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
    if(ret < 0)
    {
        ret = GETERRNO();
        goto fail;
    }

    ret = this->__SetSocketUnBlock(sock);
    if(ret < 0)
    {
        ret = GETERRNO();
        goto fail;
    }

    SDK_ASSERT(this->m_pSock==NULL);
    this->m_pSock = new SdkClientSock(sock);
    sock = -1;

    return 0;

fail:
    if(sock >= 0)
    {
        close(sock);
    }
    sock = -1;
    SETERRNO(ret);
    return -ret;
}

int SdkServerAlarm::Start()
{
    int ret;
    this->Stop();
    if(this->m_pSvrMgmt == NULL)
    {
        ret = EINVAL;
        SETERRNO(ret);
        return -ret;
    }

    ret = this->__BindSocket();
    if(ret < 0)
    {
        this->Stop();
        SETERRNO(-ret);
        return ret;
    }

    /*now we should start read io*/
    ret = this->__StartReadIo();
    if(ret < 0)
    {
        this->Stop();
        SETERRNO(-ret);
        return ret;
    }

    SETERRNO(0);
    return 0;
}

void SdkServerAlarm::__BreakOut()
{
    ev_break(EV_DEFAULT,EVBREAK_ONE);
    return ;
}

int SdkServerAlarm::__PrepareReqAlarm(sdk_client_comm_t * pComm)
{
    int ret;
    int packlen=0;
    void* pPtr=NULL;
    SysPkgMessageCode msgcode;
    std::auto_ptr<SdkProtoPack> pPack2(new SdkProtoPack(SYSCODE_GET_ALARM_REQ,pComm->m_SesId,pComm->m_SeqId));
    SdkProtoPack* pPack = pPack2.get();
    sdk_client_comm_t *pRespComm=NULL;

    /*indicate success*/
    msgcode.s_MessageCode = HOST_TO_PROTO32(0);
    msgcode.s_MessageLen = HOST_TO_PROTO32(sizeof(msgcode));
    ret = pPack->AddItem(TYPE_MESSAGECODE,&msgcode,sizeof(msgcode));
    if(ret < 0)
    {
        ret = GETERRNO();
        goto fail;
    }

    packlen = pPack->GetLength();
    if(packlen < 0)
    {
        ret = GETERRNO();
        goto fail;
    }

    pRespComm = AllocateComm(packlen);
    if(pRespComm == NULL)
    {
        ret = GETERRNO();
        goto fail;
    }

    pRespComm->m_SesId = pComm->m_SesId;
    pRespComm->m_Priv = pComm->m_Priv;
    pRespComm->m_ServerPort = pComm->m_ServerPort ? pComm->m_ServerPort : this->m_RPort;
    pRespComm->m_LocalPort = this->m_LPort;
    pRespComm->m_SeqId = pComm->m_SeqId;
    pRespComm->m_Type = pComm->m_Type;

    pRespComm->m_Frag = 0;
    pRespComm->m_FHB = 0;
    pRespComm->m_DataId = 0;
    pRespComm->m_Offset = 0;
    pRespComm->m_DataLen = packlen;

    pPtr = pPack->GetPtr();
    if(pPtr == NULL)
    {
        ret = GETERRNO();
        goto fail;
    }

    memcpy(pRespComm->m_Data,pPtr,packlen);
    /*now all is copied and will insert into the buffer*/
    if(this->m_pSock->IsWriteSth())
    {
        /*we do not handle and timer for write ,because it will write for it self*/
        this->m_pReqCommVecs.push_back(pRespComm);
        pRespComm = NULL;
        return 0;
    }

    ret = this->m_pSock->PushData(pRespComm);
    if(ret < 0)
    {
        ret = GETERRNO();
        goto fail;
    }
    pRespComm = NULL;

    /*we have push it ,so we should modify the timer*/
    if(this->m_InsertWriteIo == 0)
    {
        ret = this->__StartWriteIo();
        if(ret < 0)
        {
            ret = GETERRNO();
            goto fail;
        }
    }

    if(this->m_InsertWriteTimer == 0)
    {
        ret=  this->__StartWriteTimer();
        if(ret < 0)
        {
            ret = GETERRNO();
            goto fail;
        }
    }
    return 0;
fail:
    FreeComm(pRespComm);
    SETERRNO(ret);
    return -ret;

}

int SdkServerAlarm::__ReadIoImpl()
{
    int ret;
    sdk_client_comm_t* pComm=NULL;

    /*when call SdkServerAlarm::__StartReadIo we have succeeded in binding socket, so this must check ok*/
    SDK_ASSERT(this->m_pSock);

    ret = this->m_pSock->Read();
    if(ret < 0)
    {
        SETERRNO(-ret);
        return ret;
    }
    else if(ret == 0)
    {
        return 0;
    }

    pComm = this->m_pSock->GetRead();
    SDK_ASSERT(pComm);
    this->m_pSock->ClearRead();

    ret= this->m_pSvrMgmt->PushAlarmInfo(pComm);
    SDK_ASSERT(pComm);
    if(ret < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("Could not Push AlarmInfo Error(%d)\n",ret);
        goto fail;
    }

    /*now we should prepare for the response for handling receive the pComm*/
    ret = this->__PrepareReqAlarm(pComm);
    if(ret < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("Could not PrepareReqAlarm Error(%d)\n",ret);
        goto fail;
    }

    /*all is ok*/
    FreeComm(pComm);
    return 0;
fail:
    SDK_ASSERT(ret > 0);
    FreeComm(pComm);
    SETERRNO(ret);
    return -ret;
}

void SdkServerAlarm::ReadIoCallBack(EV_P_ ev_io * w,int revents,void * arg)
{
    SdkServerAlarm* pThis = (SdkServerAlarm*)arg;
    int ret;

    ret = pThis->__ReadIoImpl();
    if(ret < 0)
    {
        BACK_TRACE_FMT("<0x%p>SdkServerAlarm Read Error(%d)",pThis,ret);
        pThis->__BreakOut();
    }
    return ;
}

void SdkServerAlarm::ReadTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkServerAlarm* pThis = (SdkServerAlarm*)arg;
    BACK_TRACE_FMT("<0x%p>SdkServerAlarm Read Timerout",pThis);
    pThis->__BreakOut();
    return ;
}

int SdkServerAlarm::__WriteIoImpl()
{
    int ret;
    sdk_client_comm_t* pComm=NULL;

    SDK_ASSERT(this->m_pSock);
    ret = this->m_pSock->Write();
    if(ret < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("<0x%p> Write Error(%d)\n",this,ret);
        return -ret;
    }
    else if(ret == 0)
    {
        return 0;
    }

    /*first stop the timer ,and io ,and then test for if we have something to write*/
    this->__StopWriteIo();
    this->__StopWriteTimer();

    if(this->m_pReqCommVecs.size() > 0)
    {
        pComm = this->m_pReqCommVecs[0];
        this->m_pReqCommVecs.erase(this->m_pReqCommVecs.begin());
        ret = this->m_pSock->PushData(pComm);
        if(ret < 0)
        {
            ret = GETERRNO();
            ERROR_INFO("<0x%p> PushData Error(%d)\n",this,ret);
            goto fail;
        }
        pComm = NULL;

        ret = this->__StartWriteIo();
        if(ret < 0)
        {
            ret = GETERRNO();
            ERROR_INFO("<0x%p> StartWriteIo Error(%d)\n",this,ret);
            goto fail;
        }

        ret = this->__StartWriteTimer();
        if(ret < 0)
        {
            ret = GETERRNO();
            ERROR_INFO("<0x%p> StartWriteTimer Error(%d)\n",this,ret);
            goto fail;
        }

    }

    return 0;
fail:
    SDK_ASSERT(ret > 0);
    FreeComm(pComm);
    SETERRNO(ret);
    return -ret;
}

void SdkServerAlarm::WriteIoCallBack(EV_P_ ev_io * w,int revents,void * arg)
{
    SdkServerAlarm* pThis = (SdkServerAlarm*)arg;
    int ret;

    ret = pThis->__WriteIoImpl();
    if(ret < 0)
    {
        BACK_TRACE_FMT("<0x%p>SdkServerAlarm Write Error(%d)",pThis,ret);
        pThis->__BreakOut();
    }
    return ;
}

void SdkServerAlarm::WriteTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkServerAlarm* pThis = (SdkServerAlarm*)arg;
    BACK_TRACE_FMT("<0x%p>SdkServerAlarm Write Timerout",pThis);
    pThis->__BreakOut();
    return ;
}


int SdkServerAlarm::ResetLongTimeTimer()
{
    int ret;
    if(this->m_InsertReadTimer)
    {
        this->__StopReadTimer();
        ret = this->__StartReadTimer();
        if(ret < 0)
        {
            ret = GETERRNO();
            SETERRNO(ret);
            return -ret;
        }
    }

    if(this->m_InsertWriteTimer)
    {
        this->__StopWriteTimer();
        ret = this->__StartWriteTimer();
        if(ret < 0)
        {
            ret = GETERRNO();
            SETERRNO(ret);
            return -ret;
        }
    }

    return 0;
}
