#pragma once

#include "base_session.h"

#define TCP_SESSION_BUFFER_SIZE 8192
#define TCP_SESSION_IO_WAITTIME 10

class TcpSession : public BaseSession
{
public:
    TcpSession(void);
    virtual ~TcpSession(void);

    virtual GMI_RESULT Close();
    virtual GMI_RESULT Receive();
    virtual GMI_RESULT Receive( const uint8_t *Buffer, size_t BufferSize );
    virtual size_t     ReadableDataSize();
    virtual GMI_RESULT Read( size_t Offset, uint8_t *Buffer, size_t BufferSize, size_t *Transferred );
    virtual GMI_RESULT ReadAdvance( size_t Size );
    virtual GMI_RESULT Write( const uint8_t *Buffer, size_t BufferSize, size_t *Transferred );
    virtual GMI_RESULT Lock();
    virtual GMI_RESULT Unlock();
    virtual GMI_RESULT Open( ReferrencePtr<GMI_Socket> TCP_Socket, size_t SessionBufferSize );
    FD_HANDLE  GetFDHandle();

private:
    GMI_Mutex                 m_InstanceMutex;
protected:
    ReferrencePtr<GMI_Socket> m_TCP_Socket;
    GMI_Mutex                 m_OperationMutex;
};
