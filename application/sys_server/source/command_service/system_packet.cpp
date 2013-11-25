#include "system_packet.h"
#include "gmi_system_headers.h"
#include "application_packet_header.h"
#include "log.h"


SystemPacket::SystemPacket(void)
    : BasePacket()
{
}

SystemPacket::~SystemPacket(void)
{
}

GMI_RESULT  SystemPacket::Create( ReferrencePtr<BaseSession, DefaultObjectDeleter, MultipleThreadModel> Session )
{
    GMI_RESULT Result = GMI_FAIL;

    Result = BasePacket::Create( Session );
    if ( FAILED( Result ))
    {
        return Result;
    }

    static const size_t PacketHeaderSize = sizeof( SysPkgHeader );
    if ( PacketHeaderSize > Session->ReadableDataSize() )
    {
        return GMI_FAIL;
    }

    uint8_t PacketHeader[PacketHeaderSize];
    size_t Transferred = 0;
    SysPkgHeader *SysPkgHdPtr;

    do
    {
        Result = Session->Read(0, PacketHeader, PacketHeaderSize, &Transferred);
        if (FAILED(Result))
        {
            return Result;
        }

        if (Transferred < PacketHeaderSize)
        {
            return GMI_FAIL;
        }

        SysPkgHdPtr = (SysPkgHeader*)PacketHeader;
        if (0 != memcmp( GMI_SYS_MESSAGE_TAG, SysPkgHdPtr->s_SysMsgTag, GMI_SYS_MESSAGE_TAG_LENGTH))
        {
            Session->ReadAdvance(1);
            continue;
        }

        if (SysPkgHdPtr->s_Version != SYS_COMM_VERSION)
        {
            Session->ReadAdvance(1);
            continue;
        }

        if (SysPkgHdPtr->s_HeaderLen != sizeof(SysPkgHeader))
        {
            Session->ReadAdvance(1);
            continue;
        }

        uint16_t AppPkgTotalLen = NETWORK_TO_HOST_USHORT(SysPkgHdPtr->s_TotalLen);
        m_PacketSize = AppPkgTotalLen;
        m_PacketSize += sizeof(SdkPkgTransferProtocolKey); //sdk transferprotocol key add to the tail of system body ,7/23/2013
        if (Session->ReadableDataSize() < m_PacketSize)
        {
            return GMI_FAIL;
        }

        if (SysPkgHdPtr->s_HeaderLen > AppPkgTotalLen/*SysPkgHdPtr->s_TotalLen*/)
        {
            return GMI_FAIL;
        }

        m_Buffer = BaseMemoryManager::Instance().News<uint8_t>(m_PacketSize);
        if (NULL == m_Buffer.GetPtr())
        {
            return GMI_OUT_OF_MEMORY;
        }

        Result = Session->Read(0, m_Buffer.GetPtr(), m_PacketSize, &Transferred);
        if (FAILED(Result))
        {
            return Result;
        }

        if (Transferred < m_PacketSize)
        {
            return GMI_FAIL;
        }

        //check sdk transfer protocol key tag
        SdkPkgTransferProtocolKey *SdkTransferProtocolKeyPtr;
        SdkTransferProtocolKeyPtr = (SdkPkgTransferProtocolKey*)(m_Buffer.GetPtr() + AppPkgTotalLen);

        if (0 != memcmp(GMI_TRANSFER_KEY_TAG, SdkTransferProtocolKeyPtr->s_TransferKey, GMI_TRANSFER_KEY_TAG_LENGTH))
        {
            return GMI_FAIL;
        }

        m_PacketHeaderBuffer = m_Buffer.GetPtr();
        m_PacketHeaderBufferSize = PacketHeaderSize;

        m_PacketPayloadBuffer = m_PacketHeaderBuffer + m_PacketHeaderBufferSize;
        //m_PacketPayloadBufferSize = SysPkgHdPtr->s_TotalLen - m_PacketHeaderBufferSize;
        m_PacketPayloadBufferSize = m_PacketSize - m_PacketHeaderBufferSize;

        Session->ReadAdvance(m_PacketSize);

        return GMI_SUCCESS;
    }
    while ( 1 );

    return GMI_SUCCESS;
}


GMI_RESULT  SystemPacket::CalculatePacketChecksum()
{
    return GMI_NOT_IMPLEMENT;
}


GMI_RESULT  SystemPacket::AllocatePacketBuffer( size_t HeaderExtensionSize, size_t PayloadSize )
{
    static const size_t PacketHeaderSize = sizeof( SysPkgHeader );
    if ( 0 != PacketHeaderSize%sizeof(uint32_t) )
    {
        return GMI_FAIL;
    }

    m_PacketSize = PacketHeaderSize + PayloadSize;
    m_Buffer = BaseMemoryManager::Instance().News<uint8_t>(m_PacketSize);
    if (NULL == m_Buffer.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    memset(m_Buffer.GetPtr(), 0, m_PacketSize);

    m_PacketHeaderBuffer = m_Buffer.GetPtr();
    m_PacketHeaderBufferSize = PacketHeaderSize;

    m_PacketPayloadBuffer = m_PacketHeaderBuffer + m_PacketHeaderBufferSize;
    m_PacketPayloadBufferSize = PayloadSize;

    return GMI_SUCCESS;
}

GMI_RESULT  SystemPacket::Submit( ReferrencePtr<BaseSession, DefaultObjectDeleter, MultipleThreadModel> Session )
{
    size_t Transferred = 0;
    GMI_RESULT Result = Session->Write(m_Buffer.GetPtr(), m_PacketSize, &Transferred );
    if (FAILED(Result))
    {
        return Result;
    }

    if (Transferred < m_PacketSize)
    {
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}

BasePacket* SystemPacket::Clone()
{
    SystemPacket *SysPacket = BaseMemoryManager::Instance().New<SystemPacket>();
    if ( NULL == SysPacket )
    {
        return NULL;
    }

    SysPacket->m_Session = m_Session;

    SysPacket->m_Buffer  = BaseMemoryManager::Instance().News<uint8_t>( m_PacketSize );
    if ( NULL == SysPacket->m_Buffer.GetPtr() )
    {
        return NULL;
    }

    SysPacket->m_PacketSize                      = m_PacketSize;
    memcpy( SysPacket->m_Buffer.GetPtr(), m_Buffer.GetPtr(), m_PacketSize );
    SysPacket->m_PacketHeaderBuffer              = SysPacket->m_Buffer.GetPtr();
    SysPacket->m_PacketHeaderBufferSize          = m_PacketHeaderBufferSize;
    SysPacket->m_PacketPayloadBuffer             = SysPacket->m_PacketHeaderBuffer+SysPacket->m_PacketHeaderBufferSize;
    SysPacket->m_PacketPayloadBufferSize         = m_PacketPayloadBufferSize;
    SysPacket->m_CalculatedChecksum              = m_CalculatedChecksum;

    return SysPacket;
}


GMI_RESULT SystemPacket::Lock()
{
    return GMI_SUCCESS;
}


GMI_RESULT SystemPacket::Unlock()
{
    return GMI_SUCCESS;
}


const uint8_t*  SystemPacket::GetMessageTag()
{
    SysPkgHeader *SysPkgHdPtr;

    SysPkgHdPtr = (SysPkgHeader*)m_PacketHeaderBuffer;
    return SysPkgHdPtr->s_SysMsgTag;
}

uint8_t   SystemPacket::GetVersion()
{
    SysPkgHeader *SysPkgHdPtr;

    SysPkgHdPtr = (SysPkgHeader*)m_PacketHeaderBuffer;
    return SysPkgHdPtr->s_Version;
}

uint16_t  SystemPacket::GetSequenceNumber()
{
    SysPkgHeader *SysPkgHdPtr;

    SysPkgHdPtr = (SysPkgHeader*)m_PacketHeaderBuffer;
    return NETWORK_TO_HOST_USHORT(SysPkgHdPtr->s_SeqNum);
}

uint16_t  SystemPacket::GetMessageCode()
{
    SysPkgHeader *SysPkgHdPtr;

    SysPkgHdPtr = (SysPkgHeader*)m_PacketHeaderBuffer;
    return NETWORK_TO_HOST_USHORT(SysPkgHdPtr->s_Code);
}

uint16_t  SystemPacket::GetSessionId()
{
    SysPkgHeader *SysPkgHdPtr;

    SysPkgHdPtr = (SysPkgHeader*)m_PacketHeaderBuffer;
    return NETWORK_TO_HOST_USHORT(SysPkgHdPtr->s_SessionId);
}

uint16_t  SystemPacket::GetAttrCount()
{
    SysPkgHeader *SysPkgHdPtr;

    SysPkgHdPtr = (SysPkgHeader*)m_PacketHeaderBuffer;
    return NETWORK_TO_HOST_USHORT(SysPkgHdPtr->s_AttrCount);
}

uint16_t  SystemPacket::GetMessageLength()
{
    SysPkgHeader *SysPkgHdPtr;

    SysPkgHdPtr = (SysPkgHeader*)m_PacketHeaderBuffer;
    return NETWORK_TO_HOST_USHORT(SysPkgHdPtr->s_TotalLen);
}

GMI_RESULT  SystemPacket::FillPacketHeader(
    uint8_t	      *PacketHeader,
    const uint8_t *MessageTag,
    uint8_t        Version,
    uint16_t       MessageCode,
    uint16_t       AttrCount,
    uint16_t       TotalLen,
    uint16_t       SessionId,
    uint16_t       SequenceNumber)
{
    SysPkgHeader *SysPkgHdPtr;

    SysPkgHdPtr = (SysPkgHeader*)PacketHeader;
    memset( SysPkgHdPtr, 0, sizeof(SysPkgHeader) );
    memcpy( SysPkgHdPtr->s_SysMsgTag, MessageTag, sizeof(uint8_t) * GMI_SYS_MESSAGE_TAG_LENGTH);
    SysPkgHdPtr->s_Version   = Version;
    SysPkgHdPtr->s_HeaderLen = sizeof(SysPkgHeader);
    SysPkgHdPtr->s_Code      = HOST_TO_NETWORK_USHORT(MessageCode);
    SysPkgHdPtr->s_AttrCount = HOST_TO_NETWORK_USHORT(AttrCount);
    SysPkgHdPtr->s_TotalLen  = HOST_TO_NETWORK_USHORT(TotalLen);
    SysPkgHdPtr->s_SessionId = HOST_TO_NETWORK_USHORT(SessionId);
    SysPkgHdPtr->s_SeqNum    = HOST_TO_NETWORK_USHORT(SequenceNumber);

    return GMI_SUCCESS;
}

const uint8_t*  SystemPacket::GetSdkTransferKeyTag()
{
    SdkPkgTransferProtocolKey *SdkTransferProtocolKeyPtr;

    SdkTransferProtocolKeyPtr = (SdkPkgTransferProtocolKey*)(m_PacketHeaderBuffer + GetMessageLength());

    return SdkTransferProtocolKeyPtr->s_TransferKey;
}

uint16_t  SystemPacket::GetSdkTransferProtocolSequenceNumber()
{
    SdkPkgTransferProtocolKey *SdkTransferProtocolKeyPtr;

    SdkTransferProtocolKeyPtr = (SdkPkgTransferProtocolKey*)(m_PacketHeaderBuffer + GetMessageLength());
    return SdkTransferProtocolKeyPtr->s_SeqNumber;
}

uint16_t  SystemPacket::GetSdkTransferProtocolSessionId()
{
    SdkPkgTransferProtocolKey *SdkTransferProtocolKeyPtr;

    SdkTransferProtocolKeyPtr = (SdkPkgTransferProtocolKey*)(m_PacketHeaderBuffer + GetMessageLength());

    return SdkTransferProtocolKeyPtr->s_SessionId;
}

uint32_t  SystemPacket::GetSdkTransferProtocolAuthValue()
{
    SdkPkgTransferProtocolKey *SdkTransferProtocolKeyPtr;

    SdkTransferProtocolKeyPtr = (SdkPkgTransferProtocolKey*)(m_PacketHeaderBuffer + GetMessageLength());

    return SdkTransferProtocolKeyPtr->s_AuthValue;
}

GMI_RESULT  SystemPacket::FillPacketSdkTransferProtocolKey(
    uint8_t	      *PacketSdkTransferProtocol,
    const uint8_t *MessageKeyTag,
    uint16_t       SessionId,
    uint16_t       SequenceNumber,
    uint32_t       AuthValue)
{
    SdkPkgTransferProtocolKey *SdkTransferProtocolKeyPtr;

    SdkTransferProtocolKeyPtr = (SdkPkgTransferProtocolKey*)PacketSdkTransferProtocol;
    memset(SdkTransferProtocolKeyPtr, 0, sizeof(SdkPkgTransferProtocolKey));
    memcpy(SdkTransferProtocolKeyPtr->s_TransferKey, MessageKeyTag, sizeof(uint8_t) * GMI_TRANSFER_KEY_TAG_LENGTH);
    SdkTransferProtocolKeyPtr->s_SeqNumber = SequenceNumber;//do not transfer from host to network
    SdkTransferProtocolKeyPtr->s_SessionId = SessionId;
    SdkTransferProtocolKeyPtr->s_AuthValue = AuthValue;

    return GMI_SUCCESS;
}


