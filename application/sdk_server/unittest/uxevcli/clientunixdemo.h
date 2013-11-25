
#ifndef __CLIENT_UNIX_DEMO_H__
#define __CLIENT_UNIX_DEMO_H__


#include <sdk_client_unix_sock.h>
#include <libev/ev.h>

class ClientUnixDemo
{
public:
    ClientUnixDemo(int* pRunningBits);
    ~ClientUnixDemo();
    int Start(const char* pBindUnix,const char* pConnectUnix,const char* pTransFile);
    void Stop();
    int Socket();
private:
    int __StartSockConnectIo();
    void __StopSockConnectIo();
    int __StartSockConnectTimer();
    void __StopSockConnectTimer();
    int __TryConnect(const char* pBindUnix,const char* pConnectUnix);
    void __CloseSock();

    static void ConnectIoCallBack(EV_P_ ev_io *w, int revents,void* arg);
    int __ConnectIoImpl();
    static void ConnectTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);
    int __ConnectTimerImpl();
	void __BreakOut();

#if 0
    int __StartSockReadIo();
    void __StopSockReadIo();
    int __StartSockReadTimer();
    void __StopSockReadTimer();
    int __StartSockWriteIo();
    void __StopSockWriteIo();
    int __StartSockWriteTimer();
    void __StopSockWriteTimer();
#endif
private:
    int *m_pRunningBits;
    int m_Sock;
    SdkClientUnixSock *m_pSock;
    int m_SockConnected;
    int m_InsertSockConnectIo;
    ev_io m_EvSockConnectIo;
    int m_InsertSockConnectTimer;
    ev_timer m_EvSockConnectTimer;
#if 0
    int m_InsertSockReadIo;
    ev_io m_EvSockReadIo;
    int m_InsertReadTimer;
    ev_timer m_EvSockReadTimer;
    int m_InsertSockWriteIo;
    ev_io m_EvSockWriteIo;
    int m_InsertSockWriteTimer;
    ev_timer m_EvSockWriteTimer;
    int m_Fd;
#endif


};

#endif /*__CLIENT_UNIX_DEMO_H__*/

