
#include <tcp_source_buffer.h>

#if 1
#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%d[%ld]\t",__FILE__,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)
#else
#define DEBUG_INFO(...)
#endif
#define   ASSERT_MEMORY_IN(pData,pStartData,size) \
do\
{\
    unsigned long __dataoff,__datastart,__dataend;\
    __dataoff = (unsigned long)(pData);\
    __datastart = (unsigned long)(pStartData);\
    __dataend = (unsigned long) (((unsigned char*)pStartData) + (size));\
    assert(__dataoff >= __datastart);\
    assert(__dataoff <= __dataend);\
} while(0)

int __DataNumberGreat(unsigned int a,unsigned int b)
{
    if (a > b)
    {
        return 1;
    }

    /*this is because overflow the max_frame_idx*/
    if (a < 1000 && b > (MAX_FRAME_IDX - 1000))
    {
        return 1;
    }
    return 0;
}

TcpSourceBuffer::TcpSourceBuffer(int maxpack,int maxclients,int streamid) :
    m_StreamId(streamid),
    m_MaxPacks(maxpack),
    m_MaxClients(maxclients)
{
    m_State = stream_end_state;
    m_pStream = NULL;
    m_pStreamData = NULL;
    m_LastIFrame = 0;
    m_StartIdx = 0;
    m_EndIdx = 0;
    m_BufferLength = 0;
    m_StartNumber = 0;
    m_EndNumber = 0;
    m_ClientsNumber = 0;
    m_pClientDataInfo = NULL;
}

void __FreeStreamDataInfo(stream_data_info_t* pSData)
{
    if (pSData)
    {
        if (pSData->m_pData)
        {
            free(pSData->m_pData);
        }
        pSData->m_pData = NULL;
        free(pSData);
    }
    return ;
}

stream_data_info_t* __AllocateStreamDataInfo(void* pData,int datalen,int type,int idx)
{
    stream_data_info_t* pSData=NULL;
    pSData = (stream_data_info_t*) malloc(sizeof(*pSData));
    if (pSData == NULL)
    {
        return NULL;
    }

    pSData->m_DataNumber = idx;
    pSData->m_DataType = type;
    pSData->m_pData = pData;
    pSData->m_DataLen = datalen;
    return pSData;
}

stream_data_info_t* __CopyStreamDataInfo(stream_data_info_t* pSrcData)
{
    stream_data_info_t* pDstData=NULL;
    void* pData=NULL;

    pData = malloc(pSrcData->m_DataLen);
    if (pData == NULL)
    {
        goto fail;
    }

    memcpy(pData,pSrcData->m_pData,pSrcData->m_DataLen);
    pDstData = __AllocateStreamDataInfo(pData,pSrcData->m_DataLen,pSrcData->m_DataType,pSrcData->m_DataNumber);
    if (pDstData==NULL)
    {
        goto fail;
    }
    return pDstData;

fail:
    if (pData)
    {
        free(pData);
    }
    __FreeStreamDataInfo(pDstData);
    return NULL;
}

void __FreeClientDataInfo(client_data_info_t* pCData)
{
    if (pCData)
    {
        if (pCData->m_pCopyData)
        {
            __FreeStreamDataInfo(pCData->m_pCopyData);
        }
        pCData->m_pCopyData = NULL;
        free(pCData);
    }
    return ;
}

void TcpSourceBuffer::__CloseStream()
{
    int i;
    this->m_State = stream_end_state;
    if (this->m_pStreamData)
    {
        for (i=0; i<this->m_MaxPacks; i++)
        {
            __FreeStreamDataInfo(this->m_pStreamData[i]);
            this->m_pStreamData[i] = NULL;
        }

        free(this->m_pStreamData);
    }
    this->m_pStreamData = NULL;

    if (this->m_pStream)
    {
        delete this->m_pStream;
    }
    this->m_pStream = NULL;

    m_LastIFrame = 0;
    this->m_StartIdx = 0;
    this->m_EndIdx  = 0;
    this->m_BufferLength = 0;
    this->m_StartNumber = 0;
    this->m_EndNumber = 0;
    this->m_LastIFrame = 0;

    if (this->m_pClientDataInfo)
    {
        for(i=0; i<this->m_ClientsNumber; i++)
        {
            __FreeClientDataInfo(this->m_pClientDataInfo[i]);
            this->m_pClientDataInfo[i] = NULL;
        }

        free(this->m_pClientDataInfo);
    }
    this->m_pClientDataInfo = NULL;
    this->m_ClientsNumber = 0;
    return;

}



int TcpSourceBuffer::__InitStream()
{
    int size;
    int ret;

    if (this->m_MaxPacks == 0 )
    {
        return -EINVAL;
    }

    if (this->m_StreamId < 0 || this->m_StreamId >= MAX_STREAM_IDS)
    {
        return -EINVAL;
    }

    this->__CloseStream();

    /*now to allocate for stream */
    this->m_pStream = new TcpSourceStream(this->m_StreamId);
    assert(this->m_pStream);
    this->m_LastIFrame = 0;
    this->m_StartIdx = 0;
    this->m_EndIdx = 0;
    this->m_BufferLength = 0;
    this->m_StartNumber = 0;
    this->m_EndNumber = 0;


    assert(this->m_ClientsNumber == 0);
    assert(this->m_pClientDataInfo == NULL);
    assert(this->m_State == stream_end_state);

    size = this->m_MaxPacks * sizeof(this->m_pStreamData[0]);
    this->m_pStreamData = (stream_data_info_t**) malloc(size);
    if (this->m_pStreamData == NULL)
    {
        this->__CloseStream();
        return -ENOMEM;
    }
    /*to set for the NULL*/
    memset(this->m_pStreamData,0,size);
    ret = this->m_pStream->Start();
    if (ret < 0)
    {
        this->__CloseStream();
        return ret;
    }

    return 0;

}
int TcpSourceBuffer::InitStream()
{
    return this->__InitStream();
}


TcpSourceBuffer::~TcpSourceBuffer()
{
    this->__CloseStream();
    this->m_MaxPacks = 0;
    this->m_MaxClients = 0;
    this->m_StreamId = -1;
}

int TcpSourceBuffer::__IsStreamDataUsed(int pos)
{
    int i;
    client_data_info_t* pCData=NULL;
    /*now to scan the client data info */
    for (i=0; i<this->m_ClientsNumber; i++)
    {
        if (this->m_pClientDataInfo[i])
        {
            pCData = this->m_pClientDataInfo[i];
            if (pCData->m_pCopyData == NULL )
            {
                /*if not copied data ,and if position is the same ,it is used*/
                if (pCData->m_DataIdx == pos)
                {
                    return 1;
                }
            }
        }
    }

    return 0;
}

int TcpSourceBuffer::__CopyStreamDataUsed(int pos)
{
    int ret;
    stream_data_info_t* pSData=NULL;
    stream_data_info_t* pSrcData=this->m_pStreamData[pos];
    int i;
    int copied = 0;

    assert(pSrcData);

    for (i=0; i<this->m_ClientsNumber; i++)
    {
        client_data_info_t * pCData=this->m_pClientDataInfo[i];
        int shouldcopy = 1;
        if (pCData &&pCData->m_pCopyData == NULL && pCData->m_DataIdx == pos)
        {
            /*we test for it when it really */

            if (pCData->m_ClientState == client_i_frame_state && pSrcData->m_DataType != I_FRAME_TYPE)
            {
                /*this is not the frame this client handled,so we do not copied this one*/
                shouldcopy = 0;
            }

            if (shouldcopy)
            {
                pSData = __CopyStreamDataInfo(pSrcData);
                if (pSData == NULL)
                {
                    ret = -ENOMEM;
                    goto fail;
                }
                /*it is the same source index*/
                assert(pCData->m_SourceIdx == pSrcData->m_DataNumber);
                pCData->m_pCopyData = pSData;
                pCData->m_ClientState = client_block_state;
                copied += 1;
                DEBUG_INFO("change[%d] client_block_state\n",pCData->m_ClientFd);
                pSData = NULL;
            }
            else
            {
                int nextpos ;
                assert(pCData->m_ClientState == client_i_frame_state);
                assert(this->m_BufferLength == this->m_MaxPacks);
                /*we move to the next one*/
                nextpos = pos + 1;
                if (nextpos == this->m_BufferLength)
                {
                    nextpos = 0;
                }
                assert(this->m_pStreamData[nextpos]);
                pCData->m_DataIdx = nextpos;
                pCData->m_DataOffset = 0;
                pCData->m_SourceIdx = this->m_pStreamData[nextpos]->m_DataNumber;
            }
        }
    }

    return copied ;



fail:
    __FreeStreamDataInfo(pSData);
    return ret;
}


unsigned int TcpSourceBuffer::__GetDataNumber(int pos)
{
    stream_data_info_t *pSData=NULL;

    assert(pos >= 0 && pos <= this->m_MaxPacks);
    assert (this->m_pStreamData);
    assert(this->m_pStreamData[pos]);

    pSData = this->m_pStreamData[pos];
    return pSData->m_DataNumber;
}

/***********************************
*       this function is to insert the buffer we
*       find the last one ,if it is some fd just read
*       we should copy for it
*       if not ,we just ,free the old one
***********************************/
int TcpSourceBuffer::__InsertFrame(void * pData,int datalen,int type,int idx)
{
    stream_data_info_t* pSData=NULL;
    int ret = -1;
    int pos=0,nextpos;

    pSData = __AllocateStreamDataInfo(pData,datalen,type,idx);
    if (pSData == NULL)
    {
        return -ENOMEM;
    }


    assert(this->m_MaxPacks > 0);
    if (this->m_BufferLength < this->m_MaxPacks)
    {
        /*we have position to insert the buffer ,so insert it into the position*/
        pos = this->m_BufferLength;
        this->m_pStreamData[pos] = pSData;
        if (this->m_BufferLength==0)
        {
            /*it is the first one ,so set the start number*/
            this->m_StartNumber = idx;
        }
        /*this is the correct one*/
        this->m_EndIdx = this->m_BufferLength;
        //DEBUG_INFO("set pos %d\n",this->m_EndIdx);
        this->m_BufferLength +=1;
        this->m_EndNumber = idx;
        return 0;
    }

    /*the position is full ,so we should insert the start idx*/
    pos = this->m_StartIdx;
    /*to see if the pos is used */
    ret = this->__IsStreamDataUsed(pos);
    if (ret > 0)
    {
        ret = this->__CopyStreamDataUsed(pos);
        if (ret < 0)
        {
            goto fail;
        }
    }
    /*now to free the old position*/
    __FreeStreamDataInfo(this->m_pStreamData[pos]);
    /*now to put the data into the position ,and modify the start and end idx and start end number*/
    this->m_pStreamData[pos] = pSData;
    this->m_EndIdx = pos;
    //DEBUG_INFO("set pos %d\n",this->m_EndIdx);
    nextpos = pos +1;
    if (nextpos >= this->m_MaxPacks)
    {
        nextpos %= this->m_MaxPacks;
    }
    this->m_StartIdx = nextpos;
    this->m_StartNumber = this->__GetDataNumber(nextpos);
    this->m_EndNumber = idx;
    /*ok ,insert ok*/
    return 0;
fail:
    /*we pretend not to free the pData, because it will free outer the function*/
    if (pSData)
    {
        pSData->m_pData = NULL;
        __FreeStreamDataInfo(pSData);
    }
    pSData = NULL;
    return ret;
}

/***********************************
*
*   this function is to pull data in the i-frame-state
*          this will test get buffer is i-frame ,
*          if it is i-frame ,it will replace the old one
*          if not , will discard the buffer
***********************************/
int TcpSourceBuffer::__PullDataIFrame(void)
{
    void* pData=NULL;
    int datalen = 0;
    int type;
    int idx;
    int ret;

    idx = this->m_EndNumber + 1;
    if (this->m_EndNumber == MAX_FRAME_IDX)
    {
        /*over flow so do it*/
        idx = 0;
    }

    ret = this->m_pStream->GetNextStream(&idx,&pData,&datalen,&type);
    if (ret < 0)
    {
        return ret;
    }
    else if (ret == 0)
    {
        /*nothing to get*/
        return 0;
    }

    assert(pData);
    assert(datalen);
    /*we modify the index number as quickly as we can this will u can for end number search*/
    this->m_EndNumber  = idx;
    if (type != I_FRAME_TYPE)
    {
        /*not the type ,so we search for next one*/
        free(pData);
        datalen = 0;
        this->m_State = stream_i_frame_state;
        DEBUG_INFO("[%d] get idx %d\n",this->m_StreamId,idx);
        return 0;
    }

    /*ok ,this is the i-frame type so we insert it ,and make it ok*/
    ret = this->__InsertFrame(pData,datalen,type,idx);
    if (ret < 0)
    {
        free(pData);
        datalen = 0;
        this->m_State = stream_i_frame_state;
        return ret;
    }

    DEBUG_INFO("change[%d] stream_seq_state\n",this->m_StreamId);

    /*now to change the type sequence for next*/
    this->m_State = stream_seq_state;
    return 1;
}

/***********************************
*
*   this function is to pull data in the seq_state
*          this will test if the buffer is sequence
*          if it is ,it will insert the frame buffer
*          if not , it will test if it is i-frame type ,
*              if i-frame ,insert and set seq_state
*              not ,set i-frame-state and discard buffer
***********************************/
int TcpSourceBuffer::__PullDataSeq(void)
{
    void* pData=NULL;
    int datalen = 0;
    int type;
    int idx,expectidx;
    int ret;

    idx = this->m_EndNumber + 1;
    if (this->m_EndNumber == MAX_FRAME_IDX)
    {
        /*over flow so do it*/
        idx = 0;
    }

    expectidx = idx;

    ret = this->m_pStream->GetNextStream(&idx,&pData,&datalen,&type);
    if (ret < 0)
    {
        return ret;
    }
    else if (ret == 0)
    {
        /*nothing to get*/
        return 0;
    }

    assert(pData);
    assert(datalen);
    /*we modify the index number as quickly as we can*/
    this->m_EndNumber  = idx;
    if (expectidx != idx)
    {
        /*now test whether it is i-frame */
        if (type != I_FRAME_TYPE)
        {
            free(pData);
            datalen = 0;
            pData = NULL;
            this->m_State = stream_i_frame_state;
            DEBUG_INFO("change[%d] stream_i_frame_state expectidx 0x%x idx 0x%x\n",this->m_StreamId,expectidx,idx);
            return 0;
        }

        /*it is sequence data ,so insert into the buffer so passing down*/

    }

    /*ok ,this is the i-frame type so we insert it ,and make it ok*/
    ret = this->__InsertFrame(pData,datalen,type,idx);
    if (ret < 0)
    {
        free(pData);
        datalen = 0;
        this->m_State = stream_i_frame_state;
        DEBUG_INFO("change stream_i_frame_state\n");
        return ret;
    }

    /*now to change the type sequence for next*/
    this->m_State = stream_seq_state;
    return 1;
}


/***********************************
*       this function to give the start number
*               if get frame ,just set the start number and end number
*                   if i-frame, just insert and make seq_state
*                   if not i-frame, just discard frame ,and  set i-frame-state
***********************************/
int TcpSourceBuffer::__PullDataStart(void)
{
    void* pData=NULL;
    int datalen = 0;
    int type;
    int idx;
    int ret;

    idx = START_FRAME_IDX;
    ret = this->m_pStream->GetNextStream(&idx,&pData,&datalen,&type);
    if (ret < 0)
    {
        return ret;
    }
    else if (ret == 0)
    {
        /*nothing to get*/
        return 0;
    }

    assert(pData);
    assert(datalen);
    /*we modify the index number as quickly as we can*/
    this->m_StartNumber = idx;
    this->m_EndNumber  = idx;
    /*now test whether it is i-frame */
    if (type != I_FRAME_TYPE)
    {
        free(pData);
        datalen = 0;
        pData = NULL;
        this->m_State = stream_end_state;
        DEBUG_INFO("change[%d] stream_end_state\n",this->m_StreamId);
        return 0;
    }



    /*ok ,this is the i-frame type so we insert it ,and make it ok*/
    ret = this->__InsertFrame(pData,datalen,type,idx);
    if (ret < 0)
    {
        free(pData);
        datalen = 0;
        pData = NULL;
        this->m_State = stream_end_state;
        DEBUG_INFO("change stream_end_state\n");
        return ret;
    }

    /*now to change the type sequence for next*/
    this->m_State = stream_seq_state;
    DEBUG_INFO("change[%d] stream_seq_state\n",this->m_StreamId);
    return 1;

}

int TcpSourceBuffer::__WakeupAllSocks(int * pNotifyFds,int maxfds)
{
    int i;
    int noted=0;

    if (maxfds < this->m_ClientsNumber)
    {
        return -ENOSPC;
    }
    for (i=0; i<this->m_ClientsNumber; i++)
    {
        client_data_info_t *pCData=this->m_pClientDataInfo[i];
        pNotifyFds[i]  = pCData->m_ClientFd;
        noted +=1;
    }
    return noted;
}


/***********************************
*            the state clarified by oldstate => current state
*
*            stream_end_state => stream_i_frame_state
*                     this can not be happend
*                     because this will no buffer inserted,so assert not
*
*            stream_end_state => stream_seq_state
*                     wakeup all the socket
*
*            stream_i_frame_state => stream_seq_state
*                     wakeup all the socket
*
*            stream_seq_state => stream_i_frame_state
*                     this can not be happend ,because this will not insert buffer
*
*            stream_seq_state => stream_seq_state
*                      the socket in the form will be waken up
*                      socket idx is at the last one,and at the end of datalen
***********************************/
int TcpSourceBuffer::__GetNotifiedFds(enum_tcp_source_buffer_state_t oldstate,int * pNotifyFds,int maxfds)
{
    int noted = 0;

    if (oldstate == stream_end_state && this->m_State == stream_seq_state)
    {
        return this->__WakeupAllSocks(pNotifyFds,maxfds);
    }
    else if (oldstate == stream_i_frame_state && this->m_State == stream_seq_state)
    {
        return this->__WakeupAllSocks(pNotifyFds,maxfds);
    }
    else if (oldstate == stream_seq_state && this->m_State == stream_seq_state)
    {
        return this->__WakeupAllSocks(pNotifyFds,maxfds);
    }
    else
    {
        /*this is really a bug*/
        assert(0!=0);
    }

    return noted ;

}

int TcpSourceBuffer::PullData(int *pNotifyFds ,int maxfds)
{
    int i;
    int ret=0;
    enum_tcp_source_buffer_state_t oldstate;

    if (this->m_pStream == NULL)
    {
        return -EINVAL;
    }

    /*now to set the fds to -1*/
    for (i=0; i<maxfds ; i++)
    {
        pNotifyFds[i] = -1;
    }

    oldstate = this->m_State;

    /*now to test if we have data to get */
    if (this->m_State == stream_seq_state)
    {
        //DEBUG_INFO("\n");
        ret = this->__PullDataSeq();
        if (ret < 0)
        {
            this->__CloseStream();
        }

    }
    else if (this->m_State == stream_i_frame_state)
    {
        ret = this->__PullDataIFrame();
        if (ret < 0)
        {
            this->__CloseStream();
        }
        DEBUG_INFO("[%d] ret %d\n",this->m_StreamId,ret);
    }
    else if (this->m_State == stream_end_state)
    {
        ret = this->__PullDataStart();
        if (ret < 0)
        {
            this->__CloseStream();
        }
        DEBUG_INFO("[%d] ret %d\n",this->m_StreamId,ret);
    }
    else
    {
        assert(0!=0);
    }


    if (ret > 0)
    {
        /*this means that we need some fd store or changed*/
        ret = this->__GetNotifiedFds(oldstate,pNotifyFds,maxfds);
        if (ret < 0)
        {
            this->__CloseStream();
        }
        else if (ret == 0)
        {
            /*for the notify 0 as the number modified*/
            ret = 1;
        }
    }

    return ret;

}


void TcpSourceBuffer::__ShrinkClients(int fd)
{
    int i;
    int removed = -1;
    client_data_info_t*  pCData=NULL,*pRemoveCData=NULL;

    for (i=0; i<this->m_ClientsNumber; i++)
    {
        pCData = this->m_pClientDataInfo[i];
        if (pCData && pCData->m_ClientFd == fd)
        {
            removed = i;
            pRemoveCData = pCData;
            break;
        }
    }

    if (removed >= 0)
    {
        __FreeClientDataInfo(pRemoveCData);
        this->m_pClientDataInfo[removed] = NULL;
        i = removed;
        i ++;
        while(i<this->m_ClientsNumber)
        {
            /*move forward*/
            this->m_pClientDataInfo[i-1] = this->m_pClientDataInfo[i];
            i ++;
        }

        this->m_ClientsNumber --;
    }

    return;
}

void TcpSourceBuffer::ClearData(int fd)
{
    return this->__ShrinkClients(fd);
}


/***********************************
*           to get the data when allocated client first
*               we test the buffer ,whether it has some
*               data ,and see ,if it is data
*               if data ,get the data ,set client_i_frame_state
*               if no data ,set the client_i_frame_state ,and set the sourceNumber
*                   to be the endidx one
***********************************/
int TcpSourceBuffer::__GetDataStartState(client_data_info_t * pCData,void ** ppData,int & datalen)
{
    int i,idx;
    stream_data_info_t* pSData=NULL,*pFindSData=NULL;
    int findidx=-1;

    assert(pCData->m_ClientState == client_start_state);

    if (this->m_BufferLength==0)
    {
        /*we do not modify the state ,because we can not set the number*/
        return 0;
    }

    for (i=0; i<this->m_BufferLength; i++)
    {
        idx = this->m_StartIdx + i;
        if (idx >= this->m_BufferLength)
        {
            idx %= this->m_BufferLength;
        }
        pSData = this->m_pStreamData[idx];
        if (pSData->m_DataType == I_FRAME_TYPE)
        {
            /*we should find the last one ,so we try next*/
            pFindSData = pSData;
            findidx = idx;
        }
    }

    if (pFindSData == NULL)
    {
        /*we do not find any data so we set the last into the buffer*/
        if (pSData)
        {
            pCData->m_SourceIdx = pSData->m_DataNumber;
            pCData->m_ClientState = client_i_frame_state;
            pCData->m_DataIdx = idx;
            pCData->m_DataOffset = 0;
            assert(pCData->m_pCopyData == NULL);
            DEBUG_INFO("change[%d] client_i_frame_state\n",pCData->m_ClientFd);
            return 0;
        }
        return 0;
    }

    /*if we have find this one so we should do this*/
    pCData->m_ClientState = client_seq_state;
    pCData->m_SourceIdx = pFindSData->m_DataNumber;
    pCData->m_DataIdx = findidx;
    pCData->m_DataOffset = 0;
    assert(pCData->m_pCopyData == NULL);
    DEBUG_INFO("change seq_state\n");

    /*we get the data*/
    *ppData = pFindSData->m_pData;
    datalen = pFindSData->m_DataLen;
    return 1;
}

/***********************************
*          to get the data when in client_i_frame_state
***********************************/
int  TcpSourceBuffer::__GetDataIFrameState(client_data_info_t * pCData,void ** ppData,int & datalen)
{
    int i;
    unsigned int idx;
    stream_data_info_t* pSData=NULL,*pFindSData=NULL;
    int findidx=-1;
    unsigned int expectidx;

    assert(pCData->m_ClientState == client_i_frame_state);
    assert(pCData->m_pCopyData == NULL);
    if (this->m_BufferLength  == 0)
    {
        /*nothing to get just return*/
        return 0;
    }

    pSData = this->m_pStreamData[pCData->m_DataIdx];
    if (pSData && pSData->m_DataNumber == pCData->m_SourceIdx &&
            pSData->m_DataType == I_FRAME_TYPE)
    {
        /*we find this one ,so we do the job*/
        if (pCData->m_DataOffset < pSData->m_DataLen)
        {
            (*ppData) = (void*)((unsigned char*)pSData->m_pData + pCData->m_DataOffset);
            datalen = pSData->m_DataLen - pCData->m_DataOffset;
            return 1;
        }

        /*now we have read all ,so we should get the next one*/
        expectidx = pSData->m_DataNumber + 1;
        if (pSData->m_DataNumber == MAX_FRAME_IDX)
        {
            expectidx = 0;
        }
        for (i=0; i<this->m_BufferLength; i++)
        {
            idx = this->m_StartIdx + i;
            if (this->m_BufferLength > 1)
            {
                idx %= this->m_BufferLength;
            }
            pSData = this->m_pStreamData[idx];
            if (__DataNumberGreat(pCData->m_SourceIdx,pSData->m_DataNumber)==0)
            {
                continue;
            }
            /*now it is less,so we should test whether it is the get the*/
            pFindSData = pSData;
            findidx = idx;
            break;
        }

        if (pFindSData == NULL)
        {
            /*noting to find ,so we do not set anything*/
            return 0;
        }

        assert(findidx >= 0);
        if (pFindSData->m_DataNumber == expectidx)
        {
            /*we have find this one,so we put the data*/
            pCData->m_SourceIdx = pFindSData->m_DataNumber;
            pCData->m_DataIdx = findidx;
            pCData->m_DataOffset = 0;
            /*change to the next frame ,and set to the sequential state*/
            pCData->m_ClientState = client_seq_state;
            assert(pCData->m_pCopyData == NULL);
            *ppData = pFindSData->m_pData;
            datalen = pFindSData->m_DataLen;
            DEBUG_INFO("change seq_state\n");
            return 1;
        }
        /*pass down ,we do not have the next sequential one ,so we should get the i-frame number*/
    }

    /*not this one ,so we need to read i-frame buffer*/
    pFindSData = NULL;
    findidx = -1;
    for (i=0; i<this->m_BufferLength; i++)
    {
        idx = this->m_StartIdx + i;
        idx %= this->m_BufferLength;
        pSData = this->m_pStreamData[idx];
        if (__DataNumberGreat(pCData->m_SourceIdx,pSData->m_DataNumber)==0)
        {
            continue;
        }
        /*now it is less,so we should test whether it is the get the*/
        if (pSData->m_DataType == I_FRAME_TYPE)
        {
            pFindSData = pSData;
            findidx = idx;
        }
    }

    if (pFindSData == NULL)
    {
        if (pSData)
        {
            pCData->m_DataIdx = idx;
            pCData->m_DataOffset = 0;
            pCData->m_SourceIdx = pSData->m_DataNumber;
            assert(pCData->m_pCopyData == NULL);
            assert(pCData->m_ClientState == client_i_frame_state);
        }
        return 0;
    }

    assert(findidx >= 0);
    /*we have find this one,so we put the data*/
    pCData->m_SourceIdx = pFindSData->m_DataNumber;
    pCData->m_DataIdx = findidx;
    pCData->m_DataOffset = 0;
    pCData->m_ClientState = client_i_frame_state;
    *ppData = pFindSData->m_pData;
    datalen = pFindSData->m_DataLen;
    return 1;
}

int TcpSourceBuffer::__GetDataBlockState(client_data_info_t * pCData,void ** ppData,int & datalen)
{
    int i,idx,findidx = -1;
    unsigned int expectidx;
    stream_data_info_t *pSData,*pFindSData=NULL;

    assert(pCData->m_ClientState == client_block_state);
    /*only insert the buffer length full ,block state will happen*/
    assert(this->m_BufferLength == this->m_MaxPacks);
    assert (pCData->m_pCopyData);

    pSData = pCData->m_pCopyData;
    if (pSData->m_DataLen > pCData->m_DataOffset)
    {
        /**/
        (*ppData) = (void*)((unsigned char*)pSData->m_pData + pCData->m_DataOffset);
        datalen = pSData->m_DataLen - pCData->m_DataOffset;
        return 1;
    }

    if (pSData)
    {
        /*free buffer ,and see if it is the next one*/
        __FreeStreamDataInfo(pSData);
        pCData->m_pCopyData = NULL;
        pCData->m_ClientState = client_seq_state;
        DEBUG_INFO("change[%d] client_seq_state\n",pCData->m_ClientFd);
    }

    assert(pCData->m_ClientState == client_seq_state);
    /*it over ,so we should test whether the data next is sequentail*/
    expectidx = pCData->m_SourceIdx+1;
    if (pCData->m_SourceIdx == MAX_FRAME_IDX)
    {
        expectidx = 0;
    }

    assert(this->m_BufferLength > 0);
    for (i=0; i<this->m_BufferLength; i++)
    {
        idx = this->m_StartIdx + i;
        idx %= this->m_BufferLength;
        pSData = this->m_pStreamData[idx];
        if (__DataNumberGreat(pSData->m_DataNumber,pCData->m_SourceIdx)==0)
        {
            continue;
        }
        pFindSData = pSData ;
        findidx = idx;
        break;
    }

    if (pFindSData == NULL)
    {
        /*nothing to get*/
        return 0;
    }

    if (pFindSData->m_DataNumber != expectidx)
    {
        /*we are not sequential ,so we should set i frame state*/
        DEBUG_INFO("change[%d] client_i_frame_state\n",pCData->m_ClientFd);
        pCData->m_ClientState = client_i_frame_state;
        if (pFindSData->m_DataType == I_FRAME_TYPE)
        {
            pCData->m_SourceIdx = pFindSData->m_DataNumber;
            pCData->m_DataIdx = findidx;
            pCData->m_DataOffset = 0;
            *ppData = pFindSData->m_pData;
            datalen = pFindSData->m_DataLen;
            return 1;
        }

        pFindSData = NULL;
        findidx = -1;
        for (i=0; i<this->m_BufferLength; i++)
        {
            idx = this->m_StartIdx + i;
            idx %= this->m_BufferLength;
            pSData = this->m_pStreamData[idx];
            if (__DataNumberGreat(pSData->m_DataNumber,pCData->m_SourceIdx)==0)
            {
                continue;
            }
            if (pSData->m_DataType == I_FRAME_TYPE)
            {
                pFindSData = pSData ;
                findidx = idx;
            }
        }

        if (pFindSData)
        {
            DEBUG_INFO("change[%d] client_i_frame_state\n",pCData->m_ClientFd);
            pCData->m_ClientState = client_i_frame_state;
            pCData->m_SourceIdx = pFindSData->m_DataNumber;
            pCData->m_DataIdx = findidx;
            pCData->m_DataOffset = 0;
            assert(pCData->m_pCopyData == NULL);
            *ppData = pFindSData->m_pData;
            datalen = pFindSData->m_DataLen;
            return 1;
        }

        pCData->m_DataIdx = idx;
        pCData->m_SourceIdx = pSData->m_DataNumber;
        pCData->m_DataOffset = 0;
        assert(pCData->m_ClientState == client_i_frame_state);
        assert(pCData->m_pCopyData == NULL);
        return 0;
    }

    /*we get this one
      pFindSData->m_DataNumber == expectidx
    */
    DEBUG_INFO("change[%d] client_seq_state\n",pCData->m_ClientFd);
    pCData->m_ClientState = client_seq_state;
    pCData->m_SourceIdx = pFindSData->m_DataNumber;
    pCData->m_DataIdx = findidx;
    pCData->m_DataOffset = 0;
    *ppData = pFindSData->m_pData;
    datalen = pFindSData->m_DataLen;
    assert(pCData->m_pCopyData == NULL);
    return 1;
}

int TcpSourceBuffer::__GetDataSeqState(client_data_info_t * pCData,void ** ppData,int & datalen)
{
    int i,idx,findidx = -1;
    unsigned int expectidx;
    stream_data_info_t *pSData,*pFindSData=NULL;

    assert(pCData->m_ClientState == client_seq_state);
    assert(pCData->m_pCopyData == NULL);
    assert(this->m_BufferLength > 0);
    pSData = this->m_pStreamData[pCData->m_DataIdx];
    if (pSData &&
            pSData->m_DataNumber == pCData->m_SourceIdx &&
            pSData->m_DataLen > pCData->m_DataOffset)
    {
        (*ppData) = (void*)((unsigned char*)pSData->m_pData + pCData->m_DataOffset);
        datalen = pSData->m_DataLen - pCData->m_DataOffset;
        return 1;
    }

    /*it over ,so we should test whether the data next is sequentail*/
    expectidx = pCData->m_SourceIdx+1;
    if (pCData->m_SourceIdx == MAX_FRAME_IDX)
    {
        expectidx = 0;
    }

    for (i=0; i<this->m_BufferLength; i++)
    {
        idx = this->m_StartIdx + i;
        idx %= this->m_BufferLength;
        pSData = this->m_pStreamData[idx];
        if (__DataNumberGreat(pSData->m_DataNumber,pCData->m_SourceIdx)==0)
        {
            continue;
        }

        pFindSData = pSData;
        findidx = idx;
        break;
    }

    if (pFindSData == NULL)
    {
        return 0;
    }

    if (pFindSData->m_DataNumber != expectidx)
    {
        DEBUG_INFO("change[%d] client_i_frame_state\n",pCData->m_ClientFd);
        pCData->m_ClientState = client_i_frame_state;
        pCData->m_DataOffset = 0;
        pFindSData = NULL;
        findidx = -1;
        for (i=0; i<this->m_BufferLength; i++)
        {
            idx = this->m_StartIdx + i;
            idx %= this->m_BufferLength;
            pSData = this->m_pStreamData[idx];
            if (__DataNumberGreat(pSData->m_DataNumber,pCData->m_SourceIdx)==0)
            {
                continue;
            }

            if (pSData->m_DataType == I_FRAME_TYPE)
            {
                pFindSData = pSData;
                findidx = idx;
            }
        }

        if (pFindSData == NULL)
        {
            pCData->m_DataOffset = 0;
            pCData->m_DataIdx = idx;
            pCData->m_SourceIdx = pSData->m_DataNumber;
            assert(pCData->m_ClientState == client_i_frame_state);
            assert(pCData->m_pCopyData == NULL);
            return 0;
        }

        pCData->m_SourceIdx = pFindSData->m_DataNumber;
        pCData->m_DataOffset = 0;
        pCData->m_DataIdx = findidx;
        assert(pCData->m_pCopyData == NULL);
        assert(pCData->m_ClientState == client_i_frame_state);
        *ppData = pFindSData->m_pData;
        datalen = pFindSData->m_DataLen;
        return 1;
    }
    /*now to get the sequential */
    pCData->m_ClientState = client_seq_state;
    pCData->m_DataOffset = 0;
    pCData->m_DataIdx = findidx;
    pCData->m_SourceIdx = pFindSData->m_DataNumber;
    assert(pCData->m_pCopyData == NULL);
    *ppData = pFindSData->m_pData;
    datalen = pFindSData->m_DataLen;
    return 1;

}


/***********************************
*      get data
*            client_start_state
*                   this state will be most samely as the client_i_frame_state
*                   but it will find the latest one
*
*             block_state
*                   if have pCopyData  get pCopyData
*                   not pCopyData  just test the m_SourceIdx
*                   if m_SourceIdx +1 has ,so we get it and set
*                   state to be client_seq_state
*
*             client_i_frame_state
*                   if this state ,we should get the i-frame
*                   and we find the next frame
*                   not find return 0
*
*             client_seq_state
*                   test if the next is sequence ,if not ,set it in client_i_frame_state
*                   and find the next one
*
*
***********************************/
int TcpSourceBuffer::GetData(int fd,void ** ppData,int & datalen)
{
    int ret;
    client_data_info_t* pCData=NULL;
    int i;

    if (fd < 0 )
    {
        return -EINVAL;
    }

    for (i=0; i<this->m_ClientsNumber; i++)
    {
        if (this->m_pClientDataInfo[i]->m_ClientFd == fd)
        {
            pCData = this->m_pClientDataInfo[i];
            break;
        }
    }
    if (pCData == NULL)
    {
        return -ENOENT;
    }

    switch(pCData->m_ClientState)
    {
    case client_start_state:
        ret = this->__GetDataStartState(pCData,ppData,datalen);
        DEBUG_INFO("[%d] ret %d\n",pCData->m_ClientFd,ret);
        break;
    case client_block_state:
        ret = this->__GetDataBlockState(pCData,ppData,datalen);
        DEBUG_INFO("ret[%d] %d\n",pCData->m_ClientFd,ret);
        break;
    case client_i_frame_state:
        ret = this->__GetDataIFrameState(pCData,ppData,datalen);
        //DEBUG_INFO("ret %d DataIdx %d sourceidx %d offset 0x%08x\n",
        //           ret,
        //           pCData->m_DataIdx,
        //           pCData->m_SourceIdx,
        //           pCData->m_DataOffset);
#if 0
        if (pCData->m_pCopyData)
        {
            DEBUG_INFO("pData %p length %d\n",pCData->m_pCopyData->m_pData,pCData->m_pCopyData->m_DataLen);
        }
        else
        {
            DEBUG_INFO("pData %p length %d\n",this->m_pStreamData[pCData->m_DataIdx]->m_pData,
                       this->m_pStreamData[pCData->m_DataIdx]->m_DataLen);
        }
#endif
        break;
    case client_seq_state:
        ret = this->__GetDataSeqState(pCData,ppData,datalen);
        break;
    default:
        ret = -EFAULT;
        assert(0!=0);
        break;
    }


    return ret;
}


int TcpSourceBuffer::__ForwardDataIFrameState(client_data_info_t * pCData,void * pData,int pushlen)
{
    int i,idx;
    int findidx=-1;
    stream_data_info_t *pSData,*pFindSData=NULL;

    /*now be sure that the state is i_frame_state*/
    assert(pCData->m_ClientState == client_i_frame_state);
    if (pCData->m_pCopyData)
    {
        pSData = pCData->m_pCopyData;
        ASSERT_MEMORY_IN(pData,pSData->m_pData,pSData->m_DataLen);
        ASSERT_MEMORY_IN(((unsigned char*)pData+pushlen),pCData->m_pCopyData->m_pData,pCData->m_pCopyData->m_DataLen);
        pCData->m_DataOffset += pushlen;
        DEBUG_INFO("[%d] sourceidx %d pushlen %d offset %d\n",
                   pCData->m_DataIdx,
                   pCData->m_SourceIdx,
                   pushlen ,
                   pCData->m_DataOffset);
        DEBUG_INFO("data type %d\n",pCData->m_pCopyData->m_DataType);
        return 1;
    }

    assert(this->m_BufferLength > 0);

    /*now see if the data is ok*/
    for(i=0; i<this->m_BufferLength; i++)
    {
        idx = this->m_StartIdx + i;
        idx %= this->m_BufferLength;
        pSData = this->m_pStreamData[idx];
        if (pSData->m_DataNumber==pCData->m_SourceIdx)
        {
            pFindSData = pSData;
            findidx = idx;
            break;
        }
    }

    assert (pFindSData);
    assert(findidx >= 0);
    ASSERT_MEMORY_IN(pData,pFindSData->m_pData,pFindSData->m_DataLen);
    ASSERT_MEMORY_IN(((unsigned char*)pData+pushlen),pFindSData->m_pData,pFindSData->m_DataLen);
    pCData->m_DataOffset += pushlen;
    //DEBUG_INFO("pushlen %d offset %d\n",pushlen,pCData->m_DataOffset);
    //DEBUG_INFO("[%d] sourceidx %d pushlen %d offset %d\n",
    //           pCData->m_DataIdx,
    //           pCData->m_SourceIdx,
    //           pushlen ,
    //           pCData->m_DataOffset);
    //DEBUG_INFO("data type %d\n",pFindSData->m_DataType);
    return 1;
}

int TcpSourceBuffer::__ForwardDataBlockState(client_data_info_t * pCData,void * pData,int pushlen)
{
    int i,idx,findidx = -1;
    stream_data_info_t *pSData,*pFindSData=NULL;

    /*now be sure that the state is i_frame_state*/
    assert(pCData->m_ClientState == client_block_state);
    assert(pCData->m_pCopyData);
    if (pCData->m_pCopyData)
    {
        ASSERT_MEMORY_IN(pData,pCData->m_pCopyData->m_pData,pCData->m_pCopyData->m_DataLen);
        ASSERT_MEMORY_IN(((unsigned char*)pData+pushlen),pCData->m_pCopyData->m_pData,pCData->m_pCopyData->m_DataLen);
        pCData->m_DataOffset += pushlen;
        DEBUG_INFO("[%d] sourceidx %d pushlen %d offset %d\n",
                   pCData->m_DataIdx,
                   pCData->m_SourceIdx,
                   pushlen ,
                   pCData->m_DataOffset);
        DEBUG_INFO("data type %d\n",pCData->m_pCopyData->m_DataType);
        return 1;
    }

    assert(this->m_BufferLength > 0);

    /*now see if the data is ok*/
    for(i=0; i<this->m_BufferLength; i++)
    {
        idx = this->m_StartIdx + i;
        idx %= this->m_BufferLength;
        pSData = this->m_pStreamData[idx];
        if (pSData->m_DataNumber==pCData->m_SourceIdx)
        {
            pFindSData = pSData;
            findidx = idx;
            break;
        }
    }

    assert (pFindSData);
    assert(findidx >= 0);
    ASSERT_MEMORY_IN(pData,pFindSData->m_pData,pFindSData->m_DataLen);
    ASSERT_MEMORY_IN(((unsigned char*)pData+pushlen),pFindSData->m_pData,pFindSData->m_DataLen);
    pCData->m_DataOffset += pushlen;
    DEBUG_INFO("[%d] sourceidx %d pushlen %d offset %d\n",
               pCData->m_DataIdx,
               pCData->m_SourceIdx,
               pushlen ,
               pCData->m_DataOffset);
    DEBUG_INFO("data type %d\n",pFindSData->m_DataType);
    return 1;
}


int TcpSourceBuffer::__ForwardDataSeqState(client_data_info_t * pCData,void * pData,int pushlen)
{
    int i,idx,findidx = -1;
    stream_data_info_t *pSData,*pFindSData=NULL;

    /*now be sure that the state is i_frame_state*/
    assert(pCData->m_ClientState == client_seq_state);
    assert(pCData->m_pCopyData == NULL);

    assert(this->m_BufferLength > 0);

    /*now see if the data is ok*/
    for(i=0; i<this->m_BufferLength; i++)
    {
        idx = this->m_StartIdx + i;
        idx %= this->m_BufferLength;
        pSData = this->m_pStreamData[idx];
        if (pSData->m_DataNumber==pCData->m_SourceIdx)
        {
            pFindSData = pSData;
            findidx = idx;
            break;
        }
    }

    assert (pFindSData);
    assert(findidx >= 0);
    ASSERT_MEMORY_IN(pData,pFindSData->m_pData,pFindSData->m_DataLen);
    ASSERT_MEMORY_IN(((unsigned char*)pData+pushlen),pFindSData->m_pData,pFindSData->m_DataLen);
    pCData->m_DataOffset += pushlen;
    //DEBUG_INFO("[%d] sourceidx %d pushlen %d offset %d\n",
     //          pCData->m_DataIdx,
    //           pCData->m_SourceIdx,
    //           pushlen ,
    //           pCData->m_DataOffset);
    //DEBUG_INFO("data type %d\n",pFindSData->m_DataType);
    return 1;
}


int TcpSourceBuffer::ForwardData(int fd,void * pData,int pushlen)
{
    client_data_info_t* pCData=NULL;
    int i;
    int ret;

    for (i=0; i<this->m_ClientsNumber; i++)
    {
        if (this->m_pClientDataInfo[i] &&
                this->m_pClientDataInfo[i]->m_ClientFd == fd)
        {
            pCData = this->m_pClientDataInfo[i];
            break;
        }
    }
    if (pCData == NULL)
    {
        return -ENOENT;
    }

    switch(pCData->m_ClientState)
    {
    case client_i_frame_state:
        ret = this->__ForwardDataIFrameState(pCData,pData,pushlen);
        break;
    case client_seq_state:
        ret = this->__ForwardDataSeqState(pCData,pData,pushlen);
        break;
    case client_block_state:
        ret = this->__ForwardDataBlockState(pCData,pData,pushlen);
        break;
    default:
        assert(0!=0);
        return -EFAULT;
    }

    return ret;
}


int TcpSourceBuffer::GetClients()
{
    return this->m_ClientsNumber;
}

int TcpSourceBuffer::__ExpandClients(int fd)
{
    client_data_info_t* pCData=NULL;
    int size;
    client_data_info_t **pTmpCDataArray=NULL;

    pCData = (client_data_info_t*)malloc(sizeof(*pCData));
    if (pCData == NULL)
    {
        return -ENOMEM;
    }

    memset(pCData,0,sizeof(*pCData));
    pCData->m_ClientFd = fd;
    pCData->m_ClientState = client_start_state;
    pCData->m_DataIdx = 0;
    pCData->m_DataOffset = 0;
    pCData->m_pCopyData = NULL;
    pCData->m_SourceIdx = 0;

    size = (this->m_ClientsNumber + 1) * sizeof(*pTmpCDataArray);
    pTmpCDataArray = (client_data_info_t**) malloc(size);
    if (pTmpCDataArray == NULL)
    {
        free(pCData);
        return -ENOMEM;
    }

    if (this->m_ClientsNumber > 0)
    {
        memcpy(pTmpCDataArray,this->m_pClientDataInfo,this->m_ClientsNumber* sizeof(*pTmpCDataArray));
        free(this->m_pClientDataInfo);
    }

    pTmpCDataArray[this->m_ClientsNumber] = pCData;
    this->m_pClientDataInfo = pTmpCDataArray;
    this->m_ClientsNumber += 1;
    return 1;

}


int TcpSourceBuffer::StartData(int fd)
{
    this->__ShrinkClients(fd);
    /*now to insert it*/
    return this->__ExpandClients(fd);
}

int TcpSourceBuffer::GetStreamFormat(format_data_t * pFormat)
{
    if (this->m_pStream == NULL)
    {
        return -ENOENT;
    }

    return this->m_pStream->GetSFormat(pFormat);
}
