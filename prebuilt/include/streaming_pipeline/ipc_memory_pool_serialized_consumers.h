#pragma once

#include "gmi_system_headers.h"

class IpcMemoryPoolSerializedConsumers
{
public:
    IpcMemoryPoolSerializedConsumers(void);
    ~IpcMemoryPoolSerializedConsumers(void);

    GMI_RESULT      Initialize( long_t MutexId, void_t *ShareMemoryBuffer, size_t BufferSize, size_t MaxDataBlockSize, size_t MinDataBlockSize, uint32_t MediaType, uint32_t MediaId );
    GMI_RESULT      Deinitialize();

    GMI_RESULT      Open( void_t *ShareMemoryBuffer );
    GMI_RESULT      Close();

    int32_t         AddRef();
    int32_t         ReleaseRef();
    int32_t         GetRefCountThreadSafety();
    inline int32_t  GetRefCountNonThreadSafety()
    {
        return *m_BlockRefCount;
    }
    int32_t         SetRefCount( int32_t Count );

    GMI_RESULT      AllocateBlock( size_t Size, boolean_t ForceAllocate, void_t*& Block );
    boolean_t       PushBackBlock( void_t *Block );
    boolean_t       NextBlock( void_t*& Block, size_t *Length, void_t*& NextDataBlock );
    size_t		    ReleaseBlock( void_t *Block, boolean_t doneNext );
    void_t          Status( int32_t Level );
    boolean_t       CheckBlockPointer( void_t *Block );
    static size_t   BlockSize( void_t *Block );

    inline uint32_t GetMediaType()
    {
        return *m_MediaType;
    }
    inline uint32_t GetMediaId()
    {
        return *m_MediaId;
    }

private:
    void_t*         RemovePrevFromDataQueue( void_t *Node );
    void_t*         RemoveNodeFromDataQueue( void_t *Node );
    void_t*         AddToBufferQueue( void_t *Node );
    void_t*         CompelToRemoveNodeFromDataQueue();
    void_t          ReportStatus( const char_t *FunctionName, int32_t Level );

private:
    void_t          *m_Buffer;
    long_t          *m_MutexId;

    uint32_t	    *m_MediaType;
    uint32_t		*m_MediaId;
    size_t          *m_BufferSize;
    size_t          *m_MaxDataBlockSize;

    size_t          *m_BlockSize;
    size_t          *m_BlockNumber;
    int32_t         *m_BlockRefCount;

    int32_t         *m_IdleBlockCount;
    int32_t         *m_DataBlockCount;
    size_t			*m_PrintCount;
    int32_t			*m_BytesCount;

    size_t          *m_IdleHead;
    size_t          *m_IdleTail;
    size_t          *m_DataHead;
    size_t          *m_DataTail;
    size_t          *m_BlockOffsetArray;

    GMI_MutexIPC    m_Mutex;
};
