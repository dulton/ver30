#pragma once

#include "base_socket.h"

class LinuxSocket : public BaseSocket
{
public:
    LinuxSocket(void);
    virtual ~LinuxSocket(void);

#if defined( __linux__ )
    virtual GMI_RESULT Close      ();
    virtual GMI_RESULT Send       ( const uint8_t *Buffer, size_t BufferSize, long_t Flags, size_t *Transferred );
    virtual GMI_RESULT Receive    ( uint8_t *Buffer, size_t BufferSize, long_t Flags, size_t *Transferred );
    virtual GMI_RESULT SendTo     ( const uint8_t *Buffer, size_t BufferSize, long_t Flags, const struct sockaddr* To, long_t ToLength, size_t *Transferred );
    virtual GMI_RESULT ReceiveFrom( uint8_t *Buffer, size_t BufferSize, long_t Flags, struct sockaddr *From, long_t *FromLength, size_t *Transferred );
    virtual FD_HANDLE  GetFDHandle();

    virtual GMI_RESULT Create     ( enum SocketProtocolFamily ProtocolFamily, enum SocketType Type, enum SocketProtocol Protocol );
    virtual GMI_RESULT Bind       ( const struct sockaddr* Name, long_t NameLength );
    virtual GMI_RESULT Open       ( FD_HANDLE SocketHandle );
    virtual GMI_RESULT Listen     ( uint32_t MaxPendingConnectionNumber );
    virtual GMI_RESULT Accept     ( BaseSocket *Socket );
    virtual GMI_RESULT Connect    ( const struct sockaddr* Name, long_t NameLength );
    virtual GMI_RESULT SetOption  ( long_t Level, long_t OptionName, const char_t* OptionValue, long_t OptionLength );
    virtual GMI_RESULT GetOption  ( long_t Level, long_t OptionName, char_t* OptionValue, long_t* OptionLength );

    static long_t  GetSystemSocketProtocolFamily( enum SocketProtocolFamily ProtocolFamily );
    static long_t  GetSystemSocketType          ( enum SocketType Type );
    static long_t  GetSystemSocketProtocol      ( enum SocketProtocol Protocol );

private:
    long_t          m_Socket;
    struct sockaddr m_Address;
#endif
};
