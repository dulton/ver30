#pragma once

#include "gmi_media_ctrl.h"
#include "streaming_media_source.h"

class ImageSourceFitler : public StreamingMediaSource
{
protected:
    ImageSourceFitler(void);
    virtual ~ImageSourceFitler(void);

    virtual GMI_RESULT	Initialize( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize );
    virtual GMI_RESULT  Deinitialize();
    virtual GMI_RESULT  Receive( int32_t InputPinIndex, const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength );

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
    GMI_RESULT GetHardwareDogEnableConfig( boolean_t *Enable );
    GMI_RESULT GetHardwareDogGuardTime( uint32_t *GuardTime );
    GMI_RESULT GetFeedHardwareDogInterval( uint32_t *Interval );

private:
    boolean_t      m_FeedHardwareDog;
    struct timeval m_LastFeedDogTime;
    uint32_t       m_FeedDogInterval;

    uint32_t       m_SourceId;
    uint32_t       m_MediaId;
    uint32_t       m_MediaType;
    uint32_t       m_CodecType;
};
