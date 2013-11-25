#pragma once

#include "base_communication.h"

enum SerialPortSpeed
{
    SPS_B0      = 0,
    SPS_B1200   = 1200,
    SPS_B2400   = 2400,
    SPS_B4800   = 4800,
    SPS_B9600   = 9600,
    SPS_B19200  = 19200,
    SPS_B38400  = 38400,
    SPS_B57600  = 57600,
    SPS_B115200 = 115200
};

enum SerialPortDataBits
{
    SPDB_5 = 5,
    SPDB_6 = 6,
    SPDB_7 = 7,
    SPDB_8 = 8
};

enum SerialPortParity
{
    SPP_None  = 0,
    SPP_Odd   = 1,
    SPP_Even  = 2,
};

enum SerialPortStopBits
{
    SPSB_1 = 1,
    SPSB_2 = 2
};

enum SerialPortStreamControl
{
    SPSC_None     = 0,
    SPSC_Software = 1,
    SPSC_Hardware = 2,
};

class BaseSerialPort : public BaseCommunication
{
protected:
    BaseSerialPort(void);
public:
    virtual ~BaseSerialPort(void);

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

    virtual GMI_RESULT Open( const char_t *PortName, enum SerialPortSpeed BaudRate, enum SerialPortDataBits DataBits, enum SerialPortParity Parity, enum SerialPortStopBits StopBits, enum SerialPortStreamControl StreamControl )
    {
        return GMI_NOT_IMPLEMENT;
    }
};
