#include "sys_set_encode_config_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysSetEncodeConfigCommandExecutor::SysSetEncodeConfigCommandExecutor()
    :  SysBaseCommandExecutor( SYSCODE_SET_ENCODECFG_REQ, CT_ShortTask)
{
}


SysSetEncodeConfigCommandExecutor::~SysSetEncodeConfigCommandExecutor()
{
}


GMI_RESULT SysSetEncodeConfigCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysSetEncodeConfigCommandExecutor> SetEncodeCfgCommandExecutor(BaseMemoryManager::Instance().New<SysSetEncodeConfigCommandExecutor>() );
    if (NULL == SetEncodeCfgCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    if (1 != SysPacket->GetAttrCount())
    {
        return GMI_FAIL;
    }

    SetEncodeCfgCommandExecutor->m_Session = Packet->GetSession();
    SetEncodeCfgCommandExecutor->m_Reply   = Packet->Clone();
    SetEncodeCfgCommandExecutor->SetParameter( m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = SetEncodeCfgCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysSetEncodeConfigCommandExecutor::Execute()
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    int32_t     	  MessageCode     = RETCODE_OK;
    GMI_RESULT        Result          = GMI_SUCCESS;
    SystemPacket     *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();
    uint8_t          *PayloadBuff     = CommandPacket->GetPacketPayloadBuffer();
    SysPkgAttrHeader *SysPkgAttrHdPtr = (SysPkgAttrHeader*)PayloadBuff;

    SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);
    if (SysPkgAttrHdPtr->s_Type == TYPE_ENCODECFG)
    {
        SysPkgEncodeCfg  *SysEncodeCfgPtr = (SysPkgEncodeCfg*)(PayloadBuff + sizeof(SysPkgAttrHeader));
        SysEncodeCfgPtr->s_VideoId        = NETWORK_TO_HOST_UINT(SysEncodeCfgPtr->s_VideoId);
        SysEncodeCfgPtr->s_StreamType     = NETWORK_TO_HOST_UINT(SysEncodeCfgPtr->s_StreamType);
        SysEncodeCfgPtr->s_Compression    = NETWORK_TO_HOST_UINT(SysEncodeCfgPtr->s_Compression);
        SysEncodeCfgPtr->s_PicWidth       = NETWORK_TO_HOST_UINT(SysEncodeCfgPtr->s_PicWidth);
        SysEncodeCfgPtr->s_PicHeight      = NETWORK_TO_HOST_UINT(SysEncodeCfgPtr->s_PicHeight);
        SysEncodeCfgPtr->s_BitrateCtrl    = NETWORK_TO_HOST_UINT(SysEncodeCfgPtr->s_BitrateCtrl);
        SysEncodeCfgPtr->s_Quality        = NETWORK_TO_HOST_UINT(SysEncodeCfgPtr->s_Quality);
        SysEncodeCfgPtr->s_FPS            = NETWORK_TO_HOST_UINT(SysEncodeCfgPtr->s_FPS);
        SysEncodeCfgPtr->s_BitRateAverage = NETWORK_TO_HOST_UINT(SysEncodeCfgPtr->s_BitRateAverage);
        SysEncodeCfgPtr->s_BitRateUp      = NETWORK_TO_HOST_UINT(SysEncodeCfgPtr->s_BitRateUp);
        SysEncodeCfgPtr->s_BitRateDown    = NETWORK_TO_HOST_UINT(SysEncodeCfgPtr->s_BitRateDown);
        SysEncodeCfgPtr->s_Gop            = NETWORK_TO_HOST_UINT(SysEncodeCfgPtr->s_Gop);
        SysEncodeCfgPtr->s_Rotate         = NETWORK_TO_HOST_UINT(SysEncodeCfgPtr->s_Rotate);
        SysEncodeCfgPtr->s_Flag           = NETWORK_TO_HOST_UINT(SysEncodeCfgPtr->s_Flag);
        Result = m_SystemServiceManager->SvrSetVideoEncodeSetting(SysEncodeCfgPtr->s_Flag, SysEncodeCfgPtr);
        if (FAILED(Result))
        {
            SYS_ERROR("SvrSetVideoEncodeSetting fail\n");
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
        return GMI_OUT_OF_MEMORY;
    }

    //fill packet header
    Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if (FAILED(Result))
    {
        SYS_ERROR("AllocatePacketBuffer fail\n");
        return Result;
    }

    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_SET_ENCODECFG_RSP,
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

    SYS_INFO("%s:%s normal out..........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT  SysSetEncodeConfigCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}




