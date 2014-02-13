#include <ipc_fw_v3.x_resource.h>
#include <ipc_fw_v3.x_setting.h>
#include <sys_client.h>
#include <sys_env_types.h>
#include <gmi_media_ctrl.h>
#include <ipc_media_data_dispatch.h>

#include "configure.h"

typedef struct tagStreamPortInfo
{
    uint16_t s_RemotePort;
    uint16_t s_LocalPort;
} StreamPortInfo;

static const StreamPortInfo l_VideoStreamPortInfo[] = {
    {GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1, GMI_STREAMING_MEDIA_ONVIF_ENCODE_VIDEO1},
    {GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO2, GMI_STREAMING_MEDIA_ONVIF_ENCODE_VIDEO2},
    {GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO3, GMI_STREAMING_MEDIA_ONVIF_ENCODE_VIDEO3},
    {GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO4, GMI_STREAMING_MEDIA_ONVIF_ENCODE_VIDEO4},
};

static const StreamPortInfo l_AudioStreamPortInfo[] = {
    {GMI_STREAMING_MEDIA_SERVER_ENCODE_AUDIO1, GMI_STREAMING_MEDIA_ONVIF_ENCODE_AUDIO1},
};

Configure::Configure()
    : m_Initialized(false)
    , m_AuthValue(0)
    , m_SessionId(0)
    , m_UserFlag(0)
{
}

Configure::~Configure()
{
}

GMI_RESULT Configure::Initialize()
{
    if (Initialized())
    {
        PRINT_LOG(WARNING, "Configure module is already initialized");
        return GMI_ALREADY_OPERATED;
    }

    GMI_RESULT RetVal = SysInitialize(GetAuthLocalPort());
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to call SysInitialize()");
        return RetVal;
    }

    char_t SupperUsername[] = GMI_SUPER_USER_NAME;
    char_t SupperPassword[] = GMI_SUPER_USER_PASSWD;

    RetVal = SysAuthLogin(SupperUsername, SupperPassword, 0, GetAuthLocalModuleId(), &m_SessionId, &m_UserFlag, &m_AuthValue);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(WARNING, "Failed to login supper user");
        SysDeinitialize();
        return RetVal;
    }

    PRINT_LOG(VERBOSE, "Succeeded to login supper user");
    DUMP_VARIABLE(m_SessionId);
    DUMP_VARIABLE(m_UserFlag);
    DUMP_VARIABLE(m_AuthValue);

    m_Initialized = true;
    return GMI_SUCCESS;
}

GMI_RESULT Configure::Uninitialize()
{
    if (!Initialized())
    {
        PRINT_LOG(WARNING, "Configure module is not initialized yet");
        return GMI_ALREADY_OPERATED;
    }

    GMI_RESULT RetVal = SysAuthLogout(m_SessionId);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(WARNING, "Failed to logout supper user");
    }
    else
    {
        PRINT_LOG(VERBOSE, "Succeeded to logout supper user");
    }

    m_SessionId = 0;
    m_UserFlag = 0;
    m_AuthValue = 0;

    RetVal = SysDeinitialize();
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to call SysDeinitialize()");
        return RetVal;
    }

    m_Initialized = false;
    return GMI_SUCCESS;
}

uint16_t Configure::GetVideoStreamLocalPort(uint32_t StreamId) const
{
    ASSERT(StreamId < COUNT_OF(l_VideoStreamPortInfo), "StreamId(%u) is out of range", StreamId);
    return l_VideoStreamPortInfo[StreamId].s_LocalPort;
}

uint16_t Configure::GetVideoStreamRemotePort(uint32_t StreamId) const
{
    ASSERT(StreamId < COUNT_OF(l_VideoStreamPortInfo), "StreamId(%u) is out of range", StreamId);
    return l_VideoStreamPortInfo[StreamId].s_RemotePort;
}

uint16_t Configure::GetAudioStreamLocalPort(uint32_t StreamId) const
{
    ASSERT(StreamId < COUNT_OF(l_AudioStreamPortInfo), "StreamId(%u) is out of range", StreamId);
    return l_AudioStreamPortInfo[StreamId].s_LocalPort;
}

uint16_t Configure::GetAudioStreamRemotePort(uint32_t StreamId) const
{
    ASSERT(StreamId < COUNT_OF(l_AudioStreamPortInfo), "StreamId(%u) is out of range", StreamId);
    return l_AudioStreamPortInfo[StreamId].s_RemotePort;
}

uint32_t Configure::GetVideoStreamCount() const
{
    return COUNT_OF(l_VideoStreamPortInfo);
}

uint32_t Configure::GetAudioStreamCount() const
{
    return COUNT_OF(l_AudioStreamPortInfo);
}

uint32_t Configure::GetStreamApplicationId() const
{
    return ONVIF_STREAM_APPLICATION_ID;
}

uint16_t Configure::GetRtspServicePort() const
{
    if (!Initialized())
    {
        PRINT_LOG(WARNING, "Configure module is not initialized yet");
        return GMI_RTSP_SERVER_TCP_PORT;
    }

    SysPkgNetworkPort SysNetworkPorts;
    memset(&SysNetworkPorts, 0x00, sizeof(SysNetworkPorts));

    GMI_RESULT RetVal = SysGetNetworkPort(m_SessionId, m_AuthValue, &SysNetworkPorts);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to get ports for all the services");
        return GMI_RTSP_SERVER_TCP_PORT;
    }

    PRINT_LOG(VERBOSE, "RTSP server is using %d port", SysNetworkPorts.s_RTSP_Port);

    return SysNetworkPorts.s_RTSP_Port;
}

uint16_t Configure::GetAuthLocalPort() const
{
    return GMI_ONVIF_RTSP_SERVER_AUTH_PORT;
}

uint8_t Configure::GetAuthLocalModuleId() const
{
    return ID_MOUDLE_REST_RTSP;
}

