#pragma once

#include "application_packet_header.h"
#include "base_packet.h"

class ApplicationPacket : public BasePacket
{
public:
    ApplicationPacket(void);
    virtual ~ApplicationPacket(void);

    virtual GMI_RESULT  Initialize( const uint8_t PacketTag[GMI_XXX_MESSAGE_TAG_LENGTH], uint8_t MajorVersion, uint8_t MinorVersion );
    virtual GMI_RESULT  Deinitialize();
    virtual GMI_RESULT  Create( ReferrencePtr<BaseSession,DefaultObjectDeleter,MultipleThreadModel> Session );

    virtual GMI_RESULT  AllocatePacketBuffer( size_t HeaderExtensionSize, size_t PayloadSize );
    virtual GMI_RESULT  CalculatePacketChecksum();
    virtual GMI_RESULT  Submit( ReferrencePtr<BaseSession,DefaultObjectDeleter,MultipleThreadModel> Session );
    virtual BasePacket* Clone();
    virtual GMI_RESULT  Lock();
    virtual GMI_RESULT  Unlock();

    inline const uint8_t*  GetMessageTag() const
    {
        return m_PacketHeaderBuffer;
    }
    inline uint8_t         GetHeaderFlags() const
    {
        return m_PacketHeaderBuffer[4];
    }
    inline uint8_t         GetHeaderExtensionSize() const
    {
        uint8_t PaddingByteNumber = GetPaddingByteNumber();
        return m_PacketHeaderBuffer[5]-((uint8_t)((PaddingByteNumber>>6)&0x3));
    }
    inline uint8_t         GetMajorVersion() const
    {
        return m_PacketHeaderBuffer[6];
    }
    inline uint8_t         GetMinorVersion() const
    {
        return m_PacketHeaderBuffer[7];
    }

    inline uint16_t        GetSequenceNumber() const
    {
        uint16_t SequenceNumber = 0;
        BIGENDIAN_TO_USHORT( m_PacketHeaderBuffer+8, SequenceNumber );
        return SequenceNumber;
    }
    inline uint8_t         GetPaddingByteNumber() const
    {
        return m_PacketHeaderBuffer[10];
    }
    inline uint8_t         GetMessageType() const
    {
        return m_PacketHeaderBuffer[11];
    }
    inline uint16_t        GetMessageId() const
    {
        uint16_t MessageId = 0;
        BIGENDIAN_TO_USHORT( m_PacketHeaderBuffer+12, MessageId );
        return MessageId;
    }
    inline uint16_t		   GetMessageLength() const
    {
        uint8_t PaddingByteNumber = GetPaddingByteNumber();
        uint16_t MessageLength = 0;
        BIGENDIAN_TO_USHORT( m_PacketHeaderBuffer+14, MessageLength );
        return MessageLength-((uint8_t)((PaddingByteNumber>>4)&0x3));
    }
    static GMI_RESULT   FillPacketHeader( uint8_t       *PacketHeader,
                                          const uint8_t *MessageTag,
                                          uint8_t        HeaderFlags,
                                          uint8_t        MajorVersion,
                                          uint8_t        MinorVersion,
                                          uint16_t       SequenceNumber,
                                          uint8_t        MessageType,
                                          uint16_t       MessageId );

private:
    GMI_RESULT  _CalculatePacketChecksum();

private:
    uint8_t             m_PacketTag[GMI_XXX_MESSAGE_TAG_LENGTH];
    uint8_t             m_MajorVersion;
    uint8_t             m_MinorVersion;

    static const size_t ms_PacketHeaderSize;
    uint8_t             m_PacketHeader[sizeof(struct BaseMessageHeader)];
    size_t              m_Transferred;
    uint8_t             *m_Offset;
    uint32_t            m_PacketChecksum;
    GMI_Mutex           m_InstanceMutex;
};
