#include "sys_get_white_balance_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysGetWhiteBalanceCommandExecutor::SysGetWhiteBalanceCommandExecutor()
    :  SysBaseCommandExecutor(SYSCODE_GET_WHITEBALANCE_REQ, CT_ShortTask)
{
}


SysGetWhiteBalanceCommandExecutor::~SysGetWhiteBalanceCommandExecutor()
{
}


GMI_RESULT SysGetWhiteBalanceCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetWhiteBalanceCommandExecutor> GetWhiteBalanceCommandExecutor(BaseMemoryManager::Instance().New<SysGetWhiteBalanceCommandExecutor>() );
    if (NULL == GetWhiteBalanceCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetWhiteBalanceCommandExecutor->m_Session = Packet->GetSession();
    GetWhiteBalanceCommandExecutor->m_Reply   = Packet->Clone();
    GetWhiteBalanceCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = GetWhiteBalanceCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetWhiteBalanceCommandExecutor::Execute()
{
    SYS_INFO("%s in..........\n", __func__);
    int32_t          MessageCode     = RETCODE_OK;
    SystemPacket    *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();

    ReferrencePtr<SysPkgWhiteBalance>SysWhiteBalancePtr(BaseMemoryManager::Instance().New<SysPkgWhiteBalance>());
    if (NULL == SysWhiteBalancePtr.GetPtr())
    {
        SYS_ERROR("SysWhiteBalancePtr new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    memset(SysWhiteBalancePtr.GetPtr(), 0, sizeof(SysPkgWhiteBalance));
    GMI_RESULT Result = m_SystemServiceManager->SvrGetWhiteBalanceSettings(0, SysWhiteBalancePtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("SvrGetWhiteBalanceSettings fail, Result = 0x%lx\n", Result);
        MessageCode = RETCODE_ERROR;
    }
    else
    {
        SysWhiteBalancePtr.GetPtr()->s_Mode  = HOST_TO_NETWORK_UINT(SysWhiteBalancePtr.GetPtr()->s_Mode);
        SysWhiteBalancePtr.GetPtr()->s_RGain = HOST_TO_NETWORK_UINT(SysWhiteBalancePtr.GetPtr()->s_RGain);
        SysWhiteBalancePtr.GetPtr()->s_BGain = HOST_TO_NETWORK_UINT(SysWhiteBalancePtr.GetPtr()->s_BGain);
    }

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;
    uint16_t AttrCnt;

    if (MessageCode == 0)
    {
        AttrCnt            = 1;
        PacketAttrHdSize1  = sizeof(SysPkgAttrHeader) + sizeof(SysPkgWhiteBalance);
    }
    else
    {
        AttrCnt           = 1;
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    }
    AppPacketSize    = PacketHeaderSize + PacketAttrHdSize1*AttrCnt;
    PayloadTotalSize = PacketAttrHdSize1*AttrCnt + sizeof(SdkPkgTransferProtocolKey);

    ReferrencePtr<SystemPacket> Reply( BaseMemoryManager::Instance().New<SystemPacket>() );
    if (NULL == Reply.GetPtr())
    {
        SYS_ERROR("Reply new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
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
                 SYSCODE_GET_WHITEBALANCE_RSP,
                 AttrCnt,
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
    SysPkgAttrHeader *SysPkgAtrrHdPtr;

    if (MessageCode == 0)
    {
        for (int32_t Id = 0; Id < AttrCnt; Id++)
        {
            SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)PayloadBuffer;
            SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_WHITE_BALANCE);
            SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
            Offset                   += sizeof( SysPkgAttrHeader );
            memcpy((PayloadBuffer + Offset), &((SysWhiteBalancePtr.GetPtr())[Id]), sizeof(SysPkgWhiteBalance));
            Offset                   += sizeof(SysPkgWhiteBalance);
        }
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
        SYS_ERROR("FillPacketSdkTransferProtocolKey fail, Result = 0x%lx\n", Result);
        return Result;
    }

    m_Reply = Reply;

    SYS_INFO("%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT  SysGetWhiteBalanceCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}

