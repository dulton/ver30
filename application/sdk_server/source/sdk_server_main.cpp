
#include <sdk_server_main.h>
#include <sdk_server_mgmt.h>
#include <sdk_server_debug.h>
#include <netinet/tcp.h>

#define  MAX_TRY_BIND_WAITS 10

SdkServerMain::SdkServerMain(int port,int * pRunningBits ,int maxclients ) :
    m_Port(port)
    ,m_pRunningBits(pRunningBits)
    ,m_MaxClients(maxclients)
{
	DEBUG_INFO("maxclients %d\n",m_MaxClients);
    m_TryBindWaits = 0;
    m_AccSock = -1;
    m_HasInsertIoAcc = 0;
    m_HasInsertFreqTimer = 0;
    m_pServerMgmt = NULL;
}

SdkServerMain::~SdkServerMain()
{
    this->__Stop();
    this->m_Port = -1;
    this->m_pRunningBits = NULL;
    this->m_MaxClients = 0;
}


void SdkServerMain::__StopFreqTimer()
{
    if (this->m_HasInsertFreqTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvFreqTimer));
    }
    this->m_HasInsertFreqTimer=0;
    return;
}

void SdkServerMain::__StopIoAcc()
{
    if (this->m_HasInsertIoAcc)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvIoAcc));
    }
    this->m_HasInsertIoAcc = 0;
    return ;
}


void SdkServerMain::__Stop()
{
    this->__StopFreqTimer();
    this->__StopIoAcc();

    if (this->m_pServerMgmt)
    {
        delete this->m_pServerMgmt;
    }
    this->m_pServerMgmt = NULL;

    if (this->m_AccSock >= 0)
    {
        close(this->m_AccSock);
    }
    this->m_AccSock = -1;
    this->m_TryBindWaits = 0;
    return;
}

int SdkServerMain::__BindPort()
{
    int ret;
    struct sockaddr_in sinaddr;
    socklen_t saddrlen;
    int reuse;
    if (this->m_Port <= 0 || this->m_Port > 0xffff)
    {
        return -EINVAL;
    }

    this->m_AccSock = socket(AF_INET,SOCK_STREAM,0);
    if (this->m_AccSock == -1)
    {
        return -errno ? -errno : -1;
    }

    reuse = 1;
    ret = setsockopt(this->m_AccSock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (ret < 0)
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

int SdkServerMain::__StartFreqTimer()
{
    SDK_ASSERT(this->m_HasInsertFreqTimer == 0);
    ev_timer_init(&(this->m_EvFreqTimer),SdkServerMain::FreqTimerCallBack,1.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvFreqTimer));
    this->m_HasInsertFreqTimer = 1;
    return 0;
}


int SdkServerMain::__StartIoAcc()
{
    SDK_ASSERT(this->m_HasInsertIoAcc== 0);
    ev_io_init(&(this->m_EvIoAcc),SdkServerMain::IoAccCallBack,this->m_AccSock,EV_READ,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvIoAcc));
    this->m_HasInsertIoAcc= 1;
    return 0;
}

int SdkServerMain::__Start()
{
    int ret;

    this->__Stop();

    if (this->m_MaxClients == 0)
    {
        return -EINVAL;
    }

    SDK_ASSERT(this->m_pServerMgmt == NULL);
    this->m_pServerMgmt = new SdkServerMgmt(this->m_pRunningBits,this->m_MaxClients,this->m_MaxClients,10);
    SDK_ASSERT(this->m_pServerMgmt);

    ret = this->__StartFreqTimer();
    if (ret < 0)
    {
        this->__Stop();
        return ret;
    }


    ret = this->m_pServerMgmt->Start();
    if (ret< 0)
    {
        this->__Stop();
        return ret;
    }

	/*we do not start listen port ,so we try to wait for most 10 seconds ,when we are at the stream running*/
    return 0;
}

int SdkServerMain::RunLoop()
{
    int ret;

    ret = this->__Start();
    if (ret < 0)
    {
        return ret;
    }

    DEBUG_INFO("Run On Listen on %d\n",this->m_Port);
    ev_run(EV_DEFAULT,0);
    this->__Stop();
    return 0;
}


void SdkServerMain::__BreakRunLoop()
{
    ev_break (EV_DEFAULT,EVBREAK_ONE);
}

int SdkServerMain::__ResetFreqTimer()
{
    this->__StopFreqTimer();
    return this->__StartFreqTimer();
}

int SdkServerMain::__FreqTimerImpl()
{
    int ret;
    if (this->m_pRunningBits && *(this->m_pRunningBits) == 0)
    {
        return -1;
    }

    ret = this->__TryBindAndStartAcc();
    if (ret < 0)
    {
        return ret;
    }

    ret = this->m_pServerMgmt->InitDaemon();
    if (ret < 0)
    {
        return ret;
    }
    return this->__ResetFreqTimer();
}

void SdkServerMain::FreqTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkServerMain* pThis = (SdkServerMain*)arg;
    int ret;

    ret = pThis->__FreqTimerImpl();
    if (ret < 0)
    {
        pThis->__BreakRunLoop();
    }
    return ;
}


int SdkServerMain::__SetSocketUnBlock(int sock)
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


int SdkServerMain::__CanInsertSock(void)
{
    unsigned int num=0;

    num += this->m_pServerMgmt->GetClients();

    if (num >= this->m_MaxClients && this->m_MaxClients)
    {
		DEBUG_INFO("++++++++++++++++++++++\nnum[%d] maxclients[%d]\n",num,this->m_MaxClients);
        return 0;
    }

    return 1;
}


int SdkServerMain::__SetKeepAlive(int sock)
{
    int keepalive = 1;
    int keepidle = 30;
    int keepinterval =5;
    int keepcount=3;
    int ret;

    ret = setsockopt(sock,SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive ,sizeof(keepalive));
    if (ret < 0)
    {
        return -errno ? -errno : -1;
    }

    ret = setsockopt(sock,SOL_TCP, TCP_KEEPIDLE, (void*)&keepidle , sizeof(keepidle));
    if (ret < 0)
    {
        return -errno ? -errno : -1;
    }

    ret = setsockopt(sock,SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval , sizeof(keepinterval));
    if (ret < 0)
    {
        return -errno ? -errno : -1;
    }

    ret = setsockopt(sock,SOL_TCP,TCP_KEEPCNT, (void *)&keepcount , sizeof(keepcount));
    if (ret < 0)
    {
        return -errno ? -errno : -1;
    }

    return 0;
}


int SdkServerMain::__IoAccImpl()
{
    struct sockaddr_in sinaddr;
    socklen_t saddrlen;
    int ret,clisock=-1;
    SdkServerClient* pClient=NULL;
	char addrbuf[32];
    saddrlen = sizeof(sinaddr);
    clisock = accept(this->m_AccSock,(struct sockaddr*)&sinaddr,&saddrlen);
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

	DEBUG_INFO("accept sock[%d] from %s:%d\n",clisock,inet_ntop(AF_INET,&(sinaddr.sin_addr),addrbuf, INET_ADDRSTRLEN),ntohs(sinaddr.sin_port));

    ret = this->__CanInsertSock();
    if (ret == 0)
    {
		ERROR_INFO("\n");
        close(clisock);
        return 0;
    }

    ret = this->__SetSocketUnBlock(clisock);
    if (ret < 0)
    {
		ERROR_INFO("\n");
        close(clisock);
        return 0;
    }

    ret = this->__SetKeepAlive(clisock);
    if (ret< 0)
    {
		ERROR_INFO("\n");
        close(clisock);
        return 0;
    }

    pClient = new SdkServerClient(clisock,this->m_pServerMgmt,this->m_pRunningBits);
    clisock = -1;
    ret = pClient->Start();
    if (ret < 0)
    {
		ERROR_INFO("\n");
        delete pClient;
        return 0;
    }

    pClient = NULL;


    return 1;
}


void SdkServerMain::IoAccCallBack(EV_P_ ev_io * w,int revents,void * arg)
{
    SdkServerMain* pThis = (SdkServerMain*)arg;
    int ret;

    ret = pThis->__IoAccImpl();
    if (ret < 0)
    {
        pThis->__BreakRunLoop();
    }
    return ;
}


int SdkServerMain::__TryBindAndStartAcc(void)
{
    int state;
    int ret;
    if (this->m_AccSock >= 0)
    {
        return 0;
    }

    SDK_ASSERT(this->m_pServerMgmt);
    state = this->m_pServerMgmt->QueryStreamStarted();
    if (state == STREAM_STATE_RUNNING || this->m_TryBindWaits >= MAX_TRY_BIND_WAITS)
    {
    	DEBUG_INFO("start bind sock state(%d)\n",state);
        /*now to start the sock*/
        ret = this->__BindPort();
        if (ret < 0)
        {
            return ret;
        }

        ret = this->__StartIoAcc();
        if (ret < 0)
        {
            return ret;
        }
    }
    else
    {
    	DEBUG_INFO("state %d try (%d)\n",state,this->m_TryBindWaits);
        this->m_TryBindWaits ++;
    }

    return 0;
}
