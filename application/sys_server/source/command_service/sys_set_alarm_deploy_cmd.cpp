#include "log.h"
#include "sys_set_alarm_deploy_cmd.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysSetAlarmDeployCommandExecutor::SysSetAlarmDeployCommandExecutor()
    :  SysBaseCommandExecutor(SYSCODE_SET_ALMDEPLOY_REQ, CT_ShortTask)
{
}


SysSetAlarmDeployCommandExecutor::~SysSetAlarmDeployCommandExecutor()
{
}


GMI_RESULT SysSetAlarmDeployCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysSetAlarmDeployCommandExecutor> SetAlarmDeployCommandExecutor(BaseMemoryManager::Instance().New<SysSetAlarmDeployCommandExecutor>() );
    if (NULL == SetAlarmDeployCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    if (1 != SysPacket->GetAttrCount())
    {
        return GMI_FAIL;
    }

    SetAlarmDeployCommandExecutor->m_Session = Packet->GetSession();
    SetAlarmDeployCommandExecutor->m_Reply   = Packet->Clone();
    SetAlarmDeployCommandExecutor->SetParameter( m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = SetAlarmDeployCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysSetAlarmDeployCommandExecutor::Execute()
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
	    if (TYPE_ALMDEPLOY == SysPkgAttrHdPtr->s_Type)
	    {
	        SysPkgAlarmScheduleTime  *SysAlarmScheduleTimePtr = (SysPkgAlarmScheduleTime*)(PayloadBuff + sizeof(SysPkgAttrHeader));
		
		    SysAlarmScheduleTimePtr->s_ScheduleId = NETWORK_TO_HOST_UINT(SysAlarmScheduleTimePtr->s_ScheduleId);
		    SysAlarmScheduleTimePtr->s_Index  = NETWORK_TO_HOST_UINT(SysAlarmScheduleTimePtr->s_Index);
		    for (int32_t i = 0; i < DAYS_OF_WEEK; i++)
		    {
		        for (int32_t j = 0; j < TIME_SEGMENT_OF_DAY; j++)
		        {
		            SysAlarmScheduleTimePtr->s_ScheduleTime[i][j].s_StartTime = NETWORK_TO_HOST_UINT(SysAlarmScheduleTimePtr->s_ScheduleTime[i][j].s_StartTime);
		            SysAlarmScheduleTimePtr->s_ScheduleTime[i][j].s_EndTime   = NETWORK_TO_HOST_UINT(SysAlarmScheduleTimePtr->s_ScheduleTime[i][j].s_EndTime);
		        }
		    }    	
	    	
	        Result = m_SystemServiceManager->SvrSetAlmScheduleTime(SysAlarmScheduleTimePtr->s_ScheduleId, SysAlarmScheduleTimePtr, sizeof(SysPkgAlarmScheduleTime));
	        if (FAILED(Result))
	        {
	            SYS_ERROR("SvrSetAlmScheduleTime %d fail, Result = 0x%lx\n", SysAlarmScheduleTimePtr->s_ScheduleId, Result);
	            MessageCode = RETCODE_ERROR;
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
                 SYSCODE_SET_ALMDEPLOY_RSP,
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


GMI_RESULT  SysSetAlarmDeployCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}


