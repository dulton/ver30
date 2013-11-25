

#include <sdk_client_unix_sock.h>
#include <unistd.h>
#include <libev/ev.h>
#include "clientunixdemo.h"

static int st_RunLoop=1;

void SigHandler(int signo)
{
    if(signo == SIGINT ||
            signo == SIGTERM)
    {
        st_RunLoop = 0;
    }
}

int main(int argc,char* argv[])
{
    int ret;
    ClientUnixDemo* pUnixDemo=NULL;

    if(argc < 3)
    {
        fprintf(stderr,"%s bindunix connectunix\n",argv[0]);
        exit(3);
    }

    signal(SIGINT,SigHandler);
    signal(SIGTERM,SigHandler);

    pUnixDemo = new ClientUnixDemo(&st_RunLoop);
    ret = pUnixDemo->Start(argv[1],argv[2],NULL);
    delete pUnixDemo;
    pUnixDemo = NULL;
    return ret;
}

