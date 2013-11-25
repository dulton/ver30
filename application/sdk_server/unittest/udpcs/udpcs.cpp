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


#define LAST_ERROR_CODE() (-errno ? -errno : -1)

static int st_RunLoop = 1;

void SigHandler(int signo)
{
    if(signo == SIGINT ||
            signo == SIGTERM)
    {
        st_RunLoop = 0;
    }
}

int HandleClient(int sock,int rport,char* msg)
{
    int msglen = strlen(msg)+1;
    int ret;
    fd_set rset;
    struct sockaddr_in saddr;
    socklen_t socklen=0;
    int times=0;
    struct timeval tmval;
    int buflen=2000;
    unsigned char buffer[buflen];
    char addrbuf[INET_ADDRSTRLEN];

    while(st_RunLoop)
    {
        memset(&saddr,0,sizeof(saddr));
        saddr.sin_family = AF_INET;
        saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        saddr.sin_port = htons(rport);

        socklen = sizeof(saddr);
        ret = sendto(sock,msg,msglen,MSG_DONTWAIT,(struct sockaddr*)&saddr,socklen);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("could not send[%d] msg (%s) error(%d)\n",times,msg,ret);
            goto out;
        }

        FD_ZERO(&rset);
        FD_SET(sock,&rset);
        tmval.tv_sec = 2;
        tmval.tv_usec = 0;
        errno = 0;
        ret = select(sock+1,&rset,NULL,NULL,&tmval);
        if(ret <=0)
        {
            ERROR_INFO("Get errno %d\n",errno);
            ret = LAST_ERROR_CODE();
            ERROR_INFO("could make rset ok (%d)\n",ret);
            goto out;
        }

        if(!FD_ISSET(sock,&rset))
        {
            ret = -EFAULT;
            ERROR_INFO("not set rset\n");
            goto out;
        }

        socklen = sizeof(saddr);
        memset(&saddr,0,socklen);
        ret = recvfrom(sock,buffer,buflen,MSG_DONTWAIT,(struct sockaddr*)&saddr,&socklen);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("could not receive %d\n",ret);
            goto out;
        }

        DEBUG_INFO("[%d]receive from %s:%d (%s)\n",times,inet_ntop(AF_INET,&(saddr.sin_addr),addrbuf, INET_ADDRSTRLEN),ntohs(saddr.sin_port),buffer);
        times ++;
        sleep(1);
    }

    ret = 0;
out:
    return ret;
}


int BindUdp(int port)
{
    int ret;
    int sock=-1;
    struct sockaddr_in saddr;

    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0)
    {
        ret = LAST_ERROR_CODE();
		ERROR_INFO("socket error (%d)\n",ret);
        goto fail;
    }

    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    saddr.sin_port = htons(port);

    ret = bind(sock,(struct sockaddr*)&saddr,sizeof(saddr));
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
		ERROR_INFO("bind error (%d)\n",ret);
        goto fail;
    }


    return sock;
fail:
    if(sock >= 0)
    {
        close(sock);
    }
    sock = -1;
    return ret;
}

int HandleServer(int sock)
{
    fd_set rset;
    int ret;
    struct timeval tmval;
    int buflen=2000;
    unsigned char buffer[buflen];
    struct sockaddr_in saddr;
    socklen_t socklen;
    char addrbuf[INET_ADDRSTRLEN];
    int rlen;


    while(st_RunLoop)
    {
        FD_ZERO(&rset);
        FD_SET(sock,&rset);

        tmval.tv_sec = 2;
        tmval.tv_usec = 0;

        ret = select(sock+1,&rset,NULL,NULL,&tmval);
        if(ret <=0)
        {
            continue;
        }

        if(FD_ISSET(sock,&rset))
        {
            /*now receive */
            socklen = sizeof(saddr);
            ret = recvfrom(sock,buffer,buflen, MSG_DONTWAIT,(struct sockaddr*)&saddr,&socklen);
            if(ret < 0)
            {
                continue;
            }

            rlen = ret;
            DEBUG_INFO("receive from %s:%d\n",inet_ntop(AF_INET,&(saddr.sin_addr),addrbuf, INET_ADDRSTRLEN),ntohs(saddr.sin_port));
            ret = sendto(sock,buffer,rlen,MSG_DONTWAIT,(struct sockaddr*)&saddr,socklen);
            if(ret < 0)
            {
                continue;
            }
        }
    }

    return 0;
}



int main(int argc,char* argv[])
{
    int ret;
    int lport=0,rport=0;
    char* msg=NULL;
    int sock = -1;

    if(argc == 1)
    {
        fprintf(stderr,"%s OPTIONS\n",argv[1]);
        fprintf(stderr,"\tlport rport msg for client\n");
        fprintf(stderr,"\tlport for server\n");
        exit(3);
    }


    if(argc >= 4)
    {
        signal(SIGINT,SigHandler);
        signal(SIGTERM,SigHandler);
        signal(SIGPIPE,SIG_IGN);
        while(st_RunLoop)
        {
            lport =atoi(argv[1]);
            rport = atoi(argv[2]);
            msg = argv[3];

			DEBUG_INFO("\n");
            sock = BindUdp(lport);
            if(sock < 0)
            {
                ret = sock;
				DEBUG_INFO("bind error %d\n",ret);
                goto out;
            }

            ret = HandleClient(sock,rport,msg);
			close(sock);
			sock = -1;
			sleep(1);
			DEBUG_INFO("\n");
        }
    }
    else
    {
        lport = atoi(argv[1]);
        sock = BindUdp(lport);
        if(sock < 0)
        {
            ret = sock;
            goto out;
        }

        ret = daemon(0,0);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            goto out;
        }

        signal(SIGINT,SigHandler);
        signal(SIGTERM,SigHandler);
        signal(SIGPIPE,SIG_IGN);

        ret = HandleServer(sock);

    }

out:
    if(sock >= 0)
    {
        close(sock);
    }
    sock = -1;
    return ret;
}
