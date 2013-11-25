#pragma once

#include "gmi_media_ctrl.h"
#include "gmi_system_headers.h"

class BaseStreamingPipelineManager;
class IpcMediaDataDispatchHandler;
class StreamingMediaSource;

class MediaEncodePipeline
{
public:
    MediaEncodePipeline(void);
    ~MediaEncodePipeline(void);

    GMI_RESULT Initialize( uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, void_t *EncodeParameter, size_t EncodeParameterLength );
    GMI_RESULT Deinitialize();
    GMI_RESULT Play();
    GMI_RESULT Stop();
    GMI_RESULT GetEncodeConfig( void_t *EncodeParameter, size_t *EncodeParameterLength );
    GMI_RESULT SetEncodeConfig( const void_t *EncodeParameter, size_t EncodeParameterLength );
    GMI_RESULT GetOsdConfig( void_t *OsdParameter, size_t *OsdParameterLength );
    GMI_RESULT SetOsdConfig( const void_t *OsdParameter, size_t OsdParameterLength );
    GMI_RESULT ForceGenerateIdrFrame();

    inline uint32_t   GetSourceId()  const
    {
        return m_SourceId;
    }
    inline uint32_t   GetMediaId()   const
    {
        return m_MediaId;
    }
    inline uint32_t   GetMediaType() const
    {
        return m_MediaType;
    }
    inline uint32_t   GetCodecType() const
    {
        return m_CodecType;
    }

private:
    BaseStreamingPipelineManager  *m_StreamingPipelineManager;
    uint32_t                      m_SourceId;
    uint32_t                      m_MediaId;
    uint32_t                      m_MediaType;
    uint32_t                      m_CodecType;
    StreamingMediaSource          *m_MediaSource;
    IpcMediaDataDispatchHandler   *m_MediaDataDispatchHandler;
};
