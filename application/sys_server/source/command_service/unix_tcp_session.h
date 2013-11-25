#ifndef __UNIX_TCP_SESSION_H__
#define __UNIX_TCP_SESSION_H__

#include "base_session.h"
#include "communication_utils_headers.h"

class UnixTcpSession : public BaseSession
{
public:
    UnixTcpSession(void);
    virtual ~UnixTcpSession();
    virtual GMI_RESULT Initialize();
    virtual GMI_RESULT Deinitialize();
    virtual GMI_RESULT Close();
    virtual GMI_RESULT Receive();
    virtual GMI_RESULT Receive(const uint8_t *Buffer, size_t BufferSize);
    virtual int32_t    GetFDHandle();
    virtual size_t     ReadableDataSize();
    virtual GMI_RESULT Read( size_t Offset, uint8_t *Buffer, size_t BufferSize, size_t *Transferred );
    virtual GMI_RESULT ReadAdvance( size_t Size );
    virtual GMI_RESULT Write( const uint8_t *Buffer, size_t BufferSize, size_t *Transferred );
    virtual GMI_RESULT Lock();
    virtual GMI_RESULT Unlock();

    GMI_RESULT Open(int32_t Socket, size_t SessionBufferSize );

private:
    int32_t    m_Socket;
    GMI_Mutex  m_Session_Mutex;
};

#endif




