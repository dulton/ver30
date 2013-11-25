
#include <sdk_audio_dec.h>
#include <ipc_fw_v3.x_resource.h>
#include <sdk_client_buffer.h>
#include <sdk_server_debug.h>

SdkAudioDec::SdkAudioDec(int maxpacks,int * pRunningBits) :
    m_MaxPacks(maxpacks),
    m_pRunningBits(pRunningBits)
{
    GMI_RESULT gmiret;
    gmiret = m_Mutex.Create(NULL);
    SDK_ASSERT(gmiret == GMI_SUCCESS);
#ifdef NOT_MEDIA_DECODE_STREAM_PUT
    m_pFp = NULL;
#else
    m_ServerInitialized = 0;
    m_pDataServer = NULL;
#endif
    m_LastFrameIdx = 0;
    m_pThread = NULL;
    m_ThreadRunning = 0;
    m_ThreadExited = 1;   /* for exited as default*/
    SDK_ASSERT(m_RecvComm.size() == 0);
    m_StreamStarted = 0;
}

void SdkAudioDec::__ClearComms()
{
    sdk_client_comm_t* pComm =NULL;
    while(this->m_RecvComm.size() > 0)
    {
        SDK_ASSERT(pComm == NULL);
        pComm = this->m_RecvComm[0];
        this->m_RecvComm.erase(this->m_RecvComm.begin());
        free(pComm);
        pComm = NULL;
    }
    return ;
}

#define DECODE_PAUSE_ASSERT() \
do\
{\
	SDK_ASSERT(this->m_pThread == NULL);\
	SDK_ASSERT(this->m_ThreadRunning == 0);\
	SDK_ASSERT(this->m_ThreadExited == 1);\
	SDK_ASSERT(this->m_RecvComm.size() == 0);\
	SDK_ASSERT(this->m_LastFrameIdx == 0);\
}while(0)


int SdkAudioDec::__PauseStream()
{
    if(this->m_pThread)
    {
        this->m_ThreadRunning = 0;
        while(this->m_ThreadExited == 0)
        {
            /*if we have to wait for the exited ,so just wait */
            usleep(10000);
        }

        delete this->m_pThread;
    }
    this->m_pThread= NULL;

    /*because we have stop the thread ,so we can use this to clear comms by the default*/
    this->__ClearComms();
    this->m_LastFrameIdx = 0;

    DECODE_PAUSE_ASSERT();

    return 0;
}

void SdkAudioDec::__StopStream()
{

    this->__PauseStream();
    this->__DeInitializeDataServer();
    this->m_StreamStarted = 0;
    return ;
}

void SdkAudioDec::StopStream()
{
    this->__StopStream();
    return ;
}

SdkAudioDec::~SdkAudioDec()
{
    this->StopStream();
    this->m_Mutex.Destroy();
    this->m_MaxPacks = 0;
    this->m_pRunningBits = NULL;
}

int SdkAudioDec::PauseStream()
{
    if(this->m_StreamStarted == 0)
    {
        return -EPERM;
    }
    this->__PauseStream();
    return 0;
}

int SdkAudioDec::__ResumeStream(AudioDecParam * pAudioDec)
{
    /*now we should test if we have started*/
    GMI_RESULT gmiret;
    int ret;

    /*now just make things ok*/
    this->__PauseStream();
    DECODE_PAUSE_ASSERT();
    /*first we should make thread running*/
    this->m_ThreadRunning = 1;
    this->m_ThreadExited = 0;

    this->m_pThread = new GMI_Thread();
    gmiret = this->m_pThread->Create(NULL,0,SdkAudioDec::__ThreadFunc,this);
    if(gmiret != GMI_SUCCESS)
    {
        /*we pretend for it will exited*/
        ret = -errno ? -errno : -1;
        this->m_ThreadExited = 1;
        this->__PauseStream();
        return ret;
    }

    /*now to start*/
    gmiret = this->m_pThread->Start();
    if(gmiret != GMI_SUCCESS)
    {
        /*we pretend for it will exited*/
        ret = -errno ? -errno : -1;
        this->m_ThreadExited = 1;
        this->__PauseStream();
        return ret;
    }
    this->m_LastFrameIdx = 0;

    /*this is running ,so we do things ok*/
    return 0;
}

int SdkAudioDec::ResumeStream(AudioDecParam * pAudioDec)
{
    int ret;
    if(this->m_StreamStarted == 0)
    {
        return -EINVAL;
    }
    ret = this->__ResumeStream(pAudioDec);
    if(ret < 0)
    {
        this->__PauseStream();
        return ret;
    }
    return 0;
}

int SdkAudioDec::__GetBindPort()
{
    return GMI_STREAMING_MEDIA_SDK_DECODE_AUDIO1;
}


#ifdef NOT_MEDIA_DECODE_STREAM_PUT
void SdkAudioDec::__DeInitializeDataServer()
{
    if(this->m_pFp)
    {
        fclose(this->m_pFp);
    }
    this->m_pFp = NULL;
    return ;
}

int SdkAudioDec::__InitailizeDataServer(AudioDecParam * pAudioDec)
{
    char fname[32];
    int ret;
    time_t curt;
    if(this->m_pFp)
    {
        return 0;
    }

    curt = time(NULL);
    srand(curt);
    snprintf(fname,sizeof(fname),"/tmp/audio%04d.g711",rand() % 0xffff);

    this->m_pFp = fopen(fname,"w+b");
    if(this->m_pFp == NULL)
    {
        ret = -errno ? -errno : -1;
        this->__DeInitializeDataServer();
        return ret;
    }

    DEBUG_INFO("open file (%s) succ\n",fname);

    return 0;

}


#else

void SdkAudioDec::__DeInitializeDataServer()
{
    /*we delete the data server ,for it will stop the stream really*/
    if(this->m_ServerInitialized)
    {
        /*now to give the server deinitialized*/
        SDK_ASSERT(this->m_pDataServer);
        this->m_pDataServer->Deinitialize();
    }
    this->m_ServerInitialized = 0;
    if(this->m_pDataServer)
    {
        delete this->m_pDataServer;
    }
    this->m_pDataServer = NULL;
    return ;
}


int SdkAudioDec::__InitailizeDataServer(AudioDecParam * pAudioDec)
{
    GMI_RESULT gmiret;
    int ret;
    int serverport=0;
    /*make sure we are at no thread initialized*/
    SDK_ASSERT(this->m_pDataServer == NULL && this->m_pThread == NULL);

    this->m_pDataServer = new IPC_MediaDataServer();
    serverport = this->__GetBindPort();
    DEBUG_INFO("Initialize %d port mediatype 0x%08x:%d mediaid 0 sharememkey 0x%08x:%d sharememsize 0x%08x:%d maxsize 0x%08x:%d minsize 0x%08x:%d ipcmutexkey 0x%08x\n",
               serverport,MEDIA_AUDIO_G711A,MEDIA_AUDIO_G711A,GMI_STREAMING_MEDIA_SERVER_DECODE_AUDIO1_SHARE_MEMORY_KEY,GMI_STREAMING_MEDIA_SERVER_DECODE_AUDIO1_SHARE_MEMORY_KEY,
               0x1000000,0x1000000,
               0x80000,0x80000,
               0x1000,0x1000,GMI_STREAMING_MEDIA_SERVER_DECODE_AUDIO1_IPC_MUTEX_KEY);
    gmiret = this->m_pDataServer->Initialize(serverport,MEDIA_AUDIO_G711A,0,GMI_STREAMING_MEDIA_SERVER_DECODE_AUDIO1_SHARE_MEMORY_KEY,0x1000000,0x80000,0x1000,GMI_STREAMING_MEDIA_SERVER_DECODE_AUDIO1_IPC_MUTEX_KEY);
    if(gmiret != GMI_SUCCESS)
    {
        ret = -errno ? -errno : -1;
        this->__DeInitializeDataServer();
        return ret;
    }

    this->m_ServerInitialized = 1;

    return 0;

}
#endif

int SdkAudioDec::__StartStream(AudioDecParam * pAudioDec)
{
    int ret;
    this->__StopStream();

    ret = this->__InitailizeDataServer(pAudioDec);
    if(ret < 0)
    {
        this->__StopStream();
        return ret;
    }

    ret = this->__ResumeStream(pAudioDec);
    if(ret < 0)
    {
        this->__StopStream();
        return ret;
    }


    this->m_StreamStarted = 1;
    return 0;
}

int SdkAudioDec::StartStream(AudioDecParam * pAudioDec)
{
    if(this->m_MaxPacks == 0)
    {
        return -EINVAL;
    }
    return this->__StartStream(pAudioDec);
}

int SdkAudioDec::__PushAudioDec(sdk_client_comm_t * & pComm)
{
    GMI_RESULT gmiret;
    int ret=1;
    sdk_client_comm_t* pRemoveComm=NULL;

    gmiret = this->m_Mutex.Lock(TIMEOUT_INFINITE);
    SDK_ASSERT(gmiret == GMI_SUCCESS);

    if(this->m_RecvComm.size() == this->m_MaxPacks)
    {
        pRemoveComm = this->m_RecvComm[0];
        this->m_RecvComm.erase(this->m_RecvComm.begin());
        this->m_RecvComm.push_back(pComm);
        pComm = NULL;
        ret = 0;
    }
    else
    {
        this->m_RecvComm.push_back(pComm);
        pComm = NULL;
    }
    this->m_Mutex.Unlock();

    if(pRemoveComm)
    {
        FreeComm(pRemoveComm);
    }
    pRemoveComm = NULL;

    return ret;
}

int SdkAudioDec::PushAudioDec(sdk_client_comm_t * & pComm)
{
    if(this->m_StreamStarted == 0)
    {
        return -EPERM;
    }

    if(this->m_pThread == NULL)
    {
        /*started ,but pause ,so we do this*/
        FreeComm(pComm);
        return 1;
    }

    if(this->m_ThreadExited)
    {
        /*this means that the thread exited unexpected*/
        return -EFAULT;
    }

    return this->__PushAudioDec(pComm);
}

int SdkAudioDec::__GetAudioDec(sdk_client_comm_t * & pComm)
{
    int ret=0;
    GMI_RESULT gmiret;

    gmiret = this->m_Mutex.Lock(TIMEOUT_INFINITE);
    SDK_ASSERT(gmiret == GMI_SUCCESS);

    if(this->m_RecvComm.size() > 0)
    {
        pComm = this->m_RecvComm[0];
        this->m_RecvComm.erase(this->m_RecvComm.begin());
        ret = 1;
    }
    this->m_Mutex.Unlock();

    return ret;
}


void* SdkAudioDec::__ThreadFunc(void * arg)
{
    SdkAudioDec* pThis  = (SdkAudioDec*)arg;
    return pThis->__ThreadImpl();
}

#define  PTS_TO_SECOND(pts)  ( (unsigned long long)(((unsigned long long)pts)/1000000))
#define  PTS_TO_USECOND(pts)  ((unsigned long long)(((unsigned long long)pts)%1000000))


int SdkAudioDec::__TransPtsToTm(uint64_t pts,struct timeval * pTm)
{
    uint64_t basepts;


    if(pts <= 900000)
    {
        basepts = pts;
        basepts *= 100;
        basepts /= 9;
    }
    else
    {
        basepts = pts;
        basepts /= 9;
        basepts *= 100;
    }

    pTm->tv_sec =PTS_TO_SECOND(basepts);
    pTm->tv_usec = PTS_TO_USECOND(basepts);


    return 0;
}



#ifdef NOT_MEDIA_DECODE_STREAM_PUT

int SdkAudioDec::__WriteServerData(sdk_client_comm_t * & pComm)
{
    int ret;
    audio_frame_header_t* pAudioHeader = (audio_frame_header_t*)(pComm->m_Data);
    uint8_t *pData=(uint8_t*)((unsigned long)pAudioHeader + sizeof(*pAudioHeader));
    size_t datalen = pComm->m_DataLen - sizeof(*pAudioHeader);
    uint32_t n32;
    uint32_t expectidx;
    uint64_t n64;


    SDK_ASSERT(this->m_pFp);

    if(pComm->m_DataLen < sizeof(*pAudioHeader))
    {
        /*this is not the data*/
        DEBUG_INFO("\n");
        FreeComm(pComm);
        return 0;
    }

    /*now to test for the code*/
    n32 = PROTO_TO_HOST32(pAudioHeader->m_AudioCode);
    if(n32 != SDK_STREAM_AD_SEND)
    {
        DEBUG_INFO("n32 %d != SDK_STREAM_AD_SEND (%d)\n",n32,SDK_STREAM_AD_SEND);
        FreeComm(pComm);
        return 0;
    }
    if(pAudioHeader->m_FrameType != 'A')
    {
        DEBUG_INFO("not audio type\n");
        FreeComm(pComm);
        return 0;
    }

    n64 = PROTO_TO_HOST64(pAudioHeader->m_Pts);
    /*now to transfer the pts*/

    n32 = PROTO_TO_HOST32(pAudioHeader->m_FrameId);

    if(this->m_LastFrameIdx != 0)
    {
        expectidx = this->m_LastFrameIdx;
        expectidx ++;
        if(expectidx != n32)
        {
            ERROR_INFO("lastframeidx[%d] curframeidx[%d] pts 0x%llx\n",this->m_LastFrameIdx,n32,n64);
        }
    }
    this->m_LastFrameIdx = n32;


    ret = fwrite(pData,datalen,1,this->m_pFp);
    if(ret != 1)
    {
        ret = -errno ? -errno : -1;
        ERROR_INFO("\n");
        return ret;
    }
    if((this->m_LastFrameIdx % 100) == 0)
    {
        DEBUG_INFO("write %d file with frameidx %d datalen %d\n",fileno(this->m_pFp),n32,datalen);
    }
    FreeComm(pComm);
    return 1;
}


void* SdkAudioDec::__ThreadImpl()
{
    int ret=0;
    sdk_client_comm_t* pComm=NULL;

    SDK_ASSERT(this->m_pFp);

    while(this->m_ThreadRunning)
    {
        SDK_ASSERT(pComm==NULL);
        ret = this->__GetAudioDec(pComm);
        if(ret < 0)
        {
            goto out;
        }
        else if(ret == 0)
        {
            usleep(20000);
            continue;
        }

        SDK_ASSERT(pComm);

        /*now write the comm*/
        ret = this->__WriteServerData(pComm);
        if(ret < 0)
        {
            goto out;
        }
    }

    ret = 0;

out:
    FreeComm(pComm);
    this->m_ThreadExited = 1;
    return (void*)ret;
}


#else

int SdkAudioDec::__WriteServerData(sdk_client_comm_t * & pComm)
{
    int ret;
    audio_frame_header_t* pAudioHeader = (audio_frame_header_t*)(pComm->m_Data);
    uint8_t *pData=(uint8_t*)((unsigned long)pAudioHeader + sizeof(*pAudioHeader));
    size_t datalen = pComm->m_DataLen -  sizeof(*pAudioHeader);
    uint32_t n32;
    uint64_t n64;
    struct timeval tmval;
    GMI_RESULT gmiret;
    ExtMediaEncInfo ExInfo;
    uint32_t expectidx;


    SDK_ASSERT(this->m_pDataServer);
    SDK_ASSERT(this->m_ServerInitialized);

    if(pComm->m_DataLen < (sizeof(*pAudioHeader)))
    {
        /*this is not the data*/
        DEBUG_INFO("\n");
        FreeComm(pComm);
        return 0;
    }

    /*now to test for the code*/
    n32 = PROTO_TO_HOST32(pAudioHeader->m_AudioCode);
    if(n32 != SDK_STREAM_AD_SEND)
    {
        DEBUG_INFO("\n");
        FreeComm(pComm);
        return 0;
    }

    n64 = PROTO_TO_HOST64(pAudioHeader->m_Pts);
    /*now to transfer the pts*/
    this->__TransPtsToTm(n64,&tmval);

    n32 = PROTO_TO_HOST32(pAudioHeader->m_FrameId);
    memset(&ExInfo,0,sizeof(ExInfo));
    ExInfo.s_FrameNum = n32;
    ExInfo.s_Length = datalen;

    if(this->m_LastFrameIdx != 0)
    {
        expectidx = this->m_LastFrameIdx;
        expectidx ++;
        if(expectidx != ExInfo.s_FrameNum)
        {
            ERROR_INFO("lastframeidx[%d] curframeidx[%d]\n",this->m_LastFrameIdx,ExInfo.s_FrameNum);
        }
    }
    this->m_LastFrameIdx = ExInfo.s_FrameNum;
    if((ExInfo.s_FrameNum % 100) == 0)
    {
        DEBUG_BUFFER_FMT(pData,(datalen > 16 ? 16 : datalen),"Write Frame[%d] time pts %lld:0x%llx (timeval %ld:%ld)",
                         ExInfo.s_FrameNum,n64,n64,tmval.tv_sec,tmval.tv_usec);
    }
    gmiret = this->m_pDataServer->Write(pData,datalen,&tmval,&ExInfo,sizeof(ExInfo));
    if(gmiret != GMI_SUCCESS)
    {
        ret = -errno ? -errno : -1;
        return ret;
    }
    if((this->m_LastFrameIdx % 100) == 0)
    {
        DEBUG_INFO("write gmi with frameidx %d datalen %d\n",n32,datalen);
    }

    FreeComm(pComm);
    return 1;
}


void* SdkAudioDec::__ThreadImpl()
{
    int ret=0;
    sdk_client_comm_t* pComm=NULL;

    SDK_ASSERT(this->m_pDataServer);
    SDK_ASSERT(this->m_ServerInitialized);

    while(this->m_ThreadRunning)
    {
        SDK_ASSERT(pComm==NULL);
        ret = this->__GetAudioDec(pComm);
        if(ret < 0)
        {
            goto out;
        }
        else if(ret == 0)
        {
            usleep(20000);
            continue;
        }

        SDK_ASSERT(pComm);

        /*now write the comm*/
        ret = this->__WriteServerData(pComm);
        if(ret < 0)
        {
            goto out;
        }
    }

    ret = 0;

out:
    if(pComm)
    {
        free(pComm);
    }
    pComm = NULL;
    this->m_ThreadExited = 1;
    return (void*)ret;
}

#endif
