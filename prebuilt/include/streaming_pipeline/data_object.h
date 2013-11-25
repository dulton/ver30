#pragma once

#include "gmi_system_headers.h"
#include "memory_pool_parallel_consumers.h"

enum DataObjectStatus
{
    DOS_Init    = 0,
    DOS_Uninit  = 1,
    DOS_Playing = 2,
    DOS_Paused  = 3,
    DOS_Stopped = 4
};

class DataObject
{
protected:
    DataObject( boolean_t ActiveObject );
    virtual ~DataObject(void);
    friend class BaseMemoryManager;

public:
    virtual GMI_RESULT	    Initialize( uint32_t MediaType, uint32_t MediaId );
    virtual GMI_RESULT      Deinitialize();
    virtual GMI_RESULT      Play();
    virtual GMI_RESULT      Pause();
    virtual GMI_RESULT      Stop();
    virtual GMI_RESULT      Receive( const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength );
    inline uint32_t         GetMediaType()
    {
        return m_MediaType;
    }
    inline uint32_t		    GetMediaId()
    {
        return m_MediaId;
    }
    inline DataObjectStatus Stauts()
    {
        return m_Status;
    }
    int32_t                 AddRef();
    int32_t                 ReleaseRef();

protected:
    inline boolean_t        IsActiveObject() const
    {
        return m_ActiveObject;
    }
    inline boolean_t        DoThreadExit() const
    {
        return m_DataReceiveThreadExitFlag;
    }

    GMI_RESULT              SendSignal();
    GMI_RESULT              WaitSignal( long_t Timeout );
    GMI_RESULT              AddDataItem( const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength );

    virtual GMI_RESULT      Process( const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength ) = 0;

private:
    static  void_t*         DataReceiveEntry( void_t *Argument );
    virtual void_t*         DataReceive() = 0;

protected:
    MemoryPoolParallelConsumers  *m_MemoryPool;
    GMI_Mutex                    m_ProcessDataItemsMutex;
    struct ProcessData
    {
        const uint8_t        *s_Frame;
        size_t               s_FrameLength;
        const struct timeval *s_FrameTS;
        const void_t         *s_ExtraData;
        size_t               s_ExtraDataLength;
        ProcessData( const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength )
            : s_Frame( Frame )
            , s_FrameLength( FrameLength )
            , s_FrameTS( FrameTS )
            , s_ExtraData( ExtraData )
            , s_ExtraDataLength( ExtraDataLength )
        {
        }
    };
    std::list<ProcessData>  m_ProcessDataItems;
    const long_t            m_ThreadCheckInterval;

private:
    const boolean_t         m_ActiveObject;
    uint32_t                m_MediaType;
    uint32_t                m_MediaId;
    enum DataObjectStatus   m_Status;
    int32_t                 m_RefCount;

    GMI_Thread              m_DataReceiveThread;
    boolean_t               m_DataReceiveThreadWorking;
    boolean_t               m_DataReceiveThreadExitFlag;
    boolean_t               m_NeedDataReceivedSignal;
    GMI_Event               m_DataReceivedSignal;

};
