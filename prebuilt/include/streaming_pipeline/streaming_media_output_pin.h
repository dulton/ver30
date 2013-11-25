#pragma once

#include "data_source.h"

class StreamingMediaFilter;
class StreamingMediaInputPin;

class StreamingMediaOutputPin : public DataSource
{
protected:
    StreamingMediaOutputPin( boolean_t ActiveObject, StreamingMediaFilter *Filter, int32_t PinIndex );
    virtual ~StreamingMediaOutputPin(void);

public:
    static  StreamingMediaOutputPin* CreateNew( boolean_t ActiveObject, StreamingMediaFilter *Filter, int32_t PinIndex );
    friend class BaseMemoryManager;

    virtual GMI_RESULT      Deinitialize();

    GMI_RESULT              GetOwnerFilter( StreamingMediaFilter **Filter );
    GMI_RESULT              ConnectFrom( StreamingMediaInputPin *Pin );
    GMI_RESULT              Disconnect( StreamingMediaInputPin *Pin );
    inline size_t           GetInputPinNumber() const
    {
        return m_InputPins.size();
    }
    GMI_RESULT              GetConnectedFilter( int32_t InputPinIndex, StreamingMediaFilter **Filter );

protected:
    virtual GMI_RESULT      Process( const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength );

protected:
    StreamingMediaFilter                 *m_Filter;
    int32_t	                             m_PinIndex;
    std::vector<StreamingMediaInputPin*> m_InputPins;
};
