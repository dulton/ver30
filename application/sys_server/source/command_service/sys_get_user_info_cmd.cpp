#include "sys_get_user_info_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"
#include "application_packet_header.h"

SysGetUserInfoCommandExecutor::SysGetUserInfoCommandExecutor(void)
    : SysBaseCommandExecutor(SYSCODE_GET_USERINFO_REQ, CT_ShortTask )
{
}


SysGetUserInfoCommandExecutor::~SysGetUserInfoCommandExecutor(void)
{
}


GMI_RESULT  SysGetUserInfoCommandExecutor::Create(ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetUserInfoCommandExecutor> GetUserInfoCommandExecutor( BaseMemoryManager::Instance().New<SysGetUserInfoCommandExecutor>() );
    if (NULL == GetUserInfoCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetUserInfoCommandExecutor->m_Session = Packet->GetSession();
    GetUserInfoCommandExecutor->m_Reply   = Packet->Clone();
    GMI_RESULT Result = GetUserInfoCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);
    if (FAILED(Result))
    {
        return Result;
    }

    CommandExecutor = GetUserInfoCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetUserInfoCommandExecutor::Execute()
{
    uint16_t       UserCnt       = 1;
    int32_t        MessageCode   = RETCODE_OK;
    GMI_RESULT     Result        = GMI_SUCCESS;
    SystemPacket  *CommandPacket = (SystemPacket*)m_Reply.GetPtr();
    ReferrencePtr<SysPkgUserInfo, DefaultObjectsDeleter> SysUserInfoPtr;
    //uint32_t      AuthValue = CommandPacket->GetSdkTransferProtocolAuthValue();

    do
    {
        uint32_t UserNum;
        Result = m_SystemServiceManager->SvrGetUserNum(&UserNum);
        if (FAILED(Result))
        {
            MessageCode = RETCODE_ERROR;
            break;
        }

        SysUserInfoPtr = BaseMemoryManager::Instance().News<SysPkgUserInfo>(UserNum);
        if (NULL == SysUserInfoPtr.GetPtr())
        {
            SYS_ERROR("SysUserInfoPtr news fail\n");
            return GMI_OUT_OF_MEMORY;
        }

        uint32_t RealUserNum;
        Result = m_SystemServiceManager->SvrGetAllUsers(SysUserInfoPtr.GetPtr(), UserNum, &RealUserNum);
        if (FAILED(Result))
        {
            SysUserInfoPtr = NULL;
            MessageCode = RETCODE_ERROR;
            SYS_ERROR("SvrGetAllUsers fail, Result = 0x%lx\n", Result);
            break;
        }

        UserCnt = RealUserNum;

        for (uint32_t Id = 0; Id < UserCnt; Id++)
        {
            (SysUserInfoPtr.GetPtr())[Id].s_UserFlag = HOST_TO_NETWORK_USHORT((SysUserInfoPtr.GetPtr())[Id].s_UserFlag);
            (SysUserInfoPtr.GetPtr())[Id].s_UserLevel= HOST_TO_NETWORK_USHORT((SysUserInfoPtr.GetPtr())[Id].s_UserLevel);
        }
    }
    while(0);

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;
    uint16_t AttrCnt;

    if (MessageCode == RETCODE_OK)
    {
        AttrCnt = UserCnt;
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgUserInfo);
    }
    else
    {
        AttrCnt = 1;
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    }
    AppPacketSize    = PacketHeaderSize + PacketAttrHdSize1*AttrCnt;
    PayloadTotalSize = PacketAttrHdSize1*AttrCnt + sizeof(SdkPkgTransferProtocolKey);

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
                 SYSCODE_GET_USERINFO_RSP,
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
    uint16_t Offset = 0;
    SysPkgAttrHeader *SysPkgAtrrHdPtr;

    if (MessageCode == RETCODE_OK)
    {
        for (int32_t Id = 0; Id < AttrCnt; Id++)
        {
            SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)(PayloadBuffer + Offset);
            SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_USERINFOR);
            SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
            Offset                   += sizeof(SysPkgAttrHeader);
            memcpy((PayloadBuffer + Offset), &((SysUserInfoPtr.GetPtr())[Id]), sizeof(SysPkgUserInfo));
            Offset                   += sizeof(SysPkgUserInfo);
        }
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


GMI_RESULT  SysGetUserInfoCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}


