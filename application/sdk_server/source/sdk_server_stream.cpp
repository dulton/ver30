

#include <sdk_server_stream.h>
#include <ipc_fw_v3.x_resource.h>
#include <gmi_config_api.h>
#include <ipc_media_data_dispatch.h>
#include <sys_env_types.h>
#include <gmi_media_ctrl.h>
#include <sdk_server_debug.h>
#include <sys_stream_info.h>
#include <memory>

#define  VFREQ_PER_SECOND(sec)  ( (unsigned long long)(((unsigned long long)sec)*1000000))
#define  VFREQ_PER_USECOND(usec)  ((unsigned long long)(((unsigned long long)usec)))
#define  VFREQ_PER_NANOSECOND(usec)  ((unsigned long long)(((unsigned long long)usec)/1000))


SdkServerStream::SdkServerStream(int streamid,int type,int maxpacks)
    : m_StreamId(streamid) ,
      m_Type(type),
      m_MaxPacks(maxpacks)
{
    GMI_RESULT gmiret;
    gmiret = m_Mutex.Create(NULL);
    SDK_ASSERT(gmiret == GMI_SUCCESS);
#ifdef VIDEO_STREAM_EMULATE
#else
    m_Initialized = 0;
    m_Registered = 0;
#endif /*VIDEO_STREAM_EMULATE*/
    SDK_ASSERT(m_DataVec.size() == 0);
    SDK_ASSERT(m_DataLen.size() == 0);
    SDK_ASSERT(m_DataType.size() == 0);
    SDK_ASSERT(m_DataIdx.size() == 0);
    SDK_ASSERT(m_DataPts.size() == 0);
#ifdef STREAM_PULL_MODE
    m_ThreadRunning = 0;
    m_ThreadExited = 1;
    m_pThread = NULL;
#endif
}

#ifdef VIDEO_STREAM_EMULATE

#else

void SdkServerStream::__ClearIPC()
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
                DEBUG_INFO("gmiret 0x%08lx errno = %d\n",gmiret,errno);
                usleep(10000);
            }
            tries ++;
        }
        while(gmiret != GMI_SUCCESS && tries < 5);

        if(gmiret != GMI_SUCCESS)
        {
            ERROR_INFO("could not unregister 0x%08lx\n",gmiret);
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
                DEBUG_INFO("gmiret 0x%08lx errno = %d\n",gmiret,errno);
                usleep(10000);
            }
            tries ++;
        }
        while(gmiret != GMI_SUCCESS && tries < 5);
        if(gmiret != GMI_SUCCESS)
        {
            ERROR_INFO("could not deinitialize 0x%08lx\n",gmiret);
        }
    }
    this->m_Initialized = 0;
    return ;
}

int SdkServerStream::__GetServerPort()
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
                         GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1,
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
    return GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1;
}


int SdkServerStream::__GetClientStartPort()
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
                         GMI_STREAMING_MEDIA_SDK_ENCODE_VIDEO1,
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
    return GMI_STREAMING_MEDIA_SDK_ENCODE_VIDEO1;
}





int SdkServerStream::__InitIPC()
{
    int startport,serverport;
    GMI_RESULT gmiret;
    uint32_t type;
    int i;
    startport = this->__GetClientStartPort();
    serverport = this->__GetServerPort();

    //TODO we need to get startport and server here

    /*now first to initialize*/
    for(i=0; i<100; i++)
    {
        gmiret = this->m_IPCClient.Initialize(startport + i,GM_STREAM_APPLICATION_ID);
        if(gmiret == GMI_SUCCESS)
        {
            DEBUG_INFO("Initialize Port (%d)\n",startport + i);
            break;
        }
    }

    if(gmiret != GMI_SUCCESS)
    {
        return -EFAULT;
    }
    this->m_Initialized = 1;

    serverport += this->m_StreamId;
    if(this->m_Type == SYS_COMP_H264)
    {
        type = MEDIA_VIDEO_H264;
    }
    else if(this->m_Type == SYS_COMP_MJPEG)
    {
        type = MEDIA_VIDEO_MJPEG;
    }
    else if(this->m_Type == SYS_COMP_MPEG4)
    {
        type = MEDIA_VIDEO_MPEG4;
    }
    else
    {
        this->__ClearIPC();
        return -EFAULT;
    }
#ifdef STREAM_PULL_MODE
    gmiret = this->m_IPCClient.Register(serverport,type,this->m_StreamId,true,NULL,NULL);
#else
    gmiret = GMI_FAIL;
#endif
    if(gmiret != GMI_SUCCESS)
    {
        ERROR_INFO("register port %d at streamid %d error 0x%08lx\n",
                   serverport,this->m_StreamId,gmiret);
        this->__ClearIPC();
        return -EFAULT;
    }
    this->m_Registered = 1;
    return 0;
}


#endif /*VIDEO_STREAM_EMULATE*/

#ifdef VIDEO_STREAM_EMULATE

#ifdef STREAM_PULL_MODE
#define THREAD_EXIT_ASSERT() \
do\
{\
    SDK_ASSERT(this->m_ThreadRunning  == 0);\
    SDK_ASSERT(this->m_ThreadExited == 1);\
    SDK_ASSERT(this->m_pThread == NULL);\
}\
while(0)

#else

#define THREAD_EXIT_ASSERT() \
do\
{\
	;\
}\
while(0)


#endif /*STREAM_PULL_MODE*/


#else  /*VIDEO_STREAM_EMULATE*/

#ifdef STREAM_PULL_MODE
#define THREAD_EXIT_ASSERT() \
do\
{\
    SDK_ASSERT(this->m_ThreadRunning  == 0);\
    SDK_ASSERT(this->m_ThreadExited == 1);\
    SDK_ASSERT(this->m_pThread == NULL);\
    SDK_ASSERT(this->m_Initialized == 0);\
    SDK_ASSERT(this->m_Registered == 0);\
}\
while(0)

#else

#define THREAD_EXIT_ASSERT() \
do\
{\
    SDK_ASSERT(this->m_Initialized == 0);\
    SDK_ASSERT(this->m_Registered == 0);\
}\
while(0)


#endif /*STREAM_PULL_MODE*/
#endif /*VIDEO_STREAM_EMULATE*/
int SdkServerStream::PauseStream()
{
#ifdef STREAM_PULL_MODE
    GMI_RESULT gmiret;
    if(this->m_pThread==NULL)
    {
        THREAD_EXIT_ASSERT();
        return 0;
    }

    this->m_ThreadRunning = 0;
    while(this->m_ThreadExited == 0)
    {
        /*sleep for 100 milli seconds*/
        usleep(100000);
    }

    /*now exited ,so we should destroy*/
    gmiret = this->m_pThread->Destroy();
    SDK_ASSERT(gmiret == GMI_SUCCESS);
    delete this->m_pThread ;
    this->m_pThread = NULL;
#endif
#ifdef VIDEO_STREAM_EMULATE
#else
    this->__ClearIPC();
#endif /*VIDEO_STREAM_EMULATE*/
    THREAD_EXIT_ASSERT();
    return 1;
}

#define  SIZE_EQUAL_ASSERT()\
do\
{\
    if(!(this->m_DataIdx.size() <= this->m_MaxPacks || this->m_MaxPacks == 0))\
    {\
            ERROR_INFO("maxpack %d,dataidx.size %d\n",this->m_MaxPacks,this->m_DataIdx.size());\
            SDK_ASSERT(0!=0);\
    }\
    if((this->m_DataIdx.size() != this->m_DataVec.size()))\
    {\
        ERROR_INFO("dataidx.size %d datavec.size %d\n",this->m_DataIdx.size(),this->m_DataVec.size());\
        SDK_ASSERT(0!=0);\
    }\
    if(this->m_DataVec.size() != this->m_DataLen.size())\
    {\
        ERROR_INFO("datavec.size %d  datalen.size %d \n",this->m_DataVec.size(),this->m_DataLen.size());\
        SDK_ASSERT(0!=0);\
    }\
    if(this->m_DataLen.size() != this->m_DataType.size())\
    {\
        ERROR_INFO("datalen.size %d datatype.size %d \n",this->m_DataLen.size(),this->m_DataType.size());\
        SDK_ASSERT(0!=0);\
    }\
    if(this->m_DataType.size() != this->m_DataIdx.size())\
    {\
        ERROR_INFO("datatype.size %d dataidx.size %d\n",this->m_DataType.size(),this->m_DataIdx.size());\
        SDK_ASSERT(0!=0);\
    }\
    if (this->m_DataPts.size() != this->m_DataIdx.size())\
    {\
        ERROR_INFO("datapts.size %d dataidx.size %d\n",this->m_DataPts.size(),this->m_DataIdx.size());\
        SDK_ASSERT(0!=0);\
   	}\
}while(0)


void SdkServerStream::__ClearVectors()
{
    GMI_RESULT gmiret;
    gmiret = this->m_Mutex.Lock(TIMEOUT_INFINITE);
    SDK_ASSERT(gmiret ==GMI_SUCCESS);

    SIZE_EQUAL_ASSERT();
    while(this->m_DataVec.size() > 0)
    {
        void* pData=NULL;
        pData = this->m_DataVec[0];
        this->m_DataVec.erase(this->m_DataVec.begin());
        this->m_DataLen.erase(this->m_DataLen.begin());
        this->m_DataType.erase(this->m_DataType.begin());
        this->m_DataIdx.erase(this->m_DataIdx.begin());
        this->m_DataPts.erase(this->m_DataPts.begin());
        if(pData)
        {
            DEBUG_INFO("data %p\n",pData);
            free(pData);
        }
        pData = NULL;
    }

    SIZE_EQUAL_ASSERT();
    this->m_Mutex.Unlock();
    return ;
}

int SdkServerStream::StopStream()
{
    int ret;
    ret = this->PauseStream();
    this->__ClearVectors();
    return ret;
}

SdkServerStream::~SdkServerStream()
{
    this->StopStream();

    this->m_StreamId = -1;
    this->m_Type =  0;
    this->m_MaxPacks = 0;
}

int SdkServerStream::__PushStreamData(void * pData,uint32_t datalen,uint32_t datatype,uint32_t idx,uint64_t pts)
{
    GMI_RESULT gmiret;
    int ret = 0;
    void* pDiscardPtr=NULL;
    int discardidx = -1;
    unsigned int i,fidx;
    uint32_t didx;

    //DEBUG_INFO("push data idx(0x%x) type(%d) pts(0x%llx)\n",idx,datatype,pts);

    gmiret = this->m_Mutex.Lock(TIMEOUT_INFINITE);
    SDK_ASSERT(gmiret == GMI_SUCCESS);

    SIZE_EQUAL_ASSERT();

    if(this->m_DataVec.size() < this->m_MaxPacks)
    {
        ret = 1;
        this->m_DataIdx.push_back(idx);
        this->m_DataLen.push_back(datalen);
        this->m_DataVec.push_back(pData);
        this->m_DataType.push_back(datatype);
        this->m_DataPts.push_back(pts);
    }
    else if(datatype == I_FRAME_TYPE)
    {
        fidx = this->m_DataType.size();
        fidx -= 1;
        discardidx= -1;
        /*first to find the last p-frame ,if not success ,just get the oldest i-frame*/
        for(i=0; i<this->m_DataType.size(); i++)
        {
            if(this->m_DataType[fidx - i] != I_FRAME_TYPE)
            {
                discardidx = fidx - i;
                break;
            }
        }

        /*if all is i-frame ,just discard the first one*/
        if(discardidx < 0)
        {
            discardidx = 0;
        }

        pDiscardPtr = this->m_DataVec[discardidx];
        didx = this->m_DataIdx[discardidx];

        /*erase the old one*/
        this->m_DataVec.erase(this->m_DataVec.begin()+discardidx);
        this->m_DataIdx.erase(this->m_DataIdx.begin()+discardidx);
        this->m_DataType.erase(this->m_DataType.begin()+discardidx);
        this->m_DataLen.erase(this->m_DataLen.begin()+discardidx);
        this->m_DataPts.erase(this->m_DataPts.begin()+discardidx);

        this->m_DataVec.push_back(pData);
        this->m_DataIdx.push_back(idx);
        this->m_DataLen.push_back(datalen);
        this->m_DataType.push_back(datatype);
        this->m_DataPts.push_back(pts);
        ret = 1;
    }

    this->m_Mutex.Unlock();

    if(pDiscardPtr)
    {
        SDK_ASSERT(ret > 0);
        DEBUG_INFO("DISCARD 0x%08x\n",didx);
        free(pDiscardPtr);
        pDiscardPtr = NULL;
    }

    return ret;
}

int SdkServerStream::PullStreamData(stream_pack_t* pPack)
{
    GMI_RESULT gmiret;
    int ret = 0;
    gmiret = this->m_Mutex.Lock(TIMEOUT_INFINITE);
    SDK_ASSERT(gmiret == GMI_SUCCESS);

    SIZE_EQUAL_ASSERT();
    if(this->m_DataIdx.size() > 0)
    {
        pPack->m_Idx     = this->m_DataIdx[0];
        pPack->m_Type    = this->m_DataType[0];
        pPack->m_pData   = this->m_DataVec[0];
        pPack->m_DataLen = this->m_DataLen[0];
        pPack->m_Pts     = this->m_DataPts[0];

        this->m_DataIdx.erase(this->m_DataIdx.begin());
        this->m_DataType.erase(this->m_DataType.begin());
        this->m_DataVec.erase(this->m_DataVec.begin());
        this->m_DataLen.erase(this->m_DataLen.begin());
        this->m_DataPts.erase(this->m_DataPts.begin());
        ret = 1;
    }

    this->m_Mutex.Unlock();

    if(ret == 0 && this->m_ThreadExited)
    {
        ret= -EFAULT;
    }
    return ret;
}


#ifdef  STREAM_PULL_MODE

#ifdef VIDEO_STREAM_EMULATE

static unsigned char st_PFrameBuf[] = {0x0,0x0,0x0,0x1,0x9,0x30,0x0,0x0,0x0,0x1};
static unsigned char st_IFrameBuf[] = {0x0,0x0,0x0,0x1,0x9,0x10,0x0,0x0,0x0,0x1};

void* SdkServerStream::ThreadImpl()
{
    int ret;
    void* pBuf=NULL;
    uint32_t buflen=0;
    uint32_t bufsize=(1<<21);
    uint32_t type;
    uint32_t idx=0;
    uint64_t pts=0;

    while(this->m_ThreadRunning)
    {
        SDK_ASSERT(pBuf == NULL);
        SDK_ASSERT(buflen == 0);
        pBuf = malloc(bufsize);
        if(pBuf == NULL)
        {
            ret = -errno ? -errno : -1;
            ERROR_INFO("could not malloc size %d error(%d)\n",bufsize,ret);
            goto out;
        }
        type = P_FRAME_TYPE;
        if((idx % 25)==0)
        {
            type = I_FRAME_TYPE;
        }

        /*now we should set for the */
        if(type == P_FRAME_TYPE)
        {
            /*now to set for the type */
            memcpy(pBuf,st_PFrameBuf,sizeof(st_PFrameBuf));
            buflen = 0x4000;
        }
        else
        {
            memcpy(pBuf,st_IFrameBuf,sizeof(st_IFrameBuf));
            buflen = 0x80000;
        }

        ret = this->__PushStreamData(pBuf,buflen,type,idx,pts);
        if(ret <0)
        {
            ERROR_INFO("[%d] push[%d] type %d size %d pts %lld error(%d)\n",this->m_StreamId,idx,type,buflen,pts,ret);
            goto out;
        }
        else if(ret == 0)
        {
            free(pBuf);
        }
        pBuf = NULL;
        buflen = 0;

        usleep(40000);
        idx ++;
        pts += 3600;
    }

    ret = 0;
out:
    if(pBuf)
    {
        free(pBuf);
    }
    pBuf = NULL;
    this->m_ThreadExited = 1;
    return(void*) ret;

}


#else /*VIDEO_STREAM_EMULATE*/

#define  MAX_ULONGLONG     (0xffffffffffffffffULL)
#define  MAX_ULONG         (0xffffffffUL)

void* SdkServerStream::ThreadImpl()
{
    int ret;
    void* pBuf=NULL;
    uint32_t buflen=0;
    uint32_t bufsize = (1<<21);
    int infosize = 1024,inputsize;
    uint32_t type;
    uint32_t idx;
    uint64_t pts,lastidx=MAX_ULONGLONG;
    GMI_RESULT gmiret;
    std::auto_ptr<unsigned char> pExInfo2(new unsigned char[infosize]);
    unsigned char *pExInfo=pExInfo2.get();
    struct timeval tmval;
    struct timespec tmspec;
    unsigned long long lastgetpts=0,getpts;


    while(this->m_ThreadRunning)
    {
        SDK_ASSERT(pBuf == NULL);
        SDK_ASSERT(buflen == 0);

        pBuf = malloc(bufsize);
        if(pBuf ==NULL)
        {
            ret = -errno ? -errno : -1;
            goto out;
        }

        buflen = bufsize;
        inputsize= infosize;
        gmiret = this->m_IPCClient.Read(pBuf,(size_t*)&buflen,&tmval,pExInfo,(size_t*)&inputsize);
        if(gmiret==GMI_SUCCESS)
        {
            ExtMediaEncInfo* pInfo=(ExtMediaEncInfo*)pExInfo;
            type = P_FRAME_TYPE;
            idx = pInfo->s_FrameNum;

            if(lastidx != MAX_ULONGLONG)
            {
                if(lastidx != MAX_ULONG && (lastidx +1)!= idx)
                {
                    DEBUG_INFO("DISCARD lastidx 0x%08x idx 0x%08x\n",(uint32_t)lastidx,idx);
                }
                else if(lastidx == MAX_ULONG && idx != 0)
                {
                    DEBUG_INFO("DISCARD lastidx 0x%08x idx 0x%08x\n",(uint32_t)lastidx,idx);
                }
            }

            lastidx = idx;

            if(pInfo->s_FrameType == VIDEO_I_FRAME ||
                    pInfo->s_FrameType == VIDEO_IDR_FRAME)
            {
                type = I_FRAME_TYPE;
            }



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
                getpts = VFREQ_PER_SECOND(tmspec.tv_sec) + VFREQ_PER_NANOSECOND(tmspec.tv_nsec);
                if(lastgetpts != 0 && (getpts - lastgetpts) >80000)
                {
                    ERROR_INFO("type %d EXPIRE TIME getpts 0x%llx lastgetpts 0x%llx (%lld)\n",type,getpts,lastgetpts,(getpts-lastgetpts));
                }
                lastgetpts = getpts;
            }
            //DEBUG_INFO("idx (0x%x) type (%d) pts %ld:%ld pts 0x%llx frametype (%d)\n",
            //           idx,type,tmval.tv_sec,tmval.tv_usec,pts,pInfo->s_FrameType);
            if(type == I_FRAME_TYPE)
            {
                DEBUG_INFO("[%d] type %d frame idx %d pts 0x%llx size(%d) %ld:%ld\n",this->m_StreamId,pInfo->s_FrameType,idx,pts,buflen,tmspec.tv_sec,tmspec.tv_nsec);
            }
            ret = this->__PushStreamData(pBuf,buflen,type,idx,pts);
            if(ret <=0)
            {
                /*we free memory for not insert into the buffer list*/
                SDK_ASSERT(type != I_FRAME_TYPE);
                //DEBUG_INFO("[%d]insert frame idx %d failed type (%d)\n",this->m_StreamId,idx,type);
                free(pBuf);
            }
            pBuf=NULL;
            buflen = 0;
            continue;
        }
        else if(gmiret == GMI_TRY_AGAIN_ERROR)
        {
            /*nothing to read ,so we should read later*/
            free(pBuf);
            pBuf = NULL;
            buflen = 0;
            continue;
        }
        else
        {
            /*nothing to read ,so we should read later*/
            free(pBuf);
            pBuf = NULL;
            buflen = 0;
            continue;
        }

    }

    ret = 0;

out:
    if(pBuf)
    {
        free(pBuf);
    }
    pBuf = NULL;
    buflen = 0;
    this->m_ThreadExited = 1;
    return	(void*)ret;
}

#endif /*VIDEO_STREAM_EMULATE*/

void* SdkServerStream::ThreadFunc(void * arg)
{
    SdkServerStream* pThis = (SdkServerStream*)arg;

    return pThis->ThreadImpl();
}

#endif /*STREAM_PULL_MODE*/


#ifdef STREAM_PULL_MODE

int SdkServerStream::ResumeStream()
{
    GMI_RESULT gmiret;
#ifdef 	VIDEO_STREAM_EMULATE
#else
    int ret;
#endif /*VIDEO_STREAM_EMULATE*/
    this->PauseStream();
    /*now we should start thread because we used pthread to test for the exited*/
    this->m_pThread = new GMI_Thread();
    SDK_ASSERT(this->m_pThread);

#ifdef VIDEO_STREAM_EMULATE
#else
    ret = this->__InitIPC();
    if(ret < 0)
    {
        this->PauseStream();
        return ret;
    }
#endif /*VIDEO_STREAM_EMULATE*/
    this->m_ThreadRunning = 1;
    gmiret = this->m_pThread->Create("StreamThread",0,SdkServerStream::ThreadFunc,this);
    if(gmiret != GMI_SUCCESS)
    {
        this->PauseStream();
        return -EFAULT;
    }

    this->m_ThreadExited = 0;
    gmiret = this->m_pThread->Start();
    if(gmiret != GMI_SUCCESS)
    {
        /*because ,we do not start ok,so we should pretend this exited ok*/
        this->m_ThreadExited = 1;
        this->PauseStream();
        return -EFAULT;
    }
    return 0;
}

#endif


int SdkServerStream::StartStream()
{
    int ret;
    this->StopStream();
    ret = this->ResumeStream();
    if(ret < 0)
    {
        this->StopStream();
        return ret;
    }
    return 0;
}

