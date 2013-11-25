
#include <sdk_audio_buffer.h>
#include <sdk_server_debug.h>

SdkAudioBuffer::SdkAudioBuffer(sdk_audio_format_enum_t format,int maxpacks,int maxclients,int * pRunningBits) :
    m_Format(format),
    m_MaxClients(maxclients),
    m_MaxPacks(maxpacks),
    m_pRunningBits(pRunningBits)
{
	DEBUG_INFO("maxpacks %d (%d)\n",m_MaxPacks,maxpacks);
    m_Started = 0;
	m_AudioInfoInited = 0;
    memset(&m_AudioInfo,0,sizeof(m_AudioInfo));
    m_pPacks = NULL;
    m_StartIdx = 0;
    m_EndIdx = 0;
    m_PackSize = 0;
    SDK_ASSERT(m_pClientBuffers.size() == 0);
    m_pStream = NULL;
}


void SdkAudioBuffer::__FreeResource()
{
    unsigned int i;
    if(this->m_pPacks)
    {
        for(i=0; i<this->m_MaxPacks; i++)
        {
            this->__FreeStreamPack(this->m_pPacks[i]);
        }
        free(this->m_pPacks);
    }
    this->m_pPacks = NULL;
	this->m_StartIdx = 0;
	this->m_EndIdx	= 0;
	this->m_PackSize = 0;
	return ;
}

void SdkAudioBuffer::__FreeClientBuffer(client_buffer_t * pCBuffer)
{
    /*now to set all the things to default*/
    if(pCBuffer)
    {
        this->__FreeStreamPack(pCBuffer->m_pBlock);
        free(pCBuffer);
    }
    return ;
}

void SdkAudioBuffer::__ClearClientBuffers(void)
{
    client_buffer_t *pClientBuffer=NULL;
    while(this->m_pClientBuffers.size() > 0)
    {
        SDK_ASSERT(pClientBuffer == NULL);
        pClientBuffer = this->m_pClientBuffers[0];
        this->m_pClientBuffers.erase(this->m_pClientBuffers.begin());
        this->__FreeClientBuffer(pClientBuffer);
        pClientBuffer = NULL;
    }

    return ;
}

enum_cb_state_t SdkAudioBuffer::__ChangeClientBufferState(client_buffer_t * pCBuffer,enum_cb_state_t state,const char * file ,int lineno)
{
	enum_cb_state_t oldstate = pCBuffer->m_State;
	pCBuffer->m_State = state;

	if (file)
	{
		DEBUG_INFO("Change CBuffer[%d] (%d) => (%d)\n",pCBuffer->m_Sock,oldstate,state);
	}
	return oldstate;
}

void SdkAudioBuffer::__InitClientBuffer(client_buffer_t * pCBuffer,int sock,sessionid_t sesid)
{
	SDK_ASSERT(pCBuffer->m_pBlock == NULL);
	memset(pCBuffer,0,sizeof(*pCBuffer));
	pCBuffer->m_Sock = sock;
    pCBuffer->m_SesId = sesid;
    pCBuffer->m_State = cb_end_state;
    pCBuffer->m_Failed = 0;
    pCBuffer->m_CurIdx = 0;
    pCBuffer->m_SourceIdx = 0;
    pCBuffer->m_WriteLen = 0;
    pCBuffer->m_TotalLen = 0;
    memset(&(pCBuffer->m_GsspHeader),0,sizeof(pCBuffer->m_GsspHeader));
    pCBuffer->m_GsspLen = 0;
    memset(&(pCBuffer->m_Header),0,sizeof(pCBuffer->m_Header));
    pCBuffer->m_FrameLen = 0;
    pCBuffer->m_Offset = 0;
    pCBuffer->m_pBlock = NULL;
	return ;
}

void SdkAudioBuffer::__ChangePendingState()
{
    int ret;
    unsigned int i;
    client_buffer_t* pCBuffer=NULL;
    stream_pack_t* pCBlock=NULL;
    int recycle=0;

    for(i=0; i<this->m_pClientBuffers.size() ; i++)
    {
        pCBuffer = this->m_pClientBuffers[i];
        if(pCBuffer->m_Failed)
        {
            /*failed ,so we do not handle this one*/
            continue;
        }

        if(pCBuffer->m_State == cb_end_state)
        {
            /*if not start ,nothing do anything*/
            SDK_ASSERT(pCBuffer->m_pBlock == NULL);
            this->__InitClientBuffer(pCBuffer,pCBuffer->m_Sock,pCBuffer->m_SesId);
            continue;
        }
        else if(pCBuffer->m_State == cb_block_state ||
                pCBuffer->m_State == cb_pending_state)
        {
            /*it is already copied ,so we need not do so*/
            SDK_ASSERT(pCBuffer->m_pBlock);
            /*we change into the pending state*/
            this->__ChangeClientBufferState(pCBuffer,cb_pending_state,__FILE__,__LINE__);
            /*
                  if we have already send all the buffer ,or we have not send any buffer ,so we should need to
                  set for the end_state
            */
            if(pCBuffer->m_WriteLen == 0 || pCBuffer->m_WriteLen == pCBuffer->m_TotalLen)
            {
                this->__FreeStreamPack(pCBuffer->m_pBlock);
                this->__InitClientBuffer(pCBuffer,pCBuffer->m_Sock,pCBuffer->m_SesId);
            }
            continue;
        }

        /*we only in audio ,have seq state no other state*/
        SDK_ASSERT(pCBuffer->m_State == cb_seq_state);
        if(pCBuffer->m_CurIdx < this->m_PackSize)
        {
            /*now check if the block has ,and we have the block index is the same*/
            SDK_ASSERT(this->m_PackSize <= this->m_MaxPacks);
            SDK_ASSERT(this->m_pPacks);
            pCBlock = this->m_pPacks[pCBuffer->m_CurIdx];
            SDK_ASSERT(pCBlock);

            /*now compare for the source idx and some data has been sent*/
            if(pCBlock->m_Idx == pCBuffer->m_SourceIdx &&
				pCBuffer->m_WriteLen > 0 && pCBuffer->m_WriteLen < pCBuffer->m_TotalLen)
            {
                /*now copy the block*/
                ret = this->__CopyBlock(pCBuffer,pCBlock,cb_pending_state);
                if(ret < 0)
                {
                    /*to set it failed ,and will free after this check cycle*/
                    pCBuffer->m_Failed = 1;
                }
            }
            else
            {
                ERROR_INFO("[%d] CBuffer->m_SourceIdx (%d) != [%d] CBlock->m_Idx (%d)\n",
                           i,pCBuffer->m_SourceIdx,pCBuffer->m_CurIdx,pCBlock->m_Idx);
				SDK_ASSERT(pCBuffer->m_pBlock == NULL);
                this->__InitClientBuffer(pCBuffer,pCBuffer->m_Sock,pCBuffer->m_SesId);
            }
        }
        else
        {
            /*this means that we have nothing to handle*/
            SDK_ASSERT(this->m_PackSize == 0);
            this->__InitClientBuffer(pCBuffer,pCBuffer->m_Sock,pCBuffer->m_SesId);
        }

    }


    /*now we should clear all the things on the failed ,it will give the client to get failed*/
    recycle = 1;
    while(recycle)
    {
        /*we put it for recycle not ,if we find a failed client buffer ,set it recycle*/
        recycle = 0;
        for(i=0; i<this->m_pClientBuffers.size() ; i++)
        {
            pCBuffer = this->m_pClientBuffers[i];
            if(pCBuffer->m_Failed)
            {
                this->m_pClientBuffers.erase(this->m_pClientBuffers.begin() + i);
                this->__FreeClientBuffer(pCBuffer);
                /*set for next recycle*/
                recycle = 1;
                break;
            }
        }
    }

    return ;

}


int SdkAudioBuffer::PauseStream()
{
    this->__ChangePendingState();
    if(this->m_pStream)
    {
        delete this->m_pStream;
    }
    this->m_pStream = NULL;
    /*now we should clear for the buffer*/
    this->__FreeResource();
	memset(&(this->m_AudioInfo),0,sizeof(this->m_AudioInfo));
	this->m_AudioInfoInited = 0;
    return 0;
}

void SdkAudioBuffer::StopStream()
{
    /*now to clear all the clients*/
    this->PauseStream();
    this->__ClearClientBuffers();
    this->m_Started = 0;
    return;
}

SdkAudioBuffer::~SdkAudioBuffer()
{
    this->StopStream();
    this->m_pRunningBits = NULL;
    this->m_MaxPacks = 0;
    this->m_MaxClients = 0;
    this->m_Format = audio_format_default;
}

int SdkAudioBuffer::__CopyAudioStreamInfo(SysPkgAudioEncodeCfg * pCfg,sys_stream_info_t * pStreamInfo)
{
	if (pStreamInfo->m_AudioCount < 1)
	{
		return -EINVAL;
	}

	/*now we should copy for the audio*/
	memcpy(pCfg,&(pStreamInfo->m_AudioInfo[0]),sizeof(*pCfg));
	this->m_AudioInfoInited = 1;
	return 0;
}

int SdkAudioBuffer::StartStream(sys_stream_info_t * pStreamInfo)
{
    int ret;


    this->StopStream();
	if (this->m_MaxPacks == 0 )
	{
		return -EINVAL;
	}

	ret = this->__CopyAudioStreamInfo(&(this->m_AudioInfo),pStreamInfo);
	if (ret < 0)
	{
		this->StopStream();
		return ret;
	}
	
    /*now first to start the stream*/
    this->m_pStream = new SdkAudioStream(this->m_Format,this->m_MaxPacks,this->m_pRunningBits);
    ret = this->m_pStream->StartStream();
    if(ret < 0)
    {
        this->StopStream();
        return ret;
    }

    this->m_pPacks = (stream_pack_t**)calloc(sizeof(*(this->m_pPacks)),this->m_MaxPacks);
    if(this->m_pPacks == NULL)
    {
        ret = -errno ? -errno : -1;
        this->StopStream();
        return ret;
    }


    /*for it is started*/
    this->m_Started = 1;
    return 0;
}


int SdkAudioBuffer::ResumeStream(sys_stream_info_t * pStreamInfo)
{
    int ret;
    /*not started stream ,so we do not do any resume*/
    if(this->m_Started == 0)
    {
        /*we should stop stream for it*/
        this->StopStream();
        return -EINVAL;
    }

    this->PauseStream();
	ret = this->__CopyAudioStreamInfo(&(this->m_AudioInfo),pStreamInfo);
	if (ret < 0)
	{
		this->PauseStream();
		return ret;
	}
	
    this->m_pStream = new SdkAudioStream(this->m_Format,this->m_MaxPacks,this->m_pRunningBits);
    ret = this->m_pStream->StartStream();
    if(ret < 0)
    {
        this->PauseStream();
        return ret;
    }

    this->m_pPacks =(stream_pack_t**) calloc(sizeof(*(this->m_pPacks)),this->m_MaxPacks);
    if(this->m_pPacks == NULL)
    {
        ret = -errno ? -errno : -1;
        this->PauseStream();
        return ret;
    }

    return 0;
}

int SdkAudioBuffer::GetStreamStarted()
{
    return this->m_Started;
}

int SdkAudioBuffer::GetClients(std::vector<int>& clisocks)
{
    unsigned int i;
    int count = 0;
    client_buffer_t *pCBuffer=NULL;
    clisocks.clear();

    for(i=0; i<this->m_pClientBuffers.size() ; i++)
    {
        pCBuffer = this->m_pClientBuffers[i];
		/*give all the clients ,and including failed one*/
        if(pCBuffer->m_Sock >= 0)
        {
            clisocks.push_back(pCBuffer->m_Sock);
            count ++;
        }
    }
    return count;
}

client_buffer_t* SdkAudioBuffer::__AllocateClientBuffer(int sock,sessionid_t sesid)
{
	client_buffer_t* pCBuffer=NULL;

	pCBuffer = (client_buffer_t*)calloc(sizeof(*pCBuffer),1);
	if (pCBuffer == NULL)
	{
		return NULL;
	}

	this->__InitClientBuffer(pCBuffer,sock,sesid);
	return pCBuffer;
}

int SdkAudioBuffer::RegisterSock(int sock,sessionid_t sesid)
{
    unsigned int i;
    client_buffer_t *pCBuffer=NULL,*pInsertCBuffer=NULL;

    if(this->m_MaxClients && this->m_pClientBuffers.size() >= this->m_MaxClients)
    {
        return -ENOSPC;
    }


    for(i=0; i<this->m_pClientBuffers.size() ; i++)
    {
        pCBuffer = this->m_pClientBuffers[i];
        if(pCBuffer->m_Sock == sock)
        {
            /*already exist*/
            return -EEXIST;
        }
    }

    pInsertCBuffer = this->__AllocateClientBuffer(sock,sesid);
    if(pInsertCBuffer == NULL)
    {
        return -ENOMEM;
    }
    this->m_pClientBuffers.push_back(pInsertCBuffer);
    return 0;
}

void SdkAudioBuffer::UnRegisterSock(int sock)
{
    unsigned int i;
    client_buffer_t* pCBuffer=NULL;

    for(i=0; i<this->m_pClientBuffers.size() ; i++)
    {
        pCBuffer = this->m_pClientBuffers[i];
        if(pCBuffer->m_Sock == sock)
        {
            this->m_pClientBuffers.erase(this->m_pClientBuffers.begin() +i);
            this->__FreeClientBuffer(pCBuffer);
            break;
        }
        pCBuffer = NULL;
    }

    return ;
}

int SdkAudioBuffer::__FormatMessageHeader(client_buffer_t * pCBuffer,stream_pack_t * pCBlock,unsigned int idx)
{
	int totallen = 0;
	gssp_header_t *pGsspHeader=&(pCBuffer->m_GsspHeader);
	uint16_t n16;
	uint32_t avgbytes,n32;
	audio_frame_header_t *pAudioHeader = &(pCBuffer->m_Header.m_AFrame);
	int nextidx;
	
	

	/*have not initialized ,so we should refused this*/
	if (this->m_AudioInfoInited == 0)
	{
		return -ENOTSUP;
	}
	nextidx = pCBuffer->m_CurIdx;
	nextidx += 1;
	nextidx %= this->m_MaxPacks;

	if ((pCBuffer->m_SourceIdx + 1)!= pCBlock->m_Idx )
	{
		unsigned int i;
		int curidx;
		//stream_pack_t* pCurBlock=NULL;
		DEBUG_INFO("orig[%d]=>[%d](startidx %d endidx %d packsize %d maxpacks %d)Send Idx (%d) != (%d + 1)\n",pCBuffer->m_CurIdx,idx,this->m_StartIdx,this->m_EndIdx,this->m_PackSize,this->m_MaxPacks,pCBlock->m_Idx,pCBuffer->m_SourceIdx);
		for (i=0;i<this->m_PackSize;i++)
		{
			curidx = this->m_StartIdx + i;
			curidx %= this->m_PackSize;
			//pCurBlock = this->m_pPacks[curidx];
			//DEBUG_INFO("[%d][%d] %d\n",i,curidx,pCurBlock->m_Idx);
		}
	}

	pCBuffer->m_WriteLen = 0;
	totallen = 0;
	totallen += GSSP_HEADER_BASE_LEN;
	pCBuffer->m_GsspLen = GSSP_HEADER_BASE_LEN;
	totallen += sizeof(*pAudioHeader);
	pCBuffer->m_FrameLen = sizeof(*pAudioHeader);
	totallen += pCBlock->m_DataLen;

	
	memset(&(pCBuffer->m_GsspHeader),0,sizeof(pCBuffer->m_GsspHeader));
	pGsspHeader->m_Magic[0] = 'G';
	pGsspHeader->m_Magic[1] = 'S';
	pGsspHeader->m_Magic[2] = 'S';
	pGsspHeader->m_Magic[3] = 'P';
    pGsspHeader->m_Flag = 0;
    pGsspHeader->m_Flag |= GSSP_HEADER_FLAG_ENC_DEFAULT;

    pGsspHeader->m_Version =0;
    pGsspHeader->m_Version |= GSSP_HEADER_VERSION_MAJOR;
    pGsspHeader->m_Version |= GSSP_HEADER_VERSION_MINOR;

    n16 = HOST_TO_PROTO16(STREAM_DATA_SEQNUM);
	pGsspHeader->m_SeqNumber = n16;
	n16 = HOST_TO_PROTO16(pCBuffer->m_SesId);
	pGsspHeader->m_SessionId = n16;
	pGsspHeader->m_HeaderLen = GSSP_HEADER_BASE_LEN;
    pGsspHeader->m_Type = GMIS_PROTOCOL_TYPE_MEDIA_DATA;
	n32 = totallen - GSSP_HEADER_BASE_LEN;
	pGsspHeader->m_BodyLength = HOST_TO_PROTO32(n32);
	n32 = 0;
	pGsspHeader->m_CheckSum = HOST_TO_PROTO32(n32);

	pAudioHeader->m_AudioCode = HOST_TO_PROTO32(SDK_STREAM_AD_SEND);
	pAudioHeader->m_FrameType = 'A';

	pAudioHeader->m_EncodeType = this->m_AudioInfo.s_EncodeType;
	pAudioHeader->m_Chan = this->m_AudioInfo.s_Chan;
	pAudioHeader->m_BitsPerSample = this->m_AudioInfo.s_BitsPerSample;
	pAudioHeader->m_SamplePerSec = HOST_TO_PROTO32(this->m_AudioInfo.s_SamplesPerSec);
	pAudioHeader->m_FrameId = HOST_TO_PROTO32(pCBlock->m_Idx);
	pAudioHeader->m_Pts = HOST_TO_PROTO64(pCBlock->m_Pts);
	/*to calculate the bytes*/
	avgbytes = (this->m_AudioInfo.s_SamplesPerSec * this->m_AudioInfo.s_BitsPerSample * this->m_AudioInfo.s_Chan)>> 3;
	pAudioHeader->m_AvgBytesPerSec = HOST_TO_PROTO32(avgbytes);

	pCBuffer->m_TotalLen = totallen;
	pCBuffer->m_SourceIdx = pCBlock->m_Idx;
	pCBuffer->m_CurIdx = idx;

	return 0;
	
}

int SdkAudioBuffer::__SetIov(client_buffer_t * pCBuffer,struct iovec * pIoVec,int & iovlen,int & begin)
{
	unsigned int idx;
	unsigned int offset;
	stream_pack_t* pCurBlock=NULL;

	if (iovlen < 3)
	{
		return -ENOSPC;
	}

	if (pCBuffer->m_WriteLen >= pCBuffer->m_TotalLen)
	{
		return -ENODATA;
	}

	idx = 0;
	if (pCBuffer->m_WriteLen < pCBuffer->m_GsspLen)
	{
		offset = pCBuffer->m_WriteLen;
		pIoVec[idx].iov_base = ((unsigned char*)&(pCBuffer->m_GsspHeader)) + offset;
		pIoVec[idx].iov_len = pCBuffer->m_GsspLen - offset;
		idx += 1;
	}

	if (pCBuffer->m_WriteLen < (pCBuffer->m_GsspLen + pCBuffer->m_FrameLen))
	{
		if (pCBuffer->m_WriteLen <= pCBuffer->m_GsspLen)
		{
			offset = 0;
		}
		else
		{
			offset = (pCBuffer->m_WriteLen - pCBuffer->m_GsspLen);
		}

		pIoVec[idx].iov_base = ((unsigned char*)&(pCBuffer->m_Header.m_AFrame)) + offset;
		pIoVec[idx].iov_len = pCBuffer->m_FrameLen - offset;
		idx += 1;
	}

	if (pCBuffer->m_WriteLen <= (pCBuffer->m_GsspLen + pCBuffer->m_FrameLen))
	{
		offset = 0;
	}
	else
	{
		offset = pCBuffer->m_WriteLen - (pCBuffer->m_GsspLen + pCBuffer->m_FrameLen);
	}

	if (pCBuffer->m_pBlock)
	{
		pIoVec[idx].iov_base = ((unsigned char*)(pCBuffer->m_pBlock->m_pData)) + offset;
		pIoVec[idx].iov_len = pCBuffer->m_pBlock->m_DataLen - offset;
	}
	else
	{
		pCurBlock= this->m_pPacks[pCBuffer->m_CurIdx];
		pIoVec[idx].iov_base = ((unsigned char*)(pCurBlock->m_pData)) + offset;
		pIoVec[idx].iov_len = pCurBlock->m_DataLen - offset;
		
	}
	idx += 1;

	iovlen = idx;
	begin = 0;
	if (pCBuffer->m_WriteLen == 0)
	{
		begin = 1;
	}

	return 0;
}

int SdkAudioBuffer::__GetStreamDataEndState(client_buffer_t * pCBuffer,struct iovec * pIoVec,int & iovlen,int & begin)
{
    int ret = 0;
    stream_pack_t *pCBlock=NULL;

    /*now we should check for state*/
    SDK_ASSERT(pCBuffer->m_State == cb_end_state);
    if(this->m_PackSize == 0)
    {
        /*nothing to get for data*/
        return 0;
    }

    /*we can not get into this pPacks== NULL ,because ,if we get here ,it is just the case pausestream ,and all the client will change into pending state*/
    SDK_ASSERT(this->m_pPacks);
    /*now to get for the buffer and we set the endidx*/
    pCBlock = this->m_pPacks[this->m_EndIdx];
    SDK_ASSERT(pCBlock);
    /*now we should copy this block*/
    /*set the seq state for it*/
    this->__ChangeClientBufferState(pCBuffer,cb_seq_state,__FILE__,__LINE__);
    ret = this->__FormatMessageHeader(pCBuffer,pCBlock,this->m_EndIdx);
    if(ret < 0)
    {
        return ret;
    }

    /**/
    ret = this->__SetIov(pCBuffer,pIoVec,iovlen,begin);
    if(ret < 0)
    {
        return ret;
    }

    return 1;
}

int SdkAudioBuffer::__IsGreaterIdx(unsigned int aidx,unsigned int bidx)
{
	if (aidx > bidx)
	{
		return 1;
	}

	/* this is overflow situation*/
	if (aidx < 0x100 && bidx > (MAX_STREAM_IDX_32 - 0x100))
	{
		return 1;
	}
	return 0;
}


int SdkAudioBuffer::__GetStreamDataSeqState(client_buffer_t * pCBuffer,struct iovec * pIoVec,int & iovlen,int & begin)
{
    int ret=0;
    stream_pack_t *pCBlock=NULL;
    int curidx=0;
    unsigned int i;
	unsigned int expectidx;

    SDK_ASSERT(pCBuffer->m_pBlock==NULL);
    curidx = pCBuffer->m_CurIdx;
    /*if we enter seq state, we must have some */
    SDK_ASSERT(this->m_PackSize > 0);
    pCBlock = this->m_pPacks[curidx];
    SDK_ASSERT(pCBlock);

    if(pCBuffer->m_SourceIdx == pCBlock->m_Idx && pCBuffer->m_WriteLen < pCBuffer->m_TotalLen)
    {
        /*if we have send something ,make sure we can not lost the block ,if the cblock will replace by insert ,it will handled in */
        /*now to set for this*/
        ret = this->__SetIov(pCBuffer,pIoVec,iovlen,begin);
        if(ret < 0)
        {
            return ret;
        }
        return 1;
    }

	expectidx = pCBuffer->m_SourceIdx;
	expectidx += 1;

	/*now for the next one*/
	curidx = pCBuffer->m_CurIdx;
	curidx += 1;
	curidx %= this->m_PackSize;
	pCBlock = this->m_pPacks[curidx];
	if (pCBlock->m_Idx == expectidx)
	{
		ret = this->__FormatMessageHeader(pCBuffer,pCBlock,curidx);
		if (ret < 0)
		{
			return ret;
		}
		ret = this->__SetIov(pCBuffer,pIoVec,iovlen,begin);
		if (ret < 0)
		{
			return ret;
		}
		return 1;
	}

	

    /*now it is not the one ,we should get the next*/
    for(i=0; i<this->m_PackSize; i++)
    {
        curidx = this->m_StartIdx + i;
        curidx %= this->m_PackSize;
        pCBlock = this->m_pPacks[curidx];
        /*it is determined by m_PackSize number*/
        SDK_ASSERT(pCBlock);
        if(this->__IsGreaterIdx(pCBlock->m_Idx,pCBuffer->m_SourceIdx))
        {
            ret = this->__FormatMessageHeader(pCBuffer,pCBlock,curidx);
            if(ret < 0)
            {
                return ret;
            }
            ret = this->__SetIov(pCBuffer,pIoVec,iovlen,begin);
            if(ret < 0)
            {
                return ret;
            }
            return 1;
        }
    }

    /*nothing find*/
    return 0;
}

int SdkAudioBuffer::__GetStreamDataBlockState(client_buffer_t * pCBuffer,struct iovec * pIoVec,int & iovlen,int & begin)
{
    int ret;
    stream_pack_t *pCBlock=NULL,*pFindBlock=NULL;
    int findidx=-1;
    unsigned int i,curidx;

    SDK_ASSERT(pCBuffer->m_State == cb_block_state);

    SDK_ASSERT(pCBuffer->m_pBlock);
    /*because only when the packsize is fulfilled ,that can be a block state buffer*/
    SDK_ASSERT(this->m_MaxPacks == this->m_PackSize);
	SDK_ASSERT(pCBuffer->m_SourceIdx == pCBuffer->m_pBlock->m_Idx);
    if(pCBuffer->m_WriteLen < pCBuffer->m_TotalLen)
    {
        ret = this->__SetIov(pCBuffer,pIoVec,iovlen,begin);
        if(ret < 0)
        {
            return ret;
        }
        return 1;
    }

    /*now it is the last one so we should search for the next one packet*/
    findidx = -1;
    for(i=0; i<this->m_PackSize; i++)
    {
        curidx = this->m_StartIdx + i;
        curidx %= this->m_PackSize;

        pCBlock = this->m_pPacks[curidx];
        if(this->__IsGreaterIdx(pCBlock->m_Idx,pCBuffer->m_SourceIdx))
        {
            findidx = curidx;
            pFindBlock = pCBlock;
            break;
        }
    }

    if(pFindBlock == NULL)
    {
        return 0;
    }

    /*now free the packs and set for the state*/
    this->__FreeStreamPack(pCBuffer->m_pBlock);
    this->__ChangeClientBufferState(pCBuffer,cb_seq_state,__FILE__,__LINE__);
    ret = this->__FormatMessageHeader(pCBuffer,pFindBlock,findidx);
    if(ret < 0)
    {
        return ret;
    }
    ret = this->__SetIov(pCBuffer,pIoVec,iovlen,begin);
    if(ret < 0)
    {
        return ret;
    }
    return 1;
}

int SdkAudioBuffer::__GetStreamDataPendingState(client_buffer_t * pCBuffer,struct iovec * pIoVec,int & iovlen,int & begin)
{
    int ret;
    stream_pack_t *pCBlock=NULL,*pFindBlock=NULL;
    int findidx=-1;
    unsigned int i,curidx;

    SDK_ASSERT(pCBuffer->m_pBlock);
    SDK_ASSERT(pCBuffer->m_State == cb_pending_state);

    if(pCBuffer->m_WriteLen < pCBuffer->m_TotalLen)
    {
        ret = this->__SetIov(pCBuffer,pIoVec,iovlen,begin);
        if(ret < 0)
        {
            return ret;
        }
        return 1;
    }

    /*now just free the pack*/
    this->__FreeStreamPack(pCBuffer->m_pBlock);
    this->__ChangeClientBufferState(pCBuffer,cb_end_state,__FILE__,__LINE__);

    /*only when we have start for pull job ,we get the packsize > 0*/

    /*now to find the test*/
    for(i=0; i<this->m_PackSize; i++)
    {
        curidx = this->m_StartIdx + i;
        curidx %= this->m_PackSize;
        pCBlock = this->m_pPacks[curidx];
        if(this->__IsGreaterIdx(pCBlock->m_Idx,pCBuffer->m_SourceIdx))
        {
            pFindBlock = pCBlock;
            findidx = curidx;
            break;
        }
    }

    if(pFindBlock == NULL)
    {
        return 0;
    }

    /*to set for the buffer state*/
    this->__ChangeClientBufferState(pCBuffer,cb_seq_state,__FILE__,__LINE__);
    ret = this->__FormatMessageHeader(pCBuffer,pFindBlock,findidx);
    if(ret < 0)
    {
        return ret;
    }
    ret = this->__SetIov(pCBuffer,pIoVec,iovlen,begin);
    if(ret < 0)
    {
        return ret;
    }
    return 1;

}


int SdkAudioBuffer::GetStreamData(int sock,struct iovec * pIoVec,int & iovlen,int & begin)
{
    client_buffer_t* pCBuffer=NULL,*pFindCBuffer=NULL;
    unsigned int i;
    int findidx=-1;
    int ret;

    if(this->m_Started == 0)
    {
        return -ENOTSUP;
    }



    findidx = -1;
    for(i=0; i<this->m_pClientBuffers.size() ; i++)
    {
        pCBuffer = this->m_pClientBuffers[i];
        if(pCBuffer->m_Sock == sock)
        {
            pFindCBuffer = pCBuffer;
            findidx = i;
            break;
        }
    }

    if(pFindCBuffer == NULL)
    {
        return -ENODEV;
    }

    if(pFindCBuffer->m_Failed)
    {
        /*failed ,so we should return for value*/
        this->m_pClientBuffers.erase(this->m_pClientBuffers.begin()+findidx);
        this->__FreeClientBuffer(pFindCBuffer);
        return -EFAULT;
    }

    ret = -EPERM;
    switch(pFindCBuffer->m_State)
    {
    case cb_end_state:
        ret = this->__GetStreamDataEndState(pFindCBuffer,pIoVec,iovlen,begin);
        break;
    case cb_seq_state:
        ret = this->__GetStreamDataSeqState(pFindCBuffer,pIoVec,iovlen,begin);
        break;
    case cb_pending_state:
        ret = this->__GetStreamDataPendingState(pFindCBuffer,pIoVec,iovlen,begin);
        break;
    case cb_block_state:
        ret = this->__GetStreamDataBlockState(pFindCBuffer,pIoVec,iovlen,begin);
        break;
    default:
        SDK_ASSERT(0!=0);
        ret = -EPERM;
        break;
    }

    return ret;

}

stream_pack_t* SdkAudioBuffer::__CopyPack(stream_pack_t * pPack)
{
    stream_pack_t* pCBlock=NULL;

    pCBlock =(stream_pack_t*) calloc(sizeof(*pCBlock),1);
    if(pCBlock == NULL)
    {
        return NULL;
    }

    pCBlock->m_pData = malloc(pPack->m_DataLen);
    if(pCBlock->m_pData == NULL)
    {
        this->__FreeStreamPack(pCBlock);
        return NULL;
    }

    pCBlock->m_Idx = pPack->m_Idx;
    pCBlock->m_Type = pPack->m_Type;
    pCBlock->m_DataLen = pPack->m_DataLen;
    pCBlock->m_Pts = pPack->m_Pts;
    memcpy(pCBlock->m_pData,pPack->m_pData,pPack->m_DataLen);
    return pCBlock;
}


int SdkAudioBuffer::__CopyBlock(client_buffer_t * pCBuffer,stream_pack_t * pPack,enum_cb_state_t state)
{
    SDK_ASSERT(pCBuffer->m_pBlock == NULL);
    pCBuffer->m_pBlock = this->__CopyPack(pPack);
    if(pCBuffer->m_pBlock == NULL)
    {
        return -ENOMEM;
    }

    this->__ChangeClientBufferState(pCBuffer,state,__FILE__,__LINE__);

    return 0;
}

int SdkAudioBuffer::__HandleClientInsert(client_buffer_t * pCBuffer,unsigned int insertidx)
{
    int ret=0;
    stream_pack_t* pCBlock=NULL;

    if(pCBuffer->m_State == cb_end_state ||
            pCBuffer->m_State == cb_pending_state ||
            pCBuffer->m_State == cb_block_state)
    {
        /*nothing to handle in*/
        return 0;
    }
    SDK_ASSERT(pCBuffer->m_State == cb_seq_state);

    /*now to compare this*/
    if(pCBuffer->m_CurIdx != insertidx)
    {
        /*not affected*/
        return 0;
    }

    /*it is the for it */
    pCBlock = this->m_pPacks[insertidx];
    SDK_ASSERT(pCBlock);
    if(pCBuffer->m_SourceIdx != pCBlock->m_Idx)
    {
        return 0;
    }

    if( pCBuffer->m_WriteLen >= pCBuffer->m_TotalLen)
    {
        /*we have not send anything yet ,or we have send everything ,so we should just not copy*/
        return 0;
    }

	DEBUG_INFO("insertidx[%d] framenumber %d\n",insertidx,pCBuffer->m_SourceIdx);

    ret = this->__CopyBlock(pCBuffer,pCBlock,cb_block_state);
    if(ret < 0)
    {
        /*mark this is failed*/
        pCBuffer->m_Failed = 1;
        return ret;
    }

    /*we have changed*/
    return 1;
}



int SdkAudioBuffer::__InsertPack(stream_pack_t * pPack)
{
    int ret;
    unsigned int insertidx,startidx;
    stream_pack_t* pCBlock=NULL;
    client_buffer_t *pCBuffer=NULL;
    unsigned int i;
    int affected = 0;

    if(this->m_PackSize < this->m_MaxPacks)
    {
        /*we have free pack space for */
        SDK_ASSERT(this->m_pPacks[this->m_PackSize] == NULL);
        this->m_pPacks[this->m_PackSize] = pPack;
        this->m_EndIdx = this->m_PackSize;
        this->m_PackSize += 1;
        return 0;
    }

    /*it is full,so the endidx and startidx are back-to-back*/
    SDK_ASSERT(((this->m_EndIdx + 1) == this->m_StartIdx) ||
           (this->m_StartIdx == 0 && this->m_EndIdx == (this->m_MaxPacks-1)));

    startidx = this->m_StartIdx;
    insertidx = this->m_EndIdx;
    insertidx += 1;
    insertidx %= this->m_PackSize;

    SDK_ASSERT(startidx == insertidx);
    pCBlock = this->m_pPacks[insertidx];
    SDK_ASSERT(pCBlock);

    for(i=0; i<this->m_pClientBuffers.size(); i++)
    {
        pCBuffer = this->m_pClientBuffers[i];
        if(pCBuffer->m_Failed)
        {
            continue;
        }
        ret = this->__HandleClientInsert(pCBuffer,insertidx);
        if(ret < 0)
        {
            pCBuffer->m_Failed = 1;
        }
        else if(ret > 0)
        {
            affected += 1;
        }
    }

    /*now we should insert this one*/
    this->__FreeStreamPack(pCBlock);
    this->m_pPacks[insertidx] = pPack;
    this->m_EndIdx = insertidx;
    startidx += 1;
    startidx %= this->m_PackSize;
    this->m_StartIdx = startidx;
    return affected;

}

void SdkAudioBuffer::__FreeStreamPack(stream_pack_t * & pPack)
{
	if(pPack)
	{
		if (pPack->m_pData)
		{
			free(pPack->m_pData);
		}
		pPack->m_pData = NULL;
		free(pPack);
	}
	pPack = NULL;
	return ;
}

int SdkAudioBuffer::__GetNotify( std::vector<int>& notifyfds)
{
	unsigned int i;
	int count = 0;
	unsigned int lastidx;
	client_buffer_t *pCBuffer;

	notifyfds.clear();

	lastidx = this->m_EndIdx;
	if (lastidx==0 && this->m_MaxPacks == this->m_PackSize)
	{
		lastidx = this->m_MaxPacks;
	}
	else if (lastidx == 0)
	{
		/*it is the last because we have not set for the packsize == 1*/
		SDK_ASSERT(this->m_PackSize == 1);
		lastidx = 0;
	}
	else
	{
		lastidx -= 1;
	}

	for (i=0;i<this->m_pClientBuffers.size() ;i++)
	{
		pCBuffer = this->m_pClientBuffers[i];
		if (pCBuffer->m_CurIdx == lastidx  && pCBuffer->m_SourceIdx == this->m_pPacks[lastidx]->m_Idx && pCBuffer->m_WriteLen == pCBuffer->m_TotalLen)
		{
			/*last time has write over ,so we assume client is waiting*/
		}
		notifyfds.push_back(pCBuffer->m_Sock);
		count += 1;
	}

	return count;
}


int SdkAudioBuffer::PullStreamData(std::vector<int>& notifysock)
{
    int ret;
    stream_pack_t *pStreamPack=NULL;

    if(this->m_Started == 0 || this->m_pStream == NULL)
    {
        /*not started ,so we should not set */
        return 0;
    }

    pStreamPack = (stream_pack_t*)calloc(sizeof(*pStreamPack),1);
    if(pStreamPack == NULL)
    {
        return -ENOMEM;
    }

    ret = this->m_pStream->PullStreamData(pStreamPack);
    if(ret <=0)
    {
        this->__FreeStreamPack(pStreamPack);
        pStreamPack = NULL;
        return ret;
    }

    /*ok ,this is the insert pack*/
    ret = this->__InsertPack(pStreamPack);
    if(ret < 0)
    {
        this->__FreeStreamPack(pStreamPack);
        return ret;
    }

    ret = this->__GetNotify(notifysock);
    if(ret < 0)
    {
        return ret;
    }

    return 1;
}

#define POINTER_IN_RANGE(ptr,startptr,len) \
do\
{\
	SDK_ASSERT(((ptr_t)(ptr)) >= ((ptr_t)(startptr)));\
	SDK_ASSERT(((ptr_t)(ptr)) <= (((ptr_t)(startptr))+(len)));\
}while(0)


int SdkAudioBuffer::__ForwardAssert(client_buffer_t * pCBuffer,struct iovec * pIoVec,int iovlen,int forwardlen)
{
    stream_pack_t* pCBlock;
    int iototallen=0;
    unsigned int curlen ;
    SDK_ASSERT(iovlen <= 3 && iovlen >= 1);
    if(iovlen == 3)
    {
        POINTER_IN_RANGE(pIoVec[0].iov_base,&(pCBuffer->m_GsspHeader),pCBuffer->m_GsspLen);
        curlen = (pCBuffer->m_GsspLen - ((ptr_t)pIoVec[0].iov_base - (ptr_t)&(pCBuffer->m_GsspHeader)));
        SDK_ASSERT(pIoVec[0].iov_len== curlen);
        iototallen += curlen;
        POINTER_IN_RANGE(pIoVec[1].iov_base,&(pCBuffer->m_Header.m_IFrame),pCBuffer->m_FrameLen);
        curlen = pCBuffer->m_FrameLen;
        SDK_ASSERT(pIoVec[1].iov_len == curlen);
        iototallen += curlen;
        if(pCBuffer->m_pBlock)
        {
            POINTER_IN_RANGE(pIoVec[2].iov_base,(pCBuffer->m_pBlock->m_pData),(pCBuffer->m_pBlock->m_DataLen));
            curlen = pCBuffer->m_pBlock->m_DataLen;
        }
        else
        {
            pCBlock = this->m_pPacks[pCBuffer->m_CurIdx];
            SDK_ASSERT(pCBlock);
            POINTER_IN_RANGE(pIoVec[2].iov_base,pCBlock->m_pData,pCBlock->m_DataLen);
            curlen = pCBlock->m_DataLen;
        }
        SDK_ASSERT(curlen == pIoVec[2].iov_len);
        iototallen += curlen;
    }
    else if(iovlen == 2)
    {
        POINTER_IN_RANGE(pIoVec[0].iov_base,&(pCBuffer->m_Header.m_AFrame),pCBuffer->m_FrameLen);
        curlen = (pCBuffer->m_FrameLen - (((ptr_t)(pIoVec[0].iov_base)) - ((ptr_t)(&(pCBuffer->m_Header.m_AFrame)))));
        SDK_ASSERT(pIoVec[0].iov_len == curlen);
        iototallen += curlen;
        if(pCBuffer->m_pBlock)
        {
            POINTER_IN_RANGE(pIoVec[1].iov_base,(pCBuffer->m_pBlock->m_pData),(pCBuffer->m_pBlock->m_DataLen));
            curlen = pCBuffer->m_pBlock->m_DataLen;
        }
        else
        {
            pCBlock = this->m_pPacks[pCBuffer->m_CurIdx];
            SDK_ASSERT(pCBlock);
            POINTER_IN_RANGE(pIoVec[1].iov_base,pCBlock->m_pData,pCBlock->m_DataLen);
            curlen = pCBlock->m_DataLen;
        }
        SDK_ASSERT(pIoVec[1].iov_len == curlen);
        iototallen += curlen;
    }
    else if(iovlen == 1)
    {
        if(pCBuffer->m_pBlock)
        {
            POINTER_IN_RANGE(pIoVec[0].iov_base,(pCBuffer->m_pBlock->m_pData),(pCBuffer->m_pBlock->m_DataLen));
            curlen = (pCBuffer->m_pBlock->m_DataLen - (((ptr_t)(pIoVec[0].iov_base)) - ((ptr_t)(pCBuffer->m_pBlock->m_pData))));


        }
        else
        {
            pCBlock = this->m_pPacks[pCBuffer->m_CurIdx];
            SDK_ASSERT(pCBlock);
            POINTER_IN_RANGE(pIoVec[0].iov_base,pCBlock->m_pData,pCBlock->m_DataLen);
            curlen = (pCBlock->m_DataLen - (((ptr_t)(pIoVec[0].iov_base))-((ptr_t)(pCBlock->m_pData))));
        }
        SDK_ASSERT(curlen == pIoVec[0].iov_len);
        iototallen += curlen;
    }

    SDK_ASSERT(forwardlen <= iototallen);

    return 0;

}

int SdkAudioBuffer::__ForwardStreamDataInner(client_buffer_t * pCBuffer,struct iovec * pIoVec,int iovlen,int forwardlen)
{
	this->__ForwardAssert(pCBuffer,pIoVec,iovlen,forwardlen);
	/*now we should set for it */
	pCBuffer->m_WriteLen += forwardlen;
	
	return pCBuffer->m_WriteLen == pCBuffer->m_TotalLen ? 1 : 0;
}


int SdkAudioBuffer::ForwardStreamData(int sock,struct iovec * pIoVec,int iovlen,int forwardlen)
{
    int ret;
    client_buffer_t *pCBuffer=NULL,*pFindCBuffer=NULL;
    unsigned int i;

    for(i=0; i<this->m_pClientBuffers.size() ; i++)
    {
        pCBuffer = this->m_pClientBuffers[i];
        if(pCBuffer->m_Sock == sock)
        {
            pFindCBuffer = pCBuffer;
            break;
        }
    }
	

    if(pFindCBuffer == NULL)
    {
        return -ENODEV;
    }

	if (pFindCBuffer->m_Failed)
	{
		return -EINVAL;
	}

    switch(pFindCBuffer->m_State)
    {
    case cb_seq_state:
    case cb_pending_state:
    case cb_block_state:
        ret = this->__ForwardStreamDataInner(pCBuffer,pIoVec,iovlen,forwardlen);
        break;
    default:
        SDK_ASSERT(0!=0);
        ret = -EPERM;
        break;
    }

    return ret;
}

void SdkAudioBuffer::__DebugCBuffer(client_buffer_t * pCBuffer)
{
    stream_pack_t * pPack;
    DEBUG_INFO("audiotype[%d]sock[%d] state %d curidx %d sourceidx %d\n",this->m_Format,
               pCBuffer->m_Sock,pCBuffer->m_State,pCBuffer->m_CurIdx,
               pCBuffer->m_SourceIdx);
    if (pCBuffer->m_pBlock)
    {
        DEBUG_INFO("block idx %d pts (%lld)\n",pCBuffer->m_pBlock->m_Idx,
                   pCBuffer->m_pBlock->m_Pts);
    }
    DEBUG_INFO("startidx %d endidx %d packsize %d\n",this->m_StartIdx,this->m_EndIdx,this->m_PackSize);
    if (this->m_pPacks && this->m_pPacks[pCBuffer->m_CurIdx])
    {
        pPack = this->m_pPacks[pCBuffer->m_CurIdx];
        DEBUG_INFO("at[%d] %d pts(%lld)\n",
                   pCBuffer->m_CurIdx,
                   pPack->m_Idx,
                   pPack->m_Pts);
    }

    return ;
}


void SdkAudioBuffer::DebugClientBuffer(int sock)
{
    unsigned int i;
    client_buffer_t *pCBuffer=NULL,*pFindCBuffer=NULL;
    if (this->m_Started == 0 ||
            this->m_PackSize == 0)
    {
        return ;
    }

    for (i=0; i<this->m_pClientBuffers.size(); i++)
    {
        pCBuffer = this->m_pClientBuffers[i];
        if (pCBuffer->m_Sock == sock)
        {
            pFindCBuffer = pCBuffer;
            break;
        }
    }

    if (pFindCBuffer == NULL)
    {
        ERROR_INFO("sock %d clientbuffers %d\n",sock,
                   this->m_pClientBuffers.size());
        return ;
    }

    this->__DebugCBuffer(pFindCBuffer);
    return;
}

