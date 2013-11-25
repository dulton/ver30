#include "mjpeg_image_source_filter.h"

MJPEG_ImageSourceFilter::MJPEG_ImageSourceFilter(void)
{
}

MJPEG_ImageSourceFilter::~MJPEG_ImageSourceFilter(void)
{
}

MJPEG_ImageSourceFilter*  MJPEG_ImageSourceFilter::CreateNew()
{
    return BaseMemoryManager::Instance().New<MJPEG_ImageSourceFilter>();
}
