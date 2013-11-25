#pragma once

#include "gmi_media_ctrl.h"
#include "memory_pool_parallel_consumers.h"
#include "streaming_media_source.h"

class AudioSourceFilter  : public StreamingMediaSource
{
protected:
    AudioSourceFilter(void);
    virtual ~AudioSourceFilter(void);

    virtual GMI_RESULT	Initialize( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize );
    virtual GMI_RESULT  Deinitialize();

private:
    void_t                       *m_Buffer;
    size_t                       m_BufferSize;
    MemoryPoolParallelConsumers  *m_DataPool;
};
