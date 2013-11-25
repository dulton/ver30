#include "sys_set_system_time_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysSetTimeCommandExecutor::SysSetTimeCommandExecutor()
    :  SysBaseCommandExecutor(SYSCODE_SET_TIME_REQ, CT_ShortTask)
{
}


SysSetTimeCommandExecutor::~SysSetTimeCommandExecutor()
{
}


GMI_RESULT SysSetTimeCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysSetTimeCommandExecutor> SetTimeCommandExecutor(BaseMemoryManager::Instance().New<SysSetTimeCommandExecutor>() );
    if (NULL == SetTimeCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    SetTimeCommandExecutor->m_Session = Packet->GetSession();
    SetTimeCommandExecutor->m_Reply   = Packet->Clone();
    SetTimeCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = SetTimeCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysSetTimeCommandExecutor::Execute()
{
    SYS_INFO("%s in..........\n", __func__);
    int32_t     	  MessageCode     = RETCODE_OK;
    GMI_RESULT        Result          = GMI_SUCCESS;
    SystemPacket     *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();
    uint8_t          *PayloadBuff     = CommandPacket->GetPacketPayloadBuffer();
    uint16_t          ReqAttrCnt      = CommandPacket->GetAttrCount();
    SysPkgAttrHeader *SysPkgAttrHdPtr;

    if (ReqAttrCnt > 0)
    {
        uint16_t Offset = 0;

        while (ReqAttrCnt--)
        {
            SysPkgAttrHdPtr = (SysPkgAttrHeader*)(PayloadBuff + Offset);
            SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);
            Offset += sizeof(SysPkgAttrHeader);
            //SYS_INFO("SysPkgAttrHdPtr->s_Type %d\n", SysPkgAttrHdPtr->s_Type);
            switch (SysPkgAttrHdPtr->s_Type)
            {
            case TYPE_TIMETYPE:
            {
                SysPkgDateTimeType *SysDateTimeTypePtr;
                SysDateTimeTypePtr = (SysPkgDateTimeType*)(PayloadBuff + Offset);
                SysDateTimeTypePtr->s_NtpInterval = NETWORK_TO_HOST_UINT(SysDateTimeTypePtr->s_NtpInterval);
                SysDateTimeTypePtr->s_Type        = NETWORK_TO_HOST_UINT(SysDateTimeTypePtr->s_Type);
                Offset += sizeof(SysPkgDateTimeType);
                Result = m_SystemServiceManager->SvrSetTimeType(SysDateTimeTypePtr);
                if (FAILED(Result))
                {
                    SYS_ERROR("SvrSetTimeType fail Result = 0x%lx\n", Result);
                    MessageCode = RETCODE_ERROR;
                }
            }
            break;
            case TYPE_SYSTIME:
            {
                SysPkgSysTime *SysTimePtr;
                SysTimePtr = (SysPkgSysTime*)(PayloadBuff + Offset);
                SysTimePtr->s_Year   = NETWORK_TO_HOST_UINT(SysTimePtr->s_Year);
                SysTimePtr->s_Month  = NETWORK_TO_HOST_UINT(SysTimePtr->s_Month);
                SysTimePtr->s_Day    = NETWORK_TO_HOST_UINT(SysTimePtr->s_Day);
                SysTimePtr->s_Hour   = NETWORK_TO_HOST_UINT(SysTimePtr->s_Hour);
                SysTimePtr->s_Minute = NETWORK_TO_HOST_UINT(SysTimePtr->s_Minute);
                SysTimePtr->s_Second = NETWORK_TO_HOST_UINT(SysTimePtr->s_Second);
                Offset += sizeof(SysPkgSysTime);
                Result = m_SystemServiceManager->SvrSetTime(SysTimePtr);
                if (FAILED(Result))
                {
                    SYS_ERROR("SvrSetTime fail Result = 0x%lx\n", Result);
                    MessageCode = RETCODE_ERROR;
                }
            }
            break;
            case TYPE_TIMEZONE:
            {
                SysPkgTimeZone *SysTimezonePtr;
                SysTimezonePtr = (SysPkgTimeZone*)(PayloadBuff + Offset);
                SysTimezonePtr->s_TimeZone = NETWORK_TO_HOST_UINT(SysTimezonePtr->s_TimeZone);
                Offset += sizeof(SysPkgTimeZone);
                Result = m_SystemServiceManager->SvrSetTimezone(SysTimezonePtr);
                if (FAILED(Result))
                {
                    SYS_ERROR("SvrSetTimezone fail Result = 0x%lx\n", Result);
                    MessageCode = RETCODE_ERROR;
                }
            }
            break;
            case TYPE_NTPSERVER:
            {
                SysPkgNtpServerInfo *SysNtpServerInfoPtr;
                SysNtpServerInfoPtr = (SysPkgNtpServerInfo*)(PayloadBuff + Offset);
                Offset += sizeof(SysPkgNtpServerInfo);
                Result = m_SystemServiceManager->SvrSetNtpServerInfo(SysNtpServerInfoPtr);
                if (FAILED(Result))
                {
                    SYS_ERROR("SvrSetNtpServerInfo fail Result = 0x%lx\n", Result);
                    MessageCode = RETCODE_ERROR;
                }
            }
            break;
            default:
                MessageCode = RETCODE_ERROR;
                SYS_ERROR("invalid Attr type %d\n", SysPkgAttrHdPtr->s_Type);
            }

            if (RETCODE_ERROR == MessageCode)
            {
                break;
            }
        }
    }
    else
    {
        MessageCode = RETCODE_ERROR;
        SYS_ERROR("ReqAttrCnt %d error\n", ReqAttrCnt);
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
                 SYSCODE_SET_TIME_RSP,
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


GMI_RESULT  SysSetTimeCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    return GMI_SUCCESS;
}






