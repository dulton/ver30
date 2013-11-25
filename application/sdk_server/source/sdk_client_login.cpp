
#include <sdk_client_login.h>
#include <auth_center_api.h>
#include <sdk_server_mgmt.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sdk_server_debug.h>

#define SEQNUM_CHECK
//#undef SEQNUM_CHECK

static struct timespec st_StartSpec;
static struct timespec st_EndSpec;

static int GetTickCount(struct timespec* pSpec)
{
	clock_gettime(CLOCK_MONOTONIC,pSpec);
	return 0;
}

SdkClientLogin::SdkClientLogin(SdkServerMgmt* pSvrMgmt,int maxclients ,int *pRunningBits)
    : m_pSvrMgmt(pSvrMgmt),
      m_pRunningBits(pRunningBits),
      m_MaxClients(maxclients)
{
    m_pSock = NULL;
    m_LPort = -1;
    m_RPort = -1;
    m_InsertReadIo = 0;
    m_InsertReadTimer = 0;
    m_InsertWriteIo  = 0;
    m_InsertWriteTimer = 0;
    m_InsertWaitTimer = 0;

    m_pSendingRequest = NULL;
    SDK_ASSERT(m_pSendRequest.size() == 0);
}

void SdkClientLogin::__ReleaseSendRequests()
{
    while(this->m_pSendRequest.size() > 0)
    {
        sdk_client_login_header_t* pHeader = this->m_pSendRequest[0];
        this->m_pSendRequest.erase(this->m_pSendRequest.begin());
        free(pHeader);
    }
    return ;
}

void SdkClientLogin::Stop()
{
    if (this->m_pSock)
    {
        delete this->m_pSock;
    }
    this->m_pSock = NULL;
    this->m_LPort = -1;
    this->m_RPort = -1;

    this->__StopReadIo();
    this->__StopReadTimer();
    this->__StopWriteIo();
    this->__StopWriteTimer();
    this->__StopWaitTimer();

    if (this->m_pSendingRequest)
    {
        free(this->m_pSendingRequest);
    }
    this->m_pSendingRequest = NULL;
    this->__ReleaseSendRequests();
    return ;
}



SdkClientLogin::~SdkClientLogin()
{
    this->Stop();
    this->m_pSvrMgmt = NULL;
    this->m_pRunningBits = NULL;
}


int SdkClientLogin::__GetRPort()
{
    return 51243;
}

int SdkClientLogin::__GetLPort()
{
    return 57012;
}

int SdkClientLogin::__SetSocketUnBlock(int sock)
{
    int ret;
    int flags;

    errno = 0;
    flags = fcntl(sock,F_GETFL);
    if (flags == -1 && errno)
    {
        ret = -errno;
        return ret;
    }

    ret = fcntl(sock,F_SETFL,flags | O_NONBLOCK);
    if (ret < 0)
    {
        ret = -errno ?   -errno :  -1;
        return ret;
    }
    return 0;
}


int SdkClientLogin::__BindSocket()
{
    int ret;
    int sock=-1;
    struct sockaddr_in saddr;
    socklen_t socklen;


    sock = socket(AF_INET,SOCK_DGRAM,0);
    if (sock < 0)
    {
        ret = -errno ? -errno : -1;
        goto fail;
    }

    ret  = this->__GetLPort();
    if (ret < 0)
    {
        goto fail;
    }
    this->m_LPort = ret;

    ret = this->__GetRPort();
    if (ret < 0)
    {
        goto fail;
    }
    this->m_RPort = ret;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    saddr.sin_port = htons(this->m_LPort);
    DEBUG_INFO("LPORT %d\n",this->m_LPort);

    socklen = sizeof(saddr);
    ret = bind(sock,(struct sockaddr*)&saddr,socklen);
    if (ret < 0)
    {
        goto fail;
    }

    ret = this->__SetSocketUnBlock(sock);
    if (ret < 0)
    {
        goto fail;
    }

    SDK_ASSERT(this->m_pSock==NULL);
    this->m_pSock = new SdkClientSock(sock);
    sock = -1;

    return 0;

fail:
    if (sock >= 0)
    {
        close(sock);
    }
    sock = -1;
    return ret;
}

void SdkClientLogin::__StopReadIo()
{
    if (this->m_InsertReadIo)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvReadIo));
    }
    this->m_InsertReadIo = 0;
    return;
}

int SdkClientLogin::__StartReadIo()
{
    SDK_ASSERT(this->m_InsertReadIo == 0);
    SDK_ASSERT(this->m_pSock);
    ev_io_init(&(this->m_EvReadIo),SdkClientLogin::ReadIoCallBack,this->m_pSock->Socket(),EV_READ,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvReadIo));
    this->m_InsertReadIo = 1;
    return 0;
}

void SdkClientLogin::__StopReadTimer()
{
    if (this->m_InsertReadTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvReadTimer));
    }
    this->m_InsertReadTimer = 0;
    return ;
}

int SdkClientLogin::__StartReadTimer()
{
    SDK_ASSERT(this->m_InsertReadTimer == 0);
    ev_timer_init(&(this->m_EvReadTimer),SdkClientLogin::ReadTimerCallBack,3.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvReadTimer));
    this->m_InsertReadTimer = 1;
    return 0;
}

void SdkClientLogin::__StopWriteIo()
{
    if (this->m_InsertWriteIo)
    {
        ev_io_stop(EV_DEFAULT,&(this->m_EvWriteIo));
    }
    this->m_InsertWriteIo = 0;
    return ;
}

int SdkClientLogin::__StartWriteIo()
{
    SDK_ASSERT(this->m_InsertWriteIo == 0);
    SDK_ASSERT(this->m_pSock);

    ev_io_init(&(this->m_EvWriteIo),SdkClientLogin::WriteIoCallBack,this->m_pSock->Socket(),EV_WRITE,this);
    ev_io_start(EV_DEFAULT,&(this->m_EvWriteIo));
    this->m_InsertWriteIo = 1;
    return 0;
}


void SdkClientLogin::__StopWriteTimer()
{
    if (this->m_InsertWriteTimer)
    {
        ev_timer_stop(EV_DEFAULT,&(this->m_EvWriteTimer));
    }
    this->m_InsertWriteTimer = 0;
    return ;
}

int SdkClientLogin::__StartWriteTimer()
{
    SDK_ASSERT(this->m_InsertWriteTimer == 0);
    ev_timer_init(&(this->m_EvWriteTimer),SdkClientLogin::WriteTimerCallBack,3.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvWriteTimer));
    this->m_InsertWriteTimer = 1;
    return 0;
}

void SdkClientLogin::__StopWaitTimer()
{
    if (this->m_InsertWaitTimer)
    {		
        ev_timer_stop(EV_DEFAULT,&(this->m_EvWaitTimer));
		GetTickCount(&st_EndSpec);
    }
    this->m_InsertWaitTimer = 0;
    return ;
}

int SdkClientLogin::__StartWaitTimer()
{
    SDK_ASSERT(this->m_InsertWaitTimer == 0);
	GetTickCount(&st_StartSpec);
    ev_timer_init(&(this->m_EvWaitTimer),SdkClientLogin::WaitTimerCallBack,5.0,0.0,this);
    ev_timer_start(EV_DEFAULT,&(this->m_EvWaitTimer));
	this->m_InsertWaitTimer = 1;
    return 0;
}


void SdkClientLogin::__BreakOut()
{
	BACK_TRACE();
    ev_break (EV_DEFAULT,EVBREAK_ONE);
}

void SdkClientLogin::WriteTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkClientLogin* pThis =(SdkClientLogin*) arg;
    pThis->__BreakOut();
    return ;
}

void SdkClientLogin::ReadTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkClientLogin* pThis =(SdkClientLogin*) arg;
    pThis->__BreakOut();
    return ;
}

int SdkClientLogin::__FormatSendingRequest()
{
    sdk_client_comm_t* pComm=NULL;
    sdk_client_login_request_t *pLoginReq;
    sdk_client_logout_request_t *pLogoutReq;
    UserAuthRefInfo *pAuthInfo;
    int extralen=0;
    int ret;

    pComm = (sdk_client_comm_t*)calloc(sizeof(*pComm),1);
    if (pComm == NULL)
    {
        return -ENOMEM;
    }

    /*these are the filling one*/
    pComm->m_SesId = 0;
    pComm->m_Priv = 0;
    pComm->m_ServerPort = this->m_RPort;
    pComm->m_SeqId = this->m_pSendingRequest->m_ReqNum;
    /*we want request type ,so do this*/
    pComm->m_Type = 0;
    pComm->m_Frag = 0;
    pComm->m_DataId = 0;
    pComm->m_Offset = 0;
    pComm->m_Totalsize = 0;

    switch(this->m_pSendingRequest->m_OpCode)
    {
    case LOGIN_CLIENT_REQUEST:
        /*now format login request*/
        pLoginReq = (sdk_client_login_request_t*) this->m_pSendingRequest;
        pAuthInfo = (UserAuthRefInfo*) pComm->m_Data;
        pAuthInfo->s_DataType = INNER_HOST_TO_PROTO32(TYPE_AUTH_LOGIN);
        strncpy(pAuthInfo->s_Username,(const char*)pLoginReq->m_Username,sizeof(pAuthInfo->s_Username)-1);
        pAuthInfo->s_UsernameEncType = TYPE_ENCRYPTION_TEXT;
		/*password is 32 bytes*/
        memcpy(pAuthInfo->s_Password,(const char*)pLoginReq->m_Password,DES_PASSWORD_LENGTH);
        pAuthInfo->s_PasswordEncType = TYPE_ENCRYPTION_DES;
        pAuthInfo->s_SessionId = INNER_HOST_TO_PROTO16(0);
        pAuthInfo->s_SingleUserMaxLinkNum = INNER_HOST_TO_PROTO16(this->m_MaxClients);
        pAuthInfo->s_AllUserMaxLinkNum = INNER_HOST_TO_PROTO16(this->m_MaxClients);
		pAuthInfo->s_MoudleId = ID_MOUDLE_REST_SDK;


		/*key length is 8*/
        extralen = DES_KEY_LENGTH;
        if ((extralen + sizeof(*pAuthInfo)) >= (sizeof(pComm->m_Data) - 128))
        {
            extralen = sizeof(pComm->m_Data) - 128;
        }
        pAuthInfo->s_UserAuthExtDataLen = INNER_HOST_TO_PROTO16(extralen);
        memcpy(&(pComm->m_Data[sizeof(*pAuthInfo)]),pLoginReq->m_Salt,extralen);
        pComm->m_DataLen = extralen + sizeof(*pAuthInfo);
		//DEBUG_BUFFER_FMT(&(pComm->m_Data[sizeof(*pAuthInfo)]),extralen,"passkey %d",extralen);
		//DEBUG_BUFFER_FMT((unsigned char*)pAuthInfo->s_Password,DES_PASSWORD_LENGTH, "password (%d)",DES_PASSWORD_LENGTH);
        break;
    case LOGOUT_CLIENT_REQUEST:
        pLogoutReq = (sdk_client_logout_request_t*) this->m_pSendingRequest;
        pAuthInfo = (UserAuthRefInfo*) pComm->m_Data;
        memset(pAuthInfo,0,sizeof(*pAuthInfo));
        pAuthInfo->s_DataType = INNER_HOST_TO_PROTO32(TYPE_AUTH_LOGOUT);
        pAuthInfo->s_SessionId = INNER_HOST_TO_PROTO16(pLogoutReq->m_SesId);
		pAuthInfo->s_MoudleId = ID_MOUDLE_REST_SDK;
        pComm->m_DataLen = sizeof(*pAuthInfo);

        /*this would for session id */
        pComm->m_SesId = pLogoutReq->m_SesId;
        break;
    default:
        SDK_ASSERT(0!=0);
        free(pComm);
        return -EFAULT;
    }

    ret = this->__StartWriteIo();
    if (ret < 0)
    {
        /*we insert pComm ,so do not free it again*/
        free(pComm);
        pComm = NULL;
        return ret;
    }

    ret = this->__StartWriteTimer();
    if (ret < 0)
    {
        free(pComm);
        pComm = NULL;
        this->__StopWriteIo();
        return ret;
    }

	pComm->m_ServerPort = this->m_RPort;
	pComm->m_LocalPort = this->m_LPort;
    ret = this->m_pSock->PushData(pComm);
    if (ret < 0)
    {
        free(pComm);
        this->__StopWriteIo();
        this->__StopWriteTimer();
        return ret;
    }

    pComm = NULL;

    return 0;
}

int SdkClientLogin::__HandleLoginResp(sdk_client_comm_t * pComm)
{
    int ret=-1;
    UserAuthResInfo *pAuthResInfo=NULL;
    sessionid_t sesid;
    privledge_t priv;
    uint16_t h16;

	DEBUG_INFO("\n");
    ret = ret;
    pAuthResInfo = (UserAuthResInfo*)&(pComm->m_Data);
    if (pComm->m_DataLen < sizeof(*pAuthResInfo))
    {
        /*it is not the one we need ,so inform the user login ,it is not invalid*/
        ret = this->m_pSvrMgmt->UserLoginSessionCallBack(this->m_pSendingRequest->m_Sock,this->m_pSendingRequest->m_ReqNum,EINVAL,
                0,0);
        return 0;
    }


    h16 = INNER_PROTO_TO_HOST16(pAuthResInfo->s_AuthResult);
    if (h16 != GMI_CODE_SUCCESS)
    {
		ERROR_INFO("sock[%d] reqnum [%d] s_AuthResult %d\n",
			this->m_pSendingRequest->m_Sock,
			this->m_pSendingRequest->m_ReqNum,
			h16);
        ret = this->m_pSvrMgmt->UserLoginSessionCallBack(this->m_pSendingRequest->m_Sock,this->m_pSendingRequest->m_ReqNum,
                EPERM,0,0);
        return 0;
    }

    sesid = INNER_PROTO_TO_HOST16(pAuthResInfo->s_SessionId);
    priv = INNER_PROTO_TO_HOST32(pAuthResInfo->s_AuthValue);
    ret = this->m_pSvrMgmt->UserLoginSessionCallBack(this->m_pSendingRequest->m_Sock,this->m_pSendingRequest->m_ReqNum,
            0,sesid,priv);
    return 0;
}

int SdkClientLogin::__HandleLogoutResp(sdk_client_comm_t * pComm)
{
    UserAuthResInfo* pAuthResInfo;
    int ret=-1;
    uint16_t h16;
	DEBUG_INFO("\n");

    ret = ret;
    pAuthResInfo = (UserAuthResInfo*)pComm->m_Data;

    if (pComm->m_DataLen < sizeof(*pAuthResInfo))
    {
        ret = this->m_pSvrMgmt->UserLogoutSessionCallBack(this->m_pSendingRequest->m_Sock,this->m_pSendingRequest->m_ReqNum,EFAULT,
                pComm->m_SesId);
        return 0;
    }

    h16 = INNER_PROTO_TO_HOST16(pAuthResInfo->s_AuthResult);
    if (h16 != GMI_CODE_SUCCESS)
    {
        ret = this->m_pSvrMgmt->UserLogoutSessionCallBack(this->m_pSendingRequest->m_Sock,this->m_pSendingRequest->m_ReqNum,
                EPERM,pComm->m_SesId);
        return 0;
    }

    ret = this->m_pSvrMgmt->UserLogoutSessionCallBack(this->m_pSendingRequest->m_Sock,this->m_pSendingRequest->m_ReqNum,0,pComm->m_SesId);
    return 0;
}


int SdkClientLogin::__ReadIoImpl()
{
    int ret;
    sdk_client_comm_t* pComm=NULL;
#ifdef SEQNUM_CHECK	
    uint16_t expectid;
#endif

    ret = this->m_pSock->Read();
    if (ret < 0)
    {
        return ret;
    }
    else if (ret == 0)
    {
        if(this->m_InsertReadTimer == 0)
        {
            ret = this->__StartReadTimer();
            if (ret < 0)
            {
                return ret;
            }
        }
        return 0;
    }

    SDK_ASSERT(ret > 0);
    /*we read one packet ,so stop the read timer*/

    this->__StopReadTimer();
    pComm = this->m_pSock->GetRead();
    SDK_ASSERT(pComm);
    this->m_pSock->ClearRead();

    /*now to handle the job test if this is the response for just sending one*/
    if (this->m_pSendingRequest == NULL)
    {
        /*nothing to wait ,so we should do this free*/
        FreeComm(pComm);
        return 0;
    }


#ifdef SEQNUM_CHECK
    expectid = this->m_pSendingRequest->m_ReqNum;
    if (expectid != pComm->m_SeqId)
    {
        /*not the expect id we want ,so this is not one*/
        FreeComm(pComm);
        return 0;
    }
#endif

	DEBUG_INFO("OPCODE %d\n",this->m_pSendingRequest->m_OpCode);
    /*ok we check if the type is ok*/
    switch(this->m_pSendingRequest->m_OpCode)
    {
    case LOGIN_CLIENT_REQUEST:
        ret = this->__HandleLoginResp(pComm);
        break;
    case LOGOUT_CLIENT_REQUEST:
        ret = this->__HandleLogoutResp(pComm);
        break;
    default:
        SDK_ASSERT(0!=0);
        free(pComm);
        return -EFAULT;
    }
    /*we do not need the reading buffer*/
    free(pComm);
    pComm = NULL;
    if (ret < 0)
    {
        return ret;
    }

    /*now all is ok handled ,so we need to transfer the next one*/
    free(this->m_pSendingRequest);
    this->m_pSendingRequest = NULL;
    this->__StopWaitTimer();
	//DEBUG_INFO("Start %ld:%ld End %ld:%ld\n",st_StartSpec.tv_sec,st_StartSpec.tv_nsec,
	//	st_EndSpec.tv_sec,st_EndSpec.tv_nsec);
    this->__StopWriteIo();
    this->__StopWriteTimer();

    if (this->m_pSendRequest.size() > 0)
    {
    	this->m_pSendingRequest = this->m_pSendRequest[0];
		this->m_pSendRequest.erase(this->m_pSendRequest.begin());
        ret = this->__FormatSendingRequest();
        if (ret < 0)
        {
        	/*if failed ,it just because ,no memory ,so we should just make the break out exit*/
            return ret;
        }
    }

    return 1;


}

void SdkClientLogin::ReadIoCallBack(EV_P_ ev_io * w,int revents,void * arg)
{
    SdkClientLogin* pThis =(SdkClientLogin*) arg;
    int ret;
    ret = pThis->__ReadIoImpl();
    if (ret < 0)
    {
        pThis->__BreakOut();
    }
    return ;
}

int SdkClientLogin::__WriteIoImpl()
{
    int ret;

    ret = this->m_pSock->IsWriteSth();
    if (ret == 0)
    {
        /*nothing to write ,just return*/
        this->m_pSock->ClearWrite();
        this->__StopWriteIo();
        this->__StopWriteTimer();
        ret = this->__StartWaitTimer();
        if (ret < 0)
        {
            return ret;
        }
        return 0;
    }

    ret = this->m_pSock->Write();
    if (ret <0)
    {
        return ret;
    }
    else if (ret == 0)
    {
        return 0;
    }

    SDK_ASSERT(ret > 0);
    /*write things ok ,so we should stop write and wait call back*/
    this->m_pSock->ClearWrite();
    this->__StopWriteIo();
    this->__StopWriteTimer();
    ret = this->__StartWaitTimer();
    if (ret < 0)
    {
        return ret;
    }
    return 1;
}




void SdkClientLogin::WriteIoCallBack(EV_P_ ev_io * w,int revents,void * arg)
{
    SdkClientLogin* pThis =(SdkClientLogin*) arg;
    int ret;
    ret = pThis->__WriteIoImpl();
    if (ret < 0)
    {
        pThis->__BreakOut();
    }
    return ;
}

int SdkClientLogin::__WaitTimerImpl()
{
    int ret;
    sdk_client_logout_request_t* pLogoutReq;
    /*
          because, only way to enter this function ,when we wait for the response timeout
          it must send all the things ,so we make sure this is true
    */
    SDK_ASSERT(this->m_pSock->IsWriteSth() == 0);
    SDK_ASSERT(this->m_pSendingRequest);

    /*we call timed out ,and nothing return back notice ,it will ok*/
    switch (this->m_pSendingRequest->m_OpCode )
    {
    case LOGIN_CLIENT_REQUEST:
        ret = this->m_pSvrMgmt->UserLoginSessionCallBack(this->m_pSendingRequest->m_Sock,this->m_pSendingRequest->m_ReqNum,
                ETIMEDOUT,0,0);
        break;
    case LOGOUT_CLIENT_REQUEST:
        pLogoutReq = (sdk_client_logout_request_t*)this->m_pSendingRequest;
        ret = this->m_pSvrMgmt->UserLogoutSessionCallBack(this->m_pSendingRequest->m_Sock,
                this->m_pSendingRequest->m_ReqNum,ETIMEDOUT,pLogoutReq->m_SesId);
        break;
    default:
        SDK_ASSERT(0!= 0);
        ret = -EINVAL;
        break;
    }

    /*now we remove the job*/
    free(this->m_pSendingRequest);
    this->m_pSendingRequest = NULL;
    /*because the write io and timer are stopped ,when it will start for wait timer*/
    SDK_ASSERT(this->m_InsertWriteIo == 0);
    SDK_ASSERT(this->m_InsertWriteTimer == 0);
    this->__StopWaitTimer();
	DEBUG_INFO("Start %ld:%ld End %ld:%ld\n",st_StartSpec.tv_sec,st_StartSpec.tv_nsec,
		st_EndSpec.tv_sec,st_EndSpec.tv_nsec);
    if (this->m_pSendRequest.size() > 0)
    {
        this->m_pSendingRequest = this->m_pSendRequest[0];
        this->m_pSendRequest.erase(this->m_pSendRequest.begin());
        ret = this->__FormatSendingRequest();
        if (ret < 0)
        {
            return ret;
        }
    }

    return 0;
}

void SdkClientLogin::WaitTimerCallBack(EV_P_ ev_timer * w,int revents,void * arg)
{
    SdkClientLogin* pThis =(SdkClientLogin*) arg;
    int ret;
    ret = pThis->__WaitTimerImpl();
    if (ret < 0)
    {
        pThis->__BreakOut();
    }
    return ;
}

int SdkClientLogin::Start()
{
    int ret;
    this->Stop();

    ret =this->__BindSocket();
    if (ret < 0)
    {
        this->Stop();
        return ret;
    }

    /*we should start read io ,because this will give the job ok*/
    ret = this->__StartReadIo();
    if (ret < 0)
    {
        this->Stop();
        return ret;
    }

    return 0;
}

int SdkClientLogin::AddUserLoginRequest(SdkServerClient * pClient,int reqnum,const char * username,const char * salt,const char * md5check)
{
    sdk_client_login_request_t* pReq=NULL;
    int ret;
    if (pClient == NULL || username == NULL || md5check == NULL || salt == NULL)
    {
        return -EINVAL;
    }
    /*now we should take the */
    pReq = (sdk_client_login_request_t*)calloc(sizeof(*pReq),1);
    if (pReq == NULL)
    {
        return -ENOMEM;
    }

    pReq->m_Header.m_Sock = pClient->GetSocket();
    pReq->m_Header.m_OpCode = LOGIN_CLIENT_REQUEST;
    pReq->m_Header.m_ReqNum = reqnum;
    pReq->m_Header.m_BodyLength = sizeof(*pReq) - sizeof(pReq->m_Header);

    strncpy((char*)pReq->m_Username,username,sizeof(pReq->m_Username)-1);
    memcpy((char*)pReq->m_Password,md5check,DES_PASSWORD_LENGTH);
    memcpy((char*)pReq->m_Salt,salt,DES_KEY_LENGTH);

    if (this->m_pSendingRequest)
    {
        /*some one wait for the request*/
        this->m_pSendRequest.push_back((sdk_client_login_header_t*)pReq);
        return 0;
    }

    /*no one wait ,just send*/
    this->m_pSendingRequest = (sdk_client_login_header_t*)pReq;
    ret = this->__FormatSendingRequest();
    if (ret < 0)
    {
        this->m_pSendingRequest = NULL;
        free(pReq);
        return ret;
    }

    return 1;
}


int SdkClientLogin::AddUserLogoutRequest(SdkServerClient * pClient,int reqnum,sessionid_t sesid,privledge_t priv)
{
    int ret;
    sdk_client_logout_request_t* pReq=NULL;
    int sock;
    if (pClient == NULL)
    {
        sock = 0;
    }
    else
    {
        sock = pClient->GetSocket();
    }

    pReq = (sdk_client_logout_request_t*)calloc(sizeof(*pReq),1);
    if (pReq == NULL)
    {
        return -ENOMEM;
    }

    pReq->m_Header.m_Sock = sock;
    pReq->m_Header.m_OpCode = LOGOUT_CLIENT_REQUEST;
    pReq->m_Header.m_ReqNum = reqnum;
    pReq->m_Header.m_BodyLength = sizeof(*pReq) - sizeof(pReq->m_Header);

    pReq->m_Priv = priv;
    pReq->m_SesId = sesid;

    if (this->m_pSendingRequest)
    {
        this->m_pSendRequest.push_back((sdk_client_login_header_t*)pReq);
        return 0;
    }

    this->m_pSendingRequest = (sdk_client_login_header_t*)pReq;
    ret = this->__FormatSendingRequest();
    if (ret < 0)
    {
        this->m_pSendingRequest = NULL;
        free(pReq);
        return ret;
    }

    return 1;
}


int SdkClientLogin::__WriteCommandSync(time_t etime)
{
    fd_set wset;
    int sockfd;
    time_t curtime;
    struct timeval tm;
    int ret;
    sockfd = this->m_pSock->Socket();
write_flush_again:
    FD_ZERO(&wset);
    FD_SET(sockfd,&wset);
    curtime = time(NULL);
    if (curtime >= etime)
    {
        ret = -ETIMEDOUT;
        goto fail;
    }
    tm.tv_sec = etime - curtime;
    tm.tv_usec = 0;
    ret = select(sockfd + 1,NULL,&wset,NULL, &tm);
    if (ret < 0)
    {
        if (errno == EINTR)
        {
            goto write_flush_again;
        }
        ret = -errno;
        goto fail;
    }
    else if (ret == 0)
    {
        ret = -ETIMEDOUT;
        goto fail;
    }

    ret = this->m_pSock->Write();
    if (ret < 0)
    {
        goto fail;
    }
    else if (ret == 0)
    {
        goto write_flush_again;
    }

    SDK_ASSERT(ret > 0);
    this->m_pSock->ClearWrite();
    return 0;
fail:
    return ret;
}

int SdkClientLogin::__ReadCommandSync(time_t etime)
{
    fd_set rset;
    int sockfd;
    time_t curtime;
    struct timeval tm;
    int ret;
    sockfd = this->m_pSock->Socket();
    ret = this->m_pSock->Read();
    if(ret > 0)
    {
        return 0;
    }
    else if (ret < 0)
    {
        return ret;
    }
read_again:
    FD_ZERO(&rset);
    FD_SET(sockfd,&rset);
    curtime = time(NULL);
    if (curtime >= etime)
    {
        ret =-ETIMEDOUT;
        goto fail;
    }
    tm.tv_sec = 1;
    tm.tv_usec = 0;
    DEBUG_INFO("sockfd %d\n",sockfd);
    ret = select(sockfd + 1,&rset,NULL,NULL, &tm);
    if (ret < 0)
    {
        if (this->m_pRunningBits && *(this->m_pRunningBits) == 0)
        {
            ret = -EPIPE;
            goto fail;
        }
        if (errno == EINTR)
        {
            goto read_again;
        }
        ERROR_INFO("\n");
        ret = -errno;
        goto fail;
    }
    else if (ret == 0)
    {
        curtime = time(NULL);
        if (curtime >= etime)
        {
            ret =-ETIMEDOUT;
            goto fail;
        }
        if (this->m_pRunningBits && *(this->m_pRunningBits) == 0)
        {
            ret = -EPIPE;
            goto fail;
        }
    }
    ERROR_INFO("\n");

    ret = this->m_pSock->Read();
    if (ret < 0)
    {
        goto fail;
    }
    else if (ret == 0)
    {
        goto read_again;
    }

    SDK_ASSERT(ret > 0);
    return 0;
fail:
    return ret;
}

/***************************************
*
*   this function is the function is synchronous function
*   it will remove the users
***************************************/
int SdkClientLogin::UserLogoutClear(int reqnum,int timeout)
{
    int ret;
    sdk_client_comm_t *pComm=NULL;
    UserAuthRefInfo *pAuth;
    UserAuthResInfo *pResp;
    uint32_t h32;
    time_t stime,etime;

    if (this->m_pSock == NULL)
    {
        return -EINVAL;
    }

    stime = time(NULL);
    etime = stime + timeout;

    /*now first we should do the job as send the message*/
    pComm =(sdk_client_comm_t*) calloc(sizeof(*pComm),1);
    if (pComm == NULL)
    {
        return -ENOMEM;
    }
	DEBUG_INFO("reqnum %d\n",reqnum);
    pComm->m_SesId = 0;
    pComm->m_Priv =0;
    pComm->m_SeqId = reqnum;
    pComm->m_Type = GMIS_PROTOCOL_TYPE_LOGGIN;
    pComm->m_Frag = 0;
    pComm->m_DataId = 0;
    pComm->m_Offset = 0;
    pComm->m_Totalsize = 0;
    pAuth = (UserAuthRefInfo*)pComm->m_Data;
    pComm->m_DataLen = sizeof(*pAuth);

    pAuth->s_DataType = INNER_HOST_TO_PROTO32(TYPE_AUTH_CLEAR_ALL_USER);
	pAuth->s_MoudleId = ID_MOUDLE_REST_SDK;


    if (this->m_pSock->IsWriteSth())
    {
        ret = this->__WriteCommandSync(etime);
        if (ret < 0)
        {
            goto fail;
        }
    }
    pComm->m_ServerPort = this->m_RPort;
    pComm->m_LocalPort = this->m_LPort;
    /*now we should do the request job */
    ret = this->m_pSock->PushData(pComm);
    if (ret < 0)
    {
        goto fail;
    }
    /*transfer control to the pSock*/
    pComm = NULL;
    ret = this->__WriteCommandSync(etime);
    if (ret < 0)
    {
        goto fail;
    }


#ifdef  SEQNUM_CHECK
read_comm_again:
#endif
    ret = this->__ReadCommandSync(etime);
    if (ret < 0)
    {
        goto fail;
    }

    pComm = this->m_pSock->GetRead();
    SDK_ASSERT(pComm);

    DEBUG_INFO("seqid %d\n",pComm->m_SeqId);
#ifdef SEQNUM_CHECK	
    if (!((pComm->m_SeqId == (reqnum))))
    {
        goto read_comm_again;
    }
#endif
    if (pComm->m_DataLen < sizeof(*pResp))
    {
    	ERROR_INFO("\n");
        ret = -EINVAL;
        goto fail;
    }

    pResp = (UserAuthResInfo*) pComm->m_Data;
    h32 = INNER_PROTO_TO_HOST32(pResp->s_DataType);
    if (h32 != TYPE_AUTH_CLEAR_ALL_USER)
    {
    	ERROR_INFO("\n");
        ret = -EINVAL;
        goto fail;
    }

    h32 = INNER_PROTO_TO_HOST32(pResp->s_AuthResult);
    if (h32 != 0)
    {
        ret = -EFAULT;
    	ERROR_INFO("\n");
        goto fail;
    }

    free(pComm);
    pComm = NULL;


    return 0;

fail:
    if (pComm)
    {
        free(pComm);
    }
    pComm = NULL;
    return ret;
}

int SdkClientLogin::ResetLongTimeTimer()
{
	int ret;

	if (this->m_InsertReadTimer > 0)
	{
		this->__StopReadTimer();
		ret = this->__StartReadTimer();
		if (ret < 0)
		{
			return ret;
		}
	}

	if (this->m_InsertWriteTimer > 0)
	{
		this->__StopWriteTimer();
		ret = this->__StartWriteTimer();
		if (ret < 0)
		{
			return ret;
		}
	}

	if (this->m_InsertWaitTimer > 0)
	{
		this->__StopWaitTimer();
		ret = this->__StartWaitTimer();
		if (ret < 0)
		{
			return ret;
		}
	}
	return 0;
}

