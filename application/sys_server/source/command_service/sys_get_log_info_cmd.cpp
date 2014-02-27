#include "sys_get_log_info_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysGetLogInfoCommandExecutor::SysGetLogInfoCommandExecutor(void)
    : SysBaseCommandExecutor(SYSCODE_GET_LOGINFO_REQ, CT_ShortTask )
{
}


SysGetLogInfoCommandExecutor::~SysGetLogInfoCommandExecutor(void)
{
}


GMI_RESULT SysGetLogInfoCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetLogInfoCommandExecutor> GetLogInfoCommandExecutor(BaseMemoryManager::Instance().New<SysGetLogInfoCommandExecutor>() );
    if (NULL == GetLogInfoCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetLogInfoCommandExecutor->m_Session = Packet->GetSession();
    GetLogInfoCommandExecutor->m_Reply   = Packet->Clone();
    GMI_RESULT Result = GetLogInfoCommandExecutor->SetParameter( m_SystemServiceManager, m_Argument, m_ArgumentSize);
    if (FAILED(Result))
    {
        return Result;
    }

    CommandExecutor = GetLogInfoCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetLogInfoCommandExecutor::Execute()
{
    int32_t           MessageCode     = RETCODE_OK;
    int32_t           LogCount        = 0;
    SystemPacket     *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();
    uint8_t          *PayloadBuff     = CommandPacket->GetPacketPayloadBuffer();
    SysPkgAttrHeader *SysPkgAttrHdPtr = (SysPkgAttrHeader*)PayloadBuff;
    SysPkgLogInfoSearch SysLogInfoSearch;
    SysPkgLogInfoInt    SysLogInfoInt = {0};
	ReferrencePtr<SysPkgLogInfo, DefaultObjectsDeleter> SysLogInfoPtr;	
	
    do
    {
        if (1 != CommandPacket->GetAttrCount())
        {
            MessageCode = RETCODE_ERROR;
            SYS_ERROR("ReqAttrCnt %d Error\n", CommandPacket->GetAttrCount());
            break;
        }

        SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);
        if (SysPkgAttrHdPtr->s_Type != TYPE_LOGINFO_SEARCH)
        {
            MessageCode = RETCODE_ERROR;
            SYS_ERROR("SysPkgAttrHdPtr->s_Type %d error\n", SysPkgAttrHdPtr->s_Type);
            break;
        }
		
        SysPkgLogInfoSearch *SysLogInfoSearchPtr = (SysPkgLogInfoSearch*)(PayloadBuff + sizeof(SysPkgAttrHeader));
		memcpy(&SysLogInfoSearch, SysLogInfoSearchPtr, sizeof(SysPkgLogInfoSearch));
		SysLogInfoSearch.s_SelectMode = NETWORK_TO_HOST_UINT(SysLogInfoSearch.s_SelectMode);
		SysLogInfoSearch.s_MajorType  = NETWORK_TO_HOST_UINT(SysLogInfoSearch.s_MajorType);
		SysLogInfoSearch.s_MinorType  = NETWORK_TO_HOST_UINT(SysLogInfoSearch.s_MinorType);
		SysLogInfoSearch.s_Offset     = NETWORK_TO_HOST_UINT(SysLogInfoSearch.s_Offset);
		SysLogInfoSearch.s_MaxNum     = NETWORK_TO_HOST_UINT(SysLogInfoSearch.s_MaxNum);	
	        	  
		SysLogInfoPtr = BaseMemoryManager::Instance().News<SysPkgLogInfo>(SysLogInfoSearch.s_MaxNum);
        if (NULL == SysLogInfoPtr.GetPtr())
        {
            SYS_ERROR("SysLogInfoPtr news fail\n");
            return GMI_OUT_OF_MEMORY;
        }
        
		//get user log info
		GMI_RESULT Result = m_SystemServiceManager->Query(&SysLogInfoSearch, &SysLogInfoInt, SysLogInfoPtr.GetPtr());
		if (FAILED(Result))
		{
			 MessageCode = RETCODE_ERROR;
             SYS_ERROR("query user log fail, Result = 0x%lx\n", Result);
             break;
		}	

		LogCount = SysLogInfoInt.s_Count;
		SysLogInfoInt.s_Count = HOST_TO_NETWORK_UINT(SysLogInfoInt.s_Count);
		SysLogInfoInt.s_Total = HOST_TO_NETWORK_UINT(SysLogInfoInt.s_Total);
		
		for (int32_t i = 0; i < LogCount; i++)
		{													
			SysLogInfoPtr.GetPtr()[i].s_LogId     = htonll(SysLogInfoPtr.GetPtr()[i].s_LogId);			
			SysLogInfoPtr.GetPtr()[i].s_MajorType = HOST_TO_NETWORK_UINT(SysLogInfoPtr.GetPtr()[i].s_MajorType);
			SysLogInfoPtr.GetPtr()[i].s_MinorType = HOST_TO_NETWORK_UINT(SysLogInfoPtr.GetPtr()[i].s_MinorType);	
		}		        
    }
    while(0);

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
	uint16_t PacketAttrHdSize2;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;
    uint16_t AttrCnt;
	uint16_t AttrCnt2;

    if (MessageCode == RETCODE_OK)
    {
        AttrCnt = 1;
		AttrCnt2 = LogCount;
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgLogInfoInt);
		PacketAttrHdSize2 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgLogInfo);		
    }
    else
    {
        AttrCnt = 1;
        AttrCnt2 = 0;
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    }
    PayloadTotalSize = PacketAttrHdSize1*AttrCnt + PacketAttrHdSize2*AttrCnt2 + sizeof(SdkPkgTransferProtocolKey);
    AppPacketSize    = PacketHeaderSize  + PacketAttrHdSize1*AttrCnt + PacketAttrHdSize2*AttrCnt2;
    ReferrencePtr<SystemPacket> Reply(BaseMemoryManager::Instance().New<SystemPacket>());
    if (NULL == Reply.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GMI_RESULT Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if (FAILED(Result))
    {
        return Result;
    }

    //fill packet header
    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_GET_LOGINFO_RSP,
                 AttrCnt + AttrCnt2,
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
    	int32_t Id;
        for (Id = 0; Id < AttrCnt; Id++)
        {
            SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)(PayloadBuffer + Offset);
            SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_LOGINFO_INT);
            SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
            Offset                   += sizeof(SysPkgAttrHeader);
            memcpy((PayloadBuffer + Offset), &SysLogInfoInt, sizeof(SysPkgLogInfoInt));
            Offset                   += sizeof(SysPkgLogInfoInt);
        }

        for (Id = 0; Id < AttrCnt2; Id++)
        {
        	SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)(PayloadBuffer + Offset);
            SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_LOGINFO);
            SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize2);
            Offset                   += sizeof(SysPkgAttrHeader);
            memcpy((PayloadBuffer + Offset), &SysLogInfoPtr.GetPtr()[Id], sizeof(SysPkgLogInfo));
            Offset                   += sizeof(SysPkgLogInfo);
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

    SYS_INFO("%s normal out........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT  SysGetLogInfoCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}



