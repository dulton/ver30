#include "log.h"
#include "sys_get_alarm_deploy_cmd.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysGetAlarmDeployCommandExecutor::SysGetAlarmDeployCommandExecutor(void)
	: SysBaseCommandExecutor(SYSCODE_GET_ALMDEPLOY_REQ, CT_ShortTask )
{
}


SysGetAlarmDeployCommandExecutor::~SysGetAlarmDeployCommandExecutor(void)
{
}


GMI_RESULT SysGetAlarmDeployCommandExecutor::Create(ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
	SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

	if (GetCommandId() != SysPacket->GetMessageCode())
	{
		return GMI_FAIL;
	}

	SafePtr<SysGetAlarmDeployCommandExecutor> GetAlarmDeployCommandExecutor( BaseMemoryManager::Instance().New<SysGetAlarmDeployCommandExecutor>() );
	if (NULL == GetAlarmDeployCommandExecutor.GetPtr())
	{
		return GMI_OUT_OF_MEMORY;
	}

	GetAlarmDeployCommandExecutor->m_Session = Packet->GetSession();
	GetAlarmDeployCommandExecutor->m_Reply   = Packet->Clone();
	GMI_RESULT Result = GetAlarmDeployCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);
	if (FAILED(Result))
	{
		return Result;
	}

	CommandExecutor = GetAlarmDeployCommandExecutor;

	return GMI_SUCCESS;
}


GMI_RESULT	SysGetAlarmDeployCommandExecutor::Execute()
{
	SYS_INFO("%s in..........\n", __func__);
	GMI_RESULT        Result           = GMI_SUCCESS;
    int32_t           MessageCode      = RETCODE_OK;
    SystemPacket     *CommandPacket    = (SystemPacket*) m_Reply.GetPtr();     
    uint8_t          *PayloadBuff      = CommandPacket->GetPacketPayloadBuffer();
    SysPkgAttrHeader *SysPkgAttrHdPtr  = (SysPkgAttrHeader*)PayloadBuff;

	SysPkgAlarmScheduleTime SysAlarmScheduleTime;
	memset(&SysAlarmScheduleTime, 0, sizeof(SysPkgAlarmScheduleTime));
    uint16_t ToGetAttrType = 0;
    uint8_t *ToGetAttr = NULL;
    uint16_t ToGetAttrLength = 0;

	do
	{
		if (1 != CommandPacket->GetAttrCount())
        {
            MessageCode = RETCODE_ERROR;
            SYS_ERROR("ReqAttrCnt %d Error\n", CommandPacket->GetAttrCount());
            break;
        }

        SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);
        if (SysPkgAttrHdPtr->s_Type != TYPE_GET_ALMDEPLOY)
        {
            SYS_ERROR("SysPkgAttrHdPtr->s_Type %d incorrect\n", SysPkgAttrHdPtr->s_Type);
            MessageCode = RETCODE_ERROR;
            break;
        }

		SysPkgGetAlarmScheduleTime *SysGetAlarmScheduleTimePtr = (SysPkgGetAlarmScheduleTime*)(PayloadBuff + sizeof(SysPkgAttrHeader));
		
		SysGetAlarmScheduleTimePtr->s_ScheduleId = NETWORK_TO_HOST_UINT(SysGetAlarmScheduleTimePtr->s_ScheduleId);
		SysGetAlarmScheduleTimePtr->s_Index      = NETWORK_TO_HOST_UINT(SysGetAlarmScheduleTimePtr->s_Index);

		Result = m_SystemServiceManager->SvrGetAlmScheduleTime(SysGetAlarmScheduleTimePtr, &SysAlarmScheduleTime, sizeof(SysPkgAlarmScheduleTime));
		if (FAILED(Result))
		{
			SYS_ERROR("SvrGetAlmScheduleTime ScheduleId %d, Index %d fail, Result = 0x%lx\n", SysGetAlarmScheduleTimePtr->s_ScheduleId, SysGetAlarmScheduleTimePtr->s_Index, Result);
			MessageCode = RETCODE_ERROR;
			break;
		}

		SysAlarmScheduleTime.s_ScheduleId = HOST_TO_NETWORK_UINT(SysAlarmScheduleTime.s_ScheduleId);
		SysAlarmScheduleTime.s_Index      = HOST_TO_NETWORK_UINT(SysAlarmScheduleTime.s_Index);
		for (int32_t i = 0; i < DAYS_OF_WEEK; i++)
		{
			for (int32_t j = 0; j < TIME_SEGMENT_OF_DAY; j++)
			{
				SysAlarmScheduleTime.s_ScheduleTime[i][j].s_StartTime = HOST_TO_NETWORK_UINT(SysAlarmScheduleTime.s_ScheduleTime[i][j].s_StartTime);
				SysAlarmScheduleTime.s_ScheduleTime[i][j].s_EndTime   = HOST_TO_NETWORK_UINT(SysAlarmScheduleTime.s_ScheduleTime[i][j].s_EndTime);
			}
		}

		ToGetAttrType   = TYPE_ALMDEPLOY;
		ToGetAttr       = (uint8_t*)&SysAlarmScheduleTime;
		ToGetAttrLength = sizeof(SysPkgAlarmScheduleTime);
	}
	while (0);

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;
    uint16_t AttrCnt;

    if (MessageCode == RETCODE_OK)
    {
        AttrCnt            = 1;
        PacketAttrHdSize1  = sizeof(SysPkgAttrHeader) + ToGetAttrLength;
    }
    else
    {
        AttrCnt           = 1;
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    }
    AppPacketSize    = PacketHeaderSize + PacketAttrHdSize1*AttrCnt;
    PayloadTotalSize = PacketAttrHdSize1*AttrCnt + sizeof(SdkPkgTransferProtocolKey);

    ReferrencePtr<SystemPacket> Reply(BaseMemoryManager::Instance().New<SystemPacket>());
    if (NULL == Reply.GetPtr())
    {
        SYS_ERROR("Reply new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if (FAILED(Result))
    {
        SYS_ERROR("AllocatePacketBuffer fail\n");
        return Result;
    }

    //fill packet header
    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_GET_ALMDEPLOY_RSP,
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
        SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)(PayloadBuffer + Offset);
        SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(ToGetAttrType);
        SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
        Offset                   += sizeof(SysPkgAttrHeader);
        memcpy((PayloadBuffer + Offset), ToGetAttr, ToGetAttrLength);
        Offset                   += ToGetAttrLength;   
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

	return GMI_SUCCESS;
}


GMI_RESULT	SysGetAlarmDeployCommandExecutor::Reply()
{
	GMI_RESULT Result =  m_Reply->Submit(m_Session);
	if (FAILED(Result))
	{
		return Result;
	}

	return GMI_SUCCESS;
}

