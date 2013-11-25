#pragma once

#include "ipc_media_data_server.h"
#include "streaming_media_handler.h"

class IpcMediaDataDispatchHandler : public StreamingMediaHandler
{
protected:
    IpcMediaDataDispatchHandler(void);
    virtual ~IpcMediaDataDispatchHandler(void);

public:
    static  IpcMediaDataDispatchHandler*  CreateNew();
    friend class BaseMemoryManager;

    virtual GMI_RESULT Initialize  ( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize );
    virtual GMI_RESULT Deinitialize();
    virtual GMI_RESULT Play();
    virtual GMI_RESULT Stop();
    virtual GMI_RESULT Receive     ( int32_t InputPinIndex, const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength );

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
    GMI_RESULT GetUDPPort       ( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, uint16_t *UDP_Port );
    GMI_RESULT GetShareMemoryKey( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, long_t *ShareMemoryKey );
    GMI_RESULT GetIpcMutexKey   ( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, long_t *IpcMutexKey );

private:
    IPC_MediaDataServer  m_MediaDataServer;
    uint32_t             m_SourceId;
    uint32_t             m_MediaId;
    uint32_t             m_MediaType;
    uint32_t             m_CodecType;
    uint32_t             m_BitRate;
    uint32_t             m_FrameRate;
};
