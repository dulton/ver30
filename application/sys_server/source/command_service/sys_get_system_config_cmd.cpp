#include "log.h"
#include "sys_get_system_config_cmd.h"
#include "system_packet.h"
#include "sys_env_types.h"

SysGetSysCfgCommandExecutor::SysGetSysCfgCommandExecutor(void)
    : SysBaseCommandExecutor( SYSCODE_GET_DEVICEINFO_REQ, CT_ShortTask )
{
}

SysGetSysCfgCommandExecutor::~SysGetSysCfgCommandExecutor(void)
{
}


GMI_RESULT  SysGetSysCfgCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetSysCfgCommandExecutor> GetSysCfgCommandExecutor( BaseMemoryManager::Instance().New<SysGetSysCfgCommandExecutor>() );
    if (NULL == GetSysCfgCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetSysCfgCommandExecutor->m_Session = Packet->GetSession();
    GetSysCfgCommandExecutor->m_Reply = Packet->Clone();
    GetSysCfgCommandExecutor->SetParameter( m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = GetSysCfgCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetSysCfgCommandExecutor::Execute()
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    int32_t           MessageCode     = RETCODE_OK;
    SystemPacket     *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();
    SysPkgDeviceInfo  SysDevInfo;

    GMI_RESULT Result = m_SystemServiceManager->SvrGetDevinfo(&SysDevInfo);
    if (FAILED(Result))
    {
        SYS_ERROR("SvrGetDevinfo error\n");
        MessageCode = RETCODE_ERROR;
    }
    else
    {
        SysDevInfo.s_DeviceId = HOST_TO_NETWORK_UINT(SysDevInfo.s_DeviceId);
    }

    // Reply
    uint16_t PacketHeaderSize  = sizeof(SysPkgHeader );
    uint16_t PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgDeviceInfo);
    uint16_t PayloadTotalSize  = PacketAttrHdSize1 + sizeof(SdkPkgTransferProtocolKey);
    uint16_t AppPacketSize     = PacketHeaderSize + PacketAttrHdSize1;

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
                 SYSCODE_GET_DEVICEINFO_RSP,
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
    SysPkgAttrHeader *SysPkgAtrrHdPtr;

    if (RETCODE_OK == MessageCode)
    {
        SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)(PayloadBuffer + Offset);
        SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_IPCNAME);
        SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
        Offset                   += sizeof(SysPkgAttrHeader);
        memcpy((PayloadBuffer + Offset), &SysDevInfo, sizeof(SysPkgDeviceInfo));
        Offset                   += sizeof(SysPkgDeviceInfo);
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
    SYS_INFO("%s:%s normal out..........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT  SysGetSysCfgCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}

