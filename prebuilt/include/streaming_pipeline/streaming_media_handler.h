#pragma once

#include "streaming_media_filter.h"

class StreamingMediaHandler : public StreamingMediaFilter
{
protected:
    StreamingMediaHandler();
    virtual ~StreamingMediaHandler(void);

public:
    virtual GMI_RESULT  ConnectFrom( int32_t OutputPinIndex, StreamingMediaFilter *FromFilter, int32_t FromFilterInputPinIndex )
    {
        return GMI_NOT_SUPPORT;
    }

    virtual GMI_RESULT  DisconnectOutput( int32_t OutputPinIndex, StreamingMediaFilter *Filter, int32_t InputPinIndex )
    {
        return GMI_NOT_SUPPORT;
    }

    virtual boolean_t   IsHandler()
    {
        return true;
    }
};
