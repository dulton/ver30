#include "sys_get_preset_info_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"
#include "application_packet_header.h"

SysGetPresetInfoCommandExecutor::SysGetPresetInfoCommandExecutor(void)
    : SysBaseCommandExecutor(SYSCODE_GET_PTZPRESET_REQ, CT_ShortTask )
{
}


SysGetPresetInfoCommandExecutor::~SysGetPresetInfoCommandExecutor(void)
{
}


GMI_RESULT  SysGetPresetInfoCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetPresetInfoCommandExecutor> GetPresetInfoCommandExecutor( BaseMemoryManager::Instance().New<SysGetPresetInfoCommandExecutor>() );
    if (NULL == GetPresetInfoCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetPresetInfoCommandExecutor->m_Session = Packet->GetSession();
    GetPresetInfoCommandExecutor->m_Reply = Packet->Clone();
    GMI_RESULT Result = GetPresetInfoCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);
    if (FAILED(Result))
    {
        return Result;
    }

    CommandExecutor = GetPresetInfoCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetPresetInfoCommandExecutor::Execute()
{
    int32_t       MessageCode   = RETCODE_ERROR;
    GMI_RESULT    Result        = GMI_SUCCESS;
    SystemPacket *CommandPacket = (SystemPacket*)m_Reply.GetPtr();
    //uint32_t      AuthValue = CommandPacket->GetSdkTransferProtocolAuthValue();

    ReferrencePtr<SysPkgPtzPresetInfo, DefaultObjectsDeleter>SysPresetInfoPtr(BaseMemoryManager::Instance().News<SysPkgPtzPresetInfo>(MAX_PRESETS));
    if (NULL == SysPresetInfoPtr.GetPtr())
    {
        SYS_ERROR("ptz preset info news fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ptz preset info news fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    int32_t PresetCount;
    Result = m_SystemServiceManager->SvrGetPresetInfo(&PresetCount, SysPresetInfoPtr.GetPtr());
    if (SUCCEEDED(Result))
    {
        MessageCode = RETCODE_OK;
        for (int32_t i = 0; i < PresetCount; i++)
        {
            SysPresetInfoPtr.GetPtr()[i].s_PtzId = HOST_TO_NETWORK_UINT(SysPresetInfoPtr.GetPtr()[i].s_PtzId);
            SysPresetInfoPtr.GetPtr()[i].s_PresetIndex = HOST_TO_NETWORK_UINT(SysPresetInfoPtr.GetPtr()[i].s_PresetIndex);
        }
    }
    else
    {
        SYS_ERROR("get prest info from system service fail, Result = 0x%lx\n", Result);
    }

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;
    uint16_t AttrCnt;

    if (MessageCode == RETCODE_OK)
    {
        AttrCnt = PresetCount;
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
                 SYSCODE_GET_PTZPRESET_RSP,
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
            memcpy((PayloadBuffer + Offset), &(SysPresetInfoPtr.GetPtr()[Id]), sizeof(SysPkgPtzPresetInfo));
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


GMI_RESULT  SysGetPresetInfoCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}








