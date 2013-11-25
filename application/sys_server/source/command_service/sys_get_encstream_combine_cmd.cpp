#include "sys_get_encstream_combine_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"
#include "application_packet_header.h"

SysGetEncStreamCombineCommandExecutor::SysGetEncStreamCombineCommandExecutor(void)
    : SysBaseCommandExecutor(SYSCODE_GET_ENCSTREAM_COMBINE_REQ, CT_ShortTask )
{
}


SysGetEncStreamCombineCommandExecutor::~SysGetEncStreamCombineCommandExecutor(void)
{
}


GMI_RESULT  SysGetEncStreamCombineCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetEncStreamCombineCommandExecutor> GetEncStreamCombineCommandExecutor( BaseMemoryManager::Instance().New<SysGetEncStreamCombineCommandExecutor>() );
    if (NULL == GetEncStreamCombineCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetEncStreamCombineCommandExecutor->m_Session = Packet->GetSession();
    GetEncStreamCombineCommandExecutor->m_Reply = Packet->Clone();
    GMI_RESULT Result = GetEncStreamCombineCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);
    if (FAILED(Result))
    {
        return Result;
    }

    CommandExecutor = GetEncStreamCombineCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetEncStreamCombineCommandExecutor::Execute()
{
    int32_t       MessageCode   = RETCODE_ERROR;
    GMI_RESULT    Result        = GMI_SUCCESS;
    SystemPacket *CommandPacket = (SystemPacket*)m_Reply.GetPtr();
    SysPkgEncStreamCombine SysEncStreamCombine;
    //uint32_t      AuthValue = CommandPacket->GetSdkTransferProtocolAuthValue();

    memset(&SysEncStreamCombine, 0, sizeof(SysPkgEncStreamCombine));
    Result = m_SystemServiceManager->SvrGetVideoEncStreamCombine(&SysEncStreamCombine);
    if (SUCCEEDED(Result))
    {
        MessageCode = RETCODE_OK;
        SysEncStreamCombine.s_VideoId = HOST_TO_NETWORK_UINT(SysEncStreamCombine.s_VideoId);
        SysEncStreamCombine.s_EnableStreamNum = HOST_TO_NETWORK_UINT(SysEncStreamCombine.s_EnableStreamNum);
        SysEncStreamCombine.s_StreamCombineNo = HOST_TO_NETWORK_UINT(SysEncStreamCombine.s_StreamCombineNo);
    }

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;

    if (MessageCode == RETCODE_OK)
    {
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgEncStreamCombine);
    }
    else
    {
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    }
    PayloadTotalSize = PacketAttrHdSize1 + sizeof(SdkPkgTransferProtocolKey);
    AppPacketSize    = PacketHeaderSize  + PacketAttrHdSize1;
    ReferrencePtr<SystemPacket> Reply(BaseMemoryManager::Instance().New<SystemPacket>());
    if (NULL == Reply.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if (FAILED(Result))
    {
        return Result;
    }

    //fill packet header
    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_GET_ENCSTREAM_COMBINE_RSP,
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
    uint8_t *PayloadBuffer = Reply->GetPacketPayloadBuffer();
    uint16_t Offset = 0;
    SysPkgAttrHeader *SysPkgAtrrHdPtr;

    if (MessageCode == RETCODE_OK)
    {
        SysPkgEncStreamCombine *SysEncStreamCombinePtr;

        SysPkgAtrrHdPtr = (SysPkgAttrHeader*)PayloadBuffer;
        SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_ENCSTREAM_COMBINE);
        SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
        Offset                   += sizeof(SysPkgAttrHeader);
        SysEncStreamCombinePtr    = (SysPkgEncStreamCombine*)(PayloadBuffer + Offset);
        memcpy((void_t*)SysEncStreamCombinePtr, (void_t*)&SysEncStreamCombine,  sizeof(SysPkgEncStreamCombine));
        Offset                   += sizeof(SysPkgEncStreamCombine);
    }
    else
    {
        SysPkgMessageCode *SysPkgMessageCodePtr;

        SysPkgAtrrHdPtr = (SysPkgAttrHeader*)PayloadBuffer;
        SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_MESSAGECODE);
        SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
        Offset                   += sizeof(SysPkgAttrHeader);
        SysPkgMessageCodePtr      = (SysPkgMessageCode*)(PayloadBuffer + Offset);
        SysPkgMessageCodePtr->s_MessageCode = HOST_TO_NETWORK_UINT(MessageCode);
        SysPkgMessageCodePtr->s_MessageLen  = 0;
        Offset                   += sizeof(SysPkgMessageCode);
        Offset                   += SysPkgMessageCodePtr->s_MessageLen;
    }

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

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetEncStreamCombineCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}



