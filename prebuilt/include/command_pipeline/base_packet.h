#pragma once

#include "base_session.h"
#include "gmi_system_headers.h"

class BasePacket
{
protected:
    BasePacket(void);
public:
    virtual ~BasePacket(void);

    // parse packet from session
    virtual GMI_RESULT  Create( ReferrencePtr<BaseSession,DefaultObjectDeleter,MultipleThreadModel> Session )
    {
        m_Session = Session;
        return GMI_SUCCESS;
    };
    // prepare to send packet to session
    virtual GMI_RESULT  AllocatePacketBuffer( size_t HeaderExtensionSize, size_t PayloadSize ) = 0;
    virtual GMI_RESULT  CalculatePacketChecksum() = 0;
    virtual GMI_RESULT  Submit( ReferrencePtr<BaseSession,DefaultObjectDeleter,MultipleThreadModel> Session ) = 0;
    virtual BasePacket* Clone() = 0;
    virtual GMI_RESULT  Lock() = 0;
    virtual GMI_RESULT  Unlock() = 0;

    inline  ReferrencePtr<BaseSession,DefaultObjectDeleter,MultipleThreadModel>& GetSession()
    {
        return m_Session;
    }
    inline size_t                      GetPacketSize()
    {
        return m_PacketSize;
    }
    inline uint8_t*                    GetPacketHeaderBuffer()
    {
        return m_PacketHeaderBuffer;
    }
    inline size_t                      GetPacketHeaderBufferSize()
    {
        return m_PacketHeaderBufferSize;
    }
    inline uint8_t*                    GetPacketHeaderExtensionBuffer()
    {
        return m_PacketHeaderExtensionBuffer;
    }
    inline size_t                      GetPacketHeaderExtensionBufferSize()
    {
        return m_PacketHeaderExtensionBufferSize;
    }
    inline uint8_t*                    GetPacketPayloadBuffer()
    {
        return m_PacketPayloadBuffer;
    }
    inline size_t                      GetPacketPayloadBufferSize()
    {
        return m_PacketPayloadBufferSize;
    }

protected:
    ReferrencePtr<BaseSession,DefaultObjectDeleter,MultipleThreadModel>  m_Session;
    SafePtr<uint8_t, DefaultObjectsDeleter>                              m_Buffer;
    size_t    m_PacketSize;
    uint8_t   *m_PacketHeaderBuffer;
    size_t    m_PacketHeaderBufferSize;
    uint8_t   *m_PacketHeaderExtensionBuffer;
    size_t    m_PacketHeaderExtensionBufferSize;
    uint8_t   *m_PacketPayloadBuffer;
    size_t    m_PacketPayloadBufferSize;
    uint32_t  m_CalculatedChecksum;
};
