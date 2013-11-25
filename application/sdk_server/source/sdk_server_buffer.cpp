
#include <sdk_server_buffer.h>
#include <sdk_server_debug.h>



SdkServerBuffer::SdkServerBuffer(int streamid,int maxpacks,int* pRunningBits)
    : m_StreamId(streamid)
    ,m_MaxPacks(maxpacks)
    ,m_pRunningBits(pRunningBits)
{
    m_Started = 0 ;
    memset(&m_StreamInfo,0,sizeof(m_StreamInfo));
    m_pPacks = NULL;
    m_StartIdx = 0;
    m_EndIdx = 0;
    m_PackSize = 0;
    SDK_ASSERT(m_pClientBuffers.size() == 0);
    m_pStream = NULL;
}

void SdkServerBuffer::__FreeStreamPack(stream_pack_t*& pPack)
{
    if (pPack)
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


void SdkServerBuffer::__FreeResource()
{
    unsigned int i;

    /*make sure that we should release all the client*/
    SDK_ASSERT(this->m_pClientBuffers.size() == 0);
    if (this->m_pPacks)
    {
        for(i=0; i<this->m_MaxPacks; i++)
        {
            if (this->m_pPacks[i])
            {
                DEBUG_INFO("pack %p data %p\n",this->m_pPacks[i],this->m_pPacks[i]->m_pData);
            }
            this->__FreeStreamPack(this->m_pPacks[i]);
        }

        free(this->m_pPacks);
    }
    this->m_pPacks = NULL;
    this->m_StartIdx = 0;
    this->m_EndIdx = 0;
    this->m_PackSize = 0;

    if (this->m_pStream)
    {
        delete this->m_pStream;
    }
    this->m_pStream = NULL;
    this->m_Started = 0;

    memset(&(this->m_StreamInfo),0,sizeof(this->m_StreamInfo));
    return ;
}

SdkServerBuffer::~SdkServerBuffer()
{

    BACK_TRACE();
    this->StopStream();
    this->m_StreamId = -1;
    this->m_MaxPacks = 0;
    this->m_pRunningBits = NULL;
}

int SdkServerBuffer::GetStreamStarted()
{
    return this->m_Started;
}

void SdkServerBuffer::__InitClientBuffer(client_buffer_t * pCBuffer,int sock,sessionid_t sesid)
{
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

enum_cb_state_t SdkServerBuffer::__ChangeClientBufferState(client_buffer_t * pCBuffer,enum_cb_state_t state,const char * file,int lineno)
{
    enum_cb_state_t oldstate = pCBuffer->m_State;

    if (file)
    {
        //DEBUG_INFO("At[%s:%d]\tchange[%d] state %d to %d\n",file,lineno,pCBuffer->m_Sock,oldstate,state);
    }
    else
    {
        //DEBUG_INFO("change[%d] state %d to %d\n",pCBuffer->m_Sock,oldstate,state);
    }
    pCBuffer->m_State = state;
    return oldstate;
}


client_buffer_t* SdkServerBuffer::__AllocateClientBuffer(int sock,sessionid_t sesid)
{
    client_buffer_t* pCBuffer=NULL;

    pCBuffer = (client_buffer_t*)malloc(sizeof(*pCBuffer));
    if (pCBuffer==NULL)
    {
        return NULL;
    }
    this->__InitClientBuffer(pCBuffer,sock,sesid);
    return pCBuffer;
}

void SdkServerBuffer::__FreeClientBuffer(client_buffer_t* pCBuffer)
{
    if (pCBuffer)
    {
        this->__FreeStreamPack(pCBuffer->m_pBlock);
        free(pCBuffer);
    }
    return ;
}

int SdkServerBuffer::RegisterSock(int sock,sessionid_t sesid)
{
    unsigned int i;
    client_buffer_t *pCBuffer=NULL;
    if (sock < 0)
    {
        return -EINVAL;
    }
    for (i=0; i<this->m_pClientBuffers.size(); i++)
    {
        pCBuffer= this->m_pClientBuffers[i];
        if (pCBuffer->m_Sock == sock)
        {
            return -EEXIST;
        }
    }

    if (this->m_pClientBuffers.size() >= this->m_MaxPacks)
    {
        return -ENOSPC;
    }

    pCBuffer = this->__AllocateClientBuffer(sock,sesid);
    if (pCBuffer == NULL)
    {
        return -ENOMEM;
    }

    this->m_pClientBuffers.push_back(pCBuffer);

    /*if we have started ,not wait ,we can get ,if we not started ,notify the caller wait until
       it is ok*/
    if (this->m_Started)
    {
        return 1;
    }

    return 0;
}

int SdkServerBuffer::GetClients(std::vector<int>& clisocks)
{
    unsigned int i;
    clisocks.clear();
    for (i=0; i<this->m_pClientBuffers.size() ; i++)
    {
        client_buffer_t* pBuffer= this->m_pClientBuffers[i];
        clisocks.push_back(pBuffer->m_Sock);
    }
    return this->m_pClientBuffers.size();
}

void SdkServerBuffer::UnRegisterSock(int sock)
{
    unsigned int i;
    client_buffer_t* pCurCBuffer=NULL;
    for (i=0; i<this->m_pClientBuffers.size(); i++)
    {
        pCurCBuffer = this->m_pClientBuffers[i];
        if (pCurCBuffer->m_Sock == sock)
        {
            this->m_pClientBuffers.erase(this->m_pClientBuffers.begin()+i);
            this->__FreeClientBuffer(pCurCBuffer);
            return;
        }
    }
    return;
}

stream_pack_t* SdkServerBuffer::__CopyPack(stream_pack_t * pPack)
{
    stream_pack_t* pBlock=NULL;
    pBlock = (stream_pack_t*)calloc(sizeof(*pBlock),1);
    if (pBlock == NULL)
    {
        return NULL;
    }

    if (pPack->m_DataLen > 0 && pPack->m_pData)
    {
        pBlock->m_pData= malloc(pPack->m_DataLen);
        if (pBlock->m_pData ==NULL)
        {
            this->__FreeStreamPack(pBlock);
            return NULL;
        }
        memcpy(pBlock->m_pData,pPack->m_pData,pPack->m_DataLen);
    }
    pBlock->m_DataLen = pPack->m_DataLen;
    pBlock->m_Idx = pPack->m_Idx;
    pBlock->m_Pts = pPack->m_Pts;
    pBlock->m_Type = pPack->m_Type;
    return pBlock;
}

int SdkServerBuffer::__CopyBlock(client_buffer_t * pCBuffer,stream_pack_t * pPack,enum_cb_state_t state)
{

    SDK_ASSERT(pCBuffer->m_pBlock == NULL);
    SDK_ASSERT(state == cb_block_state || state == cb_pending_state);
    pCBuffer->m_pBlock = this->__CopyPack(pPack);
    if(pCBuffer->m_pBlock == NULL)
    {
        /*set it is failed*/
        pCBuffer->m_Failed = 1;
        return -ENOMEM;
    }
    SDK_ASSERT(pCBuffer->m_pBlock->m_Idx == pCBuffer->m_SourceIdx);
    this->__ChangeClientBufferState(pCBuffer,state,__FILE__,__LINE__);
    return 0;
}

void SdkServerBuffer::__ChangePendingState()
{
    unsigned int i;
    client_buffer_t *pCBuffer=NULL;
    stream_pack_t *pCBlock=NULL;
    int ret;
    int releaseone;

    /*now if we have some data just copy all the data as the block*/
    for (i=0; i<this->m_pClientBuffers.size() ; i ++)
    {
        /*now test when the client is in the state ,so we should see if it is get data now*/
        pCBuffer = this->m_pClientBuffers[i];
        if (pCBuffer->m_Failed)
        {
            /*it is failed ,so we will not handle it*/
            continue;
        }
        if (pCBuffer->m_State == cb_end_state)
        {
            /*if not start ,nothing do anything*/
            SDK_ASSERT(pCBuffer->m_pBlock == NULL);
            this->__InitClientBuffer(pCBuffer,pCBuffer->m_Sock,pCBuffer->m_SesId);
            continue;
        }
        else if (pCBuffer->m_State == cb_block_state ||
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
            if (pCBuffer->m_WriteLen == 0 || pCBuffer->m_WriteLen == pCBuffer->m_TotalLen)
            {
                this->__FreeStreamPack(pCBuffer->m_pBlock);
                this->__InitClientBuffer(pCBuffer,pCBuffer->m_Sock,pCBuffer->m_SesId);
            }
            continue;
        }

        SDK_ASSERT(pCBuffer->m_State == cb_seq_state ||
               pCBuffer->m_State == cb_i_frame_state);

        /*now we test whether it is waiting the ok*/
        if (pCBuffer->m_CurIdx < this->m_PackSize)
        {
            /*now we test whether it is sending the packet size is already in*/
            SDK_ASSERT(this->m_PackSize <= this->m_MaxPacks);
            SDK_ASSERT(this->m_pPacks);
            pCBlock = this->m_pPacks[pCBuffer->m_CurIdx];
            SDK_ASSERT(pCBlock);

            SDK_ASSERT(pCBuffer->m_WriteLen <= pCBuffer->m_TotalLen);
            /*
                             these conditions are for init client buffer to cb_end_state
                             1, for the index is not pointed,it wait for a long time
                             2, it is search for i_frame ,but pointed not i_frame
                             3, it has already write all things (including gssp header ,frame header ,and data length) or not write anything just 0
                        */
            if (pCBuffer->m_SourceIdx != pCBlock->m_Idx ||
                    (pCBuffer->m_State == cb_i_frame_state &&
                     pCBlock->m_Type != I_FRAME_TYPE) ||
                    (pCBuffer->m_WriteLen == pCBuffer->m_TotalLen || pCBuffer->m_WriteLen == 0))
            {
                this->__InitClientBuffer(pCBuffer,pCBuffer->m_Sock,pCBuffer->m_SesId);
            }
            else
            {
                /*otherthings will copy the packet and set to be pending state*/
                /*now copy the buffer*/
                ret = this->__CopyBlock(pCBuffer,pCBlock,cb_pending_state);
                /*we could not recover ,so do this*/
                if (ret < 0)
                {
                    /*it is failed ,so we can return error on the next time ,get not disturbing it*/
                    pCBuffer->m_Failed = 1;
                }
            }
        }
        else
        {
            /*only ways this happend,because we do not read anything ,so it should be cb_end_state*/
            SDK_ASSERT(this->m_PackSize == 0);
            this->__InitClientBuffer(pCBuffer,pCBuffer->m_Sock,pCBuffer->m_SesId);
        }
    }

    /*release the failed one*/
    do
    {
        releaseone = 0;
        for (i=0; i<this->m_pClientBuffers.size(); i++)
        {
            pCBuffer = this->m_pClientBuffers[i];
            if (pCBuffer->m_Failed)
            {
                this->m_pClientBuffers.erase(this->m_pClientBuffers.begin()+i);
                this->__FreeClientBuffer(pCBuffer);
                releaseone = 1;
                break;
            }
        }
    }
    while(releaseone);

    return ;
}

int SdkServerBuffer::PauseStream()
{
    unsigned int i;

    this->__ChangePendingState();

    if (this->m_pStream)
    {
        delete this->m_pStream;
    }
    this->m_pStream = NULL;

    /*now we should free the data*/
    if (this->m_pPacks)
    {
        for (i=0; i<this->m_MaxPacks; i++)
        {
            this->__FreeStreamPack(this->m_pPacks[i]);
        }

        free(this->m_pPacks);
        this->m_pPacks = NULL;
    }

    this->m_EndIdx = 0;
    this->m_StartIdx = 0;
    this->m_PackSize = 0;
    this->m_Started = 0;
    memset(&(this->m_StreamInfo),0,sizeof(this->m_StreamInfo));
    return 0;
}

int SdkServerBuffer::__CopyStreamInfo(SysPkgEncodeCfg * pCfg,sys_stream_info_t * pStreamInfo)
{
    int i,findidx=-1;

    for (i=0; i<pStreamInfo->m_Count; i++)
    {
        if (pStreamInfo->m_VideoInfo[i].s_Flag == (this->m_StreamId))
        {
            findidx = i;
            break;
        }
    }

    if (findidx < 0)
    {
        return -ENODATA;
    }

    memcpy(pCfg,&(pStreamInfo->m_VideoInfo[findidx]),sizeof(*pCfg));
    return 0;
}

void SdkServerBuffer::StopStream()
{
    client_buffer_t* pCBuffer;
    /*now first to stop all the client*/
    while(this->m_pClientBuffers.size() > 0)
    {
        pCBuffer = this->m_pClientBuffers[0];
        this->m_pClientBuffers.erase(this->m_pClientBuffers.begin());
        this->__FreeClientBuffer(pCBuffer);
    }

    this->__FreeResource();
    return ;

}

int SdkServerBuffer::StartStream(sys_stream_info_t * pStreamInfo)
{
    int ret;

    if (this->m_Started > 0)
    {
        return 0;
    }

    this->StopStream();

    if (this->m_StreamId < 0 ||
            this->m_StreamId >= MAX_STREAM_IDS || this->m_MaxPacks == 0)
    {
        return -EINVAL;
    }
    /*now ,first to change the stream*/
    ret = this->__CopyStreamInfo(&(this->m_StreamInfo),pStreamInfo);
    if (ret < 0)
    {
        this->StopStream();
        return ret;
    }
    SDK_ASSERT(this->m_pStream == NULL);

    this->m_pStream = new SdkServerStream(this->m_StreamId,this->m_StreamInfo.s_Compression);
    ret = this->m_pStream->StartStream();
    if (ret < 0)
    {
        this->StopStream();
        return ret;
    }

    SDK_ASSERT(this->m_pPacks == NULL);
    this->m_pPacks =(stream_pack_t**) calloc(sizeof(*(this->m_pPacks)),this->m_MaxPacks);
    if (this->m_pPacks == NULL)
    {
        this->StopStream();
        return -ENOMEM;
    }


    this->m_StartIdx = 0;
    this->m_EndIdx = 0;
    this->m_PackSize = 0;
    SDK_ASSERT(this->m_pClientBuffers.size() == 0);
    /*started */
    this->m_Started = 1;
    return 0;
}



int SdkServerBuffer::ResumeStream(sys_stream_info_t * pStreamInfo)
{
    int ret;


    /*ok ,to make things ok ,when we from the very sure state*/
    this->PauseStream();

    if (this->m_StreamId < 0 ||
            this->m_StreamId >= MAX_STREAM_IDS || this->m_MaxPacks == 0)
    {
        return -EINVAL;
    }
    ret = this->__CopyStreamInfo(&(this->m_StreamInfo),pStreamInfo);
    if (ret < 0)
    {
        this->PauseStream();
        return ret;
    }
    /*now ,first to change the stream*/
    SDK_ASSERT(this->m_pStream == NULL);

    this->m_pStream = new SdkServerStream(this->m_StreamId,this->m_StreamInfo.s_Compression);
    ret = this->m_pStream->StartStream();
    if (ret < 0)
    {
        this->PauseStream();
        return ret;
    }

    SDK_ASSERT(this->m_pPacks == NULL);
    this->m_pPacks = (stream_pack_t**)calloc(sizeof(*(this->m_pPacks)),this->m_MaxPacks);
    if (this->m_pPacks == NULL)
    {
        this->PauseStream();
        return -ENOMEM;
    }


    this->m_StartIdx = 0;
    this->m_EndIdx = 0;
    this->m_PackSize = 0;
    /*started */
    this->m_Started = 1;
    return 0;

}

int SdkServerBuffer::__IsGreaterIdx(unsigned int aidx,unsigned int bidx)
{
    /*this is loopback  ,and overflow one*/
    if (aidx < 1000 &&
            bidx > (MAX_STREAM_IDX_32 - 1000))
    {
        return 1;
    }
    if (aidx > bidx)
    {
        return 1;
    }

    return 0;
}

int SdkServerBuffer::__GetStreamDataEndState(client_buffer_t * pCBuffer,struct iovec * pIoVec,int & iovlen,int & begin)
{
    int ret;
    unsigned int i,curidx;
    stream_pack_t *pCBlock=NULL;
    int findidx;

    /*now it is end state*/
    SDK_ASSERT(pCBuffer->m_State == cb_end_state);
    SDK_ASSERT(pCBuffer->m_CurIdx == 0);
    SDK_ASSERT(pCBuffer->m_pBlock == NULL);

    if (this->m_Started == 0)
    {
        return 0;
    }

    /*now we should first to test whether it has something to get*/
    if (this->m_PackSize == 0)
    {
        return 0;
    }

    /*it is filled all the packs ,so we should start scan from the startidx and end to end idx*/
    findidx = -1;
    for (i=0; i<this->m_PackSize; i++)
    {
        curidx = this->m_StartIdx + i;
        curidx %= this->m_PackSize;
        pCBlock = this->m_pPacks[curidx];
        SDK_ASSERT(pCBlock);
        if (pCBlock->m_Type == I_FRAME_TYPE)
        {
            findidx = curidx;
            break;
        }

    }



    if (findidx < 0)
    {
        SDK_ASSERT(pCBlock);
        pCBuffer->m_CurIdx = curidx;
        pCBuffer->m_SourceIdx = pCBlock->m_Idx;
        this->__ChangeClientBufferState(pCBuffer,cb_i_frame_state,__FILE__,__LINE__);
        return 0;
    }
    if (iovlen < 3)
    {
        ERROR_INFO("\n");
        return -ENOSPC;
    }

    pCBuffer->m_CurIdx = findidx;
    pCBuffer->m_SourceIdx = pCBlock->m_Idx;
    ret = this->__FormatMessageHeader(pCBuffer,pCBlock);
    if (ret < 0)
    {
        return ret;
    }
    /*we have copied data ,so change to sequence state*/
    this->__ChangeClientBufferState(pCBuffer,cb_seq_state,__FILE__,__LINE__);

    ret = this->__SetIov(pCBuffer,pIoVec,iovlen,begin);
    if (ret < 0)
    {
        ERROR_INFO("\n");
        return ret;
    }
    return 1;
}


int SdkServerBuffer::__SearchUntilIframe(client_buffer_t * pCBuffer,struct iovec * pIoVec,int & iovlen,int & begin)
{
    int ret;
    unsigned int i,curidx;
    stream_pack_t *pCBlock=NULL;
    int findidx;
    /*it is ok ,so we should find the next one
             because come here four cases
             1,  cb_i_frame_state
             2,  cb_seq_state
             3,  cb_block_state
             4,  cb_pending_state
             all is ok
        */
    SDK_ASSERT(this->m_PackSize > 0);
    this->__ChangeClientBufferState(pCBuffer,cb_i_frame_state,__FILE__,__LINE__);

    findidx = -1;
    for (curidx=this->m_StartIdx,i=0; i< this->m_PackSize; curidx += 1,i++,curidx %= this->m_PackSize)
    {
        unsigned int cursourceidx;
        pCBlock = this->m_pPacks[curidx];
        SDK_ASSERT(pCBlock);
        cursourceidx = pCBlock->m_Idx;
        if (this->__IsGreaterIdx(cursourceidx,pCBuffer->m_SourceIdx) == 0)
        {
            continue;
        }
        if (pCBlock->m_Type == I_FRAME_TYPE)
        {
            findidx = curidx;
            break;
        }
    }

    if (findidx < 0)
    {
        /*this is the end ,so we should set to the endidx packs*/
        pCBlock = this->m_pPacks[this->m_EndIdx];
        pCBuffer->m_CurIdx = this->m_EndIdx;
        pCBuffer->m_SourceIdx = pCBlock->m_Idx;
        return 0;
    }

    /*ok ,we find it make sure it is the sequence state ,it will ok when you get next frame time ok*/
    this->__ChangeClientBufferState(pCBuffer,cb_seq_state,__FILE__,__LINE__);
    pCBlock = this->m_pPacks[findidx];
    pCBuffer->m_CurIdx = findidx;
    pCBuffer->m_SourceIdx = pCBlock->m_Idx;
    ret = this->__FormatMessageHeader(pCBuffer,pCBlock);
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

int SdkServerBuffer::__GetStreamDataIFrameState(client_buffer_t * pCBuffer,struct iovec * pIoVec,int & iovlen,int & begin)
{
    int ret;
    unsigned int curidx,expectidx;
    stream_pack_t *pCBlock=NULL;

    SDK_ASSERT(pCBuffer->m_State == cb_i_frame_state);
    SDK_ASSERT(pCBuffer->m_pBlock == NULL);
    /*because 4 cases will be here
        1, from cb_end_state ,
        2, from cb_seq_state ,
        3, from cb_pending_state,
        4, from cb_block_state,
        these will test for this->m_PackSize
    */
    SDK_ASSERT(this->m_PackSize > 0);

    /*first we should test for the point data is the data we want*/
    pCBlock = this->m_pPacks[pCBuffer->m_CurIdx];
    SDK_ASSERT(pCBlock);
    if (pCBlock->m_Idx == pCBuffer->m_SourceIdx &&
            pCBlock->m_Type == I_FRAME_TYPE && pCBuffer->m_WriteLen < pCBuffer->m_TotalLen)
    {
        /*it is the pointer we have get ,so */
        ret = this->__SetIov(pCBuffer,pIoVec,iovlen,begin);
        if (ret < 0)
        {
            return ret;
        }
        return 1;
    }
    else if (pCBlock->m_Type == I_FRAME_TYPE && pCBlock->m_Idx == pCBuffer->m_SourceIdx)
    {
        /*it means we have already send the job ,so we should find the next one*/
        expectidx = pCBuffer->m_SourceIdx;
        curidx = pCBuffer->m_CurIdx;
        expectidx += 1;
        if (pCBuffer->m_SourceIdx == MAX_STREAM_IDX_32)
        {
            expectidx = 0;
        }

        /*now to test whether we should find the next*/
        if (curidx == this->m_EndIdx)
        {
            return 0;
        }

        curidx += 1;
        SDK_ASSERT(this->m_PackSize > 0);
        curidx %= this->m_PackSize;
        pCBlock = this->m_pPacks[curidx];
        SDK_ASSERT(pCBlock);
        if (pCBlock->m_Idx == expectidx)
        {
            /*yes ,we have got this one*/
            this->__ChangeClientBufferState(pCBuffer,cb_seq_state,__FILE__,__LINE__);
            pCBuffer->m_CurIdx = curidx;
            pCBuffer->m_SourceIdx = pCBlock->m_Idx;
            ret = this->__FormatMessageHeader(pCBuffer,pCBlock);
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

        /*not the one we expected ,so we passdown for next*/
        pCBuffer->m_CurIdx = curidx;
        pCBuffer->m_SourceIdx = pCBlock->m_Idx;
    }

    return this->__SearchUntilIframe(pCBuffer,pIoVec,iovlen,begin);

}

int SdkServerBuffer::__GetStreamDataSeqState(client_buffer_t * pCBuffer,struct iovec * pIoVec,int & iovlen,int & begin)
{
    unsigned int expectidx,curidx;
    stream_pack_t* pCBlock=NULL;
    int ret;
    SDK_ASSERT(pCBuffer->m_State == cb_seq_state);
    SDK_ASSERT(pCBuffer->m_pBlock == NULL);
    pCBlock = this->m_pPacks[pCBuffer->m_CurIdx];
    if (pCBuffer->m_SourceIdx == pCBlock->m_Idx &&
            pCBuffer->m_WriteLen < pCBuffer->m_TotalLen)
    {
        /*have not write all the buffer ,so get the buffer*/
        ret = this->__SetIov(pCBuffer,pIoVec,iovlen,begin);
        if (ret < 0)
        {
            return ret;
        }
        return 1;
    }
    else if (pCBuffer->m_SourceIdx == pCBlock->m_Idx)
    {
        /*we have alread write down ,so we should get next one*/
        if (pCBuffer->m_CurIdx == this->m_EndIdx)
        {
            /*nothing to get ,so we do nothing*/
            return 0;
        }
        expectidx = pCBuffer->m_SourceIdx;
        expectidx += 1;
        if (pCBuffer->m_SourceIdx == MAX_STREAM_IDX_32)
        {
            expectidx = 0;
        }

        curidx = pCBuffer->m_CurIdx;
        curidx += 1;
        curidx %= this->m_PackSize;
        pCBlock = this->m_pPacks[curidx];
        SDK_ASSERT(pCBlock);

        if (pCBlock->m_Idx == expectidx)
        {
            /*we are the sequence*/
            pCBuffer->m_CurIdx = curidx;
            pCBuffer->m_SourceIdx = pCBlock->m_Idx;
            ret = this->__FormatMessageHeader(pCBuffer,pCBlock);
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

        /*passdown ,when we can not find the next sequence ,so find i-frame*/
        pCBuffer->m_CurIdx = curidx;
        pCBuffer->m_SourceIdx = pCBlock->m_Idx;
    }

    return this->__SearchUntilIframe(pCBuffer,pIoVec,iovlen,begin);
}

int SdkServerBuffer::__GetStreamDataBlockState(client_buffer_t * pCBuffer,struct iovec * pIoVec,int & iovlen,int & begin)
{
    unsigned int expectidx,curidx;
    stream_pack_t* pCBlock=NULL;
    int ret;

    SDK_ASSERT(pCBuffer->m_State == cb_block_state);
    SDK_ASSERT(pCBuffer->m_pBlock);
    if (pCBuffer->m_pBlock->m_Idx != pCBuffer->m_SourceIdx)
    {
        ERROR_INFO("pblock idx 0x%x sourceidx 0x%x\n",pCBuffer->m_pBlock->m_Idx,
                   pCBuffer->m_SourceIdx);
    }
    SDK_ASSERT(pCBuffer->m_pBlock->m_Idx == pCBuffer->m_SourceIdx);

    if (pCBuffer->m_WriteLen < pCBuffer->m_TotalLen)
    {
        ret = this->__SetIov(pCBuffer,pIoVec,iovlen,begin);
        if (ret < 0)
        {
            return ret;
        }
        return 1;
    }

    /*now we should free the buffer*/
    this->__FreeStreamPack(pCBuffer->m_pBlock);

    SDK_ASSERT(this->m_PackSize == this->m_MaxPacks);
    /*first we pretend to find the next one if not success ,just change into cb_i_frame_state*/
    this->__ChangeClientBufferState(pCBuffer,cb_seq_state,__FILE__,__LINE__);
    expectidx = pCBuffer->m_SourceIdx;
    expectidx += 1;
    if (pCBuffer->m_SourceIdx == MAX_STREAM_IDX_32)
    {
        expectidx = 0;
    }

    curidx = pCBuffer->m_CurIdx;
    curidx += 1;
    curidx %= this->m_PackSize;
    pCBlock = this->m_pPacks[curidx];
    SDK_ASSERT(pCBlock);
    if (pCBlock->m_Idx == expectidx)
    {
        /*we got this one*/
        pCBuffer->m_CurIdx = curidx;
        pCBuffer->m_SourceIdx = pCBlock->m_Idx;
        ret = this->__FormatMessageHeader(pCBuffer,pCBlock);
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
    /*we find the next one*/

    return this->__SearchUntilIframe(pCBuffer,pIoVec,iovlen,begin);
}

int SdkServerBuffer::__GetStreamDataPendingState(client_buffer_t * pCBuffer,struct iovec * pIoVec,int & iovlen,int & begin)
{
    int ret;

    SDK_ASSERT(pCBuffer->m_State == cb_pending_state);
    if (pCBuffer->m_pBlock && pCBuffer->m_WriteLen < pCBuffer->m_TotalLen)
    {
        /*if we have data left*/
        ret = this->__SetIov(pCBuffer,pIoVec,iovlen,begin);
        if (ret < 0)
        {
            return ret;
        }
        return 1;
    }

    /*we free until here ,for it will change for the next search ,and will not be cb_pending_state*/
    this->__FreeStreamPack(pCBuffer->m_pBlock);

    /*now to change the state for cb_end_state ,start from */
    this->__InitClientBuffer(pCBuffer,pCBuffer->m_Sock,pCBuffer->m_SesId);

    /*now we should get from the cb_end_state ,to do here*/
    return this->__GetStreamDataEndState(pCBuffer,pIoVec,iovlen,begin);
}


int SdkServerBuffer::GetStreamData(int sock,struct iovec * pIoVec,int & iovlen,int & begin)
{
    int ret;
    unsigned int i;
    client_buffer_t *pCBuffer=NULL,*pFindCBuffer=NULL;

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
        return -ENODEV;
    }

    if(pFindCBuffer->m_Failed)
    {
        /*we have failed before ,so we should just return error*/
        return -EFAULT;
    }
    pCBuffer = pFindCBuffer;
    switch(pCBuffer->m_State)
    {
    case cb_end_state:
        ret = this->__GetStreamDataEndState(pCBuffer,pIoVec,iovlen,begin);
        break;
    case cb_i_frame_state:
        ret = this->__GetStreamDataIFrameState(pCBuffer,pIoVec,iovlen,begin);
        break;
    case cb_seq_state:
        ret = this->__GetStreamDataSeqState(pCBuffer,pIoVec,iovlen,begin);
        break;
    case cb_block_state:
        ret = this->__GetStreamDataBlockState(pCBuffer,pIoVec,iovlen,begin);
        break;
    case cb_pending_state:
        ret = this->__GetStreamDataPendingState(pCBuffer,pIoVec,iovlen,begin);
        break;
    default:
        SDK_ASSERT(0!=0);
        ret = -EINVAL;
        break;
    }

    return ret;

}

int SdkServerBuffer::__HandleClientInsert(client_buffer_t * pCBuffer,unsigned int insertidx)
{
    stream_pack_t* pCBlock=NULL;
    int ret;
    SDK_ASSERT(this->m_PackSize == this->m_MaxPacks);
    if (pCBuffer->m_CurIdx != insertidx ||
            pCBuffer->m_State == cb_pending_state ||
            pCBuffer->m_State == cb_block_state ||
            pCBuffer->m_State == cb_end_state)
    {
        return 0;
    }

    pCBlock = this->m_pPacks[insertidx];
    SDK_ASSERT(pCBlock);

    /*it may be handled not the same index ,so it is the time pointed to it*/
    if (pCBuffer->m_SourceIdx != pCBlock->m_Idx)
    {
        return 0;
    }

    /*this block does not the one ,we notify */
    if ((pCBuffer->m_State == cb_i_frame_state &&
            (pCBlock->m_Type != I_FRAME_TYPE)) || (pCBuffer->m_WriteLen == pCBuffer->m_TotalLen))
    {
        /*
                     maybe this will modify the pCBuffer->m_CurIdx and pCBuffer->m_SourceIdx
                     but it will give the next one waken up to enter the function __GetStreamDataIFrameState,
                     in that function will scan from m_StartIdx to m_EndIdx,so we handle the function there
        */
        return 0;
    }

	DEBUG_INFO("DISCARD [%d] idx blockstate discarded\n",pCBuffer->m_Sock);

    SDK_ASSERT(pCBuffer->m_pBlock == NULL);
    ret = this->__CopyBlock(pCBuffer,pCBlock,cb_block_state);
    if (ret < 0)
    {
        return ret;
    }
    return 1;
}

int SdkServerBuffer::__InsertPack(stream_pack_t * pPack)
{
    int ret;
    unsigned int insertidx,startidx;
    stream_pack_t* pCBlock=NULL;
    client_buffer_t *pCBuffer=NULL;
    unsigned int i;
    int affected = 0;

    /*if the packsize is less ,so just insert the last one*/
    if (this->m_PackSize < this->m_MaxPacks)
    {
        /*now we should insert the last one*/
        SDK_ASSERT(this->m_pPacks[this->m_PackSize] == NULL);
        this->m_pPacks[this->m_PackSize] = pPack;
        this->m_EndIdx = this->m_PackSize;
        this->m_PackSize += 1;
        return 0;
    }

    /*it is full,so the endidx and startidx are back-to-back*/
    SDK_ASSERT(((this->m_EndIdx + 1) == this->m_StartIdx) ||
           (this->m_StartIdx == 0 && this->m_EndIdx == (this->m_MaxPacks-1)));

    /*it is full ,so we should test whether there is some client buffer is handle on the insert idx*/
    startidx = this->m_StartIdx;
    insertidx = this->m_EndIdx;
    insertidx += 1;
    insertidx %= this->m_PackSize;

    SDK_ASSERT(startidx == insertidx);
    pCBlock = this->m_pPacks[insertidx];
    SDK_ASSERT(pCBlock);

    /*now we test the client is handling the job*/
    for (i=0; i<this->m_pClientBuffers.size(); i++)
    {
        pCBuffer = this->m_pClientBuffers[i];
        ret = this->__HandleClientInsert(pCBuffer,insertidx);
        if (ret < 0)
        {
            return ret;
        }
        else if (ret > 0)
        {
            affected += 1;
        }
    }

    if (pPack->m_Type == I_FRAME_TYPE)
    {
        //DEBUG_INFO("[%d]pull insert %d into %d\n",this->m_StreamId,pPack->m_Idx,insertidx);
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


int SdkServerBuffer::__PullData()
{
    int ret;
    stream_pack_t *pGet=NULL;
    if (this->m_pStream == NULL)
    {
        return 0;
    }

    pGet = (stream_pack_t*)calloc(sizeof(*pGet),1);
    if (pGet == NULL)
    {
        return -ENOMEM;
    }

    /*now we should get the data */
    ret = this->m_pStream->PullStreamData(pGet);
    if (ret <= 0)
    {
        this->__FreeStreamPack(pGet);
        return ret;
    }
    SDK_ASSERT(ret == 1);
    //DEBUG_INFO("pull data idx 0x%08x type %d pts 0x%llx\n",pGet->m_Idx,pGet->m_Type,pGet->m_Pts);

    ret = this->__InsertPack(pGet);
    if (ret < 0)
    {
        this->__FreeStreamPack(pGet);
        return ret;
    }

    pGet = NULL;
    /*we have insert one*/
    return 1;
}


int SdkServerBuffer::__GetNotify( std::vector<int>& notifyfds)
{
    std::vector<int> dummy;
    unsigned int i;
    client_buffer_t* pCBuffer;
    unsigned int lastidx;

    lastidx = this->m_EndIdx;
    if (lastidx > 0)
    {
        lastidx -= 1;
    }
    else
    {
        //DEBUG_INFO("packsize %d\n",this->m_PackSize);
        SDK_ASSERT(this->m_PackSize >0);
        lastidx = this->m_PackSize - 1;
    }
    for (i=0; i<this->m_pClientBuffers.size(); i++)
    {
        pCBuffer = this->m_pClientBuffers[i];
        if (pCBuffer->m_State == cb_seq_state ||
                pCBuffer->m_State == cb_i_frame_state ||
                pCBuffer->m_State == cb_end_state)
        {
            if (pCBuffer->m_CurIdx == lastidx &&
                    pCBuffer->m_WriteLen == pCBuffer->m_TotalLen)
            {
                dummy.push_back(pCBuffer->m_Sock);
            }
        }
    }

    notifyfds = dummy;
    return 0;
}

int SdkServerBuffer::PullStreamData( std::vector<int>& notifysock)
{
    int ret,res;
    std::vector<int> dummysock;

    if (this->m_Started == 0)
    {
        /*this is not started ,so we do not need pull any data from the stream*/
        notifysock = dummysock;
        return 0;
    }

    ret = this->__PullData();
    if (ret < 0)
    {
        return ret;
    }

    if (ret > 0)
    {
        res = this->__GetNotify(notifysock);
        if (res < 0)
        {
            return res;
        }
    }
    else
    {
        notifysock = dummysock;
    }

    return ret;
}

#define POINTER_IN_RANGE(ptr,startptr,len) \
do\
{\
	SDK_ASSERT(((ptr_t)(ptr)) >= ((ptr_t)(startptr)));\
	SDK_ASSERT(((ptr_t)(ptr)) <= (((ptr_t)(startptr))+(len)));\
}while(0)

int SdkServerBuffer::__ForwardAssert(client_buffer_t * pCBuffer,struct iovec * pIoVec,int iovlen,int forwardlen)
{
    stream_pack_t* pCBlock;
    int iototallen=0;
    unsigned int curlen ;
    SDK_ASSERT(iovlen <= 3 && iovlen >= 1);
    if (iovlen == 3)
    {
        iototallen =0;
        POINTER_IN_RANGE(pIoVec[0].iov_base,&(pCBuffer->m_GsspHeader),pCBuffer->m_GsspLen);
        curlen = (pCBuffer->m_GsspLen - ((ptr_t)pIoVec[0].iov_base - (ptr_t)&(pCBuffer->m_GsspHeader)));
        SDK_ASSERT(pIoVec[0].iov_len== curlen);
        iototallen += curlen;
        POINTER_IN_RANGE(pIoVec[1].iov_base,&(pCBuffer->m_Header.m_IFrame),pCBuffer->m_FrameLen);
        curlen = pCBuffer->m_FrameLen;
        SDK_ASSERT(pIoVec[1].iov_len == curlen);
        iototallen += curlen;
        if (pCBuffer->m_pBlock)
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
    else if (iovlen == 2)
    {
        iototallen = 0;
        POINTER_IN_RANGE(pIoVec[0].iov_base,&(pCBuffer->m_Header.m_IFrame),pCBuffer->m_FrameLen);
        curlen = (pCBuffer->m_FrameLen - (((ptr_t)(pIoVec[0].iov_base)) - ((ptr_t)(&(pCBuffer->m_Header.m_IFrame)))));
        SDK_ASSERT(pIoVec[0].iov_len == curlen);
        iototallen += curlen;
        if (pCBuffer->m_pBlock)
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
    else if (iovlen == 1)
    {
        iototallen = 0;
        if (pCBuffer->m_pBlock)
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

int SdkServerBuffer::__ForwardStreamDataInner(client_buffer_t * pCBuffer,struct iovec * pIoVec,int iovlen,int forwardlen)
{
    int ret;

    /*now to test for the write len*/
    SDK_ASSERT (pCBuffer->m_WriteLen < pCBuffer->m_TotalLen);
    ret = this->__ForwardAssert(pCBuffer,pIoVec,iovlen,forwardlen);
    if (ret < 0)
    {
        return ret;
    }
    /*now to test whether it is ok in the format*/
    pCBuffer->m_WriteLen += forwardlen;
    SDK_ASSERT (pCBuffer->m_WriteLen <= pCBuffer->m_TotalLen);
    if (pCBuffer->m_WriteLen > (pCBuffer->m_FrameLen + pCBuffer->m_GsspLen))
    {
        pCBuffer->m_Offset = (pCBuffer->m_WriteLen - (pCBuffer->m_FrameLen + pCBuffer->m_GsspLen));
    }
    //DEBUG_INFO("writelen %d totallen %d\n",pCBuffer->m_WriteLen,pCBuffer->m_TotalLen);
    return pCBuffer->m_WriteLen == pCBuffer->m_TotalLen ? 1 : 0;
}


int SdkServerBuffer::ForwardStreamData(int sock,struct iovec * pIoVec,int iovlen,int forwardlen)
{
    int ret;
    unsigned int i;
    client_buffer_t* pCBuffer=NULL,*pFindCBuffer=NULL;

    for (i=0; i<this->m_pClientBuffers.size(); i++)
    {
        pCBuffer = this->m_pClientBuffers[i];
        if (sock == pCBuffer->m_Sock)
        {
            pFindCBuffer = pCBuffer;
            break;
        }
    }

    if (pFindCBuffer == NULL)
    {
        return -ENODEV;
    }

    pCBuffer = pFindCBuffer;
    switch(pCBuffer->m_State)
    {
    case cb_end_state:
        /*this can not be happend ,so we make bug*/
        SDK_ASSERT(0!=0);
        ret = -EINVAL;
        break;
    case cb_i_frame_state:
    case cb_seq_state:
    case cb_block_state:
    case cb_pending_state:
        ret = this->__ForwardStreamDataInner(pCBuffer,pIoVec,iovlen,forwardlen);
        break;
    default:
        /*this can not be happend ,so we make bug*/
        SDK_ASSERT(0!=0);
        ret = -EFAULT;
        break;
    }

    return ret;
}


int SdkServerBuffer::__FormatMessageHeader(client_buffer_t * pCBuffer,stream_pack_t * pCBlock)
{
    uint32_t totallen;
    gssp_header_t* pGsspHeader ;
    video_i_frame_header_t* pIFrame=NULL;
    video_p_frame_header_t* pPFrame=NULL;
    uint32_t n32;
    uint16_t n16;

    totallen = 0;
    /*now first to calculate the total length of the buffer*/
    pGsspHeader = &(pCBuffer->m_GsspHeader);

    totallen += GSSP_HEADER_BASE_LEN;
    if (pCBlock->m_Type == I_FRAME_TYPE)
    {
		struct timespec tmspec;
		clock_gettime(CLOCK_MONOTONIC,&tmspec);
		DEBUG_INFO("Idx %d time %ld:%ld\n",pCBlock->m_Idx,tmspec.tv_sec,tmspec.tv_nsec);
        totallen += sizeof(pCBuffer->m_Header.m_IFrame);
		
    }
    else
    {
        totallen += sizeof(pCBuffer->m_Header.m_PFrame);
    }
    totallen += pCBlock->m_DataLen;
    memset(&(pCBuffer->m_GsspHeader),0,sizeof(pCBuffer->m_GsspHeader));
    pCBuffer->m_GsspLen = GSSP_HEADER_BASE_LEN;
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
    n32 = HOST_TO_PROTO32(totallen - pCBuffer->m_GsspLen);
    pGsspHeader->m_BodyLength = n32;
    n32 = HOST_TO_PROTO32(0);
    pGsspHeader->m_CheckSum = n32;

    memset(&(pCBuffer->m_Header),0,sizeof(pCBuffer->m_Header));

    if (pCBlock->m_Type == I_FRAME_TYPE)
    {
        pIFrame = &(pCBuffer->m_Header.m_IFrame);
		pIFrame->m_VideoCode = HOST_TO_PROTO32(SDK_STREAM_VD_SEND);
        pIFrame->m_FrameType = 'I';
        pIFrame->m_StreamId = this->m_StreamId;
        pIFrame->m_Freq = this->m_StreamInfo.s_FPS;
        pIFrame->m_EncType = PROTO_STREAM_NONE;
        if (this->m_StreamInfo.s_Compression == SYS_COMP_H264)
        {
            pIFrame->m_EncType = PROTO_STREAM_H264;
        }
        if (this->m_StreamInfo.s_Compression == SYS_COMP_MJPEG)
        {
            pIFrame->m_EncType = PROTO_STREAM_MJPEG;
        }

        pIFrame->m_FOP = this->m_StreamInfo.s_Gop;
        pIFrame->m_Reserved[0] = 0;
        pIFrame->m_Reserved[1] = 0;
        pIFrame->m_Reserved[2] = 0;
        pIFrame->m_Pts = HOST_TO_PROTO64(pCBlock->m_Pts);
        pIFrame->m_FrameIdx = HOST_TO_PROTO32(pCBlock->m_Idx);
        pIFrame->m_Width = HOST_TO_PROTO16(this->m_StreamInfo.s_PicWidth);
        pIFrame->m_Height = HOST_TO_PROTO16(this->m_StreamInfo.s_PicHeight);
        pCBuffer->m_FrameLen = sizeof(*pIFrame);

    }
    else
    {
        pPFrame = &(pCBuffer->m_Header.m_PFrame);
		pPFrame->m_VideoCode = HOST_TO_PROTO32(SDK_STREAM_VD_SEND);
        pPFrame->m_FrameType = 'P';
        pPFrame->m_StreamId = this->m_StreamId;
        pPFrame->m_Reserved[0] = 0;
        pPFrame->m_Reserved[1] = 0;
        pPFrame->m_FrameIdx = HOST_TO_PROTO32(pCBlock->m_Idx);
        pPFrame->m_Pts = HOST_TO_PROTO64(pCBlock->m_Pts);
        pCBuffer->m_FrameLen = sizeof(*pPFrame);
    }
	//DEBUG_INFO("framelen %d\n",pCBuffer->m_FrameLen);


    pCBuffer->m_WriteLen = 0;
    pCBuffer->m_TotalLen = totallen;
    pCBuffer->m_Offset = 0;

    return 0;
}


int SdkServerBuffer::__SetIov(client_buffer_t * pCBuffer,struct iovec * pIoVec,int & iovlen,int & begin)
{
    int curidx=0;
    int curoff;
    ptr_t pdata;
    stream_pack_t* pCBlock=NULL;
    if (iovlen < 3)
    {
        return -ENOSPC;
    }

    if (pCBuffer->m_WriteLen  == pCBuffer->m_TotalLen)
    {
        return -ENODATA;
    }

    if (pCBuffer->m_WriteLen < pCBuffer->m_GsspLen)
    {
        pIoVec[curidx].iov_base = (void*)(((ptr_t)&(pCBuffer->m_GsspHeader)) + pCBuffer->m_WriteLen);
        pIoVec[curidx].iov_len = pCBuffer->m_GsspLen - pCBuffer->m_WriteLen;
        curidx ++;
    }

    if (pCBuffer->m_WriteLen < (pCBuffer->m_GsspLen + pCBuffer->m_FrameLen))
    {
        curoff = 0;
        if (pCBuffer->m_WriteLen > pCBuffer->m_GsspLen)
        {
            curoff = pCBuffer->m_WriteLen - pCBuffer->m_GsspLen;
        }
        pIoVec[curidx].iov_base = (void*) (((ptr_t)&(pCBuffer->m_Header.m_IFrame))+ curoff);
        pIoVec[curidx].iov_len = pCBuffer->m_FrameLen - curoff;
        curidx ++;
    }
    curoff = 0;
    if (pCBuffer->m_WriteLen > (pCBuffer->m_GsspLen + pCBuffer->m_FrameLen))
    {
        curoff = pCBuffer->m_WriteLen - (pCBuffer->m_GsspLen + pCBuffer->m_FrameLen);
    }


    if (pCBuffer->m_pBlock)
    {
        pCBlock = pCBuffer->m_pBlock;

    }
    else
    {
        pCBlock = this->m_pPacks[pCBuffer->m_CurIdx];
        SDK_ASSERT(pCBlock);
    }

    pdata = (ptr_t) pCBlock->m_pData;

    pIoVec[curidx].iov_base = (void*) (pdata + curoff);
    pIoVec[curidx].iov_len = pCBlock->m_DataLen - curoff;
    curidx ++ ;
    iovlen = curidx;

    begin = 0;
    if (pCBuffer->m_WriteLen == 0)
    {
        begin = 1;
    }

    return 1;
}


void SdkServerBuffer::__DebugCBuffer(client_buffer_t * pCBuffer)
{
    stream_pack_t * pPack;
    unsigned int i;
    DEBUG_INFO("stream[%d]sock[%d] state %d curidx %d sourceidx %d\n",this->m_StreamId,
               pCBuffer->m_Sock,pCBuffer->m_State,pCBuffer->m_CurIdx,
               pCBuffer->m_SourceIdx);
    if (pCBuffer->m_pBlock)
    {
        DEBUG_INFO("block idx %d type (%d)\n",pCBuffer->m_pBlock->m_Idx,
                   pCBuffer->m_pBlock->m_Type);
    }
    DEBUG_INFO("startidx %d endidx %d packsize %d\n",this->m_StartIdx,this->m_EndIdx,this->m_PackSize);
    if (this->m_pPacks && this->m_pPacks[pCBuffer->m_CurIdx])
    {
        pPack = this->m_pPacks[pCBuffer->m_CurIdx];
        DEBUG_INFO("at[%d] %d type(%d)\n",
                   pCBuffer->m_CurIdx,
                   pPack->m_Idx,
                   pPack->m_Type);
    }

    for (i=0; i<this->m_MaxPacks; i++)
    {
        if (this->m_pPacks[i])
        {
            pPack = this->m_pPacks[i];
            if (pPack->m_Type == I_FRAME_TYPE)
            {
                DEBUG_INFO("[%d][%d] idx %d type iframe\n",
                           this->m_StreamId,i,pPack->m_Idx);
            }
        }
    }
    return ;
}

void SdkServerBuffer::DebugClientBuffer(int sock)
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
