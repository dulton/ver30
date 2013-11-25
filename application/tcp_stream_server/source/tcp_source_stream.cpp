
#include <tcp_source_stream.h>
#include <gmi_media_ctrl.h>
#include <ipc_media_data_dispatch.h>
#include <ipc_fw_v3.x_resource.h>
#include <gmi_config_api.h>
#include <memory>

#if 1
#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%d[%ld]\t",__FILE__,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)
#else
#define DEBUG_INFO(...)
#endif
#define ERROR_INFO(...) do{fprintf(stderr,"%s:%d\t",__FILE__,__LINE__);fprintf(stderr,__VA_ARGS__);}while(0)


TcpSourceStream::TcpSourceStream(int streamid,int maxpacks) :
    m_StreamId(streamid),
    m_MaxPacks(maxpacks)
{
    GMI_RESULT gmiret;
    gmiret = m_Mutex.Create(NULL);
    assert(gmiret == GMI_SUCCESS);
    m_Initialized = 0;
    m_Registered = 0;
#ifdef STREAM_PULL_MODE
    m_ThreadRunning = 0;
    m_ThreadExited = 1;
    m_pThread = NULL;
#endif

    assert(m_DataVec.size() == 0);
    assert(m_DataLen.size() == 0);
    assert(m_DataType.size() == 0);
    assert(m_DataIdx.size() == 0);

}

#define  STOP_ASSERT()  \
do\
{\
    assert(this->m_Initialized == 0);\
    assert(this->m_Registered == 0);\
    assert(this->m_DataVec.size() ==0);\
    assert(this->m_DataLen.size() == 0);\
    assert(this->m_DataType.size() == 0);\
    assert(this->m_DataIdx.size() ==0);\
}while(0)


#define  SIZE_EQUAL_ASSERT()\
do\
{\
    if(!(this->m_DataIdx.size() <= this->m_MaxPacks || this->m_MaxPacks == 0))\
    {\
            ERROR_INFO("maxpack %d,dataidx.size %d\n",this->m_MaxPacks,this->m_DataIdx.size());\
            assert(0!=0);\
    }\
    if((this->m_DataIdx.size() != this->m_DataVec.size()))\
    {\
        ERROR_INFO("dataidx.size %d datavec.size %d\n",this->m_DataIdx.size(),this->m_DataVec.size());\
        assert(0!=0);\
    }\
    if(this->m_DataVec.size() != this->m_DataLen.size())\
    {\
        ERROR_INFO("datavec.size %d  datalen.size %d \n",this->m_DataVec.size(),this->m_DataLen.size());\
        assert(0!=0);\
    }\
    if(this->m_DataLen.size() != this->m_DataType.size())\
    {\
        ERROR_INFO("datalen.size %d datatype.size %d \n",this->m_DataLen.size(),this->m_DataType.size());\
        assert(0!=0);\
    }\
    if(this->m_DataType.size() != this->m_DataIdx.size())\
    {\
        ERROR_INFO("datatype.size %d dataidx.size %d\n",this->m_DataType.size(),this->m_DataIdx.size());\
        assert(0!=0);\
    }\
}while(0)

void TcpSourceStream::__ClearVector(void)
{
    GMI_RESULT gmiret;
    gmiret = this->m_Mutex.Lock(TIMEOUT_INFINITE);
    assert(gmiret ==GMI_SUCCESS);

    SIZE_EQUAL_ASSERT();

    while(this->m_DataVec.size() > 0)
    {
        void* pData;
        pData = this->m_DataVec[0];
        this->m_DataVec.erase(this->m_DataVec.begin());
        assert(pData);
        free(pData);
        this->m_DataLen.erase(this->m_DataLen.begin());
        this->m_DataType.erase(this->m_DataType.begin());
        this->m_DataIdx.erase(this->m_DataIdx.begin());
        SIZE_EQUAL_ASSERT();
    }

    this->m_Mutex.Unlock();
    return;
}


TcpSourceStream::~TcpSourceStream()
{
    this->Stop();
    DEBUG_INFO("release stream[%d]\n",this->m_StreamId);
    this->m_Mutex.Destroy();
    this->m_MaxPacks = 0;
    this->m_StreamId = 0;
}

int TcpSourceStream::__PushBack(void * pData,int datalen,int type,unsigned int idx)
{
    GMI_RESULT gmiret;
    int ret=-EFAULT;
    int i,findidx=-1;
    gmiret = this->m_Mutex.Lock(TIMEOUT_INFINITE);
    assert(gmiret ==GMI_SUCCESS);

    SIZE_EQUAL_ASSERT();
    if (this->m_DataIdx.size() < this->m_MaxPacks || this->m_MaxPacks == 0 || type == I_FRAME_TYPE)
    {
        if (this->m_DataIdx.size() < this->m_MaxPacks || this->m_MaxPacks == 0)
        {
            this->m_DataVec.push_back(pData);
            this->m_DataIdx.push_back(idx);
            this->m_DataLen.push_back(datalen);
            this->m_DataType.push_back(type);
        }
        else
        {
            void* peraseptr=NULL;
            /*we can not discard the i_frame packet ,so discard the most recent p-frame,if no p-frame,just discard the most oldest i-frame*/
            findidx = -1;
            assert(this->m_DataIdx.size() > 0);
            for (i=(this->m_DataIdx.size()-1); i>= 0; i--)
            {
                if (this->m_DataType[i] == P_FRAME_TYPE)
                {
                    findidx = i;
                    break;
                }
            }

            if (findidx == -1)
            {
                /*all are the i-frame ,so discard the oldest*/
                findidx = 0;
            }

            /*now to discard the idx of data*/
            assert(findidx >= 0 && findidx < (int)this->m_DataIdx.size());
            peraseptr = this->m_DataVec[findidx];
            assert(peraseptr);
            free(peraseptr);
            this->m_DataVec.erase(this->m_DataVec.begin() + findidx);
            this->m_DataIdx.erase(this->m_DataIdx.begin() + findidx);
            this->m_DataLen.erase(this->m_DataLen.begin() + findidx);
            this->m_DataType.erase(this->m_DataType.begin() + findidx);

            this->m_DataVec.push_back(pData);
            this->m_DataIdx.push_back(idx);
            this->m_DataLen.push_back(datalen);
            this->m_DataType.push_back(type);
        }
        ret = 1;
        SIZE_EQUAL_ASSERT();
    }
    this->m_Mutex.Unlock();
    return ret;
}


int TcpSourceStream::__GetServerPort()
{
    GMI_RESULT gmiret;
    FD_HANDLE xmlhd=NULL;
    int serverport;

    gmiret = GMI_XmlOpen(GMI_RESOURCE_CONFIG_FILE_NAME,&xmlhd);
    if (gmiret != GMI_SUCCESS)
    {
        goto set_default;
    }
    gmiret = GMI_XmlRead(xmlhd,MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH,
                         MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SERVER_UDP_PORT ,
                         GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1,
                         &serverport,GMI_CONFIG_READ_ONLY);
    if (gmiret != GMI_SUCCESS)
    {
        goto set_default;
    }
    GMI_XmlFileSave(xmlhd);
    xmlhd = NULL;

    return serverport;


set_default:
    if (xmlhd)
    {
        GMI_XmlFileSave(xmlhd);
    }
    xmlhd= NULL;
    return GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1;
}


int TcpSourceStream::__GetClientStartPort()
{
    int clientport;
    FD_HANDLE xmlhd=NULL;
    GMI_RESULT gmiret;

    gmiret = GMI_XmlOpen(GMI_RESOURCE_CONFIG_FILE_NAME,&xmlhd);
    if (gmiret != GMI_SUCCESS)
    {
        goto set_default;
    }
    gmiret = GMI_XmlRead(xmlhd,SDK_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH,
                         SDK_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_CLIENT_UDP_PORT,
                         GMI_STREAMING_MEDIA_SDK_ENCODE_VIDEO1,
                         &clientport,GMI_CONFIG_READ_ONLY);
    if (gmiret != GMI_SUCCESS)
    {
        goto set_default;
    }
    GMI_XmlFileSave(xmlhd);
    xmlhd = NULL;

    return clientport;

set_default:
    if (xmlhd)
    {
        GMI_XmlFileSave(xmlhd);
    }
    xmlhd= NULL;
    return GMI_STREAMING_MEDIA_SDK_ENCODE_VIDEO1;
}


#ifdef  STREAM_PUSH_MODE

int TcpSourceStream::Stop()
{
    if (this->m_Initialized == 0)
    {
        STOP_ASSERT();
        return 0;
    }

    if (this->m_Registered)
    {
        this->m_IPCClient.Unregister();
    }
    this->m_Registered = 0;

    if (this->m_Initialized)
    {
        this->m_IPCClient.Deinitialize();
    }
    this->m_Initialized = 0;

    this->__ClearVector();
    STOP_ASSERT();
    return 1;
}


int TcpSourceStream::Start()
{
    GMI_RESULT gmiret;
    int i,startport,serverport;
    this->Stop();

    startport = this->__GetClientStartPort();
    serverport = this->__GetServerPort();

    //TODO we need to get startport and server here

    /*now first to initialize*/
    for (i=0; i<10; i++)
    {
        DEBUG_INFO("\n");
        gmiret = this->m_IPCClient.Initialize(startport + i,GM_STREAM_APPLICATION_ID);
        if (gmiret == GMI_SUCCESS)
        {
            break;
        }
    }
    DEBUG_INFO("\n");

    if (gmiret != GMI_SUCCESS)
    {
        return -EFAULT;
    }
    this->m_Initialized = 1;
    DEBUG_INFO("\n");

    serverport += this->m_StreamId;
    gmiret = this->m_IPCClient.Register(serverport,MEDIA_VIDEO_H264,this->m_StreamId,false,TcpSourceStream::StreamCallBack,this);
    if (gmiret != GMI_SUCCESS)
    {
        this->Stop();
        return -EFAULT;
    }
    this->m_Registered = 1;
    DEBUG_INFO("\n");

    return 1;
}

int TcpSourceStream::__StreamCallBackImpl(uint32_t MediaType,uint32_t MediaId,const void_t * Frame,size_t FrameSize,const struct timeval * FramTS,const void_t * ExtraData,size_t ExtraDataLength)
{
    int type=P_FRAME_TYPE;
    unsigned int idx=0;
    void* pData=NULL;
    int ret = 0;
    ExtMediaEncInfo* pInfo=(ExtMediaEncInfo*)ExtraData;

    if (FrameSize > 0)
    {
        DEBUG_INFO("\n");
        pData = malloc(FrameSize);
        if (pData == NULL)
        {
            return -ENOMEM;
        }
        DEBUG_INFO("\n");

        memcpy(pData,Frame,FrameSize);
        if (pInfo->s_FrameType == VIDEO_I_FRAME)
        {
            type = I_FRAME_TYPE;
        }
        DEBUG_INFO("\n");

        idx = pInfo->s_FrameNum;

        /*TODO:we need to get the type and idx here*/
        ret = this->__PushBack(pData,FrameSize,type,idx);
        if (ret < 0)
        {
            /*we discard the packet ,this will on the next*/
            free(pData);
            DEBUG_INFO("\n");
            return 0;
        }
    }
    return ret;
}

void TcpSourceStream::StreamCallBack(void_t *UserData, uint32_t MediaType, uint32_t MediaId, const void_t *Frame, size_t FrameSize, const struct timeval *FramTS, const void_t *ExtraData, size_t ExtraDataLength )
{
    TcpSourceStream* pThis = (TcpSourceStream*)UserData;
    int ret;

    ret = pThis->__StreamCallBackImpl(MediaType,MediaId,Frame,FrameSize,FramTS,ExtraData,ExtraDataLength);
    if (ret < 0)
    {
        return;
    }
    return ;
}

#endif

#ifdef  STREAM_PULL_MODE


#define THREAD_STOP_ASSERT() \
do\
{\
    assert(this->m_pThread == NULL);\
    assert(this->m_ThreadRunning == 0);\
    assert(this->m_ThreadExited == 1);\
}while(0)

int TcpSourceStream::Start()
{
    GMI_RESULT gmiret;
    int ret;
    this->Stop();

    ret = this->__InitResource();
    if (ret < 0)
    {
        return ret;
    }

    this->m_ThreadRunning = 1;
    this->m_ThreadExited = 0;


    this->m_pThread = new GMI_Thread();
    gmiret = this->m_pThread->Create(NULL,0,TcpSourceStream::StreamPullCallBack,this);
    if (gmiret != GMI_SUCCESS)
    {
        /*we pretend exited*/
        this->m_ThreadExited = 1;
        this->Stop();
        return -EFAULT;
    }

    gmiret = this->m_pThread->Start();
    if (gmiret != GMI_SUCCESS)
    {
        /*we pretend exited*/
        this->m_ThreadExited = 1;
        this->Stop();
        return -EFAULT;
    }

    return 0;
}

int TcpSourceStream::Stop()
{
    if (this->m_pThread == NULL)
    {
        this->__ClearResource();
        THREAD_STOP_ASSERT();
        return 0;
    }

    this->m_ThreadRunning = 0;
    while(this->m_ThreadExited==0)
    {
        usleep(100000);
    }
    this->__ClearResource();

    this->m_pThread->Stop();
    this->m_pThread->Destroy();
    delete this->m_pThread;
    this->m_pThread = NULL;
    THREAD_STOP_ASSERT();
    return 1;
}

int TcpSourceStream::__InitResource(void)
{
    GMI_RESULT gmiret;
    int i,startport,serverport;
    this->__ClearResource();


    startport = this->__GetClientStartPort();
    serverport = this->__GetServerPort();

    /*now first to initialize*/
    for (i=0; i<10; i++)
    {
        gmiret = this->m_IPCClient.Initialize(startport + i,GM_STREAM_APPLICATION_ID);
        if (gmiret == GMI_SUCCESS)
        {
            break;
        }
    }

    if (gmiret != GMI_SUCCESS)
    {
        return -EFAULT;
    }
    this->m_Initialized = 1;

    serverport += this->m_StreamId;
    gmiret = this->m_IPCClient.Register(serverport,MEDIA_VIDEO_H264,this->m_StreamId,true,NULL,NULL);
    if (gmiret != GMI_SUCCESS)
    {
        this->__ClearResource();
        return -EFAULT;
    }
    this->m_Registered = 1;
    return 1;
}

int TcpSourceStream::__ClearResource(void)
{
    if (this->m_Initialized == 0)
    {
        STOP_ASSERT();
        return 0;
    }

    if (this->m_Registered)
    {
        this->m_IPCClient.Unregister();
    }
    this->m_Registered = 0;

    if (this->m_Initialized)
    {
        this->m_IPCClient.Deinitialize();
    }
    this->m_Initialized = 0;

    this->__ClearVector();
    STOP_ASSERT();
    return 1;
}

int TcpSourceStream::__StreamPullCallBackImpl()
{
    int ret;
    void* pBuf=NULL;
    int buflen=0;
    int bufsize = (1<<21);
    int infosize = 1024,inputsize;
    int type;
    unsigned int idx;
    GMI_RESULT gmiret;
    std::auto_ptr<unsigned char> pExInfo2(new unsigned char[infosize]);
    unsigned char *pExInfo=pExInfo2.get();
    struct timeval tmval;

    while(this->m_ThreadRunning)
    {
        assert(pBuf == NULL);
        assert(buflen == 0);

        pBuf = malloc(bufsize);
        if (pBuf ==NULL)
        {
            ret = -errno ? -errno : -1;
            goto out;
        }

        buflen = bufsize;
        inputsize= infosize;
        gmiret = this->m_IPCClient.Read(pBuf,(size_t*)&buflen,&tmval,pExInfo,(size_t*)&inputsize);
        if (gmiret==GMI_SUCCESS)
        {
            ExtMediaEncInfo* pInfo=(ExtMediaEncInfo*)pExInfo;
            type = P_FRAME_TYPE;
            idx = pInfo->s_FrameNum;
            if (pInfo->s_FrameType == VIDEO_I_FRAME ||
                    pInfo->s_FrameType == VIDEO_IDR_FRAME)
            {
                DEBUG_INFO("[%d] i-frame %d\n",this->m_StreamId,idx);
                type = I_FRAME_TYPE;
            }


            ret = this->__PushBack(pBuf,buflen,type,idx);
            if (ret <0)
            {
                /*we free memory for not insert into the buffer list*/
                DEBUG_INFO("[%d]insert frame idx %d failed\n",this->m_StreamId,idx);
                free(pBuf);
            }
            pBuf=NULL;
            buflen = 0;
            continue;
        }
        else if (gmiret == GMI_TRY_AGAIN_ERROR)
        {
            /*nothing to read ,so we should read later*/
            free(pBuf);
            pBuf = NULL;
            buflen = 0;
            /*sleep for a while 40 millsec*/
            usleep(40000);
            continue;
        }
        else
        {
            ret = -EFAULT;
            goto out;
        }

    }

    ret = 0;

out:
    if (pBuf)
    {
        free(pBuf);
    }
    pBuf = NULL;
    buflen = 0;
    this->m_ThreadExited = 1;
    return  ret;
}

void* TcpSourceStream::StreamPullCallBack(void * arg)
{
    int ret;
    TcpSourceStream* pThis = (TcpSourceStream*)arg;
    ret = pThis->__StreamPullCallBackImpl();
    return (void*) ret;
}

#endif



int TcpSourceStream::__GetNextStream(int * pIdx,void ** ppData,int * pSize,int * pType)
{
    GMI_RESULT gmiret;
    int ret=0;
    gmiret = this->m_Mutex.Lock(TIMEOUT_INFINITE);
    assert(gmiret == GMI_SUCCESS);

    SIZE_EQUAL_ASSERT();
    if (this->m_DataVec.size()>0)
    {
        *pIdx = this->m_DataIdx[0];
        *ppData = this->m_DataVec[0];
        *pSize = this->m_DataLen[0];
        *pType = this->m_DataType[0];
        this->m_DataIdx.erase(this->m_DataIdx.begin());
        this->m_DataVec.erase(this->m_DataVec.begin());
        this->m_DataLen.erase(this->m_DataLen.begin());
        this->m_DataType.erase(this->m_DataType.begin());
        SIZE_EQUAL_ASSERT();
        /*we have got one*/
        ret = 1;
    }
    SIZE_EQUAL_ASSERT();

    this->m_Mutex.Unlock();
    return ret;
}

int TcpSourceStream::GetNextStream(int * pIdx,void ** ppData,int * pSize,int * pType)
{
    return this->__GetNextStream(pIdx,ppData,pSize,pType);
}

int TcpSourceStream::GetSFormat(format_data_t * pFormat)
{
    pFormat->m_Type = 1;
    pFormat->m_Height = 1080;
    pFormat->m_Width = 1920;
    return 0;
}
