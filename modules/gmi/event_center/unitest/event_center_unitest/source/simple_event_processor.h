#pragma once

#include "event_processor.h"

class SimpleEventProcessor : public EventProcessor
{
public:
    SimpleEventProcessor(void);
    virtual ~SimpleEventProcessor(void);

    virtual GMI_RESULT Notify( uint32_t EventId, void_t *Parameter, size_t ParamLength );
};
