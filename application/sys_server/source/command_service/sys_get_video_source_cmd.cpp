#include "sys_get_video_source_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysGetVideoSourceCommandExecutor::SysGetVideoSourceCommandExecutor()
    :  SysBaseCommandExecutor(SYSCODE_GET_VIDEO_SOURCE_INFO_REQ, CT_ShortTask)
{
}


SysGetVideoSourceCommandExecutor::~SysGetVideoSourceCommandExecutor()
{
}


GMI_RESULT SysGetVideoSourceCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetVideoSourceCommandExecutor> GetVideoSourceCommandExecutor(BaseMemoryManager::Instance().New<SysGetVideoSourceCommandExecutor>() );
    if ( NULL == GetVideoSourceCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetVideoSourceCommandExecutor->m_Session = Packet->GetSession();
    GetVideoSourceCommandExecutor->m_Reply   = Packet->Clone();
    GetVideoSourceCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = GetVideoSourceCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetVideoSourceCommandExecutor::Execute()
{
    SYS_INFO("%s in..........\n", __func__);
    int32_t          MessageCode     = RETCODE_OK;
    SystemPacket    *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();

    ReferrencePtr<SysPkgVideoSource>SysVideoSourcePtr(BaseMemoryManager::Instance().New<SysPkgVideoSource>());
    if (NULL == SysVideoSourcePtr.GetPtr())
    {
        SYS_ERROR("SysVideoSourcePtr fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    GMI_RESULT Result = m_SystemServiceManager->SvrGetVideoSourceSettings(0, SysVideoSourcePtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("SvrGetVideoSourceSettings fail\n");
        MessageCode = RETCODE_ERROR;
    }
    else
    {
        SysVideoSourcePtr.GetPtr()->s_VideoId   = HOST_TO_NETWORK_UINT(SysVideoSourcePtr.GetPtr()->s_VideoId);
        SysVideoSourcePtr.GetPtr()->s_SrcHeight = HOST_TO_NETWORK_UINT(SysVideoSourcePtr.GetPtr()->s_SrcHeight);
        SysVideoSourcePtr.GetPtr()->s_SrcWidth  = HOST_TO_NETWORK_UINT(SysVideoSourcePtr.GetPtr()->s_SrcWidth);
        SysVideoSourcePtr.GetPtr()->s_SrcFps    = HOST_TO_NETWORK_UINT(SysVideoSourcePtr.GetPtr()->s_SrcFps);
        SysVideoSourcePtr.GetPtr()->s_Mirror    = HOST_TO_NETWORK_UINT(SysVideoSourcePtr.GetPtr()->s_Mirror);
    }

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;
    uint16_t AttrCnt;

    if (MessageCode == RETCODE_OK)
    {
        AttrCnt            = 1;
        PacketAttrHdSize1  = sizeof(SysPkgAttrHeader) + sizeof(SysPkgVideoSource);
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
        SYS_ERROR("AllocatePacketBuffer fail\n");
        return Result;
    }

    //fill packet header
    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_GET_VIDEO_SOURCE_INFO_RSP,
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
            SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_VIDEO_SOURCE);
            SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
            Offset                   += sizeof(SysPkgAttrHeader);
            memcpy((PayloadBuffer + Offset), &((SysVideoSourcePtr.GetPtr())[Id]), sizeof(SysPkgVideoSource));
            Offset                   += sizeof(SysPkgVideoSource);
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


GMI_RESULT  SysGetVideoSourceCommandExecutor::Reply()
{
    GMI_RESULT Result = m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}


