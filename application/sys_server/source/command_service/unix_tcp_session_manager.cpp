#include <stdio.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <errno.h>
#include "sys_close_unix_tcp_cmd.h"
#include "unix_tcp_session_manager.h"
#include "gmi_rudp_api.h"
#include "gmi_rudp.h"
#include "gmi_system_headers.h"
#include "sys_env_types.h"
#include "log.h"

typedef struct
{
    int32_t s_Fd;
    UnixTcpSessionManager  *s_UnTcpSessionManagerPtr;
} SessionManagerParam;

UnixTcpSessionManager::UnixTcpSessionManager(size_t SessionBufferSize )
    : m_SessionBufferSize(SessionBufferSize)
    , m_ListenFd(-1)
    , m_ServerThread()
    , m_ThreadWorking(false)
    , m_ThreadExitFlag(false)
    , m_CommandPipeline()
{
}

UnixTcpSessionManager::~UnixTcpSessionManager(void)
{
}


GMI_RESULT UnixTcpSessionManager::Listen(const char_t* Name, int32_t *FdPtr)
{
    int32_t Fd, Len, Result;
    struct sockaddr_un SockUn;

    Fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (0 > Fd)
    {
        return GMI_FAIL;
    }

    unlink(Name);

    memset(&SockUn, 0, sizeof(SockUn));
    SockUn.sun_family = AF_UNIX;
    strcpy(SockUn.sun_path, Name);
    Len = Offsetof(struct sockaddr_un, sun_path) + strlen(Name);

    Result = bind(Fd, (struct sockaddr*)&SockUn, Len);
    if (0 > Result)
    {
        if (0 < Fd)
        {
            close(Fd);
            Fd = -1;
        }
        return GMI_FAIL;
    }

    Result = listen(Fd, QNUM);
    if (0 > Result)
    {
        if (0 < Fd)
        {
            close(Fd);
            Fd = -1;
        }
        return GMI_FAIL;
    }

    *FdPtr = Fd;
    return GMI_SUCCESS;
}


GMI_RESULT  UnixTcpSessionManager::Initialize( void_t *Argument, size_t ArgumentSize )
{
    int32_t ListenFd;
    GMI_RESULT Result = Listen(UNIX_SERVER_PATH, &ListenFd);
    if (FAILED(Result))
    {
        return Result;
    }
    m_ListenFd = ListenFd;

    return GMI_SUCCESS;
}


GMI_RESULT  UnixTcpSessionManager::Deinitialize()
{
    if (0 < m_ListenFd)
    {
        close(m_ListenFd);
    }

    return GMI_SUCCESS;
}


GMI_RESULT  UnixTcpSessionManager::Start( ReferrencePtr<BaseCommandPipelineManager> CommandPipeline )
{
    m_CommandPipeline = CommandPipeline;

    m_ThreadWorking  = false;
    m_ThreadExitFlag = false;
    GMI_RESULT Result = m_ServerThread.Create(NULL, 0, UnixTcpSessionManager::ServerThread, this);
    if (FAILED(Result))
    {
        fprintf(stderr, "[SYS_SERVER]"__FILE__ "%d m_ServerThread.Create error %d\n", __LINE__, errno);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ServerThread.Create fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_ServerThread.Start();
    if (FAILED(Result))
    {
        fprintf(stderr, "[SYS_SERVER]"__FILE__ "%d m_ServerThread.Start error %d\n", __LINE__, errno);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ServerThread.Start fail, Result = 0x%lx\n", Result);
        m_ServerThread.Destroy();
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT  UnixTcpSessionManager::Stop()
{
    m_ThreadExitFlag = true;
    while (m_ThreadWorking)
    {
        GMI_Sleep(100);
    }

    m_CommandPipeline = NULL;
    return GMI_SUCCESS;
}


GMI_RESULT UnixTcpSessionManager::Accept(int32_t ListenFd, uid_t *UidPtr, int32_t *ClientFdPtr)
{
    int32_t CliFd, Len;
    struct sockaddr_un SockUn;
    struct stat StatBuf;

    Len = sizeof(SockUn);
    CliFd = accept(ListenFd, (struct sockaddr*)&SockUn, (socklen_t*)&Len);
    if (0 > CliFd)
    {
        return GMI_FAIL;
    }

    Len -= Offsetof(struct sockaddr_un, sun_path);
    SockUn.sun_path[Len] = 0;

    if (0 > stat(SockUn.sun_path, &StatBuf))
    {
        if (0 < CliFd)
        {
            close(CliFd);
            CliFd = -1;
        }
        return GMI_FAIL;
    }

    if (0 == S_ISSOCK(StatBuf.st_mode))
    {
        if (0 < CliFd)
        {
            close(CliFd);
            CliFd = -1;
        }
        return GMI_FAIL;
    }

    if (NULL != UidPtr)
    {
        *UidPtr = StatBuf.st_uid;
    }
    unlink(SockUn.sun_path);

    *ClientFdPtr = CliFd;

    return GMI_SUCCESS;
}


void* UnixTcpSessionManager::ServerThread(void *Argument)
{
    UnixTcpSessionManager *Manager = reinterpret_cast<UnixTcpSessionManager*> (Argument);
    return Manager->Server();
}


void* UnixTcpSessionManager::Server()
{
    m_ThreadWorking = true;

    while (!m_ThreadExitFlag)
    {
        uid_t   UidKey;
        int32_t ClientFd;

        GMI_RESULT Result = Accept(m_ListenFd, &UidKey, &ClientFd);
        if (FAILED(Result))
        {
            continue;
        }

        SYS_INFO("Accept ClientFd %d\n", ClientFd);
        SessionManagerParam *SessionManagerParamPtr(BaseMemoryManager::Instance().New<SessionManagerParam>());
        if (NULL == SessionManagerParamPtr)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SessionManagerParamPtr malloc fail, Result = 0x%lx\n", Result);
            return (void_t*)GMI_OUT_OF_MEMORY;
        }

        SessionManagerParamPtr->s_Fd = ClientFd;
        SessionManagerParamPtr->s_UnTcpSessionManagerPtr = this;

        GMI_Thread Thread;
        Result = Thread.Create(NULL, 0, UnixTcpSessionManager::ProcessThread, (void_t*)SessionManagerParamPtr);
        if (FAILED(Result))
        {
            BaseMemoryManager::Instance().Delete<SessionManagerParam>(SessionManagerParamPtr);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Thread.Create fail, Result = 0x%lx\n", Result);
            continue;
        }

        Result = Thread.Start();
        if (FAILED(Result))
        {
            Thread.Destroy();
            BaseMemoryManager::Instance().Delete<SessionManagerParam>(SessionManagerParamPtr);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Thread.Start fail, Result = 0x%lx\n", Result);
            continue;
        }

        Thread.Destroy();
    }

    m_ThreadWorking = false;
    return (void *) GMI_SUCCESS;
}


void* UnixTcpSessionManager::ProcessThread(void *Argument )
{
    pthread_detach(pthread_self());

    SessionManagerParam *SessionParam = reinterpret_cast<SessionManagerParam*> (Argument);

    UnixTcpSessionManager *Manager = SessionParam->s_UnTcpSessionManagerPtr;
    int32_t ClientFd = SessionParam->s_Fd;
    BaseMemoryManager::Instance().Delete<SessionManagerParam>(SessionParam);

    return Manager->ProcessEntry(ClientFd);
}


void* UnixTcpSessionManager::ProcessEntry(int32_t ClientFd)
{
    int32_t        BufferLen;
    GMI_RESULT     Result;
    boolean_t      UnTcpSessionInitDone = false;
    SdkPkgTransferProtocolKey SdkTransferProtocolKey;

    ReferrencePtr<UnixTcpSession, DefaultObjectDeleter, MultipleThreadModel> UnTcpSession(BaseMemoryManager::Instance().New<UnixTcpSession>());

    while (1)
    {
        struct timeval Timeout;
        fd_set FdSet;
        PkgRudpHeader RudpHeader;

        //uint32_t Time1 = TimeStamp();
        Timeout.tv_sec  = 1;
        Timeout.tv_usec = 0;
        FD_ZERO(&FdSet);
        FD_SET(ClientFd, &FdSet);
        int32_t Ret = select(ClientFd+1, &FdSet, NULL, NULL, &Timeout);
        if (Ret < 0)
        {
            SYS_ERROR("select ClientFd %d error. errno = %d\n", ClientFd, errno);
            break;
        }
        else if (Ret == 0)
        {
            SYS_ERROR("select ClientFd %d timeout. errno = %d\n", ClientFd, errno);
            break;
        }
        else
        {
            Ret = recv(ClientFd, &RudpHeader, sizeof(PkgRudpHeader), 0);
            if (Ret != sizeof(PkgRudpHeader))
            {
                SYS_INFO("recv rudp header error, Ret = %d, header len = %d\n", Ret, sizeof(PkgRudpHeader));
                break;
            }
            else
            {
                if (0 == memcmp(RudpHeader.s_MsgTag, GMI_RUDP_MESSAGE_TAG, GMI_RUDP_MESSAGE_TAG_LENGTH))
                {
                    memcpy(SdkTransferProtocolKey.s_TransferKey, GMI_TRANSFER_KEY_TAG, GMI_TRANSFER_KEY_TAG_LENGTH);
                    SdkTransferProtocolKey.s_AuthValue = RudpHeader.s_AuthValue;
                    SdkTransferProtocolKey.s_SeqNumber = RudpHeader.s_SeqNum;
                    SdkTransferProtocolKey.s_SessionId = RudpHeader.s_SessionId;

                    BufferLen = RudpHeader.s_BodyLen + sizeof(SdkPkgTransferProtocolKey);
                    ReferrencePtr<uint8_t, DefaultObjectsDeleter>ReceiveBuffer(BaseMemoryManager::Instance().News<uint8_t>(BufferLen));
                    if (NULL == ReceiveBuffer.GetPtr())
                    {
                        SYS_ERROR("ReceiveBufferPtr News fail\n");
                        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ReceiveBufferPtr News fail\n");
                        break;
                    }

                    Ret = recv(ClientFd, ReceiveBuffer.GetPtr(), RudpHeader.s_BodyLen, 0);
                    if (Ret != RudpHeader.s_BodyLen)
                    {
                        SYS_INFO("recv rudp body error, Ret = %d, body len = %d\n", Ret, RudpHeader.s_BodyLen);
                        break;
                    }
                    else
                    {
                        //SYS_INFO("unix tcp receive Packet length %d, waste Time %d\n", Ret, TimeStamp() - Time1);

                        if (!UnTcpSessionInitDone) //UnTcpSession init once;
                        {
                            Result = UnTcpSession->Initialize();
                            if (FAILED(Result))
                            {
                                break;
                            }

                            Result = UnTcpSession->Open(ClientFd, BufferLen*2);
                            if (FAILED(Result))
                            {
                                break;
                            }

                            UnTcpSessionInitDone = true;
                        }

                        memcpy(ReceiveBuffer.GetPtr()+Ret, &SdkTransferProtocolKey, sizeof(SdkPkgTransferProtocolKey));
                        Result = UnTcpSession->Receive(ReceiveBuffer.GetPtr(), BufferLen);
                        if (FAILED(Result))
                        {
                            continue;
                        }

                        m_CommandPipeline->Parse(UnTcpSession);
                        // short session,jump out then close socket.
                        break;
                    }
                }
            }
        }
    }

    //release resource
    SafePtr<SysCloseUnixTcpCommandExecutor> CloseUnixTcpCommandExecutor(BaseMemoryManager::Instance().New<SysCloseUnixTcpCommandExecutor>());
    if (NULL == CloseUnixTcpCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "CloseUnixTcpCommandExecutor new fail\n");
        return (void*)GMI_OUT_OF_MEMORY;
    }
    Result = CloseUnixTcpCommandExecutor->SetParameter(UnTcpSession, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return (void*)Result;
    }
    Result = m_CommandPipeline->AddCommandExecutor(CloseUnixTcpCommandExecutor);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "AddCommandExecutor fail, Result = 0x%lx\n", Result);
        return (void*)Result;
    }

    UnTcpSession = NULL;

    return (void *) GMI_SUCCESS;
}


