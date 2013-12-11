#pragma once

#include "gmi_system_headers.h"
#include "ring_buffer.h"

class BaseSession
{
protected:
    BaseSession(void);
public:
    virtual ~BaseSession(void);

    virtual GMI_RESULT Close();
    virtual GMI_RESULT Receive() = 0;
    virtual GMI_RESULT Receive( const uint8_t *Buffer, size_t BufferSize ) = 0;
    // To UDP session server case, FD_HANDLE can not identify some client session.
    // So let subclass support operation similar to GetFDHandle()
    // Pengfei, 2013/2/21
    //virtual FD_HANDLE  GetFDHandle() = 0;
    virtual size_t     ReadableDataSize() = 0;
    virtual GMI_RESULT Read( size_t Offset, uint8_t *Buffer, size_t BufferSize, size_t *Transferred ) = 0;
    virtual GMI_RESULT ReadAdvance( size_t Size ) = 0;
    virtual GMI_RESULT Write( const uint8_t *Buffer, size_t BufferSize, size_t *Transferred ) = 0;
    virtual GMI_RESULT Lock() = 0;
    virtual GMI_RESULT Unlock() = 0;

protected:
    RingBuffer m_ReceiveBuffer;
};
