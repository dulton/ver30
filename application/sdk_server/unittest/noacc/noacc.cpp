#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int main(int argc,char* argv[])
{
    int ret;
    struct sockaddr_in saddr;
    int sock =-1;
    int port=0;
    socklen_t socklen=0;
    int count =0;

    if(argc < 2)
    {
        fprintf(stderr,"%s port\n",argv[0]);
        return -3;
    }
    port = atoi(argv[1]);

    sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0)
    {
        ret = errno ? -errno : -1;
        goto out;
    }

    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr("0.0.0.0");

    socklen = sizeof(saddr);
    ret = bind(sock,(struct sockaddr*)&saddr,socklen);
    if(ret < 0)
    {
        ret = errno ? -errno : -1;
        fprintf(stderr,"can not bind %d port error(%d)\n",port,ret);
        goto out;
    }

    ret = listen(sock,5);
    if(ret < 0)
    {
        ret = errno ? -errno : -1;
        fprintf(stderr,"listen error(%d)\n",ret);
        goto out;
    }

    fprintf(stdout,"Listen (%d)\n",port);
    while(1)
    {
        sleep(1);
        count ++;
        fprintf(stdout,"listen (%d) count(%d)\n",port,count);
    }

	ret = 0;

out:
    if(sock >= 0)
    {
        close(sock);
    }
    sock = -1;
    return ret;
}
