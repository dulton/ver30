#include "unix_tcp_session.h"
#include "gmi_rudp.h"
#include "log.h"
#include "sys_env_types.h"

UnixTcpSession::UnixTcpSession(void)
    : m_Socket(0)
    , m_Session_Mutex()
{
}


UnixTcpSession::~UnixTcpSession(void)
{
}


GMI_RESULT UnixTcpSession::Initialize()
{
    GMI_RESULT Result = m_Session_Mutex.Create(NULL);
    if (FAILED(Result))
    {
        SYS_ERROR("m_Session_Mutex.Create error\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_Session_Mutex.Create error\n");
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT UnixTcpSession::Deinitialize()
{
    m_Session_Mutex.Destroy();
    return GMI_SUCCESS;
}


GMI_RESULT UnixTcpSession::Close()
{
    GMI_RESULT Result = m_ReceiveBuffer.Deinitialize();
    if (FAILED(Result))
    {
        SYS_ERROR("m_ReceiveBuffer.Deinitialize() error\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ReceiveBuffer.Deinitialize() error\n");
        return Result;
    }

    return Result;
}


GMI_RESULT UnixTcpSession::Receive()
{
    return GMI_NOT_IMPLEMENT;
}

GMI_RESULT UnixTcpSession::Receive( const uint8_t *Buffer, size_t BufferSize )
{
    if (0 == m_ReceiveBuffer.Write(Buffer, BufferSize))
    {
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}


int32_t  UnixTcpSession::GetFDHandle()
{
    return m_Socket;
}


size_t UnixTcpSession::ReadableDataSize()
{
    return m_ReceiveBuffer.Size();
}


GMI_RESULT UnixTcpSession::Read( size_t Offset, uint8_t *Buffer, size_t BufferSize, size_t *Transferred )
{
    *Transferred = m_ReceiveBuffer.Get( Offset, Buffer, BufferSize );
    if (0 == *Transferred)
    {
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}


GMI_RESULT UnixTcpSession::ReadAdvance( size_t Size )
{
    if (0 == m_ReceiveBuffer.ReadAdvance(Size))
    {
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}


GMI_RESULT UnixTcpSession::Write(const uint8_t *Buffer, size_t BufferSize, size_t *Transferred )
{
    size_t         BodyLen;
    size_t         TailLen;
    SdkPkgTransferProtocolKey *SdkTransferProtocolKeyPtr;

    TailLen = sizeof(SdkPkgTransferProtocolKey);
    if (BufferSize <= TailLen)
    {
        SYS_ERROR("invalid param");
        return GMI_INVALID_PARAMETER;
    }

    BodyLen = BufferSize - TailLen;
    SdkTransferProtocolKeyPtr = (SdkPkgTransferProtocolKey*)(Buffer + BodyLen);

    PkgRudpHeader RudpHeader;
    memset(&RudpHeader, 0, sizeof(PkgRudpHeader));
    memcpy(RudpHeader.s_MsgTag, GMI_RUDP_MESSAGE_TAG, GMI_RUDP_MESSAGE_TAG_LENGTH);
    RudpHeader.s_AuthValue = SdkTransferProtocolKeyPtr->s_AuthValue;
    RudpHeader.s_SeqNum    = SdkTransferProtocolKeyPtr->s_SeqNumber;
    RudpHeader.s_SessionId = SdkTransferProtocolKeyPtr->s_SessionId;
    RudpHeader.s_Version   = RUDP_VERSION;
    RudpHeader.s_BodyLen   = BodyLen;
    size_t Result = send(m_Socket, &RudpHeader, sizeof(PkgRudpHeader), 0);
    if (Result != sizeof(PkgRudpHeader))
    {
        return GMI_FAIL;
    }

    Result = send(m_Socket, Buffer, BodyLen, 0);
    if (Result != BodyLen)
    {
        return GMI_FAIL;
    }

    *Transferred = Result + TailLen;
    return GMI_SUCCESS;
}


GMI_RESULT UnixTcpSession::Open(int32_t Socket, size_t SessionBufferSize )
{
    GMI_RESULT Result = m_ReceiveBuffer.Initialize(SessionBufferSize);
    if (FAILED(Result))
    {
        SYS_ERROR("m_ReceiveBuffer Initialize error\n");
        return Result;
    }

    m_Socket = Socket;
    return GMI_SUCCESS;
}


GMI_RESULT UnixTcpSession::Lock()
{
    m_Session_Mutex.Lock(TIMEOUT_INFINITE);
    return GMI_SUCCESS;
}


GMI_RESULT UnixTcpSession::Unlock()
{
    m_Session_Mutex.Unlock();
    return GMI_SUCCESS;
}




