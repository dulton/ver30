#pragma once

#include "data_handler.h"

class StreamingMediaFilter;
class StreamingMediaOutputPin;

class StreamingMediaInputPin : public DataHandler
{
protected:
    StreamingMediaInputPin( boolean_t ActiveObject, StreamingMediaFilter *Filter, int32_t PinIndex );
    virtual ~StreamingMediaInputPin(void);

public:
    static  StreamingMediaInputPin*  CreateNew( boolean_t ActiveObject, StreamingMediaFilter *Filter, int32_t PinIndex );
    friend class BaseMemoryManager;

    virtual GMI_RESULT      Deinitialize();

    GMI_RESULT              GetOwnerFilter( StreamingMediaFilter **Filter );
    GMI_RESULT              ConnectTo( StreamingMediaOutputPin *Pin );
    GMI_RESULT              Disconnect( StreamingMediaOutputPin *Pin );
    GMI_RESULT              GetConnectedFilter( StreamingMediaFilter **Filter );

protected:
    virtual GMI_RESULT      Process( const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength );

protected:
    StreamingMediaFilter    *m_Filter;
    int32_t                 m_PinIndex;
    StreamingMediaOutputPin *m_OutputPin;
};
