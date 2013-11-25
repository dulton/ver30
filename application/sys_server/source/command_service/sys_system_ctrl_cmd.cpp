
#include <stdlib.h>
#include "sys_system_ctrl_cmd.h"
#include "system_packet.h"
#include "sys_env_types.h"
#include "daemon.h"
#include "log.h"


SysSystemCtrlCommandExecutor::SysSystemCtrlCommandExecutor(void)
    : SysBaseCommandExecutor(SYSCODE_CTL_SYSTEM_REQ, CT_ShortTask)
    , m_Reboot(false)
{
}


SysSystemCtrlCommandExecutor::~SysSystemCtrlCommandExecutor(void)
{
}


GMI_RESULT  SysSystemCtrlCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysSystemCtrlCommandExecutor> SystemCtrlCommandExecutor( BaseMemoryManager::Instance().New<SysSystemCtrlCommandExecutor>() );
    if (NULL == SystemCtrlCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    SystemCtrlCommandExecutor->m_Session = Packet->GetSession();
    SystemCtrlCommandExecutor->m_Reply   = Packet->Clone();
    GMI_RESULT Result = SystemCtrlCommandExecutor->SetParameter( m_SystemServiceManager, m_Argument, m_ArgumentSize);
    if (FAILED(Result))
    {
        return Result;
    }

    CommandExecutor = SystemCtrlCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysSystemCtrlCommandExecutor::Execute()
{
    int32_t       MessageCode   = RETCODE_OK;
    SystemPacket *CommandPacket = (SystemPacket*) m_Reply.GetPtr();
    uint8_t      *PayloadBuff   = CommandPacket->GetPacketPayloadBuffer();
    int32_t       AttrValue[2];
    SysPkgAttrHeader *SysPkgAttrHdPtr;

    do
    {
        if (1 != CommandPacket->GetAttrCount()
                && 2 != CommandPacket->GetAttrCount())
        {
            MessageCode = RETCODE_ERROR;
            SYS_ERROR("ReqAttrCnt = %d incorrect\n", CommandPacket->GetAttrCount());
            break;
        }

        uint16_t i;
        int32_t Offset = 0;
        memset(AttrValue, 0, sizeof(AttrValue));
        for (i = 0; i < CommandPacket->GetAttrCount(); i++)
        {
            SysPkgAttrHdPtr = (SysPkgAttrHeader*)(PayloadBuff + Offset);

            SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);
            if (SysPkgAttrHdPtr->s_Type != TYPE_INTVALUE)
            {
                MessageCode = RETCODE_ERROR;
                SYS_ERROR("SysPkgAttrHdPtr->s_Type = %d incorrect\n", SysPkgAttrHdPtr->s_Type);
                break;
            }

            SysPkgAttrHdPtr->s_Length = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Length);
            if ((SysPkgAttrHdPtr->s_Length - sizeof(SysPkgAttrHeader))!= sizeof(int32_t))
            {
                MessageCode = RETCODE_ERROR;
                SYS_ERROR("ReqAttrLength = %d incorrect\n", SysPkgAttrHdPtr->s_Length);
                break;
            }

            AttrValue[i] = *(int32_t*)(PayloadBuff + Offset + sizeof(SysPkgAttrHeader));
            AttrValue[i] = NETWORK_TO_HOST_UINT(AttrValue[i]);

            Offset += SysPkgAttrHdPtr->s_Length;
        }

        if (0 > MessageCode)
        {
            break;
        }

        switch (AttrValue[0])
        {
        case SYS_SYSTEM_CTRL_REBOOT:
            m_Reboot = true;
            break;
        case SYS_SYSTEM_CTRL_DEFAULT_HARD:
        case SYS_SYSTEM_CTRL_DEFAULT_SOFT:
        {
            GMI_RESULT Result = m_SystemServiceManager->SvrSetSystemDefault(AttrValue[0], AttrValue[1]);
            if (FAILED(Result))
            {
                MessageCode = RETCODE_ERROR;
                SYS_ERROR("SvrSetSystemDefault fail, Result = 0x%lx\n", Result);
            }
        }
        break;
        default:
            MessageCode = RETCODE_NOSUPPORT;
            SYS_ERROR("not support %d\n", AttrValue[0]);
            break;
        }
    }
    while(0);

    // Reply
    uint16_t PacketHeaderSize  = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    uint16_t PayloadTotalSize  = PacketAttrHdSize1 + sizeof(SdkPkgTransferProtocolKey);
    uint16_t AppPacketSize     = PacketHeaderSize + PacketAttrHdSize1;

    ReferrencePtr<SystemPacket> Reply( BaseMemoryManager::Instance().New<SystemPacket>() );
    if (NULL == Reply.GetPtr())
    {
        SYS_ERROR("Reply new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    GMI_RESULT Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if (FAILED(Result))
    {
        SYS_ERROR("AllocatePacketBuffer fail, Result = 0x%lx\n", Result);
        return Result;
    }

    //fill packet header
    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_CTL_SYSTEM_RSP,
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
    Offset += sizeof( SysPkgAttrHeader );
    SysPkgMessageCodePtr = (SysPkgMessageCode*)(PayloadBuffer + Offset);
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

    return GMI_SUCCESS;
}


GMI_RESULT  SysSystemCtrlCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    if (m_Reboot)
    {
        Result = DaemonReboot(10);
        if (FAILED(Result))
        {
            DaemonReboot(10);
            return Result;
        }
    }

    return GMI_SUCCESS;
}


