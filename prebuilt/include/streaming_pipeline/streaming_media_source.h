#pragma once

#include "streaming_media_filter.h"

class StreamingMediaSource : public StreamingMediaFilter
{
protected:
    StreamingMediaSource();
    virtual ~StreamingMediaSource(void);

public:
    virtual GMI_RESULT  ConnectTo  ( int32_t InputPinIndex, StreamingMediaFilter *ToFilter, int32_t ToFilterOutputPinIndex )
    {
        return GMI_NOT_SUPPORT;
    }

    virtual GMI_RESULT  DisconnectInput ( int32_t InputPinIndex,  StreamingMediaFilter *Filter, int32_t OutputPinIndex )
    {
        return GMI_NOT_SUPPORT;
    }

    virtual GMI_RESULT  Receive( int32_t InputPinIndex, const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength )
    {
        return GMI_NOT_SUPPORT;
    }

    virtual boolean_t   IsSource()
    {
        return true;
    }
};
