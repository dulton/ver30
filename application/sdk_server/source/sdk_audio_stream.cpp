
#include <sdk_audio_stream.h>
#include <sdk_server_debug.h>
#include <ipc_fw_v3.x_resource.h>
#include <gmi_config_api.h>
#include <ipc_media_data_dispatch.h>
#include <gmi_media_ctrl.h>
#include <memory>

#define  VFREQ_PER_SECOND(sec)  ( (unsigned long long)(((unsigned long long)sec)*1000000))
#define  VFREQ_PER_USECOND(usec)  ((unsigned long long)(((unsigned long long)usec)))
#define  VFREQ_PER_NANOSECOND(usec)  ((unsigned long long)(((unsigned long long)usec)/1000))



SdkAudioStream::SdkAudioStream(sdk_audio_format_enum_t format,int maxpackets,int * pRunningBits) :
    m_Format(format),
    m_MaxPackets(maxpackets) ,
    m_pRunningBits(pRunningBits)
{
    GMI_RESULT gmiret;
    gmiret = m_Mutex.Create(NULL);
    SDK_ASSERT(gmiret == GMI_SUCCESS);
#ifdef AUDIO_DUAL_FILE_EMULATE
#else
    m_Initialized = 0;
    m_Registered = 0;
#endif /*AUDIO_DUAL_FILE_EMULATE*/
    /*ipc client will init itself*/
    SDK_ASSERT(m_DataVec.size() == 0);
    SDK_ASSERT(m_DataLenVec.size() == 0);
    SDK_ASSERT(m_FrameNumVec.size() == 0);
    SDK_ASSERT(m_PtsVec.size() == 0);

#ifdef AUDIO_PULL_MODE
    m_ThreadRunning  =0 ;
    m_ThreadExited = 1;
    m_pThread = NULL;
#endif
}

#ifdef AUDIO_DUAL_FILE_EMULATE

#define AUDIO_PAUSE_ASSERT() \
do\
{\
	SDK_ASSERT(this->m_ThreadRunning == 0);\
	SDK_ASSERT(this->m_ThreadExited == 1);\
	SDK_ASSERT(this->m_pThread == NULL);\
}while(0)

#define AUDIO_STOP_ASSERT() \
do\
{\
	SDK_ASSERT(this->m_DataVec.size() == 0);\
	SDK_ASSERT(this->m_DataLenVec.size() == 0);\
	SDK_ASSERT(this->m_FrameNumVec.size() == 0);\
	SDK_ASSERT(this->m_PtsVec.size() == 0);\
	SDK_ASSERT(this->m_ThreadRunning == 0);\
	SDK_ASSERT(this->m_ThreadExited == 1);\
	SDK_ASSERT(this->m_pThread == NULL);\
}\
while(0)


#else /*AUDIO_DUAL_FILE_EMULATE*/

#ifdef AUDIO_PULL_MODE

#define AUDIO_PAUSE_ASSERT() \
do\
{\
	SDK_ASSERT(this->m_Initialized == 0);\
	SDK_ASSERT(this->m_Registered == 0);\
	SDK_ASSERT(this->m_ThreadRunning == 0);\
	SDK_ASSERT(this->m_ThreadExited == 1);\
	SDK_ASSERT(this->m_pThread == NULL);\
}while(0)

#define AUDIO_STOP_ASSERT() \
do\
{\
	SDK_ASSERT(this->m_Initialized == 0);\
	SDK_ASSERT(this->m_Registered == 0);\
	SDK_ASSERT(this->m_DataVec.size() == 0);\
	SDK_ASSERT(this->m_DataLenVec.size() == 0);\
	SDK_ASSERT(this->m_FrameNumVec.size() == 0);\
	SDK_ASSERT(this->m_PtsVec.size() == 0);\
	SDK_ASSERT(this->m_ThreadRunning == 0);\
	SDK_ASSERT(this->m_ThreadExited == 1);\
	SDK_ASSERT(this->m_pThread == NULL);\
}\
while(0)

#else

#define AUDIO_PAUSE_ASSERT() \
do\
{\
	SDK_ASSERT(this->m_Initialized == 0);\
	SDK_ASSERT(this->m_Registered == 0);\
}while(0)


#define AUDIO_STOP_ASSERT() \
do\
{\
	SDK_ASSERT(this->m_Initialized == 0);\
	SDK_ASSERT(this->m_Registered == 0);\
	SDK_ASSERT(this->m_DataVec.size() == 0);\
	SDK_ASSERT(this->m_DataLenVec.size() == 0);\
	SDK_ASSERT(this->m_FrameNumVec.size() == 0);\
	SDK_ASSERT(this->m_PtsVec.size() == 0);\
}\
while(0)


#endif

#endif /*AUDIO_DUAL_FILE_EMULATE*/

#ifdef AUDIO_DUAL_FILE_EMULATE

#else /*AUDIO_DUAL_FILE_EMULATE*/
void SdkAudioStream::__ClearIPC()
{
    GMI_RESULT gmiret;
    int tries;
    if(this->m_Registered)
    {
        tries = 0;
        do
        {
            gmiret = this->m_IPCClient.Unregister();
            if(gmiret != GMI_SUCCESS)
            {
                tries ++;
                /*sleep 10 millisecond*/
                usleep(10000);
            }
        }
        while(gmiret != GMI_SUCCESS && tries < 5);
        if(gmiret != GMI_SUCCESS)
        {
            DEBUG_INFO("Could not unregister succ(0x%08lx)\n",gmiret);
        }
    }
    this->m_Registered = 0;

    if(this->m_Initialized)
    {
        tries = 0;
        do
        {
            gmiret = this->m_IPCClient.Deinitialize();
            if(gmiret != GMI_SUCCESS)
            {
                tries ++;
                usleep(10000);
            }
        }
        while(gmiret != GMI_SUCCESS && tries < 5);
        if(gmiret != GMI_SUCCESS)
        {
            DEBUG_INFO("Could not deinitialize 0x%08lx\n",gmiret);
        }
    }
    this->m_Initialized = 0;

    return ;
}

int SdkAudioStream::__GetServerPort()
{
    GMI_RESULT gmiret;
    FD_HANDLE xmlhd=NULL;
    int serverport;

    gmiret = GMI_XmlOpen(GMI_RESOURCE_CONFIG_FILE_NAME,&xmlhd);
    if(gmiret != GMI_SUCCESS)
    {
        goto set_default;
    }
    gmiret = GMI_XmlRead(xmlhd,MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH,
                         MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SERVER_UDP_PORT ,
                         GMI_STREAMING_MEDIA_SERVER_ENCODE_AUDIO1,
                         &serverport,GMI_CONFIG_READ_ONLY);
    if(gmiret != GMI_SUCCESS)
    {
        goto set_default;
    }
    GMI_XmlFileSave(xmlhd);
    xmlhd = NULL;

    return serverport;
set_default:
    if(xmlhd)
    {
        GMI_XmlFileSave(xmlhd);
    }
    xmlhd= NULL;
    return GMI_STREAMING_MEDIA_SERVER_ENCODE_AUDIO1;
}

int SdkAudioStream::__GetClientStartPort()
{
    int clientport;
    FD_HANDLE xmlhd=NULL;
    GMI_RESULT gmiret;

    gmiret = GMI_XmlOpen(GMI_RESOURCE_CONFIG_FILE_NAME,&xmlhd);
    if(gmiret != GMI_SUCCESS)
    {
        goto set_default;
    }
    gmiret = GMI_XmlRead(xmlhd,SDK_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH,
                         SDK_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_CLIENT_UDP_PORT,
                         GMI_STREAMING_MEDIA_SDK_ENCODE_AUDIO1,
                         &clientport,GMI_CONFIG_READ_ONLY);
    if(gmiret != GMI_SUCCESS)
    {
        goto set_default;
    }
    GMI_XmlFileSave(xmlhd);
    xmlhd = NULL;

    return clientport;

set_default:
    if(xmlhd)
    {
        GMI_XmlFileSave(xmlhd);
    }
    xmlhd= NULL;
    return GMI_STREAMING_MEDIA_SDK_ENCODE_AUDIO1;
}

int SdkAudioStream::__InitIPC()
{
    int serverport,clientstartport;
    int i,ret;
    GMI_RESULT gmiret;
    uint32_t type;
    this->__ClearIPC();

    clientstartport = this->__GetClientStartPort();
    for(i=0; i<100; i++)
    {
        gmiret = this->m_IPCClient.Initialize(clientstartport + i,GM_STREAM_APPLICATION_ID);
        if(gmiret == GMI_SUCCESS)
        {
            DEBUG_INFO("start port %d\n",clientstartport+i);
            break;
        }
    }
    if(gmiret != GMI_SUCCESS)
    {
        ret = -errno ? -errno : -1;
        this->__ClearIPC();
        ERROR_INFO("\n");
        return ret;
    }

    this->m_Initialized = 1;

    serverport = this->__GetServerPort();

    if(this->m_Format == audio_format_default ||
            this->m_Format == audio_format_g711)
    {
        type = MEDIA_AUDIO_G711A;
    }
    else
    {
        this->__ClearIPC();
        return -ENOTSUP;
    }
#ifdef AUDIO_PULL_MODE
    DEBUG_INFO("serverport %d type %d:0x%x\n",serverport,type,type);
    gmiret = this->m_IPCClient.Register(serverport,type,0,true,NULL,NULL);
#else
    gmiret = GMI_NOT_IMPLEMENT;
#endif
    if(gmiret != GMI_SUCCESS)
    {
        ret = -errno ? -errno : -1;
        this->__ClearIPC();
        ERROR_INFO("gmiret 0x%08lx\n",gmiret);
        return ret;
    }
    this->m_Registered = 1;
    return 0;
}

#endif /* not define AUDIO_DUAL_FILE_EMULATE*/

int SdkAudioStream::PauseStream()
{
#ifdef AUDIO_PULL_MODE
    this->m_ThreadRunning = 0;
    /*to notify the thread will exit*/
    while(this->m_ThreadExited == 0)
    {
        /*sleep for 100 milli seconds*/
        usleep(100000);
    }

    if(this->m_pThread)
    {
        delete this->m_pThread;
    }
    this->m_pThread = NULL;
#endif

#ifndef AUDIO_DUAL_FILE_EMULATE
    /*now to stop the ipc*/
    this->__ClearIPC();
#endif
    AUDIO_PAUSE_ASSERT();
    return 0;
}

#define VECTOR_EQUAL_ASSERT() \
do\
{\
	SDK_ASSERT(this->m_DataVec.size() <= this->m_MaxPackets || this->m_MaxPackets == 0);\
	SDK_ASSERT(this->m_DataVec.size() == this->m_DataLenVec.size());\
	SDK_ASSERT(this->m_DataLenVec.size() == this->m_PtsVec.size());\
	SDK_ASSERT(this->m_PtsVec.size() == this->m_FrameNumVec.size());\
}while(0)

void SdkAudioStream::__ClearVectors()
{
#ifndef AUDIO_DUAL_FILE_EMULATE
    SDK_ASSERT(this->m_Initialized == 0 && this->m_Registered == 0);
#endif /*AUDIO_DUAL_FILE_EMULATE*/
    /*this is from the single threaded ,so we can do this*/
    VECTOR_EQUAL_ASSERT();

    while(this->m_DataVec.size() > 0)
    {
        void* pData = this->m_DataVec[0];
        this->m_DataVec.erase(this->m_DataVec.begin());
        this->m_DataLenVec.erase(this->m_DataLenVec.begin());
        this->m_PtsVec.erase(this->m_PtsVec.begin());
        this->m_FrameNumVec.erase(this->m_FrameNumVec.begin());
        if(pData)
        {
            free(pData);
        }
        pData = NULL;
        VECTOR_EQUAL_ASSERT();
    }

    return ;

}

int SdkAudioStream::StopStream()
{
    /*now first to stop the pull thread*/
    this->PauseStream();
    this->__ClearVectors();
    return 0;
}


SdkAudioStream::~SdkAudioStream()
{
    GMI_RESULT gmiret;
    this->StopStream();
    gmiret = this->m_Mutex.Destroy();
    SDK_ASSERT(gmiret == GMI_SUCCESS);
    this->m_pRunningBits = NULL;
    this->m_MaxPackets = 0;
    this->m_Format = audio_format_default;
}


int SdkAudioStream::StartStream()
{
    int ret;
    GMI_RESULT gmiret;
    this->StopStream();
#ifndef AUDIO_DUAL_FILE_EMULATE
    ret = this->__InitIPC();
    if(ret < 0)
    {
        this->StopStream();
        ERROR_INFO("\n");
        return ret;
    }
#endif /*not define AUDIO_DUAL_FILE_EMULATE*/

#ifdef AUDIO_PULL_MODE
    /*now we should set for the thread running*/
    this->m_ThreadRunning = 1;
    this->m_ThreadExited = 0;
    this->m_pThread = new GMI_Thread();
    gmiret = this->m_pThread->Create(NULL,0,SdkAudioStream::ThreadFunc,this);
    if(gmiret != GMI_SUCCESS)
    {
        ret = -errno ? -errno : -1;
        /*we make the thread exited ,so */
        this->m_ThreadExited = 1;
        this->StopStream();
        ERROR_INFO("\n");
        return ret;
    }

    gmiret = this->m_pThread->Start();
    if(gmiret != GMI_SUCCESS)
    {
        ret = -errno ? -errno : -1;
        /*we make the thread exited ,so */
        this->m_ThreadExited = 1;
        this->StopStream();
        ERROR_INFO("\n");
        return ret;
    }
#endif


    return 0;
}

int SdkAudioStream::ResumeStream()
{
    int ret;
    GMI_RESULT gmiret;

    this->PauseStream();
#ifndef AUDIO_DUAL_FILE_EMULATE
    ret = this->__InitIPC();
    if(ret < 0)
    {
        this->PauseStream();
        ERROR_INFO("\n");
        return ret;
    }
#endif /*AUDIO_DUAL_FILE_EMULATE*/
#ifdef AUDIO_PULL_MODE
    /*now we should set for the thread running*/
    this->m_ThreadRunning = 1;
    this->m_ThreadExited = 0;
    this->m_pThread = new GMI_Thread();
    gmiret = this->m_pThread->Create(NULL,0,SdkAudioStream::ThreadFunc,this);
    if(gmiret != GMI_SUCCESS)
    {
        ret = -errno ? -errno : -1;
        /*we make the thread exited ,so */
        this->m_ThreadExited = 1;
        this->PauseStream();
        ERROR_INFO("\n");
        return ret;
    }
    gmiret = this->m_pThread->Start();
    if(gmiret != GMI_SUCCESS)
    {
        ret = -errno ? -errno : -1;
        this->m_ThreadExited = 1;
        this->PauseStream();
        ERROR_INFO("\n");
        return ret;
    }
#endif

    return 0;
}


int SdkAudioStream::__PushStreamData(void * pData,uint32_t datalen,uint32_t idx,uint64_t pts)
{
    GMI_RESULT gmiret;
    int ret = 1;
    void* pRemovePtr=NULL;
    uint32_t discardnum =0;

    gmiret = this->m_Mutex.Lock(TIMEOUT_INFINITE);
    SDK_ASSERT(gmiret == GMI_SUCCESS);
    VECTOR_EQUAL_ASSERT();
    if(this->m_MaxPackets && this->m_DataVec.size() == this->m_MaxPackets)
    {
        /*we pick the most early one to get this packet*/
        pRemovePtr = this->m_DataVec[0];
        discardnum = this->m_FrameNumVec[0];
        this->m_DataVec.erase(this->m_DataVec.begin());
        this->m_DataLenVec.erase(this->m_DataLenVec.begin());
        this->m_PtsVec.erase(this->m_PtsVec.begin());
        this->m_FrameNumVec.erase(this->m_FrameNumVec.begin());

        /*now to insert*/
        this->m_DataVec.push_back(pData);
        this->m_DataLenVec.push_back(datalen);
        this->m_FrameNumVec.push_back(idx);
        this->m_PtsVec.push_back(pts);
        ret = 0;
        SDK_ASSERT(this->m_DataVec.size() == this->m_MaxPackets || this->m_MaxPackets == 0);

    }
    else
    {
        ret = 1;
        /*now to insert*/
        this->m_DataVec.push_back(pData);
        this->m_DataLenVec.push_back(datalen);
        this->m_FrameNumVec.push_back(idx);
        this->m_PtsVec.push_back(pts);
    }
    this->m_Mutex.Unlock();

    if(pRemovePtr)
    {
        DEBUG_INFO("discard frame number %d\n",discardnum);
        free(pRemovePtr);
    }
    pRemovePtr = NULL;
    return ret;
}

#ifdef AUDIO_PULL_MODE

#ifdef AUDIO_DUAL_FILE_EMULATE

void* SdkAudioStream::__ThreadImpl()
{
    int ret;
    void* pBuffer=NULL;
    uint32_t buflen = 1024,bufret;
    FILE* fp=NULL;
    uint32_t seekoff=0;
    uint32_t framenum=0;
    unsigned long long pts=0;
    off_t filesize=0;
    char fname[32];

    snprintf(fname,sizeof(fname),"/tmp/test.g711");
    fp = fopen(fname,"r+b");
    if(fp == NULL)
    {
        ret = -errno ? -errno : -1;
        DEBUG_INFO("open %s error %d\n",fname,ret);
        goto out;
    }

    pBuffer = malloc(buflen);
    if(pBuffer == NULL)
    {
        ret = -errno ? -errno : -1;
        goto out;
    }

    ret = fseeko(fp,0,SEEK_END);
    if(ret != 0)
    {
        ret = -errno ? -errno : -1;
        goto out;
    }

    errno = 0;
    filesize = ftello(fp);
    if(filesize == -1 && errno != 0)
    {
        ret = -errno;
        goto out;
    }

    if(filesize < (int)buflen)
    {
        ret = -EINVAL;
        DEBUG_INFO("\n");
        goto out;
    }

    rewind(fp);
    DEBUG_INFO("\n");

    while(this->m_ThreadRunning)
    {
        if(pBuffer == NULL)
        {
            pBuffer = malloc(buflen);
        }
        if(pBuffer == NULL)
        {
            ret = -errno ? -errno : -1;
            DEBUG_INFO("\n");
            goto out;
        }

        if((filesize - seekoff) < buflen)
        {
            seekoff =0;
            rewind(fp);
        }

        if((seekoff % 16000) == 0)
        {
            DEBUG_INFO("seekoff %d\n",seekoff);
        }

        bufret = 160;
        ret = fread(pBuffer,bufret,1,fp);
        if(ret != 1)
        {
            ret = -errno ? -errno : -1;
            DEBUG_INFO("read at 0x%08x error %d\n",seekoff,ret);
            goto out;
        }
        //DEBUG_BUFFER_FMT(pBuffer,(bufret > 20 ? 20 : bufret),"At seekoff %d\t",seekoff);

        ret = this->__PushStreamData(pBuffer,bufret,framenum,pts);
        if(ret < 0)
        {
            DEBUG_INFO("\n");
            goto out;
        }
        /*success ,so we do this*/
        pBuffer = NULL;
        usleep(20000);

        seekoff += bufret;
        framenum += 1;
        pts += 1800;
    }

out:
    if(pBuffer)
    {
        free(pBuffer);
    }
    pBuffer = NULL;
    if(fp)
    {
        fclose(fp);
    }
    fp = NULL;

    DEBUG_INFO("\n");
    this->m_ThreadExited = 1;
    return (void*)ret;
}

#else /*AUDIO_DUAL_FILE_EMULATE*/
void* SdkAudioStream::__ThreadImpl()
{
    int ret;
    void* pBuffer = NULL;
    /*for 16 k*/
    uint32_t buflen,bufret;
    uint32_t infosize=1024,inputsize;
    std::auto_ptr<unsigned char> pExInfo2(new unsigned char[infosize]);
    unsigned char* pExInfo =pExInfo2.get();
    struct timeval tmval;
    uint32_t lastidx=0;
    GMI_RESULT gmiret;
    unsigned long long pts=0,lasttick=0,curtick=0;
    struct timespec tmspec;

    buflen = 0x1000;
    if(this->m_Format == audio_format_g711 ||
            this->m_Format == audio_format_default)
    {
        /*for 16k bytes*/
        buflen = 0x4000;
    }
    DEBUG_INFO("start thread impl\n");
    while(this->m_ThreadRunning)
    {

        if(pBuffer== NULL)
        {
            /*we reuse the buffer ,when last time not get data and insert ,so we can limit the time*/
            pBuffer = malloc(buflen);
        }
        if(pBuffer == NULL)
        {
            ret = -errno ? -errno : -1;
            goto out;
        }

        bufret = buflen;
        inputsize= infosize;
        gmiret = this->m_IPCClient.Read(pBuffer,&bufret,&tmval,pExInfo,&inputsize);
        if(gmiret ==GMI_SUCCESS)
        {
            ExtMediaEncInfo* pInfo=(ExtMediaEncInfo*)pExInfo;
            /*now to get the */
            pts = VFREQ_PER_SECOND(tmval.tv_sec) + VFREQ_PER_USECOND(tmval.tv_usec);
            if(pts < 100000)
            {
                /*make sure it is not overflow*/
                pts *= 9;
                pts /= 100;
            }
            else
            {
                pts /= 100;
                pts *= 9;
            }

            ret = clock_gettime(CLOCK_MONOTONIC,&tmspec);
            if(ret < 0)
            {
                ERROR_INFO("could not get time error(%d)\n",errno);
            }
            else
            {
                curtick = VFREQ_PER_SECOND(tmspec.tv_sec) + VFREQ_PER_NANOSECOND(tmspec.tv_nsec);
                if(lasttick && (curtick - lasttick) >200000)
                {
                    ERROR_INFO("audio[%d] curtick(%lld) lasttick(%lld) (%lld)\n",pInfo->s_FrameNum,curtick,lasttick,(curtick-lasttick));
                }
				lasttick = curtick;
            }



            if(pInfo->s_FrameNum != (lastidx + 1))
            {
                DEBUG_INFO("framenum (%d) != (%d + 1)\n",pInfo->s_FrameNum,lastidx);
            }
            lastidx = pInfo->s_FrameNum;
            if((lastidx % 200)==0)
            {
                DEBUG_BUFFER_FMT(pBuffer,bufret,"framenum %d",lastidx);
            }

            ret = this->__PushStreamData(pBuffer,bufret,pInfo->s_FrameNum,pts);
            if(ret < 0)
            {
                goto out;
            }
            /*insert success ,so we should not free this buffer*/
            pBuffer = NULL;

        }
        else
        {
            /*we are not success ,so we sleep for a while*/
            usleep(20000);
        }

    }
    ret = 0;

out:
    if(pBuffer)
    {
        free(pBuffer);
    }
    pBuffer = NULL;
    DEBUG_INFO("stop thread impl\n");
    this->m_ThreadExited = 1;
    return (void*) ret;
}
#endif /*else AUDIO_DUAL_FILE_EMULATE */

void* SdkAudioStream::ThreadFunc(void * arg)
{
    SdkAudioStream* pThis = (SdkAudioStream*)arg;
    return pThis->__ThreadImpl();
}
#endif /*AUDIO_PULL_MODE*/

int SdkAudioStream::PullStreamData(stream_pack_t * pPack)
{
    int ret=0;
    GMI_RESULT gmiret;
#ifdef AUDIO_DUAL_FILE_EMULATE
#ifdef AUDIO_PULL_MODE
    if(this->m_pThread == NULL)
    {
        return 0;
    }
#endif /*AUDIO_PULL_MODE*/
#else
    if(this->m_Initialized == 0)
    {
        /*nothing get */
        return 0;
    }
#endif	/*AUDIO_DUAL_FILE_EMULATE*/

    gmiret = this->m_Mutex.Lock(TIMEOUT_INFINITE);
    SDK_ASSERT(gmiret == GMI_SUCCESS);

    VECTOR_EQUAL_ASSERT();
    if(this->m_DataVec.size() > 0)
    {
        /*now put the value*/
        pPack->m_pData = this->m_DataVec[0];
        pPack->m_DataLen = this->m_DataLenVec[0];
        pPack->m_Idx = this->m_FrameNumVec[0];
        pPack->m_Pts = this->m_PtsVec[0];
        pPack->m_Type = 0;

        this->m_DataVec.erase(this->m_DataVec.begin());
        this->m_DataLenVec.erase(this->m_DataLenVec.begin());
        this->m_PtsVec.erase(this->m_PtsVec.begin());
        this->m_FrameNumVec.erase(this->m_FrameNumVec.begin());

        /*we have get one*/
        ret = 1;
    }

    this->m_Mutex.Unlock();
    return ret;
}

