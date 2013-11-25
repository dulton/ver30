#pragma once

#include "base_session.h"

#define UDP_SESSION_BUFFER_SIZE 8192
#define UDP_MAX_MESSAGE_LENGTH  1500
#define UDP_SESSION_IO_WAITTIME 10

class UDPSession : public BaseSession
{
public:
    UDPSession(void);
    virtual ~UDPSession(void);

    virtual GMI_RESULT Close();
    virtual GMI_RESULT Receive();
    virtual GMI_RESULT Receive( const uint8_t *Buffer, size_t BufferSize );
    virtual size_t     ReadableDataSize();
    virtual GMI_RESULT Read( size_t Offset, uint8_t *Buffer, size_t BufferSize, size_t *Transferred );
    virtual GMI_RESULT ReadAdvance( size_t Size );
    virtual GMI_RESULT Write( const uint8_t *Buffer, size_t BufferSize, size_t *Transferred );
    virtual GMI_RESULT Lock();
    virtual GMI_RESULT Unlock();
    virtual GMI_RESULT Open( ReferrencePtr<GMI_Socket> UDP_Socket, long_t Remote_UDP_IP, uint16_t Remote_UDP_Port, size_t SessionBufferSize );

    inline  long_t     GetRemoteIP() const
    {
        return m_Remote_UDP_IP;
    }
    inline  uint16_t   GetRemotePort() const
    {
        return m_Remote_UDP_Port;
    }

protected:
    ReferrencePtr<GMI_Socket> m_UDP_Socket;
    long_t                    m_Remote_UDP_IP;
    uint16_t                  m_Remote_UDP_Port;
    GMI_Mutex                 m_OperationMutex;
private:
    GMI_Mutex                 m_InstanceMutex;
};
