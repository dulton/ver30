
#include <sdk_server_sys_req.h>
#include <gmi_config_api.h>
#include <sdk_server_mgmt.h>
#include <sdk_server_debug.h>
#include <ipc_fw_v3.x_resource.h>


SdkServerSysReq::SdkServerSysReq(SdkServerMgmt * pSvrMgmt,int * pRunningBits)
    : m_pSvrMgmt(pSvrMgmt),
      m_pRunningBits(pRunningBits)
{
    m_pSock = NULL;
    m_LPort = -1;
    m_RPort = -1;
    m_InsertReadIo = 0;
    m_InsertReadTimer = 0;
    m_InsertWriteIo = 0;
    m_InsertWriteTimer = 0;

    SDK_ASSERT(m_pSendComm.size()==0);
}

void SdkServerSysReq::__FreeSendComms()
{
    while(this->m_pSendComm.size() >0)
    {
        sdk_client_comm_t* pComm = this->m_pSendComm[0];
        this->m_pSendComm.erase(this->m_pSendComm.begin());
        free(pComm);
    }

    return ;
}


void SdkServerSysReq::Stop()
{
    if(this->m_pSock)
    {
        delete this->m_pSock;
    }
    this->m_pSock = NULL;
    this->m_LPort = -1;
    this->m_RPort = -1;
    this->__FreeSendComms();
    this->__StopReadIo();
    this->__StopReadTimer();
    this->__StopWriteIo();
    this->__StopWriteTimer();
    return ;
}

SdkServerSysReq::~SdkServerSysReq()
{
    this->Stop();
    this->m_pSvrMgmt = NULL;
    this->m_pRunningBits = NULL;
}




int SdkServerSysReq::__GetRPort()
{
    GMI_RESULT gmiret;
    FD_HANDLE xmlhd=NULL;
    int serverport;

    gmiret = GMI_XmlOpen(GMI_RESOURCE_XML,&xmlhd);
    if(gmiret != GMI_SUCCESS)
    {
        goto set_default;
    }
    gmiret = GMI_XmlRead(xmlhd,GMI_SYS_SDK_PORT_PATH,
                         GMI_SYS_SERVER_TO_SDK_REQ_PORT_ITEM ,
                         GMI_CONTROL_S_PORT,
                         &serverport,GMI_CONFIG_READ_ONLY);
    if(gmiret != GMI_SUCCESS)
    {
        goto set_default;
    }
    GMI_XmlFileSave(xmlhd);
    xmlhd = NULL;

    return serverport;


set_default:
    if(xmlhd)
    {
        GMI_XmlFileSave(xmlhd);
    }
    xmlhd= NULL;
    return GMI_CONTROL_S_PORT;
}


int SdkServerSysReq::__GetLPort()
{
    GMI_RESULT gmiret;
    FD_HANDLE xmlhd=NULL;
    int serverport;

    gmiret = GMI_XmlOpen(GMI_RESOURCE_XML,&xmlhd);
    if(gmiret != GMI_SUCCESS)
    {
        goto set_default;
    }
    gmiret = GMI_XmlRead(xmlhd,GMI_SYS_SDK_PORT_PATH,
                         GMI_SDK_TO_SYS_SERVER_REQ_PORT_ITEM,
                         SDK_TO_SYS_SERVER_REQ_PORT,
                         &serverport,GMI_CONFIG_READ_ONLY);
    if(gmiret != GMI_SUCCESS)
    {
        goto set_default;
    }
    GMI_XmlFileSave(xmlhd);
    xmlhd = NULL;

    return serverport;


set_default:
    if(xmlhd)
    {
        GMI_XmlFileSave(xmlhd);
    }
    xmlhd= NULL;
    return SDK_TO_SYS_SERVER_REQ_PORT;
}



int SdkServerSysReq::__SetSocketUnBlock(int sock)
{
    int ret;
    int flags;

    errno = 0;
    flags = fcntl(sock,F_GETFL);
    if(flags == -1 && errno)
    {
        ret = -errno;
        return ret;
    }

    ret = fcntl(sock,F_SETFL,flags | O_NONBLOCK);
    if(ret < 0)
    {
        ret = -errno ?   -errno :  -1;
        return ret;
    }
    return 0;
}


int SdkServerSysReq::__BindPort()
{
    int ret;
    int sock=-1;
    struct sockaddr_in saddr;
    socklen_t socklen;


    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0)
    {
        ret = -errno ? -errno : -1;
        goto fail;
    }

    ret  = this->__GetLPort();
    if(ret < 0)
    {
        goto fail;
    }
    this->m_LPort = ret;

    ret = this->__GetRPort();
    if(ret < 0)
    {
        goto fail;
    }
    this->m_RPort = ret;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    saddr.sin_port = htons(this->m_LPort);

    socklen = sizeof(saddr);
    ret = bind(sock,(struct sockaddr*)&saddr,socklen);
    if(ret < 0)
    {
        goto fail;
    }

    ret = this->__SetSocketUnBlock(sock);
    if(ret < 0)
    {
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
    return ret;
}


void SdkServerSysReq::__StopReadIo()
{
    if(this->m_InsertReadIo)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvReadIo));
    }
    this->m_InsertReadIo = 0;
    return;
}

int SdkServerSysReq::__StartReadIo()
{
    SDK_ASSERT(this->m_InsertReadIo == 0);
    SDK_ASSERT(this->m_pSock);
    ev_io_init(&(this->m_EvReadIo),SdkServerSysReq::ReadIoCallBack,this->m_pSock->Socket(),EV_READ,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvReadIo));
    this->m_InsertReadIo = 1;
    return 0;
}

void SdkServerSysReq::__StopReadTimer()
{
    if(this->m_InsertReadTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvReadTimer));
    }
    this->m_InsertReadTimer = 0;
    return ;
}

int SdkServerSysReq::__StartReadTimer()
{
    SDK_ASSERT(this->m_InsertReadTimer == 0);
    ev_timer_init(&(this->m_EvReadTimer),SdkServerSysReq::ReadTimerCallBack,3.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvReadTimer));
    this->m_InsertReadTimer = 1;
    return 0;
}

void SdkServerSysReq::__StopWriteIo()
{
    if(this->m_InsertWriteIo)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvWriteIo));
    }
    this->m_InsertWriteIo = 0;
    return ;
}

int SdkServerSysReq::__StartWriteIo()
{
    SDK_ASSERT(this->m_InsertWriteIo == 0);
    SDK_ASSERT(this->m_pSock);

    ev_io_init(&(this->m_EvWriteIo),SdkServerSysReq::WriteIoCallBack,this->m_pSock->Socket(),EV_WRITE,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvWriteIo));
    this->m_InsertWriteIo = 1;
    return 0;
}


void SdkServerSysReq::__StopWriteTimer()
{
    if(this->m_InsertWriteTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvWriteTimer));
    }
    this->m_InsertWriteTimer = 0;
    return ;
}

int SdkServerSysReq::__StartWriteTimer()
{
    SDK_ASSERT(this->m_InsertWriteTimer == 0);
    ev_timer_init(&(this->m_EvWriteTimer),SdkServerSysReq::WriteTimerCallBack,3.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvWriteTimer));
    this->m_InsertWriteTimer = 1;
    return 0;
}


void SdkServerSysReq::__BreakOut()
{
	BACK_TRACE();
	ev_break(EV_DEFAULT,EVBREAK_ONE);
}


void SdkServerSysReq::ReadTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkServerSysReq* pThis = (SdkServerSysReq*)arg;
    pThis->__BreakOut();
    return ;
}

void SdkServerSysReq::WriteTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkServerSysReq* pThis = (SdkServerSysReq*)arg;
    pThis->__BreakOut();
    return ;
}

int SdkServerSysReq::__ReadIoImpl()
{
    int ret;
    sdk_client_comm_t* pComm=NULL;

    ret = this->m_pSock->Read();
    if(ret < 0)
    {
        return ret;
    }
    else if(ret == 0)
    {
        /*we start timer to read ,this will give the things ok,when time out of read one packet */
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

    SDK_ASSERT(ret > 0);
    /*we have alread read all the timer things ,so we should read all the timer*/
    this->__StopReadTimer();
    pComm = this->m_pSock->GetRead();
    SDK_ASSERT(pComm);

    DEBUG_BUFFER_FMT(pComm->m_Data,pComm->m_DataLen,"read sessionid %d reqnum %d datalen %d\n",pComm->m_SesId,pComm->m_SeqId,pComm->m_DataLen);
    /*now we should reset the read it should be less*/
    /*we should give the ok for configuration*/
    pComm->m_Type = GMIS_PROTOCOL_TYPE_CONF;
    ret = this->m_pSvrMgmt->PushSysResp(pComm);
    if(ret <= 0)
    {
		/*
		   becaue ,if we have negative return it maybe remove the pComm ,so we do not make SDK_ASSERT
		   SDK_ASSERT(pComm);
		*/
		FreeComm(pComm);
        /*we could not find this one ,so just pretend things are ok*/
        return 0;
    }

    SDK_ASSERT(pComm == NULL);
    /*we have handle 1 packet*/
    return 1;
}


void SdkServerSysReq::ReadIoCallBack(EV_P_ ev_io * w,int revents,void * arg)
{
    SdkServerSysReq* pThis = (SdkServerSysReq*)arg;
    int ret;
    ret = pThis->__ReadIoImpl();
    if(ret < 0)
    {
        pThis->__BreakOut();
    }
    return ;
}

int SdkServerSysReq::__WriteIoImpl()
{
    int ret;
    int haswrite=0;
    sdk_client_comm_t* pComm=NULL;

try_again:
    SDK_ASSERT(pComm == NULL);
    ret = this->m_pSock->IsWriteSth();
    if(ret > 0)
    {
        ret = this->m_pSock->Write();
        if(ret < 0)
        {
            return ret;
        }
        else if(ret == 0)
        {
            if(haswrite > 0)
            {
                ret = this->__ResetWriteTimer();
                if(ret < 0)
                {
                    return ret;
                }
            }
            return haswrite;
        }

        SDK_ASSERT(ret > 0);
        haswrite += 1;
        this->m_pSock->ClearWrite();
    }

    if(this->m_pSendComm.size() > 0)
    {
        pComm = this->m_pSendComm[0];
        pComm->m_ServerPort = this->m_RPort;
        pComm->m_LocalPort = this->m_LPort;
        ret = this->m_pSock->PushData(pComm);
        if(ret < 0)
        {
            /*we can not push the data ,so we do not to free pComm*/
            return ret;
        }

        this->m_pSendComm.erase(this->m_pSendComm.begin());
        /*we assign the pComm into the m_pSock*/
        pComm = NULL;
        goto try_again;
    }

    this->__StopWriteIo();
    this->__StopWriteTimer();
    return haswrite;
}

void SdkServerSysReq::WriteIoCallBack(EV_P_ ev_io * w,int revents,void * arg)
{
    SdkServerSysReq* pThis = (SdkServerSysReq*)arg;
    int ret;
    ret = pThis->__WriteIoImpl();
    if(ret < 0)
    {
        pThis->__BreakOut();
    }
    return ;
}

int SdkServerSysReq::Start()
{
    int ret;
    this->Stop();
    ret = this->__BindPort();
    if(ret < 0)
    {
    	ERROR_INFO("\n");
        this->Stop();
        return ret;
    }

    ret = this->__StartReadIo();
    if(ret < 0)
    {
    	ERROR_INFO("\n");
        this->Stop();
        return ret;
    }

    return 0;
}



int SdkServerSysReq::AddRequest(sdk_client_comm_t*& pComm)
{
    int ret;
    /*now we should put the server port into the data*/
    if(pComm == NULL)
    {
        return -EINVAL;
    }

	ret = this->__SplitFragPackets(pComm);
	if (ret < 0)
	{
		return ret;
	}
	else if (ret > 0)
	{
		/*has splitted ,so just return */
		return 0;
	}
    //DEBUG_BUFFER(pComm->m_Data,pComm->m_DataLen);

    /*we add the server port*/
    ret = this->m_pSock->IsWriteSth();
    if(ret > 0 || this->m_pSendComm.size() > 0)
    {
        /*it means something is already in writing ,just push into the vector*/
        this->m_pSendComm.push_back(pComm);
    }
    else
    {
        /*this means that we need to start write io and timer*/
        ret = this->__StartWriteIo();
        if(ret < 0)
        {
            return ret;
        }
        ret = this->__StartWriteTimer();
        if(ret < 0)
        {
            this->__StopWriteIo();
            return ret;
        }
        pComm->m_ServerPort = this->m_RPort;
        pComm->m_LocalPort = this->m_LPort;
        ret = this->m_pSock->PushData(pComm);
        if(ret < 0)
        {
            this->__StopWriteIo();
            this->__StopWriteTimer();
            return ret;
        }
    }

    /*not set this one*/
    pComm = NULL;

    return 0;
}

int SdkServerSysReq::__ResetWriteTimer()
{
    this->__StopWriteTimer();
    return this->__StartWriteTimer();
}


int SdkServerSysReq::__SplitFragPackets(sdk_client_comm_t * & pComm)
{
    std::vector<sdk_client_comm_t*> splitcomms;
    int ret;
    unsigned int i;
    sdk_client_comm_t* pCurComm=NULL;
    unsigned int numalloc=0;
    unsigned int copiedlen=0,curcopy;
    unsigned int insertsock = 0,insertsending=0;
    if(pComm->m_DataLen > MAX_CLIENT_COMM_SIZE)
    {
        return -EINVAL;
    }

    if(pComm->m_DataLen <= (CLIENT_COMM_PACK_SIZE))
    {
        return 0;
    }

    numalloc = (pComm->m_DataLen + CLIENT_COMM_PACK_SIZE - 1) / CLIENT_COMM_PACK_SIZE;
    copiedlen = 0;
    for(i=0; i<numalloc; i++)
    {
        SDK_ASSERT(pCurComm == NULL);
		SDK_ASSERT(copiedlen < pComm->m_DataLen);
        curcopy = CLIENT_COMM_PACK_SIZE;
        if(curcopy > (pComm->m_DataLen - copiedlen))
        {
            curcopy = pComm->m_DataLen - copiedlen;
        }
		pCurComm = AllocateComm(curcopy);
        if(pCurComm == NULL)
        {
            ret = -errno ? -errno : -1;
            goto fail;
        }
        pCurComm->m_SesId = pComm->m_SesId;
        pCurComm->m_Priv = pComm->m_Priv;
        pCurComm->m_ServerPort = this->m_RPort;
        pCurComm->m_LocalPort = this->m_LPort;
        pCurComm->m_SeqId = pComm->m_SeqId;
        pCurComm->m_Type = pComm->m_Type;
        pCurComm->m_FHB = 0;
        pCurComm->m_Frag = 1;
        pCurComm->m_DataId = i;
        pCurComm->m_Offset = copiedlen;
        pCurComm->m_Totalsize = pComm->m_DataLen;
        memcpy(pCurComm->m_Data,pComm->m_Data + copiedlen,curcopy);
		pCurComm->m_DataLen = curcopy;
		DEBUG_BUFFER_FMT(pCurComm->m_Data,pCurComm->m_DataLen,"At[%d] offset[%d]:",i,copiedlen);
        copiedlen += curcopy;
        splitcomms.push_back(pCurComm);
        pCurComm = NULL;
    }

    /*now we should insert into the comm*/
    while(splitcomms.size()>0)
    {
        pCurComm = splitcomms[0];
        splitcomms.erase(splitcomms.begin());
        if(this->m_pSendComm.size() == 0 && this->m_pSock->IsWriteSth() == 0)
        {
            ret = this->__StartWriteIo();
            if(ret < 0)
            {
                goto fail;
            }
            ret = this->__StartWriteTimer();
            if(ret < 0)
            {
                this->__StopWriteIo();
                goto fail;
            }

            ret = this->m_pSock->PushData(pCurComm);
            if(ret < 0)
            {
				this->__StopWriteIo();
				this->__StopWriteTimer();
                goto fail;
            }
            pCurComm = NULL;
            insertsock = 1;
        }
		else
		{
			this->m_pSendComm.push_back(pCurComm);
			insertsending += 1;
			pCurComm = NULL;
		}
    }

	/*now it is safe to free received comm*/
	FreeComm(pComm);

    SDK_ASSERT(splitcomms.size() == 0);

    return 1;

fail:
    if(insertsock)
    {
        this->__StopWriteIo();
        this->__StopWriteTimer();
        this->m_pSock->ClearWrite();
    }
    insertsock = 0;

    if(insertsending)
    {
        SDK_ASSERT(this->m_pSendComm.size() >= insertsending);
        for(i=0; i<insertsending; i++)
        {
			int lastindx = this->m_pSendComm.size();
			sdk_client_comm_t* pRemoved=NULL;
			SDK_ASSERT(lastindx > 0);
			lastindx --;
			pRemoved = this->m_pSendComm[lastindx];
			this->m_pSendComm.erase(this->m_pSendComm.begin() + lastindx);
			free(pRemoved);
        }
    }
    insertsending = 0;
    while(splitcomms.size() > 0)
    {
        pCurComm = splitcomms[0];
        free(pCurComm);
        splitcomms.erase(splitcomms.begin());
    }
    SDK_ASSERT(splitcomms.size() == 0);

	FreeComm(pCurComm);
    return ret;
}


int SdkServerSysReq::ResetLongTimeTimer(void)
{
	int ret;

	if (this->m_InsertReadTimer > 0)
	{
		this->__StopReadTimer();
		ret = this->__StartReadTimer();
		if (ret < 0)
		{
			return ret;
		}
	}

	if (this->m_InsertWriteTimer > 0)
	{
		this->__StopWriteTimer();
		ret = this->__StartWriteTimer();
		if (ret < 0)
		{
			return ret;
		}
	}

	return 0;
}
