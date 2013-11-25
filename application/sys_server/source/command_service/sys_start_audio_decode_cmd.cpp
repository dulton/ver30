#include "sys_start_audio_decode_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"
#include "application_packet_header.h"

SysStartAudioDecodeCommandExecutor::SysStartAudioDecodeCommandExecutor(void)
    : SysBaseCommandExecutor(SYSCODE_START_AUDIO_DECODE_REQ, CT_ShortTask )
{
}


SysStartAudioDecodeCommandExecutor::~SysStartAudioDecodeCommandExecutor(void)
{
}


GMI_RESULT  SysStartAudioDecodeCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor)
{
    SystemPacket *SysPacket = (SystemPacket*)Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysStartAudioDecodeCommandExecutor> StartAudioDecodeCommandExecutor( BaseMemoryManager::Instance().New<SysStartAudioDecodeCommandExecutor>() );
    if (NULL == StartAudioDecodeCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    StartAudioDecodeCommandExecutor->m_Session = Packet->GetSession();
    StartAudioDecodeCommandExecutor->m_Reply = Packet->Clone();
    GMI_RESULT Result = StartAudioDecodeCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);
    if (FAILED(Result))
    {
        return Result;
    }

    CommandExecutor = StartAudioDecodeCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysStartAudioDecodeCommandExecutor::Execute()
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
        if (SysPkgAttrHdPtr->s_Type != TYPE_AUDIO_DECODE)
        {
            SYS_ERROR("SysPkgAttrHdPtr->s_Type %d incorrect\n", SysPkgAttrHdPtr->s_Type);
            MessageCode = RETCODE_ERROR;
            break;
        }

        SysPkgAudioDecParam *SysAudioDecParamPtr = (SysPkgAudioDecParam*)(PayloadBuff + sizeof(SysPkgAttrHeader));
        SysPkgAudioDecParam  SysAudioDecParam;

        memset(&SysAudioDecParam, 0, sizeof(SysPkgAudioDecParam));
        memcpy(&SysAudioDecParam, SysAudioDecParamPtr, sizeof(SysPkgAudioDecParam));
        SysAudioDecParam.s_AudioId      = NETWORK_TO_HOST_UINT(SysAudioDecParam.s_AudioId);
        SysAudioDecParam.s_AecDelayTime = NETWORK_TO_HOST_USHORT(SysAudioDecParam.s_AecDelayTime);
        SysAudioDecParam.s_AecFlag      = NETWORK_TO_HOST_USHORT(SysAudioDecParam.s_AecFlag);
        SysAudioDecParam.s_BitRate      = NETWORK_TO_HOST_USHORT(SysAudioDecParam.s_BitRate);
        SysAudioDecParam.s_BitWidth     = NETWORK_TO_HOST_UINT(SysAudioDecParam.s_BitWidth);
        SysAudioDecParam.s_ChannelNum   = NETWORK_TO_HOST_USHORT(SysAudioDecParam.s_ChannelNum);
        SysAudioDecParam.s_Codec        = NETWORK_TO_HOST_UINT(SysAudioDecParam.s_Codec);
        SysAudioDecParam.s_FrameRate    = NETWORK_TO_HOST_USHORT(SysAudioDecParam.s_FrameRate);
        SysAudioDecParam.s_SampleFreq   = NETWORK_TO_HOST_UINT(SysAudioDecParam.s_SampleFreq);
        SysAudioDecParam.s_Volume       = NETWORK_TO_HOST_USHORT(SysAudioDecParam.s_Volume);

        Result = m_SystemServiceManager->SvrStartAudioDecode(SysAudioDecParam.s_AudioId, &SysAudioDecParam);
        if (FAILED(Result))
        {
            MessageCode = RETCODE_ERROR;
            SYS_ERROR("sever start audio decode fail, Result = 0x%lx\n", Result);
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
                 SYSCODE_START_AUDIO_DECODE_RSP,
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
        SYS_ERROR("FillPacketSdkTransferProtocolKey fail, Result = 0x%lx\n", Result);
        return Result;
    }

    m_Reply = Reply;

    SYS_INFO("%s:%s normal out..........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT  SysStartAudioDecodeCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}




