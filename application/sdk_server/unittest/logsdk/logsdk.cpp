
#include <sdk_server_debug.h>

static int st_RunLoop=1;

void SigHandler(int signo)
{
    st_RunLoop = 0;
	fprintf(stderr,"catch signo %d\n",signo);
    return;
}

typedef struct
{
    int m_ThreadNum;
    int m_ThreadExited;
} thrd_info_t;

void* ThreadRunning(void* arg)
{

    thrd_info_t* pThrInfo= (thrd_info_t*)arg;
    int randtime;
    unsigned int rseed ;
    int times = 0;

    while(st_RunLoop)
    {
        DEBUG_INFO("Run Thread %d (time %d)\n",pThrInfo->m_ThreadNum,times);
        randtime =((rand_r(&rseed) * 1000)%500000);
		randtime = abs(randtime);
        usleep(randtime);
        DEBUG_BUFFER_FMT(pThrInfo,sizeof(*pThrInfo),"Thread %d",pThrInfo->m_ThreadNum);
        BACK_TRACE_FMT("[%d]call backtrace at (%d)",pThrInfo->m_ThreadNum,times);
        times ++;
    }

    pThrInfo->m_ThreadExited = 1;
    return (void*)0;
}

int main(int argc,char* argv[])
{
    int ret;
    GMI_Thread** pGmThread=NULL;
    thrd_info_t* pThrInfo=NULL;
    int i;
    int numthrds=0;
    GMI_RESULT gmiret;
    sighandler_t sigret;

    if(argc<2)
    {
        fprintf(stderr,"%s numthreads\n",argv[0]);
        return -EINVAL;
    }


    DEBUG_INFO("should not see in the file\n");

    ret = InitializeSdkLog();
    if(ret < 0)
    {
        fprintf(stderr,"can not initialize\n");
        return ret;
    }

    numthrds = atoi(argv[1]);
    if(numthrds <= 0)
    {
        fprintf(stderr,"numthreads must > 0 \n");
        ret = -EINVAL;
		goto out;
    }

    sigret = signal(SIGINT,SigHandler);
    if(sigret == SIG_ERR)
    {
        ret = -errno ? -errno : -1;
        ERROR_INFO("could not set sigint handler\n");
        goto out;
    }


    pGmThread =(GMI_Thread**) calloc(sizeof(*pGmThread),numthrds);
    if(pGmThread == NULL)
    {
        ret = -errno ? -errno : -1;
        ERROR_INFO("gmthread allocate\n");
        goto out;
    }

    pThrInfo =(thrd_info_t*) calloc(sizeof(*pThrInfo),numthrds);
    if(pThrInfo == NULL)
    {
        ret=  -errno ? -errno : -1;
        goto out;
    }



    for(i=0; i<numthrds; i++)
    {
        /*now first to make the thread exited ok*/
        pThrInfo[i].m_ThreadExited = 1;
    }

    /*now to startup the thread*/
    for(i=0; i<numthrds; i++)
    {
        pThrInfo[i].m_ThreadNum = i;
        pGmThread[i] = new GMI_Thread();
        pThrInfo[i].m_ThreadExited = 0;
        gmiret = pGmThread[i]->Create(NULL,0,ThreadRunning,&(pThrInfo[i]));
        if(gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
            pThrInfo[i].m_ThreadExited = 1;
            ERROR_INFO("could not create[%d] thread error(0x%08lx) ret(%d)\n",i,gmiret,ret);
            goto out;
        }

        gmiret = pGmThread[i]->Start();
        if(gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
            pThrInfo[i].m_ThreadExited = 1;
            ERROR_INFO("could not start[%d] thread error(0x%08lx) ret(%d)\n",i,gmiret,ret);
            goto out;
        }
    }

    while(st_RunLoop)
    {
        DEBUG_INFO("main thread\n");
        usleep(1000000);
    }

    ret = 0;

out:
    st_RunLoop = 0;
    if(pThrInfo)
    {

        for(i=0; i<numthrds; i++)
        {
            if(pGmThread[i])
            {
                while(pThrInfo[i].m_ThreadExited == 0)
                {
					fprintf(stderr,"wait thread %d\n",i);
                    usleep(10000);
                }
                pGmThread[i]->Destroy();
                delete pGmThread[i];
                pGmThread[i] = NULL;
            }
        }
        free(pThrInfo);
    }
    pThrInfo = NULL;

    if(pGmThread)
    {
        free(pGmThread);
    }
    pGmThread = NULL;

    DeInitializeSdkLog();
    return ret;
}


