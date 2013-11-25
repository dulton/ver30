#pragma once

#include "base_serial_port.h"

class LinuxSerialPort : public BaseSerialPort
{
public:
    LinuxSerialPort(void);
    virtual ~LinuxSerialPort(void);

#if defined( __linux__ )
    virtual GMI_RESULT Close();
    virtual GMI_RESULT Send( const uint8_t *Buffer, size_t BufferSize, long_t Flags, size_t *Transferred );
    virtual GMI_RESULT Receive( uint8_t *Buffer, size_t BufferSize, long_t Flags, size_t *Transferred );
    virtual GMI_RESULT SendTo( const uint8_t *Buffer, size_t BufferSize, long_t Flags, const struct sockaddr* To, long_t ToLength, size_t *Transferred );
    virtual GMI_RESULT ReceiveFrom( uint8_t *Buffer, size_t BufferSize, long_t Flags, struct sockaddr *From, long_t *FromLength, size_t *Transferred );
    virtual FD_HANDLE  GetFDHandle();

    virtual GMI_RESULT Open( const char_t *PortName, enum SerialPortSpeed BaudRate, enum SerialPortDataBits DataBits, enum SerialPortParity Parity, enum SerialPortStopBits StopBits, enum SerialPortStreamControl StreamControl );

private:
    GMI_RESULT  SetBaudRate     ( enum SerialPortSpeed BaudRate );
    GMI_RESULT  SetDataBits     ( enum SerialPortDataBits DataBits );
    GMI_RESULT  SetParity       ( enum SerialPortParity Parity );
    GMI_RESULT  SetStopBits     ( enum SerialPortStopBits StopBits );
    GMI_RESULT  SetStreamControl( enum SerialPortStreamControl StreamControl );

private:
    long_t	        m_SeriralPortFD;
    struct termios  m_Options;
#endif
};
