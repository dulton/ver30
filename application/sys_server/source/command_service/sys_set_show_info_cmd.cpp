#include "sys_set_show_info_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysSetShowInfoCommandExecutor::SysSetShowInfoCommandExecutor()
    :  SysBaseCommandExecutor(SYSCODE_SET_SHOWCFG_REQ, CT_ShortTask)
{
}


SysSetShowInfoCommandExecutor::~SysSetShowInfoCommandExecutor()
{
}


GMI_RESULT SysSetShowInfoCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysSetShowInfoCommandExecutor> SetShowInfoCommandExecutor(BaseMemoryManager::Instance().New<SysSetShowInfoCommandExecutor>() );
    if (NULL == SetShowInfoCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    if (1 != SysPacket->GetAttrCount())
    {
        return GMI_FAIL;
    }

    SetShowInfoCommandExecutor->m_Session = Packet->GetSession();
    SetShowInfoCommandExecutor->m_Reply   = Packet->Clone();
    SetShowInfoCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = SetShowInfoCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysSetShowInfoCommandExecutor::Execute()
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    int32_t     	  MessageCode     = RETCODE_OK;
    GMI_RESULT        Result          = GMI_SUCCESS;
    SystemPacket     *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();
    uint8_t          *PayloadBuff     = CommandPacket->GetPacketPayloadBuffer();
    SysPkgAttrHeader *SysPkgAttrHdPtr = (SysPkgAttrHeader*)PayloadBuff;

    SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);
    if (SysPkgAttrHdPtr->s_Type == TYPE_SHOWCFG)
    {
        SysPkgShowCfg *SysShowCfgPtr = (SysPkgShowCfg*)(PayloadBuff + sizeof(SysPkgAttrHeader));
        SysShowCfgPtr->s_VideoId                   = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_VideoId);
        SysShowCfgPtr->s_Flag                      = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_Flag);
        SysShowCfgPtr->s_TimeInfo.s_Enable         = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_TimeInfo.s_Enable);
        SysShowCfgPtr->s_TimeInfo.s_Language       = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_TimeInfo.s_Language);
        SysShowCfgPtr->s_TimeInfo.s_DisplayX       = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_TimeInfo.s_DisplayX);
        SysShowCfgPtr->s_TimeInfo.s_DisplayY       = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_TimeInfo.s_DisplayY);
        SysShowCfgPtr->s_TimeInfo.s_DateStyle      = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_TimeInfo.s_DateStyle);
        SysShowCfgPtr->s_TimeInfo.s_TimeStyle      = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_TimeInfo.s_TimeStyle);
        SysShowCfgPtr->s_TimeInfo.s_FontColor      = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_TimeInfo.s_FontColor);
        SysShowCfgPtr->s_TimeInfo.s_FontSize       = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_TimeInfo.s_FontSize);
        SysShowCfgPtr->s_TimeInfo.s_FontBlod       = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_TimeInfo.s_FontBlod);
        SysShowCfgPtr->s_TimeInfo.s_FontRotate     = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_TimeInfo.s_FontRotate);
        SysShowCfgPtr->s_TimeInfo.s_FontItalic     = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_TimeInfo.s_FontItalic);
        SysShowCfgPtr->s_TimeInfo.s_FontOutline    = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_TimeInfo.s_FontOutline);
        SysShowCfgPtr->s_ChannelInfo.s_Enable      = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_ChannelInfo.s_Enable);
        SysShowCfgPtr->s_ChannelInfo.s_DisplayX    = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_ChannelInfo.s_DisplayX);
        SysShowCfgPtr->s_ChannelInfo.s_DisplayY    = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_ChannelInfo.s_DisplayY);
        SysShowCfgPtr->s_ChannelInfo.s_FontColor   = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_ChannelInfo.s_FontColor);
        SysShowCfgPtr->s_ChannelInfo.s_FontSize    = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_ChannelInfo.s_FontSize);
        SysShowCfgPtr->s_ChannelInfo.s_FontBlod    = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_ChannelInfo.s_FontBlod );
        SysShowCfgPtr->s_ChannelInfo.s_FontRotate  = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_ChannelInfo.s_FontRotate);
        SysShowCfgPtr->s_ChannelInfo.s_FontItalic  = NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_ChannelInfo.s_FontItalic);
        SysShowCfgPtr->s_ChannelInfo.s_FontOutline =  NETWORK_TO_HOST_UINT(SysShowCfgPtr->s_ChannelInfo.s_FontOutline);
        Result = m_SystemServiceManager->SvrSetShowCfg(SysShowCfgPtr->s_Flag, SysShowCfgPtr);
        if (FAILED(Result))
        {
            SYS_ERROR("SvrSetShowCfg fail, Result = 0x%lx\n", Result);
            MessageCode = RETCODE_ERROR;
        }
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

    ReferrencePtr<SystemPacket> Reply( BaseMemoryManager::Instance().New<SystemPacket>() );
    if (NULL == Reply.GetPtr())
    {
        SYS_ERROR("Reply new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    //fill packet header
    Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if (FAILED(Result))
    {
        SYS_ERROR("AllocatePacketBuffer fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_SET_SHOWCFG_RSP,
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


GMI_RESULT  SysSetShowInfoCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}

