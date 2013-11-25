#pragma once

#include "data_object.h"

class DataHandler : public DataObject
{
protected:
    DataHandler( boolean_t ActiveObject );
    virtual ~DataHandler(void);

public:
    virtual GMI_RESULT      Receive( const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength );
    void_t                  SetMemoryPool( MemoryPoolParallelConsumers *Pool );

private:
    virtual void_t*         DataReceive();
};
