#pragma once

#include "base_communication.h"

enum SocketProtocolFamily
{
    SPF_INET = 0,
    SPF_INET6 = 1
};

enum SocketType
{
    ST_STREEAM = 0,
    ST_DATAGRAM = 1
};

enum SocketProtocol
{
    SP_DEFAULT = 0,
    SP_TCP = 1,
    SP_UDP = 2,
};

// socket option level
#define GMI_SOCKET_OPTION                     1

// socket option
#define GMI_SOCKET_OPTION_NONBLOCK_MODE       1
//#define GMI_SOCKET_OPTION_SEND_TIMEOUT      2
//#define GMI_SOCKET_OPTION_RECV_TIMEOUT      3
//#define GMI_SOCKET_OPTION_SEND_BUFFER_SIZE  4
//#define GMI_SOCKET_OPTION_RECV_BUFFER_SIZE  5

class BaseSocket : public BaseCommunication
{
protected:
    BaseSocket(void);
public:
    virtual ~BaseSocket(void);

    virtual GMI_RESULT Close()
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Send( const uint8_t *Buffer, size_t BufferSize, long_t Flags, size_t *Transferred )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Receive( uint8_t *Buffer, size_t BufferSize, long_t Flags, size_t *Transferred )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT SendTo( const uint8_t *Buffer, size_t BufferSize, long_t Flags, const struct sockaddr* To, long_t ToLength, size_t *Transferred )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT ReceiveFrom( uint8_t *Buffer, size_t BufferSize, long_t Flags, struct sockaddr *From, long_t *FromLength, size_t *Transferred )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual FD_HANDLE  GetFDHandle()
    {
        return NULL;
    }

    virtual GMI_RESULT Create( enum SocketProtocolFamily ProtocolFamily, enum SocketType Type, enum SocketProtocol Protocol )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Bind( const struct sockaddr* Name, long_t NameLength )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Open( FD_HANDLE SocketHandle )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Listen( uint32_t MaxPendingConnectionNumber )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Accept( BaseSocket *Socket )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Connect( const struct sockaddr* Name, long_t NameLength )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT SetOption( long_t Level, long_t OptionName, const char_t* OptionValue, long_t OptionLength )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT GetOption( long_t Level, long_t OptionName, char_t* OptionValue, long_t* OptionLength )
    {
        return GMI_NOT_IMPLEMENT;
    }

    static long_t  GetSystemSocketProtocolFamily( enum SocketProtocolFamily ProtocolFamily );
    static long_t  GetSystemSocketType          ( enum SocketType Type );
    static long_t  GetSystemSocketProtocol      ( enum SocketProtocol Protocol );
};
