#pragma once

#include "video_source_filter.h"

class Mpeg4VideoSourceFilter : public VideoSourceFilter
{
protected:
    Mpeg4VideoSourceFilter(void);
    virtual ~Mpeg4VideoSourceFilter(void);

public:
    static  Mpeg4VideoSourceFilter*  CreateNew();
    friend class BaseMemoryManager;
};
