#pragma once

#include "ipc_media_data_client.h"
#include "memory_pool_parallel_consumers.h"
#include "streaming_media_source.h"

class IpcMediaDataDispatchSource : public StreamingMediaSource
{
protected:
    IpcMediaDataDispatchSource(void);
    virtual ~IpcMediaDataDispatchSource(void);

public:
    static  IpcMediaDataDispatchSource*  CreateNew();
    friend class BaseMemoryManager;

    virtual GMI_RESULT Initialize  ( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize );
    virtual GMI_RESULT Deinitialize();
    virtual GMI_RESULT Play();
    virtual GMI_RESULT Stop();

    inline uint32_t   GetSourceId()  const
    {
        return m_SourceId;
    }
    inline uint32_t   GetMediaId()   const
    {
        return m_MediaId;
    }
    inline uint32_t   GetMediaType() const
    {
        return m_MediaType;
    }
    inline uint32_t   GetCodecType() const
    {
        return m_CodecType;
    }

private:
    static void_t* FetchThread( void_t *Argument );
    void_t* FetchEntry();

    GMI_RESULT GetServerUDPPort ( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, uint16_t *UDP_Port );
    GMI_RESULT GetClientUDPPort ( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, uint16_t *UDP_Port );

    GMI_RESULT GetDecodeSourceMonitorEnableConfig( boolean_t *Enable );
    GMI_RESULT GetDecodeSourceFrameCheckInterval( uint32_t *FrameNumber );

private:
    IPC_MediaDataClient          m_MediaDataClient;
    void_t                       *m_Buffer;
    size_t                       m_BufferSize;
    MemoryPoolParallelConsumers  *m_DataPool;

    GMI_Thread			 m_FetchThread;
    boolean_t            m_ThreadWorking;
    boolean_t            m_ThreadExitFlag;

    void_t               *m_FrameBuffer;
    size_t               m_FrameBufferSize;
    void_t               *m_ExtraData;
    size_t               m_ExtraDataSize;

    uint32_t             m_SourceId;
    uint32_t             m_MediaId;
    uint32_t             m_MediaType;
    uint32_t             m_CodecType;
    uint32_t             m_FrameRate;

    boolean_t            m_DecodeSourceMonitorEnable;
    uint32_t             m_DecodeSourceFrameCheckInterval;
    uint64_t             m_FrameNumber;
    struct timeval       m_FirstFrameTime;
};
