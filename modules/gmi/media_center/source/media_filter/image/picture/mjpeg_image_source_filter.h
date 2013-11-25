#pragma once

#include "image_source_fitler.h"

class MJPEG_ImageSourceFilter : public ImageSourceFitler
{
protected:
    MJPEG_ImageSourceFilter(void);
    virtual ~MJPEG_ImageSourceFilter(void);

public:
    static  MJPEG_ImageSourceFilter*  CreateNew();
    friend class BaseMemoryManager;
};
