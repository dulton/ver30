
#include <tcp_stream_acc.h>
#include <ev.h>
#include <memory>

#if 1
#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%d[%ld]\t",__FILE__,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)
#else
#define DEBUG_INFO(...)
#endif

TcpStreamAcc::TcpStreamAcc(int port,int * pRunningBits,int maxclients,int maxpacket)
    : m_pRunbits(pRunningBits) ,
      m_Port (port),
      m_MaxClients(maxclients),
      m_MaxPackets(maxpacket)
{
    m_AccSock = -1;
    /*for read timer*/
    m_HasInsertTimer = 0;
    /*for write timer*/
    m_HasInsertTimerOut = 0;
    /*we do not init any timer out */
    m_HasInsertIO = 0;
    m_HasInsertSigTimer = 0;
    m_Streams = 0;
    m_pSourceBufferArray = NULL;
    m_StreamDatas = 0;
    m_pStreamDataArray = NULL;
}

void TcpStreamAcc::__StopTimer()
{
    if (this->m_HasInsertTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvTimerRead));
    }
    this->m_HasInsertTimer = 0;

    if (this->m_HasInsertTimerOut)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvTimerOut));
    }
    this->m_HasInsertTimerOut = 0;
    return ;
}

void TcpStreamAcc::__StopSigTimer()
{
    if (this->m_HasInsertSigTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvTimerSig));
    }
    this->m_HasInsertSigTimer = 0;
    return;
}


void TcpStreamAcc::__ResetAcc()
{
    int i;

    /*we should stop timer first and let it ok*/
    this->__StopTimer();
    this->__StopSigTimer();
    if (this->m_HasInsertIO)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvIoAcc));
    }
    this->m_HasInsertIO = 0;


    while(this->m_StreamDatas > 0)
    {
        assert(this->m_pStreamDataArray[0]);
        this->FreeTcpStreamData(this->m_pStreamDataArray[0]);
    }

    if (this->m_pStreamDataArray)
    {
        free(this->m_pStreamDataArray);
    }
    this->m_pStreamDataArray = NULL;
    this->m_StreamDatas = 0;

    for (i=0; i<this->m_Streams; i++)
    {
        if (this->m_pSourceBufferArray[i])
        {
            delete this->m_pSourceBufferArray[i];
        }
        this->m_pSourceBufferArray[i] = NULL;
    }

    if (this->m_pSourceBufferArray)
    {
        free(this->m_pSourceBufferArray);
    }
    this->m_pSourceBufferArray = NULL;
    this->m_Streams = 0;

    if (this->m_AccSock >= 0)
    {
        close(this->m_AccSock);
    }
    this->m_AccSock = -1;
    return ;
}

TcpStreamAcc::~TcpStreamAcc()
{
    this->__ResetAcc();
    this->m_pRunbits = NULL;
    this->m_Port = 0;
    this->m_MaxClients = 0;
    this->m_MaxPackets = 0;
}

int TcpStreamAcc::__SetSocketUnBlock(int sock)
{
    int ret;
    int flags;

    errno = 0;
    flags = fcntl(sock,F_GETFL);
    if (flags == -1 && errno)
    {
        ret = -errno;
        return ret;
    }

    ret = fcntl(sock,F_SETFL,flags | O_NONBLOCK);
    if (ret < 0)
    {
        ret = -errno ?   -errno :  -1;
        return ret;
    }
    return 0;
}

int TcpStreamAcc::__BindPort()
{
    int ret;
    struct sockaddr_in sinaddr;
    socklen_t saddrlen;
    if (this->m_Port <= 0 || this->m_Port > 0xffff)
    {
        return -EINVAL;
    }

    this->m_AccSock = socket(AF_INET,SOCK_STREAM,0);
    if (this->m_AccSock == -1)
    {
        return -errno ? -errno : -1;
    }

    memset(&(sinaddr),0,sizeof(sinaddr));
    sinaddr.sin_family = AF_INET;
    sinaddr.sin_addr.s_addr = INADDR_ANY;
    sinaddr.sin_port = htons (  this->m_Port );
    saddrlen = sizeof ( sinaddr );
    ret = bind(this->m_AccSock,(struct sockaddr*)&sinaddr,saddrlen);
    if (ret < 0)
    {
        ret = -errno ? -errno : -1;
        return ret;
    }

    ret = this->__SetSocketUnBlock(this->m_AccSock);
    if (ret < 0)
    {
        return ret;
    }

    ret = listen(this->m_AccSock,10);
    if (ret < 0)
    {
        ret = -errno ? -errno : -1;
        return ret;
    }

    return 0;
}

int TcpStreamAcc::__ExpandStreamData(TcpStreamData * pData)
{
    TcpStreamData** pTmpStreamArray = NULL;
    int size;

    size = (this->m_StreamDatas + 1) * sizeof(pData);
    pTmpStreamArray = (TcpStreamData**)malloc(size);
    if (pTmpStreamArray == NULL)
    {
        return -ENOMEM;
    }

    memset(pTmpStreamArray,0,size);
    if (this->m_StreamDatas > 0)
    {
        assert(this->m_pStreamDataArray);
        memcpy(pTmpStreamArray,this->m_pStreamDataArray,sizeof(pData)*this->m_StreamDatas);
        free(this->m_pStreamDataArray);
    }
    pTmpStreamArray[this->m_StreamDatas] = pData;
    this->m_pStreamDataArray = pTmpStreamArray;
    this->m_StreamDatas += 1;
    return 0;
}

int TcpStreamAcc::__AcceptStreamClientImpl()
{
    struct sockaddr_in sinaddr;
    socklen_t saddrlen;
    int ret,clisock=-1;
    TcpStreamData *pStreamData = NULL;
    saddrlen = sizeof(sinaddr);
    clisock = accept(this->m_AccSock,(struct sockaddr*)&sinaddr,&saddrlen);
    DEBUG_INFO("accept %d\n",clisock);
    if (clisock < 0)
    {
        ret = -errno ? -errno : -1;
        /*these errors are normal*/
        if (errno == EAGAIN ||
                errno == EWOULDBLOCK ||
                errno == EINTR)
        {
            ret = 0;
        }

        return ret;
    }

    ret = this->__SetSocketUnBlock(clisock);
    if (ret < 0)
    {
        close(clisock);
        return 0;
    }

    /*now we should test if we are the most*/
    if (this->m_MaxClients != 0 && this->m_StreamDatas == this->m_MaxClients)
    {
        /*we can not accept this socket ,just close for it*/
        close(clisock);
        return 0;
    }

    pStreamData = new TcpStreamData(clisock,this);
    clisock = -1;

    ret = pStreamData->StartData();
    if (ret < 0)
    {
        delete pStreamData;
        return 0;
    }

    /*now we should insert the stream into the client data array*/
    ret = this->__ExpandStreamData(pStreamData);
    if (ret < 0)
    {
        delete pStreamData;
        return ret;
    }

    return 1;
}

void TcpStreamAcc::__AcceptStreamClient(EV_P_ ev_io * w,int revents,void * arg)
{
    TcpStreamAcc* pThis = (TcpStreamAcc*)arg;
    int ret;
    ret = pThis->__AcceptStreamClientImpl();
    if (ret < 0)
    {
        pThis->__BreakRunLoop();
    }
    pThis->__TestRunBits();
    return;
}

int TcpStreamAcc::__InitAccIo()
{
    /*do not init any timer read ,
      because when we has some client, we should read*/

    /*init io */
    assert(this->m_AccSock >= 0);
    ev_io_init(&(this->m_EvIoAcc),TcpStreamAcc::__AcceptStreamClient,this->m_AccSock,EV_READ,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvIoAcc));
    this->m_HasInsertIO  = 1;
    return 0;
}

int TcpStreamAcc::__InitStreamSource()
{
    int size;
    assert(this->m_pSourceBufferArray == NULL);
    assert(this->m_Streams == 0);
    size = MAX_STREAM_IDS * sizeof(this->m_pSourceBufferArray[0]);
    this->m_pSourceBufferArray = (TcpSourceBuffer**)malloc(size);
    if (this->m_pSourceBufferArray == NULL)
    {
        return -ENOMEM;
    }

    memset(this->m_pSourceBufferArray,0,size);
    this->m_Streams = MAX_STREAM_IDS;
    return 0;
}


int TcpStreamAcc::RunLoop()
{
    int ret;

    this->__ResetAcc();

    /*now we should reinitialize the run loop*/
    ret =this->__BindPort();
    if (ret < 0)
    {
        this->__ResetAcc();
        return ret;
    }

    ret = this->__InitAccIo();
    if (ret < 0)
    {
        this->__ResetAcc();
        return ret;
    }

    ret = this->__InitStreamSource();
    if (ret < 0)
    {
        this->__ResetAcc();
        return ret;
    }

    ret = this->__InitSigTimer();
    if (ret < 0)
    {
        this->__ResetAcc();
        return ret;
    }

    ev_run(EV_DEFAULT,0);

    return 0;
}


int TcpStreamAcc::AllocateStreamSource(TcpStreamData * pStreamData,int streamid)
{
    int ret;
    int inittimer=1;
    int allocated=0;
    int i;

    if (streamid < 0 || streamid >= MAX_STREAM_IDS)
    {
        return -EINVAL;
    }

    for (i=0; i<this->m_Streams ; i++)
    {
        if (this->m_pSourceBufferArray[i])
        {
            inittimer = 0;
            break;
        }
    }

    if (this->m_pSourceBufferArray[streamid] == NULL)
    {
        /*if we have not init this streamid ,so we should start it*/

        this->m_pSourceBufferArray[streamid] = new TcpSourceBuffer(this->m_MaxPackets,this->m_MaxClients,streamid);
        ret = this->m_pSourceBufferArray[streamid]->InitStream();

        DEBUG_INFO("[%d]ret = %d\n",streamid,ret);
        if (ret < 0)
        {
            delete this->m_pSourceBufferArray[streamid];
            this->m_pSourceBufferArray[streamid] = NULL;
            DEBUG_INFO("\n");
            return ret;
        }
        allocated = 1;
    }

    ret = this->m_pSourceBufferArray[streamid]->StartData(pStreamData->GetSocket());
    if (ret < 0)
    {
        if (allocated)
        {
            delete this->m_pSourceBufferArray[streamid];
            this->m_pSourceBufferArray[streamid] = NULL;
        }
        DEBUG_INFO("\n");

        return ret;
    }

    ret = pStreamData->SetSourceBuffer(this->m_pSourceBufferArray[streamid]);
    if (ret < 0)
    {
        if (allocated)
        {
            delete this->m_pSourceBufferArray[streamid];
            this->m_pSourceBufferArray[streamid] = NULL;
        }
        DEBUG_INFO("\n");

        return ret;
    }

    if (inittimer)
    {
        ret = this->__InitTimer();
        if (ret < 0)
        {
            if (allocated)
            {
                delete this->m_pSourceBufferArray[streamid];
                this->m_pSourceBufferArray[streamid] = NULL;
            }
            return ret;
        }
    }

    return 0;
}

void TcpStreamAcc::FreeTcpStreamData(TcpStreamData * pStreamData)
{
    int stoptimer = 1;
    int i,streamid;

    /*now first to */
    streamid = pStreamData->GetStreamId();
    if (streamid >= 0 && streamid < MAX_STREAM_IDS)
    {
        /*nothing to do*/
        if (this->m_pSourceBufferArray[streamid])
        {
            this->m_pSourceBufferArray[streamid]->ClearData(pStreamData->GetSocket());
            if (this->m_pSourceBufferArray[streamid]->GetClients() ==0)
            {
                delete this->m_pSourceBufferArray[streamid];
                this->m_pSourceBufferArray[streamid] = NULL;
            }
        }
    }

    /*now to clear*/
    this->__ShrinkStreamData(pStreamData);

    for (i=0; i<this->m_Streams; i++)
    {
        if (this->m_pSourceBufferArray[i])
        {
            stoptimer = 0;
            break;
        }
    }

    if (stoptimer)
    {
        DEBUG_INFO("stoptimer %d streamid(%d) time %ld\n",stoptimer,streamid,time(NULL));
        assert(this->m_StreamDatas == 0);
        this->__StopTimer();
    }

    return ;

}

void TcpStreamAcc::__ReadTimeout(EV_P_ ev_timer * w,int revents,void * arg)
{
    TcpStreamAcc* pThis = (TcpStreamAcc*)arg;
    pThis->__BreakRunLoop();
    DEBUG_INFO("\n");
    return;
}

void TcpStreamAcc::__SigTimer(EV_P_ ev_timer * w,int revents,void * arg)
{
    TcpStreamAcc* pThis = (TcpStreamAcc*)arg;
    if (pThis->m_pRunbits && *(pThis->m_pRunbits) == 0)
    {
        pThis->__BreakRunLoop();
    }
    pThis->__StopSigTimer();
    pThis->__InitSigTimer();
    return;
}


int TcpStreamAcc::__InitSigTimer()
{
    assert(this->m_HasInsertSigTimer==0);
    ev_timer_init(&(this->m_EvTimerSig),TcpStreamAcc::__SigTimer,1.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvTimerSig));
    this->m_HasInsertSigTimer = 1;
    return 0;
}

int TcpStreamAcc::__InitTimer()
{
    /*we set the 40 milliseconds to read once*/
    ev_timer_init(&(this->m_EvTimerRead),TcpStreamAcc::__ReadStreamSource,0.04,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvTimerRead));
    this->m_HasInsertTimer = 1;
    /*we set 3 seconds for not read the timer*/
    ev_timer_init(&(this->m_EvTimerOut),TcpStreamAcc::__ReadTimeout,3.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvTimerOut));
    this->m_HasInsertTimerOut = 1;
    return 0;
}

int TcpStreamAcc::__ResetReadTimer(void)
{
    if (this->m_HasInsertTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvTimerRead));
    }
    this->m_HasInsertTimer = 0;
    /*we set the 40 milliseconds to read once*/
    ev_timer_init(&(this->m_EvTimerRead),TcpStreamAcc::__ReadStreamSource,0.04,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvTimerRead));
    this->m_HasInsertTimer =1;
    return 0;
}


void TcpStreamAcc::__BreakRunLoop()
{
    ev_break (EV_DEFAULT,EVBREAK_ONE);
}

void TcpStreamAcc::__TestRunBits()
{
    if (this->m_pRunbits && *(this->m_pRunbits) == 0)
    {
        this->__BreakRunLoop();
    }
}

void TcpStreamAcc::__ShrinkStreamData(TcpStreamData * pData)
{
    int i=0;
    int delidx=-1;
    if (this->m_StreamDatas > 0)
    {
        for (i=0; i<this->m_StreamDatas; i++)
        {
            if (pData == this->m_pStreamDataArray[i])
            {
                delidx = i;
                break;
            }
        }

        if (delidx >= 0)
        {
            if (this->m_StreamDatas > 1)
            {
                this->m_pStreamDataArray[delidx] = NULL;
                i = delidx;
                /*we get the next one,so move it back*/
                i ++;
                while( i < this->m_StreamDatas)
                {
                    this->m_pStreamDataArray[i-1] = this->m_pStreamDataArray[i];
                    i ++;
                }
                this->m_StreamDatas -= 1;
            }
            else
            {
                free(this->m_pStreamDataArray);
                this->m_pStreamDataArray = NULL;
                this->m_StreamDatas = 0;
            }
        }
    }

    return ;
}


int TcpStreamAcc::__ReadStreamSourceImpl()
{
    int i,j,k,l;
    int ret,hasread=0;
    std::auto_ptr<int> pNotifyFds2(new int[3]);
    int *pNotifyFds=pNotifyFds2.get();
    std::auto_ptr<void*> pFreeData2(new ptr_t[3]);
    ptr_t* pFreeData = pFreeData2.get();
    int removed=0;
    int curread = 0;
    if (this->m_StreamDatas > 0)
    {
        pNotifyFds2.reset(new int[this->m_StreamDatas]);
        pNotifyFds = pNotifyFds2.get();
        pFreeData2.reset(new ptr_t[this->m_StreamDatas]);
        pFreeData = pFreeData2.get();

        for (i=0; i<this->m_StreamDatas; i++)
        {
            pFreeData[i] = NULL;
        }
    }

    for (i=0; i<this->m_Streams; i++)
    {
        if (this->m_pSourceBufferArray[i])
        {
            do
            {
                curread = 0;
                ret = this->m_pSourceBufferArray[i]->PullData(pNotifyFds,this->m_StreamDatas);
                if (ret < 0)
                {
                    /*we read data error*/
                    return ret;
                }
                else if (ret > 0)
                {
                    hasread = 1;
                    curread = 1;
                    for (j=0; j<this->m_StreamDatas; j++)
                    {
                        if (pNotifyFds[j] == -1)
                        {
                            continue;
                        }
                        for (k=0; k<this->m_StreamDatas; k++)
                        {
                            if (this->m_pStreamDataArray[k]->GetSocket() == pNotifyFds[j])
                            {

                                /*now we should test whether it is in the remove one*/
                                if (removed > 0)
                                {
                                    for (l=0; l<removed; l++)
                                    {
                                        /*we have put it to the removed ,so scan for next one*/
                                        if (((void*)this->m_pStreamDataArray[k]) == (void*)pFreeData[l])
                                        {
                                            goto next_cycle;
                                        }
                                    }
                                }
                                ret = this->m_pStreamDataArray[k]->ResetToRunning();
                                if (ret < 0)
                                {
                                    /*now we should remove this*/
                                    pFreeData[removed] = this->m_pStreamDataArray[k];
                                    removed ++;
                                }
                            }
next_cycle:
                            removed = removed;
                        }
                    }
                }
            }
            while (curread > 0);
        }
    }

    if (hasread > 0)
    {
        this->__ResetTimeoutTimer();
    }

    /*if we need remove error tcp stream data ,just remove here not modify the Array of stream data*/
    if (removed > 0)
    {
        /*this may be stop timer ,so we put here */
        for (i=0; i<removed; i++)
        {
            TcpStreamData* pData;
            pData = (TcpStreamData*)pFreeData[i];
            this->FreeTcpStreamData(pData);
        }
    }
    this->__ResetReadTimer();


    return 0;
}


int TcpStreamAcc::__ResetTimeoutTimer(void)
{
    if (this->m_HasInsertTimerOut)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvTimerOut));
    }
    /*we set 3 seconds for not read the timer*/
    ev_timer_init(&(this->m_EvTimerOut),TcpStreamAcc::__ReadTimeout,3.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvTimerOut));
    this->m_HasInsertTimerOut = 1;
    return 0;
}
void TcpStreamAcc::__ReadStreamSource(EV_P_ ev_timer * w,int revents,void * arg)
{
    TcpStreamAcc* pThis = (TcpStreamAcc*)arg;
    int ret;

    ret = pThis->__ReadStreamSourceImpl();
    if (ret < 0)
    {
        pThis->__BreakRunLoop();
    }

    return;
}
