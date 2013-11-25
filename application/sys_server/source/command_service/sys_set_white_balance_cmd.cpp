#include "sys_set_white_balance_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysSetWhiteBalanceCommandExecutor::SysSetWhiteBalanceCommandExecutor()
    :  SysBaseCommandExecutor( SYSCODE_SET_WHITEBALANCE_REQ, CT_ShortTask)
{
}


SysSetWhiteBalanceCommandExecutor::~SysSetWhiteBalanceCommandExecutor()
{
}


GMI_RESULT SysSetWhiteBalanceCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysSetWhiteBalanceCommandExecutor> SetWhiteBalanceCommandExecutor(BaseMemoryManager::Instance().New<SysSetWhiteBalanceCommandExecutor>());
    if (NULL == SetWhiteBalanceCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    if (1 != SysPacket->GetAttrCount())
    {
        return GMI_FAIL;
    }

    SetWhiteBalanceCommandExecutor->m_Session = Packet->GetSession();
    SetWhiteBalanceCommandExecutor->m_Reply   = Packet->Clone();
    SetWhiteBalanceCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = SetWhiteBalanceCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysSetWhiteBalanceCommandExecutor::Execute()
{
    SYS_INFO("%s in..........\n", __func__);
    int32_t     	  MessageCode     = RETCODE_OK;
    GMI_RESULT        Result          = GMI_SUCCESS;
    SystemPacket     *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();
    uint8_t          *PayloadBuff     = CommandPacket->GetPacketPayloadBuffer();
    SysPkgAttrHeader *SysPkgAttrHdPtr = (SysPkgAttrHeader*)PayloadBuff;

    SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);
    if (SysPkgAttrHdPtr->s_Type == TYPE_WHITE_BALANCE)
    {
        SysPkgWhiteBalance  *SysWhiteBalancePtr = (SysPkgWhiteBalance*)(PayloadBuff + sizeof(SysPkgAttrHeader));
        SysWhiteBalancePtr->s_Mode    = NETWORK_TO_HOST_UINT(SysWhiteBalancePtr->s_Mode);
        SysWhiteBalancePtr->s_BGain   = NETWORK_TO_HOST_UINT(SysWhiteBalancePtr->s_BGain);
        SysWhiteBalancePtr->s_RGain   = NETWORK_TO_HOST_UINT(SysWhiteBalancePtr->s_RGain);

        Result = m_SystemServiceManager->SvrSetWhiteBalanceSettings(0, SysWhiteBalancePtr);
        if ( FAILED(Result) )
        {
            SYS_ERROR("SvrSetWhiteBalanceSettings fail, Result = 0x%lx\n", Result);
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
                 SYSCODE_SET_WHITEBALANCE_RSP,
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


GMI_RESULT  SysSetWhiteBalanceCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}





