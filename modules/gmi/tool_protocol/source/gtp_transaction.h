#ifndef __GMI_GTP_TRANSACTION_H__
#define __GMI_GTP_TRANSACTION_H__

#include "gtp_packet.h"
#include "gtp_instance.h"
#include "gtp_buffer.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define GTP_MAX_SEND_RETRY_TIMES 5

typedef struct tagGtpSendigPacket
{
    uint8_t     s_PacketBuf[GTP_MAX_PACKET_LENGTH];
    uint32_t    s_PacketLength;
    GtpListNode s_ListNode;
} GtpSendingPacket;

typedef struct tagGtpTransaction
{
    // Reference
    GtpReference          s_Reference;

    // Point to GTP instance
    GtpInstance * s_GtpInstance;

    // Broadcast flag
    boolean_t     s_IsBroadcast;

    // First sending flag
    boolean_t     s_IsFirstTimeSend;

    // Bind MAC address
    uint8_t       s_MacAddr[MAC_ADDRESS_LENGTH];

    // Bind transaction id
    uint16_t      s_TransId;

    // Bind session id
    uint32_t      s_SessionId;

    // Sequence number count, used to allocate sequence number
    uint16_t      s_SeqNumCount;

    // Send retry times
    uint16_t      s_SendRetryTimes;

    // Point to user instance if set
    void_t      * s_UserInstance;

    // Sending packet list
    GtpList       s_SendingPacketList;

    // Receiving buffer
    GtpBuffer     s_ReceivingBuffer;

    // Last send packet sequence number
    uint16_t      s_LastSendSeqNum;

    // Last recv packet sequence number
    uint16_t      s_LastRecvSeqNum;

    // Last recv packet data type
    uint8_t       s_LastRecvDataType;

    // Element for transaction list in GTP instance
    GtpListNode   s_ListNode;
} GtpTransaction;

GtpTransaction * GtpCreateTransHandleInternal(GtpInstance * GtpIns, uint16_t TransId,
    uint32_t SessionId, const uint8_t MacAddr[MAC_ADDRESS_LENGTH], boolean_t IsBroadcast);

void_t GtpTransFeedPacket(GtpTransaction * GtpTrans, GtpPacket * Packet,
    boolean_t IsBroadcast);

#ifdef __cplusplus
}
#endif

#endif // __GMI_GTP_TRANSACTION_H__
