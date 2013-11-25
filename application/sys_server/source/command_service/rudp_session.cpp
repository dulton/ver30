
#include "rudp_session.h"
#include "gmi_rudp_api.h"
#include "log.h"
#include "sys_env_types.h"

RUDPSession::RUDPSession(void)
    : m_RudpSendSocket(0)
    , m_RemoteSPort(0)
    , m_Session_Mutex()
{
}


RUDPSession::~RUDPSession(void)
{
}


GMI_RESULT RUDPSession::Initialize()
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


GMI_RESULT RUDPSession::Deinitialize()
{
    m_Session_Mutex.Destroy();
    return GMI_SUCCESS;
}


/*===============================================================
func name:Close
func:close rudp handle and uninitial ringbuffer
param:
return:success--return GMI_SUCCESSr,
    failed -- return GMI_FAIL
---------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/2/17      1.0                 guqiang.lu           create
**********************************************************************************/
GMI_RESULT RUDPSession::Close()
{
    GMI_RESULT Result = m_ReceiveBuffer.Deinitialize();
    if (FAILED(Result))
    {
        SYS_ERROR("m_ReceiveBuffer.Deinitialize() error, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ReceiveBuffer.Deinitialize() error, Result = 0x%lx\n", Result);
        return Result;
    }

    return Result;
}


/*===============================================================
func name:Receive
func:receive from rudp
param:
return:success--return GMI_SUCCESSr,
    failed -- return GMI_FAIL
---------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/2/17      1.0                 guqiang.lu           create
**********************************************************************************/
GMI_RESULT RUDPSession::Receive()
{
    return GMI_NOT_IMPLEMENT;
}

GMI_RESULT RUDPSession::Receive( const uint8_t *Buffer, size_t BufferSize )
{
    if (0 == m_ReceiveBuffer.Write(Buffer, BufferSize))
    {
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}

/*===============================================================
func name:GetFDHandle
func:get session handle
param:
return:
---------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/2/17      1.0                 guqiang.lu           create
**********************************************************************************/
FD_HANDLE  RUDPSession::GetFDHandle()
{
    return m_RudpSendSocket;
}


size_t RUDPSession::ReadableDataSize()
{
    return m_ReceiveBuffer.Size();
}


/*===============================================================
func name:Read
func:get read data from ringbuffer
param:
return:
---------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/2/17      1.0                 guqiang.lu           create
**********************************************************************************/
GMI_RESULT RUDPSession::Read( size_t Offset, uint8_t *Buffer, size_t BufferSize, size_t *Transferred )
{
    *Transferred = m_ReceiveBuffer.Get( Offset, Buffer, BufferSize );
    if (0 == *Transferred)
    {
        //fprintf(stderr, "[SYS_SERVER]"__FILE__ "%d m_ReceiveBuffer.Get() error %d\n", __LINE__, errno);
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}


/*===============================================================
func name:ReadAdvance
func:get read pointer move back
param:
return:
---------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/2/17      1.0                 guqiang.lu           create
**********************************************************************************/
GMI_RESULT RUDPSession::ReadAdvance( size_t Size )
{
    if (0 == m_ReceiveBuffer.ReadAdvance(Size))
    {
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}


/*===============================================================
func name:Write
func:write data to rudp
param:
return:
---------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/2/17      1.0                 guqiang.lu           create
**********************************************************************************/
GMI_RESULT RUDPSession::Write( const uint8_t *Buffer, size_t BufferSize, size_t *Transferred )
{
    size_t         BodyLen;
    size_t         TailLen;
    PkgRudpSendInput RudpSendInput = {0};
    PkgRudpSendOutput RudpSendOutput = {0};
    SdkPkgTransferProtocolKey *SdkTransferProtocolKeyPtr;

    TailLen = sizeof(SdkPkgTransferProtocolKey);
    if (BufferSize <= TailLen)
    {
        SYS_ERROR("invalid param, BufferSize %d\n", BufferSize);
        return GMI_INVALID_PARAMETER;
    }

    BodyLen = BufferSize - TailLen;
    SdkTransferProtocolKeyPtr = (SdkPkgTransferProtocolKey*)(Buffer + BodyLen);

    memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
    RudpSendInput.s_RemotePort = m_RemoteSPort;
    RudpSendInput.s_Buffer = (uint8_t*)Buffer;
    RudpSendInput.s_SendLength = BodyLen;
    RudpSendInput.s_TimeoutMS = 5000;
    RudpSendInput.s_AuthValue = SdkTransferProtocolKeyPtr->s_AuthValue;
    RudpSendInput.s_SequenceNum = SdkTransferProtocolKeyPtr->s_SeqNumber;
    RudpSendInput.s_SessionId = SdkTransferProtocolKeyPtr->s_SessionId;

    //printf("m_Remote_RUDP_Port %d, BufferSize %d\n", m_RemoteSPort, BufferSize);
    GMI_RESULT Result = GMI_RudpSend(m_RudpSendSocket, &RudpSendInput, &RudpSendOutput);
    if (FAILED(Result))
    {
        SYS_ERROR("GMI_RudpSend error, Result = 0x%lx\n", Result);
        return Result;
    }

    *Transferred = RudpSendOutput.s_RealSendBytes + TailLen;
    return GMI_SUCCESS;
}


/*===============================================================
func name:Open
func:rudp create and ringbuffer initial
param:
return:
---------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/2/17      1.0                 guqiang.lu           create
**********************************************************************************/
GMI_RESULT RUDPSession::Open( FD_HANDLE RudpSendSocket, uint16_t RemoteSPort, size_t SessionBufferSize )
{
    GMI_RESULT Result = m_ReceiveBuffer.Initialize(SessionBufferSize);
    if (FAILED(Result))
    {
        SYS_ERROR("m_ReceiveBuffer Initialize error, Result = 0x%lx\n", Result);
        return Result;
    }

    m_RudpSendSocket = RudpSendSocket;
    m_RemoteSPort = RemoteSPort;
    return GMI_SUCCESS;
}


GMI_RESULT RUDPSession::Lock()
{
    m_Session_Mutex.Lock(TIMEOUT_INFINITE);
    return GMI_SUCCESS;
}


GMI_RESULT RUDPSession::Unlock()
{
    m_Session_Mutex.Unlock();
    return GMI_SUCCESS;
}


