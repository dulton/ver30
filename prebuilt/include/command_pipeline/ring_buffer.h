// ring_buffer.h

#pragma once

#include "gmi_system_headers.h"

class RingBuffer
{
public:
    RingBuffer();
    ~RingBuffer( void );

    GMI_RESULT Initialize( size_t Capacity );
    GMI_RESULT Deinitialize();

    // integrated write
    size_t Write( const uint8_t *Buffer, size_t Length, boolean_t MoveWritePointer = true );
    // partial read
    size_t Read( uint8_t *Buffer, size_t Length, boolean_t MoveReadPointer = true );
    // integrated write_advance
    size_t WriteAdvance( size_t Length );
    // integrated read_advance
    size_t ReadAdvance( size_t Length );

    // integrated set in ring buffer, only in valid data section
    size_t Set( size_t RelativeOffset, const uint8_t *Buffer, size_t Length );
    // integrated get in ring buffer, only in valid data section
    size_t Get( size_t RelativeOffset, uint8_t *Buffer, size_t Length );

    void_t	Reset();
    void_t	Purge();

    inline size_t Size() const
    {
        return m_Size;
    }

    inline size_t Capacity() const
    {
        return m_Capacity;
    }

private:

    // integrated set in entire ring buffer, not check valid data section
    size_t _Set( size_t AbsoluteWriteOffset, const uint8_t *Buffer, size_t Length );
    // integrated get in entire ring buffer, not check valid data section
    size_t _Get( size_t AbsoluteReadOffset, uint8_t *Buffer, size_t Length );

    // integrated write offset advance in entire ring buffer, not check valid data section
    size_t _WriteAdvance( size_t Length );
    // integrated read offset advance in entire ring buffer, not check valid data section
    size_t _ReadAdvance( size_t Length );

private:
    SafePtr<uint8_t,DefaultObjectsDeleter> m_Buffer;
    size_t	m_Capacity;
    size_t	m_Size;
    size_t	m_ReadOffset;
    size_t	m_WriteOffset;
};
