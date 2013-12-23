#include "sys_stop_3a_cmd.h"
#include "system_packet.h"
#include "sys_env_types.h"
#include "log.h"


SysStop3ACommandExecutor::SysStop3ACommandExecutor()
    :  SysBaseCommandExecutor(SYSCODE_STOP_3A_REQ, CT_ShortTask)
{
}


SysStop3ACommandExecutor::~SysStop3ACommandExecutor()
{
}


GMI_RESULT  SysStop3ACommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysStop3ACommandExecutor>Stop3ACommandExecutor( BaseMemoryManager::Instance().New<SysStop3ACommandExecutor>() );
    if (NULL == Stop3ACommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    if (1 != SysPacket->GetAttrCount())
    {
        return GMI_FAIL;
    }

    Stop3ACommandExecutor->m_Session = Packet->GetSession();
    Stop3ACommandExecutor->m_Reply   = Packet->Clone();
    Stop3ACommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = Stop3ACommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysStop3ACommandExecutor::Execute()
{
    int32_t       MessageCode         = RETCODE_OK;
    GMI_RESULT    Result              = GMI_SUCCESS;
    SystemPacket *CommandPacket       = (SystemPacket*) m_Reply.GetPtr();
    uint8_t      *PayloadBuff         = CommandPacket->GetPacketPayloadBuffer();
    SysPkgAttrHeader *SysPkgAttrHdPtr = (SysPkgAttrHeader*)PayloadBuff;


    SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);
    if (SysPkgAttrHdPtr->s_Type != TYPE_INTVALUE)
    {      
        SYS_ERROR("SysPkgAttrHdPtr->s_Type %d incorrect\n", SysPkgAttrHdPtr->s_Type);
        MessageCode = RETCODE_ERROR;
    }
    else
    {
    	GMI_RESULT Result = m_SystemServiceManager->SvrStop3A();
    	if (FAILED(Result))
    	{
    		SYS_ERROR("Stop 3A fail, Result = 0x%lx\n", Result);
    		MessageCode = RETCODE_ERROR;
    	}
    }

    // Reply
    uint16_t PacketHeaderSize  = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    uint16_t PayloadTotalSize  = PacketAttrHdSize1 + sizeof(SdkPkgTransferProtocolKey);
    uint16_t AppPacketSize     = PacketHeaderSize + PacketAttrHdSize1;

    ReferrencePtr<SystemPacket> Reply(BaseMemoryManager::Instance().New<SystemPacket>());
    if (NULL == Reply.GetPtr())
    {
        SYS_ERROR("Reply new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if (FAILED(Result))
    {
        SYS_ERROR("AllocatePacketBuffer fail, Result = 0x%lx\n", Result);
        return Result;
    }

    //fill packet header
    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_STOP_3A_RSP,
                 1,
                 AppPacketSize,
                 CommandPacket->GetSessionId(),
                 CommandPacket->GetSequenceNumber()
             );
    if (FAILED(Result))
    {
        SYS_ERROR("FillPacketHeader fail, Result = 0x%lx\n", Result);
        return Result;
    }

    uint8_t *PayloadBuffer = Reply->GetPacketPayloadBuffer();
    uint16_t Offset = 0;
    SysPkgAttrHeader *SysPkgAtrrHdPtr;
    SysPkgMessageCode *SysPkgMessageCodePtr;

    SysPkgAtrrHdPtr = (SysPkgAttrHeader*)PayloadBuffer;
    SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_MESSAGECODE);
    SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
    Offset += sizeof(SysPkgAttrHeader);
    SysPkgMessageCodePtr = (SysPkgMessageCode*)(PayloadBuffer + Offset);
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
        SYS_ERROR("FillPacketSdkTransferProtocolKey fail, Result = 0x%lx\n", Result);
        return Result;
    }

    m_Reply = Reply;

    return GMI_SUCCESS;
}


GMI_RESULT  SysStop3ACommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}




