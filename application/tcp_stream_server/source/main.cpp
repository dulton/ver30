
#include <ev.h>
#include <tcp_stream_acc.h>
#include <execinfo.h>


#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%d\t",__FILE__,__LINE__);fprintf(stderr,__VA_ARGS__);}while(0)

static int st_RunLoop=1;

void Usage(int exitcode,const char * fmt,...)
{
    FILE* fp=stderr;
    if (exitcode == 0)
    {
        fp = stdout;
    }

    if (fmt)
    {
        va_list ap;
        va_start(ap,fmt);
        vfprintf(fp,fmt,ap);
        fprintf(fp,"\n");
    }

    fprintf(fp,"tcp_stream_acc port maxpacks maxclients\n");
    exit(exitcode);
}

static void BackTrace_Symbol()
{
    void *array[30];
    size_t size;
    int i;

    char **strings;
    size = backtrace(array,30);
    strings = backtrace_symbols(array,size);
    for (i=0; i<(int)size; i++)
    {
        fprintf(stderr,"%s\n",strings[i]);
    }
    return ;
}


void SigStop(int signo)
{
    if(signo == SIGINT ||
            signo == SIGTERM)
    {
        DEBUG_INFO("signo %d\n",signo);
        st_RunLoop = 0;
    }
    else if (signo == SIGSEGV)
    {
        DEBUG_INFO("segment fault\n");
        BackTrace_Symbol();
        exit(3);
    }
}

int main(int argc,char* argv[])
{
    int port = 30002;
    int maxpacks = 10;
    int maxclients = 5;
    TcpStreamAcc* pAcc=NULL;
    int ret;
    sighandler_t sigret;

    if (argc > 1)
    {
        if (strcmp(argv[1],"-h")==0 ||
                strcmp(argv[1],"--help") ==0)
        {
            Usage(0,NULL);
        }
        port = atoi(argv[1]);
    }

    if (argc > 2)
    {
        maxpacks = atoi(argv[2]);
    }

    if (argc > 3)
    {
        maxclients = atoi(argv[3]);
    }


    sigret = signal(SIGINT,SigStop);
    if (sigret == SIG_ERR)
    {
        ret = -errno ? -errno : -1;
        goto out;
    }

    sigret = signal(SIGTERM,SigStop);
    if (sigret == SIG_ERR)
    {
        ret = -errno ? -errno : -1;
        goto out;
    }

    sigret = signal(SIGSEGV,SigStop);
    if (sigret==SIG_ERR)
    {
        ret = -errno ? -errno : -1;
        goto out;
    }

    sigret = signal(SIGPIPE,SIG_IGN);
    if (sigret==SIG_ERR)
    {
        ret = -errno ? -errno : -1;
        goto out;
    }

    /*now to make */
    pAcc = new TcpStreamAcc(port,&st_RunLoop,maxclients,maxpacks);

    ret = pAcc->RunLoop();

    /*we */
    DEBUG_INFO("\n");

out:
    if (pAcc)
    {
        delete pAcc;
    }
    pAcc = NULL;
    return ret;
}
