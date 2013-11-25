#include "sys_get_daynight_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysGetDaynightCommandExecutor::SysGetDaynightCommandExecutor()
    :  SysBaseCommandExecutor(SYSCODE_GET_DAY_NIGHT_REQ, CT_ShortTask)
{
}


SysGetDaynightCommandExecutor::~SysGetDaynightCommandExecutor()
{
}


GMI_RESULT SysGetDaynightCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetDaynightCommandExecutor> GetDaynightCommandExecutor(BaseMemoryManager::Instance().New<SysGetDaynightCommandExecutor>() );
    if (NULL == GetDaynightCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetDaynightCommandExecutor->m_Session = Packet->GetSession();
    GetDaynightCommandExecutor->m_Reply   = Packet->Clone();
    GetDaynightCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = GetDaynightCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetDaynightCommandExecutor::Execute()
{
    SYS_INFO("%s in..........\n", __func__);
    int32_t          MessageCode     = RETCODE_OK;
    SystemPacket    *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();

    ReferrencePtr<SysPkgDaynight>SysDaynight(BaseMemoryManager::Instance().New<SysPkgDaynight>());
    if (NULL == SysDaynight.GetPtr())
    {
        SYS_ERROR("SysDaynight new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    memset(SysDaynight.GetPtr(), 0, sizeof(SysPkgDaynight));
    GMI_RESULT Result = m_SystemServiceManager->SvrGetDaynightSettings(0, SysDaynight.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("SvrGetDaynightSettings fail, Result = 0x%lx\n", Result);
        MessageCode = RETCODE_ERROR;
    }
    else
    {
        SysDaynight.GetPtr()->s_Mode          = HOST_TO_NETWORK_UINT(SysDaynight.GetPtr()->s_Mode);
        SysDaynight.GetPtr()->s_DurationTime  = HOST_TO_NETWORK_UINT(SysDaynight.GetPtr()->s_DurationTime);
        SysDaynight.GetPtr()->s_NightToDayThr = HOST_TO_NETWORK_UINT(SysDaynight.GetPtr()->s_NightToDayThr);
        SysDaynight.GetPtr()->s_DayToNightThr = HOST_TO_NETWORK_UINT(SysDaynight.GetPtr()->s_DayToNightThr);
        for (int32_t i = 0; i < SCHEDULE_WEEK_DAYS; i++)
        {
            SysDaynight.GetPtr()->s_SchedEnable[i]    = HOST_TO_NETWORK_UINT(SysDaynight.GetPtr()->s_SchedEnable[i]);
            SysDaynight.GetPtr()->s_SchedStartTime[i] = HOST_TO_NETWORK_UINT(SysDaynight.GetPtr()->s_SchedStartTime[i]);
            SysDaynight.GetPtr()->s_SchedEndTime[i]   = HOST_TO_NETWORK_UINT(SysDaynight.GetPtr()->s_SchedEndTime[i]);
        }
    }

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
    uint16_t PacketAttrValueSize = sizeof(SysPkgDaynight);
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;
    uint16_t AttrCnt;


    if (MessageCode == 0)
    {
        AttrCnt            = 1;
        PacketAttrHdSize1  = sizeof(SysPkgAttrHeader) + PacketAttrValueSize;
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
                 SYSCODE_GET_DAY_NIGHT_RSP,
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
            SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_DAY_NIGHT);
            SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
            Offset                   += sizeof(SysPkgAttrHeader);
            memcpy((PayloadBuffer + Offset), &((SysDaynight.GetPtr())[Id]), PacketAttrValueSize);
            Offset                   += PacketAttrValueSize;
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


GMI_RESULT  SysGetDaynightCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}




