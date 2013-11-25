#pragma once

#include "gmi_system_headers.h"
#include "streaming_media_input_pin.h"
#include "streaming_media_output_pin.h"

class BaseStreamingPipelineManager;

#define MAX_MEDIA_FILTER_NAME_LENGTH    512

enum MediaFilterMode
{
    MFM_Push = 0,
    MFM_Pull = 1,
};

enum MediaFilterStatus
{
    MFS_Init    = 0,
    MFS_Uninit  = 1,
    MFS_Playing = 2,
    MFS_Paused  = 3,
    MFS_Stopped = 4
};

class StreamingMediaFilter
{
protected:
    StreamingMediaFilter();
    virtual ~StreamingMediaFilter(void);
    friend class BaseMemoryManager;

public:
    virtual GMI_RESULT	Initialize( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize );
    virtual GMI_RESULT  Deinitialize();
    virtual GMI_RESULT  Play();
    virtual GMI_RESULT  Pause();
    virtual GMI_RESULT  Stop();

    virtual GMI_RESULT  ConnectTo  ( int32_t InputPinIndex, StreamingMediaFilter *ToFilter, int32_t ToFilterOutputPinIndex );
    virtual GMI_RESULT  ConnectFrom( int32_t OutputPinIndex, StreamingMediaFilter *FromFilter, int32_t FromFilterInputPinIndex );
    virtual GMI_RESULT  DisconnectInput ( int32_t InputPinIndex,  StreamingMediaFilter *Filter, int32_t OutputPinIndex );
    virtual GMI_RESULT  DisconnectOutput( int32_t OutputPinIndex, StreamingMediaFilter *Filter, int32_t InputPinIndex );
    virtual GMI_RESULT  Receive( int32_t InputPinIndex, const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength );

    virtual boolean_t   IsSource()
    {
        return false;
    }
    virtual boolean_t   IsTransformer()
    {
        return false;
    }
    virtual boolean_t   IsHandler()
    {
        return false;
    }

    void_t              SetStreamingPipelineManager( BaseStreamingPipelineManager *StreamingPipelineManager );

    inline  size_t      GetInputPinNumber() const
    {
        return m_Inputs.size();
    }
    GMI_RESULT	        GetInputPinConnectedFilter( int32_t InputPinIndex, StreamingMediaFilter **Filter );
    GMI_RESULT          GetInputPin( int32_t InputPinIndex, StreamingMediaInputPin **InputPin );

    inline  size_t      GetOutputPinNumber() const
    {
        return m_Outputs.size();
    }
    size_t              GetOutputPinInputPinNumber( int32_t OutputPinIndex ) const;
    GMI_RESULT	        GetOutputPinConnectedFilter( int32_t OutputPinIndex, int32_t InputPinIndex, StreamingMediaFilter **Filter );
    GMI_RESULT          GetOutputPin( int32_t OutputPinIndex, StreamingMediaOutputPin **OutputPin );

    inline  enum MediaFilterMode    Mode()
    {
        return m_Mode;
    }
    inline  enum MediaFilterStatus  Status()
    {
        return m_Status;
    }
    inline  int32_t				    GetId()
    {
        return m_FilterId;
    }
    char_t*                         GetName( char_t *Name, size_t *Length );
    int32_t                         AddRef();
    int32_t                         ReleaseRef();

protected:
    BaseStreamingPipelineManager  *m_StreamingPipelineManagerr;
    enum MediaFilterMode          m_Mode;
    enum MediaFilterStatus	      m_Status;
    int32_t                       m_FilterId;
    char_t                        *m_FilterName;
    size_t                        m_FilterNameLength;
    int32_t                       m_RefCount;

    std::vector<StreamingMediaInputPin*>   m_Inputs;
    std::vector<StreamingMediaOutputPin*>  m_Outputs;
};
