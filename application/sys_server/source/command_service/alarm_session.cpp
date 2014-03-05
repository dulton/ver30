#include "alarm_session.h"
#include "auth_center_api.h"
#include "log.h"
#include "gmi_rudp_api.h"

AlarmSession::AlarmSession(uint16_t LocalPort, uint16_t SdkPort)
:m_RUDP_Socket()
,m_LocalPort(LocalPort)
,m_RemotePort(SdkPort)
{
}

AlarmSession::~AlarmSession()
{
}


GMI_RESULT AlarmSession::Initialize()
{
	GMI_RESULT Result = m_SDK_Mutex.Create(NULL);
    if (FAILED(Result))
    {
        SYS_ERROR("m_SDK_Mutex.Create error\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_SDK_Mutex.Create error\n");
        return Result;
    }

    m_RUDP_Socket = GMI_RudpSocket((long_t)m_LocalPort);
    if (NULL == m_RUDP_Socket)
    {
        m_SDK_Mutex.Destroy();
        SYS_ERROR("GMI_RudpSocket fail on port %d\n", m_LocalPort);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GMI_RudpSocket fail on port %d\n", m_LocalPort);
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT AlarmSession::Deinitialize()
{
	if (NULL != m_RUDP_Socket)
    {
        GMI_RudpSocketClose(m_RUDP_Socket);
        m_RUDP_Socket = NULL;
    }

    m_SDK_Mutex.Destroy();
    return GMI_SUCCESS;
}


GMI_RESULT AlarmSession::Send(const uint8_t *Buffer, size_t BufferSize, size_t *Transferred)
{
    PkgRudpSendInput  RudpSendInput = {0};
    PkgRudpSendOutput RudpSendOutput = {0};           

    memset(&RudpSendInput, 0, sizeof(PkgRudpSendInput));
    RudpSendInput.s_RemotePort = m_RemotePort;
    RudpSendInput.s_Buffer = (uint8_t*)Buffer;
    RudpSendInput.s_SendLength = BufferSize;
    RudpSendInput.s_TimeoutMS = 5000;   
    RudpSendInput.s_SequenceNum = 1;
    RudpSendInput.s_SessionId = ID_SESSIONID_INTER_SDK;
   
    GMI_RESULT Result = GMI_RudpSend(m_RUDP_Socket, &RudpSendInput, &RudpSendOutput);
    if (FAILED(Result))
    {
        SYS_ERROR("GMI_RudpSend error, Result = 0x%lx\n", Result);
        return Result;
    }

    *Transferred = RudpSendOutput.s_RealSendBytes;
    return GMI_SUCCESS;	
}




