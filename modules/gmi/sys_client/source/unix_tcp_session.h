#ifndef __UNIX_TCP_SESSION_H__
#define __UNIX_TCP_SESSION_H__

#include "gmi_system_headers.h"

GMI_RESULT UnixTcpOpen(int32_t *SocketFd);
GMI_RESULT UnixTcpClose(int32_t SocketFd);
GMI_RESULT UnixTcpReceive(int32_t Fd, struct timeval *TimeoutPtr, uint8_t *Buffer, size_t Buffersize, size_t *Transferred);
GMI_RESULT UnixTcpSend(int32_t Fd, uint8_t *Buffer, size_t BufferSize, size_t *Transferred);


#endif

