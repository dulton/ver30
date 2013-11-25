
#include "clientunixdemo.h"
#include <sdk_server_debug.h>
#include <sys/un.h>

ClientUnixDemo::ClientUnixDemo(int * pRunningBits) : m_pRunningBits(pRunningBits)
{
    m_Sock = -1;
    m_pSock = NULL;
    m_SockConnected = 0;
    m_InsertSockConnectIo = 0;
    m_InsertSockConnectTimer = 0;
}

void ClientUnixDemo::__CloseSock()
{
    SDK_ASSERT(!(this->m_Sock >= 0 && this->m_pSock));
    this->__StopSockConnectIo();
    this->__StopSockConnectTimer();
    if(this->m_Sock >= 0)
    {
        close(this->m_Sock);
    }
    this->m_Sock = -1;

    if(this->m_pSock)
    {
        delete this->m_pSock;
    }
    this->m_pSock = NULL;
    this->m_SockConnected = 0;
    return ;
}

void ClientUnixDemo::Stop()
{
    this->__CloseSock();

    return ;
}

ClientUnixDemo::~ClientUnixDemo()
{
    this->Stop();
}


int ClientUnixDemo::__TryConnect(const char * pBindUnix,const char * pConnectUnix)
{
    struct sockaddr_un sunaddr;
    int ret;
    int flag;

    SDK_ASSERT(this->m_Sock < 0 && this->m_pSock == NULL);
    SDK_ASSERT(this->m_SockConnected == 0);

    this->m_Sock = socket(AF_UNIX,SOCK_STREAM,0);
    if(this->m_Sock < 0)
    {
        ret = -errno ? -errno : -1;
        this->__CloseSock();
        return ret;
    }

    memset(&sunaddr,0,sizeof(sunaddr));
    sunaddr.sun_family = AF_UNIX;
    strncpy(sunaddr.sun_path,pBindUnix,sizeof(sunaddr.sun_path));

    ret = bind(this->m_Sock,(struct sockaddr*)&sunaddr,sizeof(sunaddr));
    if(ret < 0)
    {
        ret = -errno ? -errno : -1;
        this->__CloseSock();
        ERROR_INFO("could not bind %s socket error(%d)\n",pBindUnix,ret);
        return ret;
    }

    /*now set for flags*/
    errno = 0;
    flag = fcntl(this->m_Sock,F_GETFL);
    if(errno != 0)
    {
        ret = -errno;
        this->__CloseSock();
        ERROR_INFO("could not get sockaddr flag error(%d)\n",ret);
        return ret;
    }

    ret = fcntl(this->m_Sock,F_SETFL,flag | O_NONBLOCK);
    if(ret < 0)
    {
        ret = -errno ? -errno : -1;
        this->__CloseSock();
        ERROR_INFO("could not set nonblock error(%d)\n",ret);
        return ret;
    }

    /*now to connect*/
    memset(&sunaddr,0,sizeof(sunaddr));
    sunaddr.sun_family = AF_UNIX;
    strncpy(sunaddr.sun_path,pConnectUnix,sizeof(sunaddr.sun_path));
    errno = 0;
    ret = connect(this->m_Sock,(struct sockaddr*)&sunaddr,sizeof(sunaddr));
    if(ret >= 0)
    {
        /*this connect ok, so we should connect*/
        ERROR_INFO("connect %s succ\n",pConnectUnix);
        this->m_pSock = new SdkClientUnixSock(this->m_Sock);
        this->m_Sock = -1;
        this->m_SockConnected = 1;
    }
    else
    {
        ERROR_INFO("socket connect %s errno %d\n",pConnectUnix,errno);
        if(errno == EINPROGRESS)
        {
            ret = this->__StartSockConnectIo();
            if(ret < 0)
            {
                ret = -errno ? -errno : -1;
                this->__CloseSock();
                return ret;
            }

            ret = this->__StartSockConnectTimer();
            if(ret < 0)
            {
                ret = -errno ? -errno : -1;
                this->__CloseSock();
                return ret;
            }
        }
        else
        {
            ret = -errno ? -errno : -1;
            this->__CloseSock();
			return ret;

        }
    }

    return 0;
}

int ClientUnixDemo::__ConnectIoImpl()
{
    int ret;
    int error=0;
    socklen_t errlen=0;
    /*now we should get the socket error*/
    DEBUG_INFO("sock %d pSock 0x%p\n",this->m_Sock,this->m_pSock);
    SDK_ASSERT(this->m_Sock >= 0 && this->m_pSock == NULL);
    errlen = sizeof(error);
    ret = getsockopt(this->m_Sock,SOL_SOCKET,SO_ERROR,&error,&errlen);
    if(ret < 0)
    {
        ret = -errno ? -errno : -1;
        ERROR_INFO("get sock opt error(%d)\n",ret);
        return ret;
    }

    if(error)
    {
        ret = -EFAULT;
        ERROR_INFO("could not connect error code(%d)\n",error);
        return ret;
    }

    this->m_SockConnected = 1;
    this->m_pSock = new SdkClientUnixSock(this->m_Sock);
    this->m_Sock = -1;
    DEBUG_INFO("connect succ\n");
    return 0;
}

void ClientUnixDemo::ConnectIoCallBack(EV_P_ ev_io * w,int revents,void * arg)
{
    ClientUnixDemo* pThis=(ClientUnixDemo*)arg;
    int ret;

    ret = pThis->__ConnectIoImpl();
    if(ret < 0)
    {
        pThis->__BreakOut();
    }
    pThis->__StopSockConnectIo();
    pThis->__StopSockConnectTimer();
    return;
}

int ClientUnixDemo::__ConnectTimerImpl()
{
    ERROR_INFO("connect timeout\n");
    return -ETIMEDOUT;
}

void ClientUnixDemo::ConnectTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    ClientUnixDemo* pThis=(ClientUnixDemo*)arg;
    int ret;

    ret = pThis->__ConnectTimerImpl();
    if(ret < 0)
    {
        pThis->__BreakOut();
    }
    return;
}


void ClientUnixDemo::__StopSockConnectIo()
{
    if(this->m_InsertSockConnectIo)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvSockConnectIo));
    }
    this->m_InsertSockConnectIo = 0;
    return ;
}

int ClientUnixDemo::__StartSockConnectIo()
{
    SDK_ASSERT(this->m_InsertSockConnectIo == 0);
    ev_io_init(&(this->m_EvSockConnectIo),ClientUnixDemo::ConnectIoCallBack,this->Socket(),EV_WRITE,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvSockConnectIo));
    this->m_InsertSockConnectIo = 1;
    return 0;
}


void ClientUnixDemo::__StopSockConnectTimer()
{
    if(this->m_InsertSockConnectTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvSockConnectTimer));
    }
    this->m_InsertSockConnectTimer = 0;
    return ;
}

int ClientUnixDemo::__StartSockConnectTimer()
{
    SDK_ASSERT(this->m_InsertSockConnectTimer == 0);
    ev_timer_init(&(this->m_EvSockConnectTimer),ClientUnixDemo::ConnectTimerCallBack,3.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvSockConnectTimer));
    this->m_InsertSockConnectTimer = 1;
    return 0;
}

int ClientUnixDemo::Start(const char * pBindUnix,const char * pConnectUnix,const char * pTransFile)
{
    int ret;

    this->Stop();
    ret = this->__TryConnect(pBindUnix,pConnectUnix);
    if(ret < 0)
    {
        this->Stop();
        return ret;
    }

    /*now to set the timer*/
    ev_run(EV_DEFAULT,0);
    this->Stop();
    return 0;
}

void ClientUnixDemo::__BreakOut()
{
    BACK_TRACE();
    ev_break(EV_DEFAULT,EVBREAK_ONE);
    return ;
}

int ClientUnixDemo::Socket()
{
    if(this->m_pSock)
    {
        return this->m_pSock->Socket();
    }

    return this->m_Sock;
}

