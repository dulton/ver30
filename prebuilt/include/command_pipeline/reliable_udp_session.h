#pragma once

#include "udp_session.h"

#define RELIABLE_UDP_DEFAULT_RESEND_NUMBER   3

class ReliableUDPSession : public UDPSession
{
public:
    ReliableUDPSession();
    virtual ~ReliableUDPSession(void);

    virtual GMI_RESULT Receive();
    virtual GMI_RESULT Receive( const uint8_t *Buffer, size_t BufferSize );
    virtual GMI_RESULT Open( ReferrencePtr<GMI_Socket> UDP_Socket, long_t Remote_UDP_IP, uint16_t Remote_UDP_Port, size_t SessionBufferSize );
    GMI_RESULT         SendCommand( const uint8_t *Buffer, size_t BufferSize, size_t *Transferred, long_t TryCount );
    GMI_RESULT         SendNotify( const uint8_t *Buffer, size_t BufferSize, size_t *Transferred, long_t TryCount );
    // for udp client
    GMI_RESULT         Receive( uint8_t *Buffer, size_t BufferSize, size_t *Transferred, long_t TryCount );
    // for udp server
    GMI_RESULT         Receive( uint8_t *Buffer, size_t BufferSize, long_t Flags, struct sockaddr *From, long_t *FromLength, size_t *Transferred, long_t TryCount );

private:
    GMI_RESULT  ReceiveTimeout( uint8_t *Buffer, size_t BufferSize, long_t Flags, struct sockaddr *From, long_t *FromLength, size_t *Transferred, long_t Timeout /*ms unit*/ );
    GMI_RESULT  ReceiveTimeout( uint8_t *Buffer, size_t BufferSize, long_t Flags, long_t IP, uint16_t Port, size_t *Transferred, long_t Timeout /*ms unit*/ );

private:
    FD_HANDLE   m_SocketFD;
    uint8_t     m_SendBuffer[UDP_MAX_MESSAGE_LENGTH];
    uint8_t     m_RecvBuffer[UDP_MAX_MESSAGE_LENGTH];
    uint16_t    m_LocalSequenceNumber;
    uint16_t    m_RemoteSequenceNumber;
};
