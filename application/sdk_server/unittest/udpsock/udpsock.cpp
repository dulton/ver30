#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#define ERROR_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__ASSERT_FUNCTION,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)
#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__ASSERT_FUNCTION,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)


static int st_RunLoop = 1;

#define GETERRNO() ( errno ? errno : 1)
#define SETERRNO(ret) (errno = ret)

int BindPort(int port)
{
    int sock = -1;
    int ret;
    struct sockaddr_in saddr;

    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("socket error(%d)\n",ret);
        goto fail;
    }

    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    saddr.sin_port = htons(port);

    ret = bind(sock,(struct sockaddr*)&saddr,sizeof(saddr));
    if(ret < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("Bind Port(%d) error(%d)\n",port,ret);
        goto fail;
    }


    return sock;
fail:
    assert(ret > 0);
    if(sock >= 0)
    {
        close(sock);
    }
    sock = -1;
    SETERRNO(ret);
    return -1;
}



#define MAX_CLIENT_SOCKS   3

int ServeClient(int sock)
{
    int ret;
    char RcvBuf[1024];
    int rcvlen=0;
    struct sockaddr_in saddr;
    socklen_t saddrlen;

    saddrlen = sizeof(saddr);
    ret = recvfrom(sock,RcvBuf,sizeof(RcvBuf),MSG_DONTWAIT,(struct sockaddr*)&saddr,&saddrlen);
    if(ret <= 0)
    {
        ret = GETERRNO();
        goto fail;
    }

    rcvlen = ret;
    ret = sendto(sock,RcvBuf,rcvlen,MSG_DONTWAIT,(struct sockaddr*)&saddr,saddrlen);
    if(ret < 0)
    {
        ret = GETERRNO();
        goto fail;
    }

    return 0;

fail:

    SETERRNO(ret);
    return -1;
}

int ServerHandle(int port)
{
    int accsock = -1;
    int ret;
    int maxfd;
    fd_set rset;


    accsock = BindPort(port);
    if(accsock < 0)
    {
        ret = GETERRNO();
        goto out;
    }

    while(st_RunLoop)
    {
        FD_ZERO(&rset);
        FD_SET(accsock,&rset);
        maxfd = accsock;

        ret = select(maxfd + 1,&rset,NULL,NULL,NULL);
        if(ret < 0)
        {
            ret = GETERRNO();
            if(ret == EINTR || ret == EWOULDBLOCK || ret == EAGAIN)
            {
                continue;
            }
            ERROR_INFO("Select Error(%d)\n",ret);
            goto out;
        }
        else if(ret == 0)
        {
            continue;
        }

        if(FD_ISSET(accsock,&rset))
        {
            ServeClient(accsock);
        }
    }

    ret = 0;

out:
    if(accsock >= 0)
    {
        close(accsock);
    }
    accsock = -1;
    SETERRNO(ret);
    return ret;
}

int ClientHandle(const char* ip,int port)
{
    int sock=-1;
    int ret;
    char RcvBuf[1024];
    int rcvlen=0;
    struct sockaddr_in saddr;
    socklen_t saddrlen;
    struct timespec stime,etime;
	unsigned long long sll,ell;
	unsigned long long elapse;

    sock = BindPort(0);
    if(sock < 0)
    {
        ret = GETERRNO();
        goto out;
    }

    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family =AF_INET;
    saddr.sin_addr.s_addr = inet_addr(ip);
    saddr.sin_port = htons(port);

    strcpy(RcvBuf,"Hello World");
    rcvlen = strlen(RcvBuf)+1;

    ret = clock_gettime(CLOCK_MONOTONIC,&stime);
    if(ret < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("get start time error(%d)\n",ret);
        goto out;
    }

    saddrlen = sizeof(saddr);
    ret = sendto(sock,RcvBuf,rcvlen,MSG_DONTWAIT,(struct sockaddr*)&saddr,saddrlen);
    if(ret < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("send buffer error(%d)\n",ret);
        goto out;
    }

    saddrlen = sizeof(saddr);
    ret= recvfrom(sock,RcvBuf,sizeof(RcvBuf),MSG_DONTWAIT,(struct sockaddr*)&saddr,&saddrlen);
    if(ret < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("receive buffer error(%d)\n",ret);
        goto out;
    }


    ret = clock_gettime(CLOCK_MONOTONIC,&etime);
    if(ret < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("get end time error(%d)\n",ret);
        goto out;
    }

	sll = stime.tv_sec * 1000 * 1000 * 1000;
	sll += stime.tv_nsec;

	ell = etime.tv_sec * 1000 * 1000 * 1000;
	ell += etime.tv_nsec;
	elapse = ((ell - sll )*1000) /(1000 * 1000 * 1000);

    fprintf(stdout,"start (%ld:%ld) end (%ld:%ld) (%lld)mills\n",stime.tv_sec,stime.tv_nsec,etime.tv_sec,etime.tv_nsec,elapse);

    ret =0 ;

out:
    if(sock >= 0)
    {
        close(sock);
    }
    sock = -1;
    SETERRNO(ret);
    return -1;
}

void SigHandler(int signo)
{
    if(signo == SIGINT || signo == SIGTERM)
    {
        st_RunLoop = 0;
    }
	return ;
}

int main(int argc,char* argv[])
{
    int ret;
    int port;

    signal(SIGINT,SigHandler);
	signal(SIGTERM,SigHandler);

    if(argc < 2)
    {
        fprintf(stderr,"%s port for server\n",argv[0]);
        fprintf(stderr,"%s host port for client\n",argv[1]);
    }

    if(argc == 2)
    {
        port = atoi(argv[1]);
        ret = ServerHandle(port);
    }
    else
    {
        port = atoi(argv[2]);
        ret = ClientHandle(argv[1],port);
    }

    return ret;
}
