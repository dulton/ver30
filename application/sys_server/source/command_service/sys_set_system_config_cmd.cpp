#include "sys_set_system_config_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysSetSystemConfigCommandExecutor::SysSetSystemConfigCommandExecutor()
    :  SysBaseCommandExecutor(SYSCODE_SET_DEVICEINFO_REQ, CT_ShortTask)
{
}


SysSetSystemConfigCommandExecutor::~SysSetSystemConfigCommandExecutor()
{
}


GMI_RESULT SysSetSystemConfigCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysSetSystemConfigCommandExecutor> SetSystemConfigCommandExecutor(BaseMemoryManager::Instance().New<SysSetSystemConfigCommandExecutor>() );
    if (NULL == SetSystemConfigCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    if (1 != SysPacket->GetAttrCount())
    {
        return GMI_FAIL;
    }

    SetSystemConfigCommandExecutor->m_Session = Packet->GetSession();
    SetSystemConfigCommandExecutor->m_Reply   = Packet->Clone();
    SetSystemConfigCommandExecutor->SetParameter( m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = SetSystemConfigCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysSetSystemConfigCommandExecutor::Execute()
{
    SYS_INFO("%s in..........\n", __func__);
    int32_t     	  MessageCode     = RETCODE_OK;
    GMI_RESULT        Result          = GMI_SUCCESS;
    SystemPacket     *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();
    uint8_t          *PayloadBuff     = CommandPacket->GetPacketPayloadBuffer();
    SysPkgAttrHeader *SysPkgAttrHdPtr = (SysPkgAttrHeader*)PayloadBuff;

    SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);
    if (SysPkgAttrHdPtr->s_Type == TYPE_IPCNAME)
    {
        SysPkgDeviceInfo *SysDeviceInfoPtr = (SysPkgDeviceInfo*)(PayloadBuff + sizeof(SysPkgAttrHeader));
        SysDeviceInfoPtr->s_DeviceId = NETWORK_TO_HOST_UINT(SysDeviceInfoPtr->s_DeviceId);
        Result = m_SystemServiceManager->SvrSetDeviceInfo(SysDeviceInfoPtr);
        if (FAILED(Result))
        {
            SYS_ERROR("SvrSetDeviceInfo fail\n");
            MessageCode = RETCODE_ERROR;
        }
    }
    else
    {
        SYS_ERROR("SysPkgAttrHdPtr->s_Type %d incorrect\n", SysPkgAttrHdPtr->s_Type);
        MessageCode = RETCODE_ERROR;
    }

    // Reply
    uint16_t PacketHeaderSize  = sizeof( SysPkgHeader );
    uint16_t PacketAttrHdSize1 = sizeof( SysPkgAttrHeader ) + sizeof( SysPkgMessageCode );
    uint16_t PayloadTotalSize  = PacketAttrHdSize1 + sizeof(SdkPkgTransferProtocolKey);
    uint16_t AppPacketSize     = PacketHeaderSize + PacketAttrHdSize1;

    ReferrencePtr<SystemPacket> Reply( BaseMemoryManager::Instance().New<SystemPacket>() );
    if (NULL == Reply.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if (FAILED(Result))
    {
        return Result;
    }

    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_SET_DEVICEINFO_RSP,
                 1,
                 AppPacketSize,
                 CommandPacket->GetSessionId(),
                 CommandPacket->GetSequenceNumber()
             );
    if (FAILED(Result))
    {
        SYS_ERROR("\n");
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

    SYS_INFO("%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT  SysSetSystemConfigCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}


