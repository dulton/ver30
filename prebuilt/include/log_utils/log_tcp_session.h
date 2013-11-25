#pragma once

#include "tcp_session.h"

#define LOG_TCP_SESSION_MAX_BUFFER_MULTIPLER 1024
#define LOG_TCP_SESSION_IO_WAITTIME          10

class LogTcpSession : public TcpSession
{
public:
    LogTcpSession(void);
    virtual ~LogTcpSession(void);

    virtual GMI_RESULT Close();
    virtual GMI_RESULT Receive();
    virtual GMI_RESULT Write( const uint8_t *Buffer, size_t BufferSize, size_t *Transferred );
    virtual GMI_RESULT Open( ReferrencePtr<GMI_Socket> TCP_Socket, size_t SessionBufferSize );

    void_t     SetToken( uint32_t Token )
    {
        m_Token = Token;
    }
    uint32_t   GetToken() const
    {
        return m_Token;
    }
    uint16_t   GetSequenceNumber()
    {
        return ++m_SequenceNumber;
    }
    void_t     MarkToRemove( boolean_t Remove )
    {
        m_ToRemove = Remove;
    }
    boolean_t  NeedToRemoved() const
    {
        return m_ToRemove;
    }

private:
    RingBuffer    m_Buffer;
    uint8_t       *m_SendBuffer;
    size_t        m_SendBufferSize;
    uint32_t      m_Token;
    uint16_t      m_SequenceNumber;
    boolean_t     m_ToRemove;
};
