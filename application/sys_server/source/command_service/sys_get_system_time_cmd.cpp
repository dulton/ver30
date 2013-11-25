#include "sys_get_system_time_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysGetTimeCommandExecutor::SysGetTimeCommandExecutor()
    :  SysBaseCommandExecutor(SYSCODE_GET_TIME_REQ, CT_ShortTask)
{
}


SysGetTimeCommandExecutor::~SysGetTimeCommandExecutor()
{
}


GMI_RESULT SysGetTimeCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetTimeCommandExecutor> GetTimeCommandExecutor(BaseMemoryManager::Instance().New<SysGetTimeCommandExecutor>() );
    if (NULL == GetTimeCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetTimeCommandExecutor->m_Session = Packet->GetSession();
    GetTimeCommandExecutor->m_Reply   = Packet->Clone();
    GetTimeCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = GetTimeCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetTimeCommandExecutor::Execute()
{
    SYS_INFO("%s in..........\n", __func__);
    int32_t          MessageCode     = RETCODE_OK;
    SystemPacket    *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();

    ReferrencePtr<SysPkgDateTimeType>SysTimeTypePtr(BaseMemoryManager::Instance().New<SysPkgDateTimeType>());
    if (NULL == SysTimeTypePtr.GetPtr())
    {
        SYS_ERROR("SysTimeTypePtr new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    ReferrencePtr<SysPkgSysTime>SysTimePtr(BaseMemoryManager::Instance().New<SysPkgSysTime>());
    if (NULL == SysTimePtr.GetPtr())
    {
        SYS_ERROR("SysTimePtr new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    ReferrencePtr<SysPkgTimeZone>SysTimezonePtr(BaseMemoryManager::Instance().New<SysPkgTimeZone>());
    if (NULL == SysTimezonePtr.GetPtr())
    {
        SYS_ERROR("SysTimezonePtr new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    ReferrencePtr<SysPkgNtpServerInfo>SysNtpServerInfoPtr(BaseMemoryManager::Instance().New<SysPkgNtpServerInfo>());
    if (NULL == SysNtpServerInfoPtr.GetPtr())
    {
        SYS_ERROR("SysNtpServerInfoPtr new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    do
    {
        GMI_RESULT Result = m_SystemServiceManager->SvrGetTimeType(SysTimeTypePtr.GetPtr());
        if (FAILED(Result))
        {
            SYS_ERROR("SvrGetTimeType fail Result = 0x%lx\n", Result);
            MessageCode = RETCODE_ERROR;
            break;
        }
        SysTimeTypePtr.GetPtr()->s_Type        = HOST_TO_NETWORK_UINT(SysTimeTypePtr.GetPtr()->s_Type);
        SysTimeTypePtr.GetPtr()->s_NtpInterval = HOST_TO_NETWORK_UINT(SysTimeTypePtr.GetPtr()->s_NtpInterval);

        Result = m_SystemServiceManager->SvrGetTime(SysTimePtr.GetPtr());
        if (FAILED(Result))
        {
            SYS_ERROR("SvrGetTime fail Result = 0x%lx\n", Result);
            MessageCode = RETCODE_ERROR;
            break;
        }
        SysTimePtr.GetPtr()->s_Year   = HOST_TO_NETWORK_UINT(SysTimePtr.GetPtr()->s_Year);
        SysTimePtr.GetPtr()->s_Month  = HOST_TO_NETWORK_UINT(SysTimePtr.GetPtr()->s_Month);
        SysTimePtr.GetPtr()->s_Day    = HOST_TO_NETWORK_UINT(SysTimePtr.GetPtr()->s_Day);
        SysTimePtr.GetPtr()->s_Hour   = HOST_TO_NETWORK_UINT(SysTimePtr.GetPtr()->s_Hour);
        SysTimePtr.GetPtr()->s_Minute = HOST_TO_NETWORK_UINT(SysTimePtr.GetPtr()->s_Minute);
        SysTimePtr.GetPtr()->s_Second = HOST_TO_NETWORK_UINT(SysTimePtr.GetPtr()->s_Second);

        Result = m_SystemServiceManager->SvrGetTimezone(SysTimezonePtr.GetPtr());
        if (FAILED(Result))
        {
            SYS_ERROR("SvrGetTimezone fail Result = 0x%lx\n", Result);
            MessageCode = RETCODE_ERROR;
            break;
        }
        SysTimezonePtr.GetPtr()->s_TimeZone = HOST_TO_NETWORK_UINT(SysTimezonePtr.GetPtr()->s_TimeZone);

        Result = m_SystemServiceManager->SvrGetNtpServerInfo(SysNtpServerInfoPtr.GetPtr());
        if (FAILED(Result))
        {
            SYS_ERROR("SvrGetNtpServerInfo fail Result = 0x%lx\n", Result);
            MessageCode = RETCODE_ERROR;
            break;
        }
    }
    while(0);

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t MessagePacketAttrSize;
    uint16_t TimePacketAttrSize;
    uint16_t TimezonePacketAttrSize;
    uint16_t NtpPacketAttrSize;
    uint16_t TimeTypePacketAttrSize;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;
    uint16_t AttrCnt;

    if (MessageCode == RETCODE_OK)
    {
        AttrCnt                = 4;
        TimePacketAttrSize     = sizeof(SysPkgAttrHeader) + sizeof(SysPkgSysTime);
        TimezonePacketAttrSize = sizeof(SysPkgAttrHeader) + sizeof(SysPkgTimeZone);
        NtpPacketAttrSize      = sizeof(SysPkgAttrHeader) + sizeof(SysPkgNtpServerInfo);
        TimeTypePacketAttrSize = sizeof(SysPkgAttrHeader) + sizeof(SysPkgDateTimeType);

        PayloadTotalSize       = TimePacketAttrSize + TimezonePacketAttrSize + NtpPacketAttrSize + TimeTypePacketAttrSize + sizeof(SdkPkgTransferProtocolKey);
        AppPacketSize          = PacketHeaderSize  + TimePacketAttrSize + TimezonePacketAttrSize + NtpPacketAttrSize + TimeTypePacketAttrSize;
    }
    else
    {
        AttrCnt                = 1;
        MessagePacketAttrSize  = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
        PayloadTotalSize       = MessagePacketAttrSize + sizeof(SdkPkgTransferProtocolKey);
        AppPacketSize          = PacketHeaderSize  + MessagePacketAttrSize;
    }

    ReferrencePtr<SystemPacket> Reply(BaseMemoryManager::Instance().New<SystemPacket>());
    if (NULL == Reply.GetPtr())
    {
        SYS_ERROR("Reply new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    GMI_RESULT Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if (FAILED(Result))
    {
        SYS_ERROR("AllocatePacketBuffer fail Result = 0x%lx\n", Result);
        return Result;
    }

    //fill packet header
    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_GET_TIME_RSP,
                 AttrCnt,
                 AppPacketSize,
                 CommandPacket->GetSessionId(),
                 CommandPacket->GetSequenceNumber()
             );
    if (FAILED(Result))
    {
        SYS_ERROR("FillPacketHeader fail Result = 0x%lx\n", Result);
        return Result;
    }

    //fill  packet body
    uint8_t *PayloadBuffer = Reply->GetPacketPayloadBuffer();
    uint16_t Offset        = 0;
    SysPkgAttrHeader *SysPkgAtrrHdPtr;

    if (MessageCode == RETCODE_OK)
    {
        SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)(PayloadBuffer + Offset);
        SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_TIMETYPE);
        SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(TimeTypePacketAttrSize);
        Offset                   += sizeof(SysPkgAttrHeader);
        memcpy((PayloadBuffer + Offset), SysTimeTypePtr.GetPtr(), sizeof(SysPkgDateTimeType));
        Offset                   += sizeof(SysPkgDateTimeType);

        SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)(PayloadBuffer + Offset);
        SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_SYSTIME);
        SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(TimePacketAttrSize);
        Offset                   += sizeof(SysPkgAttrHeader);
        memcpy((PayloadBuffer + Offset), SysTimePtr.GetPtr(), sizeof(SysPkgSysTime));
        Offset                   += sizeof(SysPkgSysTime);

        SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)(PayloadBuffer + Offset);
        SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_TIMEZONE);
        SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(TimezonePacketAttrSize);
        Offset                   += sizeof(SysPkgAttrHeader);
        memcpy((PayloadBuffer + Offset), SysTimezonePtr.GetPtr(), sizeof(SysPkgTimeZone));
        Offset                   += sizeof(SysPkgTimeZone);

        SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)(PayloadBuffer + Offset);
        SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_NTPSERVER);
        SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(NtpPacketAttrSize);
        Offset                   += sizeof(SysPkgAttrHeader);
        memcpy((PayloadBuffer + Offset), SysNtpServerInfoPtr.GetPtr(), sizeof(SysPkgNtpServerInfo));
        Offset                   += sizeof(SysPkgNtpServerInfo);
    }
    else
    {
        SysPkgMessageCode *SysPkgMessageCodePtr;

        SysPkgAtrrHdPtr                     = (SysPkgAttrHeader*)(PayloadBuffer + Offset);
        SysPkgAtrrHdPtr->s_Type             = HOST_TO_NETWORK_USHORT(TYPE_MESSAGECODE);
        SysPkgAtrrHdPtr->s_Length           = HOST_TO_NETWORK_USHORT(MessagePacketAttrSize);
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

    SYS_INFO("%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT  SysGetTimeCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}




