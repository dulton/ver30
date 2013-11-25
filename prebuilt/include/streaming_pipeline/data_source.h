#pragma once

#include "data_object.h"

class DataSource : public DataObject
{
protected:
    DataSource( boolean_t ActiveObject );
    virtual ~DataSource(void);

public:
    virtual GMI_RESULT	    Initialize( uint32_t MediaType, uint32_t MediaId, MemoryPoolParallelConsumers *Pool );
    virtual GMI_RESULT      Deinitialize();
    virtual GMI_RESULT      Receive( const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength );
    MemoryPoolParallelConsumers* GetMemoryPool()
    {
        return m_MemoryPool;
    }

    inline void_t           AllowForceAllocateBuffer( boolean_t Enable )
    {
        m_AllowForceAllocateBuffer = Enable;
    }
    inline boolean_t        IsAllowForceAllocateBuffer() const
    {
        return m_AllowForceAllocateBuffer;
    }

private:
    virtual void_t*         DataReceive();

private:
    boolean_t  m_AllowForceAllocateBuffer;
};
