#include "mpeg4_video_source_filter.h"

Mpeg4VideoSourceFilter::Mpeg4VideoSourceFilter(void)
{
}

Mpeg4VideoSourceFilter::~Mpeg4VideoSourceFilter(void)
{
}

Mpeg4VideoSourceFilter*  Mpeg4VideoSourceFilter::CreateNew()
{
    return BaseMemoryManager::Instance().New<Mpeg4VideoSourceFilter>();
}
