
#include <ipc_media_data_dispatch/ipc_media_data_client.h>
#include <ipc_fw_v3.x_resource.h>
#include <ipc_media_data_dispatch/ipc_media_data_dispatch.h>
#include <media/gmi_media_ctrl.h>

#define ERROR_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__FUNCTION__,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)
#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__FUNCTION__,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)


int GetClientStream(IPC_MediaDataClient* pClient,int streamid,int *pRunningBits)
{
    void* pBuffer=NULL;
    int bufsize=(1<<21);
    uint32_t buflen = 0;
    int serverport = GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1;
    int clientport = GMI_STREAMING_MEDIA_SDK_ENCODE_VIDEO1;
    unsigned int i;
    GMI_RESULT gmiret;
    int initialized=0;
    int registered=0;
    int ret;
	unsigned int exbuf[1024];
	int exbufsize=1024;
	uint32_t exbuflen;
	struct timeval tmval;

    serverport += streamid;

    for(i=0; i<100; i++)
    {
        gmiret = pClient->Initialize(clientport,GM_STREAM_APPLICATION_ID);
        if(gmiret == GMI_SUCCESS)
        {
            break;
        }
        clientport += 1;
    }

    if(gmiret != GMI_SUCCESS)
    {
        ret = -errno ? -errno : -1;
		ERROR_INFO("could not initialize clientport %d error(%d)gmiret(0x%lx)\n",clientport,ret,gmiret);
		goto out;
    }
	initialized = 1;

	gmiret = pClient->Register(serverport,MEDIA_VIDEO_H264,streamid,true,NULL,NULL);
	if (gmiret != GMI_SUCCESS)
	{
		ret = -errno ? -errno : -1;
		ERROR_INFO("could not register port %d type %d streamid %d pullmode error(%d) gmiret(0x%lx)\n",
			serverport,MEDIA_VIDEO_H264,streamid,ret,gmiret);
		goto out;
	}
	registered = 1;

	pBuffer = malloc(bufsize);
	if (pBuffer == NULL)
	{
		ret = -errno ? -errno : -1;
		goto out;
	}

	while( pRunningBits && *pRunningBits )
	{
		exbuflen = exbufsize;
		buflen = bufsize;

		gmiret = pClient->Read(pBuffer,&buflen,&tmval,exbuf,&exbuflen);
		if (gmiret == GMI_SUCCESS)
		{
            ExtMediaEncInfo* pInfo=(ExtMediaEncInfo*)exbuf;
			if ((pInfo->s_FrameNum % 200)==0)
			{
				DEBUG_INFO("[%d] type %d frameidx %d size %d\n",
					streamid,pInfo->s_FrameType,pInfo->s_FrameNum,
					pInfo->s_Length);
			}
		}
		else
		{
			usleep(40000);
		}
	}

	ret = 0;
	

out:
	if (pBuffer)
	{
		free(pBuffer);
	}
	pBuffer = NULL;
	if (registered)
	{
		pClient->Unregister();
	}
	registered = 0;
	if (initialized)
	{
		pClient->Deinitialize();
	}
	initialized = 0;
    return ret;
}

static int st_RunLoop=1;

void SigHandler(int signo)
{
	if (signo == SIGINT || signo == SIGTERM)
	{
		st_RunLoop = 0;
	}
}


int main(int argc,char* argv[])
{
	int streamid=0;
	IPC_MediaDataClient *pClient=NULL;
	int ret;

	if (argc > 1)
	{
		streamid = atoi(argv[1]);
	}

	signal(SIGINT,SigHandler);
	signal(SIGTERM,SigHandler);

	pClient = new IPC_MediaDataClient();

	ret = GetClientStream(pClient,streamid,&st_RunLoop);
	delete pClient;
	return ret;
	
}

