#pragma once

#include "gmi_media_ctrl.h"
#include "streaming_media_handler.h"

class AudioHandlerFilter  : public StreamingMediaHandler
{
protected:
    AudioHandlerFilter(void);
    virtual ~AudioHandlerFilter(void);

    virtual GMI_RESULT	Initialize( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize );
    virtual GMI_RESULT  Deinitialize();
};
