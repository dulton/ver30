#pragma once

#include "event_process_header.h"

class EventProcessor
{
protected:
    EventProcessor( uint32_t EventProcessorId, uint32_t Index ) : m_Callback( NULL ), m_UserData( NULL ), m_ProcessorId( EventProcessorId ), m_Index( Index ) 
	{
	}

    // on usual, Start and Stop do not need to take specail measure, return GMI_SUCCESS simply. but for some case, some thread can be created to execute special task.
    virtual GMI_RESULT Start( const void_t *Parameter, size_t ParameterLength )
    {
        return GMI_SUCCESS;
    }

    virtual GMI_RESULT Stop()
    {
        return GMI_SUCCESS;
    }
    friend class EventProcessCenter;

public:
    virtual ~EventProcessor(void) {}

    // Instance Lock interface, instead of Operation Lock interface
    virtual GMI_RESULT Lock()
    {
        return GMI_SUCCESS;
    }

    // Instance Lock interface, instead of Operation Lock interface
    virtual GMI_RESULT Unlock()
    {
        return GMI_SUCCESS;
    }

    virtual GMI_RESULT Notify( uint32_t EventId, uint32_t Index, enum EventType Type, void_t *Parameter, size_t ParameterLength ) = 0;

    void_t  AddDetectorId( struct DetectorInfo DetectorData )
    {
        std::vector<struct DetectorInfo>::iterator DetectorIdIt = m_DetectorIds.begin(), DetectorIdEnd = m_DetectorIds.end();
        for ( ; DetectorIdIt != DetectorIdEnd ; ++DetectorIdIt )
        {
            if ( (DetectorData.s_DetectorId == (*DetectorIdIt).s_DetectorId)
				&& (DetectorData.s_Index == (*DetectorIdIt).s_Index))
            {
                return;
            }
        }

        m_DetectorIds.push_back( DetectorData );
    }

    void_t  RemoveDetectId( struct DetectorInfo DetectorData )
    {
        std::vector<struct DetectorInfo>::iterator DetectorIdIt = m_DetectorIds.begin(), DetectorIdEnd = m_DetectorIds.end();
        for ( ; DetectorIdIt != DetectorIdEnd ; ++DetectorIdIt )
        {
            if ( (DetectorData.s_DetectorId == (*DetectorIdIt).s_DetectorId)
				&& (DetectorData.s_Index == (*DetectorIdIt).s_Index))
            {
                m_DetectorIds.erase( DetectorIdIt );
                return;
            }
        }
    }

    void_t  SetEventCallback( EventCallback Callback, void_t *UserData )
    {
        m_Callback = Callback;
        m_UserData = UserData;
    }

    inline uint32_t GetId() const
    {
        return m_ProcessorId;
    }

	
	inline uint32_t GetIndex() const
	{
		return m_Index;
	}

protected:
    std::vector<struct DetectorInfo>  m_DetectorIds;
    EventCallback          m_Callback;
    void_t                 *m_UserData;

private:
    uint32_t               m_ProcessorId;
	uint32_t               m_Index;
};


