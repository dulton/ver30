#include "log.h"
#include "sys_get_alarm_config_cmd.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysGetAlarmConfigCommandExecutor::SysGetAlarmConfigCommandExecutor(void)
	: SysBaseCommandExecutor(SYSCODE_GET_ALMCFG_REQ, CT_ShortTask )
{
}


SysGetAlarmConfigCommandExecutor::~SysGetAlarmConfigCommandExecutor(void)
{
}


GMI_RESULT SysGetAlarmConfigCommandExecutor::Create(ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
	SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

	if (GetCommandId() != SysPacket->GetMessageCode())
	{
		return GMI_FAIL;
	}

	SafePtr<SysGetAlarmConfigCommandExecutor> GetAlarmConfigCommandExecutor( BaseMemoryManager::Instance().New<SysGetAlarmConfigCommandExecutor>() );
	if (NULL == GetAlarmConfigCommandExecutor.GetPtr())
	{
		return GMI_OUT_OF_MEMORY;
	}

	GetAlarmConfigCommandExecutor->m_Session = Packet->GetSession();
	GetAlarmConfigCommandExecutor->m_Reply   = Packet->Clone();
	GMI_RESULT Result = GetAlarmConfigCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);
	if (FAILED(Result))
	{
		return Result;
	}

	CommandExecutor = GetAlarmConfigCommandExecutor;

	return GMI_SUCCESS;
}


GMI_RESULT	SysGetAlarmConfigCommandExecutor::Execute()
{
	SYS_INFO("%s in..........\n", __func__);
	GMI_RESULT        Result           = GMI_SUCCESS;
    int32_t           MessageCode      = RETCODE_OK;
    SystemPacket     *CommandPacket    = (SystemPacket*) m_Reply.GetPtr();     
    uint8_t          *PayloadBuff      = CommandPacket->GetPacketPayloadBuffer();
    SysPkgAttrHeader *SysPkgAttrHdPtr  = (SysPkgAttrHeader*)PayloadBuff;

    SysPkgAlarmInConfig    SysAlarmInConfig;
    SysPkgAlarmOutConfig   SysAlarmOutConfig;
    SysPkgAlarmEventConfig SysAlarmEventConfig;
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
        if (SysPkgAttrHdPtr->s_Type != TYPE_GET_ALMCONFIG)
        {
            SYS_ERROR("SysPkgAttrHdPtr->s_Type %d incorrect\n", SysPkgAttrHdPtr->s_Type);
            MessageCode = RETCODE_ERROR;
            break;
        }

		SysPkgGetAlarmConfig SysGetAlarmConfig;	
		memcpy(&SysGetAlarmConfig, (PayloadBuff + sizeof(SysPkgAttrHeader)), sizeof(SysPkgGetAlarmConfig));
		int32_t  AlarmId  = NETWORK_TO_HOST_UINT(SysGetAlarmConfig.s_AlarmId);
		int32_t  Index    = NETWORK_TO_HOST_UINT(SysGetAlarmConfig.s_Index);		
		switch (AlarmId)
		{
		case SYS_DETECTOR_ID_ALARM_INPUT:
			ToGetAttrType = TYPE_ALARM_IN;
			memset(&SysAlarmInConfig, 0, sizeof(SysPkgAlarmInConfig));	
			Result = m_SystemServiceManager->SvrGetAlarmConfig(SYS_DETECTOR_ID_ALARM_INPUT, Index, &SysAlarmInConfig, sizeof(SysPkgAlarmInConfig));
			if (FAILED(Result))
			{
				MessageCode = RETCODE_ERROR;
				SYS_ERROR("SvrGetAlarmConfig %d fail, Result = 0x%lx\n", SYS_DETECTOR_ID_ALARM_INPUT, Result);
				break;
			}		
			SysAlarmInConfig.s_EnableFlag   = HOST_TO_NETWORK_UINT(SysAlarmInConfig.s_EnableFlag);
	    	SysAlarmInConfig.s_InputNumber  = HOST_TO_NETWORK_UINT(SysAlarmInConfig.s_InputNumber);
	    	SysAlarmInConfig.s_CheckTime    = HOST_TO_NETWORK_UINT(SysAlarmInConfig.s_CheckTime);
	    	SysAlarmInConfig.s_NormalStatus = HOST_TO_NETWORK_UINT(SysAlarmInConfig.s_NormalStatus);
	    	SysAlarmInConfig.s_LinkAlarmStrategy = HOST_TO_NETWORK_UINT(SysAlarmInConfig.s_LinkAlarmStrategy);
	    	SysAlarmInConfig.s_LinkAlarmExtInfo.s_OperateSeqNum = HOST_TO_NETWORK_UINT(SysAlarmInConfig.s_LinkAlarmExtInfo.s_OperateSeqNum);	    	
			ToGetAttr = (uint8_t*)&SysAlarmInConfig;
			ToGetAttrLength = sizeof(SysPkgAlarmInConfig);
			break;
		case SYS_DETECTOR_ID_PIR:
			ToGetAttrType = TYPE_ALARM_EVENT;
			memset(&SysAlarmEventConfig, 0, sizeof(SysPkgAlarmEventConfig));
			Result = m_SystemServiceManager->SvrGetAlarmConfig(SYS_DETECTOR_ID_PIR, 0, &SysAlarmEventConfig, sizeof(SysPkgAlarmEventConfig));
			if (FAILED(Result))
			{
				MessageCode = RETCODE_ERROR;
				SYS_ERROR("SvrGetAlarmConfig %d fail, Result = 0x%lx\n", SYS_DETECTOR_ID_PIR, Result);
				break;
			}
			SysAlarmEventConfig.s_AlarmId = HOST_TO_NETWORK_UINT(SysAlarmEventConfig.s_AlarmId);
			SysAlarmEventConfig.s_EnableFlag = HOST_TO_NETWORK_UINT(SysAlarmEventConfig.s_EnableFlag);
			SysAlarmEventConfig.s_CheckTime  = HOST_TO_NETWORK_UINT(SysAlarmEventConfig.s_CheckTime);
			SysAlarmEventConfig.s_LinkAlarmStrategy = HOST_TO_NETWORK_UINT(SysAlarmEventConfig.s_LinkAlarmStrategy);
			SysAlarmEventConfig.s_AlarmUnionExtData.s_PIRDetectInfo.s_Sensitive = HOST_TO_NETWORK_UINT(SysAlarmEventConfig.s_AlarmUnionExtData.s_PIRDetectInfo.s_Sensitive);	    	
			SysAlarmEventConfig.s_LinkAlarmExtInfo.s_OperateSeqNum = HOST_TO_NETWORK_UINT(SysAlarmEventConfig.s_LinkAlarmExtInfo.s_OperateSeqNum);    			    				
			ToGetAttr = (uint8_t*)&SysAlarmEventConfig;
			ToGetAttrLength = sizeof(SysPkgAlarmEventConfig);
			break;		
		case SYS_PROCESSOR_ID_ALARM_OUTPUT:
			ToGetAttrType = TYPE_ALARM_OUT;
			memset(&SysAlarmOutConfig, 0, sizeof(SysPkgAlarmOutConfig));
			Result = m_SystemServiceManager->SvrGetAlarmConfig(SYS_PROCESSOR_ID_ALARM_OUTPUT, Index, &SysAlarmOutConfig, sizeof(SysPkgAlarmOutConfig));
			if (FAILED(Result))
			{
				MessageCode = RETCODE_ERROR;
				SYS_ERROR("SvrGetAlarmConfig %d fail, Result = 0x%lx\n", SYS_PROCESSOR_ID_ALARM_OUTPUT, Result);
				break;
			}
			SysAlarmOutConfig.s_EnableFlag   = HOST_TO_NETWORK_UINT(SysAlarmOutConfig.s_EnableFlag);
	    	SysAlarmOutConfig.s_OutputNumber = HOST_TO_NETWORK_UINT(SysAlarmOutConfig.s_OutputNumber);
	    	SysAlarmOutConfig.s_NormalStatus = HOST_TO_NETWORK_UINT(SysAlarmOutConfig.s_NormalStatus);
	    	SysAlarmOutConfig.s_DelayTime    = HOST_TO_NETWORK_UINT(SysAlarmOutConfig.s_DelayTime);
			ToGetAttr = (uint8_t*)&SysAlarmOutConfig;
			ToGetAttrLength = sizeof(SysAlarmOutConfig);
			break;
		default:
			MessageCode = RETCODE_NOSUPPORT;
			break;
		}        
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
                 SYSCODE_GET_ALMCFG_RSP,
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


GMI_RESULT	SysGetAlarmConfigCommandExecutor::Reply()
{
	GMI_RESULT Result =  m_Reply->Submit(m_Session);
	if (FAILED(Result))
	{
		return Result;
	}

	return GMI_SUCCESS;
}


