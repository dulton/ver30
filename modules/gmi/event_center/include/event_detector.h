#pragma once

#include "event_processor.h"
#include "event_process_header.h"

class EventDetector
{
protected:
    EventDetector( enum EventDetectorType Type, uint32_t EventDetectorId ) : m_DetectorType( Type ), m_DetectorId( EventDetectorId ), m_Handle( NULL ), m_ProcessCenter() {}

    // to passive event source, for example, we need to query stauts to know if or not event we concern happen, we use an independent thread to check event, then notify event process center,
    // comparatively speaking, this way is more flexible, but consume more CPU resource.
    virtual GMI_RESULT Start( const void_t *Parameter, size_t ParameterLength )
    {
        return GMI_SUCCESS;
    }

    virtual GMI_RESULT Stop()
    {
        return GMI_SUCCESS;
    }

    void_t  SetEventProcessor( ReferrencePtr<EventProcessor, DefaultObjectDeleter, MultipleThreadModel> ProcessCenter )
    {
        m_ProcessCenter = ProcessCenter;
    }
    friend class EventDetectCenter;

public:
    virtual ~EventDetector(void) {}

    // to active event source, we use a thread to select all event handle, and then notify event process center, event detect center is responsible for doing it, use GetFDHandle() to get handle
    FD_HANDLE GetFDHandle() const
    {
        return m_Handle;
    }

    inline enum EventDetectorType  GetType() const
    {
        return m_DetectorType;
    }

    inline uint32_t                GetId() const
    {
        return m_DetectorId;
    }

private:
    const enum EventDetectorType                                              m_DetectorType;
    const uint32_t                                                            m_DetectorId;

protected:
    FD_HANDLE                                                                 m_Handle;
    ReferrencePtr<EventProcessor, DefaultObjectDeleter, MultipleThreadModel>  m_ProcessCenter;
};
