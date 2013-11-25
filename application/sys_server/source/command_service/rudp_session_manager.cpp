

#include "rudp_session_manager.h"
#include "gmi_rudp_api.h"
#include "gmi_system_headers.h"
#include "sys_env_types.h"
#include "log.h"


RUDPSessionManager::RUDPSessionManager( uint16_t LocalRudpCPort, uint16_t LocalRudpSPort, size_t SessionBufferSize )
    : m_LocalRudpCPort(LocalRudpCPort)
    , m_LocalRudpSPort(LocalRudpSPort)
    , m_SessionBufferSize(SessionBufferSize)
    , m_RudpSendSocket(NULL)
    , m_RudpRecvSocket(NULL)
    , m_ReceiveThread()
    , m_ThreadWorking(false)
    , m_ThreadExitFlag(false)
    , m_RUDP_Sessions()
    , m_CommandPipeline()
{
}

RUDPSessionManager::~RUDPSessionManager(void)
{
}


/*===============================================================
func name:Initialize
func:create rudp session and open
param:
return:
---------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/2/17      1.0                guqiang.lu           create
**********************************************************************************/
GMI_RESULT  RUDPSessionManager::Initialize( void_t *Argument, size_t ArgumentSize )
{
    m_RudpRecvSocket = GMI_RudpSocket((long_t)m_LocalRudpSPort);
    if (m_RudpRecvSocket == NULL)
    {
        fprintf(stderr, "[SYS_SERVER]"__FILE__ "%d GMI_RudpSocket error %d\n", __LINE__, errno);
        return GMI_FAIL;
    }

    m_RudpSendSocket = GMI_RudpSocket((long_t)m_LocalRudpCPort);
    if (m_RudpSendSocket == NULL)
    {
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}

/*===============================================================
func name:Deinitialize
func:close session
param:
return:
---------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/2/17      1.0                guqiang.lu           create
**********************************************************************************/
GMI_RESULT  RUDPSessionManager::Deinitialize()
{
    if (m_RudpRecvSocket != NULL)
    {
        GMI_RudpSocketClose(m_RudpRecvSocket);
        m_RudpRecvSocket = NULL;
    }

    if (m_RudpSendSocket != NULL)
    {
        GMI_RudpSocketClose(m_RudpSendSocket);
        m_RudpSendSocket = NULL;
    }

    std::map< uint64_t, ReferrencePtr<RUDPSession, DefaultObjectDeleter, MultipleThreadModel> >::iterator SessionIt = m_RUDP_Sessions.begin(), SessionEnd = m_RUDP_Sessions.end();
    for ( ; SessionIt != SessionEnd; ++SessionIt)
    {
        SessionIt->second->Close();
        SessionIt->second->Deinitialize();
    }

    m_RUDP_Sessions.clear();
    return GMI_SUCCESS;
}

/*===============================================================
func name:Start
func:create receiveThread and start
param:
return:
---------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/2/17      1.0                guqiang.lu           create
**********************************************************************************/
GMI_RESULT  RUDPSessionManager::Start( ReferrencePtr<BaseCommandPipelineManager> CommandPipeline )
{
    m_CommandPipeline = CommandPipeline;

    m_ThreadWorking  = false;
    m_ThreadExitFlag = false;
    GMI_RESULT Result = m_ReceiveThread.Create(NULL, 0, RUDPSessionManager::ReceiveThread, this);
    if (FAILED(Result))
    {
        fprintf(stderr, "[SYS_SERVER]"__FILE__ "%d m_ReceiveThread.Create error %d\n", __LINE__, errno);
        return Result;
    }

    Result = m_ReceiveThread.Start();
    if (FAILED(Result))
    {
        fprintf(stderr, "[SYS_SERVER]"__FILE__ "%d m_ReceiveThread.Start error %d\n", __LINE__, errno);
        m_ReceiveThread.Destroy();
        return Result;
    }

    return GMI_SUCCESS;
}

/*===============================================================
func name:Stop
func:stop receive data from rudp, stop submit commanPipeline to process
param:
return:
---------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/2/17      1.0                guqiang.lu           create
**********************************************************************************/
GMI_RESULT  RUDPSessionManager::Stop()
{
    m_ThreadExitFlag = true;
    while (m_ThreadWorking)
    {
        GMI_Sleep(100);
    }

    m_CommandPipeline = NULL;
    return GMI_SUCCESS;
}

ReferrencePtr<RUDPSession, DefaultObjectDeleter, MultipleThreadModel> RUDPSessionManager::GetUDPSession(int32_t index)
{
    std::map< uint64_t, ReferrencePtr<RUDPSession, DefaultObjectDeleter, MultipleThreadModel> >::iterator SessionIt = m_RUDP_Sessions.begin(), SessionEnd = m_RUDP_Sessions.end();
    for (int32_t i = 0; SessionIt != SessionEnd; ++SessionIt, ++i)
    {
        if (index == i)
        {
            return SessionIt->second;
        }
    }

    return ReferrencePtr<RUDPSession, DefaultObjectDeleter, MultipleThreadModel>(NULL);
}


/*===============================================================
func name:ReceiveThread
func:
param:
return:
---------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/2/17      1.0                guqiang.lu           create
**********************************************************************************/
void* RUDPSessionManager::ReceiveThread( void *Argument )
{
    RUDPSessionManager *Manager = reinterpret_cast<RUDPSessionManager*> ( Argument );
    return Manager->ReceiveEntry();
}


/*===============================================================
func name:ReceiveEntry
func:receive data from rudp, then submit commanPipeline to process
param:
return:success--return GMI_SUCCESSr,
    failed -- return GMI_FAIL
---------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/2/17      1.0                 guqiang.lu           create
**********************************************************************************/
void_t * RUDPSessionManager::ReceiveEntry()
{
    ReferrencePtr<uint8_t, DefaultObjectsDeleter>ReceiveBuffer(BaseMemoryManager::Instance().News<uint8_t>(RUDP_MAX_PACKAGE_SIZE));
    if (NULL == ReceiveBuffer.GetPtr())
    {
        SYS_ERROR("ReceiveBufferPtr News fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ReceiveBufferPtr News fail\n");
        return (void_t*)GMI_OUT_OF_MEMORY;
    }

    m_ThreadWorking = true;

    while (!m_ThreadExitFlag)
    {
        size_t               Transferred    = 0;
        PkgRudpRecvInput     RudpRecvInput  = {0};
        PkgRudpRecvOutput    RudpRecvOutput = {0};
        SdkPkgTransferProtocolKey SdkTransferProtocolKey;

        struct timeval Timeout;
        fd_set FdSet;
        Timeout.tv_sec  = 1;
        Timeout.tv_usec = 0;
        FD_ZERO(&FdSet);
        FD_SET((int32_t)m_RudpRecvSocket, &FdSet);
        int32_t Ret = select((int32_t)m_RudpRecvSocket+1, &FdSet, NULL, NULL, &Timeout);
        if (Ret <= 0)
        {
            continue;
        }

        RudpRecvInput.s_TimeoutMS = 5000;
        RudpRecvOutput.s_Buffer = ReceiveBuffer.GetPtr();
        RudpRecvOutput.s_BufferLength = RUDP_MAX_PACKAGE_SIZE;
        uint32_t Time1 = TimeStamp();
        GMI_RESULT Result = GMI_RudpRecv(m_RudpRecvSocket, &RudpRecvInput, &RudpRecvOutput);
        if (FAILED(Result))
        {
            continue;
        }
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "GMI_RudpRecv Packet length %d, waste Time %d\n", RudpRecvOutput.s_RecvLength, TimeStamp() - Time1);

        Transferred = RudpRecvOutput.s_RecvLength;
        memset(&SdkTransferProtocolKey, 0, sizeof(SdkPkgTransferProtocolKey));
        memcpy(SdkTransferProtocolKey.s_TransferKey, GMI_TRANSFER_KEY_TAG, GMI_TRANSFER_KEY_TAG_LENGTH);
        SdkTransferProtocolKey.s_AuthValue = RudpRecvOutput.s_AuthValue;
        SdkTransferProtocolKey.s_SeqNumber = RudpRecvOutput.s_SequenceNum;
        SdkTransferProtocolKey.s_SessionId = RudpRecvOutput.s_SessionId;

        //SYS_INFO("ReceiveEntry:s_AuthValue = %d\n", SdkTransferProtocolKey.s_AuthValue);
        //SYS_INFO("ReceiveEntry:s_SeqNumber = %d\n", SdkTransferProtocolKey.s_SeqNumber);
        //SYS_INFO("ReceiveEntry:s_SessionId = %d\n", SdkTransferProtocolKey.s_SessionId);

        uint64_t RUDPSessionKey = RudpRecvOutput.s_RemoteSvrPort;
        std::map< uint64_t, ReferrencePtr<RUDPSession, DefaultObjectDeleter, MultipleThreadModel> >::iterator SessionIt = m_RUDP_Sessions.find( RUDPSessionKey );
        if (m_RUDP_Sessions.end() == SessionIt)
        {
            ReferrencePtr<RUDPSession, DefaultObjectDeleter, MultipleThreadModel> RUDP_Session( BaseMemoryManager::Instance().New<RUDPSession>() );

            Result = RUDP_Session->Initialize();
            if (FAILED(Result))
            {
                RUDP_Session = NULL;
                break;
            }

            Result = RUDP_Session->Open( m_RudpSendSocket, RudpRecvOutput.s_RemoteSvrPort, m_SessionBufferSize );
            if (FAILED(Result))
            {
                RUDP_Session = NULL;
                break;
            }

            m_RUDP_Sessions.insert(std::pair<uint64_t, ReferrencePtr<RUDPSession, DefaultObjectDeleter, MultipleThreadModel> >( RUDPSessionKey, RUDP_Session) );
            SessionIt = m_RUDP_Sessions.find(RUDPSessionKey);
        }

        //add SdkPkgTransferProtocolKey to the tail of app packet
        size_t  BufferLen = Transferred + sizeof(SdkPkgTransferProtocolKey);
        memcpy(ReceiveBuffer.GetPtr()+Transferred, &SdkTransferProtocolKey, sizeof(SdkPkgTransferProtocolKey));

        Result = SessionIt->second->Receive(ReceiveBuffer.GetPtr(), BufferLen);
        if (FAILED(Result))
        {
            continue;
        }

        m_CommandPipeline->Parse(SessionIt->second);
    }

    ReceiveBuffer = NULL;
    m_ThreadWorking = false;
    return (void *) GMI_SUCCESS;
}


