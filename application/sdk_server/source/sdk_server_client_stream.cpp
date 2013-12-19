
/************************************
*  this file is to handle client stream handle
************************************/
#include <sdk_server_client.h>
#include <sdk_server_mgmt.h>
#include <memory>
#include <sdk_server_debug.h>

typedef uint32_t sdk_stream_op_t;

#define  SECOND_TO_MILLS(sec) (((uint64_t)(sec)) * 1000)
#define  NANOSEC_TO_MILLS(nsec) (((uint64_t)(nsec)) / 1000000)


typedef struct
{
    sdk_stream_op_t m_OpCode;
    uint16_t m_StreamMask;
    uint16_t m_Reserved;
} sdk_open_video_request_t;


typedef struct
{
    uint8_t m_FrameType;
    uint8_t m_StreamId;
    uint8_t m_Rate;
    uint8_t m_EncType;
    uint8_t m_FOP;
    uint8_t m_Reserved[3];
    uint64_t m_Pts;
    uint32_t m_FrameID;
    uint16_t m_Width;
    uint16_t m_Height;
} sdk_video_i_frame_info_t;

typedef struct
{
    uint8_t m_FrameType;
    uint8_t m_StreamId;
    uint8_t m_Reserved[2];
    uint32_t m_FrameID;
    uint64_t m_Pts;
} sdk_video_p_frame_info_t;

typedef struct
{
    sdk_stream_op_t m_OpCode;
    uint32_t m_Result;
    uint32_t m_Count;
    /*at least just 1 infos otherwise */
    sdk_video_info_t m_Infos[MAX_STREAM_IDS];
} sdk_open_video_response_t;

typedef struct
{
    sdk_stream_op_t m_OpCode;
    uint32_t m_Result;
    sdk_audio_info_t m_AInfo;
} sdk_open_audio_response_t;




#define   SDK_RESPONSE_SUCC      0x0
#define   SDK_RESPONSE_FAILED    0x1

int SdkServerClient::__HandleStreamAudioOpen(sdk_client_comm_t*& pComm)
{
    return -ENOTSUP;
}

int SdkServerClient::__AudioFailedSend(void)
{
    sdk_client_comm_t* pComm=NULL;
    sdk_open_audio_response_t* pAResp=NULL;
    int ret;

    SDK_ASSERT(pComm == NULL);
    pComm = (sdk_client_comm_t*)calloc(sizeof(*pComm),1);
    if(pComm == NULL)
    {
        ret = -errno ? -errno : -1;
        goto fail;
    }

    pComm->m_SesId = this->m_SessionId;
    pComm->m_Priv = this->m_Priv;
    pComm->m_ServerPort = 0;
    if(this->m_pLoginComm)
    {
        pComm->m_SeqId = this->m_pLoginComm->m_SeqId;
        free(this->m_pLoginComm);
        this->m_pLoginComm = NULL;
    }
    else
    {
        pComm->m_SeqId = SDK_NOTIFY_SEQID;
    }
    pComm->m_Type = GMIS_PROTOCOL_TYPE_MEDIA_CTRL;
    pComm->m_Frag = 0;
    pComm->m_DataId = 0;
    pComm->m_Offset = 0;
    pComm->m_Totalsize = 0;
    pComm->m_DataLen = sizeof(*pAResp);

    pAResp = (sdk_open_audio_response_t*)pComm->m_Data;

    pAResp->m_OpCode = HOST_TO_PROTO32(SDK_STREAM_OA_RESPONSE);
    pAResp->m_Result = HOST_TO_PROTO32(SDK_RESPONSE_FAILED);


    /*now insert into the response message*/
    ret = this->__PushSysRespVec(pComm);
    if(ret < 0)
    {
        goto fail;
    }

    SDK_ASSERT(pComm == NULL);
    return 0;

fail:
    if(pComm)
    {
        free(pComm);
    }
    pComm = NULL;
    return ret;
}

int SdkServerClient::__HandleStreamVideoOpen(sdk_client_comm_t*& pComm)
{
    int ret;
    sdk_open_video_request_t* pVreq = (sdk_open_video_request_t*) pComm->m_Data;
    uint16_t h16;
    uint32_t h32;
    int curstreamid=0;
    std::vector<int> succstreamids;
    unsigned int i;
    int needaudiostream=0;
    int audiofailed=0;


    h32 = PROTO_TO_HOST32(pVreq->m_OpCode);
    SDK_ASSERT(h32 == SDK_STREAM_OV_REQUEST);

    if(pComm->m_DataLen< sizeof(*pVreq))
    {
        ERROR_INFO("\n");
        return -EINVAL;
    }

    if(this->m_OpenIds.size() > 0)
    {
        ERROR_INFO("\n");
        return -EPERM;
    }
    SDK_ASSERT(this->m_StreamIds.size() == 0);
    h16 = PROTO_TO_HOST16(pVreq->m_StreamMask);
    DEBUG_INFO("stream mask 0x%x datalen %d\n",h16,pComm->m_DataLen);

    if(h16 == 0)
    {
        ERROR_INFO("\n");
        return -EINVAL;
    }

    SDK_ASSERT(this->m_CurGetStreamIds == 0);
    curstreamid = 0;
    while(h16)
    {
        if(h16 & 0x1 && curstreamid < MAX_STREAM_IDS)
        {
            this->m_OpenIds.push_back(curstreamid);
        }

        h16 >>= 1;
        curstreamid += 1;
    }

    if(this->m_OpenIds.size() == 0)
    {
        ERROR_INFO("\n");
        return -ENODEV;
    }

    for(i=0; i<this->m_OpenIds.size(); i++)
    {
        ret = this->m_pSvrMgmt->StartStreamId(this->m_OpenIds[i],this);
        if(ret >= 0)
        {
            succstreamids.push_back(this->m_OpenIds[i]);
        }
        else
        {
            ERROR_INFO("start stream %d ret %d\n",this->m_OpenIds[i],ret);
        }
    }

    DEBUG_INFO("\n");

    if(succstreamids.size() ==0)
    {
        /*we do not get any success so just close the socket and make return*/
        this->m_OpenIds.clear();
        ERROR_INFO("\n");
        return -ENODEV;
    }

    for(i=0; i<succstreamids.size(); i++)
    {
        ret = this->m_pSvrMgmt->IsNeedOpenAudio(succstreamids[i]);
        if(ret > 0)
        {
            needaudiostream = 1;
        }
    }
    if(needaudiostream)
    {
        /*now we should open audio stream*/
        ret = this->m_pSvrMgmt->StartStreamId(AUDIO_STREAM_ID,this);
        if(ret < 0)
        {
            ERROR_INFO("start audio stream error\n");
            audiofailed = 1;
        }
        else
        {
            succstreamids.push_back(AUDIO_STREAM_ID);
        }
    }


    ret = this->m_pSvrMgmt->QueryStreamStarted();
    if(ret == STREAM_STATE_RUNNING)
    {
        std::auto_ptr<sys_stream_info_t> pSysQueryStreamIds2(new sys_stream_info_t);
        sys_stream_info_t* pSysQueryStreamIds = pSysQueryStreamIds2.get();
        ret = this->m_pSvrMgmt->GetStreamIdInfo(pSysQueryStreamIds);
        if(ret < 0)
        {
            this->m_OpenIds.clear();
            ERROR_INFO("\n");
            return ret;
        }

        if(this->m_pLoginComm)
        {
            free(this->m_pLoginComm);
            this->m_pLoginComm = NULL;
        }
        this->m_pLoginComm = pComm;
        pComm = NULL;

        ret = this->__PrepareSendStreamInfo(pSysQueryStreamIds,succstreamids);
        if(ret < 0)
        {
            this->m_StreamIds.clear();
            ERROR_INFO("\n");
            return ret;
        }
        DEBUG_INFO("\n");

        if(audiofailed)
        {
            ret = this->__AudioFailedSend();
            if(ret < 0)
            {
                return ret;
            }
        }


        DEBUG_INFO("\n");
        /*do not restart  writeio because in the __PrepareSendStreamInfo we __PushSysRespVec*/
        this->m_CurGetStreamIds = 0;
        this->m_StreamStarted = 1;
        DEBUG_INFO("\n");
        return 1;
    }

    if(this->m_pLoginComm)
    {
        free(this->m_pLoginComm);
        this->m_pLoginComm = NULL;
    }
    this->m_pLoginComm = pComm;
    pComm = NULL;


    /*SDK_ASSERT for pause ,because when we are at stopped state ,it will never let any streamid start success*/
    SDK_ASSERT(ret == STREAM_STATE_PAUSE);
    this->__StopWriteIo();
    this->__StopWriteTimer();
    this->m_StreamStarted = 0;
    /*we record all the streamids should open*/
    this->m_StreamIds = succstreamids;
    return 0;
}

int SdkServerClient::__HandleStreamRead(sdk_client_comm_t*& pComm)
{
    sdk_stream_op_t opCode,*popCode;
    int ret;

    if(pComm->m_DataLen < sizeof(opCode))
    {
        ERROR_INFO("\n");
        return -EINVAL;
    }

    popCode = (sdk_stream_op_t*)pComm->m_Data;

    opCode= PROTO_TO_HOST32(*popCode);
    switch(opCode)
    {
    case SDK_STREAM_OV_REQUEST:
        ret = this->__HandleStreamVideoOpen(pComm);
        break;
    case SDK_STREAM_OA_REQUEST:
        ret = this->__HandleStreamAudioOpen(pComm);
        break;
    case SDK_STREAM_AC_REQUEST:
        ret = this->__HandleStreamAudioDualOpen(pComm);
        break;
    default:
        return -EINVAL;
    }


    DEBUG_INFO("ret %d\n",ret);
    return ret;

}



int SdkServerClient::__ReadStreamIo()
{

    int ret;
    sdk_client_comm_t  *pComm=NULL;

    DEBUG_INFO("\n");
    ret = this->m_pSock->Read();
    if(ret < 0)
    {
        return ret;
    }
    else if(ret == 0)
    {
        DEBUG_INFO("\n");
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
    DEBUG_INFO("\n");

    /*all is read so we get data*/
    this->__StopReadTimer();
    pComm = this->m_pSock->GetRead();
    SDK_ASSERT(pComm);


    ret = this->__HandleStreamRead(pComm);
    if(ret < 0)
    {
        if(pComm)
        {
            free(pComm);
        }
        pComm = NULL;
        return ret;
    }

    if(pComm)
    {
        free(pComm);
    }
    pComm = NULL;


    return ret;
}


int SdkServerClient::__WriteResponse()
{
    int ret =0;
    int writepacket = 0;
    sdk_client_comm_t* pComm=NULL;

try_packet:
    SDK_ASSERT(pComm == NULL);
    if(this->m_RespVec.size() == 0 && this->m_pSock->IsWriteSth() == 0)
    {
        /*all is done ,so return 1*/
        if(writepacket > 0)
        {
            ret = this->__ResetWriteTimer();
            if(ret < 0)
            {
                return ret;
            }
        }
        /*we return 1 ,for it will write more*/
        return writepacket ? writepacket : 1;
    }

    if(this->m_pSock->IsWriteSth() > 0)
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
                ret = this->__ResetWriteTimer();
                if(ret < 0)
                {
                    return ret;
                }
            }
            return 0;
        }

        SDK_ASSERT(ret > 0);
        writepacket += 1;
        this->m_pSock->ClearWrite();
        goto try_packet;
    }

    pComm = this->m_RespVec[0];
    SDK_ASSERT(pComm);
    this->m_RespVec.erase(this->m_RespVec.begin());
    ret = this->m_pSock->PutData(pComm);
    if(ret < 0)
    {
        FreeComm(pComm);
        return ret;
    }
    pComm = NULL;

    goto try_packet;

    /*we can not reach the end of this*/
    return 0;
}

int SdkServerClient::__WriteStreamData(int streamid,struct iovec * iov,int iovlen)
{
    int ret;
    int i;
    //DEBUG_INFO("iovlen %d\n",iovlen);
    for(i=0; i<iovlen; i++)
    {
        //DEBUG_INFO("[%d].base %p len %d\n",i,iov[i].iov_base,iov[i].iov_len);
    }
    ret = writev(this->m_pSock->GetSocket(),iov,iovlen);
    if(ret < 0)
    {
        ret = -errno;
        if(errno == EAGAIN || errno == EINTR ||
                errno == EWOULDBLOCK)
        {
            DEBUG_INFO("errno %d\n",errno);
            ret = 0;
        }
        return ret;
    }

    return ret;

}


int SdkServerClient::__WriteStreamIo()
{
    int ret,res;
    struct iovec iov[MAX_IOV_LEN];
    int iovlen = MAX_IOV_LEN;
    int begin=0;
    int curstreamid;
    int maxtries = this->m_StreamIds.size();
    int tries = 0;
    int haswritelen = 0,haswritepacket=0,wlen;
    int blocked = 0;
    unsigned int i;
    struct timespec tmspec;
    uint64_t curmills;

    if(this->m_StreamIds.size() == 0)
    {
        ret = this->__WriteResponse();
        if(ret < 0)
        {
            ERROR_INFO("\n");
            return ret;
        }
        else if(ret == 0)
        {
            return 0;
        }

        /*this is we write all the response packet*/
        DEBUG_INFO("stop write io\n");
        this->__StopWriteIo();
        this->__StopWriteTimer();

        return 1;
    }

    /*now to get for the stream data*/
try_again:
    /*we have try all the streamids or blocked */
    if(tries >= maxtries || blocked)
    {
        if(haswritepacket > 0)
        {
            /*if we have write packet success ,so we make sure that the network is ok*/
            this->__ResetWriteTimer();
        }
        if(haswritelen == 0 && blocked == 0)
        {
            /*if we have nothing to write ,so we should not give any write io detection*/
            //DEBUG_INFO("[%d]stop write io\n",this->GetSocket());
            this->__StopWriteIo();
            for(i=0; i<this->m_StreamIds.size(); i++)
            {
                //this->m_pSvrMgmt->DebugStreamBufferBlock(this->m_StreamIds[i],this->GetSocket());
            }

            if(this->m_StreamStarted == 0)
            {
                /*this is because of stream paused ,so we start */
                DEBUG_INFO("stop write timer\n");
                this->__StopWriteTimer();
            }
        }
        else if(haswritelen == 0)
        {
            DEBUG_INFO("[%d] not write anyone\n",this->GetSocket());
        }

        return 0;
    }
    iovlen = MAX_IOV_LEN;
    if(this->m_CurGetStreamIds >= this->m_StreamIds.size())
    {
        ERROR_INFO("curGetStreamIds (%d) >= size(%d)\n",this->m_CurGetStreamIds,this->m_StreamIds.size());
    }
    SDK_ASSERT(this->m_CurGetStreamIds < this->m_StreamIds.size());
    curstreamid = this->m_StreamIds[this->m_CurGetStreamIds];

    ret = this->m_pSvrMgmt->GetStreamData(this->m_pSock->GetSocket(),
                                          curstreamid,iov,iovlen,begin);
    if(ret < 0)
    {
        ERROR_INFO("\n");
        return ret;
    }
    else if(ret == 0)
    {
        ret = this->__WriteResponse();
        if(ret < 0)
        {
            ERROR_INFO("\n");
            return ret;
        }
        else if(ret == 0)
        {
            return ret;
        }

        SDK_ASSERT(ret > 0);

        /*we try next stream sending */
        tries += 1;
        this->m_CurGetStreamIds += 1;
        this->m_CurGetStreamIds %= this->m_StreamIds.size();
        goto try_again;
    }
    else if(ret > 0 && begin == 1)
    {
        res = clock_gettime(CLOCK_MONOTONIC,&tmspec);
        if(res >= 0)
        {
            curmills = SECOND_TO_MILLS(tmspec.tv_sec)+ NANOSEC_TO_MILLS(tmspec.tv_nsec);
            this->m_StartSendMills = curmills;
        }
        else
        {
            ERROR_INFO("can not get CLOCK_MONOTONIC error(%d)\n",errno);
        }

        ret = this->__WriteResponse();
        if(ret < 0)
        {
            ERROR_INFO("\n");
            return ret;
        }
        else if(ret == 0)
        {
            ERROR_INFO("\n");
            return ret;
        }

        SDK_ASSERT(ret > 0);

        /*passdown ,we have write something*/
    }
    tries = 0;

    ret = this->__WriteStreamData(curstreamid,iov,iovlen);
    if(ret < 0)
    {
        ERROR_INFO("\n");
        return ret;
    }

    wlen = ret;
    haswritelen += wlen;
    //DEBUG_INFO("write wlen %d\n",ret);

    ret = this->m_pSvrMgmt->ForwardStreamData(this->m_pSock->GetSocket(),
            curstreamid,iov,iovlen,wlen);
    if(ret < 0)
    {
        ERROR_INFO("\n");
        return ret;
    }
    else if(ret > 0)
    {
        res = clock_gettime(CLOCK_MONOTONIC,&tmspec);
        if(res >= 0)
        {
            curmills = SECOND_TO_MILLS(tmspec.tv_sec)+ NANOSEC_TO_MILLS(tmspec.tv_nsec);
            if(this->m_LastSendMills != 0 && (curmills - this->m_LastSendMills) > 100)
            {
                ERROR_INFO("SENDOUTOUTTIME last mills (%lld) curmills(%lld)(%ld:%ld) (%lld)\n",this->m_LastSendMills,
                           curmills,tmspec.tv_sec,tmspec.tv_nsec,(curmills - this->m_LastSendMills));
            }
            if((curmills - this->m_StartSendMills) > 100)
            {
                ERROR_INFO("PACKETSENDTIME start (%lld) curmills(%lld) (%lld)\n",this->m_StartSendMills,curmills,
                           (curmills - this->m_StartSendMills));
            }
            this->m_LastSendMills = curmills;
        }
        else
        {
            ERROR_INFO("can not get CLOCK_MONOTONIC error(%d)\n",errno);
        }

        haswritepacket += 1;
        //DEBUG_INFO("write packet %d\n",haswritepacket);

        /*for next stream ids get */
        this->m_CurGetStreamIds += 1;
        this->m_CurGetStreamIds %= this->m_StreamIds.size();
    }
    else
    {
        /*this indicates it will blocked ,not write all the things ok*/
        blocked = 1;
    }

    goto try_again;

    /*can not reach here*/

    return 0;
}


int SdkServerClient::__CopyVideoInfo(sdk_video_info_t * pVInfo,SysPkgEncodeCfg * pSysCfg)
{
    DEBUG_INFO("\n");
    pVInfo->m_Result= HOST_TO_PROTO32(0);
    pVInfo->m_StreamId = pSysCfg->s_Flag;
    pVInfo->m_Rate = pSysCfg->s_FPS;
    pVInfo->m_FOP = pSysCfg->s_Gop;

    DEBUG_INFO("\n");

    if(pSysCfg->s_Compression == SYS_COMP_H264)
    {
        pVInfo->m_EncType = PROTO_STREAM_H264;
    }
    else if(pSysCfg->s_Compression == SYS_COMP_MJPEG)
    {
        pVInfo->m_EncType = PROTO_STREAM_MJPEG;
    }
    else
    {
        pVInfo->m_EncType = PROTO_STREAM_NONE;
    }
    DEBUG_INFO("\n");

    pVInfo->m_Width = HOST_TO_PROTO16(pSysCfg->s_PicWidth);
    pVInfo->m_Height = HOST_TO_PROTO16(pSysCfg->s_PicHeight);
    DEBUG_INFO("\n");

    return 0;
}

int SdkServerClient::__CopyAudioInfo(sdk_audio_info_t * pAInfo,SysPkgAudioEncodeCfg * pAudioCfg)
{
    memset(pAInfo,0,sizeof(*pAInfo));
    pAInfo->m_EncodeType = pAudioCfg->s_EncodeType;
    pAInfo->m_Chan = pAudioCfg->s_Chan;
    pAInfo->m_BitPerSample = pAudioCfg->s_BitsPerSample;
    pAInfo->m_SamplePerSec = HOST_TO_PROTO32(pAudioCfg->s_SamplesPerSec);
    return 0;
}

int SdkServerClient::__PrepareSendStreamInfo(sys_stream_info_t * pStreamInfo,std::vector<int>& succstreamids)
{
    int ret,res;
    sdk_client_comm_t *pComm=NULL;
    sdk_open_video_response_t* pResp=NULL;
    sdk_video_info_t* pVInfo=NULL;
    sdk_open_audio_response_t* pAResp=NULL;
    sdk_audio_info_t* pAInfo=NULL;
    uint32_t n32;
    int findsucc= -1;
    unsigned int i,j,k;
	int curstreamid;
    int oldaudio=0,newaudio=0;
    int cpyidx=0;
    struct iovec iov[4];
    int iovlen = 4;
    int begin;

    DEBUG_INFO("\n");
    if(pStreamInfo == NULL)
    {
        return -EINVAL;
    }

    if(this->m_OpenIds.size() == 0)
    {
        return -ENODEV;
    }

    if(succstreamids.size() == 0)
    {
        return -EINVAL;
    }

    DEBUG_INFO("\n");
    pComm = AllocateComm(sizeof(*pResp));
    if(pComm == NULL)
    {
        return -ENOMEM;
    }

    pComm->m_SesId = this->m_SessionId;
    pComm->m_Priv = this->m_Priv;
    pComm->m_ServerPort = 0;
    if(this->m_pLoginComm)
    {
        pComm->m_SeqId = this->m_pLoginComm->m_SeqId;
        DEBUG_INFO("\n");
        free(this->m_pLoginComm);
        this->m_pLoginComm = NULL;
        DEBUG_INFO("\n");
    }
    else
    {
        pComm->m_SeqId = SDK_NOTIFY_SEQID;
    }
    DEBUG_INFO("\n");
    pComm->m_Type = GMIS_PROTOCOL_TYPE_MEDIA_CTRL;
    pComm->m_Frag = 0;
    pComm->m_DataId = 0;
    pComm->m_Offset = 0;
    pComm->m_Totalsize = 0;
    pResp = (sdk_open_video_response_t*)pComm->m_Data;
    pVInfo = &(pResp->m_Infos[0]);
    DEBUG_INFO("\n");

    cpyidx = 0;
    for(i=0; i<this->m_OpenIds.size(); i++)
    {
        for(j=0; j<pStreamInfo->m_Count; j++)
        {
            if((this->m_OpenIds[i]) == pStreamInfo->m_VideoInfo[j].s_Flag)
            {
                findsucc = -1;
                for(k=0; k<succstreamids.size(); k++)
                {
                    if(succstreamids[k] == this->m_OpenIds[i])
                    {
                        findsucc = k;
                        break;
                    }
                }
                /*now to set for the job*/
                if(findsucc >= 0)
                {
                    DEBUG_INFO("idx %d j %d\n",cpyidx,j);
                    ret = this->__CopyVideoInfo(&(pVInfo[cpyidx]),&(pStreamInfo->m_VideoInfo[j]));
                    if(ret < 0)
                    {
                        ERROR_INFO("\n");
                        goto fail;
                    }
                }
                else
                {
                    pVInfo[cpyidx].m_Result = 1;
                }
                cpyidx ++;
                goto next_cycle;
            }
        }

next_cycle:
        j = j;
    }


    /*all is copied ,so we should copied ,so we check if all is  started success */

    if(cpyidx > 0)
    {
        /*if we have ,so we should notify */
        n32 = HOST_TO_PROTO32(SDK_STREAM_OV_RESPONSE);
        pResp->m_OpCode = n32;
        n32 = HOST_TO_PROTO32(SDK_RESPONSE_SUCC);
        pResp->m_Result = n32;
        n32 = HOST_TO_PROTO32(cpyidx);
        pResp->m_Count = n32;
        /*we should copy the index to do so,because we have audio */
        pComm->m_DataLen = sizeof(*pResp) - ((MAX_STREAM_IDS-cpyidx)*sizeof(pResp->m_Infos[0]));
        DEBUG_BUFFER_FMT(pComm->m_Data,pComm->m_DataLen,"Buff video");



        /*now to push notify vector we should reset timer*/
        ret = this->__PushSysRespVec(pComm);
        if(ret < 0)
        {
            goto fail;
        }
        SDK_ASSERT(pComm == NULL);
    }
    else
    {
        /*if we have not any one ,so we should not send this packet*/
        ret = -ENODEV;
        ERROR_INFO("copy video info zero\n");
        goto fail;
    }



    /*now we should change the socket send and receive buffer*/
    n32 = (1<<21);
    res = setsockopt(this->GetSocket(),SOL_SOCKET,SO_SNDBUF,(const char*)&n32,sizeof(n32));
    if(res >= 0)
    {
        socklen_t sndbufsize=0;
        uint32_t sndsize=0;
        sndbufsize = sizeof(sndsize);
        res = getsockopt(this->GetSocket(),SOL_SOCKET,SO_SNDBUF,(void*)&sndsize,&sndbufsize);
        if(res >= 0)
        {
            if(sndsize != (n32*2))
            {
                ERROR_INFO("[%d] getopt sndsize (0x%08x) set sndsize(0x%08x)\n",this->GetSocket(),sndsize,n32);
            }
        }
        else
        {
            ERROR_INFO("[%d] can not get sndbufsize error(%d)\n",this->GetSocket(),errno);
        }
    }
    else
    {
        ERROR_INFO("[%d] set 0x%08x sndbuffer error(%d)\n",this->GetSocket(),n32,errno);
    }


    DEBUG_INFO("\n");

    /*now we should test if we have audio opened*/
    oldaudio = 0;
    newaudio = 0;
    for(i=0; i<this->m_StreamIds.size(); i++)
    {
        if(this->m_StreamIds[i] == AUDIO_STREAM_ID)
        {
            oldaudio = 1;
            break;
        }
    }
    DEBUG_INFO("\n");

    for(i=0; i<succstreamids.size(); i++)
    {
        if(succstreamids[i] == AUDIO_STREAM_ID)
        {
            newaudio = 1;
            break;
        }
    }

    DEBUG_INFO("\n");
    if(oldaudio == 1 || newaudio == 1)
    {
        /*if we have some value of the so we should do the pComm*/
        SDK_ASSERT(pComm == NULL);
        pComm = AllocateComm(sizeof(*pAResp));
        if(pComm == NULL)
        {
            ret = -errno ? -errno : -1;
            goto fail;
        }

        pComm->m_SesId = this->m_SessionId;
        pComm->m_Priv = this->m_Priv;
        pComm->m_ServerPort = 0;
        if(this->m_pLoginComm)
        {
            pComm->m_SeqId = this->m_pLoginComm->m_SeqId;
            FreeComm(this->m_pLoginComm);
        }
        else
        {
            pComm->m_SeqId = SDK_NOTIFY_SEQID;
        }
        pComm->m_Type = GMIS_PROTOCOL_TYPE_MEDIA_CTRL;
        pComm->m_Frag = 0;
        pComm->m_DataId = 0;
        pComm->m_Offset = 0;
        pComm->m_Totalsize = 0;
        pComm->m_DataLen = sizeof(*pAResp);

        pAResp =(sdk_open_audio_response_t*) pComm->m_Data;

        pAResp->m_OpCode = HOST_TO_PROTO32(SDK_STREAM_OA_RESPONSE);
        if(oldaudio && newaudio == 0)
        {
            pAResp->m_Result = HOST_TO_PROTO32(SDK_RESPONSE_FAILED);
        }
        else
        {
            /*it means we have open audio ok ,so it is ok to do this*/
            pAResp->m_Result = HOST_TO_PROTO32(SDK_RESPONSE_SUCC);
            /*now to fill the information*/
            pAInfo = &(pAResp->m_AInfo);
            /*now search for the copy info*/
            if(pStreamInfo == NULL || pStreamInfo->m_AudioCount < 1)
            {
                ret = -ENODEV;
                goto fail;
            }

            ret = this->__CopyAudioInfo(pAInfo,&(pStreamInfo->m_AudioInfo[0]));
            if(ret < 0)
            {
                goto fail;
            }
        }

        DEBUG_BUFFER_FMT(pComm->m_Data,pComm->m_DataLen,"oldaudio %d newaudio %d",oldaudio,newaudio);

        /*now insert into the response message*/
        ret = this->__PushSysRespVec(pComm);
        if(ret < 0)
        {
            goto fail;
        }
        SDK_ASSERT(pComm == NULL);
    }

    DEBUG_INFO("\n");

    /*now at last ,to check out whether the streamids is in the new succstreamids ,if in it and not*/
    for(i=0; i<this->m_StreamIds.size() ; i++)
    {
        findsucc = -1;
        for(j=0; j<succstreamids.size() ; j++)
        {
            if(this->m_StreamIds[i] == succstreamids[j])
            {
                findsucc = j;
                break;
            }
        }

        if(findsucc < 0)
        {
            /*if we have not opened it yet ,to test whether this is send over*/
            iovlen = 4;
            begin = 0;
            ret = this->m_pSvrMgmt->GetStreamData(this->GetSocket(),this->m_StreamIds[i],iov,iovlen,begin);
            if(ret < 0)
            {
                ERROR_INFO("[%d] Stream(%d) GetData Error(%d)\n",this->GetSocket(),this->m_StreamIds[i],ret);
                goto fail;
            }
            else if(ret > 0 && begin == 0)
            {
                ERROR_INFO("[%d] Stream(%d) not opened ,but has data send\n",this->GetSocket(),this->m_StreamIds[i]);
                ret = -EINVAL;
                goto fail;
            }
        }
    }

    SDK_ASSERT(this->m_StreamIds.size() == 0 || this->m_CurGetStreamIds < this->m_StreamIds.size());
    if(this->m_StreamIds.size() > 0)
    {
        /*now we should change for the curGetStreamIds*/
        curstreamid = this->m_StreamIds[this->m_CurGetStreamIds];
        findsucc = -1;
        for(i=0; i<succstreamids.size(); i++)
        {
            if(succstreamids[i] == curstreamid)
            {
                findsucc = i;
                break;
            }
        }

        if(findsucc >= 0)
        {
            this->m_CurGetStreamIds = findsucc;
        }
        else
        {
            this->m_CurGetStreamIds = 0;
        }
    }
    else
    {
        this->m_CurGetStreamIds = 0;
    }

    this->m_StreamIds= succstreamids;
    return 0;

fail:
    FreeComm(pComm);
    return ret;
}


int SdkServerClient::PauseStream()
{
    /*we do not change anything ,just change the state*/
    this->m_StreamStarted = 0;
    DEBUG_INFO("pause stream\n");
    if(this->__GetState() == sdk_client_stream_state)
    {
        /*because ,this may be in the case when the state of no waiting,so we restart io and let the write to find it will stop write timer*/
        this->__StopWriteIo();
        this->__StartWriteIo();
        /*we reset timer ,because it will cost time in the pause stream handling,so we can do this ok*/
        this->__ResetWriteTimer();
    }
    else if(this->__GetState() == sdk_client_stream_audio_dual_state)
    {
        /*this is almost like sdk_client_stream_state ,we need to read time*/
        this->__StopWriteIo();
        this->__StartWriteIo();
        this->__ResetWriteTimer();

        /*we do not fill the readio*/
    }
    else if(this->__GetState() == sdk_client_stream_audio_dual_pending_state)
    {
        /*this is almost like sdk_client_stream_state ,we need to read time*/
        this->__StopWriteIo();
        this->__StartWriteIo();
        this->__ResetWriteTimer();
    }
    else if(this->__GetState() == sdk_client_stream_audio_dual_pending_start_state)
    {
        /*this is almost like sdk_client_stream_state ,we need to read time*/
        this->__StopWriteIo();
        this->__StartWriteIo();
        this->__ResetWriteTimer();
    }
    return 0;
}


int SdkServerClient::__InsertAudioDualNotify(uint32_t result,int seqid)
{
    sdk_client_comm_t* pComm=NULL;
    std::auto_ptr<sys_stream_info_t> pStreamInfo2(new sys_stream_info_t);
    sys_stream_info_t *pStreamInfo = pStreamInfo2.get();
    int ret;
    start_talk_response_t* pTalkResp=NULL;
    start_talk_resp_t *pFillResp=NULL;
    uint32_t n32;

    pComm = AllocateComm(sizeof(*pTalkResp));

    if(pComm == NULL)
    {
        ret = -errno ? -errno : -1;
        goto fail;
    }

    pComm->m_SesId = this->m_SessionId;
    pComm->m_Priv = this->m_Priv;
    pComm->m_ServerPort = 0;
    pComm->m_LocalPort = 0;
    pComm->m_SeqId = seqid;
    pComm->m_Type = GMIS_PROTOCOL_TYPE_MEDIA_CTRL;
    pComm->m_FHB = 0;
    pComm->m_Frag = 0;
    pComm->m_DataId = 0;
    pComm->m_Offset = 0;
    pComm->m_Totalsize = 0;
    pComm->m_DataLen = sizeof(*pTalkResp);

    pTalkResp = (start_talk_response_t*) pComm->m_Data;
    pFillResp = &(pTalkResp->m_Resp);
    pTalkResp->m_OpCode = HOST_TO_PROTO32(SDK_STREAM_AC_RESPONSE);


    if(result)
    {
        pTalkResp->m_Result = HOST_TO_PROTO32(result);
    }
    else
    {
        ret = this->m_pSvrMgmt->GetStreamIdInfo(pStreamInfo);
        if(ret < 0)
        {
            ERROR_INFO("[%d]could not get streamid info\n",this->GetSocket());
            goto fail;
        }

        pTalkResp->m_Result = HOST_TO_PROTO32(0);

        memset(pFillResp->m_szDestIpAddr,0,sizeof(pFillResp->m_szDestIpAddr));
        pFillResp->m_DestPort = HOST_TO_PROTO32(0);

        /*now it is for the default*/
        if(pStreamInfo->m_AudioCount == 0)
        {
            pFillResp->m_EncodeType = 1;
            /*1 channel*/
            pFillResp->m_Channel = 1;
            pFillResp->m_BitPerSample = 16;
            pFillResp->m_Reserv1 = 0;
            /*8000 frequency*/
            pFillResp->m_SamplePerSec = HOST_TO_PROTO16(8000);
            pFillResp->m_AvgBytesPerSec = HOST_TO_PROTO16(8000 * 16 * 1 /8);
            /*20 millsec frequency*/
            pFillResp->s_FrameRate = HOST_TO_PROTO16(50);
            pFillResp->s_BitRate = HOST_TO_PROTO16(8000);
            /*volume for 50%*/
            pFillResp->s_Volume = HOST_TO_PROTO16(50);
            pFillResp->s_AecFlag = HOST_TO_PROTO16(0);
            pFillResp->s_AecDelayTime = HOST_TO_PROTO16(0);
            pFillResp->s_Reserv2 = HOST_TO_PROTO16(0);

        }
        else
        {
            SysPkgAudioEncodeCfg* pAudioInfo=&(pStreamInfo->m_AudioInfo[0]);
            pFillResp->m_EncodeType = pAudioInfo->s_EncodeType;
            pFillResp->m_Channel = pAudioInfo->s_Chan;
            pFillResp->m_BitPerSample = pAudioInfo->s_BitsPerSample;
            pFillResp->m_Reserv1 = 0;
            pFillResp->m_SamplePerSec = HOST_TO_PROTO32(pAudioInfo->s_SamplesPerSec);
            n32 = pAudioInfo->s_Chan* pAudioInfo->s_BitsPerSample* pAudioInfo->s_SamplesPerSec/ 8;
            pFillResp->m_AvgBytesPerSec = HOST_TO_PROTO32(n32);
            /*20 millsec frequency*/
            pFillResp->s_FrameRate = HOST_TO_PROTO16(50);
            pFillResp->s_BitRate = HOST_TO_PROTO16(pAudioInfo->s_SamplesPerSec);
            /*volume for 50%*/
            pFillResp->s_Volume = HOST_TO_PROTO16(pAudioInfo->s_PlayVolume);
            pFillResp->s_AecFlag = HOST_TO_PROTO16(0);
            pFillResp->s_AecDelayTime = HOST_TO_PROTO16(0);
            pFillResp->s_Reserv2 = HOST_TO_PROTO16(0);

        }
    }

    ret = this->__PushSysRespVec(pComm);
    if(ret < 0)
    {
        goto fail;
    }
    SDK_ASSERT(pComm == NULL);

    if(result)
    {
        /*this is the end*/
        ERROR_INFO("\n");
        this->m_Ended = 1;
    }

    return 1;

fail:
    FreeComm(pComm);
    return ret;
}


int SdkServerClient::ResumeStream(sys_stream_info_t * pStreamInfo)
{
    int ret;
    sdk_client_state_t state;
    unsigned int i,j;
    int findidx=-1;
    std::vector<int> succstreamids;
    int isvideostream=0,needaudiostream=0;
    int findaudio=-1;

    /*now to change the state*/
    if(this->m_StreamStarted > 0)
    {
        /*yes this is really a bug on the job ,but we can not make sure where it is from ,it may
               be the bug in sdk_server or it may be in sys_server*/
        return -EINVAL;
    }

    /*now it is the way to give the change of state*/
    state = this->__GetState();
    if(state != sdk_client_stream_state &&
            state != sdk_client_stream_audio_dual_start_state &&
            state != sdk_client_stream_audio_dual_state &&
            state != sdk_client_stream_audio_dual_pending_state &&
            state != sdk_client_stream_audio_dual_pending_start_state)
    {
        /*not state of the stream ,so we pretend to handle it*/
        return 0;
    }

    for(i=0; i<this->m_StreamIds.size(); i++)
    {
        ret = this->m_pSvrMgmt->ResumeStreamId(this->m_StreamIds[i],this);
        if(ret >= 0)
        {
            succstreamids.push_back(this->m_StreamIds[i]);
        }
    }

    /*now check for last time not opened*/
    for(i=0; i<this->m_OpenIds.size() ; i++)
    {
        findidx = -1;
        for(j=0; j<this->m_StreamIds.size(); j++)
        {
            if(this->m_OpenIds[i] == this->m_StreamIds[j])
            {
                findidx = j;
                break;
            }
        }

        if(findidx < 0)
        {
            /*now we should to start this streamids ,so we can test*/
            ret = this->m_pSvrMgmt->StartStreamId(this->m_OpenIds[i],this);
            if(ret >= 0)
            {
                succstreamids.push_back(this->m_OpenIds[i]);
            }
        }
    }

    /*we could not set for null */
    if(succstreamids.size() == 0 && this->__GetState() == sdk_client_stream_state)
    {
        return -ENODEV;
    }



    /*now we should test if it is just audio stream no video stream*/
    isvideostream = 0;
    needaudiostream = 0;
    for(i=0; i<succstreamids.size(); i++)
    {
        if(succstreamids[i] >= 0 && succstreamids[i] < MAX_STREAM_IDS)
        {
            isvideostream = 1;
            ret = this->m_pSvrMgmt->IsNeedOpenAudio(succstreamids[i]);
            if(ret > 0)
            {
                needaudiostream = 1;
            }
        }
    }

    if(isvideostream == 0 && state == sdk_client_stream_state)
    {
        /*no video stream open ,so we should close this socket */
        return -ENODEV;
    }

    /*to check if we have already open audio*/
    findaudio = -1;
    for(i=0; i<succstreamids.size(); i++)
    {
        if(succstreamids[i] == AUDIO_STREAM_ID)
        {
            findaudio = i;
            break;
        }
    }
    if(needaudiostream)
    {
        if(findaudio < 0)
        {
            ret = this->m_pSvrMgmt->StartStreamId(AUDIO_STREAM_ID,this);
            if(ret < 0)
            {
                ERROR_INFO("could not use audio open\n");
            }
            else
            {
                succstreamids.push_back(AUDIO_STREAM_ID);
            }
        }
    }
    else
    {
        if(findaudio >= 0)
        {
            /*it is really a bug ,when we can open audio but it is not need any video*/
            ERROR_INFO("open audio but not need\n");
            this->m_pSvrMgmt->StopStreamId(succstreamids[findaudio],this);
            succstreamids.erase(succstreamids.begin()+findaudio);
        }
    }




    /*now to give the job */
    if(state == sdk_client_stream_state)
    {
        ret =  this->__PrepareSendStreamInfo(pStreamInfo,succstreamids);
        if(ret < 0)
        {
            return ret;
        }
    }
    else
    {
        /*now test for the audio is in the successstreamids*/
        findidx = -1;
        for(i=0; i<succstreamids.size() ; i++)
        {
            if(succstreamids[i] >= 0 && succstreamids[i] < MAX_STREAM_IDS)
            {
                ERROR_INFO("[%d].[%d] streamid (%d)\n",this->GetSocket(),i,succstreamids[i]);
                return -EINVAL;
            }
            else if(succstreamids[i] == AUDIO_STREAM_ID)
            {
                findidx = i;
            }
        }

        if(findidx < 0)
        {
            ERROR_INFO("[%d] no audio stream opened",this->GetSocket());

        }
        /*this is to set audio dual seqid for it will */
        ret = this->__InsertAudioDualNotify(0,this->m_AudioDualSeqId);
        if(ret < 0)
        {
            return ret;
        }
        this->m_AudioDualSeqId = SDK_NOTIFY_SEQID;
		/*only for the cur streamid == 0*/
		this->m_CurGetStreamIds = 0;

    }

    /*do not restart  writeio because in the __PrepareSendStreamInfo we __PushSysRespVec*/
    this->m_StreamStarted = 1;

    /*we reset the stream reset*/
    DEBUG_INFO("resume stream\n");
    this->__StopWriteIo();
    this->__StartWriteIo();
    this->__ResetWriteTimer();
    return 1;
}

int SdkServerClient::NotifyClient()
{
    int ret;
    sdk_client_state_t state;

    state = this->__GetState();
    if(this->m_StreamStarted && this->m_InsertWriteIo == 0 && (state == sdk_client_stream_state  ||
            state == sdk_client_stream_audio_dual_start_state ||
            state == sdk_client_stream_audio_dual_state ||
            state == sdk_client_stream_audio_dual_pending_state ||
            state == sdk_client_stream_audio_dual_pending_start_state))
    {
        /*if we are in the write timer */
        //DEBUG_INFO("notify client %d\n",this->GetSocket());
        ret = this->__StartWriteIo();
        if(ret < 0)
        {
            return ret;
        }
    }

    return 0;
}



/************************************************************
*    this is the read dualstart io this can not be do this,so return error
*
************************************************************/
int SdkServerClient::__ReadStreamAudioDualStartIo()
{
    /*we can not accep make this ok*/
    return -EINVAL;
}



int SdkServerClient::__ReadStreamAudioDualIo()
{
    int ret;
    sdk_client_comm_t* pComm=NULL;
    uint32_t opcode,*popcode=NULL;

    ret = this->m_pSock->Read();
    if(ret <= 0)
    {
        return ret;
    }

    this->__StopReadTimer();
    if(this->m_StreamStarted)
    {
        /*if we need start timer ,so we should every 3 seconds to read */
        ret = this->__StartReadTimer();
        if(ret < 0)
        {
            return ret;
        }
    }

    pComm = this->m_pSock->GetRead();
    SDK_ASSERT(pComm);
    /*now determine which opcode it is */
    if(pComm->m_DataLen < sizeof(opcode))
    {
        ERROR_INFO("[%d]comm receive len(%d) < sizeof(%d)\n",this->GetSocket(),
                   pComm->m_DataLen,sizeof(opcode));
        FreeComm(pComm);
        return -EINVAL;
    }
    popcode = (uint32_t*)pComm->m_Data;
    opcode = PROTO_TO_HOST32(*popcode);
    if(opcode == SDK_STREAM_AD_SEND)
    {
        /*now we should test if this is the one ,we want */
        ret = this->m_pSvrMgmt->PushAudioDecodeData(this->GetSocket(),pComm);
        if(ret < 0)
        {
            SDK_ASSERT(pComm);
            FreeComm(pComm);
            return ret;
        }

        SDK_ASSERT(pComm == NULL);
    }
    else
    {
        ERROR_INFO("[%d]we receive opcode (0x%08x:%d)\n",this->GetSocket(),opcode,opcode);
        FreeComm(pComm);
        return -EINVAL;
    }
    return 1;
}

int SdkServerClient::__ReadStreamAudioDualPendingIo()
{
    return -ENOTSUP;
}


int SdkServerClient::__ReadStreamAudioDualPendingStartIo()
{
    return -ENOTSUP;
}

int SdkServerClient::__WriteStreamAudioDualPendingIo()
{
    return -ENOTSUP;
}

int SdkServerClient::__WriteStreamAudioDualPendingStartIo()
{
    return -ENOTSUP;
}

int SdkServerClient::__WriteStreamAudioDualStartIo()
{
    int ret;
    DEBUG_INFO("write response\n");
    /*now to test if we have data to send*/
    if(this->m_pSock->IsWriteSth())
    {
        ret = this->m_pSock->Write();
        if(ret <= 0)
        {
            return ret;
        }
    }

    this->m_pSock->ClearWrite();

    this->__SetState(sdk_client_stream_audio_dual_state);
    /*all is write ,so we should change state*/
    ret = this->__ResetWriteTimer();
    if(ret < 0)
    {
        return ret;
    }

    /*all is ok*/
    return 1;
}

int SdkServerClient::__WriteStreamAudioDualIo()
{
    int ret,res;
    struct iovec iov[MAX_IOV_LEN];
    int iovlen = MAX_IOV_LEN;
    int begin=0;
    int curstreamid;
    int maxtries = this->m_StreamIds.size();
    int tries = 0;
    int haswritelen = 0,haswritepacket=0,wlen;
    int blocked = 0;
    unsigned int i;
    struct timespec tmspec;
    uint64_t curmills;

    if(this->m_StreamIds.size() == 0)
    {
        ret = this->__WriteResponse();
        if(ret < 0)
        {
            ERROR_INFO("\n");
            return ret;
        }
        else if(ret == 0)
        {
            return 0;
        }

        /*this is we write all the response packet*/
        DEBUG_INFO("stop write io\n");
        this->__StopWriteIo();
        this->__StopWriteTimer();

        return 1;
    }

    /*now to get for the stream data*/
try_again:
    /*we have try all the streamids or blocked */
    if(tries >= maxtries || blocked)
    {
        if(haswritepacket > 0)
        {
            /*if we have write packet success ,so we make sure that the network is ok*/
            this->__ResetWriteTimer();
        }
        if(haswritelen == 0 && blocked == 0)
        {
            /*if we have nothing to write ,so we should not give any write io detection*/
            //DEBUG_INFO("[%d]stop write io\n",this->GetSocket());
            this->__StopWriteIo();
            for(i=0; i<this->m_StreamIds.size(); i++)
            {
                //this->m_pSvrMgmt->DebugStreamBufferBlock(this->m_StreamIds[i],this->GetSocket());
            }

            if(this->m_StreamStarted == 0)
            {
                /*this is because of stream paused ,so we start */
                DEBUG_INFO("[%d]stop write timer\n",this->GetSocket());
                this->__StopWriteTimer();
            }
        }
        else if(haswritelen == 0)
        {
            DEBUG_INFO("[%d] not write anyone\n",this->GetSocket());
        }

        return 0;
    }
    iovlen = MAX_IOV_LEN;
    SDK_ASSERT(this->m_CurGetStreamIds < this->m_StreamIds.size());
    curstreamid = this->m_StreamIds[this->m_CurGetStreamIds];

    ret = this->m_pSvrMgmt->GetStreamData(this->m_pSock->GetSocket(),curstreamid,iov,iovlen,begin);
    if(ret < 0)
    {
        ERROR_INFO("[%d] get data error(%d)\n",this->GetSocket(),ret);
        return ret;
    }
    else if(ret == 0)
    {
        ret = this->__WriteResponse();
        if(ret < 0)
        {
            ERROR_INFO("\n");
            return ret;
        }
        else if(ret == 0)
        {
            DEBUG_INFO("\n");
            return ret;
        }

        SDK_ASSERT(ret > 0);

        /*we try next stream sending */
        tries += 1;
        this->m_CurGetStreamIds += 1;
        this->m_CurGetStreamIds %= this->m_StreamIds.size();
        goto try_again;
    }
    else if(ret > 0 && begin == 1)
    {
        ret = this->__WriteResponse();
        if(ret < 0)
        {
            ERROR_INFO("\n");
            return ret;
        }
        else if(ret == 0)
        {
            ERROR_INFO("\n");
            return ret;
        }

        SDK_ASSERT(ret > 0);

        /*passdown ,we have write something*/
    }
    tries = 0;

    ret = this->__WriteStreamData(curstreamid,iov,iovlen);
    if(ret < 0)
    {
        ERROR_INFO("\n");
        return ret;
    }

    wlen = ret;
    haswritelen += wlen;
    //DEBUG_INFO("write wlen %d\n",ret);

    ret = this->m_pSvrMgmt->ForwardStreamData(this->m_pSock->GetSocket(),
            curstreamid,iov,iovlen,wlen);
    if(ret < 0)
    {
        ERROR_INFO("\n");
        return ret;
    }
    else if(ret > 0)
    {
        res = clock_gettime(CLOCK_MONOTONIC,&tmspec);
        if(res >= 0)
        {
            curmills = SECOND_TO_MILLS(tmspec.tv_sec)+ NANOSEC_TO_MILLS(tmspec.tv_nsec);
            if(this->m_LastSendMills != 0 && (curmills - this->m_LastSendMills) > 100)
            {
                ERROR_INFO("SENDOUTOUTTIME last mills (%lld) curmills(%lld)(%ld:%ld) (%lld)\n",this->m_LastSendMills,
                           curmills,tmspec.tv_sec,tmspec.tv_nsec,(curmills - this->m_LastSendMills));
            }
            this->m_LastSendMills = curmills;
        }
        else
        {
            ERROR_INFO("can not get CLOCK_MONOTONIC error(%d)\n",errno);
        }
        haswritepacket += 1;
        //DEBUG_INFO("write packet %d\n",haswritepacket);

        /*for next stream ids get */
        this->m_CurGetStreamIds += 1;
        this->m_CurGetStreamIds %= this->m_StreamIds.size();
    }
    else
    {
        /*this indicates it will blocked ,not write all the things ok*/
        blocked = 1;
    }

    goto try_again;

    /*can not reach here*/

    return 0;
}



/************************************************************
* open and close
************************************************************/
int SdkServerClient::__HandleStreamAudioDualOpen(sdk_client_comm_t * & pComm)
{
    int ret,wait;
    AudioDecParam *pAudioDec=NULL;
    start_talk_request_t *pStartTalkReq=(start_talk_request_t*)pComm->m_Data;
    start_talk_t *pStartTalk=&(pStartTalkReq->m_Talk);
    uint16_t n16;
    uint32_t n32;

    if(this->m_OpenIds.size() > 0)
    {
        return -EINVAL;
    }

    SDK_ASSERT(this->m_StreamIds.size() == 0);
    this->m_OpenIds.push_back(AUDIO_STREAM_ID);

    DEBUG_INFO("\n");
    /*now first to test if we should change for the audio decodec*/
    pAudioDec = (AudioDecParam*)calloc(sizeof(*pAudioDec),1);
    if(pAudioDec == NULL)
    {
        ret = -errno ? -errno : -1;
        goto fail;
    }


    this->m_AudioDualSeqId = pComm->m_SeqId;
    if(this->m_AudioDualSeqId == 0)
    {
        ERROR_INFO("[%d]request audio dual seqid (%d)(%d)\n",pComm->m_SeqId,this->m_AudioDualSeqId,pComm->m_SeqId);
        this->m_AudioDualSeqId = 1;
    }


    pAudioDec->s_AudioId = 0;
    pAudioDec->s_Codec = (CodecType)pStartTalk->m_ucEncodeType;
    n32 = PROTO_TO_HOST32(pStartTalk->m_uSamplesPerSec);
    pAudioDec->s_SampleFreq = n32;
    pAudioDec->s_BitWidth = pStartTalk->m_uBitsPerSample;
    pAudioDec->s_ChannelNum = pStartTalk->m_ucChan;
    n16 = PROTO_TO_HOST16(pStartTalk->s_FrameRate);
    pAudioDec->s_FrameRate = n16;
    n16 = PROTO_TO_HOST16(pStartTalk->s_BitRate);
    pAudioDec->s_BitRate = n16;
    n16 = PROTO_TO_HOST16(pStartTalk->s_Volume);
    pAudioDec->s_Volume = n16;
    n16 = PROTO_TO_HOST16(pStartTalk->s_AecFlag);
    pAudioDec->s_AecFlag = n16;
    n16 = PROTO_TO_HOST16(pStartTalk->s_AecDelayTime);
    pAudioDec->s_AecDelayTime = n16;
    DEBUG_BUFFER_FMT(pAudioDec,sizeof(*pAudioDec),"audio dec [0x%p]",pAudioDec);
    DEBUG_BUFFER_FMT(pComm->m_Data,pComm->m_DataLen,"[%d]data from ",this->GetSocket());


    /*now we should start for the handle*/
    wait = this->m_pSvrMgmt->StartAudioDecoder(this->GetSocket(),pAudioDec);
    if(wait < 0)
    {
        ret = wait;
        goto fail;
    }
    else if(wait > 0)
    {
        DEBUG_INFO("\n");
        /*now to test if we have to send the command*/
        ret = this->m_pSvrMgmt->RequestAudioStart(this,pAudioDec);
        if(ret < 0)
        {
            goto fail;
        }

        DEBUG_INFO("\n");
        this->__SetState(sdk_client_stream_audio_dual_start_state);
        /*stop write timer ,so we should handle no sending data*/
        this->__StopWriteIo();
        this->__StopWriteTimer();
        DEBUG_INFO("\n");

        /*waiting for the sys_server response ,if we do not have this ok*/
        ret = this->__StartWaitSysTimer();
        if(ret < 0)
        {
            goto fail;
        }
        DEBUG_INFO("\n");
    }
    else
    {
        ERROR_INFO("[%d]StreamAudioDualOpenHandle get no wait for AudioDecoderStart\n",this->GetSocket());
        ret = -ETIMEDOUT;
        goto fail;

    }
    if(pAudioDec)
    {
        free(pAudioDec);
    }
    pAudioDec = NULL;

    FreeComm(pComm);
    return 0;
fail:
    if(pAudioDec)
    {
        free(pAudioDec);
    }
    pAudioDec = NULL;
    return ret;
}



