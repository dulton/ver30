#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "stream_ctrl.h"

#define RTSP_CTRL_SOCK_FILE "/tmp/rtsp_ctrl_sock"
#define MAX_PACKET_SIZE     1024

// Define control command
enum
{
    eOpenStream = 0,
    eCloseStream,
    eQueryStream,
};

// Define stream status
enum
{
    eStreamUnknown = 0,
    eStreamOpened,
    eStreamClosed,
};

// Define request packet head
typedef struct tagRequest
{
    uint32_t s_Operation;
    uint32_t s_StreamId;
} Request;

// Define request packet for open stream operation
typedef struct tagRequestOpenStream
{
    Request       s_Request;
    uint32_t      s_SubStreamNum;
    SubStreamInfo s_SubStreamInfo[0];
} RequestOpenStream;

// Define request packet for close stream operation
typedef struct tagRequestCloseStream
{
    Request s_Request;
} RequestCloseStream;

// Define request packet for query stream status operation
typedef struct tagRequestQueryStream
{
    Request s_Request;
} RequestQueryStream;

// Define response packet head
typedef struct tagResponse
{
    uint32_t   s_Operation;
    uint32_t   s_StreamId;
    GMI_RESULT s_RetVal;
} Response;

// Define response packet for open stream operation
typedef struct tagResponseOpenStream
{
    Response s_Response;
} ResponseOpenStream;

// Define response packet for close stream operation
typedef struct tagResponseCloseStream
{
    Response s_Response;
} ResponseCloseStream;

// Define response packet for query stream status operation
typedef struct tagResponseQueryStream
{
    Response s_Response;
    uint32_t s_Status;
} ResponseQueryStream;



StreamCtrl::StreamCtrl()
    : m_RtspService(NULL)
    , m_SockFd(-1)
    , m_Initialized(false)
{
}

StreamCtrl::~StreamCtrl()
{
    if (Initialized())
    {
        PRINT_LOG(WARNING, "StreamCtrl is still initialized");
        Uninitialize();
    }
}

GMI_RESULT StreamCtrl::Initialize(RtspService * Service)
{
    if (NULL == Service)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    if (Initialized())
    {
        PRINT_LOG(WARNING, "Already initialized");
        return GMI_ALREADY_OPERATED;
    }

    GMI_RESULT         RetVal = GMI_SUCCESS;
    struct sockaddr_un Addr;

    memset(&Addr, 0x00, sizeof(Addr));

    Addr.sun_family = AF_UNIX;
    strncpy(Addr.sun_path, RTSP_CTRL_SOCK_FILE, sizeof(Addr.sun_path) - 1);

    do
    {
        m_SockFd = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (m_SockFd < 0)
        {
            PRINT_LOG(ERROR, "Failed to create socket, errno = %d", errno);
            RetVal = GMI_FAIL;
            break;
        }

        // Make sure RTSP_CTRL_SOCK_FILE does not exist.
        unlink(RTSP_CTRL_SOCK_FILE);

        if (bind(m_SockFd, (struct sockaddr *)&Addr, sizeof(Addr)) < 0)
        {
            PRINT_LOG(ERROR, "Failed to bind socket, errno = %d", errno);
            RetVal = GMI_FAIL;
            break;
        }

        Service->AddFdListener(m_SockFd, StreamCtrl::SocketHandlerProc, this);
        m_RtspService = Service;
        m_Initialized = true;

        return GMI_SUCCESS;
    } while (0);

    if (m_SockFd >= 0)
    {
        close(m_SockFd);
        m_SockFd = -1;
    }

    return RetVal;
}

GMI_RESULT StreamCtrl::Uninitialize()
{
    if (!Initialized())
    {
        PRINT_LOG(WARNING, "Not initialized yet");
        return GMI_ALREADY_OPERATED;
    }

    m_RtspService->RemoveFdListener(m_SockFd);
    m_RtspService = NULL;
    close(m_SockFd);
    m_SockFd = -1;
    m_Initialized = false;

    unlink(RTSP_CTRL_SOCK_FILE);

    return GMI_SUCCESS;
}

void_t StreamCtrl::SocketHandlerProc(void_t * Data, int32_t Mask)
{
    ASSERT(Data != NULL, "Data MUST NOT be non-pointer");
    StreamCtrl * This = (StreamCtrl *)Data;
    This->SocketHandler();
}

void_t StreamCtrl::SocketHandler()
{
    uint8_t              Buf[MAX_PACKET_SIZE];
    struct sockaddr_un   Addr;
    socklen_t            AddrLen = sizeof(Addr);
    int32_t              Ret     = 0;
    int32_t              BufLen  = 0;
    GMI_RESULT           RetVal  = GMI_SUCCESS;

    Ret = recvfrom(m_SockFd, Buf, sizeof(Buf), MSG_DONTWAIT, (struct sockaddr *)&Addr, &AddrLen);
    if (Ret < 0)
    {
        PRINT_LOG(WARNING, "Failed to read socket, errno = %d", errno);
        return;
    }

    if (Ret < (int32_t)sizeof(Request))
    {
        PRINT_LOG(WARNING, "Packet is not large enough");
        return;
    }

    Request * Req = (Request *)Buf;
    switch (Req->s_Operation)
    {
        case eOpenStream:
        {
            if (Ret < (int32_t)sizeof(RequestOpenStream))
            {
                PRINT_LOG(WARNING, "Packet is not large enough");
                return;
            }

            RequestOpenStream  * ReqOS = (RequestOpenStream *)Buf;

            if (ReqOS->s_SubStreamNum == 0)
            {
                PRINT_LOG(WARNING, "MUST be at least 1 sub stream");
                return;
            }

            if (Ret < (int32_t)(sizeof(RequestOpenStream) + (ReqOS->s_SubStreamNum * sizeof(SubStreamInfo))))
            {
                PRINT_LOG(WARNING, "Packet is not large enough");
                return;
            }

            ResponseOpenStream * ResOS = (ResponseOpenStream *)Buf;
            RetVal = m_RtspService->StartStream(Req->s_StreamId, ReqOS->s_SubStreamNum, ReqOS->s_SubStreamInfo);
            PRINT_LOG(INFO, "Open stream %u %s", Req->s_StreamId, RetVal == GMI_SUCCESS ? "Succeed" : "Failed");
            ResOS->s_Response.s_RetVal = RetVal;
            BufLen = sizeof(ResponseOpenStream);
            break;
        }

        case eCloseStream:
        {
            // RequestCloseStream  * ReqCS = (RequestCloseStream *)Buf;
            ResponseCloseStream * ResCS = (ResponseCloseStream *)Buf;
            RetVal = m_RtspService->StopStream(Req->s_StreamId);
            PRINT_LOG(INFO, "Close stream %u %s", Req->s_StreamId, RetVal == GMI_SUCCESS ? "Succeed" : "Failed");
            ResCS->s_Response.s_RetVal = RetVal;
            BufLen = sizeof(ResponseCloseStream);
            break;
        }

        case eQueryStream:
        {
            // RequestQueryStream  * ReqQS = (RequestQueryStream *)Buf;
            ResponseQueryStream * ResQS = (ResponseQueryStream *)Buf;
            boolean_t Started = m_RtspService->IsStreamStarted(Req->s_StreamId);
            PRINT_LOG(INFO, "Stream %u %s", Req->s_StreamId, Started ? "Opened" : "Closed");
            ResQS->s_Response.s_RetVal = GMI_SUCCESS;
            ResQS->s_Status = Started ? eStreamOpened : eStreamClosed;
            BufLen = sizeof(ResponseQueryStream);
            break;
        }

        default:
            PRINT_LOG(WARNING, "Unknown operation");
            return;
    }

    Ret = sendto(m_SockFd, Buf, BufLen, MSG_DONTWAIT, (struct sockaddr *)&Addr, AddrLen);
    if (Ret < 0)
    {
        PRINT_LOG(WARNING, "Failed to write socket, errno = %d", errno);
        return;
    }
}
