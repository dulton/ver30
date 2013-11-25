#ifndef __RUDP_SESSION_H__
#define __RUDP_SESSION_H__

#include "base_session.h"
#include "communication_utils_headers.h"

class RUDPSession : public BaseSession
{
public:
    RUDPSession(void);
    virtual ~RUDPSession();
    virtual GMI_RESULT Initialize();
    virtual GMI_RESULT Deinitialize();
    virtual GMI_RESULT Close();
    virtual GMI_RESULT Receive();
    virtual GMI_RESULT Receive(const uint8_t *Buffer, size_t BufferSize);
    virtual FD_HANDLE  GetFDHandle();
    virtual size_t     ReadableDataSize();
    virtual GMI_RESULT Read( size_t Offset, uint8_t *Buffer, size_t BufferSize, size_t *Transferred );
    virtual GMI_RESULT ReadAdvance( size_t Size );
    virtual GMI_RESULT Write( const uint8_t *Buffer, size_t BufferSize, size_t *Transferred );
    virtual GMI_RESULT Lock();
    virtual GMI_RESULT Unlock();

    GMI_RESULT Open( FD_HANDLE RudpSendSocket, uint16_t RemoteSPort, size_t SessionBufferSize );

private:
    FD_HANDLE  m_RudpSendSocket;
    uint16_t   m_RemoteSPort;
    GMI_Mutex  m_Session_Mutex;
};

#endif

