#include "sys_del_user_info_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"
#include "application_packet_header.h"

SysDelUserInfoCommandExecutor::SysDelUserInfoCommandExecutor(void)
    : SysBaseCommandExecutor(SYSCODE_DEL_USERINFO_REQ, CT_ShortTask )
{
}


SysDelUserInfoCommandExecutor::~SysDelUserInfoCommandExecutor(void)
{
}


GMI_RESULT  SysDelUserInfoCommandExecutor::Create(ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysDelUserInfoCommandExecutor> DelUserInfoCommandExecutor( BaseMemoryManager::Instance().New<SysDelUserInfoCommandExecutor>() );
    if (NULL == DelUserInfoCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    DelUserInfoCommandExecutor->m_Session = Packet->GetSession();
    DelUserInfoCommandExecutor->m_Reply   = Packet->Clone();
    GMI_RESULT Result = DelUserInfoCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);
    if (FAILED(Result))
    {
        return Result;
    }

    CommandExecutor = DelUserInfoCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysDelUserInfoCommandExecutor::Execute()
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    int32_t     	  MessageCode     = RETCODE_OK;
    GMI_RESULT        Result          = GMI_SUCCESS;
    SystemPacket     *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();
    uint8_t          *PayloadBuff     = CommandPacket->GetPacketPayloadBuffer();
    uint16_t          ReqAttrCnt      = CommandPacket->GetAttrCount();
    SysPkgAttrHeader *SysPkgAttrHdPtr;
    SysPkgUserInfo   *SysUserInfoPtr;

    if (ReqAttrCnt > 0)
    {
        uint16_t i = 0;
        uint16_t Offset = 0;
        for (i = 0; i < ReqAttrCnt; i++)
        {
            SysPkgAttrHdPtr = (SysPkgAttrHeader*)(PayloadBuff + Offset);
            if (NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type) != TYPE_USERINFOR)
            {
                SYS_ERROR("SysPkgAttrHdPtr->s_Type %d incorrect\n", NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type));
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysPkgAttrHdPtr->s_Type %d incorrect\n", NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type));
                break;
            }

            if (NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Length) != (sizeof(SysPkgUserInfo) + sizeof(SysPkgAttrHeader)))
            {
                SYS_ERROR("SysPkgAttrHdPtr->s_Length %d incorrect\n", NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Length));
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysPkgAttrHdPtr->s_Length %d incorrect\n", NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Length));
                break;
            }

            Offset += sizeof(SysPkgAttrHeader);
            Offset += sizeof(SysPkgUserInfo);
        }

        if (i >= ReqAttrCnt)
        {
            Offset = 0;
            for (i = 0; i < ReqAttrCnt; i++)
            {
                SysPkgAttrHdPtr = (SysPkgAttrHeader*)(PayloadBuff + Offset);
                SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);
                Offset += sizeof(SysPkgAttrHeader);
                SysUserInfoPtr = (SysPkgUserInfo*)(PayloadBuff + Offset);
                SysUserInfoPtr->s_UserFlag = NETWORK_TO_HOST_USHORT(SysUserInfoPtr->s_UserFlag);
                SysUserInfoPtr->s_UserLevel= NETWORK_TO_HOST_USHORT(SysUserInfoPtr->s_UserLevel);
                Result = m_SystemServiceManager->SvrDelUser(SysUserInfoPtr);
                if (FAILED(Result))
                {
                    SYS_ERROR("SvrSetUser fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SvrSetUser fail, Result = 0x%lx\n", Result);
                    MessageCode = RETCODE_ERROR;
                    break;
                }
                Offset += sizeof(SysPkgUserInfo);
            }
        }
        else
        {
            MessageCode = RETCODE_ERROR;
        }
    }
    else
    {
        SYS_ERROR("ReqAttrCnt %d incorrect\n", ReqAttrCnt);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ReqAttrCnt %d incorrect\n", ReqAttrCnt);
        MessageCode = RETCODE_ERROR;
    }

    // Reply
    uint16_t PacketHeaderSize  = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    uint16_t PayloadTotalSize  = PacketAttrHdSize1 + sizeof(SdkPkgTransferProtocolKey);
    uint16_t AppPacketSize     = PacketHeaderSize + PacketAttrHdSize1;

    ReferrencePtr<SystemPacket> Reply(BaseMemoryManager::Instance().New<SystemPacket>());
    if (NULL == Reply.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    //fill packet header
    Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if (FAILED(Result))
    {
        SYS_ERROR("AllocatePacketBuffer fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_DEL_USERINFO_RSP,
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
        SYS_ERROR("FillPacketSdkTransferProtocolKey fail, Result = 0x%lx\n", Result);
        return Result;
    }

    m_Reply = Reply;

    SYS_INFO("%s:%s normal out..........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT  SysDelUserInfoCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}

