// communication_utils_headers.h
#if !defined( GMI_COMMUNICATION_UTILS_HEADERS )
#define GMI_COMMUNICATION_UTILS_HEADERS

#if defined( __linux__ )

#include "linux_serial_port.h"
#include "linux_socket.h"

typedef LinuxSerialPort     GMI_SerialPort;
typedef LinuxSocket         GMI_Socket;

#elif defined( _WIN32 )

#include "windows_serial_port.h"
#include "windows_socket.h"

typedef WindowsSerialPort   GMI_SerialPort;
typedef WindowsSocket       GMI_Socket;

#else
#error not support current platform
#endif

#define GMI_GetSystemSocketProtocolFamily   BaseSocket::GetSystemSocketProtocolFamily
#define GMI_GetSystemSocketType	            BaseSocket::GetSystemSocketType
#define GMI_GetSystemSocketProtocol         BaseSocket::GetSystemSocketProtocol

#endif//GMI_COMMUNICATION_UTILS_HEADERS
