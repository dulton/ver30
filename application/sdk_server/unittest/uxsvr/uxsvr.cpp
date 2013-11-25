
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/un.h>
#include <stddef.h>


#define ERROR_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__FUNCTION__,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)
#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__FUNCTION__,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)


static int st_RunLoop=1;

void SigHandler(int signo)
{
    if(signo == SIGINT ||
            signo == SIGTERM)
    {
        st_RunLoop = 0;
    }
}

int SetNonBlock(int sock)
{
    int ret,flags;
    errno = 0;
    flags = fcntl(sock,F_GETFL);
    if(errno)
    {
        ret = -errno;
        ERROR_INFO("could not get flag (%d)\n",ret);
        return ret;
    }

    ret = fcntl(sock,F_SETFL,flags | O_NONBLOCK);
    if(ret < 0)
    {
        ret = -errno ? -errno : -1;
        ERROR_INFO("could not set flag (%d)\n",ret);
        return ret;
    }
    return 0;
}

int BindSock(const char* pBindUnix)
{
    int ret;
    int sock=-1;
    struct sockaddr_un sunaddr;
	socklen_t socklen;

    sock = socket(AF_UNIX,SOCK_STREAM,0);
    if(sock < 0)
    {
        ret = -errno ? -errno : -1;
        ERROR_INFO("can not socket(%d)\n",ret);
        goto fail;
    }

    memset(&sunaddr,0,sizeof(sunaddr));
    sunaddr.sun_family = AF_UNIX;
    strncpy(sunaddr.sun_path,pBindUnix,sizeof(sunaddr.sun_path));

	socklen = sizeof(sunaddr);

    ret = bind(sock,(struct sockaddr*)&sunaddr,socklen);
    if(ret < 0)
    {
        ret = -errno ? -errno : -1;
        ERROR_INFO("bind %s error(%d)\n",pBindUnix,ret);
        goto fail;
    }

	ret = listen(sock,5);
	if (ret < 0)
	{
		ret = -errno ? -errno : -1;
		ERROR_INFO("listen %s error(%d)\n",pBindUnix,ret);
		goto fail;
	}
	
	DEBUG_INFO("Bind %s\n",pBindUnix);
#if 1
    ret = SetNonBlock(sock);
    if(ret < 0)
    {
        goto fail;
    }
#endif

    return sock;
fail:
    if(sock >= 0)
    {
        close(sock);
    }
    sock = -1;
    return ret;
}

int SelectAccept(int sock,int timeout)
{
    struct timeval tmout;
    fd_set rset;
    int ret;
    int accsock=-1;
    struct sockaddr_un sunaddr;
    socklen_t sunlen=0;

    tmout.tv_sec = timeout;
    tmout.tv_usec = 0;

    while(st_RunLoop)
    {
        FD_ZERO(&rset);
        FD_SET(sock,&rset);
		ret = tmout.tv_sec;

        ret = select(sock+1,&rset,NULL,NULL,NULL);
        if(ret < 0)
        {
            ret = -errno ? -errno : -1;
            if(errno == EINTR ||errno == EWOULDBLOCK || errno == EAGAIN)
            {
                continue;
            }
            return ret;
        }
        else if(ret == 0)
        {
            return -ETIMEDOUT;
        }

        if(!FD_ISSET(sock,&rset))
        {
            continue;
        }

        sunlen = sizeof(sunaddr);
		memset(&sunaddr,0,sizeof(sunaddr));
		DEBUG_INFO("sunlen %d\n",sunlen);
        accsock = accept(sock,(struct sockaddr*)&sunaddr,&sunlen);
        if(accsock < 0)
        {
            ret = -errno ? -errno : -1;
            ERROR_INFO("[%d]accept error(%d) sunlen(%d)\n",sock,ret,sunlen);
            goto fail;
        }

        break;

    }

    if(accsock < 0)
    {
        return -EINTR;
    }

    ret = SetNonBlock(accsock);
    if(ret < 0)
    {
        goto fail;
    }

    return accsock;

fail:
    if(accsock >= 0)
    {
        close(accsock);
    }
    accsock = -1;
    return ret;
}

int main(int argc,char* argv[])
{
    int sock=-1;
    int ret;
    int accsock=-1;

    if(argc < 1)
    {
        fprintf(stderr,"%s bindunix\n",argv[0]);
        return -3;
    }

    sock = BindSock(argv[1]);
    if(sock < 0)
    {
        ret = sock;
        goto out;
    }

    while(st_RunLoop)
    {
        accsock = SelectAccept(sock,0);
        if(accsock < 0)
        {
            continue;
        }

        close(accsock);
        accsock = -1;
    }

out:
    if(accsock >= 0)
    {
        close(accsock);
    }
    accsock = -1;
    if(sock >= 0)
    {
        close(sock);
    }
    sock = -1;
    return ret;
}

