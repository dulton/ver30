// media_codec_parameter.h
#if !defined( MEDIA_CODEC_PARAMETER )
#define MEDIA_CODEC_PARAMETER

#include "gmi_system_headers.h"
#include "media_center_parameter.h"

#define MEDIA_FILTER_MEMORY_POOL_MINIMUM_MEMORY_BLOCK_NUMBER  16

struct MediaSourceParameter
{
    uint32_t s_SourceId;
    uint32_t s_MediaId;
    uint32_t s_MediaType;
    uint32_t s_CodecType;
    uint32_t s_BitRate;
    uint32_t s_FrameRate;
    void_t   *s_InternalParamter;
};

#endif//MEDIA_CODEC_PARAMETER
