#include <string.h>

#include "gtp_packet.h"

#include "debug.h"

GtpPacket * GtpPacketParse(const uint8_t * Buf, uint32_t BufLength)
{
    GtpPacket * Packet                            = (GtpPacket *)Buf;
    uint8_t     Checksum                          = 0x00;
    uint32_t    i                                 = 0;
    uint32_t    PacketLength                      = 0;
    uint32_t    DataLength                        = 0;
    uint8_t     DataChecksum[GTP_CHECKSUM_LENGTH] = {0x00, 0x00, 0x00, 0x00};

    if (BufLength < GTP_HEAD_LENGTH)
    {
        PRINT_LOG(WARNING, "Data length is less than packet head");
        return NULL;
    }

    if (Packet->s_Magic[0] != 'G' || Packet->s_Magic[1] != 'M')
    {
        PRINT_LOG(WARNING, "This packet is not GTP packet");
        return NULL;
    }

    for (i = 0; i < GTP_HEAD_LENGTH - 1; ++ i)
    {
        Checksum ^= Buf[i];
    }

    if (Checksum != Packet->s_Checksum)
    {
        PRINT_LOG(WARNING, "Packet head is broken");
        return NULL;
    }

    DataLength = GTP_PACKET_GET_DATA_LEN(Packet);
    if (DataLength > 0 && !GTP_PACKET_IS_ACK(Packet))
    {
        PacketLength = GTP_HEAD_LENGTH + DataLength + GTP_CHECKSUM_LENGTH;
        if (BufLength < PacketLength)
        {
            PRINT_LOG(WARNING, "Data length is less than the whole packet");
            return NULL;
        }

        for (i = 0; i < DataLength; ++ i)
        {
            DataChecksum[i % GTP_CHECKSUM_LENGTH] ^= Packet->s_DataBuf[i];
        }

        if (0 != memcmp(DataChecksum, Packet->s_DataBuf + DataLength,
            GTP_CHECKSUM_LENGTH))
        {
            PRINT_LOG(WARNING, "Packet body is broken");
            return NULL;
        }
    }

    return Packet;
}

void_t GtpPacketPrePack(uint8_t PacketBuf[GTP_MAX_PACKET_LENGTH], uint16_t TransId,
    uint32_t SessionId, const uint8_t * Buf, uint32_t BufLength, uint8_t Flags)
{
    GtpPacket * Packet                            = (GtpPacket *)PacketBuf;
    uint32_t    i                                 = 0;
    uint8_t     DataChecksum[GTP_CHECKSUM_LENGTH] = {0x00, 0x00, 0x00, 0x00};

    ASSERT(BufLength <= GTP_MAX_DATA_LENGTH, "BufLength MUST NOT be greater than %u",
        GTP_MAX_DATA_LENGTH);
    ASSERT(BufLength == 0 || Buf != NULL, "Buf MUST NOT be non-pointer, "
        "when BufLength is greater than 0");

    // Fill magic field
    Packet->s_Magic[0] = 'G';
    Packet->s_Magic[1] = 'M';

    // Fill transaction id field
    GTP_PACKET_SET_TRANS_ID(Packet, TransId);

    // Fill session id field
    GTP_PACKET_SET_SESSION_ID(Packet, SessionId);

    // Fill data length field
    GTP_PACKET_SET_DATA_LEN(Packet, BufLength);

    // Fill flags field
    Packet->s_Flags.u_Byte = Flags;

    if (BufLength > 0)
    {
        // Calculate checksum for packet body
        for (i = 0; i < BufLength; ++ i)
        {
            DataChecksum[i % GTP_CHECKSUM_LENGTH] ^= Buf[i];
        }

        // Fill data field
        memcpy(Packet->s_DataBuf, Buf, BufLength);

        // Add data checksum
        memcpy(Packet->s_DataBuf + BufLength, DataChecksum, GTP_CHECKSUM_LENGTH);
    }
}

void_t GtpPacketFixUpHead(uint8_t PacketBuf[GTP_MAX_PACKET_LENGTH], uint16_t SeqNum)
{
    GtpPacket * Packet   = (GtpPacket *)PacketBuf;
    uint32_t    i        = 0;
    uint8_t     Checksum = 0x00;

    // Fill sequence number field
    GTP_PACKET_SET_SEQ_NUM(Packet, SeqNum);

    // Calculate checksum for packet head
    for (i = 0; i < sizeof(GtpPacket) - 1; ++ i)
    {
        Checksum ^= PacketBuf[i];
    }

    // Fill checksum field
    Packet->s_Checksum = Checksum;
}

void_t GtpPackAckPacket(uint8_t PacketBuf[GTP_HEAD_LENGTH], GtpPacket * Packet)
{
    GtpPacket * PacketAck = (GtpPacket *)PacketBuf;
    uint8_t     Checksum  = 0x00;
    uint32_t    i         = 0;

    ASSERT(Packet != NULL, "Packet MUST NOT be non-pointer");

    // Fill magic field
    PacketAck->s_Magic[0] = 'G';
    PacketAck->s_Magic[1] = 'M';

    // Fill transaction id field
    GTP_PACKET_SET_TRANS_ID(PacketAck, GTP_PACKET_GET_TRANS_ID(Packet));

    // Fill session id field
    GTP_PACKET_SET_SESSION_ID(PacketAck, GTP_PACKET_GET_SESSION_ID(Packet));

    // Fill sequence number field
    PacketAck->s_SequenceNumber = Packet->s_SequenceNumber;

    // Fill data length field
    PacketAck->s_DataLength = 0;

    // Fill flags field
    PacketAck->s_Flags.u_Byte = 0;
    PacketAck->s_Flags.u_BitFields.s_ACK = 1;

    // Calculate checksum for packet head
    for (i = 0; i < sizeof(GtpPacket) - 1; ++ i)
    {
        Checksum ^= PacketBuf[i];
    }

    // Fill checksum field
    PacketAck->s_Checksum = Checksum;
}

