#include "sys_set_imaging_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysSetImagingCommandExecutor::SysSetImagingCommandExecutor()
    :  SysBaseCommandExecutor( SYSCODE_SET_IMAGING_REQ, CT_ShortTask)
{
}


SysSetImagingCommandExecutor::~SysSetImagingCommandExecutor()
{
}


GMI_RESULT SysSetImagingCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysSetImagingCommandExecutor> SetImagingCommandExecutor(BaseMemoryManager::Instance().New<SysSetImagingCommandExecutor>() );
    if (NULL == SetImagingCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    if (1 != SysPacket->GetAttrCount())
    {
        return GMI_FAIL;
    }

    SetImagingCommandExecutor->m_Session = Packet->GetSession();
    SetImagingCommandExecutor->m_Reply   = Packet->Clone();
    SetImagingCommandExecutor->SetParameter( m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = SetImagingCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysSetImagingCommandExecutor::Execute()
{
    SYS_INFO("%s in..........\n", __func__);
    int32_t     	  MessageCode     = RETCODE_OK;
    GMI_RESULT        Result          = GMI_SUCCESS;
    SystemPacket     *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();
    uint8_t          *PayloadBuff     = CommandPacket->GetPacketPayloadBuffer();
    SysPkgAttrHeader *SysPkgAttrHdPtr = (SysPkgAttrHeader*)PayloadBuff;

    SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);
    if (SysPkgAttrHdPtr->s_Type == TYPE_IMAGING)
    {
        SysPkgImaging  *SysImagingPtr = (SysPkgImaging*)(PayloadBuff + sizeof(SysPkgAttrHeader));
        SysImagingPtr->s_VideoId    = NETWORK_TO_HOST_UINT(SysImagingPtr->s_VideoId);
        SysImagingPtr->s_Brightness = NETWORK_TO_HOST_UINT(SysImagingPtr->s_Brightness);
        SysImagingPtr->s_Contrast   = NETWORK_TO_HOST_UINT(SysImagingPtr->s_Contrast);
        SysImagingPtr->s_Saturation = NETWORK_TO_HOST_UINT(SysImagingPtr->s_Saturation);
        SysImagingPtr->s_Hue        = NETWORK_TO_HOST_UINT(SysImagingPtr->s_Hue);
        SysImagingPtr->s_Sharpness  = NETWORK_TO_HOST_UINT(SysImagingPtr->s_Sharpness);
        SysImagingPtr->s_Exposure.s_ExposureMode     = NETWORK_TO_HOST_UINT(SysImagingPtr->s_Exposure.s_ExposureMode);
        SysImagingPtr->s_Exposure.s_ShutterMax       = NETWORK_TO_HOST_UINT(SysImagingPtr->s_Exposure.s_ShutterMax);
        SysImagingPtr->s_Exposure.s_ShutterMin       = NETWORK_TO_HOST_UINT(SysImagingPtr->s_Exposure.s_ShutterMin);
        SysImagingPtr->s_Exposure.s_GainMax          = NETWORK_TO_HOST_UINT(SysImagingPtr->s_Exposure.s_GainMax);

        Result = m_SystemServiceManager->SvrSetVideoSourceImageSettings(SysImagingPtr->s_VideoId, SysImagingPtr);
        if ( FAILED(Result) )
        {
            SYS_ERROR("SvrSetVideoSourceImageSettings fail\n");
            MessageCode = -1;
        }
    }
    else
    {
        MessageCode = RETCODE_ERROR;
        SYS_ERROR("SysPkgAttrHdPtr->s_Type %d incorrect\n", SysPkgAttrHdPtr->s_Type);
    }

    // Reply
    uint16_t PacketHeaderSize  = sizeof( SysPkgHeader );
    uint16_t PacketAttrHdSize1 = sizeof( SysPkgAttrHeader ) + sizeof( SysPkgMessageCode );
    uint16_t PayloadTotalSize  = PacketAttrHdSize1 + sizeof(SdkPkgTransferProtocolKey);
    uint16_t AppPacketSize     = PacketHeaderSize + PacketAttrHdSize1;

    ReferrencePtr<SystemPacket> Reply( BaseMemoryManager::Instance().New<SystemPacket>() );
    if ( NULL == Reply.GetPtr() )
    {
        SYS_ERROR("Reply new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if ( FAILED( Result ) )
    {
        SYS_ERROR("AllocatePacketBuffer fail\n");
        return Result;
    }

    //fill packet header
    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_SET_IMAGING_RSP,
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

    SYS_INFO("%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT  SysSetImagingCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}


