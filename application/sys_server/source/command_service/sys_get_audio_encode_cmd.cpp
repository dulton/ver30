#include "sys_get_audio_encode_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"
#include "application_packet_header.h"

SysGetAudioEncodeCommandExecutor::SysGetAudioEncodeCommandExecutor(void)
    : SysBaseCommandExecutor(SYSCODE_GET_AUDIO_ENCODE_REQ, CT_ShortTask )
{
}


SysGetAudioEncodeCommandExecutor::~SysGetAudioEncodeCommandExecutor(void)
{
}


GMI_RESULT  SysGetAudioEncodeCommandExecutor::Create(ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetAudioEncodeCommandExecutor> GetAudioEncodeCommandExecutor( BaseMemoryManager::Instance().New<SysGetAudioEncodeCommandExecutor>() );
    if (NULL == GetAudioEncodeCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetAudioEncodeCommandExecutor->m_Session = Packet->GetSession();
    GetAudioEncodeCommandExecutor->m_Reply = Packet->Clone();
    GMI_RESULT Result = GetAudioEncodeCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);
    if (FAILED(Result))
    {
        return Result;
    }

    CommandExecutor = GetAudioEncodeCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT SysGetAudioEncodeCommandExecutor::Execute()
{
    int32_t       MessageCode   = RETCODE_ERROR;
    GMI_RESULT    Result        = GMI_SUCCESS;
    SystemPacket *CommandPacket = (SystemPacket*)m_Reply.GetPtr();
    SysPkgAudioEncodeCfg SysAudioEncodeCfg;
    //uint32_t      AuthValue = CommandPacket->GetSdkTransferProtocolAuthValue();

    Result = m_SystemServiceManager->SvrGetAudioEncodeSetting(1, &SysAudioEncodeCfg);
    if (SUCCEEDED(Result))
    {
        MessageCode = RETCODE_OK;
        SysAudioEncodeCfg.s_AudioId       = HOST_TO_NETWORK_UINT(SysAudioEncodeCfg.s_AudioId );
        SysAudioEncodeCfg.s_SamplesPerSec = HOST_TO_NETWORK_UINT(SysAudioEncodeCfg.s_SamplesPerSec);
        SysAudioEncodeCfg.s_CapVolume     = HOST_TO_NETWORK_USHORT(SysAudioEncodeCfg.s_CapVolume);
        SysAudioEncodeCfg.s_PlayVolume     = HOST_TO_NETWORK_USHORT(SysAudioEncodeCfg.s_PlayVolume);
        SysAudioEncodeCfg.s_AecDelayTime  = HOST_TO_NETWORK_USHORT(SysAudioEncodeCfg.s_AecDelayTime);
        SysAudioEncodeCfg.s_BitRate       = HOST_TO_NETWORK_USHORT(SysAudioEncodeCfg.s_BitRate);
        SysAudioEncodeCfg.s_FrameRate     = HOST_TO_NETWORK_USHORT(SysAudioEncodeCfg.s_FrameRate);
    }

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;

    if (MessageCode == RETCODE_OK)
    {
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgAudioEncodeCfg);
    }
    else
    {
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    }
    PayloadTotalSize = PacketAttrHdSize1 + sizeof(SdkPkgTransferProtocolKey);
    AppPacketSize    = PacketHeaderSize  + PacketAttrHdSize1;
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
                 SYSCODE_GET_AUDIO_ENCODE_RSP,
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
    uint16_t Offset = 0;
    SysPkgAttrHeader *SysPkgAtrrHdPtr;

    if (MessageCode == RETCODE_OK)
    {
        SysPkgAudioEncodeCfg *SysAudioEncodeCfgPtr;

        SysPkgAtrrHdPtr = (SysPkgAttrHeader*)PayloadBuffer;
        SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_AUDIO_ENCODECFG);
        SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
        Offset                   += sizeof(SysPkgAttrHeader);
        SysAudioEncodeCfgPtr         = (SysPkgAudioEncodeCfg*)(PayloadBuffer + Offset);
        memcpy((void_t*)SysAudioEncodeCfgPtr, (void_t*)&SysAudioEncodeCfg,  sizeof(SysPkgAudioEncodeCfg));
        Offset                   += sizeof(SysPkgAudioEncodeCfg);
    }
    else
    {
        SysPkgMessageCode *SysPkgMessageCodePtr;

        SysPkgAtrrHdPtr = (SysPkgAttrHeader*)PayloadBuffer;
        SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_MESSAGECODE);
        SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
        Offset                   += sizeof(SysPkgAttrHeader);
        SysPkgMessageCodePtr      = (SysPkgMessageCode*)(PayloadBuffer + Offset);
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

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetAudioEncodeCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}

