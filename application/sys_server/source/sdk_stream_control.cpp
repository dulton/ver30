#include "gmi_rudp_api.h"
#include "log.h"
#include "sdk_stream_control.h"
#include "stream_control.h"


SdkStreamControl::SdkStreamControl(uint16_t LocalPort, uint16_t SdkPort)
    :m_RUDP_Socket()
    ,m_SDK_Port(SdkPort)
    ,m_Local_Port(LocalPort)
{
}

SdkStreamControl::~SdkStreamControl()
{
}


GMI_RESULT SdkStreamControl::Initialize()
{
    GMI_RESULT Result = m_SDK_Mutex.Create(NULL);
    if (FAILED(Result))
    {
        SYS_ERROR("m_SDK_Mutex.Create error\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_SDK_Mutex.Create error\n");
        return Result;
    }

    m_RUDP_Socket = GMI_RudpSocket((long_t)m_Local_Port);
    if (NULL == m_RUDP_Socket)
    {
        m_SDK_Mutex.Destroy();
        SYS_ERROR("GMI_RudpSocket fail on port %d\n", m_Local_Port);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GMI_RudpSocket fail on port %d\n", m_Local_Port);
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT SdkStreamControl::Deinitialize()
{
    if (m_RUDP_Socket != NULL)
    {
        GMI_RudpSocketClose(m_RUDP_Socket);
        m_RUDP_Socket = NULL;
    }

    m_SDK_Mutex.Destroy();
    return GMI_SUCCESS;
}


GMI_RESULT SdkStreamControl::Start(SysPkgEncodeCfg *SysEncodeCfg, int32_t VideoCount, SysPkgAudioEncodeCfg *SysAudioCfg, int32_t AudioCount, int32_t Timeout)
{
    if (NULL == SysEncodeCfg
            || VideoCount < 0
            || NULL == SysAudioCfg
            || AudioCount < 0)
    {
        return GMI_INVALID_PARAMETER;
    }

    if (NULL == m_RUDP_Socket)
    {
        return GMI_INVALID_OPERATION;
    }

    ReferrencePtr<SysPkgEncodeCfg, DefaultObjectsDeleter>SysVideoEncodeCfgPtr(BaseMemoryManager::Instance().News<SysPkgEncodeCfg>(VideoCount));
    if (NULL == SysVideoEncodeCfgPtr.GetPtr())
    {
        SYS_ERROR("SysVideoEncodeCfgPtr news fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysVideoEncodeCfgPtr news fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    memset(SysVideoEncodeCfgPtr.GetPtr(), 0, sizeof(SysPkgEncodeCfg)*VideoCount);
    memcpy(SysVideoEncodeCfgPtr.GetPtr(), SysEncodeCfg, sizeof(SysPkgEncodeCfg)*VideoCount);

    ReferrencePtr<SysPkgAudioEncodeCfg, DefaultObjectsDeleter>SysAudioEncodeCfgPtr(BaseMemoryManager::Instance().News<SysPkgAudioEncodeCfg>(AudioCount));
    if (NULL == SysVideoEncodeCfgPtr.GetPtr())
    {
        SYS_ERROR("SysVideoEncodeCfgPtr news fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysVideoEncodeCfgPtr news fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    memset(SysAudioEncodeCfgPtr.GetPtr(), 0, sizeof(SysPkgAudioEncodeCfg)*AudioCount);
    memcpy(SysAudioEncodeCfgPtr.GetPtr(), SysAudioCfg, sizeof(SysPkgAudioEncodeCfg)*AudioCount);

    Lock();
    GMI_RESULT Result = StartStreamTransfer(m_RUDP_Socket, m_SDK_Port, SysVideoEncodeCfgPtr.GetPtr(), VideoCount, SysAudioEncodeCfgPtr.GetPtr(), AudioCount, Timeout);
    if (FAILED(Result))
    {
        SysVideoEncodeCfgPtr = NULL;
        SysAudioEncodeCfgPtr = NULL;
        Unlock();
        SYS_ERROR("StartStreamTransfer fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StartStreamTransfer fail, Result = 0x%lx\n", Result);
        return Result;
    }
    SysVideoEncodeCfgPtr = NULL;
    SysAudioEncodeCfgPtr = NULL;
    Unlock();
    return GMI_SUCCESS;
}


GMI_RESULT SdkStreamControl::Stop(int32_t Timeout)
{
    if (NULL == m_RUDP_Socket)
    {
        return GMI_INVALID_OPERATION;
    }

    Lock();
    GMI_RESULT Result = StopStreamTransfer(m_RUDP_Socket, m_SDK_Port, Timeout);
    if (FAILED(Result))
    {
        Unlock();
        SYS_ERROR("StopStreamTransfer fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StopStreamTransfer fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Unlock();
    return GMI_SUCCESS;
}


GMI_RESULT SdkStreamControl::Pause(int32_t Timeout)
{
    if (NULL == m_RUDP_Socket)
    {
        return GMI_INVALID_OPERATION;
    }

    Lock();
    GMI_RESULT Result = PauseStreamTransfer(m_RUDP_Socket, m_SDK_Port, Timeout);
    if (FAILED(Result))
    {
        Unlock();
        SYS_ERROR("PauseStreamTransfer fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PauseStreamTransfer fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Unlock();
    return GMI_SUCCESS;
}


GMI_RESULT SdkStreamControl::Resume(SysPkgEncodeCfg * SysEncodeCfg, int32_t VideoCount, SysPkgAudioEncodeCfg * SysAudioCfg, int32_t AudioCount, int32_t Timeout)
{
    if (NULL == SysEncodeCfg
            || VideoCount < 0
            || NULL == SysAudioCfg
            || AudioCount < 0)
    {
        return GMI_INVALID_PARAMETER;
    }

    if (NULL == m_RUDP_Socket)
    {
        return GMI_INVALID_OPERATION;
    }

    ReferrencePtr<SysPkgEncodeCfg, DefaultObjectsDeleter>SysVideoEncodeCfgPtr(BaseMemoryManager::Instance().News<SysPkgEncodeCfg>(VideoCount));
    if (NULL == SysVideoEncodeCfgPtr.GetPtr())
    {
        SYS_ERROR("SysVideoEncodeCfgPtr news fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysVideoEncodeCfgPtr news fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    memset(SysVideoEncodeCfgPtr.GetPtr(), 0, sizeof(SysPkgEncodeCfg)*VideoCount);
    memcpy(SysVideoEncodeCfgPtr.GetPtr(), SysEncodeCfg, sizeof(SysPkgEncodeCfg)*VideoCount);

    ReferrencePtr<SysPkgAudioEncodeCfg, DefaultObjectsDeleter>SysAudioEncodeCfgPtr(BaseMemoryManager::Instance().News<SysPkgAudioEncodeCfg>(AudioCount));
    if (NULL == SysVideoEncodeCfgPtr.GetPtr())
    {
        SYS_ERROR("SysVideoEncodeCfgPtr news fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysVideoEncodeCfgPtr news fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    memset(SysAudioEncodeCfgPtr.GetPtr(), 0, sizeof(SysPkgAudioEncodeCfg)*AudioCount);
    memcpy(SysAudioEncodeCfgPtr.GetPtr(), SysAudioCfg, sizeof(SysPkgAudioEncodeCfg)*AudioCount);

    Lock();
    GMI_RESULT Result = ResumeStreamTransfer(m_RUDP_Socket, m_SDK_Port, SysVideoEncodeCfgPtr.GetPtr(), VideoCount, SysAudioEncodeCfgPtr.GetPtr(), AudioCount, Timeout);
    if (FAILED(Result))
    {
        SysVideoEncodeCfgPtr = NULL;
        SysAudioEncodeCfgPtr = NULL;
        Unlock();
        SYS_ERROR("StartStreamTransfer fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StartStreamTransfer fail, Result = 0x%lx\n", Result);
        return Result;
    }
    SysVideoEncodeCfgPtr = NULL;
    SysAudioEncodeCfgPtr = NULL;
    Unlock();
    return GMI_SUCCESS;
}


GMI_RESULT SdkStreamControl::Query(int32_t Timeout, int32_t *Started)
{
    if (NULL == Started)
    {
        return GMI_INVALID_PARAMETER;
    }

    if (NULL == m_RUDP_Socket)
    {
        return GMI_INVALID_OPERATION;
    }

    int32_t Start;
    Lock();
    GMI_RESULT Result = QueryStreamTransfer(m_RUDP_Socket, m_SDK_Port, Timeout, &Start);
    if (FAILED(Result))
    {
        Unlock();
        SYS_ERROR("QueryStreamTransfer fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "QueryStreamTransfer fail, Result = 0x%lx\n", Result);
        return Result;
    }
    *Started = Start;
    Unlock();
    return GMI_SUCCESS;
}


GMI_RESULT SdkStreamControl::Lock()
{
    m_SDK_Mutex.Lock(TIMEOUT_INFINITE);
    return GMI_SUCCESS;
}


GMI_RESULT SdkStreamControl::Unlock()
{
    m_SDK_Mutex.Unlock();
    return GMI_SUCCESS;
}

