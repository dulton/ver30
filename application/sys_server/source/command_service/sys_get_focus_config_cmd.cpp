#include "sys_get_focus_config_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"
#include "application_packet_header.h"

SysGetFocusConfigCommandExecutor::SysGetFocusConfigCommandExecutor(void)
    : SysBaseCommandExecutor(SYSCODE_GET_AUTOFOCUS_REQ, CT_ShortTask )
{
}


SysGetFocusConfigCommandExecutor::~SysGetFocusConfigCommandExecutor(void)
{
}


GMI_RESULT  SysGetFocusConfigCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetFocusConfigCommandExecutor> GetFocusConfigCommandExecutor( BaseMemoryManager::Instance().New<SysGetFocusConfigCommandExecutor>() );
    if (NULL == GetFocusConfigCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetFocusConfigCommandExecutor->m_Session = Packet->GetSession();
    GetFocusConfigCommandExecutor->m_Reply = Packet->Clone();
    GMI_RESULT Result = GetFocusConfigCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);
    if (FAILED(Result))
    {
        return Result;
    }

    CommandExecutor = GetFocusConfigCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetFocusConfigCommandExecutor::Execute()
{
    int32_t       MessageCode   = RETCODE_ERROR;
    GMI_RESULT    Result        = GMI_SUCCESS;
    SystemPacket *CommandPacket = (SystemPacket*)m_Reply.GetPtr();
    SysPkgAutoFocus SysAutoFocusTmp;
    //uint32_t      AuthValue = CommandPacket->GetSdkTransferProtocolAuthValue();

    Result = m_SystemServiceManager->SvrGetAutoFocus(&SysAutoFocusTmp);
    if (SUCCEEDED(Result))
    {
        MessageCode = RETCODE_OK;
        SysAutoFocusTmp.s_FocusMode = HOST_TO_NETWORK_UINT(SysAutoFocusTmp.s_FocusMode);
    }

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;

    if (MessageCode == RETCODE_OK)
    {
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgAutoFocus);
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
                 SYSCODE_GET_AUTOFOCUS_RSP,
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
        SysPkgAutoFocus *SysAutoFocusPtr;

        SysPkgAtrrHdPtr = (SysPkgAttrHeader*)PayloadBuffer;
        SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_AUTO_FOCUS);
        SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
        Offset                   += sizeof(SysPkgAttrHeader);
        SysAutoFocusPtr           = (SysPkgAutoFocus*)(PayloadBuffer + Offset);
        memcpy((void_t*)SysAutoFocusPtr, (void_t*)&SysAutoFocusTmp,  sizeof(SysPkgAutoFocus));
        Offset                   += sizeof(SysPkgAutoFocus);
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


GMI_RESULT  SysGetFocusConfigCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}




