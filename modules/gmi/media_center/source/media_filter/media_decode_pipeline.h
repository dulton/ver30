#pragma once

#include "gmi_media_ctrl.h"
#include "gmi_system_headers.h"

class BaseStreamingPipelineManager;
class IpcMediaDataDispatchSource;
class StreamingMediaHandler;

class MediaDecodePipeline
{
public:
    MediaDecodePipeline(void);
    ~MediaDecodePipeline(void);

    GMI_RESULT Initialize( uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, void_t *DecodeParameter, size_t DecodeParameterLength );
    GMI_RESULT Deinitialize();
    GMI_RESULT Play();
    GMI_RESULT Stop();
    GMI_RESULT GetDecodeConfig( void_t *DecodeParameter, size_t *DecodeParameterLength );
    GMI_RESULT SetDecodeConfig( const void_t *DecodeParameter, size_t DecodeParameterLength );

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
    IpcMediaDataDispatchSource    *m_MediaDataDispatchSource;
    StreamingMediaHandler         *m_MediaHandler;
};
