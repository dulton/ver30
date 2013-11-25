#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>

#include <rtsp_stream_ctrl.h>

#include "debug.h"

#define RTSP_CTRL_SOCK_FILE         "/tmp/rtsp_ctrl_sock"
#define RTSP_CTRL_CLIENT_FMT_STRING "/tmp/rtsp_ctrl_client_%d_sock"
#define SOCK_FILE_NAME_LENGTH       128
#define MAX_PACKET_SIZE             1024
#define TIME_OUT_USEC               1000000

enum
{
    eOpenStream = 0,
    eCloseStream,
    eQueryStream,
};

typedef struct tagRequest
{
    uint32_t s_Operation;
    uint32_t s_StreamId;
} Request;

typedef struct tagRequestOpenStream
{
    Request       s_Request;
    uint32_t      s_SubStreamNum;
    SubStreamInfo s_SubStreamInfo[0];
} RequestOpenStream;

typedef struct tagRequestCloseStream
{
    Request s_Request;
} RequestCloseStream;

typedef struct tagRequestQueryStream
{
    Request s_Request;
} RequestQueryStream;

typedef struct tagResponse
{
    uint32_t   s_Operation;
    uint32_t   s_StreamId;
    GMI_RESULT s_RetVal;
} Response;

typedef struct tagResponseOpenStream
{
    Response s_Response;
} ResponseOpenStream;

typedef struct tagResponseCloseStream
{
    Response s_Response;
} ResponseCloseStream;

typedef struct tagResponseQueryStream
{
    Response s_Response;
    uint32_t s_Status;
} ResponseQueryStream;

static uint8_t l_Buffer[MAX_PACKET_SIZE];

static const char_t * MakeClientSockFileName()
{
    static char_t FileName[SOCK_FILE_NAME_LENGTH];
    snprintf(FileName, sizeof(FileName), RTSP_CTRL_CLIENT_FMT_STRING, getpid());
    return FileName;
}

static GMI_RESULT SendAndRecv(const uint8_t * SendBuf, uint32_t SendBufSize, uint8_t * RecvBuf, uint32_t * RecvBufSize, uint32_t USecTimedOut)
{
    GMI_RESULT           RetVal    = GMI_SUCCESS;
    int                  SockFd    = -1;
    struct sockaddr_un   AddrLocal;
    struct sockaddr_un   AddrRemote;
    const char_t       * SockFile  = MakeClientSockFileName();
    fd_set               Fds;
    struct timeval       TimedOut  = { USecTimedOut / 1000000, USecTimedOut % 1000000 };
    int                  RetSelect = 0;


    if (NULL == SendBuf || 0 == SendBufSize || NULL == RecvBuf || NULL == RecvBufSize || 0 == *RecvBufSize)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    memset(&AddrLocal, 0x00, sizeof(AddrLocal));
    AddrLocal.sun_family = AF_UNIX;
    strncpy(AddrLocal.sun_path, SockFile, sizeof(AddrLocal.sun_path) - 1);

    memset(&AddrRemote, 0x00, sizeof(AddrLocal));
    AddrRemote.sun_family = AF_UNIX;
    strncpy(AddrRemote.sun_path, RTSP_CTRL_SOCK_FILE, sizeof(AddrRemote.sun_path) - 1);

    do
    {
        SockFd = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (SockFd < 0)
        {
            PRINT_LOG(ERROR, "Failed to create socket, errno = %d", errno);
            RetVal = GMI_FAIL;
            break;
        }

        if (bind(SockFd, (struct sockaddr *)&AddrLocal, sizeof(AddrLocal)) < 0)
        {
            PRINT_LOG(ERROR, "Failed to bind socket, errno = %d", errno);
            RetVal = GMI_FAIL;
            break;
        }

        // Add flag 'MSG_DONTWAIT', make sure sendto will not be blocked
        if (sendto(SockFd, SendBuf, SendBufSize, MSG_DONTWAIT, (struct sockaddr *)&AddrRemote, sizeof(AddrRemote)) < 0)
        {
            PRINT_LOG(ERROR, "Failed to send buf, errno = %d", errno);
            RetVal = GMI_FAIL;
            break;
        }

        FD_ZERO(&Fds);
        FD_SET(SockFd, &Fds);

        RetSelect = select(SockFd + 1, &Fds, NULL, NULL, &TimedOut);
        if (RetSelect < 0)
        {
            PRINT_LOG(ERROR, "Failed to select socket, errno = %d", errno);
            RetVal = GMI_FAIL;
            break;
        }
        else if (RetSelect == 0)
        {
            PRINT_LOG(WARNING, "Select timed out");
            RetVal = GMI_WAIT_TIMEOUT;
            break;
        }

        // Add flag 'MSG_DONTWAIT', make sure sendto will not be blocked
        *RecvBufSize = recvfrom(SockFd, RecvBuf, *RecvBufSize, MSG_DONTWAIT, NULL, NULL);
        if (*RecvBufSize < 0)
        {
            PRINT_LOG(ERROR, "Failed to recv buf, errno = %d", errno);
            RetVal = GMI_FAIL;
            break;
        }

    } while (0);

    if (SockFd >= 0)
    {
        close(SockFd);
        unlink(SockFile);
    }

    return RetVal;
}

GMI_RESULT RtspOpenStream(uint32_t StreamId, uint32_t SubStreamNum, const SubStreamInfo * Infos)
{
    GMI_RESULT           RetVal    = GMI_SUCCESS;
    uint32_t             i         = 0;
    RequestOpenStream  * Req       = (RequestOpenStream *)l_Buffer;
    ResponseOpenStream * Res       = (ResponseOpenStream *)l_Buffer;
    uint32_t             SendBytes = 0;
    uint32_t             RecvBytes = sizeof(l_Buffer);

    if (StreamId == 0 || SubStreamNum == 0 || Infos == NULL)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    Req->s_Request.s_Operation = eOpenStream;
    Req->s_Request.s_StreamId = StreamId;
    Req->s_SubStreamNum = SubStreamNum;
    SendBytes = sizeof(RequestOpenStream);
    for (i = 0; i < SubStreamNum; ++ i)
    {
        SendBytes += sizeof(SubStreamInfo);
        if (SendBytes > sizeof(l_Buffer))
        {
            PRINT_LOG(ERROR, "To large for the request packet");
            return GMI_NOT_ENOUGH_SPACE;
        }
        memcpy(&Req->s_SubStreamInfo[i], &Infos[i], sizeof(SubStreamInfo));
    }

    RetVal = SendAndRecv(l_Buffer, SendBytes, l_Buffer, &RecvBytes, TIME_OUT_USEC);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to send or recv the packet");
        return RetVal;
    }

    if (RecvBytes < sizeof(ResponseOpenStream))
    {
        PRINT_LOG(ERROR, "To small for the response packet");
        return GMI_NOT_ENOUGH_SPACE;
    }

    if (Res->s_Response.s_Operation != eOpenStream || Res->s_Response.s_StreamId != StreamId)
    {
        PRINT_LOG(ERROR, "Response packet maybe damaged");
        return GMI_FAIL;
    }

    return Res->s_Response.s_RetVal;
}

GMI_RESULT RtspCloseStream(uint32_t StreamId)
{
    GMI_RESULT            RetVal    = GMI_SUCCESS;
    RequestCloseStream  * Req       = (RequestCloseStream *)l_Buffer;
    ResponseCloseStream * Res       = (ResponseCloseStream *)l_Buffer;
    uint32_t              SendBytes = 0;
    uint32_t              RecvBytes = sizeof(l_Buffer);

    if (StreamId == 0)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    Req->s_Request.s_Operation = eCloseStream;
    Req->s_Request.s_StreamId = StreamId;
    SendBytes = sizeof(RequestCloseStream);

    RetVal = SendAndRecv(l_Buffer, SendBytes, l_Buffer, &RecvBytes, TIME_OUT_USEC);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to send or recv the packet");
        return RetVal;
    }

    if (RecvBytes < sizeof(ResponseCloseStream))
    {
        PRINT_LOG(ERROR, "To small for the response packet");
        return GMI_NOT_ENOUGH_SPACE;
    }

    if (Res->s_Response.s_Operation != eCloseStream || Res->s_Response.s_StreamId != StreamId)
    {
        PRINT_LOG(ERROR, "Response packet maybe damaged");
        return GMI_FAIL;
    }

    return Res->s_Response.s_RetVal;
}

GMI_RESULT RtspQueryStreamStatus(uint32_t StreamId, uint32_t * StreamStatus)
{
    GMI_RESULT            RetVal    = GMI_SUCCESS;
    RequestQueryStream  * Req       = (RequestQueryStream *)l_Buffer;
    ResponseQueryStream * Res       = (ResponseQueryStream *)l_Buffer;
    uint32_t              SendBytes = 0;
    uint32_t              RecvBytes = sizeof(l_Buffer);

    if (StreamId == 0 || StreamStatus == NULL)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    *StreamStatus = eStreamUnknown;

    Req->s_Request.s_Operation = eQueryStream;
    Req->s_Request.s_StreamId = StreamId;
    SendBytes = sizeof(RequestQueryStream);

    RetVal = SendAndRecv(l_Buffer, SendBytes, l_Buffer, &RecvBytes, TIME_OUT_USEC);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to send or recv the packet");
        return RetVal;
    }

    if (RecvBytes < sizeof(ResponseQueryStream))
    {
        PRINT_LOG(ERROR, "To small for the response packet");
        return GMI_NOT_ENOUGH_SPACE;
    }

    if (Res->s_Response.s_Operation != eQueryStream || Res->s_Response.s_StreamId != StreamId)
    {
        PRINT_LOG(ERROR, "Response packet maybe damaged");
        return GMI_FAIL;
    }

    if (Res->s_Response.s_RetVal == GMI_SUCCESS)
    {
        *StreamStatus = Res->s_Status;
    }

    return Res->s_Response.s_RetVal;
}

