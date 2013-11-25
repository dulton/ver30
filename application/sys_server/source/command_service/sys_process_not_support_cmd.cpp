#include "sys_process_not_support_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"
#include "application_packet_header.h"

SysProcessNotSupportCommandExecutor::SysProcessNotSupportCommandExecutor(void)
    : SysBaseCommandExecutor(0xffff, CT_ShortTask )
{
}


SysProcessNotSupportCommandExecutor::~SysProcessNotSupportCommandExecutor(void)
{
}


GMI_RESULT  SysProcessNotSupportCommandExecutor::Create(ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SafePtr<SysProcessNotSupportCommandExecutor> PorcessNotSupportCommandExecutor( BaseMemoryManager::Instance().New<SysProcessNotSupportCommandExecutor>() );
    if (NULL == PorcessNotSupportCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    PorcessNotSupportCommandExecutor->m_Session = Packet->GetSession();
    PorcessNotSupportCommandExecutor->m_Reply   = Packet->Clone();
    GMI_RESULT Result = PorcessNotSupportCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);
    if (FAILED(Result))
    {
        return Result;
    }

    CommandExecutor = PorcessNotSupportCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysProcessNotSupportCommandExecutor::Execute()
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    int32_t     	  MessageCode     = RETCODE_NOSUPPORT;
    GMI_RESULT        Result          = GMI_SUCCESS;
    SystemPacket     *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();

    // Reply
    uint16_t PacketHeaderSize  = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    uint16_t PayloadTotalSize  = PacketAttrHdSize1 + sizeof(SdkPkgTransferProtocolKey);
    uint16_t AppPacketSize     = PacketHeaderSize + PacketAttrHdSize1;

    ReferrencePtr<SystemPacket> Reply(BaseMemoryManager::Instance().New<SystemPacket>() );
    if (NULL == Reply.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    //fill packet header
    Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if (FAILED(Result))
    {
        SYS_ERROR("AllocatePacketBuffer fail\n");
        return Result;
    }

    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 0xffff,
                 1,
                 AppPacketSize,
                 CommandPacket->GetSessionId(),
                 CommandPacket->GetSequenceNumber()
             );
    if (FAILED(Result))
    {
        SYS_ERROR("FillPacketHeader fail\n");
        return Result;
    }

    //fill packet body
    uint8_t           *PayloadBuffer = Reply->GetPacketPayloadBuffer();
    uint16_t           Offset        = 0;
    SysPkgAttrHeader  *SysPkgAtrrHdPtr;
    SysPkgMessageCode *SysPkgMessageCodePtr;

    SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)PayloadBuffer;
    SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_MESSAGECODE);
    SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
    Offset                   += sizeof(SysPkgAttrHeader);
    SysPkgMessageCodePtr      = (SysPkgMessageCode*)(PayloadBuffer + Offset);
    SysPkgMessageCodePtr->s_MessageCode = HOST_TO_NETWORK_UINT(MessageCode);
    SysPkgMessageCodePtr->s_MessageLen  = 0;

    Offset += sizeof(SysPkgMessageCode);
    Offset += SysPkgMessageCodePtr->s_MessageLen;

    //fill packet tail
    Result = SystemPacket::FillPacketSdkTransferProtocolKey(
                 (PayloadBuffer + Offset),
                 CommandPacket->GetSdkTransferKeyTag(),
                 CommandPacket->GetSdkTransferProtocolSessionId(),
                 CommandPacket->GetSdkTransferProtocolSequenceNumber(),
                 CommandPacket->GetSdkTransferProtocolAuthValue()
             );
    if (FAILED(Result))
    {
        SYS_ERROR("FillPacketSdkTransferProtocolKey fail\n");
        return Result;
    }

    m_Reply = Reply;

    SYS_INFO("%s:%s normal out..........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT  SysProcessNotSupportCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}

