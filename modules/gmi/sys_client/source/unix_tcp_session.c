#include <stdio.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include "log.h"
#include "unix_tcp_session.h"
#include "gmi_system_headers.h"

#define CLI_PATH "/tmp/"
#define SERVER_PATH "/tmp/sys_server"
#define Offsetof(TYPE, MEMBER)  ((int)&((TYPE*)0)->MEMBER)


int CliConn(const char *Name)
{
    int Fd, Len;
    struct sockaddr_un SockUn;

    if ((Fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }

    memset(&SockUn, 0, sizeof(SockUn));
    SockUn.sun_family = AF_UNIX;
    sprintf(SockUn.sun_path, "%s%05d", CLI_PATH, getpid());
    Len = Offsetof(struct sockaddr_un, sun_path) + strlen(SockUn.sun_path);

    unlink(SockUn.sun_path);
    if (bind(Fd, (struct sockaddr*)&SockUn, Len) < 0)
    {
        if (0 < Fd)
        {
            close(Fd);
            Fd = -1;
        }
        return -2;
    }

    memset(&SockUn, 0, sizeof(SockUn));
    SockUn.sun_family = AF_UNIX;
    strcpy(SockUn.sun_path, Name);
    Len = Offsetof(struct sockaddr_un, sun_path) + strlen(Name);
    if (connect(Fd, (struct sockaddr*)&SockUn, Len) < 0)
    {
        if (0 < Fd)
        {
            close(Fd);
            Fd = -1;
        }
        return -3;
    }

    return Fd;
}


GMI_RESULT UnixTcpSend(int32_t Fd, uint8_t *Buffer, size_t BufferSize, size_t *Transferred)
{
    size_t Result = send(Fd, Buffer, BufferSize, 0);
    if (Result != BufferSize)
    {
        SYS_CLIENT_ERROR("Result = %d\n", Result);
        return GMI_FAIL;
    }

    *Transferred = Result;

    return GMI_SUCCESS;
}


GMI_RESULT UnixTcpReceive(int32_t Fd, struct timeval *TimeoutPtr, uint8_t *Buffer, size_t Buffersize, size_t *Transferred)
{
    struct timeval Timeout;
    fd_set FdSet;

    Timeout.tv_sec  = TimeoutPtr->tv_sec;
    Timeout.tv_usec = TimeoutPtr->tv_usec;
    FD_ZERO(&FdSet);
    FD_SET(Fd, &FdSet);
    int32_t Ret = select(Fd+1, &FdSet, NULL, NULL, &Timeout);
    if (Ret < 0)
    {
        SYS_CLIENT_ERROR("select error. errno = %d\n", errno);
        return GMI_FAIL;
    }
    else if (Ret == 0)
    {
        SYS_CLIENT_ERROR("select timeout. errno = %d\n", errno);
        return GMI_WAIT_TIMEOUT;
    }
    else
    {
        size_t RevLen;
        RevLen = recv(Fd, Buffer, Buffersize, 0);
        if (RevLen != Buffersize)
        {
            SYS_CLIENT_ERROR("recv buffer data error, Ret = %d, buffersize = %d\n", RevLen, Buffersize);
            return GMI_FAIL;
        }
        else
        {
            *Transferred = RevLen;
        }
    }

    return GMI_SUCCESS;
}


GMI_RESULT UnixTcpOpen(int32_t *SocketFd)
{
    int32_t Fd = CliConn(SERVER_PATH);
    if (0 > Fd)
    {
        return GMI_FAIL;
    }

    *SocketFd = Fd;
    return GMI_SUCCESS;
}


GMI_RESULT UnixTcpClose(int32_t SocketFd)
{
    close(SocketFd);
    return GMI_SUCCESS;
}


