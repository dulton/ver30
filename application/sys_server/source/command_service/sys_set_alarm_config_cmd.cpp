#include "log.h"
#include "sys_set_alarm_config_cmd.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysSetAlarmConfigCommandExecutor::SysSetAlarmConfigCommandExecutor()
    :  SysBaseCommandExecutor( SYSCODE_SET_ALMCFG_REQ, CT_ShortTask)
{
}


SysSetAlarmConfigCommandExecutor::~SysSetAlarmConfigCommandExecutor()
{
}


GMI_RESULT SysSetAlarmConfigCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysSetAlarmConfigCommandExecutor> SetAlarmConfigCommandExecutor(BaseMemoryManager::Instance().New<SysSetAlarmConfigCommandExecutor>() );
    if (NULL == SetAlarmConfigCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    if (1 != SysPacket->GetAttrCount())
    {
        return GMI_FAIL;
    }

    SetAlarmConfigCommandExecutor->m_Session = Packet->GetSession();
    SetAlarmConfigCommandExecutor->m_Reply   = Packet->Clone();
    SetAlarmConfigCommandExecutor->SetParameter( m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = SetAlarmConfigCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysSetAlarmConfigCommandExecutor::Execute()
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    int32_t     	  MessageCode     = RETCODE_OK;
    GMI_RESULT        Result          = GMI_SUCCESS;
    SystemPacket     *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();
    uint8_t          *PayloadBuff     = CommandPacket->GetPacketPayloadBuffer();
    SysPkgAttrHeader *SysPkgAttrHdPtr = (SysPkgAttrHeader*)PayloadBuff;

	do
	{
	    SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);        
	    if (TYPE_ALARM_IN == SysPkgAttrHdPtr->s_Type)
	    {
	        SysPkgAlarmInConfig  *SysAlarmInConfigPtr = (SysPkgAlarmInConfig*)(PayloadBuff + sizeof(SysPkgAttrHeader));
		
	    	SysAlarmInConfigPtr->s_EnableFlag   = NETWORK_TO_HOST_UINT(SysAlarmInConfigPtr->s_EnableFlag);
	    	SysAlarmInConfigPtr->s_InputNumber  = NETWORK_TO_HOST_UINT(SysAlarmInConfigPtr->s_InputNumber);
	    	SysAlarmInConfigPtr->s_CheckTime    = NETWORK_TO_HOST_UINT(SysAlarmInConfigPtr->s_CheckTime);
	    	SysAlarmInConfigPtr->s_NormalStatus = NETWORK_TO_HOST_UINT(SysAlarmInConfigPtr->s_NormalStatus);
	    	SysAlarmInConfigPtr->s_LinkAlarmStrategy = NETWORK_TO_HOST_UINT(SysAlarmInConfigPtr->s_LinkAlarmStrategy);
	    	SysAlarmInConfigPtr->s_LinkAlarmExtInfo.s_OperateSeqNum = NETWORK_TO_HOST_USHORT(SysAlarmInConfigPtr->s_LinkAlarmExtInfo.s_OperateSeqNum);	 
	    	SysAlarmInConfigPtr->s_LinkAlarmExtInfo.s_DelayTime     = NETWORK_TO_HOST_USHORT(SysAlarmInConfigPtr->s_LinkAlarmExtInfo.s_DelayTime);
	    	SysAlarmInConfigPtr->s_LinkAlarmExtInfo.s_PtzDelayTime  = NETWORK_TO_HOST_USHORT(SysAlarmInConfigPtr->s_LinkAlarmExtInfo.s_PtzDelayTime);
	        Result = m_SystemServiceManager->SvrSetAlarmConfig(SYS_DETECTOR_ID_ALARM_INPUT, SysAlarmInConfigPtr->s_InputNumber, SysAlarmInConfigPtr, sizeof(SysPkgAlarmInConfig));
	        if (FAILED(Result))
	        {
	            SYS_ERROR("SvrSetAlarmConfig %d fail, Result = 0x%lx\n", SYS_DETECTOR_ID_ALARM_INPUT, Result);
	            MessageCode = RETCODE_ERROR;
	            break;
	        }
	    }
	    else if (TYPE_ALARM_OUT == SysPkgAttrHdPtr->s_Type)
	    {
	    	SysPkgAlarmOutConfig *SysAlarmOutConfigPtr = (SysPkgAlarmOutConfig*)(PayloadBuff + sizeof(SysPkgAttrHeader));
	    	SysAlarmOutConfigPtr->s_EnableFlag   = NETWORK_TO_HOST_UINT(SysAlarmOutConfigPtr->s_EnableFlag);
	    	SysAlarmOutConfigPtr->s_OutputNumber = NETWORK_TO_HOST_UINT(SysAlarmOutConfigPtr->s_OutputNumber);
	    	SysAlarmOutConfigPtr->s_NormalStatus = NETWORK_TO_HOST_UINT(SysAlarmOutConfigPtr->s_NormalStatus);
	    	SysAlarmOutConfigPtr->s_DelayTime    = NETWORK_TO_HOST_UINT(SysAlarmOutConfigPtr->s_DelayTime);
	    	Result = m_SystemServiceManager->SvrSetAlarmConfig(SYS_PROCESSOR_ID_ALARM_OUTPUT, SysAlarmOutConfigPtr->s_OutputNumber, SysAlarmOutConfigPtr, sizeof(SysPkgAlarmOutConfig));
	        if (FAILED(Result))
	        {
	            SYS_ERROR("SvrSetAlarmConfig %d fail, Result = 0x%lx\n", SYS_PROCESSOR_ID_ALARM_OUTPUT, Result);
	            MessageCode = RETCODE_ERROR;
	            break;
	        }
	    }
	    else if (TYPE_ALARM_EVENT == SysPkgAttrHdPtr->s_Type)
	    {
	    	SysPkgAlarmEventConfig *SysAlarmEventConfigPtr = (SysPkgAlarmEventConfig*)(PayloadBuff + sizeof(SysPkgAttrHeader));
	    	SysAlarmEventConfigPtr->s_AlarmId = NETWORK_TO_HOST_UINT(SysAlarmEventConfigPtr->s_AlarmId);
	    	if (SYS_DETECTOR_ID_PIR == SysAlarmEventConfigPtr->s_AlarmId)
	    	{
		    	SysAlarmEventConfigPtr->s_EnableFlag = NETWORK_TO_HOST_UINT(SysAlarmEventConfigPtr->s_EnableFlag);
		    	SysAlarmEventConfigPtr->s_CheckTime  = NETWORK_TO_HOST_UINT(SysAlarmEventConfigPtr->s_CheckTime);
		    	SysAlarmEventConfigPtr->s_LinkAlarmStrategy = NETWORK_TO_HOST_UINT(SysAlarmEventConfigPtr->s_LinkAlarmStrategy);
		    	SysAlarmEventConfigPtr->s_AlarmUnionExtData.s_PIRDetectInfo.s_Sensitive = NETWORK_TO_HOST_UINT(SysAlarmEventConfigPtr->s_AlarmUnionExtData.s_PIRDetectInfo.s_Sensitive);	    	
		    	SysAlarmEventConfigPtr->s_LinkAlarmExtInfo.s_OperateSeqNum = NETWORK_TO_HOST_USHORT(SysAlarmEventConfigPtr->s_LinkAlarmExtInfo.s_OperateSeqNum); 
		    	SysAlarmEventConfigPtr->s_LinkAlarmExtInfo.s_DelayTime     = NETWORK_TO_HOST_USHORT(SysAlarmEventConfigPtr->s_LinkAlarmExtInfo.s_DelayTime);
		    	SysAlarmEventConfigPtr->s_LinkAlarmExtInfo.s_PtzDelayTime  = NETWORK_TO_HOST_USHORT(SysAlarmEventConfigPtr->s_LinkAlarmExtInfo.s_PtzDelayTime);
		    	Result = m_SystemServiceManager->SvrSetAlarmConfig(SYS_DETECTOR_ID_PIR, 0, SysAlarmEventConfigPtr, sizeof(SysPkgAlarmEventConfig));
		        if (FAILED(Result))
		        {
		            SYS_ERROR("SvrSetAlarmConfig %d fail, Result = 0x%lx\n", SYS_DETECTOR_ID_PIR, Result);
		            MessageCode = RETCODE_ERROR;
		            break;
		        }
		    }
		    else
		    {
			 	SYS_ERROR("AlarmId %d not support, Result = 0x%lx\n", SysAlarmEventConfigPtr->s_AlarmId, Result);
			    MessageCode = RETCODE_NOSUPPORT;
		    	break;
		    }
	    }
	    else 
	    {
	        SYS_ERROR("SysPkgAttrHdPtr->s_Type %d incorrect\n", SysPkgAttrHdPtr->s_Type);
	        MessageCode = RETCODE_ERROR;
	        break;
	    }
    }
    while (0);

    // Reply
    uint16_t PacketHeaderSize  = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    uint16_t PayloadTotalSize  = PacketAttrHdSize1 + sizeof(SdkPkgTransferProtocolKey);
    uint16_t AppPacketSize     = PacketHeaderSize + PacketAttrHdSize1;

    ReferrencePtr<SystemPacket> Reply( BaseMemoryManager::Instance().New<SystemPacket>() );
    if (NULL == Reply.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    //fill packet header
    Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if (FAILED(Result))
    {
        SYS_ERROR("AllocatePacketBuffer fail\n");
        return Result;
    }

    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_SET_ALMCFG_RSP,
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
        SYS_ERROR("FillPacketSdkTransferProtocolKey fail\n");
        return Result;
    }

    m_Reply = Reply;

    SYS_INFO("%s:%s normal out..........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT  SysSetAlarmConfigCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}



