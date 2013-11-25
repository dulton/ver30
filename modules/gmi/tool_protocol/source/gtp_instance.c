#include <stdlib.h>
#include <string.h>

#include "gtp_instance.h"
#include "gtp_transaction.h"

#include "debug.h"

#ifdef _WIN32

#else
#include <unistd.h> 
#endif

static uint32_t GMI_GetCurrentProcessId()
{
#ifdef _WIN32
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}

// Broadcast Mac Address
static const uint8_t l_BcMacAddr[MAC_ADDRESS_LENGTH] =
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#if 0
static const char_t * MacAddr2String(const uint8_t MacAddr[MAC_ADDRESS_LENGTH])
{
    static char_t StringBuf[20];
    snprintf(StringBuf, sizeof(StringBuf), "%02x:%02x:%02x:%02x:%02x:%02x",
        MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]);
    return StringBuf;
}
#endif

static void_t __GtpDestroyHandle(void_t * Handle)
{
    GtpInstance * GtpIns = (GtpInstance *)Handle;
    GtpListNode * Node   = NULL;

    ASSERT(Handle != GTP_INVALID_HANDLE, "Handle MUST be invalid");

    GTP_LIST_FOR_EACH(Node, &GtpIns->s_TransList)
    {
        GtpTransaction * GtpTrans = GTP_LIST_ENTRY(Node, GtpTransaction);
        Node = GTP_LIST_REMOVE(&GtpIns->s_TransList, GtpTrans);
        GtpDestroyTransHandle((GtpTransHandle)GtpTrans);
    }

    GTP_LIST_CLEAR(&GtpIns->s_TransList);
    GtpScheduleListDestroy(&GtpIns->s_ScheduleTaskList);

    free(GtpIns);
}

GtpHandle GtpCreateHandle(PcapSessionHandle Handle, boolean_t IsServer)
{
    GMI_RESULT    RetVal = GMI_SUCCESS;
    GtpInstance * GtpIns = NULL;

    if (PCAP_SESSION_INVALID_HANDLE == Handle)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GTP_INVALID_HANDLE;
    }

    do
    {
        GtpIns = (GtpInstance *)malloc(sizeof(GtpInstance));
        if (NULL == GtpIns)
        {
            PRINT_LOG(ERROR, "Not enough memory");
            RetVal = GMI_OUT_OF_MEMORY;
            break;
        }

        memset(GtpIns, 0x00, sizeof(GtpInstance));
        GtpReferenceInit(GtpIns, __GtpDestroyHandle);
        GtpIns->s_PcapHandle = Handle;
        GtpIns->s_IsServer = IsServer;
        GtpIns->s_SessionId = GMI_GetCurrentProcessId();
        GtpIns->s_TransIdCount = GTP_MIN_TRANS_ID;
        GTP_LIST_INIT(&GtpIns->s_TransList);
        GtpScheduleListInit(&GtpIns->s_ScheduleTaskList);
    } while (0);

    if (RetVal != GMI_SUCCESS)
    {
        if (GtpIns != NULL)
        {
            GtpDestroyHandle(GtpIns);
        }

        return GTP_INVALID_HANDLE;
    }

    return (GtpHandle)GtpIns;
}

GMI_RESULT GtpDestroyHandle(GtpHandle Handle)
{
    GtpInstance * GtpIns = (GtpInstance *)Handle;

    if (GTP_INVALID_HANDLE == Handle)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    GtpRelease(GtpIns);

    return GMI_SUCCESS;
}

void_t GtpSetSentCallback(GtpHandle Handle, CallbackSent CbFunc, void_t * Data)
{
    GtpInstance * GtpIns = (GtpInstance *)Handle;

    if (GTP_INVALID_HANDLE == Handle)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return;
    }

    GtpIns->s_CbSent = CbFunc;
    GtpIns->s_CbSentData = Data;
}

void_t GtpSetRecvCallback(GtpHandle Handle, CallbackRecv CbFunc, void_t * Data)
{
    GtpInstance * GtpIns = (GtpInstance *)Handle;

    if (GTP_INVALID_HANDLE == Handle)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return;
    }

    GtpIns->s_CbRecv = CbFunc;
    GtpIns->s_CbRecvData = Data;
}

void_t GtpSetTransKilledCallback(GtpHandle Handle, CallbackTransKilled CbFunc,
    void_t * Data)
{
    GtpInstance * GtpIns = (GtpInstance *)Handle;

    if (GTP_INVALID_HANDLE == Handle)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return;
    }

    GtpIns->s_CbTransKilled = CbFunc;
    GtpIns->s_CbTransKilledData = Data;
}

static GMI_RESULT GtpRecvPacket(GtpInstance * GtpIns)
{
    GMI_RESULT    RetVal        = GMI_SUCCESS;
    uint8_t     * RecvBuf       = NULL;
    uint32_t      RecvBufLength = 0;
    boolean_t     IsBroadcast   = false;
    uint8_t       MacAddr[MAC_ADDRESS_LENGTH];

    ASSERT(GtpIns != NULL, "GtpIns MUST NOT be non-pointer");

    RecvBuf = GtpIns->s_RecvBuf;
    RecvBufLength = sizeof(GtpIns->s_RecvBuf);

    RetVal = PcapSessionRecvFrom(GtpIns->s_PcapHandle, RecvBuf, &RecvBufLength, MacAddr,
        &IsBroadcast);
    if (RetVal == GMI_SUCCESS)
    {
        GtpTransaction * GtpTrans = NULL;
        GtpPacket      * Packet   = NULL;
        GtpListNode    * Node     = NULL;

        // PRINT_LOG(VERBOSE, "Receive %u bytes data, from %s", RecvBufLength,
        //     MacAddr2String(MacAddr));

        do
        {
            Packet = GtpPacketParse(RecvBuf, RecvBufLength);
            if (NULL == Packet)
            {
                PRINT_LOG(WARNING, "Failed to parse GTP packet");
                break;
            }

            if (!GtpIns->s_IsServer && !IsBroadcast &&
                GtpIns->s_SessionId != GTP_PACKET_GET_SESSION_ID(Packet))
            {
                PRINT_LOG(WARNING, "GTP packet is not for this client");
                break;
            }

            // Find transaction
            GTP_LIST_FOR_EACH(Node, &GtpIns->s_TransList)
            {
                GtpTrans = GTP_LIST_ENTRY(Node, GtpTransaction);
                if (GtpTrans->s_TransId == GTP_PACKET_GET_TRANS_ID(Packet) &&
                    GtpTrans->s_SessionId == GTP_PACKET_GET_SESSION_ID(Packet) &&
                    0 == memcmp(GtpTrans->s_MacAddr, MacAddr, MAC_ADDRESS_LENGTH))
                {
                    // Find a matched transaction
                    break;
                }

                GtpTrans = NULL;
            }

            if (NULL == GtpTrans)
            {
                if (GTP_PACKET_IS_ACK(Packet))
                {
                    // PRINT_LOG(WARNING, "Drop ack packet if not found transaction");
                    break;
                }

                // Create new transaction
                GtpTrans = GtpCreateTransHandleInternal(GtpIns,
                    GTP_PACKET_GET_TRANS_ID(Packet), GTP_PACKET_GET_SESSION_ID(Packet),
                    MacAddr, false);
                if (NULL == GtpTrans)
                {
                    PRINT_LOG(ERROR, "Failed to create new transaction");
                    return GMI_FAIL;
                }
            }

            // Feed packet to transaction
            GtpTransFeedPacket(GtpTrans, Packet, IsBroadcast);
        } while (0);
    }
    else if (RetVal == GMI_WAIT_TIMEOUT)
    {
        // PRINT_LOG(VERBOSE, "Receive timed out");
    }
    else
    {
        PRINT_LOG(ERROR, "Failed to receive data packet");
        return RetVal;
    }

    return GMI_SUCCESS;
}

GMI_RESULT GtpAddDelayTask(GtpHandle Handle, CbOnSchedule CbFunc, void_t * Data,
    uint32_t Sec, uint32_t USec)
{
    GtpInstance * GtpIns   = (GtpInstance *)Handle;
    GtpList     * TaskList = NULL;

    if (GTP_INVALID_HANDLE == Handle || NULL == CbFunc)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    TaskList = &GtpIns->s_ScheduleTaskList;
    return GtpScheduleAddDelayTask(TaskList, CbFunc, Data, Sec, USec);
}

GMI_RESULT GtpUpdateDelayTask(GtpHandle Handle, CbOnSchedule CbFunc, void_t * Data,
    uint32_t Sec, uint32_t USec)
{
    GtpInstance * GtpIns   = (GtpInstance *)Handle;
    GtpList     * TaskList = NULL;

    if (GTP_INVALID_HANDLE == Handle || NULL == CbFunc)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    TaskList = &GtpIns->s_ScheduleTaskList;
    return GtpScheduleUpdateDelayTask(TaskList, CbFunc, Data, Sec, USec);
}

GMI_RESULT GtpCancelDelayTask(GtpHandle Handle, CbOnSchedule CbFunc, void_t * Data)
{
    GtpInstance * GtpIns   = (GtpInstance *)Handle;
    GtpList     * TaskList = NULL;

    if (GTP_INVALID_HANDLE == Handle || NULL == CbFunc)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    TaskList = &GtpIns->s_ScheduleTaskList;
    return GtpScheduleCancelDelayTask(TaskList, CbFunc, Data);
}

GMI_RESULT GtpDoEvent(GtpHandle Handle)
{
    GMI_RESULT    RetVal = GMI_SUCCESS;
    GtpInstance * GtpIns = (GtpInstance *)Handle;

    if (GTP_INVALID_HANDLE == Handle)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    // Add reference
    GtpAddRef(GtpIns);

    do
    {
        // Try to receive packet
        RetVal = GtpRecvPacket(GtpIns);
        if (RetVal != GMI_SUCCESS)
        {
            break;
        }

        // Schedule task if time up
        RetVal = GtpScheduleTask(&GtpIns->s_ScheduleTaskList);
        if (RetVal != GMI_SUCCESS)
        {
            break;
        }
    } while (0);

    // Release reference
    GtpRelease(GtpIns);

    return RetVal;
}

