#include "sys_search_preset_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysSearchPresetCommandExecutor::SysSearchPresetCommandExecutor(void)
    : SysBaseCommandExecutor(SYSCODE_SEARCH_PTZPRESET_REQ, CT_ShortTask )
{
}


SysSearchPresetCommandExecutor::~SysSearchPresetCommandExecutor(void)
{
}


GMI_RESULT SysSearchPresetCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysSearchPresetCommandExecutor> SearchPresetCommandExecutor(BaseMemoryManager::Instance().New<SysSearchPresetCommandExecutor>() );
    if (NULL == SearchPresetCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    SearchPresetCommandExecutor->m_Session = Packet->GetSession();
    SearchPresetCommandExecutor->m_Reply   = Packet->Clone();
    GMI_RESULT Result = SearchPresetCommandExecutor->SetParameter( m_SystemServiceManager, m_Argument, m_ArgumentSize);
    if (FAILED(Result))
    {
        return Result;
    }

    CommandExecutor = SearchPresetCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysSearchPresetCommandExecutor::Execute()
{
    int32_t           MessageCode     = RETCODE_OK;
    SystemPacket     *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();
    uint8_t          *PayloadBuff     = CommandPacket->GetPacketPayloadBuffer();
    SysPkgAttrHeader *SysPkgAttrHdPtr = (SysPkgAttrHeader*)PayloadBuff;
    SysPkgPtzPresetInfo SysPtzPresetInfo;

    do
    {
        if (1 != CommandPacket->GetAttrCount())
        {
            MessageCode = RETCODE_ERROR;
            SYS_ERROR("ReqAttrCnt %d Error\n", CommandPacket->GetAttrCount());
            break;
        }

        SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);
        if (SysPkgAttrHdPtr->s_Type != TYPE_PTZPRESET_SEARCH)
        {
            MessageCode = RETCODE_ERROR;
            SYS_ERROR("SysPkgAttrHdPtr->s_Type %d error\n", SysPkgAttrHdPtr->s_Type);
            break;
        }

        SysPkgPresetSearch *SysPresetSearch = (SysPkgPresetSearch*)(PayloadBuff + sizeof(SysPkgAttrHeader));
        int32_t Index = NETWORK_TO_HOST_UINT(SysPresetSearch->s_PresetIndex);
        GMI_RESULT Result = m_SystemServiceManager->SvrGetPresetInfo(Index, &SysPtzPresetInfo);
        if (FAILED(Result))
        {
            MessageCode = RETCODE_ERROR;
            SYS_ERROR("get index%d info fail Result = 0x%lx\n", Index, Result);
            break;
        }
        SysPtzPresetInfo.s_PtzId       = HOST_TO_NETWORK_UINT(SysPtzPresetInfo.s_PtzId);
        SysPtzPresetInfo.s_PresetIndex = HOST_TO_NETWORK_UINT(SysPtzPresetInfo.s_PresetIndex);
    }
    while(0);

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;
    uint16_t AttrCnt;

    if (MessageCode == RETCODE_OK)
    {
        AttrCnt = 1;
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgPtzPresetInfo);
    }
    else
    {
        AttrCnt = 1;
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    }
    PayloadTotalSize = PacketAttrHdSize1*AttrCnt + sizeof(SdkPkgTransferProtocolKey);
    AppPacketSize    = PacketHeaderSize  + PacketAttrHdSize1*AttrCnt;
    ReferrencePtr<SystemPacket> Reply(BaseMemoryManager::Instance().New<SystemPacket>());
    if (NULL == Reply.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GMI_RESULT Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if (FAILED(Result))
    {
        return Result;
    }

    //fill packet header
    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_SEARCH_PTZPRESET_RSP,
                 AttrCnt,
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
        for (int32_t Id = 0; Id < AttrCnt; Id++)
        {
            SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)(PayloadBuffer + Offset);
            SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_PTZPRESET);
            SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
            Offset                   += sizeof(SysPkgAttrHeader);
            memcpy((PayloadBuffer + Offset), &SysPtzPresetInfo, sizeof(SysPkgPtzPresetInfo));
            Offset                   += sizeof(SysPkgPtzPresetInfo);
        }
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

    SYS_INFO("%s normal out........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT  SysSearchPresetCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}










