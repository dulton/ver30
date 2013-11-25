#pragma once

#include "streaming_media_filter.h"

class StreamingMediaTransformer : public StreamingMediaFilter
{
protected:
    StreamingMediaTransformer();
    virtual ~StreamingMediaTransformer(void);

public:
    virtual boolean_t   IsTransformer()
    {
        return true;
    }
};
