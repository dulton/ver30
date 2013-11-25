
#include <tcp_stream_data.h>
#include <memory>

#if 1
#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%d[%ld]\t",__FILE__,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)
#else
#define DEBUG_INFO(...)
#endif
#define ERROR_INFO(...) do{fprintf(stderr,"%s:%d\t",__FILE__,__LINE__);fprintf(stderr,__VA_ARGS__);}while(0)
TcpStreamData::TcpStreamData(int sock,TcpStreamAcc * pAcc) : m_Sock(sock),m_pStreamAcc(pAcc)
{
    m_StreamId = -1;
    m_State = stop_state;
    m_LengthSendSize = 0;
    m_HasInsertTimer = 0;
    m_HasInsertWrite = 0;
    m_HasInsertRead = 0;
    m_pSourceBuffer = NULL;
}

void TcpStreamData::__Reset()
{
    /*now first to delete timer*/
    this->__StopReadIo();
    this->__StopWriteIo();
    this->__StopTimer();
    if (this->m_pStreamAcc)
    {
        this->m_pStreamAcc->FreeTcpStreamData(this);
    }
    /*we put the source buffer for reset */
    this->m_pSourceBuffer = NULL;
    m_State = stop_state;
    m_LengthSendSize = 0;
    m_StreamId = -1;
    return;
}

TcpStreamData::~TcpStreamData()
{
    this->__Reset();
    this->m_pStreamAcc = NULL;
    if (this->m_Sock >= 0)
    {
        close(this->m_Sock);
    }
    this->m_Sock = -1;
    DEBUG_INFO("\n");
}

int TcpStreamData::__StartReadIo()
{
    assert(this->m_HasInsertRead == 0);
    assert(this->m_Sock >= 0);
    ev_io_init(&(this->m_EvIoRead),TcpStreamData::__ReadSocket,this->m_Sock,EV_READ,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvIoRead));
    this->m_HasInsertRead = 1;
    return 0;
}

void TcpStreamData::__StopReadIo()
{
    if (this->m_HasInsertRead)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvIoRead));
    }
    this->m_HasInsertRead = 0;
    return;
}

int TcpStreamData::__StartWriteIo()
{
    assert(this->m_HasInsertWrite ==0);
    assert(this->m_Sock >= 0);
    ev_io_init(&(this->m_EvIoWrite),TcpStreamData::__WriteSocket,this->m_Sock,EV_WRITE,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvIoWrite));
    this->m_HasInsertWrite = 1;
    return 0;
}

void TcpStreamData::__StopWriteIo()
{
    if (this->m_HasInsertWrite)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvIoWrite));
    }
    this->m_HasInsertWrite = 0;
    return;
}

int TcpStreamData::__StartTimer()
{
    assert(this->m_HasInsertTimer == 0);
    assert(this->m_Sock >= 0);
    /*we put 3 seconds for timer*/
    ev_timer_init(&(this->m_EvTimer),TcpStreamData::__Timeout,3.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvTimer));
    this->m_HasInsertTimer = 1;
    return 0;
}

void TcpStreamData::__StopTimer()
{
    if (this->m_HasInsertTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvTimer));
    }
    this->m_HasInsertTimer = 0;
    return;
}


int TcpStreamData::StartData()
{
    int ret;
    this->__Reset();

    /*now we should start the read io and then we can write*/
    ret = this->__StartReadIo();
    if (ret < 0)
    {
        this->__Reset();
        return ret;
    }

    ret = this->__StartTimer();
    if (ret < 0)
    {
        this->__Reset();
        return ret;
    }

    /*we need to do the read state*/
    this->m_State = read_state;

    return 0;
}

void TcpStreamData::__Timeout(EV_P_ ev_timer * w,int revents,void * arg)
{
    TcpStreamData* pThis = (TcpStreamData*)arg;
    /*that is out ,so we should delete this*/
    DEBUG_INFO("[%d] timeout\n",pThis->m_Sock);
    delete pThis;
    return;
}

void TcpStreamData::__ReadSocket(EV_P_ ev_io * w,int revents,void * arg)
{
    TcpStreamData* pThis = (TcpStreamData*)arg;
    int ret;
    ret = pThis->__ReadSocketImpl();
    if(ret < 0)
    {
        /*we should delete this*/
        delete pThis;
    }
    return;
}

int TcpStreamData::__ReadSocketImpl()
{
    int streamid;
    int size=sizeof(streamid);
    int ret;

    errno = 0;
    ret = recv(this->m_Sock,&streamid,size, MSG_DONTWAIT);
    DEBUG_INFO("read[%d] %d\n",this->m_Sock,ret);
    if (ret < 0)
    {
        ret = -errno ? -errno : -1;
        if (errno == EAGAIN || errno == EINTR ||
                errno == EWOULDBLOCK)
        {
            ret =  0;
        }
        return ret;
    }

    if (ret != size)
    {
        /*not valid one*/
        return -EINVAL;
    }
    DEBUG_INFO("[%d]streamid %d\n",this->m_Sock,streamid);
    this->m_StreamId = streamid;

    /*now we should set for the buffer*/
    ret= this->m_pStreamAcc->AllocateStreamSource(this,streamid);
    if (ret < 0)
    {
        ERROR_INFO("\n");
        return ret;
    }
    /*we set the source buffer in AllocateStreamSource*/
    assert(this->m_pSourceBuffer);
    /*now to set the timer*/
    this->__StopTimer();
    this->__StopReadIo();
    ret = this->__StartWriteIo();
    if (ret < 0)
    {
        ERROR_INFO("\n");
        return ret;
    }
    ret = this->__StartTimer();
    if (ret < 0)
    {
        ERROR_INFO("\n");
        return ret;
    }
    /*now we should set the state of response state*/
    this->m_State = response_state;
    return 0;
}


void TcpStreamData::__WriteSocket(EV_P_ ev_io * w,int revents,void * arg)
{

    TcpStreamData* pThis = (TcpStreamData*)arg;
    int ret;
    ret = pThis->__WriteSocketImpl();
    if (ret < 0)
    {
        DEBUG_INFO("\n");
        delete pThis;
    }
    return;
}

int TcpStreamData::__WriteNonBlock(void * pData,int size)
{
    int ret;
    unsigned char *pCur=(unsigned char*)pData;
    int leftlen = size;
    int writelen = 0;

    while(leftlen > 0)
    {
        errno = 0;
        ret = send(this->m_Sock,pCur,leftlen,MSG_DONTWAIT);
        if (ret < 0)
        {
            ret = -errno ? -errno : -1;
            if (errno != EWOULDBLOCK &&
                    errno != EAGAIN &&
                    errno != EINTR)
            {
                return ret;
            }
            DEBUG_INFO("write[%d] %d errno = %d\n",this->m_Sock,writelen,errno);
            return writelen;
        }
        leftlen -= ret;
        writelen += ret;
        pCur += ret;
    }
    return writelen;

}

#define  ASSERT_WRITE_LENGTH_STATE()\
do\
{\
    if (this->m_State != write_length_state)\
        {\
            DEBUG_INFO("state %d not write_length_state\n",this->m_State);\
            assert(this->m_State == write_length_state);\
        }\
}while(0)
int TcpStreamData::__WriteSocketImpl()
{
    int ret;
    int sendsth = 0;
    int hassend=0;

    if (this->m_State == response_state)
    {
        std::auto_ptr<format_data_t> pFormat2(new format_data_t);
        format_data_t *pFormat = pFormat2.get();
        assert(this->m_pSourceBuffer);
        ret = this->m_pSourceBuffer->GetStreamFormat(pFormat);
        if (ret < 0)
        {
            return ret;
        }
        /*now we should write stream format*/
        DEBUG_INFO("[%d]write %d size\n",this->m_Sock,sizeof(*pFormat));
        ret = send(this->m_Sock,pFormat,sizeof(*pFormat),MSG_DONTWAIT);
        if (ret != sizeof(*pFormat))
        {
            ret = -errno ? -errno : -1;
            return ret;
        }
        hassend = sizeof(*pFormat);

        /*now change the state*/
        this->m_State = write_length_state;
        sendsth = 1;

    }
    else
    {
        /*now we get data*/
        void* pData=NULL;
        int datalen =0;
        int curlen =0;

next_read:
        assert(pData == NULL);
        assert(datalen == 0);
        ret = this->m_pSourceBuffer->GetData(this->m_Sock,&pData,datalen);
        if (ret < 0)
        {
            DEBUG_INFO("\n");
            return ret;
        }
        else if (ret == 0)
        {
            /*now to set for the */
            ASSERT_WRITE_LENGTH_STATE();
            /*we stop write io detect ,it will wakeup by reset running*/
            this->__StopWriteIo();
            goto out;
        }
        assert(curlen == 0);
        /*now we have test we have send length*/
        if (this->m_State == write_length_state)
        {
            /*now to write for the */
            unsigned char* pCurPtr=((unsigned char*)&(datalen)) + this->m_LengthSendSize;
            int leftlen = sizeof(datalen) - this->m_LengthSendSize;
            ret = this->__WriteNonBlock(pCurPtr,leftlen);
            if (ret < 0)
            {
                DEBUG_INFO("[%d] %d\n",this->m_Sock,ret);
                return ret;
            }
            else if (ret < leftlen)
            {
                this->m_LengthSendSize += ret;
                if (ret > 0)
                {
                    sendsth = 1;
                }
                goto out;
            }

            this->m_State = write_data_state;
            this->m_LengthSendSize = 0;
            sendsth = 1;
        }

        assert(this->m_State == write_data_state);
        curlen = this->__WriteNonBlock(pData,datalen);
        if (curlen < 0)
        {
            ret = curlen;
            return ret;
        }
        hassend += curlen;
        ret = this->m_pSourceBuffer->ForwardData(this->m_Sock,pData,curlen);
        if (ret < 0)
        {
            DEBUG_INFO("\n");
            return ret;
        }
        sendsth =1;
        if (curlen == datalen)
        {
            /*we have send all the data over ,so get the next data*/
            this->m_State = write_length_state;
            this->m_LengthSendSize = 0;
            pData = NULL;
            datalen = 0;
            curlen = 0;
            goto next_read;
        }
        //DEBUG_INFO("pData %p curlen %d datalen %d\n",pData,curlen,datalen);
        pData = NULL;
        datalen = 0;
        curlen = 0;
    }
out:
    if (sendsth)
    {
        //DEBUG_INFO("time %ld\n",time(NULL));
        ret = this->__ResetTimer();
        if (ret < 0)
        {
            DEBUG_INFO("\n");
            return ret;
        }
        DEBUG_INFO("[%d] reset timer\n",this->m_Sock);

    }
    else
    {
        /*if we do not write anything ,it means ,we should stop for a while*/
        if (this->m_State == write_length_state)
        {
            this->__StopWriteIo();
        }
    }
    return sendsth ;
}

int TcpStreamData::__ResetTimer()
{
    int ret;
    this->__StopTimer();
    ret = this->__StartTimer();
    return ret;
}


int TcpStreamData::ResetToRunning(void)
{
    int ret=-EINVAL;
    if (this->m_State != write_length_state )
    {
        return 0;
    }

    //DEBUG_INFO("\n");
    assert(this->m_State == write_length_state);
    /*because we only in write length state*/
    this->__StopWriteIo();
    ret = this->__StartWriteIo();

    return ret;
}


int TcpStreamData::SetSourceBuffer(TcpSourceBuffer * pBuffer)
{
    assert(this->m_pSourceBuffer==NULL);
    this->m_pSourceBuffer = pBuffer;
    return 0;
}

int TcpStreamData::GetSocket()
{
    return this->m_Sock;
}

int TcpStreamData::GetStreamId()
{
    return this->m_StreamId;
}
