#if 1
#include "sys_get_show_info_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysGetShowInfoCommandExecutor::SysGetShowInfoCommandExecutor()
    :  SysBaseCommandExecutor(SYSCODE_GET_SHOWCFG_REQ, CT_ShortTask)
{
}


SysGetShowInfoCommandExecutor::~SysGetShowInfoCommandExecutor()
{
}


GMI_RESULT SysGetShowInfoCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetShowInfoCommandExecutor> GetShowInfoCommandExecutor(BaseMemoryManager::Instance().New<SysGetShowInfoCommandExecutor>() );
    if (NULL == GetShowInfoCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetShowInfoCommandExecutor->m_Session = Packet->GetSession();
    GetShowInfoCommandExecutor->m_Reply = Packet->Clone();
    GetShowInfoCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = GetShowInfoCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetShowInfoCommandExecutor::Execute()
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    int32_t          MessageCode     = RETCODE_OK;
    int32_t          StreamNum       = 0;
    SystemPacket    *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();

    GMI_RESULT Result = m_SystemServiceManager->SvrGetVideoStreamNum(&StreamNum);
    if (FAILED(Result))
    {
        SYS_ERROR("SvrGetVideoStreamNum fail\n");
        MessageCode = RETCODE_ERROR;
    }

    ReferrencePtr<SysPkgShowCfg, DefaultObjectsDeleter> SysShowCfgPtr(BaseMemoryManager::Instance().News<SysPkgShowCfg>(StreamNum));
    if (NULL == SysShowCfgPtr.GetPtr())
    {
        SYS_ERROR("SysShowCfgPtr news fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    Result = m_SystemServiceManager->SvrGetShowCfg(0xff, SysShowCfgPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("SvrGetShowCfg fail, Result = 0x%lx\n", Result);
        MessageCode = RETCODE_ERROR;
    }
    else
    {
        for (int32_t Id = 0; Id < StreamNum; Id++)
        {
            (SysShowCfgPtr.GetPtr())[Id].s_VideoId                 = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_VideoId);
            (SysShowCfgPtr.GetPtr())[Id].s_Flag                    = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_Flag);
            (SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_Enable       = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_Enable);
            (SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_Language     = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_Language);
            (SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_DisplayX     = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_DisplayX);
            (SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_DisplayY     = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_DisplayY);
            (SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_DateStyle    = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_DateStyle);
            (SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_TimeStyle    = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_TimeStyle);
            (SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_FontColor    = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_FontColor);
            (SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_FontSize     = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_FontSize);
            (SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_FontBlod     = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_FontBlod );
            (SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_FontRotate   = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_FontRotate);
            (SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_FontItalic   = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_FontItalic);
            (SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_FontOutline  = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_TimeInfo.s_FontOutline );
            (SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_Enable    = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_Enable);
            (SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_DisplayX  = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_DisplayX);
            (SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_DisplayY  = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_DisplayY);
            (SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_FontColor = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_FontColor);
            (SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_FontSize  = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_FontSize);
            (SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_FontBlod  = HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_FontBlod);
            (SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_FontRotate= HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_FontRotate);
            (SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_FontItalic= HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_FontItalic);
            (SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_FontOutline=  HOST_TO_NETWORK_UINT((SysShowCfgPtr.GetPtr())[Id].s_ChannelInfo.s_FontOutline);
        }
    }

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;
    uint16_t AttrCnt;

    if (MessageCode == RETCODE_OK)
    {
        AttrCnt            = StreamNum;
        PacketAttrHdSize1  = sizeof(SysPkgAttrHeader) + sizeof(SysPkgShowCfg);
    }
    else
    {
        AttrCnt           = 1;
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    }
    AppPacketSize    = PacketHeaderSize + PacketAttrHdSize1*AttrCnt;
    PayloadTotalSize = PacketAttrHdSize1*AttrCnt + sizeof(SdkPkgTransferProtocolKey);

    ReferrencePtr<SystemPacket> Reply( BaseMemoryManager::Instance().New<SystemPacket>() );
    if (NULL == Reply.GetPtr())
    {
        SYS_ERROR("Reply new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if (FAILED(Result))
    {
        SYS_ERROR("PakcetBuffer allocate fail\n");
        return Result;
    }

    //fill packet header
    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_GET_SHOWCFG_RSP,
                 AttrCnt,
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
    uint8_t *PayloadBuffer = Reply->GetPacketPayloadBuffer();
    uint16_t Offset        = 0;
    SysPkgAttrHeader *SysPkgAtrrHdPtr;

    if (MessageCode == 0)
    {
        for (int32_t Id = 0; Id < AttrCnt; Id++)
        {
            SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)(PayloadBuffer + Offset);
            SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_SHOWCFG);
            SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
            Offset                   += sizeof(SysPkgAttrHeader);
            memcpy((PayloadBuffer + Offset), &((SysShowCfgPtr.GetPtr())[Id]), sizeof(SysPkgShowCfg));
            Offset                   += sizeof(SysPkgShowCfg);
        }
    }
    else
    {
        SysPkgMessageCode *SysPkgMessageCodePtr;

        SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)PayloadBuffer;
        SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_MESSAGECODE);
        SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
        Offset                   += sizeof( SysPkgAttrHeader );
        SysPkgMessageCodePtr                = (SysPkgMessageCode*)(PayloadBuffer + Offset);
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

    SYS_INFO("%s:%s normal out..........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT  SysGetShowInfoCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}




#endif

