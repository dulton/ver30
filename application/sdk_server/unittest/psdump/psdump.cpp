#include <ipc_media_data_client.h>
#include <gmi_media_ctrl.h>
#include <ipc_media_data_dispatch.h>
#include <ipc_fw_v3.x_resource.h>

#define ERROR_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__FUNCTION__,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)
#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__FUNCTION__,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)


int PSDump(int streamid,char* pWfile,int rport,int lport,int timeout)
{
    IPC_MediaDataClient ipclient;
    int registered = 0;
    int initialized = 0;
    GMI_RESULT gmiret;
    int ret=-1;
    int tries;
    void* pExInfo=NULL;
    void* pMemPtr=NULL;
    int memsize=(1<<21),exsize=1024;
    size_t retsize,exretsize;
    int i;
    int serverport;
    uint32_t type;
    time_t stime,etime,ctime;
    struct timeval tmval;
    FILE* pWFp=NULL;
    uint32_t lastidx,curidx;
    ExtMediaEncInfo* pInfo;
    int writeone=0;

    for(i=0; i<100; i++)
    {
        gmiret = ipclient.Initialize(lport + i,GM_STREAM_APPLICATION_ID);
        if(gmiret == GMI_SUCCESS)
        {
            DEBUG_INFO("Initialize Port (%d)\n",lport + i);
            break;
        }
    }

    if(gmiret != GMI_SUCCESS)
    {
        ret = -errno ? -errno : -1;
        goto out;
    }

    initialized = 1;

    serverport = rport;
    serverport += streamid;
    type = MEDIA_VIDEO_H264;
    gmiret = ipclient.Register(serverport,type,streamid,true,NULL,NULL);
    if(gmiret != GMI_SUCCESS)
    {
        ret = -errno ? -errno : -1;
        goto out;
    }

    pMemPtr = malloc(memsize);
    if(pMemPtr == NULL)
    {
        ret = -errno ? -errno : -1;
        goto out;
    }

    pExInfo = malloc(exsize);
    if(pExInfo == NULL)
    {
        ret = -errno ? -errno : -1;
        goto out;
    }

    stime = time(NULL);
    etime = stime + timeout;
    ctime = stime;

    if(pWfile)
    {
        pWFp=fopen(pWfile,"w+b");
        if(pWFp==NULL)
        {
            ret = -errno ? -errno : -1;
            goto out;
        }
    }

    /*now first to get the time*/
    while(1)
    {
        retsize = memsize;
        exretsize = exsize;
        gmiret = ipclient.Read(pMemPtr,&retsize,&tmval,pExInfo,&exretsize);
        if(gmiret == GMI_SUCCESS)
        {
            pInfo=(ExtMediaEncInfo*)pExInfo;
            if(pInfo->s_FrameType == VIDEO_I_FRAME ||
                    pInfo->s_FrameType == VIDEO_IDR_FRAME)
            {
                if(pWFp)
                {
                    ret =fwrite(pMemPtr,retsize,1,pWFp);
                    if(ret != 1)
                    {
                        ret = -errno ? -errno : -1;
                        goto out;
                    }
                }
                lastidx = pInfo->s_FrameNum;
                writeone = 1;
#if NEW_FREE				
                free(pMemPtr);
                pMemPtr = NULL;
#endif				
                break;
            }
        }
        else
        {
            usleep(30000);
        }
    }

    assert(writeone > 0);
    while(timeout== 0 || ctime < etime)
    {
#if NEW_FREE		
        assert(pMemPtr == NULL);
        pMemPtr = malloc(memsize);
        if(pMemPtr == NULL)
        {
            ret = -errno ? -errno : -1;
            goto out;
        }
#endif		
        retsize = memsize;
        exretsize = exsize;
        gmiret = ipclient.Read(pMemPtr,&retsize,&tmval,pExInfo,&exretsize);
        if(gmiret == GMI_SUCCESS)
        {
            pInfo=(ExtMediaEncInfo*)pExInfo;
            curidx = pInfo->s_FrameNum;
            if(lastidx == 0xffffffff)
            {
                if(curidx != 0)
                {
                    ERROR_INFO("lastidx 0x%x curidx 0x%x\n",lastidx,curidx);
                    writeone = 0;
                }
            }
            else
            {
                if((curidx)!= (lastidx+1))
                {
                    ERROR_INFO("lastidx 0x%x curidx 0x%x\n",lastidx,curidx);
                    writeone = 0;
                }
            }
            if(pInfo->s_FrameType == VIDEO_I_FRAME ||
                    pInfo->s_FrameType == VIDEO_IDR_FRAME)
            {
                writeone = 1;
            }
            lastidx = curidx;
            if(pWFp && writeone)
            {
                ret =fwrite(pMemPtr,retsize,1,pWFp);
                if(ret != 1)
                {
                    ret = -errno ? -errno : -1;
                    goto out;
                }
            }
        }
        else
        {
            usleep(30000);
        }
        ctime = time(NULL);
#if NEW_FREE		
        free(pMemPtr);
        pMemPtr = NULL;
#endif		
    }


    ret = 0;



out:
    if(pWFp)
    {
        fclose(pWFp);
    }
    pWFp = NULL;
    if(pExInfo)
    {
        free(pExInfo);
    }
    pExInfo= NULL;
    if(pMemPtr)
    {
        free(pMemPtr);
    }
    pMemPtr = NULL;

    if(registered)
    {
        tries = 0;
        do
        {
            gmiret = ipclient.Unregister();
            if(gmiret != GMI_SUCCESS && tries < 5)
            {
                usleep(10000);
            }
            tries ++;
        }
        while(gmiret != GMI_SUCCESS && tries < 5);
        registered = 0;
    }

    if(initialized)
    {
        tries = 0;
        do
        {
            gmiret = ipclient.Deinitialize();
            if(gmiret != GMI_SUCCESS && tries < 5)
            {
                usleep(10000);
            }
        }
        while(gmiret != GMI_SUCCESS && tries < 5);
        initialized = 0;
    }

    return ret;
}

static int st_StreamId =0 ;
static int st_Timeout = 0;
static char* st_WFile = NULL;
static int st_RPort = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1;
static int st_LPort = GMI_STREAMING_MEDIA_ONVIF_ENCODE_VIDEO1;

void Usage(int ec,const char* fmt,...)
{
    FILE* fp=stderr;
    va_list ap;
    if(ec == 0)
    {
        fp = stdout;
    }

    if(fmt)
    {
        va_start(ap,fmt);
        vfprintf(fp,fmt,ap);
		fprintf(fp,"\n");
    }

    fprintf(fp,"psdump [OPTIONS]\n");
    fprintf(fp,"\t-s|--streamid streamid         : to specify streamid,default is 0\n");
    fprintf(fp,"\t-t|--timeout timeout           : to specify timeout ,default is 0\n");
    fprintf(fp,"\t-w|--write writefile           : to specify write file ,default is NULL\n");
    fprintf(fp,"\t-r|--rport rport               : to specify remote port,default is %d\n",GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1);
    fprintf(fp,"\t-l|--lport lport               : to specify local port,default is %d\n",GMI_STREAMING_MEDIA_ONVIF_ENCODE_VIDEO1);
    fprintf(fp,"\t-h|--help                      : to display this help information\n");
    exit(ec) ;
}

#define  NEED_ARGS_INT(var,...) \
do\
{\
	if ((i+1)>=argc)\
	{\
		Usage(3,__VA_ARGS__);\
	}\
	i++;\
	var = atoi(argv[i]);	\
}while(0)

#define  NEED_ARGS_STR(var,...) \
	do\
	{\
		if ((i+1)>=argc)\
		{\
			Usage(3,__VA_ARGS__);\
		}\
		i++;\
		var = argv[i];	\
	}while(0)


void ParseParam(int argc,char* argv[])
{
    int i;
    for(i=1; i<argc; i++)
    {
        if(strcmp(argv[i],"-h")==0 ||
                strcmp(argv[i],"--help") == 0)
        {
            Usage(0,NULL);
        }
        else if(strcmp(argv[i],"-s")==0 ||
                strcmp(argv[i],"--streamid") == 0)
        {
            NEED_ARGS_INT(st_StreamId,"%s need one arg",argv[i]);
        }
        else if(strcmp(argv[i],"-t")==0 ||
                strcmp(argv[i],"--timeout") == 0)
        {
            NEED_ARGS_INT(st_Timeout,"%s need one arg",argv[i]);
        }
        else if(strcmp(argv[i],"-w")==0 ||
                strcmp(argv[i],"--write") == 0)
        {
			NEED_ARGS_STR(st_WFile,"%s need one arg",argv[i]);
        }
        else if(strcmp(argv[i],"-r")==0 ||
                strcmp(argv[i],"--rport") == 0)
        {
            NEED_ARGS_INT(st_RPort,"%s need one arg",argv[i]);
        }
        else if(strcmp(argv[i],"-l")==0 ||
                strcmp(argv[i],"--lport") == 0)
        {
            NEED_ARGS_INT(st_LPort,"%s need one arg",argv[i]);
        }
        else 
        {
			Usage(3,"Unkown %s args",argv[i]);
        }
    }

	return;
}


int main(int argc,char* argv[])
{

    int ret;
	ParseParam(argc,argv);
    ret = PSDump(st_StreamId,st_WFile,st_RPort,st_LPort,st_Timeout);
    return ret;
}
