#pragma once

#include "image_source_fitler.h"
#include "memory_pool_parallel_consumers.h"

class VideoSourceFilter : public ImageSourceFitler
{
protected:
    VideoSourceFilter(void);
    virtual ~VideoSourceFilter(void);

    virtual GMI_RESULT	Initialize( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize );
    virtual GMI_RESULT  Deinitialize();

private:
    void_t                       *m_Buffer;
    size_t                       m_BufferSize;
    MemoryPoolParallelConsumers  *m_DataPool;
};
