
#include <sdk_server_mgmt.h>
#include <sdk_server_debug.h>
#include <memory>
#include <sdk_sys_cp.h>


SdkServerMgmt::SdkServerMgmt(int * pRunningBits ,int maxclients ,int maxsessions ,int maxpacks)
    : m_pRunningBits(pRunningBits),
      m_MaxClients(maxclients),
      m_MaxSessions(maxsessions),
      m_MaxPacks(maxpacks)
{
    time_t curt;
    curt = time(NULL);
    srand(curt);
    /*we init once ,for it will give the random number ,and will not down*/
    m_ReqNum = (rand() & MAX_REQNUM);
    m_ExpireTime = 900 ;
    m_KeepTime = 15;
    SDK_ASSERT(m_pSessions.size() == 0);
    SDK_ASSERT(m_pClientLogins.size() == 0);
    SDK_ASSERT(m_pClientConfs.size() == 0);
    SDK_ASSERT(m_pClientStreams.size() == 0);
    SDK_ASSERT(m_WaitingRemoveSessionId.size() == 0);

    m_InsertRemoveStreamsTimer = 0;
    m_InsertRemoveConfTimer = 0;
    m_InsertReleaseDBufferTimer = 0;
    SDK_ASSERT(m_WillRemoveBuffers.size() == 0);
    SDK_ASSERT(m_WillRemoveClients.size() == 0);
    m_pWillRemoveAudioBuffer = NULL;
    m_pWillRemoveDBuffer = NULL;

    memset(m_pStreamBuffers,0,sizeof(m_pStreamBuffers));
    m_pAudioBuffer = NULL;
    m_pDBuffer = NULL;
    m_DBufferEnding = 0;
    memset(&(m_AudioDecParam),0,sizeof(m_AudioDecParam));
    m_pSysReq = NULL;
    m_pSysComm = NULL;
    m_pLoginHandler = NULL;
    m_pDaemon = NULL;
    m_pStreamInfo = NULL;
    /*not started*/
    m_StreamStarted = 0;
    m_InsertPullTimer = 0;
#ifdef AUDIO_DUAL_FILE_EMULATE
    m_InsertStartAudioDualFileTimer = 0;
    m_InsertStopAudioDualFileTimer = 0;
    m_StartSeqId = 0;
    m_StopSeqId = 0;
#endif
}

SdkServerMgmt::~SdkServerMgmt()
{
    this->Stop();
    this->m_pRunningBits = NULL;
    this->m_MaxClients = 0;
    this->m_MaxSessions = 0;
    this->m_MaxPacks = 0;
    this->m_ReqNum = 0;
    this->m_ExpireTime = 0;
    this->m_KeepTime = 0;
}


void SdkServerMgmt::__ReleaseClientConfs()
{
    while(this->m_pClientConfs.size() > 0)
    {
        SdkServerClient* pClient=this->m_pClientConfs[0];
        this->m_pClientConfs.erase(this->m_pClientConfs.begin());
        delete pClient;
    }
}

void SdkServerMgmt::__ReleaseClientLogins()
{
    while(this->m_pClientLogins.size() > 0)
    {
        SdkServerClient* pClient=this->m_pClientLogins[0];
        this->m_pClientLogins.erase(this->m_pClientLogins.begin());
        delete pClient;
    }
}

void SdkServerMgmt::__ReleaseClientStreams()
{
    while(this->m_pClientStreams.size() > 0)
    {
        SdkServerClient* pClient=this->m_pClientStreams[0];
        this->m_pClientStreams.erase(this->m_pClientStreams.begin());
        delete pClient;
    }
    return ;
}


void SdkServerMgmt::__ReleaseAllSessions()
{
    while(this->m_pSessions.size() > 0)
    {
        SdkServerSession* pSession = this->m_pSessions[0];
        this->m_pSessions.erase(this->m_pSessions.begin());
        /*we call the release clients before ,so this must be zero*/
        SDK_ASSERT(pSession->Clients() == 0);
        delete pSession;
    }

    return ;
}

void SdkServerMgmt::__ReleaseAllRemoveSessionIds()
{
    this->m_WaitingRemoveSessionId.clear();
}

int SdkServerMgmt::__IncReqNum()
{
    this->m_ReqNum += 1;
    if(this->m_ReqNum > MAX_REQNUM)
    {
        this->m_ReqNum = 0;
    }
    return this->m_ReqNum;
}

void SdkServerMgmt::__StopRemoveStreamTimer()
{
    if(this->m_InsertRemoveStreamsTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvRemoveStreamsTimer));
    }
    this->m_InsertRemoveStreamsTimer = 0;
    return ;
}

int SdkServerMgmt::__StartRemoveStreamTimer()
{
    if(this->m_InsertRemoveStreamsTimer == 0)
    {
        ev_timer_init(&(this->m_EvRemoveStreamsTimer),SdkServerMgmt::RemoveStreamTimerCallBack,0.1,0.0,this);
        ev_timer_start(EV_DEFAULT,&(this->m_EvRemoveStreamsTimer));
    }
    this->m_InsertRemoveStreamsTimer = 1;
    return 0;
}

void SdkServerMgmt::__StopRemoveConfTimer()
{
    if(this->m_InsertRemoveConfTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvRemoveConfigTimer));
    }
    this->m_InsertRemoveConfTimer = 0;
    return ;
}

int SdkServerMgmt::__StartRemoveConfTimer()
{
    if(this->m_InsertRemoveConfTimer == 0)
    {
        ev_timer_init(&(this->m_EvRemoveConfigTimer),SdkServerMgmt::RemoveConfTimerCallBack,0.1,0.0,this);
        ev_timer_start(EV_DEFAULT,&(this->m_EvRemoveConfigTimer));
    }
    this->m_InsertRemoveConfTimer= 1;
    return 0;
}

int SdkServerMgmt::__RemoveStreamTimerImpl()
{
    unsigned int i,j,k;
    SdkServerClient *pWillRemove,*pCurCli;
    SdkServerBuffer *pWillBuffer,*pCurBuf;
    int findidx;
    std::vector<int> clisocks;
    std::vector<int> clearsocks;
    int index;
    unsigned int numsocks;

    /*now first to test whether the clients*/
    for(i=0; i<this->m_WillRemoveClients.size() ; i++)
    {
        pWillRemove = this->m_WillRemoveClients[i];
        findidx = -1;
        for(j=0; j<this->m_pClientStreams.size(); j++)
        {
            pCurCli = this->m_pClientStreams[j];
            if(pWillRemove == pCurCli)
            {
                findidx = j;
                break;
            }
        }
        if(findidx < 0)
        {
            ERROR_INFO("could not find [%d] %p client\n",i,pWillRemove);
            continue;
        }

        /*now remove this one*/
        this->m_pClientStreams.erase(this->m_pClientStreams.begin() + findidx);
        delete pWillRemove;
    }

    this->m_WillRemoveClients.clear();

    /*now we should remove the buffers*/
    for(i=0; i<this->m_WillRemoveBuffers.size(); i++)
    {
        findidx = -1;
        pWillBuffer = this->m_WillRemoveBuffers[i];
        for(j=0; j<MAX_STREAM_IDS; j++)
        {
            pCurBuf = this->m_pStreamBuffers[j];
            if(pCurBuf && pCurBuf == pWillBuffer)
            {
                findidx = j;
                break;
            }
        }

        if(findidx < 0)
        {
            ERROR_INFO("could not find [%d] buffer %p\n",i,pWillBuffer);
            continue;
        }

        /*
        	this will disturbing ,when we clear the buffer ,other clients will be affected ,but it is ok ,when they call
        	GetStreamData  will return error code ,this will let them done so
               */
        clisocks.clear();
        numsocks = pWillBuffer->GetClients(clisocks);
        for(j=0; j<numsocks; j++)
        {
            index = -1;
            for(k=0; k<clearsocks.size(); k++)
            {
                if(clisocks[j] == clearsocks[k])
                {
                    index = k;
                    break;
                }
            }
            if(index >= 0)
            {
                continue;
            }

            clearsocks.push_back(clisocks[j]);
        }
        /*we do not release the buffer here ,worried about the clients attach for it*/
    }

    if(this->m_pWillRemoveAudioBuffer)
    {
        /*this will clear all the buffers*/
        if(this->m_pWillRemoveAudioBuffer ==
                this->m_pAudioBuffer)
        {
            clisocks.clear();
            numsocks = this->m_pWillRemoveAudioBuffer->GetClients(clisocks);
            for(j=0; j<numsocks; j++)
            {
                index = -1;
                for(k=0; k<clearsocks.size(); k++)
                {
                    if(clisocks[j] == clearsocks[k])
                    {
                        index = k;
                        break;
                    }
                }
                if(index >= 0)
                {
                    continue;
                }

                clearsocks.push_back(clisocks[j]);
            }
        }
        else
        {
            ERROR_INFO("fixup bug for Remove AudioBuffer not equal AudioBuffer now\n");
            /*set for null and we will not remove it*/
            this->m_pWillRemoveAudioBuffer = NULL;
        }
    }


#if 0
    /*************************************************
    	not clear all the clients ,because one reason
        1, one client will open multi video stream ,it will
           remove one ,so it still ok
        2, one client will open video and audio ,it will open audio
           failed, but it can still send audio to remote
        so we do not delete client stream
    *************************************************/
    for(i=0; i<clearsocks.size(); i++)
    {
        pWillRemove = NULL;
        findidx = -1;
        for(j=0; j<this->m_pClientStreams.size(); j++)
        {
            pCurCli = this->m_pClientStreams[j];
            if(pCurCli->GetSocket() == clearsocks[i])
            {
                pWillRemove = pCurCli;
                findidx = j;
                break;
            }
        }

        if(findidx < 0)
        {
            ERROR_INFO("no %d socket client for stream\n",clearsocks[i]);
            continue;
        }
        this->m_pClientStreams.erase(this->m_pClientStreams.begin() + findidx);
        delete pWillRemove;
    }
#endif
    /*now we should remove the buffers*/
    for(i=0; i<this->m_WillRemoveBuffers.size(); i++)
    {
        findidx = -1;
        pWillBuffer = this->m_WillRemoveBuffers[i];
        for(j=0; j<MAX_STREAM_IDS; j++)
        {
            pCurBuf = this->m_pStreamBuffers[j];
            if(pCurBuf && pCurBuf == pWillBuffer)
            {
                findidx = j;
                break;
            }
        }

        if(findidx < 0)
        {
            ERROR_INFO("could not find [%d] buffer %p\n",i,pWillBuffer);
            continue;
        }

        //clisocks.clear();
        //numsocks = pWillBuffer->GetClients(clisocks);
        /*we delete the clients before ,so we can do this*/
        //SDK_ASSERT(numsocks == 0);

        delete pWillBuffer;
        this->m_pStreamBuffers[findidx] = NULL;
    }

    if(this->m_pWillRemoveAudioBuffer && this->m_pWillRemoveAudioBuffer == this->m_pAudioBuffer)
    {
        delete this->m_pAudioBuffer;
    }
    this->m_pAudioBuffer = NULL;
    this->m_pWillRemoveAudioBuffer = NULL;

    if(this->m_pWillRemoveDBuffer && this->m_pWillRemoveDBuffer == this->m_pDBuffer)
    {
        /*we clear directly ,so no recursive error would appear*/
        this->__DirectReleaseDBuffer();
    }
    this->m_pWillRemoveDBuffer = NULL;

    clisocks.clear();
    clearsocks.clear();

    if(this->m_pClientStreams.size() == 0)
    {
        index  = -1;
        for(i=0; i<MAX_STREAM_IDS; i++)
        {
            if(this->m_pStreamBuffers[i])
            {
                index = i;
                break;
            }
        }

        if(this->m_pAudioBuffer)
        {
            index = AUDIO_STREAM_ID;
        }

        if(this->m_pDBuffer && this->m_DBufferEnding == 0)
        {
            index = AUDIO_DECODE_ID;
        }



        if(index < 0)
        {
            this->__CopySysStreamInfo(NULL);
            this->m_StreamStarted = 0;
        }
        else
        {
            ERROR_INFO("clients none ,but stream buffer still [%d]\n",index);
        }
    }
    return 0;
}

void SdkServerMgmt::RemoveStreamTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkServerMgmt* pThis =(SdkServerMgmt*)arg;
    int ret;
    /*put here ,because in the later ,it will recall start remove stream timer */
    pThis->__StopRemoveStreamTimer();
    ret = pThis->__RemoveStreamTimerImpl();
    if(ret < 0)
    {
        pThis->__BreakOut();
    }
    return ;
}

int SdkServerMgmt::__RemoveConfTimerImpl()
{
    this->__ReleaseClientConfs();
    return 0;
}

void SdkServerMgmt::RemoveConfTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkServerMgmt* pThis =(SdkServerMgmt*)arg;
    int ret;
    ret = pThis->__RemoveConfTimerImpl();
    if(ret < 0)
    {
        pThis->__BreakOut();
    }
    pThis->__StopRemoveConfTimer();
    return ;
}

#ifdef AUDIO_DUAL_FILE_EMULATE

#define  STOP_ALL_RELEASE_ASSERT() \
do\
{\
    unsigned int __i;\
    unsigned char* __ptr;\
    SDK_ASSERT(this->m_pSessions.size() ==0);\
    SDK_ASSERT(this->m_pClientLogins.size() == 0);\
    SDK_ASSERT(this->m_pClientConfs.size() == 0);\
    SDK_ASSERT(this->m_pClientStreams.size() == 0);\
    SDK_ASSERT(this->m_WaitingRemoveSessionId.size() == 0);\
	SDK_ASSERT(this->m_InsertRemoveStreamsTimer == 0);\
	SDK_ASSERT(this->m_InsertRemoveConfTimer == 0);\
	SDK_ASSERT(this->m_InsertReleaseDBufferTimer == 0);\
	SDK_ASSERT(this->m_WillRemoveClients.size() == 0);\
	SDK_ASSERT(this->m_WillRemoveBuffers.size ()==0);\
	SDK_ASSERT(this->m_pWillRemoveAudioBuffer == NULL);\
	SDK_ASSERT(this->m_pWillRemoveDBuffer == NULL);\
	__ptr = (unsigned char*) &(this->m_AudioDecParam);\
	for (__i=0;__i < sizeof(this->m_AudioDecParam);__i++)\
	{\
		SDK_ASSERT(__ptr[__i] == 0x0);\
	}\
	for (__i=0; __i < MAX_STREAM_IDS; __i ++)\
    {\
    	SDK_ASSERT(this->m_pStreamBuffers[__i] == NULL);\
    }\
    SDK_ASSERT(this->m_pAudioBuffer == NULL);\
	SDK_ASSERT(this->m_DBufferEnding == 0);\
    SDK_ASSERT(this->m_pDBuffer == NULL);\
	SDK_ASSERT(this->m_pSysReq == NULL);\
	SDK_ASSERT(this->m_pSysComm == NULL);\
	SDK_ASSERT(this->m_pLoginHandler == NULL);\
	SDK_ASSERT(this->m_pStreamInfo == NULL);\
	SDK_ASSERT(this->m_StreamStarted == 0);\
	SDK_ASSERT(this->m_InsertPullTimer == 0);\
	SDK_ASSERT(this->m_InsertStartAudioDualFileTimer == 0);\
	SDK_ASSERT(this->m_InsertStopAudioDualFileTimer == 0);\
	SDK_ASSERT(this->m_StartSeqId == 0);\
	SDK_ASSERT(this->m_StopSeqId == 0);\
}\
while(0)


#else

#define  STOP_ALL_RELEASE_ASSERT() \
do\
{\
    unsigned int __i;\
    unsigned char* __ptr;\
    SDK_ASSERT(this->m_pSessions.size() ==0);\
    SDK_ASSERT(this->m_pClientLogins.size() == 0);\
    SDK_ASSERT(this->m_pClientConfs.size() == 0);\
    SDK_ASSERT(this->m_pClientStreams.size() == 0);\
    SDK_ASSERT(this->m_WaitingRemoveSessionId.size() == 0);\
	SDK_ASSERT(this->m_InsertRemoveStreamsTimer == 0);\
	SDK_ASSERT(this->m_InsertRemoveConfTimer == 0);\
	SDK_ASSERT(this->m_InsertReleaseDBufferTimer == 0);\
	SDK_ASSERT(this->m_WillRemoveClients.size() == 0);\
	SDK_ASSERT(this->m_WillRemoveBuffers.size ()==0);\
	SDK_ASSERT(this->m_pWillRemoveAudioBuffer == NULL);\
	SDK_ASSERT(this->m_pWillRemoveDBuffer == NULL);\
	__ptr = (unsigned char*) &(this->m_AudioDecParam);\
	for (__i=0;__i < sizeof(this->m_AudioDecParam);__i++)\
	{\
		SDK_ASSERT(__ptr[__i] == 0x0);\
	}\
	for (__i=0; __i < MAX_STREAM_IDS; __i ++)\
    {\
    	SDK_ASSERT(this->m_pStreamBuffers[__i] == NULL);\
    }\
    SDK_ASSERT(this->m_pAudioBuffer == NULL);\
	SDK_ASSERT(this->m_DBufferEnding == 0);\
    SDK_ASSERT(this->m_pDBuffer == NULL);\
	SDK_ASSERT(this->m_pSysReq == NULL);\
	SDK_ASSERT(this->m_pSysComm == NULL);\
	SDK_ASSERT(this->m_pLoginHandler == NULL);\
	SDK_ASSERT(this->m_pStreamInfo == NULL);\
	SDK_ASSERT(this->m_StreamStarted == 0);\
	SDK_ASSERT(this->m_InsertPullTimer == 0);\
}\
while(0)

#endif /*AUDIO_DUAL_FILE_EMULATE*/

void SdkServerMgmt::Stop()
{
    unsigned int i;
    int res;
    BACK_TRACE_FMT("call stop");
    /*first we should delete all client sessions*/
    this->__ReleaseClientConfs();
    this->__ReleaseClientLogins();
    this->__ReleaseClientStreams();

    this->__ReleaseAllSessions();
    this->__ReleaseAllRemoveSessionIds();

    this->__StopReleaseDBufferTimer();
    this->__StopRemoveConfTimer();
    this->__StopRemoveStreamTimer();
    this->m_WillRemoveClients.clear();
    this->m_WillRemoveBuffers.clear();
    this->m_pWillRemoveAudioBuffer = NULL;
    this->m_pWillRemoveDBuffer = NULL;

    for(i=0; i<MAX_STREAM_IDS; i++)
    {
        /*because ,we have release all the client ,so we can release the stream buffer*/
        SdkServerBuffer* pBuffer=this->m_pStreamBuffers[i];
        if(pBuffer)
        {
            delete pBuffer;
        }
        this->m_pStreamBuffers[i] = NULL;
    }

    this->__DirectReleaseDBuffer();

    if(this->m_pAudioBuffer)
    {
        delete this->m_pAudioBuffer;
    }
    this->m_pAudioBuffer = NULL;

    if(this->m_pSysReq)
    {
        delete this->m_pSysReq;
    }
    this->m_pSysReq = NULL;

    if(this->m_pSysComm)
    {
        delete this->m_pSysComm;
    }
    this->m_pSysComm = NULL;

    if(this->m_pLoginHandler)
    {
        res = this->m_pLoginHandler->UserLogoutClear(this->__IncReqNum(),3);
        if(res < 0)
        {
            ERROR_INFO("can not remove all users %d\n",res);
        }
        delete this->m_pLoginHandler;
    }
    this->m_pLoginHandler = NULL;

    if(this->m_pDaemon)
    {
        delete this->m_pDaemon;
    }
    this->m_pDaemon = NULL;


    this->__CopySysStreamInfo(NULL);

    this->__StopPullTimer();
    this->m_StreamStarted = 0;
    STOP_ALL_RELEASE_ASSERT();
    return ;
}

int SdkServerMgmt::__CopySysStreamInfo(sys_stream_info_t * pSysStreamInfo)
{
    if(pSysStreamInfo==NULL)
    {
        if(this->m_pStreamInfo)
        {
            free(this->m_pStreamInfo);
        }
        this->m_pStreamInfo = NULL;
        return 0;
    }
    if(this->m_pStreamInfo == NULL)
    {
        this->m_pStreamInfo = (sys_stream_info_t*)calloc(sizeof(*pSysStreamInfo),1);
    }
    if(this->m_pStreamInfo == NULL)
    {
        return -ENOMEM;
    }

    memcpy(this->m_pStreamInfo,pSysStreamInfo,sizeof(*pSysStreamInfo));
    DEBUG_INFO("audio count 0x%p %d\n",this->m_pStreamInfo,this->m_pStreamInfo->m_AudioCount);
    return 0;
}

int SdkServerMgmt::Start()
{
    int ret;
    this->Stop();

    if(this->m_MaxClients == 0 ||
            this->m_MaxSessions == 0 ||
            this->m_MaxPacks == 0)
    {
        ERROR_INFO("\n");
        return -EINVAL;
    }

    /*now first to start sys request ,this will give ok*/
    this->m_pSysReq = new SdkServerSysReq(this,this->m_pRunningBits);
    SDK_ASSERT(this->m_pSysReq);

    ret = this->m_pSysReq->Start();
    if(ret < 0)
    {
        ERROR_INFO("\n");
        this->Stop();
        return ret;
    }

    this->m_pSysComm = new SdkServerSysComm(this,this->m_pRunningBits);
    SDK_ASSERT(this->m_pSysComm);
    ret = this->m_pSysComm->Start();
    if(ret < 0)
    {
        ERROR_INFO("\n");
        this->Stop();
        return ret;
    }

    this->m_pLoginHandler = new SdkClientLogin(this,this->m_MaxClients,this->m_pRunningBits);
    SDK_ASSERT(this->m_pLoginHandler);

    /*first to set the sessions ok ,if not just delete this one ,and we will not let it ok*/
    ret = this->m_pLoginHandler->Start();
    if(ret >= 0)
    {
        ret = this->m_pLoginHandler->UserLogoutClear(this->__IncReqNum(),3);
        if(ret < 0)
        {
            delete this->m_pLoginHandler;
            this->m_pLoginHandler = NULL;
        }
    }
    else
    {
        delete this->m_pLoginHandler ;
        this->m_pLoginHandler = NULL;
    }


    /*no daemon to do this ,because if the login handler failed, it will affect the daemon timeout ,so we should init in the freqtime */

    /*
       first start ,we do not need any streamstarted == 0 ,we must set it not started ,but
       as the not started ,so when the sys_server call the startallstreams and set the
       */
    return 0;
}

int SdkServerMgmt::__GarbageCollectSessions(int num)
{
    int removed=0,releaseone=0;
    unsigned int i;
    SdkServerSession* pSession;
    int ret,res;
    sessionid_t sesid;
    privledge_t priv;

    /*first we just garbage not valid and 0 clients one*/
    do
    {
        releaseone = 0;
        for(i=0; i<this->m_pSessions.size(); i++)
        {
            pSession = this->m_pSessions[i];
            pSession = this->m_pSessions[i];
            ret = pSession->Clients();
            if(ret == 0 && pSession->IsValid() == 0)
            {
                /*it not in the use ,just delete this*/
                pSession->GetSessionId(sesid,priv);
                if(this->m_pLoginHandler)
                {
                    DEBUG_INFO("Logout Session %d\n",sesid);
                    res = this->m_pLoginHandler->AddUserLogoutRequest(
                              NULL,this->__IncReqNum(),
                              sesid,priv);
                    if(res < 0)
                    {
                        ERROR_INFO("could not add session 0x%08x priv 0x%08x value\n",sesid,priv);
                        /*yes this is really a big problem ,so we should do this break out*/
                        this->__BreakOut();
                    }
                    else
                    {
                        /*
                        			we push the remove session id ,for it will check for user info compare in the
                        			UserQueryLoginCallBack function.
                                            */
                        this->m_WaitingRemoveSessionId.push_back(sesid);
                    }
                }
                else
                {
                    ERROR_INFO("no auth server\n");
                }
                this->m_pSessions.erase(this->m_pSessions.begin() + i);
                delete pSession;
                releaseone = 1;
                removed ++;
                break;
            }
        }
    }
    while(releaseone && removed < num);

    if(removed >= num)
    {
        return removed;
    }

    /*we enforce garbage collection ,if it is zero clients ,just removed*/
    do
    {
        releaseone = 0;
        for(i=0; i<this->m_pSessions.size(); i++)
        {
            pSession = this->m_pSessions[i];
            ret = pSession->Clients();
            if(ret == 0)
            {
                /*it not in the use ,just delete this*/
                pSession->GetSessionId(sesid,priv);
                if(this->m_pLoginHandler)
                {
                    DEBUG_INFO("Logout Session %d ret(%d)\n",sesid,ret);
                    res = this->m_pLoginHandler->AddUserLogoutRequest(
                              NULL,this->__IncReqNum(),
                              sesid,priv);
                    if(res < 0)
                    {
                        ERROR_INFO("could not add session 0x%08x priv 0x%08x value\n",sesid,priv);
                        /*yes this is really a big problem ,so we should do this break out*/
                        this->__BreakOut();
                    }
                    else
                    {
                        /*
                        			we push the remove session id ,for it will check for user info compare in the
                        			UserQueryLoginCallBack function.
                        					*/
                        this->m_WaitingRemoveSessionId.push_back(sesid);
                    }
                }
                else
                {
                    ERROR_INFO("no auth server\n");
                }
                this->m_pSessions.erase(this->m_pSessions.begin() + i);
                delete pSession;
                releaseone = 1;
                removed ++;
                break;
            }
        }
    }
    while(releaseone && removed < num);

    return removed;

}


int SdkServerMgmt::RegisterSdkClient(SdkServerClient * pClient)
{
    int clients;

    if(this->m_InsertRemoveConfTimer ||
            this->m_InsertRemoveStreamsTimer)
    {
        /*
                   this means we need to remove the clients for conf or stream now ,but it has not recover,so it refuse for adding the
                   clients
        */
        return -EPERM;
    }

    /*now to test whether clients is more than we can serve*/
    clients = 0;
    clients += this->m_pClientConfs.size();
    DEBUG_INFO("conf %d\n",clients);
    clients += this->m_pClientLogins.size();
    DEBUG_INFO("login after %d\n",clients);
    clients += this->m_pClientStreams.size();
    DEBUG_INFO("all %d\n",clients);
    DEBUG_INFO("session id %d\n",this->m_pSessions.size());
    if(clients >= this->m_MaxClients)
    {
        return -ENOSPC;
    }

    if((this->m_pSessions.size() >= this->m_MaxSessions) ||
            (this->m_MaxSessions > 10 && (this->m_pSessions.size() >= (this->m_MaxSessions - 4))))
    {
        this->__GarbageCollectSessions(1);
        if(this->m_pSessions.size() >= this->m_MaxSessions)
        {
            return -ENOSPC;
        }
    }

    this->m_pClientLogins.push_back(pClient);
    return 0;
}

#define ASSERT_IN_RANGE_CLIENTS(pClient,vec)\
do\
{\
    unsigned int __i;\
    SdkServerClient *__pCur,*__pFind=NULL;\
    for(__i; __i < vec.size(); __i ++)\
    {\
        __pCur = vec[__i];\
        if (__pCur == (pClient))\
        {\
            __pFind = (pClient);\
            break;\
        }\
    }\
    SDK_ASSERT(__pFind);\
}\
while(0)

#define ASSERT_IN_LOGIN_CLIENTS(pClient)  ASSERT_IN_RANGE_CLIENTS(pClient,this->m_pClientLogins)




#define ASSERT_IN_RANGE_CLIENT_AND_REMOVE(pClient,vec) \
do\
{\
    unsigned int __i;\
    int __findidx = -1;\
    SdkServerClient *__pCur,*__pFind=NULL;\
    for(__i = 0; __i < vec.size(); __i ++)\
    {\
        __pCur = vec[__i];\
        if (__pCur == (pClient))\
        {\
            __pFind = (pClient);\
            __findidx = __i;\
            break;\
        }\
    }\
    SDK_ASSERT(__pFind);\
    SDK_ASSERT(__findidx >= 0);\
    vec.erase(vec.begin()+__findidx);\
}\
while(0)

#define ASSERT_IN_LOGIN_CLIENTS_AND_REMOVE(pClient) ASSERT_IN_RANGE_CLIENT_AND_REMOVE(pClient,this->m_pClientLogins)


int SdkServerMgmt::ChangeClientConf(SdkServerClient * pClient)
{
    /*now we should change into the conf */
    ASSERT_IN_LOGIN_CLIENTS_AND_REMOVE(pClient);
    this->m_pClientConfs.push_back(pClient);
    return 0;
}


int SdkServerMgmt::ChangeClientStream(SdkServerClient * pClient)
{
    /*now we should change into the conf */
    ASSERT_IN_LOGIN_CLIENTS_AND_REMOVE(pClient);
    this->m_pClientStreams.push_back(pClient);
    return 0;
}

#define ASSERT_NOT_IN_RANGE(pClient,vec) \
do\
{\
    unsigned int __i;\
    SdkServerClient *__pCur,*__pFind=NULL;\
    for(__i=0; __i < vec.size(); __i ++)\
    {\
        __pCur = vec[__i];\
        if (__pCur == (pClient))\
        {\
            __pFind = (pClient);\
            break;\
        }\
    }\
    SDK_ASSERT(__pFind==NULL);\
}\
while(0)


void SdkServerMgmt::__RemoveClientVectors(SdkServerClient * pClient)
{
    unsigned int i;
    SdkServerClient *pCurCli;
    /*now to search for client */
    for(i=0; i<this->m_pClientLogins.size(); i++)
    {
        pCurCli = this->m_pClientLogins[i];
        if(pCurCli == pClient)
        {
            this->m_pClientLogins.erase(this->m_pClientLogins.begin()+i);
            ASSERT_NOT_IN_RANGE(pClient,this->m_pClientConfs);
            ASSERT_NOT_IN_RANGE(pClient,this->m_pClientStreams);
            return;
        }
    }

    for(i=0; i<this->m_pClientConfs.size(); i++)
    {
        pCurCli = this->m_pClientConfs[i];
        if(pCurCli == pClient)
        {
            this->m_pClientConfs.erase(this->m_pClientConfs.begin()+i);
            ASSERT_NOT_IN_RANGE(pClient,this->m_pClientLogins);
            ASSERT_NOT_IN_RANGE(pClient,this->m_pClientStreams);
            return;
        }
    }


    for(i=0; i<this->m_pClientStreams.size(); i++)
    {
        pCurCli = this->m_pClientStreams[i];
        if(pCurCli == pClient)
        {
            this->m_pClientStreams.erase(this->m_pClientStreams.begin()+i);
            ASSERT_NOT_IN_RANGE(pClient,this->m_pClientLogins);
            ASSERT_NOT_IN_RANGE(pClient,this->m_pClientConfs);
            return;
        }
    }

    //ERROR_INFO("we could not get here for client %d\n",pClient->GetSocket());
}

void SdkServerMgmt::__RemoveClientSessions(SdkServerClient * pClient)
{
    unsigned int i,releaseone;
    SdkServerSession *pSession;
    int ret,res;
    sessionid_t sesid;
    privledge_t priv;
    int hascleared=0;

    for(i=0; i<this->m_pSessions.size(); i++)
    {
        pSession = this->m_pSessions[i];
        ret = pSession->UnRegisterServerSession(pClient);
        SDK_ASSERT((ret >0 && hascleared == 0)|| (ret <= 0));
        if(ret > 0)
        {
            hascleared = 1;
        }
    }

    do
    {
        releaseone = 0;
        for(i=0; i<this->m_pSessions.size(); i++)
        {
            pSession = this->m_pSessions[i];
            ret = pSession->Clients();
            if(ret == 0 && pSession->IsValid() == 0)
            {
                /*it not in the use ,just delete this*/
                pSession->GetSessionId(sesid,priv);
                if(this->m_pLoginHandler)
                {
                    res = this->m_pLoginHandler->AddUserLogoutRequest(
                              NULL,this->__IncReqNum(),
                              sesid,priv);
                    if(res < 0)
                    {
                        ERROR_INFO("could not add session 0x%08x priv 0x%08x value\n",sesid,priv);
                        /*yes this is really a big problem ,so we should do this break out*/
                        this->__BreakOut();
                    }
                    else
                    {
                        /*
                        			we push the remove session id ,for it will check for user info compare in the
                        			UserQueryLoginCallBack function.
                                            */
                        this->m_WaitingRemoveSessionId.push_back(sesid);
                    }
                }
                else
                {
                    ERROR_INFO("no auth server\n");
                }
                this->m_pSessions.erase(this->m_pSessions.begin() + i);
                delete pSession;
                releaseone = 1;
                break;
            }
        }

    }
    while(releaseone);

    return ;
}

void SdkServerMgmt::UnRegisterSdkClient(SdkServerClient * pClient)
{
    std::vector<int> clisocks;
    int numsocks;
    unsigned int i;
    int ret;
    /*first we should remove the stream id*/
    for(i=0; i<MAX_STREAM_IDS; i++)
    {
        SdkServerBuffer* pBuffer= this->m_pStreamBuffers[i];
        if(pBuffer)
        {
            pBuffer->UnRegisterSock(pClient->GetSocket());
            clisocks.clear();
            numsocks = pBuffer->GetClients(clisocks);
            if(numsocks == 0)
            {
                /*no clients ,so we delete the buffer for it ok*/
                delete pBuffer;
                this->m_pStreamBuffers[i] = NULL;
            }
        }
    }

    if(this->m_pAudioBuffer)
    {
        this->m_pAudioBuffer->UnRegisterSock(pClient->GetSocket());
        clisocks.clear();
        numsocks = this->m_pAudioBuffer->GetClients(clisocks);
        if(numsocks == 0)
        {
            /*no clients ,so we should delete this audio buffer*/
            delete this->m_pAudioBuffer;
            this->m_pAudioBuffer = NULL;
        }
    }

    if(this->m_pDBuffer)
    {
        this->m_pDBuffer->UnregisterSock(pClient->GetSocket());
        clisocks.clear();
        numsocks = this->m_pDBuffer->GetClients(clisocks);
        if(numsocks == 0 && this->m_DBufferEnding == 0)
        {
            DEBUG_INFO("release dbuffer\n");
            ret = this->__StartRequestReleaseDBuffer(SDK_SERVER_PRIVATE_SESID);
            if(ret < 0)
            {
                ERROR_INFO("could not start rquest releaseDBuffer(%d)\n",ret);
                this->__BreakOut();
            }
            else if(ret == 0)
            {
                this->__DirectReleaseDBuffer();
            }
        }
    }

    this->__RemoveClientVectors(pClient);
    this->__RemoveClientSessions(pClient);
    return ;
}

int SdkServerMgmt::__HandleInnerSysReq(sdk_client_comm_t * & pComm)
{
    std::auto_ptr<SdkSysCp> pSysCp2(new SdkSysCp());
    SdkSysCp *pSysCp=pSysCp2.get();
    uint32_t code;
    /*now we should parse the */
    int ret;

    ret = pSysCp->ParsePkg(pComm);
    if(ret < 0)
    {
        return ret;
    }

    /*now to get the code*/
    ret = pSysCp->GetCode(code);
    if(ret < 0)
    {
        return ret;
    }

    switch(code)
    {
    case SYSCODE_START_AUDIO_DECODE_RSP:
        ret = this->HandleAudioStartCallBack(pComm);
        break;
    case SYSCODE_STOP_AUDIO_DECODE_RSP:
        ret = this->HandleAudioStopCallBack(pComm);
        break;
    default:
        ERROR_INFO("could not handle code[%d]\n",code);
        ret = -ENOTSUP;
        break;
    }
    if(ret > 0)
    {
        SDK_ASSERT(pComm == NULL);
    }
    else
    {
        FreeComm(pComm);
    }

    return ret;
}


int SdkServerMgmt::PushSysResp(sdk_client_comm_t*& pComm)
{
    int ret=0;
    unsigned int i;
    SdkServerClient *pClient,*pFindClient=NULL;
    sessionid_t sesid;
    privledge_t priv;
    int findidx = -1;

    if(pComm->m_SesId == SDK_SERVER_PRIVATE_SESID)
    {
        /*if we have inner handling*/
        return this->__HandleInnerSysReq(pComm);
    }



    /*we should find the client from the conf */
    for(i=0; i<this->m_pClientConfs.size(); i++)
    {
        pClient = this->m_pClientConfs[i];
        ret = pClient->GetSessionId(sesid,priv);
        if(ret < 0)
        {
            continue;
        }
        if(sesid == pComm->m_SesId)
        {
            pFindClient = pClient;
            findidx = i;
            break;
        }
    }

    if(pFindClient == NULL)
    {
        /*if we do not find ,so we should */
        return -ENODEV;
    }

    SDK_ASSERT(findidx >= 0);
    ret = pFindClient->PushSysResp(pComm);
    if(ret < 0)
    {
        /*now we should delete this and it will */
        delete pFindClient;
        return ret;
    }
    return ret;
}


int SdkServerMgmt::__RemoveAllStreamAndBuffers()
{
    unsigned int i;
    this->m_WillRemoveBuffers.clear();
    this->m_WillRemoveClients.clear();
    this->m_pWillRemoveAudioBuffer = NULL;

    this->m_WillRemoveClients = this->m_pClientStreams;
    for(i=0; i<MAX_STREAM_IDS; i++)
    {
        if(this->m_pStreamBuffers[i])
        {
            this->m_WillRemoveBuffers.push_back(this->m_pStreamBuffers[i]);
        }
    }

    if(this->m_pAudioBuffer)
    {
        this->m_pWillRemoveAudioBuffer = this->m_pAudioBuffer;
    }

    if(this->m_pDBuffer && this->m_DBufferEnding == 0)
    {
        this->m_pWillRemoveDBuffer = this->m_pDBuffer;
    }

    return this->__StartRemoveStreamTimer();
}

int SdkServerMgmt::__RemoveSpecStreamBuffer(SdkServerBuffer * pBuffer)
{
    this->m_WillRemoveBuffers.push_back(pBuffer);
    return this->__StartRemoveStreamTimer();
}

int SdkServerMgmt::__RemoveSpecStreamClient(SdkServerClient * pClient)
{
    unsigned int i;
    for(i=0; i<this->m_WillRemoveClients.size(); i++)
    {
        if(this->m_WillRemoveClients[i] == pClient)
        {
            /*we have inserted ,so we should not insert ok*/
            return 0;
        }
    }
    this->m_WillRemoveClients.push_back(pClient);
    return this->__StartRemoveStreamTimer();
}

int SdkServerMgmt::__RemoveSpecAudioBuffer(SdkAudioBuffer * pAudio)
{
    this->m_pWillRemoveAudioBuffer = pAudio;
    return this->__StartRemoveStreamTimer();
}

int SdkServerMgmt::__RemoveSpecDBuffer(SdkAudioDBuffer * pDBuffer)
{
    this->m_pWillRemoveDBuffer = pDBuffer;
    return this->__StartRemoveStreamTimer();
}

int SdkServerMgmt::PauseAllStreams(void)
{
    int ret,res;
    unsigned int i;
    SdkServerClient* pClient;
    SdkServerBuffer *pBuffer;
    std::vector<SdkServerClient*> passclients;


    /*to stop the client first*/
    for(i=0; i<this->m_pClientStreams.size() ; i++)
    {
        pClient = this->m_pClientStreams[i];
        ret = pClient->PauseStream();
        if(ret < 0)
        {
            /*we call start remove stream timers ,this will give next time*/
            res = this->__RemoveSpecStreamClient(pClient);
            if(res < 0)
            {
                ERROR_INFO("can not start remove stream timer %d\n",res);
                this->__BreakOut();
            }
            ERROR_INFO("pause client [%d] error\n",pClient->GetSocket());
        }
    }

    /*now to stop stream buffer */
    for(i=0; i<MAX_STREAM_IDS; i++)
    {
        pBuffer = this->m_pStreamBuffers[i];
        if(pBuffer)
        {
            ret = pBuffer->PauseStream();
            if(ret < 0)
            {
                res = this->__RemoveSpecStreamBuffer(pBuffer);
                if(res < 0)
                {
                    ERROR_INFO("can not start remove stream timer %d\n",res);
                    this->__BreakOut();
                }
                ERROR_INFO("pause buffer [%d] error\n",i);
            }

        }
    }

    if(this->m_pAudioBuffer)
    {
        ret = this->m_pAudioBuffer->PauseStream();
        if(ret < 0)
        {
            res = this->__RemoveSpecAudioBuffer(this->m_pAudioBuffer);
            if(res < 0)
            {
                ERROR_INFO("can not start remove stream timer %d\n",res);
                this->__BreakOut();
            }
            ERROR_INFO("pause audio buffer [0x%p] error\n",this->m_pAudioBuffer);
        }
    }

    if(this->m_pDBuffer && this->m_DBufferEnding == 0)
    {
        /* we should pause the stream when it is ok in the control and */
        ret = this->m_pDBuffer->PauseStream();
        if(ret < 0)
        {
            res = this->__RemoveSpecDBuffer(this->m_pDBuffer);
            if(res < 0)
            {
                ERROR_INFO("can not start remove stream timer %d\n",res);
                this->__BreakOut();
            }
            ERROR_INFO("pause audio decode buffer [0x%p] error\n",this->m_pDBuffer);
        }
    }

    /*now set the stop stream ok*/
    this->m_StreamStarted = 0;

    /*we have start stream ,so put here to avoid long time delay*/
    this->__ResetLongTimeTimer();

    return 0;
}


int SdkServerMgmt::ResumeAllStreams(sys_stream_info_t * pStreamInfo)
{
    int ret,res;
    unsigned int i;
    SdkServerClient *pClient;
    SdkServerBuffer *pBuffer;


    /*set stream started ,for it let things pass ok*/
    this->m_StreamStarted = 1;
    DEBUG_INFO("pStreamInfo->m_AudioCount %d\n",pStreamInfo->m_AudioCount);
    /*first to copy for the stream info*/
    ret = this->__CopySysStreamInfo(pStreamInfo);
    if(ret < 0)
    {
        this->m_StreamStarted = 0;
        res = this->__RemoveAllStreamAndBuffers();
        if(res < 0)
        {
            ERROR_INFO("can not start remove stream timer %d\n",res);
            this->__BreakOut();
        }
        return ret;
    }

    /*start stream buffer first*/
    for(i=0; i<MAX_STREAM_IDS; i++)
    {
        pBuffer = this->m_pStreamBuffers[i];
        if(pBuffer)
        {
            ret = pBuffer->ResumeStream(pStreamInfo);
            if(ret < 0)
            {
                res = this->__RemoveSpecStreamBuffer(pBuffer);
                if(res < 0)
                {
                    ERROR_INFO("can not remove [%d] %p stream buffer\n",i,pBuffer);
                    this->__BreakOut();
                }
            }
        }
    }

    if(this->m_pAudioBuffer)
    {
        ret = this->m_pAudioBuffer->ResumeStream(pStreamInfo);
        if(ret < 0)
        {
            res = this->__RemoveSpecAudioBuffer(this->m_pAudioBuffer);
            if(res < 0)
            {
                ERROR_INFO("can not remove audio [0x%p] buffer\n",this->m_pAudioBuffer);
                this->__BreakOut();
            }
            ERROR_INFO("could not resume audio buffer [0x%p]\n",this->m_pAudioBuffer);
        }
    }

    if(this->m_pDBuffer && this->m_DBufferEnding == 0)
    {
        ret = this->m_pDBuffer->ResumeStream(&(this->m_AudioDecParam));
        if(ret < 0)
        {
            res = this->__RemoveSpecDBuffer(this->m_pDBuffer);
            if(res < 0)
            {
                ERROR_INFO("can not remove dbuffer [0x%p]\n",this->m_pDBuffer);
                this->__BreakOut();
            }
            ERROR_INFO("could not resume dbuffer [0x%p]\n",this->m_pDBuffer);
        }
    }


    /*now to start client stream*/
    for(i=0; i<this->m_pClientStreams.size(); i++)
    {
        pClient = this->m_pClientStreams[i];
        ret = pClient->ResumeStream(pStreamInfo);
        if(ret < 0)
        {
            res = this->__RemoveSpecStreamClient(pClient);
            if(res < 0)
            {
                ERROR_INFO("can not resume client %d on socket %d\n",i,pClient->GetSocket());
                this->__BreakOut();
            }
        }
    }

    /*we have start stream ,so put here to avoid long time delay*/
    this->__ResetLongTimeTimer();

    return 0;
}


int SdkServerMgmt::QueryStreamStarted(void)
{
    int ret ;
    if(this->m_StreamStarted)
    {
        ret = STREAM_STATE_RUNNING;
    }
    else if(this->m_pStreamInfo)
    {
        ret = STREAM_STATE_PAUSE;
    }
    else
    {
        ret = STREAM_STATE_STOPPED;
    }
    //DEBUG_INFO("query result %d\n",ret);
    return ret;
}

int SdkServerMgmt::GetClients()
{
    int clients = 0;

    clients += this->m_pClientConfs.size();
    clients += this->m_pClientLogins.size();
    clients += this->m_pClientStreams.size();
    return clients;
}

int SdkServerMgmt::SessionRenew(sessionid_t sesid,privledge_t & priv,int& expiretime,int& keeptime,int & err)
{
    int ret;
    SdkServerSession *pSession,*pFindSession=NULL;
    unsigned int i;
    sessionid_t getsesid;
    privledge_t getpriv;

    if(this->m_pLoginHandler == NULL)
    {
        return -ENOENT;
    }

    for(i=0; i<this->m_pSessions.size(); i++)
    {
        pSession = this->m_pSessions[i];
        ret = pSession->GetSessionId(getsesid,getpriv);
        if(ret < 0)
        {
            continue;
        }

        if(getsesid == sesid)
        {
            pFindSession = pSession;
            break;
        }
    }

    if(pFindSession == NULL)
    {
        err = ENODEV;
        return -ENODEV;
    }

    ret = pFindSession->SessionRenew(sesid,priv,expiretime,keeptime,err);
    if(ret < 0)
    {
        return ret;
    }
    SDK_ASSERT(ret > 0);
    return 1;
}





int SdkServerMgmt::UserLoginSession(SdkServerClient* pClient,const char * username,const char * salt,const char * md5check)
{
    int ret;

    if(this->m_pLoginHandler == NULL)
    {
        return -ENOENT;
    }

    ret = this->m_pLoginHandler->AddUserLoginRequest(pClient,this->__IncReqNum(),username,salt,md5check);
    if(ret < 0)
    {
        return ret;
    }

    return this->m_ReqNum;
}

int SdkServerMgmt::UserLoginSessionCallBack(int sock,int reqnum,int err,sessionid_t sesid,privledge_t priv)
{
    int ret;
    unsigned int i;
    SdkServerClient* pClient,*pFindClient=NULL;
    SdkServerSession *pSession=NULL,*pNewSession=NULL,*pFindSession=NULL;
    int findidx = -1;
    sessionid_t getsesid;
    privledge_t getpriv;
    int res;
    for(i=0; i<this->m_pClientLogins.size(); i++)
    {
        pClient = this->m_pClientLogins[i];
        if(pClient->GetSocket() == sock)
        {
            pFindClient = pClient ;
            break;
        }
    }

    if(pFindClient == NULL)
    {
        if(err == 0)
        {
            /*if success ,we can not make any client ,so we should set this to logout */
            if(this->m_pLoginHandler)
            {
                res = this->m_pLoginHandler->AddUserLogoutRequest(NULL,this->__IncReqNum(),sesid,priv);
                if(res < 0)
                {
                    /*this will not run ok ,so we should break run loop*/
                    this->__BreakOut();
                }
                else
                {
                    this->m_WaitingRemoveSessionId.push_back(sesid);
                }
            }
        }
        return -ENODEV;
    }

    if(err == 0)
    {
        /****************************************
        	*   if success login ,so we find out whether there is
        	*   a session exist ,
        	*        exists:
        	*             1, register  , not success ,just set call back not success
        	*             2, register success ,callback not success , just unregister
        	*                     if it is the new or the pFindSession ,this will delete and
        	*                      set addlogoutrequest
        	*        not exists:
        	*             1, new session
        	*             2, register , not success ,just call back not success delete ths job
        	*             3, register success ,not call back success , just unregister ,if
        	*                       clients == 0 ,delete this one ,and call addlogoutrequest
                ****************************************/
        for(i=0; i<this->m_pSessions.size(); i++)
        {
            pSession = this->m_pSessions[i];
            ret = pSession->GetSessionId(getsesid,getpriv);
            if(ret < 0)
            {
                continue;
            }

            if(getsesid == sesid)
            {
                pFindSession = pSession;
                findidx=  i;
                break;
            }
        }
        if(pFindSession == NULL)
        {
            /*no session ,so we should make a new one*/
            pNewSession = new SdkServerSession(sesid,priv,this->m_MaxClients,this->m_ExpireTime,this->m_KeepTime);
            SDK_ASSERT(pNewSession);
            ret = pNewSession->Start();
            if(ret < 0)
            {
                res = pNewSession->Clients();
                SDK_ASSERT(res == 0);
                delete pNewSession;
                pNewSession = NULL;
				DEBUG_INFO("MGMT KEEPTIME %d\n",this->m_KeepTime);
                res = pFindClient->LoginCallBack(EFAULT,reqnum,sesid,priv,this->m_ExpireTime,this->m_KeepTime);
                if(this->m_pLoginHandler)
                {
                    res = this->m_pLoginHandler->AddUserLogoutRequest(NULL,this->__IncReqNum(),sesid,priv);
                    if(res < 0)
                    {
                        /*this will not run ok ,so we should break run loop*/
                        this->__BreakOut();
                    }
                    else
                    {
                        this->m_WaitingRemoveSessionId.push_back(sesid);
                    }
                }
                return ret;
            }

            /*this will be new for the sessions*/
            this->m_pSessions.push_back(pNewSession);

			DEBUG_INFO("MGMT KEEPTIME %d\n",this->m_KeepTime);
            ret = pFindClient->LoginCallBack(err,reqnum,sesid,priv,this->m_ExpireTime,this->m_KeepTime);
            if(ret < 0)
            {
                /*
                			we do not delete the pFindClient ,because the client is in the fail timer
                			and it will auto delete itself
                		*/
                res = pNewSession->UnRegisterServerSession(pFindClient);
				SDK_ASSERT(pNewSession->Clients() == 0);
				this->m_pSessions.erase(this->m_pSessions.end());
				delete pNewSession ;
				pNewSession = NULL;
                if(res > 0)
                {
                    ERROR_INFO("[%d] client has already register for session\n",pFindClient->GetSocket());
                }
                if(this->m_pLoginHandler)
                {
                    res = this->m_pLoginHandler->AddUserLogoutRequest(NULL,this->__IncReqNum(),sesid,priv);
                    if(res < 0)
                    {
                        /*this will not run ok ,so we should break run loop*/
                        this->__BreakOut();
                    }
                    else
                    {
                        this->m_WaitingRemoveSessionId.push_back(sesid);
                    }
                }
                return ret;
            }

            /*all is ok */
            return 1;
        }
        else
        {
            SDK_ASSERT(findidx >= 0);
			DEBUG_INFO("MGMT KEEPTIME %d\n",this->m_KeepTime);
            ret = pFindClient->LoginCallBack(err,reqnum,sesid,priv,this->m_ExpireTime,this->m_KeepTime);
            if(ret < 0)
            {
                res = pFindSession->UnRegisterServerSession(pFindClient);
                if(pFindSession->Clients() == 0)
                {
                    this->m_pSessions.erase(this->m_pSessions.begin()+findidx);
                    delete pFindSession ;
                    pFindSession = NULL;
                    findidx = -1;
                    if(this->m_pLoginHandler)
                    {
                        res = this->m_pLoginHandler->AddUserLogoutRequest(NULL,this->__IncReqNum(),sesid,priv);
                        if(res < 0)
                        {
                            /*this will not run ok ,so we should break run loop*/
                            this->__BreakOut();
                        }
                        else
                        {
                            this->m_WaitingRemoveSessionId.push_back(sesid);
                        }
                    }
                }
                return ret;
            }

            return 1;
        }

    }
    else
    {
		DEBUG_INFO("MGMT KEEPTIME %d\n",this->m_KeepTime);
        ret = pFindClient->LoginCallBack(err,reqnum,sesid,priv,this->m_ExpireTime,this->m_KeepTime);
        if(ret < 0)
        {
            return ret;
        }
    }

    return 1;
}


int SdkServerMgmt::UserLogoutSessionCallBack(int sock,int reqnum,int err,sessionid_t sesid)
{
    unsigned int i;

    for(i=0; i<this->m_WaitingRemoveSessionId.size(); i++)
    {
        if(this->m_WaitingRemoveSessionId[i] == sesid)
        {
            this->m_WaitingRemoveSessionId.erase(this->m_WaitingRemoveSessionId.begin() + i);
            return 0;
        }
    }

    return -ENODEV;
}

int SdkServerMgmt::UserQueryLoginCallBack(int reqnum,
        std::vector<sessionid_t>& sesids,std::vector<privledge_t>& privs)
{
    int findidx;
    std::vector<sessionid_t> notlogoutses,notloginses;
    unsigned int i,j;
    sessionid_t cursesid,getsesid;
    privledge_t getpriv;
    SdkServerSession* pSession;


    SDK_ASSERT(notlogoutses.size() == 0);
    SDK_ASSERT(notloginses.size() == 0);

    for(i=0; i<sesids.size(); i++)
    {
        cursesid = sesids[i];
        findidx = -1;
        for(j=0; j<this->m_pSessions.size(); j++)
        {
            pSession = this->m_pSessions[j];
            pSession->GetSessionId(getsesid,getpriv);
            if(cursesid == getsesid)
            {
                findidx = j;
                break;
            }
        }

        if(findidx < 0)
        {
            /*this is notlogout session*/
            notlogoutses.push_back(cursesid);
        }
    }

    for(i=0; i<this->m_pSessions.size(); i++)
    {
        pSession = this->m_pSessions[i];
        pSession->GetSessionId(getsesid,getpriv);
        findidx = -1;
        for(j=0; j<sesids.size(); j++)
        {
            cursesid = sesids[j];
            if(getsesid == cursesid)
            {
                findidx = j;
                break;
            }
        }

        if(findidx < 0)
        {
            notloginses.push_back(getsesid);
        }
    }

    if(notlogoutses.size() > 0)
    {
        /**/
        for(i=0; i<notlogoutses.size(); i++)
        {
            cursesid = notlogoutses[i];
            findidx = -1;
            for(j=0; j<this->m_WaitingRemoveSessionId.size(); j++)
            {
                getsesid = this->m_WaitingRemoveSessionId[j];
                if(getsesid == cursesid)
                {
                    findidx = j;
                    break;
                }
            }

            if(findidx < 0)
            {
                ERROR_INFO("has not logout session [%d]\n",cursesid);
                this->__BreakOut();
            }
        }
    }

    if(notloginses.size() > 0)
    {
        ERROR_INFO("has not login session\n");
        this->__BreakOut();
    }

    return 0;
}

int SdkServerMgmt::StartStreamId(int streamid,SdkServerClient* pClient)
{
    int ret;
    int i;
    int findidx = -1;
    sessionid_t sesid;
    privledge_t priv;


    if(this->m_pStreamInfo == NULL)
    {
        ERROR_INFO("\n");
        return -ENODEV;
    }

    if(streamid >=  MAX_STREAM_IDS && streamid != AUDIO_STREAM_ID)
    {
        ERROR_INFO("\n");
        return -EINVAL;
    }
    DEBUG_INFO("\n");

    ret = pClient->GetSessionId(sesid,priv);
    DEBUG_INFO("sesid %d priv %d\n",sesid,priv);
    SDK_ASSERT(ret >= 0);
    if(streamid < MAX_STREAM_IDS && this->m_pStreamBuffers[streamid] == NULL)
    {
        /*it is not exists ,so we should */
        /*now to test whether ,we have the streamid inform*/
        findidx = -1;
        //DEBUG_INFO("count of streaminfo %d\n",this->m_pStreamInfo->m_Count);
        for(i=0; i<this->m_pStreamInfo->m_Count; i++)
        {
            SysPkgEncodeCfg* pCfg = &(this->m_pStreamInfo->m_VideoInfo[i]);
            DEBUG_INFO("flag[%d] %d\n",i,pCfg->s_Flag);
            if(pCfg->s_Flag == (streamid))
            {
                findidx = i;
                break;
            }
        }
        DEBUG_INFO("0x%p audiocount %d\n",this->m_pStreamInfo,this->m_pStreamInfo->m_AudioCount);

        if(findidx < 0)
        {
            ERROR_INFO("\n");
            /*we do not have the streamid ,so we should do this job not ok*/
            return -ENODEV;
        }

        //DEBUG_INFO("\n");
        /*ok ,we should test whether it is ok to start*/
        this->m_pStreamBuffers[streamid] = new SdkServerBuffer(streamid,this->m_MaxPacks,this->m_pRunningBits);
        SDK_ASSERT(this->m_pStreamBuffers[streamid]);
        if(this->m_StreamStarted)
        {
            DEBUG_INFO("\n");
            /*if we set started ,so we should do this ok when start stream*/
            ret = this->m_pStreamBuffers[streamid]->StartStream(this->m_pStreamInfo);
            if(ret < 0)
            {
                this->m_pStreamBuffers[streamid]->UnRegisterSock(pClient->GetSocket());
                delete this->m_pStreamBuffers[streamid];
                this->m_pStreamBuffers[streamid] = NULL;

                /*we have start stream ,so put here to avoid long time delay*/
                this->__ResetLongTimeTimer();
                ERROR_INFO("\n");
                return ret;
            }
            /*we have start stream ,so put here to avoid long time delay*/
            this->__ResetLongTimeTimer();
        }
        DEBUG_INFO("\n");
        ret = this->m_pStreamBuffers[streamid]->RegisterSock(pClient->GetSocket(),sesid);
        if(ret < 0)
        {
            delete this->m_pStreamBuffers[streamid];
            this->m_pStreamBuffers[streamid] = NULL;
            ERROR_INFO("\n");
            return ret;
        }
        DEBUG_INFO("\n");

        ret = this->__TryStartPullTimer();
        if(ret < 0)
        {
            this->m_pStreamBuffers[streamid]->UnRegisterSock(pClient->GetSocket());
            delete this->m_pStreamBuffers[streamid];
            this->m_pStreamBuffers[streamid] = NULL;
            ERROR_INFO("\n");
            return ret;
        }
        DEBUG_INFO("\n");
        return this->m_StreamStarted;
    }
    else if(streamid == AUDIO_STREAM_ID)
    {
        DEBUG_INFO("\n");
        if(this->m_pAudioBuffer == NULL)
        {
            /*it is null ,so we should open the audio buffer ,first to check for the audio buffer is ok*/
            if(this->m_pStreamInfo == NULL ||
                    this->m_pStreamInfo->m_AudioCount < 1)
            {
                /*we are not support for audio*/
                DEBUG_INFO("0x%p audiocount %d\n",this->m_pStreamInfo,this->m_pStreamInfo ? this->m_pStreamInfo->m_AudioCount : 0);
                return -ENODEV;
            }

            /*now we should make the new buffer*/
            DEBUG_INFO("\n");
            this->m_pAudioBuffer = new SdkAudioBuffer((sdk_audio_format_enum_t)this->m_pStreamInfo->m_AudioInfo[0].s_EncodeType,this->m_MaxPacks*20,this->m_MaxClients,this->m_pRunningBits);
            ret = this->m_pAudioBuffer->StartStream(this->m_pStreamInfo);
            if(ret < 0)
            {
                delete this->m_pAudioBuffer;
                this->m_pAudioBuffer = NULL;
                ERROR_INFO("could not start audio stream\n");

                /*we have start stream ,so put here to avoid long time delay*/
                this->__ResetLongTimeTimer();
                return ret;
            }

            /*we have start stream ,so put here to avoid long time delay*/
            this->__ResetLongTimeTimer();

            ret = this->m_pAudioBuffer->RegisterSock(pClient->GetSocket(),sesid);
            if(ret < 0)
            {
                delete this->m_pAudioBuffer;
                this->m_pAudioBuffer = NULL;
                ERROR_INFO("Register client[%d] error (%d)\n",pClient->GetSocket(),ret);
                return ret;
            }

            /*we start the pull timer ,so we should make it running*/
            ret = this->__TryStartPullTimer();
            if(ret < 0)
            {
                this->m_pAudioBuffer->UnRegisterSock(pClient->GetSocket());
                delete this->m_pAudioBuffer;
                this->m_pAudioBuffer = NULL;
                return ret;
            }

            return this->m_StreamStarted;

        }

        /*we have alread audio buffer ,so we do the job*/
        ret = this->m_pAudioBuffer->RegisterSock(pClient->GetSocket(),sesid);
        if(ret < 0)
        {
            ERROR_INFO("Register client[%d] error (%d)\n",pClient->GetSocket(),ret);
            return ret;
        }

        /*if we have already running,no __TryStartPullTimer will running*/

        return this->m_StreamStarted;
    }


    /*we have alread this stream buffer ,so we just register*/
    ret = this->m_pStreamBuffers[streamid]->RegisterSock(pClient->GetSocket(),sesid);
    if(ret < 0)
    {
        ERROR_INFO("\n");
        return ret;
    }


    /*we return ,when 1 ,just started ,nothing to wait , 0 for waiting*/
    return this->m_StreamStarted;
}


int SdkServerMgmt::GetStreamIdInfo(sys_stream_info_t* pSysQueryStreamids)
{
    if(this->m_pStreamInfo == NULL)
    {
        return -ENODEV;
    }
    DEBUG_INFO("\n");

    memcpy(pSysQueryStreamids,this->m_pStreamInfo,sizeof(*pSysQueryStreamids));
    return 0;
}

int SdkServerMgmt::GetStreamData(int sock,int streamid,struct iovec * pIoVec,int & iovlen,int & begin)
{
    if(this->m_pStreamInfo==NULL)
    {
        ERROR_INFO("\n");
        return -ENODEV;
    }

    if(streamid <  MAX_STREAM_IDS && this->m_pStreamBuffers[streamid])
    {
        return this->m_pStreamBuffers[streamid]->GetStreamData(sock,pIoVec,iovlen,begin);
    }

    if(streamid == AUDIO_STREAM_ID && this->m_pAudioBuffer)
    {
        return this->m_pAudioBuffer->GetStreamData(sock,pIoVec,iovlen,begin);
    }

    return -ENODEV;
}


int SdkServerMgmt::ForwardStreamData(int sock,int streamid,struct iovec * pIoVec,int iovlen,int forwardlen)
{
    if(this->m_pStreamInfo==NULL)
    {
        return -ENODEV;
    }

    if(streamid <  MAX_STREAM_IDS && this->m_pStreamBuffers[streamid])
    {
        return this->m_pStreamBuffers[streamid]->ForwardStreamData(sock,pIoVec,iovlen,forwardlen);
    }

    if(streamid == AUDIO_STREAM_ID && this->m_pAudioBuffer)
    {
        return this->m_pAudioBuffer->ForwardStreamData(sock,pIoVec,iovlen,forwardlen);
    }

    return -ENODEV;
}

int SdkServerMgmt::PushSysReq(sdk_client_comm_t*& pComm)
{
    int ret;
    SDK_ASSERT(this->m_pSysReq);

    ret = this->m_pSysReq->AddRequest(pComm);
    if(ret < 0)
    {
        return ret;
    }

    return 1;
}

int SdkServerMgmt::__ResumeStreamIdVideo(int streamid,SdkServerClient * pClient)
{
    int ret;
    std::vector<int> clisocks;
    int newbuffer=0;
    sessionid_t sesid;
    privledge_t priv;
    unsigned int i;
    int findidx=-1;

    SDK_ASSERT(streamid >= 0 && streamid < MAX_STREAM_IDS);
    ret = pClient->GetSessionId(sesid,priv);
    SDK_ASSERT(ret >= 0);

    if(this->m_pStreamBuffers[streamid] == NULL)
    {
        /*we should start for this*/
        this->m_pStreamBuffers[streamid] = new SdkServerBuffer(streamid,this->m_MaxPacks,this->m_pRunningBits);
        SDK_ASSERT(this->m_pStreamBuffers[streamid]);
        ret = this->m_pStreamBuffers[streamid]->StartStream(this->m_pStreamInfo);
        if(ret < 0)
        {
            delete this->m_pStreamBuffers[streamid];
            this->m_pStreamBuffers[streamid] = NULL;
            return ret;
        }
        newbuffer = 1;
    }

    /*now to test whether this is the client into the socket*/
    clisocks.clear();
    ret = this->m_pStreamBuffers[streamid]->GetClients(clisocks);
    if(ret == 0)
    {
        ret = this->m_pStreamBuffers[streamid]->RegisterSock(pClient->GetSocket(),sesid);
        if(ret < 0)
        {
            /*because this is no clients ,so it*/
            delete this->m_pStreamBuffers[streamid];
            this->m_pStreamBuffers[streamid] = NULL;
            return ret;
        }

        /*this is new buffer so we should try start pull timer*/
        ret = this->__TryStartPullTimer();
        if(ret < 0)
        {
            this->m_pStreamBuffers[streamid]->UnRegisterSock(pClient->GetSocket());
            delete this->m_pStreamBuffers[streamid];
            this->m_pStreamBuffers[streamid] = NULL;
            return ret;
        }
        return 1;
    }

    SDK_ASSERT(newbuffer == 0);

    /*now to check it is in this*/
    findidx = -1;
    for(i=0; i<clisocks.size(); i++)
    {
        if(pClient->GetSocket() == clisocks[i])
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        return 0;
    }

    ret = this->m_pStreamBuffers[streamid]->RegisterSock(pClient->GetSocket(),sesid);
    if(ret < 0)
    {
        return ret;
    }
    return 1;
}

int SdkServerMgmt::__ResumeStreamIdAudio(int streamid,SdkServerClient * pClient)
{
    int ret;
    std::vector<int> clisocks;
    int newbuffer=0;
    sessionid_t sesid;
    privledge_t priv;
    unsigned int i;
    int findidx=-1;

    SDK_ASSERT(streamid == AUDIO_STREAM_ID);
    ret = pClient->GetSessionId(sesid,priv);
    SDK_ASSERT(ret >= 0);

    if(this->m_pStreamInfo == NULL ||
            this->m_pStreamInfo->m_AudioCount < 1)
    {
        return -ENODEV;
    }

    if(this->m_pAudioBuffer == NULL)
    {
        DEBUG_INFO("\n");
        this->m_pAudioBuffer = new SdkAudioBuffer((sdk_audio_format_enum_t)this->m_pStreamInfo->m_AudioInfo[0].s_EncodeType,this->m_MaxPacks*4,this->m_MaxClients,this->m_pRunningBits);
        ret = this->m_pAudioBuffer->StartStream(this->m_pStreamInfo);
        if(ret < 0)
        {
            delete this->m_pAudioBuffer;
            this->m_pAudioBuffer = NULL;
            return ret;
        }

        newbuffer = 1;
    }


    clisocks.clear();
    ret = this->m_pAudioBuffer->GetClients(clisocks);
    if(ret == 0)
    {
        /*no client in this audio buffer */
        ret = this->m_pAudioBuffer->RegisterSock(pClient->GetSocket(),sesid);
        if(ret < 0)
        {
            /*because this is no clients ,so it*/
            delete this->m_pAudioBuffer;
            this->m_pAudioBuffer= NULL;
            return ret;
        }

        ret = this->__TryStartPullTimer();
        if(ret < 0)
        {
            this->m_pAudioBuffer->UnRegisterSock(pClient->GetSocket());
            delete this->m_pAudioBuffer;
            this->m_pAudioBuffer = NULL;
            return ret;
        }
        return 1;
    }


    /*because it means we have register some clients before,so it must not the newer one*/
    SDK_ASSERT(newbuffer == 0);

    /*now to check it is in this*/
    findidx = -1;
    for(i=0; i<clisocks.size(); i++)
    {
        if(pClient->GetSocket() == clisocks[i])
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        return 0;
    }

    ret = this->m_pAudioBuffer->RegisterSock(pClient->GetSocket(),sesid);
    if(ret < 0)
    {
        return ret;
    }
    return 1;

}


/***************************************
*    it is almost like the StartStreamId but it will
*    test if this is ok for the registered
***************************************/
int SdkServerMgmt::ResumeStreamId(int streamid,SdkServerClient * pClient)
{


    if(streamid < MAX_STREAM_IDS)
    {
        return this->__ResumeStreamIdVideo(streamid,pClient);
    }
    else if(streamid == AUDIO_STREAM_ID)
    {
        return this->__ResumeStreamIdAudio(streamid,pClient);
    }

    return -EINVAL;

}


/************************************************
* we regard it as the stream start ,this will give the
*
************************************************/
int SdkServerMgmt::StartAllStreams(sys_stream_info_t * pStreamInfo)
{
    DEBUG_INFO("start all streams\n");
    DEBUG_INFO("pStream->m_AudioCount %d\n",pStreamInfo->m_AudioCount);
    return this->ResumeAllStreams(pStreamInfo);
}


/*this will release all the clients to get streams */
int SdkServerMgmt::StopAllStreams(void)
{
    unsigned int i;

    /*now to remove the clients*/
    this->__ReleaseClientStreams();
    for(i=0; i<MAX_STREAM_IDS; i++)
    {
        /*because ,we have release all the client ,so we can release the stream buffer*/
        SdkServerBuffer* pBuffer=this->m_pStreamBuffers[i];
        if(pBuffer)
        {
            delete pBuffer;
        }
        this->m_pStreamBuffers[i] = NULL;
    }

    if(this->m_pAudioBuffer)
    {
        delete this->m_pAudioBuffer;
    }
    this->m_pAudioBuffer = NULL;

    this->__CopySysStreamInfo(NULL);
    this->m_StreamStarted = 0;
    this->__StopPullTimer();


    /*we have start stream ,so put here to avoid long time delay*/
    this->__ResetLongTimeTimer();
    return 0;
}


void SdkServerMgmt::__StopPullTimer()
{
    if(this->m_InsertPullTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvPullTimer));
    }
    this->m_InsertPullTimer = 0;
    return ;
}

int SdkServerMgmt::__StartPullTimer()
{
    SDK_ASSERT(this->m_InsertPullTimer == 0);
    ev_timer_init(&(this->m_EvPullTimer),SdkServerMgmt::PullTimerCallBack,0.01,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvPullTimer));
    this->m_InsertPullTimer = 1;
    return 0;
}

int SdkServerMgmt::__ResetPullTimer()
{
    this->__StopPullTimer();
    return this->__StartPullTimer();
}


int SdkServerMgmt::__TryStartPullTimer()
{
    unsigned int i;
    int findidx = -1;
    int ret;
    for(i=0; i<MAX_STREAM_IDS; i++)
    {
        if(this->m_pStreamBuffers[i])
        {
            findidx = i;
            break;
        }
    }

    if(findidx < 0)
    {
        if(this->m_pAudioBuffer == NULL)
        {
            return 0;
        }
    }

    if(this->m_InsertPullTimer == 0)
    {
        ret= this->__StartPullTimer();
        if(ret < 0)
        {
            return ret;
        }
        return 1;
    }
    return 0;
}

int SdkServerMgmt::__TryStopPullTimer()
{
    unsigned int i;
    for(i=0; i<MAX_STREAM_IDS; i++)
    {
        if(this->m_pStreamBuffers[i])
        {
            return 0;
        }
    }

    if(this->m_pAudioBuffer)
    {
        return 0;
    }

    this->__StopPullTimer();
    return 1;
}


static int __AddSocks(std::vector<int>& allsocks,std::vector<int>& clisocks)
{
    unsigned int i,j;
    int findidx = -1;

    for(i=0; i<clisocks.size(); i++)
    {
        findidx =-1;
        for(j=0; j<allsocks.size(); j++)
        {
            if(allsocks[j] == clisocks[i])
            {
                findidx = j;
                break;
            }
        }
        if(findidx < 0)
        {
            allsocks.push_back(clisocks[i]);
        }
    }

    return allsocks.size();
}

int SdkServerMgmt::__PullTimerImpl()
{
    unsigned int i,j;
    std::vector<int> clisocks,allsocks;
    SdkServerBuffer *pBuffer;
    int ret;
    std::vector<SdkServerClient*> pRemoveClients;
    SdkServerClient* pClient;
    std::vector<SdkServerClient*> FailedClients;


    for(i=0; i<MAX_STREAM_IDS; i++)
    {
        pBuffer = this->m_pStreamBuffers[i];
        if(pBuffer)
        {
            do
            {
                ret = pBuffer->PullStreamData(clisocks);
                if(ret < 0)
                {
                    delete pBuffer;
                    this->m_pStreamBuffers[i] = NULL;
                    pBuffer = NULL;
                    break;
                }
                else if(ret > 0)
                {
                    __AddSocks(allsocks,clisocks);
                }
            }
            while(ret > 0);
        }
    }

    if(this->m_pAudioBuffer)
    {
        do
        {
            ret = this->m_pAudioBuffer->PullStreamData(clisocks);
            if(ret < 0)
            {
                delete this->m_pAudioBuffer;
                this->m_pAudioBuffer = NULL;
                break;
            }
            else if(ret > 0)
            {
                __AddSocks(allsocks,clisocks);
            }
        }
        while(ret > 0);
    }

    /*now all is getting , so we should notify the client*/
    for(i=0; i<allsocks.size(); i++)
    {
        for(j=0; j<this->m_pClientStreams.size(); j++)
        {

            pClient = this->m_pClientStreams[j];
            if(pClient->GetSocket() == allsocks[i])
            {
                /*can not notify success ,just delete this one,*/
                //DEBUG_INFO("notify %d client %p\n",pClient->GetSocket(),pClient);
                ret = pClient->NotifyClient();
                if(ret < 0)
                {
                    /*we delete the client,it will modify the pClientStreams and pStreamBuffer
                    but we break the search scan ,so it will not disturbing*/
                    delete pClient;
                }
                break;
            }
        }
    }


    ret = this->__TryStopPullTimer();
    if(ret == 0)
    {
        /*if we do not stop pull timer ,we should reset pull timer for the next time to pull*/
        ret = this->__ResetPullTimer();
        if(ret < 0)
        {
            return ret;
        }
    }


    return 0;


}


void SdkServerMgmt::__BreakOut()
{
    BACK_TRACE();
    ev_break(EV_DEFAULT,EVBREAK_ONE);
    return ;
}

void SdkServerMgmt::PullTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkServerMgmt* pThis= (SdkServerMgmt*)arg;
    int ret;

    ret = pThis->__PullTimerImpl();
    if(ret < 0)
    {
        pThis->__BreakOut();
    }
    return ;
}


void SdkServerMgmt::DebugStreamBufferBlock(int streamid,int sock)
{
    if(streamid < MAX_STREAM_IDS && streamid >= 0 && this->m_pStreamBuffers[streamid])
    {
        this->m_pStreamBuffers[streamid]->DebugClientBuffer(sock);
    }
    else if(streamid == AUDIO_STREAM_ID && this->m_pAudioBuffer)
    {
        this->m_pAudioBuffer->DebugClientBuffer(sock);
    }
    return ;
}

int SdkServerMgmt::InitDaemon()
{
    int ret;
    if(this->m_pDaemon)
    {
        return 0;
    }

    DEBUG_INFO("init daemon\n");
    this->m_pDaemon = new SdkClientDaemon(this,5,this->m_pRunningBits);
    SDK_ASSERT(this->m_pDaemon);
    ret = this->m_pDaemon->Start();
    if(ret < 0)
    {
        delete this->m_pDaemon;
        this->m_pDaemon = NULL;
        return ret;
    }

    return 0;
}

int SdkServerMgmt::IsNeedOpenAudio(int streamid)
{
    int i;
    if(streamid < 0 || streamid >= MAX_STREAM_IDS)
    {
        return 0;
    }

    if(this->m_pStreamInfo == NULL)
    {
        return 0;
    }

    for(i=0; i<this->m_pStreamInfo->m_Count; i++)
    {
        if(this->m_pStreamInfo->m_VideoInfo[i].s_Flag == streamid)
        {
            if(this->m_pStreamInfo->m_VideoInfo[i].s_StreamType == 2)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }
    return 0;
}

void SdkServerMgmt::StopStreamId(int streamid,SdkServerClient * pClient)
{
    if(streamid >=0 && streamid < MAX_STREAM_IDS)
    {
        if(this->m_pStreamBuffers[streamid])
        {
            this->m_pStreamBuffers[streamid]->UnRegisterSock(pClient->GetSocket());
        }
    }
    else if(streamid == AUDIO_STREAM_ID)
    {
        if(this->m_pAudioBuffer)
        {
            this->m_pAudioBuffer->UnRegisterSock(pClient->GetSocket());
        }
    }

    return ;
}

void SdkServerMgmt::__StopReleaseDBufferTimer()
{
    if(this->m_InsertReleaseDBufferTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvReleaseDBufferTimer));
    }
    this->m_InsertReleaseDBufferTimer = 0;
    return;
}

int SdkServerMgmt::__StartReleaseDBufferTimer()
{
    BACK_TRACE();
    SDK_ASSERT(this->m_InsertReleaseDBufferTimer == 0);
    /*we give 5 times*/
    ev_timer_init(&(this->m_EvReleaseDBufferTimer),SdkServerMgmt::ReleaseDBufferTimerCallBack,5.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvReleaseDBufferTimer));
    this->m_InsertReleaseDBufferTimer = 1;
    return 0;
}

void SdkServerMgmt::ReleaseDBufferTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkServerMgmt* pThis = (SdkServerMgmt*)arg;

    ERROR_INFO("Release DBuffer timer out\n");
    pThis->__DirectReleaseDBuffer();
    return;
}

void SdkServerMgmt::__DirectReleaseDBuffer()
{
    if(this->m_pDBuffer)
    {
        delete this->m_pDBuffer;
        if(this->m_DBufferEnding == 1)
        {
            /*because this will start timer ,so we do this*/
            this->__StopReleaseDBufferTimer();
        }

    }
#ifdef AUDIO_DUAL_FILE_EMULATE
    /*we should stop all the timer for audio dual this will not give any ok*/
    this->__StopStopAudioDualFileTimer();
    this->__StopStartAudioDualFileTimer();
    this->m_StartSeqId = 0;
    this->m_StopSeqId = 0;
#endif
    this->m_pDBuffer = NULL;
    this->m_DBufferEnding = 0;
    memset(&(this->m_AudioDecParam),0,sizeof(this->m_AudioDecParam));
    return;

}


#ifdef AUDIO_DUAL_FILE_EMULATE

int SdkServerMgmt::__SendStartAudioDecodeRequest(int sock,AudioDecParam * pAudioDec)
{
    int ret;
    this->m_StartSeqId = sock;
    ret = this->__StartStartAudioDualFileTimer();
    if(ret < 0)
    {
        return ret;
    }
    return 1;
}

int SdkServerMgmt::__SendStopAudioDecodeRequest(int sock)
{
    int ret;
    this->m_StopSeqId = sock;
    ret = this->__StartStopAudioDualFileTimer();
    if(ret < 0)
    {
        return ret;
    }
    return 1;
}

#else

int SdkServerMgmt::__SendStartAudioDecodeRequest(int sock,AudioDecParam * pAudioDec)
{
    int ret;
    std::auto_ptr<SdkSysCp> pSysCp2(new SdkSysCp());
    SdkSysCp* pSysCp = pSysCp2.get();
    sdk_client_comm_t *pComm=NULL;
    SysPkgAttrHeader* pAttr=NULL;
    SysPkgAudioDecParam* pSendAudioDec=NULL;
    int decsize = sizeof(*pAttr) + sizeof(*pSendAudioDec);
    std::auto_ptr<unsigned char> pSendBuf2(new unsigned char[decsize]);
    unsigned char* pSendBuf = pSendBuf2.get();

    if(this->m_pSysReq == NULL)
    {
        ERROR_INFO("[%d]no sysreq set\n",sock);
        return -EFAULT;
    }

    ret = pSysCp->SetCode(SYSCODE_START_AUDIO_DECODE_REQ);
    if(ret < 0)
    {
        ERROR_INFO("\n");
        goto fail;
    }

    ret = pSysCp->SetSessionSeq(SDK_SERVER_PRIVATE_SESID,sock);
    if(ret < 0)
    {
        ERROR_INFO("\n");
        goto fail;
    }

    ret = pSysCp->SetAttrCount(1);
    if(ret < 0)
    {
        ERROR_INFO("\n");
        goto fail;
    }

    pAttr = (SysPkgAttrHeader*)pSendBuf;
    pSendAudioDec = (SysPkgAudioDecParam*)((ptr_t)pSendBuf + sizeof(*pAttr));

    pAttr->s_Type = HOST_TO_PROTO16(TYPE_AUDIO_DECODE);
    pAttr->s_Length = HOST_TO_PROTO16(decsize);

    pSendAudioDec->s_AudioId = HOST_TO_PROTO32(pAudioDec->s_AudioId);
    if(pAudioDec->s_Codec == 1)
    {
        pSendAudioDec->s_Codec = HOST_TO_PROTO32(pAudioDec->s_Codec);
    }
    else
    {
        pSendAudioDec->s_Codec = HOST_TO_PROTO32(pAudioDec->s_Codec);
    }
    pSendAudioDec->s_SampleFreq = HOST_TO_PROTO32(pAudioDec->s_SampleFreq);
    pSendAudioDec->s_BitWidth = HOST_TO_PROTO32(pAudioDec->s_BitWidth);
    pSendAudioDec->s_ChannelNum = HOST_TO_PROTO16(pAudioDec->s_ChannelNum);
    pSendAudioDec->s_FrameRate = HOST_TO_PROTO16(pAudioDec->s_FrameRate);
    pSendAudioDec->s_BitRate = HOST_TO_PROTO16(pAudioDec->s_BitRate);
    pSendAudioDec->s_Volume = HOST_TO_PROTO16(pAudioDec->s_Volume);
    pSendAudioDec->s_AecFlag = HOST_TO_PROTO16(pAudioDec->s_AecFlag);
    pSendAudioDec->s_AecDelayTime = HOST_TO_PROTO16(pAudioDec->s_AecDelayTime);
    memset(&(pSendAudioDec->s_Reserved[0]),0,sizeof(pSendAudioDec->s_Reserved));
    ret = pSysCp->SetData(pSendBuf,decsize);
    if(ret < 0)
    {
        goto fail;
    }

    pComm = pSysCp->GetClientComm();
    if(pComm == NULL)
    {
        ret = -errno ? -errno : -1;
        goto fail;
    }

    DEBUG_BUFFER_FMT(pComm->m_Data,pComm->m_DataLen,"[%d]Format StartAudioDec",sock);
    ret = this->m_pSysReq->AddRequest(pComm);
    if(ret < 0)
    {
        goto fail;
    }
    SDK_ASSERT(pComm == NULL);
    return 1;

fail:
    FreeComm(pComm);
    return ret;
}


int SdkServerMgmt::__SendStopAudioDecodeRequest(int sock)
{
    int ret;
    std::auto_ptr<SdkSysCp> pSysCp2(new SdkSysCp());
    SdkSysCp* pSysCp = pSysCp2.get();
    sdk_client_comm_t *pComm=NULL;
    SysPkgAttrHeader* pAttr=NULL;
    uint32_t* pAudioId=NULL;

    int stopsize=sizeof(*pAttr)+ sizeof(*pAudioId);
    std::auto_ptr<unsigned char> pStopBuf2(new unsigned char[stopsize]);
    unsigned char* pStopBuf = pStopBuf2.get();

    if(this->m_pSysReq == NULL)
    {
        ERROR_INFO("[%d]no sysreq set\n",sock);
        return -EFAULT;
    }

    ret = pSysCp->SetCode(SYSCODE_STOP_AUDIO_DECODE_REQ);
    if(ret < 0)
    {
        goto fail;
    }

    ret = pSysCp->SetSessionSeq(SDK_SERVER_PRIVATE_SESID,sock);
    if(ret < 0)
    {
        goto fail;
    }

    ret = pSysCp->SetAttrCount(1);
    if(ret < 0)
    {
        goto fail;
    }

    pAttr = (SysPkgAttrHeader*)pStopBuf;
    pAudioId = (uint32_t*)((ptr_t)pStopBuf + sizeof(*pAttr));
    pAttr->s_Type = HOST_TO_PROTO16(TYPE_INTVALUE);
    pAttr->s_Length = HOST_TO_PROTO16(stopsize);
    *pAudioId = HOST_TO_PROTO32(0);
    DEBUG_INFO("pAttr 0x%p pAudioId 0x%p pStopBuf 0x%p\n",pAttr,pAudioId,pStopBuf);
    DEBUG_BUFFER_FMT(pAttr,sizeof(*pAttr),"AttrBuffer");
    DEBUG_BUFFER_FMT(pAudioId,sizeof(*pAudioId),"AudioId");
    DEBUG_BUFFER_FMT(pStopBuf,stopsize,"stopsize %d",stopsize);
    ret = pSysCp->SetData(pStopBuf,stopsize);
    if(ret < 0)
    {
        goto fail;
    }

    pComm = pSysCp->GetClientComm();
    if(pComm == NULL)
    {
        ret = -errno ? -errno : -1;
        goto fail;
    }

    DEBUG_BUFFER_FMT(pComm->m_Data,pComm->m_DataLen,"[%d] Format StopAudioDec",sock);
    ret = this->m_pSysReq->AddRequest(pComm);
    if(ret < 0)
    {
        goto fail;
    }

    SDK_ASSERT(pComm == NULL);

    return 0;
fail:
    FreeComm(pComm);
    return ret;
}

#endif /*AUDIO_DUAL_FILE_EMULATE*/



/***************************************
* to make sure we should set for the client
* return 1 for processing and will wait
* return 0 for no wait just delete
* return negative error code ,it is the big error ,should
*         stop the program
***************************************/
int SdkServerMgmt::__StartRequestReleaseDBuffer(int sock)
{
    int ret;

    if(this->m_pDBuffer && this->m_DBufferEnding == 0)
    {
        /*now we should start this*/
        if(this->m_pSysComm)
        {
            /*sock for the most big socket ,this will give it ok*/
            ret = this->__SendStopAudioDecodeRequest(sock);
            if(ret < 0)
            {
                return ret;
            }

            /*this may be for the*/
            ret = this->__StartReleaseDBufferTimer();
            if(ret < 0)
            {
                return ret;
            }

            /*indicate that we processing the DBuffer ending request*/
            this->m_DBufferEnding = 1;
            return 1;
        }

    }
    else if(this->m_pDBuffer && this->m_DBufferEnding == 1)
    {
        /*already has to call the dbuffering*/
        return 1;
    }

    return 0;
}


int SdkServerMgmt::HandleAudioStopCallBack(sdk_client_comm_t*& pComm)
{
    int ret,res;
    unsigned int i;
    SdkServerClient* pClient=NULL;
    /*this is the call back for mgmt ,not for the client, so we handled */
    uint32_t h32=0;
    uint16_t h16;
    std::auto_ptr<SdkSysCp> pSysCp2(new SdkSysCp());
    void* pData=NULL;
    uint32_t datalen=0;
    SysPkgAttrHeader* pAttr=NULL;
    SysPkgMessageCode *pMessageCode=NULL;
    std::auto_ptr<unsigned char> pDataHolder(new unsigned char[32]);
    SdkSysCp* pSysCp=pSysCp2.get();
    DEBUG_INFO("seqid 0x%08x\n",pComm->m_SeqId);
    if(pComm->m_SeqId == SDK_SERVER_MGMT_SOCK)
    {

        ret = pSysCp->ParsePkg(pComm);
        if(ret < 0)
        {
            DEBUG_BUFFER_FMT(pComm->m_Data,pComm->m_DataLen,"Parse Packege Error (%d)",ret);
            return 0;
        }

        ret = pSysCp->GetData(pData,datalen);
        if(ret < 0)
        {
            DEBUG_BUFFER_FMT(pComm->m_Data,pComm->m_DataLen,"Get Data Error (%d)",ret);
            return 0;
        }
        /*to hold the pointer ,and it will release when return*/
        pDataHolder.reset((unsigned char*)pData);

        pAttr =(SysPkgAttrHeader*) pData;
        if(datalen < (sizeof(*pAttr) + sizeof(*pMessageCode)))
        {
            DEBUG_BUFFER_FMT(pData,datalen,"datalen (%d) < (%d + %d)",datalen,sizeof(*pAttr),sizeof(*pMessageCode));
            return 0;
        }

        ret = pSysCp->GetAttrCount(h32);
        if(ret < 0)
        {
            ERROR_INFO("could not get attr count\n");
            return 0;
        }

        if(h32 != 1)
        {
            ERROR_INFO("attrcount(%d) != 1\n",h32);
            return 0;
        }

        h16 = PROTO_TO_HOST16(pAttr->s_Type);
        if(h16 != TYPE_MESSAGECODE)
        {
            ERROR_INFO("attr type(%d) != (%d)\n",h16,TYPE_MESSAGECODE);
            return 0;
        }

        h16 = PROTO_TO_HOST16(pAttr->s_Length);
        if(h16 != datalen)
        {
            ERROR_INFO("attrlength (%d) != datalen (%d)\n",h16,datalen);
            return 0;
        }

        pMessageCode = (SysPkgMessageCode*)((ptr_t)pAttr + sizeof(*pAttr));
        h32 = PROTO_TO_HOST32(pMessageCode->s_MessageCode);
        if(h32 != 0)
        {
            ERROR_INFO("not stop audio decoder error (%d)\n",h32);
            return 0;
        }

        this->__DirectReleaseDBuffer();

        FreeComm(pComm);
        return 1;
    }
    else
    {
        /*it is for the socket of client ,so we should change it to the client*/
        /*we have received ,so we delete the buffer*/
        this->__DirectReleaseDBuffer();
        for(i=0; i<this->m_pClientStreams.size() ; i++)
        {
            pClient = this->m_pClientStreams[i];
            ret = pClient->PushSysResp(pComm);
            if(ret < 0)
            {
                /*error for client ,so make it delete*/
                res = this->__RemoveSpecStreamClient(pClient);
                if(res < 0)
                {
                    ERROR_INFO("could not remove spec stream client 0x%p [%d]\n",pClient,pClient->GetSocket());
                    this->__BreakOut();
                }
                ERROR_INFO("stopaudio callback error in 0x%p[%d] error(%d)\n",pClient,pClient->GetSocket(),ret);
                /*if we met an error ,this pComm has been handled ,so do a break */
                break;
            }
            else if(ret > 0)
            {
                /*it is ok ,so we should test if pComm == NULL*/
                SDK_ASSERT(pComm == NULL);
                return 1;
            }
        }

        return 0;

    }

    return 0;
}


int SdkServerMgmt::HandleAudioStartCallBack(sdk_client_comm_t * & pComm)
{
    unsigned int i;
    int ret,res;
    SdkServerClient* pClient=NULL;
    for(i=0; i<this->m_pClientStreams.size(); i++)
    {
        pClient = this->m_pClientStreams[i];
        ret = pClient->PushSysResp(pComm);
        if(ret < 0)
        {
            res = this->__RemoveSpecStreamClient(pClient);
            if(res < 0)
            {
                ERROR_INFO("could not remove client 0x%p[%d] error(%d)\n",
                           pClient,pClient->GetSocket(),res);
                this->__BreakOut();
            }
            ERROR_INFO("could not start audio decode callback 0x%p[%d] error(%d)\n",
                       pClient,pClient->GetSocket(),ret);
            /*if we met error ,it means it handled it ,so we should do a break */
            break;
        }
        else if(ret > 0)
        {
            SDK_ASSERT(pComm == NULL);
            return 1;
        }
    }

    return 0;
}


/***************************************************
*  this function return value
*  1 for notify the start request for audio decoder
*  0 for not notify the start request for audio decoder
***************************************************/
int SdkServerMgmt::StartAudioDecoder(int sock,AudioDecParam * pAudioDec)
{
    int ret;
    if(this->m_pDBuffer && this->m_DBufferEnding == 1)
    {
        /*we are running in the decode buffering ending*/
        return -EPERM;
    }
    else if(this->m_pDBuffer)
    {
        ERROR_INFO("\n");
        return -EBUSY;
    }

    /*we should create the dbuffer ,and call not ready*/
    this->m_pDBuffer = new SdkAudioDBuffer(this->m_MaxPacks*2,this->m_pRunningBits);
    ret = this->m_pDBuffer->StartStream(pAudioDec);
    if(ret < 0)
    {
        ERROR_INFO("[%d]start stream %d error\n",sock,ret);
        /*delete directly ,and will not notify the sys_server*/
        this->__DirectReleaseDBuffer();
        return ret;
    }

    memcpy(&(this->m_AudioDecParam),pAudioDec,sizeof(*pAudioDec));
    ret = this->m_pDBuffer->RegisterSock(sock);
    if(ret < 0)
    {
        ERROR_INFO("[%d] register dbuffer error (%d)\n",sock,ret);
        this->__DirectReleaseDBuffer();
        return ret;
    }

    return 1;

}

int SdkServerMgmt::RequestAudioStart(SdkServerClient * pClient,AudioDecParam * pAudioDec)
{
    if(this->m_pDBuffer == NULL ||
            this->m_DBufferEnding == 1)
    {
        return -EPERM;
    }

    /*now we should test if add request*/

    return this->__SendStartAudioDecodeRequest(pClient->GetSocket(),pAudioDec);
}


/****************************************
1 for wait
0 for not wait
negative error code
****************************************/
int SdkServerMgmt::RequestAudioStop(SdkServerClient * pClient)
{
    int ret;
    if(this->m_pDBuffer == NULL)
    {
        return 0;
    }

    if(this->m_pDBuffer && this->m_DBufferEnding == 1)
    {
        return 1;
    }

    if(this->m_pSysComm == NULL)
    {
        return -ENODEV;
    }

    /*now to wait*/
    ret = this->__StartRequestReleaseDBuffer(pClient->GetSocket());
    if(ret < 0)
    {
        return ret;
    }

    /**/
    return ret;
}


/****************************************
1 for wait
0 for not wait
negative error code
****************************************/
int SdkServerMgmt::StopAudioDecoder(int sock)
{
    int ret;

    if(this->m_pDBuffer == NULL)
    {
        return 0;
    }

    if(this->m_DBufferEnding == 1)
    {
        /*some one delete wait until delete */
        return 1;
    }

    ret = this->m_pDBuffer->UnregisterSock(sock);
    if(ret < 0)
    {
        return ret;
    }

    this->__DirectReleaseDBuffer();
    return 0;
}

int SdkServerMgmt::PushAudioDecodeData(int sock,sdk_client_comm_t * & pComm)
{
    int ret;

    if(this->m_pDBuffer == NULL)
    {
        return -ENODEV;
    }

    if(this->m_pDBuffer  && this->m_DBufferEnding == 1)
    {
        return -EPERM;
    }

    /*now we should put the error*/
    ret = this->m_pDBuffer->PushDecodeData(sock,pComm);
    return ret;
}


#ifdef AUDIO_DUAL_FILE_EMULATE

int SdkServerMgmt::__StartStartAudioDualFileTimer()
{
    SDK_ASSERT(this->m_InsertStartAudioDualFileTimer == 0);
    ev_timer_init(&(this->m_EvStartAudioDualFileTimer),SdkServerMgmt::StartAudioDualFileTimerCallBack,1.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvStartAudioDualFileTimer));
    this->m_InsertStartAudioDualFileTimer = 1;
    return 0;
}

void SdkServerMgmt::__StopStartAudioDualFileTimer()
{
    if(this->m_InsertStartAudioDualFileTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvStartAudioDualFileTimer));
    }
    this->m_InsertStartAudioDualFileTimer = 0;
    return;
}

int SdkServerMgmt::__StartStopAudioDualFileTimer()
{
    SDK_ASSERT(this->m_InsertStopAudioDualFileTimer == 0);
    ev_timer_init(&(this->m_EvStopAudioDualFileTimer),SdkServerMgmt::StopAudioDualFileTimerCallBack,1.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvStopAudioDualFileTimer));
    this->m_InsertStopAudioDualFileTimer = 1;
    return 0;
}

void SdkServerMgmt::__StopStopAudioDualFileTimer()
{
    if(this->m_InsertStopAudioDualFileTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvStopAudioDualFileTimer));
    }
    this->m_InsertStopAudioDualFileTimer = 0;
    return;
}

int SdkServerMgmt::__StartAudioDualFileTimerImpl()
{
    int ret;
    sdk_client_comm_t *pComm=NULL;
    std::auto_ptr<SdkSysCp> pSysCp2(new SdkSysCp());
    SdkSysCp* pSysCp=pSysCp2.get();
    SysPkgAttrHeader* pAttr=NULL;
    SysPkgMessageCode* pMessageCode=NULL;
    int messagesize= sizeof(*pAttr) + sizeof(*pMessageCode);
    std::auto_ptr<unsigned char> pMessageBuf2(new unsigned char[messagesize]);
    unsigned char* pMessageBuf = pMessageBuf2.get();
    pAttr = (SysPkgAttrHeader*) pMessageBuf;
    pMessageCode = (SysPkgMessageCode*)((ptr_t)pAttr + sizeof(*pAttr));

    ret = pSysCp->SetCode(SYSCODE_START_AUDIO_DECODE_RSP);
    if(ret < 0)
    {
        goto fail;
    }

    ret = pSysCp->SetSessionSeq(SDK_SERVER_PRIVATE_SESID,this->m_StartSeqId);
    if(ret < 0)
    {
        goto fail;
    }

    /*now to give the attributes ok*/
    pAttr->s_Type = HOST_TO_PROTO16(TYPE_MESSAGECODE);
    pAttr->s_Length = HOST_TO_PROTO16(messagesize);
    pMessageCode->s_MessageLen = HOST_TO_PROTO32(0);
    pMessageCode->s_MessageCode = HOST_TO_PROTO32(0);

    ret = pSysCp->SetAttrCount(1);
    if(ret < 0)
    {
        goto fail;
    }

    ret = pSysCp->SetData(pMessageBuf,messagesize);
    if(ret < 0)
    {
        goto fail;
    }

    pComm = pSysCp->GetClientComm();
    if(pComm == NULL)
    {
        goto fail;
    }

    /*now for the data*/

    ret = this->HandleAudioStartCallBack(pComm);
    if(ret <=0)
    {
        goto fail;
    }
    SDK_ASSERT(pComm == NULL);
    return 1;
fail:
    FreeComm(pComm);
    return 0;
}

void SdkServerMgmt::StartAudioDualFileTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkServerMgmt* pThis = (SdkServerMgmt*)arg;
    int ret;

    ret = pThis->__StartAudioDualFileTimerImpl();
    if(ret < 0)
    {
        pThis->__BreakOut();
    }
    pThis->__StopStartAudioDualFileTimer();
    return ;
}

int SdkServerMgmt::__StopAudioDualFileTimerImpl()
{
    int ret;
    sdk_client_comm_t *pComm=NULL;
    std::auto_ptr<SdkSysCp> pSysCp2(new SdkSysCp());
    SdkSysCp* pSysCp=pSysCp2.get();
    SysPkgAttrHeader* pAttr=NULL;
    SysPkgMessageCode* pMessageCode=NULL;
    int messagesize= sizeof(*pAttr) + sizeof(*pMessageCode);
    std::auto_ptr<unsigned char> pMessageBuf2(new unsigned char[messagesize]);
    unsigned char* pMessageBuf = pMessageBuf2.get();
    pAttr = (SysPkgAttrHeader*) pMessageBuf;
    pMessageCode = (SysPkgMessageCode*)((ptr_t)pAttr + sizeof(*pAttr));

    ret = pSysCp->SetCode(SYSCODE_STOP_AUDIO_DECODE_RSP);
    if(ret < 0)
    {
        goto fail;
    }

    ret = pSysCp->SetSessionSeq(SDK_SERVER_PRIVATE_SESID,this->m_StopSeqId);
    if(ret < 0)
    {
        goto fail;
    }

    /*now to give the attributes ok*/
    pAttr->s_Type = HOST_TO_PROTO16(TYPE_MESSAGECODE);
    pAttr->s_Length = HOST_TO_PROTO16(messagesize);
    pMessageCode->s_MessageLen = HOST_TO_PROTO32(0);
    pMessageCode->s_MessageCode = HOST_TO_PROTO32(0);

    ret = pSysCp->SetAttrCount(1);
    if(ret < 0)
    {
        goto fail;
    }

    ret = pSysCp->SetData(pMessageBuf,messagesize);
    if(ret < 0)
    {
        goto fail;
    }

    pComm = pSysCp->GetClientComm();
    if(pComm == NULL)
    {
        goto fail;
    }

    /*now for the data*/

    ret = this->HandleAudioStopCallBack(pComm);
    if(ret <=0)
    {
        goto fail;
    }
    SDK_ASSERT(pComm == NULL);
    return 1;
fail:
    FreeComm(pComm);
    return 0;
}

void SdkServerMgmt::StopAudioDualFileTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkServerMgmt* pThis = (SdkServerMgmt*)arg;
    int ret;

    ret = pThis->__StopAudioDualFileTimerImpl();
    if(ret < 0)
    {
        pThis->__BreakOut();
    }
    pThis->__StopStopAudioDualFileTimer();
    return ;
}

#endif


int SdkServerMgmt::__ResetLongTimeTimer()
{
    int ret,res;
    unsigned int i;
    SdkServerClient* pClient=NULL;
    if(this->m_InsertReleaseDBufferTimer > 0)
    {
        this->__StopReleaseDBufferTimer();
        ret = this->__StartReleaseDBufferTimer();
        if(ret < 0)
        {
            /*we do not start release dbuffer rightly ,so delete this directly*/
            ERROR_INFO("reset ReleaseDBuffer Timer error(%d)\n",ret);
            this->__DirectReleaseDBuffer();
        }
    }

    for(i=0; i<this->m_pClientLogins.size(); i++)
    {
        pClient = this->m_pClientLogins[i];
        ret = pClient->ResetLongTimeTimer();
        if(ret < 0)
        {
            res = this->__RemoveSpecStreamClient(pClient);
            if(res < 0)
            {
                ERROR_INFO("[%d]could not remove [%d](0x%p) client error(%d)\n",i,
                           pClient->GetSocket(),
                           pClient,res);
                this->__BreakOut();
            }
            ERROR_INFO("[%d]could not reset login [%d](0x%p) client error(%d)\n",i,
                       pClient->GetSocket(),pClient,ret);
        }
    }

    for(i=0; i<this->m_pClientConfs.size(); i++)
    {
        pClient = this->m_pClientConfs[i];
        ret = pClient->ResetLongTimeTimer();
        if(ret < 0)
        {
            res = this->__RemoveSpecStreamClient(pClient);
            if(res < 0)
            {
                ERROR_INFO("[%d]could not remove [%d](0x%p) client error(%d)\n",i,
                           pClient->GetSocket(),
                           pClient,res);
                this->__BreakOut();
            }
            ERROR_INFO("[%d]could not reset login [%d](0x%p) client error(%d)\n",i,
                       pClient->GetSocket(),pClient,ret);
        }
    }

    for(i=0; i<this->m_pClientStreams.size() ; i++)
    {
        pClient = this->m_pClientStreams[i];
        ret = pClient->ResetLongTimeTimer();
        if(ret < 0)
        {
            res = this->__RemoveSpecStreamClient(pClient);
            if(res < 0)
            {
                ERROR_INFO("[%d]could not remove [%d](0x%p) client error(%d)\n",i,
                           pClient->GetSocket(),
                           pClient,res);
                this->__BreakOut();
            }
            ERROR_INFO("[%d]could not reset login [%d](0x%p) client error(%d)\n",i,
                       pClient->GetSocket(),pClient,ret);
        }
    }

    if(this->m_pSysReq)
    {
        ret = this->m_pSysReq->ResetLongTimeTimer();
        if(ret < 0)
        {
            ERROR_INFO("could not reset sysreq long timer error(%d)\n",ret);
            this->__BreakOut();
        }
    }

    if(this->m_pSysComm)
    {
        ret = this->m_pSysComm->ResetLongTimeTimer();
        if(ret < 0)
        {
            ERROR_INFO("could not reset syscomm long timer error(%d)\n",ret);
            this->__BreakOut();
        }
    }

    if(this->m_pLoginHandler)
    {
        ret = this->m_pLoginHandler->ResetLongTimeTimer();
        if(ret < 0)
        {
            ERROR_INFO("could not reset LoginHandler long timer error(%d)\n",ret);
            this->__BreakOut();
        }
    }

    if(this->m_pDaemon)
    {
        ret = this->m_pDaemon->ResetLongTimeTimer();
        if(ret < 0)
        {
            ERROR_INFO("could not reset Daemon long timer error(%d)\n",ret);
            this->__BreakOut();
        }
    }

    return 0;
}


int SdkServerMgmt::RegisterClientSession(sessionid_t sesid,SdkServerClient * pClient)
{
    unsigned int i;
    int ret;
    sessionid_t getsesid;
    privledge_t getpriv;
    SdkServerSession* pSession=NULL,*pFindSession=NULL;

    for(i=0; i<this->m_pSessions.size(); i++)
    {
        pSession = this->m_pSessions[i];
        ret = pSession->GetSessionId(getsesid,getpriv);
        if(ret < 0)
        {
            continue;
        }

        if(getsesid == sesid)
        {
            pFindSession = pSession;
            break;
        }
    }

    if(pFindSession == NULL)
    {
        ERROR_INFO("[%d]No session for %d\n",pClient->GetSocket(),sesid);
        return -ENODEV;
    }

    return pFindSession->RegisterServerSession(pClient);
}

