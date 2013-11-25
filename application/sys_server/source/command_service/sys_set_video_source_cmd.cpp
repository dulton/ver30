#include "log.h"
#include "sys_set_video_source_cmd.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysSetVideoSourceCommandExecutor::SysSetVideoSourceCommandExecutor()
    :  SysBaseCommandExecutor(SYSCODE_SET_VIDEO_SOURCE_INFO_REQ, CT_ShortTask)
{
}


SysSetVideoSourceCommandExecutor::~SysSetVideoSourceCommandExecutor()
{
}


GMI_RESULT  SysSetVideoSourceCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysSetVideoSourceCommandExecutor> SetVideoSourceCommandExecutor( BaseMemoryManager::Instance().New<SysSetVideoSourceCommandExecutor>() );
    if (NULL == SetVideoSourceCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    SetVideoSourceCommandExecutor->m_Session = Packet->GetSession();
    SetVideoSourceCommandExecutor->m_Reply = Packet->Clone();
    SetVideoSourceCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = SetVideoSourceCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysSetVideoSourceCommandExecutor::Execute()
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    int32_t       MessageCode   = RETCODE_OK;
    GMI_RESULT    Result        = GMI_SUCCESS;
    SystemPacket *CommandPacket = (SystemPacket*) m_Reply.GetPtr();
    uint8_t      *PayloadBuff   = CommandPacket->GetPacketPayloadBuffer();
    SysPkgAttrHeader *SysPkgAttrHdPtr = (SysPkgAttrHeader*)PayloadBuff;
    //uint32_t      AuthValue     = CommandPacket->GetSdkTransferProtocolAuthValue();

    do
    {
        if (1 != CommandPacket->GetAttrCount())
        {
            MessageCode = RETCODE_ERROR;
            SYS_ERROR("ReqAttrCnt %d Error\n", CommandPacket->GetAttrCount());
            break;
        }

        SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);
        if (SysPkgAttrHdPtr->s_Type != TYPE_VIDEO_SOURCE)
        {
            SYS_ERROR("SysPkgAttrHdPtr->s_Type %d incorrect\n", SysPkgAttrHdPtr->s_Type);
            MessageCode = RETCODE_ERROR;
            break;
        }

        SysPkgVideoSource *SysVideoSourcePtr = (SysPkgVideoSource*)(PayloadBuff + sizeof(SysPkgAttrHeader));
        SysPkgVideoSource  SysVideoSource;

        memset(&SysVideoSource, 0, sizeof(SysPkgVideoSource));
        memcpy(&SysVideoSource, SysVideoSourcePtr, sizeof(SysPkgVideoSource));
        SysVideoSource.s_VideoId   = NETWORK_TO_HOST_UINT(SysVideoSource.s_VideoId);
        SysVideoSource.s_SrcHeight = NETWORK_TO_HOST_UINT(SysVideoSource.s_SrcHeight);
        SysVideoSource.s_SrcWidth  = NETWORK_TO_HOST_UINT(SysVideoSource.s_SrcWidth);
        SysVideoSource.s_Mirror    = NETWORK_TO_HOST_UINT(SysVideoSource.s_Mirror);
        SysVideoSource.s_SrcFps    = NETWORK_TO_HOST_UINT(SysVideoSource.s_SrcFps);

        Result = m_SystemServiceManager->SvrSetVideoSourceSettings(1, &SysVideoSource);
        if (FAILED(Result))
        {
            MessageCode = RETCODE_ERROR;
            SYS_ERROR("SvrSetVideoSourceSettings fail, Result = 0x%lx\n", Result);
            break;
        }
    }
    while(0);

    // Reply
    uint16_t PacketHeaderSize  = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    uint16_t PayloadTotalSize  = PacketAttrHdSize1+ sizeof(SdkPkgTransferProtocolKey);
    uint16_t AppPacketSize     = PacketHeaderSize + PacketAttrHdSize1;

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
                 SYSCODE_SET_VIDEO_SOURCE_INFO_RSP,
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
    uint16_t Offset        = 0;
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


GMI_RESULT  SysSetVideoSourceCommandExecutor::Reply()
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    GMI_RESULT Result =  m_Reply->Submit( m_Session );
    if (FAILED(Result))
    {
        return Result;
    }
    SYS_INFO("%s:%s normal out..........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}




