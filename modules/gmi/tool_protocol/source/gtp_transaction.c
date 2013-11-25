#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include "gtp_transaction.h"
#include "gtp_packet.h"
#include "gtp_schedule.h"

#include "debug.h"

static const uint8_t l_BcMacAddr[MAC_ADDRESS_LENGTH] =
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static void_t GtpReSendFirstPacket(void_t * Data);

static void_t GtpResumeSentCallback(GtpTransaction * GtpTrans, GMI_RESULT Errno)
{
    GtpInstance * GtpIns = NULL;
    ASSERT(GtpTrans != NULL, "GtpTrans MUST NOT be non-pointer");

    GtpIns = GtpTrans->s_GtpInstance;
    ASSERT(GtpIns != NULL, "GtpIns MUST NOT be non-pointer");

    if (GtpIns->s_CbSent)
    {
        GtpIns->s_CbSent(GtpIns->s_CbSentData, (GtpTransHandle)GtpTrans, Errno);
    }
}

static void_t GtpResumeRecvCallback(GtpTransaction * GtpTrans, const uint8_t * Buf,
    uint32_t BufLength, uint8_t Type)
{
    GtpInstance * GtpIns = NULL;
    ASSERT(GtpTrans != NULL, "GtpTrans MUST NOT be non-pointer");

    GtpIns = GtpTrans->s_GtpInstance;
    ASSERT(GtpIns != NULL, "GtpIns MUST NOT be non-pointer");

    if (GtpIns->s_CbRecv)
    {
        GtpIns->s_CbRecv(GtpIns->s_CbRecvData, (GtpTransHandle)GtpTrans, Buf, BufLength,
            Type);
    }
}

static void_t GtpResumeTransKilledCallback(GtpTransaction * GtpTrans)
{
    GtpInstance * GtpIns = NULL;
    ASSERT(GtpTrans != NULL, "GtpTrans MUST NOT be non-pointer");

    GtpIns = GtpTrans->s_GtpInstance;
    ASSERT(GtpIns != NULL, "GtpIns MUST NOT be non-pointer");

    if (GtpIns->s_CbTransKilled)
    {
        GtpIns->s_CbTransKilled(GtpIns->s_CbTransKilledData, (GtpTransHandle)GtpTrans);
    }
}

static void_t GtpDestroyTransHandleInternal(GtpTransaction * GtpTrans)
{
    GtpInstance * GtpIns = NULL;
    GtpListNode * Node   = NULL;

    ASSERT(GtpTrans != NULL, "GtpTrans MUST NOT be non-pointer");

    GtpIns = GtpTrans->s_GtpInstance;
    ASSERT(GtpIns != NULL, "GtpIns MUST NOT be non-pointer");

    GTP_LIST_FOR_EACH(Node, &GtpTrans->s_SendingPacketList)
    {
        GtpSendingPacket * SendingPacket = GTP_LIST_ENTRY(Node, GtpSendingPacket);
        Node = GTP_LIST_REMOVE(&GtpTrans->s_SendingPacketList, SendingPacket);
        free(SendingPacket);
    }

    // Cancel retry task
    GtpScheduleCancelDelayTask(&GtpIns->s_ScheduleTaskList, GtpReSendFirstPacket,
        GtpTrans);

    GtpBufferDestroy(&GtpTrans->s_ReceivingBuffer);

    // Remove from transaction list, ignore return value
    GTP_LIST_REMOVE(&GtpIns->s_TransList, GtpTrans);
    free(GtpTrans);
}

static void_t __GtpDestroyTransHandle(void_t * Handle)
{
    GtpTransaction * GtpTrans = (GtpTransaction *)Handle;
    GtpInstance    * GtpIns   = NULL;

    ASSERT(Handle != GTP_INVALID_HANDLE, "Handle MUST be invalid");

    GtpIns = GtpTrans->s_GtpInstance;
    ASSERT(GtpIns != NULL, "GtpIns MUST NOT be non-pointer");

    GtpResumeTransKilledCallback(GtpTrans);

    GtpDestroyTransHandleInternal(GtpTrans);
}

GtpTransaction * GtpCreateTransHandleInternal(GtpInstance * GtpIns, uint16_t TransId,
    uint32_t SessionId, const uint8_t MacAddr[MAC_ADDRESS_LENGTH], boolean_t IsBroadcast)
{
    GMI_RESULT       RetVal   = GMI_SUCCESS;
    GtpTransaction * GtpTrans = NULL;

    ASSERT(GtpIns != NULL, "GtpIns MUST NOT be non-pointer");

    do
    {
        GtpTrans = (GtpTransaction *)malloc(sizeof(GtpTransaction));
        if (NULL == GtpTrans)
        {
            PRINT_LOG(ERROR, "Not enough memory");
            RetVal = GMI_OUT_OF_MEMORY;
            break;
        }

        memset(GtpTrans, 0x00, sizeof(GtpTransaction));
        GtpReferenceInit(GtpTrans, __GtpDestroyTransHandle);
        GtpTrans->s_GtpInstance = GtpIns;
        GtpTrans->s_IsBroadcast = IsBroadcast;
        GtpTrans->s_IsFirstTimeSend = true;
        memcpy(GtpTrans->s_MacAddr, MacAddr, MAC_ADDRESS_LENGTH);
        GtpTrans->s_TransId = TransId;
        GtpTrans->s_SessionId = SessionId;
        GtpTrans->s_SeqNumCount = GTP_MIN_SEQ_NUM;
        GtpTrans->s_LastSendSeqNum = GTP_INVALID_SEQ_NUM;
        GtpTrans->s_LastRecvSeqNum = GTP_INVALID_SEQ_NUM;

        GtpBufferInit(&GtpTrans->s_ReceivingBuffer);

        GTP_LIST_INIT(&GtpTrans->s_SendingPacketList);

        GTP_LIST_ADD(&GtpIns->s_TransList, GtpTrans);
    } while (0);

    if (RetVal != GMI_SUCCESS)
    {
        if (GtpTrans != NULL)
        {
            GtpDestroyTransHandleInternal(GtpTrans);
        }

        return NULL;
    }

    return GtpTrans;
}

GtpTransHandle GtpCreateTransHandle(GtpHandle Handle,
    const uint8_t MacAddr[MAC_ADDRESS_LENGTH])
{
    GtpInstance * GtpIns  = (GtpInstance *)Handle;
    uint16_t      TransId = GTP_MIN_TRANS_ID;

    if (GTP_INVALID_HANDLE == Handle)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GTP_INVALID_HANDLE;
    }

    if (0 != memcmp(MacAddr, l_BcMacAddr, MAC_ADDRESS_LENGTH))
    {
        if (GtpIns->s_IsServer)
        {
            PRINT_LOG(WARNING, "Server instance can't create non-broadcast transaction");
            return GTP_INVALID_HANDLE;
        }

        TransId = GtpIns->s_TransIdCount;
        if (GtpIns->s_TransIdCount == GTP_MAX_TRANS_ID)
        {
            GtpIns->s_TransIdCount = GTP_MIN_TRANS_ID;
        }
        else
        {
            GtpIns->s_TransIdCount ++;
        }
    }
    else
    {
        return GtpCreateBroadcastTransHandle(Handle);
    }

    return (GtpTransHandle)GtpCreateTransHandleInternal(GtpIns, TransId,
        GtpIns->s_SessionId, MacAddr, false);
}

GtpTransHandle GtpCreateBroadcastTransHandle(GtpHandle Handle)
{
    GtpInstance * GtpIns = (GtpInstance *)Handle;

    if (GTP_INVALID_HANDLE == Handle)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GTP_INVALID_HANDLE;
    }

    return (GtpTransHandle)GtpCreateTransHandleInternal(GtpIns, GTP_BROADCAST_TRANS_ID,
        GtpIns->s_SessionId, l_BcMacAddr, true);
}

GMI_RESULT GtpDestroyTransHandle(GtpTransHandle Handle)
{
    GtpTransaction * GtpTrans = (GtpTransaction *)Handle;

    if (GTP_INVALID_HANDLE == Handle)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    GtpRelease(GtpTrans);

    return GMI_SUCCESS;
}

GtpHandle GtpTransGetGtpHandle(GtpTransHandle Handle)
{
    GtpTransaction * GtpTrans = (GtpTransaction *)Handle;
    GtpInstance    * GtpIns   = NULL;

    if (GTP_INVALID_HANDLE == Handle)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GTP_INVALID_HANDLE;
    }

    GtpIns = GtpTrans->s_GtpInstance;
    ASSERT(GtpIns != NULL, "GtpIns MUST NOT be non-pointer");

    return (GtpHandle)GtpIns;
}

GMI_RESULT GtpTransGetMacAddress(GtpTransHandle Handle,
    uint8_t MacAddr[MAC_ADDRESS_LENGTH])
{
    GtpTransaction * GtpTrans = (GtpTransaction *)Handle;

    if (GTP_INVALID_HANDLE == Handle)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    memcpy(MacAddr, GtpTrans->s_MacAddr, MAC_ADDRESS_LENGTH);

    return GMI_SUCCESS;
}

static GMI_RESULT GtpSendFirstPacket(GtpTransaction * GtpTrans, boolean_t IsRetry)
{
    GMI_RESULT         RetVal     = GMI_SUCCESS;
    GtpInstance      * GtpIns     = NULL;
    GtpSendingPacket * SendPacket = NULL;

    ASSERT(GtpTrans != NULL, "GtpTrans MUST NOT be non-pointer");

    GtpIns = GtpTrans->s_GtpInstance;
    ASSERT(GtpIns != NULL, "GtpIns MUST NOT be non-pointer");

    ASSERT(!GTP_LIST_IS_EMPTY(&GtpTrans->s_SendingPacketList),
        "Sending list MUST NOT be empty");

    // Get first packet
    SendPacket = GTP_LIST_ENTRY(GTP_LIST_FIRST(&GtpTrans->s_SendingPacketList),
        GtpSendingPacket);

    if (!IsRetry)
    {
        // Fix up packet head
        GtpPacketFixUpHead(SendPacket->s_PacketBuf, GtpTrans->s_SeqNumCount);

        // Clear retry times
        GtpTrans->s_SendRetryTimes = 0;
    }

    if (!GtpTrans->s_IsBroadcast)
    {
        // Add retry task if not broadcast
        RetVal = GtpScheduleAddDelayTask(&GtpIns->s_ScheduleTaskList,
            GtpReSendFirstPacket, GtpTrans, 0, 200000);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to add schedule task");
            return RetVal;
        }
    }

    // Send packet
    RetVal = PcapSessionSendTo(GtpIns->s_PcapHandle, SendPacket->s_PacketBuf,
        SendPacket->s_PacketLength, GtpTrans->s_MacAddr);

    if (RetVal == GMI_SUCCESS && !IsRetry)
    {
        // Record last send sequence number
        GtpTrans->s_LastSendSeqNum = GtpTrans->s_SeqNumCount;

        // Increase sequence number, if send succeed
        if (GtpTrans->s_SeqNumCount == GTP_MAX_SEQ_NUM)
        {
            GtpTrans->s_SeqNumCount = GTP_MAX_SEQ_NUM;
        }
        else
        {
            GtpTrans->s_SeqNumCount ++;
        }
    }

    return RetVal;
}

static void_t GtpReSendFirstPacket(void_t * Data)
{
    GtpTransaction * GtpTrans = (GtpTransaction *)Data;
    GtpListNode    * Node     = NULL;
    GMI_RESULT       RetVal   = GMI_SUCCESS;

    ASSERT(Data != NULL, "Data MUST NOT be non-pointer");

    GtpAddRef(GtpTrans);

    // Increase send retry times
    GtpTrans->s_SendRetryTimes ++;

    if (GtpTrans->s_SendRetryTimes < 5)
    {
        RetVal = GtpSendFirstPacket(GtpTrans, true);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to resend packet");
        }
    }
    else
    {
        PRINT_LOG(ERROR, "Send packet timed out");
        RetVal = GMI_WAIT_TIMEOUT;
    }

    if (RetVal != GMI_SUCCESS)
    {
        GTP_LIST_FOR_EACH(Node, &GtpTrans->s_SendingPacketList)
        {
            GtpSendingPacket * SendingPacket = GTP_LIST_ENTRY(Node, GtpSendingPacket);
            Node = GTP_LIST_REMOVE(&GtpTrans->s_SendingPacketList, SendingPacket);
            free(SendingPacket);
        }

        GtpResumeSentCallback(GtpTrans, RetVal);
    }

    GtpRelease(GtpTrans);
}

GMI_RESULT GtpTransSendData(GtpTransHandle Handle, const uint8_t * Buffer,
    uint32_t BufferLength, uint8_t Type)
{
    GMI_RESULT       RetVal      = GMI_SUCCESS;
    GtpTransaction * GtpTrans    = (GtpTransaction *)Handle;
    GtpInstance    * GtpIns      = NULL;
    boolean_t        NeedSend    = false;
    GtpPacketFlags   PacketFlags;

    if (GTP_INVALID_HANDLE == Handle || NULL == Buffer || 0 == BufferLength)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    GtpIns = GtpTrans->s_GtpInstance;
    ASSERT(GtpIns != NULL, "GtpIns MUST NOT be non-pointer");

    // If sending list is empty, we should trigger send task
    if (GTP_LIST_IS_EMPTY(&GtpTrans->s_SendingPacketList))
    {
        NeedSend = true;
    }

    GtpAddRef(GtpTrans);

    if (GtpTrans->s_IsBroadcast)
    {
        // Check data buffer length
        if (BufferLength > GTP_MAX_PACKET_LENGTH)
        {
            GtpRelease(GtpTrans);

            PRINT_LOG(WARNING, "In broadcast mode, not permit to send more than %u"
                " bytes data", GTP_MAX_PACKET_LENGTH);
            return GMI_FAIL;
        }
    }
    else if (GtpTrans->s_IsFirstTimeSend)
    {
        // Insert an start packet at the first without any data
        GtpSendingPacket * SendingPacket =
            (GtpSendingPacket *)malloc(sizeof(GtpSendingPacket));
        if (NULL == SendingPacket)
        {
            GtpRelease(GtpTrans);

            PRINT_LOG(ERROR, "Not enough memory");
            return GMI_OUT_OF_MEMORY;
        }

        memset(SendingPacket, 0x00, sizeof(GtpSendingPacket));

        // Pre-pack the packet data field
        GtpPacketPrePack(SendingPacket->s_PacketBuf, GtpTrans->s_TransId,
            GtpTrans->s_SessionId, NULL, 0, 0);

        // Set packet length
        SendingPacket->s_PacketLength = GTP_HEAD_LENGTH;

        // Add the pre-packed packet
        GTP_LIST_ADD(&GtpTrans->s_SendingPacketList, SendingPacket);

        // Clear the first time send flag
        GtpTrans->s_IsFirstTimeSend = false;
    }

    // Initialize flags, add begin flag
    PacketFlags.u_Byte = 0;
    GTP_FLAGS_SET_DATA_TYPE(PacketFlags, Type);
    GTP_FLAGS_SET_BEG(PacketFlags);

    // Split the data buffer into some packets
    for(;;)
    {
        GtpSendingPacket * SendingPacket = NULL;
        uint32_t           DataLength    = GTP_MAX_DATA_LENGTH;

        SendingPacket = (GtpSendingPacket *)malloc(sizeof(GtpSendingPacket));
        if (NULL == SendingPacket)
        {
            PRINT_LOG(ERROR, "Not enough memory");
            RetVal = GMI_OUT_OF_MEMORY;
            break;
        }

        memset(SendingPacket, 0x00, sizeof(GtpSendingPacket));

        if (BufferLength <= GTP_MAX_DATA_LENGTH)
        {
            DataLength = BufferLength;

            // Add finish flag
            GTP_FLAGS_SET_FIN(PacketFlags);
        }

        // Pre-pack the packet data field
        GtpPacketPrePack(SendingPacket->s_PacketBuf, GtpTrans->s_TransId,
            GtpTrans->s_SessionId, Buffer, DataLength, PacketFlags.u_Byte);

        // Set packet length
        SendingPacket->s_PacketLength =
            GTP_HEAD_LENGTH + GTP_CHECKSUM_LENGTH + DataLength;

        // Add the pre-packed packet
        GTP_LIST_ADD(&GtpTrans->s_SendingPacketList, SendingPacket);

        // Clear begin flag
        GTP_FLAGS_CLR_BEG(PacketFlags);

        BufferLength -= DataLength;
        Buffer += DataLength;

        // All data buffer is done
        if (BufferLength == 0)
        {
            break;
        }
    }

    if (GMI_SUCCESS == RetVal && NeedSend)
    {
        // Send the first packet in the list
        RetVal = GtpSendFirstPacket(GtpTrans, false);
    }

    if (RetVal != GMI_SUCCESS || GtpTrans->s_IsBroadcast)
    {
        // Clear the sending packet list
        GtpListNode * Node = NULL;
        GTP_LIST_FOR_EACH(Node, &GtpTrans->s_SendingPacketList)
        {
            GtpSendingPacket * SendingPacket = GTP_LIST_ENTRY(Node, GtpSendingPacket);
            Node = GTP_LIST_REMOVE(&GtpTrans->s_SendingPacketList, SendingPacket);
            free(SendingPacket);
        }
    }

    GtpRelease(GtpTrans);

    return RetVal;
}

void_t GtpTransSetReferenceInstance(GtpTransHandle Handle, void_t * Ptr)
{
    GtpTransaction * GtpTrans = (GtpTransaction *)Handle;

    if (GTP_INVALID_HANDLE == Handle)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return;
    }

    GtpTrans->s_UserInstance = Ptr;
}

void_t * GtpTransGetReferenceInstance(GtpTransHandle Handle)
{
    GtpTransaction * GtpTrans = (GtpTransaction *)Handle;

    if (GTP_INVALID_HANDLE == Handle)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return NULL;
    }

    return GtpTrans->s_UserInstance;
}

static GMI_RESULT GtpTransSendAckPacket(GtpTransaction * GtpTrans, GtpPacket * Packet)
{
    GtpInstance     * GtpIns     = NULL;
    uint8_t           Buf[GTP_HEAD_LENGTH];

    ASSERT(GtpTrans != NULL, "GtpTrans MUST NOT be non-ponter");
    ASSERT(Packet != NULL, "Packet MUST NOT be non-ponter");

    GtpIns = GtpTrans->s_GtpInstance;
    ASSERT(GtpIns != NULL, "GtpIns MUST NOT be non-pointer");

    GtpPackAckPacket(Buf, Packet);

    return PcapSessionSendTo(GtpIns->s_PcapHandle, Buf, GTP_HEAD_LENGTH,
        GtpTrans->s_MacAddr);
}

void_t GtpTransFeedPacket(GtpTransaction * GtpTrans, GtpPacket * Packet,
    boolean_t IsBroadcast)
{
    GMI_RESULT RetVal = GMI_SUCCESS;

    ASSERT(GtpTrans != NULL, "GtpTrans MUST NOT be non-ponter");
    ASSERT(Packet != NULL, "Packet MUST NOT be non-ponter");

    GtpAddRef(GtpTrans);

    do
    {
        if (GTP_PACKET_IS_ACK(Packet))
        {
            GtpSendingPacket * SendPacket = NULL;
            GtpPacket        * TempPacket = NULL;
            GtpInstance      * GtpIns     = GtpTrans->s_GtpInstance;

            ASSERT(!IsBroadcast, "MUST NOT be broadcast");
            ASSERT(GtpIns != NULL, "GtpIns MUST NOT be broadcast");

            if (GTP_PACKET_GET_SEQ_NUM(Packet) != GtpTrans->s_LastSendSeqNum)
            {
                PRINT_LOG(WARNING, "Not the ACK we are waiting for, drop it");
                break;
            }

            if (GTP_LIST_IS_EMPTY(&GtpTrans->s_SendingPacketList))
            {
                PRINT_LOG(WARNING, "It is too late to receive this ACK, drop it");
                break;
            }

            // Cancel retry task
            GtpScheduleCancelDelayTask(&GtpIns->s_ScheduleTaskList,
                GtpReSendFirstPacket, GtpTrans);

            // Get first packet
            SendPacket = GTP_LIST_ENTRY(GTP_LIST_FIRST(&GtpTrans->s_SendingPacketList),
                GtpSendingPacket);

            // If finish flag is set and data length is greater than zeror
            // resume sent callback with succeed error code
            TempPacket = (GtpPacket *)SendPacket->s_PacketBuf;
            if (GTP_PACKET_IS_FIN(TempPacket) &&
                GTP_PACKET_GET_DATA_LEN(TempPacket) > 0)
            {
                GtpResumeSentCallback(GtpTrans, GMI_SUCCESS);
            }

            // Remove the it from sending list
            GTP_LIST_REMOVE(&GtpTrans->s_SendingPacketList, SendPacket);

            // Release memory
            free(SendPacket);

            // If sending is not empty, send next packet
            if (!GTP_LIST_IS_EMPTY(&GtpTrans->s_SendingPacketList))
            {
                RetVal = GtpSendFirstPacket(GtpTrans, false);
                if (RetVal != GMI_SUCCESS)
                {
                    GtpListNode * Node = NULL;
                    GTP_LIST_FOR_EACH(Node, &GtpTrans->s_SendingPacketList)
                    {
                        GtpSendingPacket * SendingPacket =
                            GTP_LIST_ENTRY(Node, GtpSendingPacket);
                        Node = GTP_LIST_REMOVE(&GtpTrans->s_SendingPacketList,
                            SendingPacket);
                        free(SendingPacket);
                    }

                    PRINT_LOG(ERROR, "Failed to send packet");
                    GtpResumeSentCallback(GtpTrans, RetVal);
                }
            }
        }
        else // if (GTP_PACKET_IS_ACK(Packet))
        {
            // If it is broadcast
            if (IsBroadcast)
            {
                // Ignore sequence number, ignore all flags
                GtpResumeRecvCallback(GtpTrans, Packet->s_DataBuf,
                    GTP_PACKET_GET_DATA_LEN(Packet), GTP_PACKET_GET_DATA_TYPE(Packet));
                break;
            }

            // Whatever we received, send ack back
            RetVal = GtpTransSendAckPacket(GtpTrans, Packet);
            if (RetVal != GMI_SUCCESS)
            {
                PRINT_LOG(ERROR, "Failed to send ack");
                break;
            }

            // We received an packet without data, and it is not an ACK
            // It is the start packet
            if (GTP_PACKET_GET_DATA_LEN(Packet) == 0)
            {
                // Reset the received status
                GtpTrans->s_LastRecvSeqNum = GTP_INVALID_SEQ_NUM;
                break;
            }

            // If not the new transaction, check the sequence number
            if (GtpTrans->s_LastRecvSeqNum != GTP_INVALID_SEQ_NUM)
            {
                // Get next packet sequence number
                uint16_t NextSeqNum = GtpTrans->s_LastRecvSeqNum == GTP_MAX_SEQ_NUM ?
                    GTP_MIN_SEQ_NUM : GtpTrans->s_LastRecvSeqNum + 1;
                if (GTP_PACKET_GET_SEQ_NUM(Packet) != NextSeqNum)
                {
                    PRINT_LOG(WARNING, "Unknown packet, drop it");
                    break;
                }
            }

            // Update last received sequence number
            GtpTrans->s_LastRecvSeqNum = GTP_PACKET_GET_SEQ_NUM(Packet);


            // If begin flag is set
            if (GTP_PACKET_IS_BEG(Packet))
            {
                // Update last received data type
                GtpTrans->s_LastRecvDataType = GTP_PACKET_GET_DATA_TYPE(Packet);

                // If buffer is dirty, clear it
                GtpBufferClear(&GtpTrans->s_ReceivingBuffer);
            }
            else
            {
                if (GtpTrans->s_LastRecvDataType != GTP_PACKET_GET_DATA_TYPE(Packet))
                {
                    PRINT_LOG(WARNING, "Data type not matched, use the first one");
                }

                if (GtpBufferGetDataLength(&GtpTrans->s_ReceivingBuffer) == 0)
                {
                    PRINT_LOG(WARNING, "Parts of the data is broken");
                }
            }

            // Put the data carried by packet into the buffer
            RetVal = GtpBufferPutData(&GtpTrans->s_ReceivingBuffer, Packet->s_DataBuf,
                GTP_PACKET_GET_DATA_LEN(Packet));
            if (RetVal != GMI_SUCCESS)
            {
                PRINT_LOG(ERROR, "Failed to put data into buffer");
                break;
            }

            // If finish flag is set
            if (GTP_PACKET_IS_FIN(Packet))
            {
                // Resume the receive callback
                GtpResumeRecvCallback(GtpTrans,
                    GtpBufferGetDataBuffer(&GtpTrans->s_ReceivingBuffer),
                    GtpBufferGetDataLength(&GtpTrans->s_ReceivingBuffer),
                    GtpTrans->s_LastRecvDataType);
            }
        }
    } while (0);

    GtpRelease(GtpTrans);
}
