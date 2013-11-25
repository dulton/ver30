#include "sys_get_imaging_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysGetImagingCommandExecutor::SysGetImagingCommandExecutor()
    :  SysBaseCommandExecutor(SYSCODE_GET_IMAGING_REQ, CT_ShortTask)
{
}


SysGetImagingCommandExecutor::~SysGetImagingCommandExecutor()
{
}


GMI_RESULT SysGetImagingCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetImagingCommandExecutor> GetImagingCommandExecutor(BaseMemoryManager::Instance().New<SysGetImagingCommandExecutor>() );
    if (NULL == GetImagingCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetImagingCommandExecutor->m_Session = Packet->GetSession();
    GetImagingCommandExecutor->m_Reply   = Packet->Clone();
    GetImagingCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = GetImagingCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetImagingCommandExecutor::Execute()
{
    SYS_INFO("%s in..........\n", __func__);
    int32_t          MessageCode     = RETCODE_OK;
    SystemPacket    *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();

    ReferrencePtr<SysPkgImaging>SysImagingPtr(BaseMemoryManager::Instance().New<SysPkgImaging>());
    if (NULL == SysImagingPtr.GetPtr())
    {
        SYS_ERROR("SysImagingPtr new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    memset(SysImagingPtr.GetPtr(), 0, sizeof(SysPkgImaging));
    GMI_RESULT Result = m_SystemServiceManager->SvrGetVideoSourceImageSettings(1, SysImagingPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("SvrGetVideoSourceImageSettings fail\n");
        MessageCode = RETCODE_ERROR;
    }
    else
    {
        SysImagingPtr.GetPtr()->s_VideoId    = HOST_TO_NETWORK_UINT(SysImagingPtr.GetPtr()->s_VideoId);
        SysImagingPtr.GetPtr()->s_Brightness = HOST_TO_NETWORK_UINT(SysImagingPtr.GetPtr()->s_Brightness);
        SysImagingPtr.GetPtr()->s_Contrast   = HOST_TO_NETWORK_UINT(SysImagingPtr.GetPtr()->s_Contrast);
        SysImagingPtr.GetPtr()->s_Saturation = HOST_TO_NETWORK_UINT(SysImagingPtr.GetPtr()->s_Saturation);
        SysImagingPtr.GetPtr()->s_Hue        = HOST_TO_NETWORK_UINT(SysImagingPtr.GetPtr()->s_Hue);
        SysImagingPtr.GetPtr()->s_Sharpness  = HOST_TO_NETWORK_UINT(SysImagingPtr.GetPtr()->s_Sharpness);
        SysImagingPtr.GetPtr()->s_Exposure.s_ExposureMode     = HOST_TO_NETWORK_UINT(SysImagingPtr.GetPtr()->s_Exposure.s_ExposureMode);
        SysImagingPtr.GetPtr()->s_Exposure.s_ShutterMax       = HOST_TO_NETWORK_UINT(SysImagingPtr.GetPtr()->s_Exposure.s_ShutterMax);
        SysImagingPtr.GetPtr()->s_Exposure.s_ShutterMin       = HOST_TO_NETWORK_UINT(SysImagingPtr.GetPtr()->s_Exposure.s_ShutterMin);
        SysImagingPtr.GetPtr()->s_Exposure.s_GainMax          = HOST_TO_NETWORK_UINT(SysImagingPtr.GetPtr()->s_Exposure.s_GainMax);
    }

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;
    uint16_t AttrCnt;

    if (MessageCode == 0)
    {
        AttrCnt            = 1;
        PacketAttrHdSize1  = sizeof(SysPkgAttrHeader) + sizeof(SysPkgImaging);
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
        SYS_ERROR("AllocatePacketBuffer fail\n");
        return Result;
    }

    //fill packet header
    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_GET_IMAGING_RSP,
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
    uint16_t Offset        = 0;
    SysPkgAttrHeader *SysPkgAtrrHdPtr;

    if (MessageCode == 0)
    {
        for (int32_t Id = 0; Id < AttrCnt; Id++)
        {
            SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)PayloadBuffer;
            SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_IMAGING);
            SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
            Offset                   += sizeof( SysPkgAttrHeader );
            memcpy((PayloadBuffer + Offset), &((SysImagingPtr.GetPtr())[Id]), sizeof(SysPkgImaging));
            Offset                   += sizeof(SysPkgImaging);
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
        SYS_ERROR("FillPacketSdkTransferProtocolKey fail\n");
        return Result;
    }

    m_Reply = Reply;

    SYS_INFO("%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT  SysGetImagingCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}




