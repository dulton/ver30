
#include <sdk_server_sys_comm.h>
#include <gmi_config_api.h>
#include <sdk_server_mgmt.h>
#include <sdk_server_debug.h>




SdkServerSysComm::SdkServerSysComm(SdkServerMgmt * pSvrMmgt,int * pRunningBits)
    : m_pSvrMgmt(pSvrMmgt),
      m_pRunningBits(pRunningBits)
{
    m_pSock = NULL;
    m_LPort = -1;
    m_RPort = -1;
    m_InsertReadIo = 0;
    m_InsertReadTimer = 0;
    m_InsertWriteIo = 0;
    m_InsertWriteTimer = 0;
    SDK_ASSERT(m_pResponse.size()==0);
}

void SdkServerSysComm::Stop()
{
    if(this->m_pSock)
    {
        delete this->m_pSock;
    }
    this->m_pSock = NULL;
    this->__StopReadIo();
    this->__StopReadTimer();
    this->__StopWriteIo();
    this->__StopWriteTimer();
    this->m_LPort = -1;
    this->m_RPort = -1;
    return ;
}

SdkServerSysComm::~SdkServerSysComm()
{
    this->Stop();
    this->m_pSvrMgmt = NULL;
    this->m_pRunningBits = NULL;
}


int SdkServerSysComm::__GetRPort()
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
                         GMI_SYS_SERVER_TO_SDK_PORT_ITEM ,
                         SYS_SERVER_TO_SDK_PORT,
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
    return SYS_SERVER_TO_SDK_PORT;
}

int SdkServerSysComm::__GetLPort()
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
                         GMI_SDK_TO_SYS_SERVER_PORT_ITEM,
                         SDK_TO_SYS_SERVER_PORT,
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
    return SDK_TO_SYS_SERVER_PORT;
}

int SdkServerSysComm::__SetSocketUnBlock(int sock)
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


int SdkServerSysComm::__BindSocket()
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

void SdkServerSysComm::__StopReadIo()
{
    if(this->m_InsertReadIo)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvReadIo));
    }
    this->m_InsertReadIo = 0;
    return;
}

int SdkServerSysComm::__StartReadIo()
{
    SDK_ASSERT(this->m_InsertReadIo == 0);
    SDK_ASSERT(this->m_pSock);
    ev_io_init(&(this->m_EvReadIo),SdkServerSysComm::ReadIoCallBack,this->m_pSock->Socket(),EV_READ,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvReadIo));
    this->m_InsertReadIo = 1;
    return 0;
}

void SdkServerSysComm::__StopReadTimer()
{
    if(this->m_InsertReadTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvReadTimer));
    }
    this->m_InsertReadTimer = 0;
    return ;
}

int SdkServerSysComm::__StartReadTimer()
{
    SDK_ASSERT(this->m_InsertReadTimer == 0);
    ev_timer_init(&(this->m_EvReadTimer),SdkServerSysComm::ReadTimerCallBack,3.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvReadTimer));
    this->m_InsertReadTimer = 1;
    return 0;
}

void SdkServerSysComm::__StopWriteIo()
{
    if(this->m_InsertWriteIo)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvWriteIo));
    }
    this->m_InsertWriteIo = 0;
    return ;
}

int SdkServerSysComm::__StartWriteIo()
{
    SDK_ASSERT(this->m_InsertWriteIo == 0);
    SDK_ASSERT(this->m_pSock);

    ev_io_init(&(this->m_EvWriteIo),SdkServerSysComm::WriteIoCallBack,this->m_pSock->Socket(),EV_WRITE,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvWriteIo));
    this->m_InsertWriteIo = 1;
    return 0;
}


void SdkServerSysComm::__StopWriteTimer()
{
    if(this->m_InsertWriteTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvWriteTimer));
    }
    this->m_InsertWriteTimer = 0;
    return ;
}

int SdkServerSysComm::__StartWriteTimer()
{
    SDK_ASSERT(this->m_InsertWriteTimer == 0);
    ev_timer_init(&(this->m_EvWriteTimer),SdkServerSysComm::WriteTimerCallBack,3.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvWriteTimer));
    this->m_InsertWriteTimer = 1;
    return 0;
}

void SdkServerSysComm::__BreakOut()
{
    BACK_TRACE();
    ev_break(EV_DEFAULT,EVBREAK_ONE);
}

int SdkServerSysComm::__ResponseCode(uint32_t opcode,int err,sdk_client_comm_t* pReqComm)
{
    sdk_client_comm_t* pComm=NULL;
    sys_stream_response_t* pResponse=NULL;
    int ret;

    pComm = AllocateComm(sizeof(*pResponse));
    if(pComm == NULL)
    {
        return -ENOMEM;
    }

    pComm->m_SesId = pReqComm->m_SesId;
    pComm->m_Priv = pReqComm->m_Priv;
    pComm->m_ServerPort = pReqComm->m_ServerPort;
    pComm->m_LocalPort = this->m_LPort;
    pComm->m_SeqId = pReqComm->m_SeqId;
    /*for reply of this*/
    pComm->m_Type = GMIS_PROTOCOL_TYPE_SDK_TO_SYS;
    pComm->m_Frag = 0;
    pComm->m_DataId = 0;
    pComm->m_Offset = 0;
    pComm->m_Totalsize = 0;
    pComm->m_DataLen = sizeof(*pResponse);
    pResponse = (sys_stream_response_t*)pComm->m_Data;

    pResponse->m_OpCode = INNER_HOST_TO_PROTO32(opcode);
    pResponse->m_Result = INNER_HOST_TO_PROTO32(err);

    ret = this->__InsertRequestComm(pComm);
    if(ret < 0)
    {
        goto fail;
    }
    SDK_ASSERT(pComm == NULL);

    return 0;
fail:
    FreeComm(pComm);
    return ret;
}

int SdkServerSysComm::__InsertRequestComm(sdk_client_comm_t * & pComm)
{
    int ret;
    if(this->m_pSock->IsWriteSth() || this->m_pResponse.size() > 0)
    {
        /*we just put for the later caller*/
        this->m_pResponse.push_back(pComm);
        pComm = NULL;
    }
    else
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
        if(pComm->m_ServerPort == 0)
        {
            pComm->m_ServerPort = this->m_RPort;
        }
        pComm->m_LocalPort = this->m_LPort;
        ret = this->m_pSock->PushData(pComm);
        if(ret < 0)
        {
            DEBUG_INFO("\n");
            this->__StopWriteIo();
            this->__StopWriteTimer();
            goto fail;
        }
        pComm = NULL;

    }
    return 1;
fail:
    return ret;
}


int SdkServerSysComm::__ProtoToHostRequest(sys_stream_request_t * pRequest,int datalen)
{
    sys_stream_info_t *pInfo=&(pRequest->m_StreamInfos);
    SysPkgEncodeCfg *pCfg;
    SysPkgAudioEncodeCfg *pAudioCfg;
    unsigned int i;

    if(datalen < (int)sizeof(*pRequest))
    {
        return -EINVAL;
    }

    pRequest->m_OpCode = INNER_PROTO_TO_HOST32(pRequest->m_OpCode);
    pInfo->m_Count = INNER_PROTO_TO_HOST16(pInfo->m_Count);
    pInfo->m_AudioCount= INNER_PROTO_TO_HOST16(pInfo->m_AudioCount);

    if(pInfo->m_Count > MAX_STREAM_IDS ||datalen < (int)(sizeof(*pRequest)))
    {
        return -EINVAL;
    }

    if(pInfo->m_AudioCount > (sizeof(pRequest->m_StreamInfos.m_AudioInfo)/sizeof(pRequest->m_StreamInfos.m_AudioInfo[0])))
    {
        return -EINVAL;
    }

    for(i=0; i<pInfo->m_Count ; i ++)
    {
        pCfg = &(pInfo->m_VideoInfo[i]);
        pCfg->s_VideoId = INNER_PROTO_TO_HOST32(pCfg->s_VideoId);
        pCfg->s_Compression = INNER_PROTO_TO_HOST32(pCfg->s_Compression);
        //pCfg->s_Resolution = PROTO_TO_HOST32(pCfg->s_Resolution);
        pCfg->s_BitrateCtrl = INNER_PROTO_TO_HOST32(pCfg->s_BitrateCtrl);
        pCfg->s_Quality = INNER_PROTO_TO_HOST32(pCfg->s_Quality);
        pCfg->s_FPS = INNER_PROTO_TO_HOST32(pCfg->s_FPS);
        pCfg->s_BitRateAverage = INNER_PROTO_TO_HOST32(pCfg->s_BitRateAverage);
        pCfg->s_BitRateUp = INNER_PROTO_TO_HOST32(pCfg->s_BitRateUp);
        pCfg->s_BitRateDown = INNER_PROTO_TO_HOST32(pCfg->s_BitRateDown);
        pCfg->s_Gop = INNER_PROTO_TO_HOST32(pCfg->s_Gop);
        pCfg->s_Rotate = INNER_PROTO_TO_HOST32(pCfg->s_Rotate);
        pCfg->s_Flag = INNER_PROTO_TO_HOST32(pCfg->s_Flag);
    }

    for(i=0; i<pInfo->m_AudioCount; i++)
    {
        pAudioCfg = &(pInfo->m_AudioInfo[i]);
        pAudioCfg->s_AudioId = INNER_PROTO_TO_HOST32(pAudioCfg->s_AudioId);
        pAudioCfg->s_SamplesPerSec = INNER_PROTO_TO_HOST32(pAudioCfg->s_SamplesPerSec);
        pAudioCfg->s_CapVolume = INNER_PROTO_TO_HOST16(pAudioCfg->s_CapVolume);
        pAudioCfg->s_PlayVolume = INNER_PROTO_TO_HOST16(pAudioCfg->s_PlayVolume);
    }

    return 0;
}


int SdkServerSysComm::__HandleStartAllStream(uint32_t opcode,sys_stream_request_t * pRequest,int datalen)
{
    int ret;

    ret = this->__ProtoToHostRequest(pRequest,datalen);
    if(ret < 0)
    {
        return ret;
    }

    ret = this->m_pSvrMgmt->StartAllStreams(&(pRequest->m_StreamInfos));
    if(ret < 0)
    {
        return ret;
    }

    return 0;
}

int SdkServerSysComm::__HandleStopAllStream(uint32_t opcode,sys_stream_request_t * pRequest,int datalen)
{
    int ret;
    ret = this->m_pSvrMgmt->StopAllStreams();
    if(ret < 0)
    {
        return ret;
    }
    return 0;
}

int SdkServerSysComm::__HandlePauseAllStream(uint32_t opcode,sys_stream_request_t * pRequest,int datalen)
{
    int ret;
    ret = this->m_pSvrMgmt->PauseAllStreams();
    if(ret < 0)
    {
        return ret;
    }
    return 0;
}

int SdkServerSysComm::__HandleResumeAllStream(uint32_t opcode,sys_stream_request_t * pRequest,int datalen)
{
    int ret;

    ret = this->__ProtoToHostRequest(pRequest,datalen);
    if(ret < 0)
    {
        return ret;
    }

    ret = this->m_pSvrMgmt->ResumeAllStreams(&(pRequest->m_StreamInfos));
    if(ret < 0)
    {
        return ret;
    }
    return 0;
}

int SdkServerSysComm::__HandleQueryStreamState(uint32_t opcode,sys_stream_request_t * pRequest,int datalen)
{
    return  this->m_pSvrMgmt->QueryStreamStarted();
}

int SdkServerSysComm::__HandleStreamRequest(sdk_client_comm_t * & pComm)
{
    int ret,res;
    sys_stream_request_t * pRequest=NULL;
    uint32_t h32;
    if(pComm->m_DataLen < sizeof(*pRequest))
    {
        /*it is the smallest data*/
        return 0;
    }
    pRequest = (sys_stream_request_t*)pComm->m_Data;
    h32 = INNER_PROTO_TO_HOST32(pRequest->m_OpCode);
    switch(h32)
    {
    case STREAM_START_OPCODE_REQ:
        ret = this->__HandleStartAllStream(h32,pRequest,pComm->m_DataLen);
        break;
    case STREAM_STOP_OPCODE_REQ:
        ret = this->__HandleStopAllStream(h32,pRequest,pComm->m_DataLen);
        break;
    case STREAM_PAUSE_OPCODE_REQ:
        ret = this->__HandlePauseAllStream(h32,pRequest,pComm->m_DataLen);
        break;
    case STREAM_RESUME_OPCODE_REQ:
        ret = this->__HandleResumeAllStream(h32,pRequest,pComm->m_DataLen);
        break;
    case STREAM_QUERY_OPCODE_REQ:
        ret = this->__HandleQueryStreamState(h32,pRequest,pComm->m_DataLen);
        break;
    default:
        SDK_ASSERT(0!=0);
        ret = -ENOTSUP;
        break;
    }

    if(ret >= 0)
    {
        ret = this->__ResponseCode(h32,ret,pComm);
        if(ret < 0)
        {
            goto fail;
        }
    }
    else
    {
        res = this->__ResponseCode(h32,-ret,pComm);
        if(res < 0)
        {
            ret = res;
            goto fail;
        }
    }

    FreeComm(pComm);

    return 1;
fail:
    return ret;
}





int SdkServerSysComm::__ReadIoImpl()
{
    int ret;
    sdk_client_comm_t*pComm=NULL;
    sys_stream_request_t * pRequest=NULL;
    uint32_t h32;

    ret= this->m_pSock->Read();
    if(ret < 0)
    {
        return ret;
    }
    else if(ret == 0)
    {
        /*we just get the start of the timer for it will give the time out*/
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

    /*we already read all the things ,so we should stop timer*/
    this->__StopReadTimer();
    pComm = this->m_pSock->GetRead();
    SDK_ASSERT(pComm);
    /*now to test for the read*/
    if(pComm->m_DataLen < sizeof(pRequest->m_OpCode))
    {
        ERROR_INFO("datalen %d < sizeof(%d)\n",pComm->m_DataLen,
                   sizeof(pRequest->m_OpCode));
        FreeComm(pComm);
        return 0;
    }
    pRequest = (sys_stream_request_t*)pComm->m_Data;
    h32 = INNER_PROTO_TO_HOST32(pRequest->m_OpCode);
    switch(h32)
    {
    case STREAM_START_OPCODE_REQ:
    case STREAM_STOP_OPCODE_REQ:
    case STREAM_PAUSE_OPCODE_REQ:
    case STREAM_RESUME_OPCODE_REQ:
    case STREAM_QUERY_OPCODE_REQ:
        ret = this->__HandleStreamRequest(pComm);
        break;
    default:
        ERROR_INFO("unsupported %d(0x%08x)\n",h32,h32);
        ret = 0;
        break;
    }

    if(ret < 0)
    {
        goto fail;
    }

    FreeComm(pComm);
    return 1;
fail:
    FreeComm(pComm);
    return ret;
}

void SdkServerSysComm::ReadIoCallBack(EV_P_ ev_io * w,int revents,void * arg)
{
    SdkServerSysComm* pThis = (SdkServerSysComm*)arg;
    int ret;
    ret = pThis->__ReadIoImpl();
    if(ret < 0)
    {
        pThis->__BreakOut();
    }
    return ;
}

int SdkServerSysComm::__WriteIoImpl()
{
    int ret;
    int writepacket=0;
try_again:
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
            if(writepacket > 0)
            {
                this->__StopWriteTimer();
                ret = this->__StartWriteTimer();
                if(ret < 0)
                {
                    return ret;
                }
            }
            return 0;
        }
        SDK_ASSERT(ret > 0);
        this->m_pSock->ClearWrite();
        writepacket += 1;

        /*passdown for next write*/
    }

	/*now we put end of timer ,and end of io ,if we have something to write ,we will give it ok*/
    this->__StopWriteIo();
    this->__StopWriteTimer();
	
    if(this->m_pResponse.size() > 0)
    {
        sdk_client_comm_t* pComm=NULL;
        pComm = this->m_pResponse[0];
        this->m_pResponse.erase(this->m_pResponse.begin());
		if (pComm->m_ServerPort == 0)
		{
        	pComm->m_ServerPort = this->m_RPort;
		}
        pComm->m_LocalPort  = this->m_LPort;
        ret = this->m_pSock->PushData(pComm);
        if(ret < 0)
        {
            free(pComm);
            return ret;
        }

        pComm = NULL;
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
        goto try_again;
    }

    return writepacket;
}

void SdkServerSysComm::WriteIoCallBack(EV_P_ ev_io * w,int revents,void * arg)
{
    SdkServerSysComm* pThis = (SdkServerSysComm*)arg;
    int ret;
    ret = pThis->__WriteIoImpl();
    if(ret < 0)
    {
        pThis->__BreakOut();
    }
    return ;
}

void SdkServerSysComm::ReadTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkServerSysComm* pThis = (SdkServerSysComm*)arg;
    pThis->__BreakOut();
    return ;
}

void SdkServerSysComm::WriteTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkServerSysComm* pThis = (SdkServerSysComm*)arg;
    pThis->__BreakOut();
    return ;
}


int SdkServerSysComm::Start()
{
    int ret;
    this->Stop();

    ret = this->__BindSocket();
    if(ret < 0)
    {
    	ERROR_INFO("\n");
        this->Stop();
        return ret;
    }


    /*now we should start the read io*/
    ret = this->__StartReadIo();
    if(ret < 0)
    {
    	ERROR_INFO("\n");
        this->Stop();
        return ret;
    }
    return 0;
}

int SdkServerSysComm::ResetLongTimeTimer()
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

