#include <ipc_fw_v3.x_resource.h>
#include <gmi_media_ctrl.h>

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
{
}

Configure::~Configure()
{
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
    return 0x00000008;
}

uint16_t Configure::GetRtspServicePort() const
{
    return GMI_RTSP_SERVER_TCP_PORT;
}

