#include "sys_ptz_3dctrl_cmd.h"
#include "system_packet.h"
#include "sys_env_types.h"
#include "log.h"


SysPtz3DCtrlCommandExecutor::SysPtz3DCtrlCommandExecutor()
    :  SysBaseCommandExecutor(SYSCODE_3DPTZ_CTL_REQ, CT_ShortTask)
{
}


SysPtz3DCtrlCommandExecutor::~SysPtz3DCtrlCommandExecutor()
{
}


GMI_RESULT  SysPtz3DCtrlCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysPtz3DCtrlCommandExecutor>Ptz3DCtrlCommandExecutor( BaseMemoryManager::Instance().New<SysPtz3DCtrlCommandExecutor>() );
    if (NULL == Ptz3DCtrlCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    if (1 != SysPacket->GetAttrCount())
    {
        return GMI_FAIL;
    }

    Ptz3DCtrlCommandExecutor->m_Session = Packet->GetSession();
    Ptz3DCtrlCommandExecutor->m_Reply   = Packet->Clone();
    Ptz3DCtrlCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = Ptz3DCtrlCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysPtz3DCtrlCommandExecutor::Execute()
{
    int32_t       MessageCode         = RETCODE_OK;
    GMI_RESULT    Result              = GMI_SUCCESS;
    SystemPacket *CommandPacket       = (SystemPacket*) m_Reply.GetPtr();
    uint8_t      *PayloadBuff         = CommandPacket->GetPacketPayloadBuffer();
    SysPkgAttrHeader *SysPkgAttrHdPtr = (SysPkgAttrHeader*)PayloadBuff;


    SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);
    if (SysPkgAttrHdPtr->s_Type == TYPE_3DCTLPTZ)
    {
        SysPkgPtz3Dctrl *SysPtz3Dctrl = (SysPkgPtz3Dctrl*)(PayloadBuff + sizeof(SysPkgAttrHeader));
        SysPtz3Dctrl->s_PtzId    = NETWORK_TO_HOST_UINT(SysPtz3Dctrl->s_PtzId);        
        SysPtz3Dctrl->s_X        = NETWORK_TO_HOST_UINT(SysPtz3Dctrl->s_X);
        SysPtz3Dctrl->s_Y        = NETWORK_TO_HOST_UINT(SysPtz3Dctrl->s_Y);
        SysPtz3Dctrl->s_Width    = NETWORK_TO_HOST_UINT(SysPtz3Dctrl->s_Width);
        SysPtz3Dctrl->s_Height   = NETWORK_TO_HOST_UINT(SysPtz3Dctrl->s_Height);       
        //Result = m_SystemServiceManager->SvrPtzControl(SysPtz3Dctrl);
        //if (FAILED(Result))
        //{
        //    SYS_ERROR("SvrPtzControl fail, Result = 0x%lx\n", Result);
        //    MessageCode = RETCODE_ERROR;
        //}
    }
    else
    {
        SYS_ERROR("SysPkgAttrHdPtr->s_Type %d incorrect\n", SysPkgAttrHdPtr->s_Type);
        MessageCode = RETCODE_ERROR;
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
                 SYSCODE_3DPTZ_CTL_RSP,
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


GMI_RESULT  SysPtz3DCtrlCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}



