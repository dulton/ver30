#include "sys_get_network_port_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"
#include "application_packet_header.h"

SysGetNetworkPortCommandExecutor::SysGetNetworkPortCommandExecutor(void)
    : SysBaseCommandExecutor(SYSCODE_GET_NETWORK_PORT_REQ, CT_ShortTask )
{
}


SysGetNetworkPortCommandExecutor::~SysGetNetworkPortCommandExecutor(void)
{
}


GMI_RESULT  SysGetNetworkPortCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetNetworkPortCommandExecutor> GetNetworkPortCommandExecutor( BaseMemoryManager::Instance().New<SysGetNetworkPortCommandExecutor>() );
    if (NULL == GetNetworkPortCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetNetworkPortCommandExecutor->m_Session = Packet->GetSession();
    GetNetworkPortCommandExecutor->m_Reply = Packet->Clone();
    GMI_RESULT Result = GetNetworkPortCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);
    if (FAILED(Result))
    {
        return Result;
    }

    CommandExecutor = GetNetworkPortCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetNetworkPortCommandExecutor::Execute()
{
    int32_t       MessageCode   = RETCODE_ERROR;
    GMI_RESULT    Result        = GMI_SUCCESS;
    SystemPacket *CommandPacket = (SystemPacket*)m_Reply.GetPtr();
    SysPkgNetworkPort SysNetworkPortTmp;
    //uint32_t      AuthValue = CommandPacket->GetSdkTransferProtocolAuthValue();

    Result = m_SystemServiceManager->SvrGetNetworkPort(&SysNetworkPortTmp);
    if (SUCCEEDED(Result))
    {
        MessageCode = RETCODE_OK;
        SysNetworkPortTmp.s_HTTP_Port = HOST_TO_NETWORK_UINT(SysNetworkPortTmp.s_HTTP_Port);
        SysNetworkPortTmp.s_RTSP_Port = HOST_TO_NETWORK_UINT(SysNetworkPortTmp.s_RTSP_Port);
        SysNetworkPortTmp.s_SDK_Port  = HOST_TO_NETWORK_UINT(SysNetworkPortTmp.s_SDK_Port);
        SysNetworkPortTmp.s_Upgrade_Port= HOST_TO_NETWORK_UINT(SysNetworkPortTmp.s_Upgrade_Port);
        SysNetworkPortTmp.s_ONVIF_Port  = HOST_TO_NETWORK_UINT(SysNetworkPortTmp.s_ONVIF_Port);
    }

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;

    if (MessageCode == RETCODE_OK)
    {
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgNetworkPort);
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
                 SYSCODE_GET_NETWORK_PORT_RSP,
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
    uint16_t Offset = 0;
    SysPkgAttrHeader *SysPkgAtrrHdPtr;

    if (MessageCode == RETCODE_OK)
    {
        SysPkgNetworkPort *SysNetworkPortPtr;

        SysPkgAtrrHdPtr = (SysPkgAttrHeader*)PayloadBuffer;
        SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_NETWORK_PORT);
        SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
        Offset                   += sizeof(SysPkgAttrHeader);
        SysNetworkPortPtr           = (SysPkgNetworkPort*)(PayloadBuffer + Offset);
        memcpy((void_t*)SysNetworkPortPtr, (void_t*)&SysNetworkPortTmp,  sizeof(SysPkgNetworkPort));
        Offset                   += sizeof(SysPkgNetworkPort);
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


GMI_RESULT  SysGetNetworkPortCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}






