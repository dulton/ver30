
#include <sdk_server_client.h>
#include <sdk_server_mgmt.h>
#include <sdk_server_debug.h>
#include <memory>
#include <sdk_sys_cp.h>

int SdkServerClient::__HandleCommandRead(sdk_client_comm_t*& pComm)
{
    int ret;
    /*read this comm and put it into the */
    if(pComm->m_Type == GMIS_PROTOCOL_TYPE_CONF)
    {
        ret = this->__PushSysReqVec(pComm);
        if(ret < 0)
        {
            return ret;
        }
    }
    else
    {
        ret = this->__LogoutHandle(pComm);
        if(ret < 0)
        {
            return ret;
        }

        free(pComm);
        pComm = NULL;
    }

    return 1;
}

int SdkServerClient::__ReadCommandIo()
{
    int ret;
    sdk_client_comm_t *pComm=NULL;
    int hasread=0;

try_again:
    SDK_ASSERT(pComm == NULL);
    ret = this->m_pSock->Read();
    if(ret < 0)
    {
        return ret;
    }
    else if(ret == 0)
    {
        return hasread;
    }

    hasread ++;
    SDK_ASSERT(ret > 0);
    pComm = this->m_pSock->GetRead();
    SDK_ASSERT(pComm);

    if(pComm->m_Type != GMIS_PROTOCOL_TYPE_CONF &&
            pComm->m_Type != GMIS_PROTOCOL_TYPE_LOGGIN)
    {
        free(pComm);
        return -EINVAL;
    }

    ret = this->__HandleCommandRead(pComm);
    if(ret < 0)
    {
        free(pComm);
        return ret;
    }
    pComm = NULL;

    goto try_again;


    return 0;
}


int SdkServerClient::__PushSysReqVec(sdk_client_comm_t*& pComm)
{
    int ret;

    //DEBUG_INFO("\n");
    ret = this->m_pSvrMgmt->PushSysReq(pComm);
    if(ret < 0)
    {
        return ret;
    }

    return 0;
}

int SdkServerClient::PushSysResp(sdk_client_comm_t*& pComm)
{
    return this->__PushSysRespVec(pComm);
}

int SdkServerClient::__HandleSdkServerInnerResp(sdk_client_comm_t * & pComm)
{
    std::auto_ptr<SdkSysCp> pSysCp2(new SdkSysCp());
    SdkSysCp *pSysCp = pSysCp2.get();
    int ret;
    uint32_t code;

    ret = pSysCp->ParsePkg(pComm);
    if(ret < 0)
    {
        ERROR_INFO("[%d] could not parse package error(%d)\n",this->GetSocket(),ret);
        return ret;
    }

    /*now we should get the data*/
    ret= pSysCp->GetCode(code);
    if(ret < 0)
    {
        ERROR_INFO("[%d] could not get code (%d)\n",this->GetSocket(),ret);
        return ret;
    }

    switch(code)
    {
    case SYSCODE_START_AUDIO_DECODE_RSP:
        ret = this->StartAudioDecodeCallBack(pComm);
        break;
    case SYSCODE_STOP_AUDIO_DECODE_RSP:
        ret = this->StopAudioDecodeCallBack(pComm);
        break;
    default:
        ERROR_INFO("[%d]code 0x%08x not handled\n",this->GetSocket(),code);
        ret = 0;
        break;
    }

    SDK_ASSERT(pComm == NULL || ret <= 0);


    return ret;

}


/**********************************************
*  return 1 ,means handled and pComm==NULL
*  return 0 ,means not handled ,and pComm != NULL
*  return negative error code ,means not handle properly ,and the pComm != NULL
**********************************************/
int SdkServerClient::__PushSysRespVec(sdk_client_comm_t*& pComm)
{
    int ret,res;

    if(pComm->m_SesId == SDK_SERVER_PRIVATE_SESID && pComm->m_SeqId == this->GetSocket())
    {
        /*this is the inner sdk server send to the sys_server ,so we should do this handle*/
        return  this->__HandleSdkServerInnerResp(pComm);
    }
    else if(pComm->m_SesId == this->m_SessionId)
    {
        /*now to compare whether this is for us*/

        /*now we should set for the conf*/
        if(pComm->m_Frag)
        {
            sdk_client_comm_t *pReassemble=NULL;
            ret = this->__ReassembleFragResp(pComm,&pReassemble);
            if(ret < 0)
            {
                res = this->__StartFailTimer();
                if(res < 0)
                {
                    ERROR_INFO("[%d]could not stop fail\n",this->m_pSock->GetSocket());
                }
                return ret;
            }
            else if(ret == 0)
            {
                /*ret == 0 means not assembly the fragmentation ,so we handled it ,so it will ok
                we do not check for the pComm because it will removed*/
                SDK_ASSERT(pComm == NULL);
                return 1;
            }
            SDK_ASSERT(pComm == NULL);

            /*now it is insert ,so we should make sure */
            SDK_ASSERT(pReassemble);
            if(this->m_RespVec.size() == 0 && this->m_pSock->IsWriteSth() == 0)
            {
                /*now we should set the write io*/
                this->__StopWriteIo();
                ret = this->__StartWriteIo();
                if(ret < 0)
                {
                    res = this->__StartFailTimer();
                    if(res < 0)
                    {
                        ERROR_INFO("could not stop fail %d\n",this->m_pSock->GetSocket());
                    }
                    return ret;
                }

                ret = this->__ResetWriteTimer();
                if(ret < 0)
                {
                    res = this->__StartFailTimer();
                    if(res < 0)
                    {
                        ERROR_INFO("could not stop fail %d\n",this->m_pSock->GetSocket());
                    }
                    return ret;
                }


                /*we should put into the data this will give the data for */
                ret = this->m_pSock->PutData(pReassemble);
                if(ret < 0)
                {
                    free(pReassemble);
                    pReassemble = NULL;
                    res = this->__StartFailTimer();
                    if(res < 0)
                    {
                        ERROR_INFO("[%d]could not stop fail\n",this->m_pSock->GetSocket());
                    }
                    return ret;
                }
                pReassemble = NULL;
                FreeComm(pComm);
                return 1;
            }
            else
            {
                this->m_RespVec.push_back(pReassemble);
                pReassemble= NULL;
            }
            FreeComm(pComm);
            return 1;

        }
        else
        {
            /*this means that we have discard all the frag vectors ,so discard this one */
            this->__ClearFragVecs();
            if(this->m_RespVec.size() == 0 && this->m_pSock->IsWriteSth() == 0)
            {

                /*now we should set the write io*/
                this->__StopWriteIo();
                ret = this->__StartWriteIo();
                if(ret < 0)
                {
                    res = this->__StartFailTimer();
                    if(res < 0)
                    {
                        ERROR_INFO("could not stop fail %d\n",this->m_pSock->GetSocket());
                    }
                    return ret;
                }

                ret = this->__ResetWriteTimer();
                if(ret < 0)
                {
                    res = this->__StartFailTimer();
                    if(res < 0)
                    {
                        ERROR_INFO("could not stop fail %d\n",this->m_pSock->GetSocket());
                    }
                    return ret;
                }


                /*we should put into the data this will give the data for */
                ret = this->m_pSock->PutData(pComm);
                if(ret < 0)
                {
                    res = this->__StartFailTimer();
                    if(res < 0)
                    {
                        ERROR_INFO("could not stop fail %d\n",this->m_pSock->GetSocket());
                    }
                    return ret;
                }

                pComm=NULL;
                return 1;
            }
            else
            {
                this->m_RespVec.push_back(pComm);
                pComm = NULL;
            }
            return 1;
        }
        SDK_ASSERT(pComm == NULL);
        return 1;
    }
    return 0;

}

int SdkServerClient::__WriteCommandIo()
{
    int writepacket=0;
    int ret;
try_again:
    if(this->m_pSock->IsWriteSth())
    {
        ret = this->m_pSock->Write();
        if(ret < 0)
        {
            return ret;
        }
        else if(ret == 0)
        {
            if(writepacket)
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
        this->m_pSock->ClearWrite();
        writepacket += 1;
        /*passdown for write put*/
    }

    if(this->m_RespVec.size() > 0)
    {
        sdk_client_comm_t *pComm =NULL;
        pComm = this->m_RespVec[0];
        this->m_RespVec.erase(this->m_RespVec.begin());
        ret = this->m_pSock->PutData(pComm);
        if(ret < 0)
        {
            free(pComm);
            return ret;
        }
        pComm = NULL;
        goto try_again;
    }


    /*nothing to write*/
    this->__StopWriteIo();
    this->__StopWriteTimer();
    return 0;
}


int SdkServerClient::__LogoutHandle(sdk_client_comm_t * pComm)
{
    /*
           when logout we just return error code ,this will cause the SdkServerClient deleted ,and it will make the
           session deleted ok
    */
    return -EPERM;
}

int SdkServerClient::__HandleUpgradeRead(sdk_client_comm_t * & pComm)
{
    return -ENOTSUP;
}

int SdkServerClient::PushAlarmComm(sdk_client_comm_t * pComm)
{
    int ret,res;
    sdk_client_comm_t* pRetComm=NULL;
    SDK_ASSERT(this->__GetState() == sdk_client_message_state);
    SDK_ASSERT(pComm->m_Frag == 0);

    /*now allocate the new pRetComm*/
    pRetComm = AllocateComm(pComm->m_DataLen);
    if(pRetComm == NULL)
    {
        ret = GETERRNO();
        goto fail;
    }

    pRetComm->m_SesId = this->m_SessionId;
    pRetComm->m_Priv = this->m_Priv;
    pRetComm->m_ServerPort = 0;
    pRetComm->m_LocalPort = 0;
    pRetComm->m_SeqId = pComm->m_SeqId;
    pRetComm->m_Type = GMIS_PROTOCOL_TYPE_WARNING;
    pRetComm->m_FHB = 0;
    pRetComm->m_Frag = 0;
    pRetComm->m_DataId = 0;
    pRetComm->m_Offset = 0;
    pRetComm->m_Totalsize = 0;
    pRetComm->m_DataLen = pComm->m_DataLen;
    memcpy(pRetComm->m_Data,pComm->m_Data,pComm->m_DataLen);

    ret = this->__PushSysRespVec(pRetComm);
    if(ret < 0)
    {
        ret = GETERRNO();
        goto fail;
    }
    SDK_ASSERT(pRetComm == NULL);

    return 0;

fail:
    SDK_ASSERT(ret > 0);
    res = this->__StartFailTimer();
    if(res < 0)
    {
        res = GETERRNO();
        ERROR_INFO("[%d] could not StartFail Timer Error(%d)\n",this->GetSocket(),res);
        this->__BreakOut();
    }
    FreeComm(pRetComm);
    SETERRNO(ret);
    return -ret;
}

int SdkServerClient::__HandleMessageRead(sdk_client_comm_t * & pComm)
{
    int ret,err=0;
    sessionid_t sesid;
    privledge_t priv;
    sdk_client_comm_t *pRetComm=NULL;
    int expiretime,keeptime;

    if(pComm->m_Type != GMIS_PROTOCOL_TYPE_LOGGIN)
    {
        return 0;
    }

    if(pComm->m_FHB == 0 && pComm->m_Type == GMIS_PROTOCOL_TYPE_LOGGIN)
    {
        ret = EINVAL;
        ERROR_INFO("[%d]Message Not In The HeartBeat\n",this->GetSocket());
        goto fail;
    }


    sesid = pComm->m_SesId;
    if(this->m_SessionId != 0 && sesid != this->m_SessionId)
    {
        ret = EINVAL;
        ERROR_INFO("[%d]sessionid (%d) != comm sessionid(%d)\n",this->GetSocket(),
                   this->m_SessionId,sesid);
        goto fail;
    }
    ret = this->m_pSvrMgmt->SessionRenew(sesid,priv,expiretime,keeptime,err);
    if(ret < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("[%d] SessionNew(%d) Error(%d)\n",this->GetSocket(),sesid,ret);
        goto fail;
    }

    /*now set the sessionid */
    this->m_SessionId = sesid;
    this->m_Priv = priv;

    pRetComm = AllocateComm(sizeof(login_response_t));
    if(pRetComm == NULL)
    {
        ret = GETERRNO();
        goto fail;
    }


    ret = this->__LoginSuccResponse(pComm,pRetComm,sesid);
    if(ret < 0)
    {
        ret = GETERRNO();
        goto fail;
    }
    /*we put login type or warning type into the body*/
    pRetComm->m_Type = pComm->m_Type;

    /*nothing for read ,just call the read ok*/
    ret = this->__PushSysRespVec(pRetComm);
    if(ret < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("[%d]PushSysRespVec Error(%d)\n",this->GetSocket(),ret);
        goto fail;
    }
    SDK_ASSERT(pRetComm==NULL);

    return 0;
fail:
    SDK_ASSERT(ret > 0);
    FreeComm(pRetComm);
    SETERRNO(ret);
    return -ret;
}


int SdkServerClient::__WriteUpgradeIo()
{
    return -ENOTSUP;
}

int SdkServerClient::__ReadMessageIo()
{
    int ret;
    sdk_client_comm_t* pComm = NULL;
    SDK_ASSERT(this->m_pSock);
    ret= this->m_pSock->Read();
    if(ret < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("[%d] ReadIo Error(%d)\n",this->GetSocket(),ret);
        SETERRNO(ret);
        return -ret;
    }
    else if(ret == 0)
    {
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

    /*stop read timer ,for it will not make error when read time out*/
    this->__StopReadTimer();

    pComm = this->m_pSock->GetRead();
    SDK_ASSERT(pComm);
    this->m_pSock->ClearRead();
    ret = this->__HandleMessageRead(pComm);
    if(ret < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("[%d]Handle pComm Error(%d)\n",this->GetSocket(),ret);
        goto fail;
    }

    FreeComm(pComm);
    return 0;
fail:
    SDK_ASSERT(ret > 0);
    FreeComm(pComm);
    SETERRNO(ret);
    return -ret;
}


int SdkServerClient::__WriteMessageIo()
{
    int ret;
    sdk_client_comm_t *pComm=NULL;

    if(this->m_pSock->IsWriteSth())
    {
        ret = this->m_pSock->Write();
        if(ret < 0)
        {
            return ret;
        }
        else if(ret == 0)
        {
            return 0;
        }
        this->m_pSock->ClearWrite();
    }

	/*we stop for the next packet get*/
    this->__StopWriteIo();
    this->__StopWriteTimer();

    if(this->m_RespVec.size() > 0)
    {
        pComm = this->m_RespVec[0];
        this->m_RespVec.erase(this->m_RespVec.begin());
        ret = this->m_pSock->PutData(pComm);
        if(ret < 0)
        {
            ret = GETERRNO();
            goto fail;
        }

        ret = this->__StartWriteIo();
        if(ret < 0)
        {
            ret =GETERRNO();
            goto fail;
        }

        ret= this->__StartWriteTimer();
        if(ret < 0)
        {
            ret =GETERRNO();
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


int SdkServerClient::StartAudioDecodeCallBack(sdk_client_comm_t * & pComm)
{
    int ret,res;
    std::auto_ptr<SdkSysCp> pSysCp2(new SdkSysCp());
    SdkSysCp* pSysCp = pSysCp2.get();
    void* pData=NULL;
    uint32_t datalen=0;
    SysPkgAttrHeader* pAttr=NULL;
    SysPkgMessageCode* pMessageCode=NULL;
    uint16_t n16;
    uint32_t n32,result;
    std::auto_ptr<unsigned char> pDataHolder(new unsigned char[32]);
    if(pComm->m_SesId == SDK_SERVER_PRIVATE_SESID && pComm->m_SeqId == this->GetSocket())
    {
        /*now it is the call back for us ,and test we are in the state to handle this call back*/
        if(this->__GetState() == sdk_client_stream_audio_dual_start_state)
        {
            ret = pSysCp->ParsePkg(pComm);
            if(ret < 0)
            {
                goto fail;
            }

            ret = pSysCp->GetData(pData,datalen);
            if(ret < 0)
            {
                goto fail;
            }

            /*this will let the  auto release data buffer*/
            pDataHolder.reset((unsigned char*)pData);

            if(datalen < (sizeof(*pAttr) + sizeof(*pMessageCode)))
            {
                ERROR_INFO("[%d] datalen (%d) < (%d + %d)\n",this->GetSocket(),datalen,sizeof(*pAttr),sizeof(*pMessageCode));
                return 0;
            }
            pAttr = (SysPkgAttrHeader*)pData;
            n16 = PROTO_TO_HOST16(pAttr->s_Type);
            if(n16 != TYPE_MESSAGECODE)
            {
                ERROR_INFO("[%d] type is %d != (%d)\n",this->GetSocket(),n16,TYPE_MESSAGECODE);
                return 0;
            }
            n16 = PROTO_TO_HOST16(pAttr->s_Length);
            if(n16 != datalen)
            {
                ERROR_INFO("[%d] length(%d) != datalen(%d)\n",this->GetSocket(),n16,datalen);
                return 0;
            }

            pMessageCode = (SysPkgMessageCode*)((ptr_t)pData+sizeof(*pAttr));
            n32 = PROTO_TO_HOST32(pMessageCode->s_MessageCode);
            result = n32;


            /*we stop wait sys timer for it will ok*/
            this->__StopWaitSysTimer();


            if(result == 0)
            {
                /*it means this is success ,so we handle this*/
                std::vector<int> succstreamids;
                DEBUG_INFO("\n");
                ret = this->m_pSvrMgmt->StartStreamId(AUDIO_STREAM_ID,this);
                if(ret < 0)
                {
                    ERROR_INFO("[%d]StartStreamId error (%d)\n",this->GetSocket(),ret);

                    /*we make end for this sending */
                    ret = this->__InsertAudioDualNotify(AUDIO_DUAL_OPEN_AUDIO_STREAM_START_FAILED,this->m_AudioDualSeqId);
                    if(ret < 0)
                    {
                        goto fail;
                    }

                    /*at last we do not change state ,because in the sdk_client_stream_audio_dual_start_state state ,will just write send packet information*/

                    FreeComm(pComm);
                    return 1;
                }
                succstreamids.push_back(AUDIO_STREAM_ID);
                ret = this->m_pSvrMgmt->QueryStreamStarted();
                if(ret == STREAM_STATE_PAUSE)
                {
                    /*means we are in pausing state ,so should*/
                    /*change into the stream ids*/
                    this->__SetState(sdk_client_stream_audio_dual_state);
                    this->m_StreamIds = succstreamids;

                    /*to stop for write ok*/
                    this->__StopWriteIo();
                    this->__StopWriteTimer();
                    /*enter this way ,we should start read io*/
                    this->__StopReadIo();
                    ret = this->__StartReadIo();
                    if(ret < 0)
                    {
                        return ret;
                    }

                    /*no reset read timer, because ,we do not notify the client to send data ,
                                    but reset read io because ,we should check if the client close the socket,*/
                    this->m_StreamStarted = 0;
                    FreeComm(pComm);
                    return 1;
                }

                /*this is running-stream because last time we start the AUDIO_STREAM_ID ok*/
                SDK_ASSERT(ret == STREAM_STATE_RUNNING);

                DEBUG_INFO("\n");
                this->__SetState(sdk_client_stream_audio_dual_state);
                this->m_StreamIds = succstreamids;

                ret = this->__InsertAudioDualNotify(0,this->m_AudioDualSeqId);
                if(ret < 0)
                {
                    goto fail;
                }
                /*we make sure that the audio dual seqid ,first time not 0 but later ,all is 0*/
                this->m_AudioDualSeqId = SDK_NOTIFY_SEQID;
                DEBUG_INFO("\n");

                /*now to start all the read io read timer ,and write io ,write timer*/
                this->__StopReadIo();
                ret = this->__StartReadIo();
                if(ret < 0)
                {
                    goto fail;
                }

                ret = this->__ResetReadTimer();
                if(ret < 0)
                {
                    goto fail;
                }

                this->__StopWriteIo();
                ret = this->__StartWriteIo();
                if(ret < 0)
                {
                    goto fail;
                }

                ret = this->__ResetWriteTimer();
                if(ret < 0)
                {
                    goto fail;
                }
                DEBUG_INFO("\n");

                /*start ok*/
                this->m_StreamStarted = 1;
            }
            else
            {
                /*it means this is failed ,so we should */
                ERROR_INFO("[%d]get start decode error(%d:0x%08x)\n",this->GetSocket(),result,result);
                res = this->__InsertAudioDualNotify(result,this->m_AudioDualSeqId);
                if(res < 0)
                {
                    goto fail;
                }
                /*we make sure that the audio dual seqid ,first time not 0 but later ,all is 0*/
                this->m_AudioDualSeqId = SDK_NOTIFY_SEQID;
            }
            /*we have handled this one*/
            FreeComm(pComm);
            return 1;
        }
        else
        {
            ERROR_INFO("[%d] not right state for handling start decoder\n",this->GetSocket());
        }
    }


    return 0;

fail:
    return ret;
}


int SdkServerClient::StopAudioDecodeCallBack(sdk_client_comm_t * & pComm)
{
    if(pComm->m_SesId == SDK_SERVER_PRIVATE_SESID && pComm->m_SeqId == this->GetSocket())
    {
        /*this is call us ,but we should not do anything ,because ,when we stop decode callback and close it immediately ,so
             when we see this handle again ,it is not the current socket ,but newer one that just has the same socket number to handle it,
             we just free the buffer ,and return 1 indicating handled this one because this is the client handling*/
        FreeComm(pComm);
        return 1;
    }

    return 0;
}

