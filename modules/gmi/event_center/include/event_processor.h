#pragma once

#include "event_process_header.h"

class EventProcessor
{
protected:
    EventProcessor( uint32_t EventProcessorId ) : m_ProcessorId( EventProcessorId ) {}

public:
    virtual ~EventProcessor(void) {}

    // on usual, Start and Stop do not need to take specail measure, return GMI_SUCCESS simply. but for some case, some thread can be created to execute special task.
    virtual GMI_RESULT Start( const void_t *Parameter, size_t ParameterLength )
    {
        return GMI_SUCCESS;
    }
    virtual GMI_RESULT Stop()
    {
        return GMI_SUCCESS;
    }

    virtual GMI_RESULT Notify( uint32_t EventId, void_t *Parameter, size_t ParamLength ) = 0;
    uint32_t GetId() const
    {
        return m_ProcessorId;
    }

private:
    uint32_t  m_ProcessorId;
};
