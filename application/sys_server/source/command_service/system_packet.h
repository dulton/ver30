
#ifndef __SYSTEM_PACKET_H__
#define __SYSTEM_PACKET_H__
#include "sys_env_types.h"
#include "base_packet.h"

class SystemPacket : public BasePacket
{
public:
    SystemPacket(void);
    virtual ~SystemPacket(void);

    virtual GMI_RESULT  Create( ReferrencePtr<BaseSession, DefaultObjectDeleter, MultipleThreadModel> Session );
    virtual GMI_RESULT  AllocatePacketBuffer( size_t HeaderExtensionSize, size_t PayloadSize );
    virtual GMI_RESULT  CalculatePacketChecksum();
    virtual GMI_RESULT  Submit( ReferrencePtr<BaseSession, DefaultObjectDeleter, MultipleThreadModel> Session );
    virtual BasePacket* Clone();
    virtual GMI_RESULT  Lock();
    virtual GMI_RESULT  Unlock();

    const uint8_t*      GetMessageTag();
    uint8_t             GetVersion();
    uint16_t            GetSequenceNumber();
    uint16_t            GetMessageCode();
    uint16_t            GetMessageId();
    uint16_t            GetSessionId();
    uint16_t            GetAttrCount();
    uint16_t            GetMessageLength();
    static GMI_RESULT   FillPacketHeader(
        uint8_t       *PacketHeader,
        const uint8_t *MessageTag,
        uint8_t        Version,
        uint16_t       MessageCode,
        uint16_t       AttrCount,
        uint16_t       TotalLen,
        uint16_t       SessionId,
        uint16_t       SequenceNumber
    );

    const uint8_t*      GetSdkTransferKeyTag();
    uint16_t            GetSdkTransferProtocolSequenceNumber();
    uint16_t            GetSdkTransferProtocolSessionId();
    uint32_t            GetSdkTransferProtocolAuthValue();
    static GMI_RESULT    FillPacketSdkTransferProtocolKey(
        uint8_t	      *PacketSdkTransferProtocol,
        const uint8_t *MessageKeyTag,
        uint16_t       SessionId,
        uint16_t       SequenceNumber,
        uint32_t       AuthValue);

};
#endif

