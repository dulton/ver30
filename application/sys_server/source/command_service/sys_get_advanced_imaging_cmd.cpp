
#include "sys_get_advanced_imaging_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysGetAdvancedImagingCommandExecutor::SysGetAdvancedImagingCommandExecutor()
    :  SysBaseCommandExecutor(SYSCODE_GET_ADVANCED_IMAGING_REQ, CT_ShortTask)
{
}


SysGetAdvancedImagingCommandExecutor::~SysGetAdvancedImagingCommandExecutor()
{
}


GMI_RESULT SysGetAdvancedImagingCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetAdvancedImagingCommandExecutor> GetAdvancedImagingCommandExecutor(BaseMemoryManager::Instance().New<SysGetAdvancedImagingCommandExecutor>() );
    if (NULL == GetAdvancedImagingCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetAdvancedImagingCommandExecutor->m_Session = Packet->GetSession();
    GetAdvancedImagingCommandExecutor->m_Reply   = Packet->Clone();
    GetAdvancedImagingCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = GetAdvancedImagingCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetAdvancedImagingCommandExecutor::Execute()
{
    SYS_INFO("%s in..........\n", __func__);
    int32_t          MessageCode     = RETCODE_OK;
    GMI_RESULT       Result          = GMI_SUCCESS;
    SystemPacket    *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();

    ReferrencePtr<SysPkgAdvancedImaging>SysAdvancedImagingPtr(BaseMemoryManager::Instance().New<SysPkgAdvancedImaging>());
    if (NULL == SysAdvancedImagingPtr.GetPtr())
    {
        SYS_ERROR("SysImagingPtr new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    do
    {
        memset(SysAdvancedImagingPtr.GetPtr(), 0, sizeof(SysPkgAdvancedImaging));
        Result = m_SystemServiceManager->SvrGetAdvancedImageSettings(1, SysAdvancedImagingPtr.GetPtr());
        if (FAILED(Result))
        {
            SYS_ERROR("SvrGetVideoSourceImageSettings fail, Result = 0x%lx\n", Result);
            MessageCode = RETCODE_ERROR;
            break;
        }
        else
        {
            SysAdvancedImagingPtr.GetPtr()->s_VideoId           = HOST_TO_NETWORK_UINT(SysAdvancedImagingPtr.GetPtr()->s_VideoId);
            SysAdvancedImagingPtr.GetPtr()->s_LocalExposure     = HOST_TO_NETWORK_USHORT(SysAdvancedImagingPtr.GetPtr()->s_LocalExposure);
            SysAdvancedImagingPtr.GetPtr()->s_MctfStrength      = HOST_TO_NETWORK_USHORT(SysAdvancedImagingPtr.GetPtr()->s_MctfStrength);
            SysAdvancedImagingPtr.GetPtr()->s_DcIrisDuty        = HOST_TO_NETWORK_USHORT(SysAdvancedImagingPtr.GetPtr()->s_DcIrisDuty);
            SysAdvancedImagingPtr.GetPtr()->s_AeTargetRatio     = HOST_TO_NETWORK_USHORT(SysAdvancedImagingPtr.GetPtr()->s_AeTargetRatio);
        }
    }
    while(0);

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;
    uint16_t AttrCnt;

    if (MessageCode == 0)
    {
        AttrCnt            = 1;
        PacketAttrHdSize1  = sizeof(SysPkgAttrHeader) + sizeof(SysPkgAdvancedImaging);
    }
    else
    {
        AttrCnt           = 1;
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    }
    AppPacketSize    = PacketHeaderSize + PacketAttrHdSize1*AttrCnt;
    PayloadTotalSize = PacketAttrHdSize1*AttrCnt + sizeof(SdkPkgTransferProtocolKey);

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
                 SYSCODE_GET_ADVANCED_IMAGING_RSP,
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
            SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)PayloadBuffer;
            SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_ADVANCED_IMAGING);
            SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
            Offset                   += sizeof( SysPkgAttrHeader );
            memcpy((PayloadBuffer + Offset), &((SysAdvancedImagingPtr.GetPtr())[Id]), sizeof(SysPkgAdvancedImaging));
            Offset                   += sizeof(SysPkgAdvancedImaging);
        }
    }
    else
    {
        SysPkgMessageCode *SysPkgMessageCodePtr;

        SysPkgAtrrHdPtr                     = (SysPkgAttrHeader*)(PayloadBuffer + Offset);
        SysPkgAtrrHdPtr->s_Type             = HOST_TO_NETWORK_USHORT(TYPE_MESSAGECODE);
        SysPkgAtrrHdPtr->s_Length           = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
        Offset                             += sizeof(SysPkgAttrHeader);
        SysPkgMessageCodePtr                = (SysPkgMessageCode*)(PayloadBuffer + Offset);
        SysPkgMessageCodePtr->s_MessageCode = HOST_TO_NETWORK_UINT(MessageCode);
        SysPkgMessageCodePtr->s_MessageLen  = 0;
        Offset                             += sizeof(SysPkgMessageCode);
        Offset                             += SysPkgMessageCodePtr->s_MessageLen;
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
        SYS_ERROR("FillPacketSdkTransferProtocolKey fail, Result = 0x%lx\n", Result);
        return Result;
    }

    m_Reply = Reply;

    SYS_INFO("%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT  SysGetAdvancedImagingCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}


