#include "sys_force_idr_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysForceIdrCommandExecutor::SysForceIdrCommandExecutor(void)
    : SysBaseCommandExecutor(SYSCODE_FORCE_IDR_REQ, CT_ShortTask )
    , m_StreamId(0xff)
{
}


SysForceIdrCommandExecutor::~SysForceIdrCommandExecutor(void)
{
}


GMI_RESULT SysForceIdrCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysForceIdrCommandExecutor> ForceIdrCommandExecutor( BaseMemoryManager::Instance().New<SysForceIdrCommandExecutor>() );
    if (NULL == ForceIdrCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    ForceIdrCommandExecutor->m_Session = Packet->GetSession();
    ForceIdrCommandExecutor->m_Reply   = Packet->Clone();
    GMI_RESULT Result = ForceIdrCommandExecutor->SetParameter( m_SystemServiceManager, m_Argument, m_ArgumentSize);
    if (FAILED(Result))
    {
        return Result;
    }

    CommandExecutor = ForceIdrCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysForceIdrCommandExecutor::Execute()
{
    int32_t           MessageCode     = RETCODE_OK;
    SystemPacket     *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();
    uint8_t          *PayloadBuff     = CommandPacket->GetPacketPayloadBuffer();
    SysPkgAttrHeader *SysPkgAttrHdPtr = (SysPkgAttrHeader*)PayloadBuff;

    do
    {
        if (1 != CommandPacket->GetAttrCount())
        {
            MessageCode = RETCODE_ERROR;
            SYS_ERROR("ReqAttrCnt %d Error\n", CommandPacket->GetAttrCount());
            break;
        }

        SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);
        if (SysPkgAttrHdPtr->s_Type != TYPE_FORCEIDR)
        {
            MessageCode = RETCODE_ERROR;
            SYS_ERROR("SysPkgAttrHdPtr->s_Type %d error\n", SysPkgAttrHdPtr->s_Type);
            break;
        }

        int32_t StreamNum;
        GMI_RESULT Result = m_SystemServiceManager->SvrGetVideoStreamNum(&StreamNum);
        if (FAILED(Result))
        {
            MessageCode = RETCODE_ERROR;
            SYS_ERROR("SvrGetVideoStreamNum fail Result = 0x%lx\n", Result);
            break;
        }


        SysPkgForceIdr  *SysForceIdrPtr;
        SysForceIdrPtr = (SysPkgForceIdr*)(PayloadBuff + sizeof(SysPkgAttrHeader));
        m_StreamId = NETWORK_TO_HOST_UINT(SysForceIdrPtr->s_Flag);
        if (m_StreamId >= StreamNum)
        {
            MessageCode = RETCODE_ERROR;
            SYS_ERROR("StreamId = %d Error\n", m_StreamId);
            m_StreamId  = 0xff;
            break;
        }
    }
    while(0);

    // Reply
    uint16_t PacketHeaderSize  = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    uint16_t PayloadSize       = PacketAttrHdSize1 + sizeof(SdkPkgTransferProtocolKey);
    uint16_t PacketSize        = PacketHeaderSize + PacketAttrHdSize1;

    ReferrencePtr<SystemPacket> Reply(BaseMemoryManager::Instance().New<SystemPacket>() );
    if (NULL == Reply.GetPtr())
    {
        SYS_ERROR("Reply new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    GMI_RESULT Result = Reply->AllocatePacketBuffer(0, PayloadSize);
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
                 SYSCODE_FORCE_IDR_RSP,
                 1,
                 PacketSize,
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

    return GMI_SUCCESS;
}


GMI_RESULT  SysForceIdrCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    usleep(150);

    if (m_StreamId != 0xff)
    {
        GMI_RESULT Result = m_SystemServiceManager->SvrForceIdrFrame(m_StreamId);
        if (FAILED(Result))
        {
            SYS_ERROR("SvrForceIdrFrame fail, Result = 0x%lx\n", Result);
            return Result;
        }
    }

    return GMI_SUCCESS;
}






