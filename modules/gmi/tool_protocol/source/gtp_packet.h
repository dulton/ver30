#ifndef __GMI_GTP_INTERNAL_H__
#define __GMI_GTP_INTERNAL_H__

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include <gmi_type_definitions.h>
#include <gmi_errors.h>

#include <pcap_session.h>

#include "gtp_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define GTP_CHECKSUM_LENGTH    4
#define GTP_MAX_PACKET_LENGTH  PCAP_SESSION_MAX_BUF_LENGTH
#define GTP_HEAD_LENGTH        sizeof(GtpPacket)
#define GTP_MAX_DATA_LENGTH    \
    (PCAP_SESSION_MAX_BUF_LENGTH - GTP_HEAD_LENGTH - GTP_CHECKSUM_LENGTH)

#define GTP_BROADCAST_TRANS_ID 0

#define GTP_TRANS_RES_FLAG     0x8000

#define GTP_MIN_TRANS_ID       1
#define GTP_MAX_TRANS_ID       0x7fff

#define GTP_INVALID_SEQ_NUM    0

#define GTP_INVALID_SESSION_ID 0

#define GTP_MIN_SEQ_NUM        1
#define GTP_MAX_SEQ_NUM        0xffff

#define GTP_PACKET_GET_TRANS_ID(p)    (ntohs((p)->s_TransId) ^ GTP_TRANS_RES_FLAG)
#define GTP_PACKET_SET_TRANS_ID(p, v) do { (p)->s_TransId = htons(v); } while (0)

#define GTP_PACKET_GET_SESSION_ID(p)     ntohl((p)->s_SessionId)
#define GTP_PACKET_SET_SESSION_ID(p, v)  do { (p)->s_SessionId = htonl(v); } while (0)

#define GTP_PACKET_GET_SEQ_NUM(p)     ntohs((p)->s_SequenceNumber)
#define GTP_PACKET_SET_SEQ_NUM(p, v)  do { (p)->s_SequenceNumber = htons(v); } while (0)

#define GTP_PACKET_GET_DATA_LEN(p)    ntohs((p)->s_DataLength)
#define GTP_PACKET_SET_DATA_LEN(p, v) do { (p)->s_DataLength = htons(v); } while (0)

#define GTP_FLAGS_IS_SET_ACK(f)       ((f).u_BitFields.s_ACK)
#define GTP_FLAGS_IS_SET_BEG(f)       ((f).u_BitFields.s_BEG)
#define GTP_FLAGS_IS_SET_FIN(f)       ((f).u_BitFields.s_FIN)

#define GTP_FLAGS_SET_ACK(f)          do { (f).u_BitFields.s_ACK = 1; } while (0)
#define GTP_FLAGS_SET_BEG(f)          do { (f).u_BitFields.s_BEG = 1; } while (0)
#define GTP_FLAGS_SET_FIN(f)          do { (f).u_BitFields.s_FIN = 1; } while (0)

#define GTP_FLAGS_CLR_ACK(f)          do { (f).u_BitFields.s_ACK = 0; } while (0)
#define GTP_FLAGS_CLR_BEG(f)          do { (f).u_BitFields.s_BEG = 0; } while (0)
#define GTP_FLAGS_CLR_GIN(f)          do { (f).u_BitFields.s_FIN = 0; } while (0)

#define GTP_FLAGS_SET_DATA_TYPE(f, v) do { (f).u_BitFields.s_DataType = (v); } while (0)
#define GTP_FLAGS_GET_DATA_TYPE(f)    ((f).u_BitFields.s_DataType)

#define GTP_PACKET_IS_ACK(p)          GTP_FLAGS_IS_SET_ACK((p)->s_Flags)
#define GTP_PACKET_IS_BEG(p)          GTP_FLAGS_IS_SET_BEG((p)->s_Flags)
#define GTP_PACKET_IS_FIN(p)          GTP_FLAGS_IS_SET_FIN((p)->s_Flags)

#define GTP_PACKET_GET_DATA_TYPE(p)   ((p)->s_Flags.u_BitFields.s_DataType)

typedef union tagGtpPacketFlags
{
    struct
    {
        uint8_t s_DataType : 2;
        uint8_t s_Reserved : 3;
        uint8_t s_BEG      : 1;
        uint8_t s_FIN      : 1;
        uint8_t s_ACK      : 1;
    } u_BitFields;
    uint8_t u_Byte;
} GtpPacketFlags;

typedef struct tagGtpPacket
{
    uint8_t        s_Magic[2];       // MUST be 'GM'
    uint16_t       s_TransId;        // 0 for broadcast and broadcast response
    uint32_t       s_SessionId;      // Use process id ad session id
    uint16_t       s_SequenceNumber; // 0 for invalid sequence number
    uint8_t        s_Reserved[2];    // Reserve 2 byte for extend
    uint16_t       s_DataLength;
    GtpPacketFlags s_Flags;
    uint8_t        s_Checksum;
    uint8_t        s_DataBuf[0];
} GtpPacket;

GtpPacket * GtpPacketParse(const uint8_t * Buf, uint32_t BufLength);

void_t GtpPacketPrePack(uint8_t PacketBuf[GTP_MAX_PACKET_LENGTH], uint16_t TransId,
    uint32_t SessionId, const uint8_t * Buf, uint32_t BufLength,
    uint8_t Flags);

void_t GtpPacketFixUpHead(uint8_t PacketBuf[GTP_MAX_PACKET_LENGTH], uint16_t SeqNum);

void_t GtpPackAckPacket(uint8_t PacketBuf[GTP_HEAD_LENGTH], GtpPacket * Packet);

#ifdef __cplusplus
}
#endif

#endif // __GMI_GTP_INTERNAL_H__
