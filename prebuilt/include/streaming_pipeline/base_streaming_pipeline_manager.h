#pragma once

#include "gmi_system_headers.h"
#include "streaming_media_filter.h"

class BaseStreamingPipelineManager
{
protected:
    BaseStreamingPipelineManager( enum MediaFilterMode Mode );
    virtual ~BaseStreamingPipelineManager(void);

public:
    static  BaseStreamingPipelineManager* CreateNew( enum MediaFilterMode Mode );
    friend class BaseMemoryManager;

    virtual GMI_RESULT  Initialize();
    virtual GMI_RESULT  Deinitialize();
    virtual GMI_RESULT	AddFilter( StreamingMediaFilter *Filter );
    virtual GMI_RESULT  RemoveFilter( StreamingMediaFilter *Filter );
    virtual GMI_RESULT  FindFilter( int32_t FilterId, StreamingMediaFilter **Filter );
    virtual GMI_RESULT  FindFilter( const char_t *FilterName, size_t FiletNameLength, StreamingMediaFilter **Filter );
    virtual GMI_RESULT  Play();
    virtual GMI_RESULT  Pause();
    virtual GMI_RESULT  Stop();
    inline  enum MediaFilterMode    Mode()
    {
        return m_Mode;
    }
    inline  enum MediaFilterStatus  Status()
    {
        return m_Status;
    }
    int32_t                         AddRef();
    int32_t                         ReleaseRef();
    static  GMI_RESULT  ConnectFilter( StreamingMediaFilter *Output, int32_t OutputPinIndex, StreamingMediaFilter *Input, int32_t InputPinIndex );
    static  GMI_RESULT  DisconnectFilter( StreamingMediaFilter *Output, int32_t OutputPinIndex, StreamingMediaFilter *Input, int32_t InputPinIndex );

protected:

    enum Pipeline_Operation_Type
    {
        POT_Play  = 0,
        POT_Pause = 1,
        POT_Stop  = 2
    };

    GMI_RESULT PipelineReverseOperate  ( enum Pipeline_Operation_Type Type, StreamingMediaFilter *Filter );
    GMI_RESULT _PipelineReverseOperate ( enum Pipeline_Operation_Type Type, StreamingMediaFilter *Filter );

    GMI_RESULT PipelineForwardOperate  ( enum Pipeline_Operation_Type Type, StreamingMediaFilter *Filter );
    GMI_RESULT _PipelineForwardOperate ( enum Pipeline_Operation_Type Type, StreamingMediaFilter *Filter );

protected:
    std::list<StreamingMediaFilter*> m_Filters;

private:
    const enum MediaFilterMode  m_Mode;
    enum MediaFilterStatus	    m_Status;
    int32_t                     m_RefCount;
};
