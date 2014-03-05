#include "log.h"
#include "sys_alarm_info_report.h"
#include "ipc_fw_v3.x_resource.h"


SysAlarmInfoReport::SysAlarmInfoReport()
{
}


SysAlarmInfoReport::~SysAlarmInfoReport()
{
}


GMI_RESULT SysAlarmInfoReport::Initialize()
{
	m_PackageSize = sizeof(SysPkgHeader) + sizeof(SysPkgAttrHeader) + sizeof(SysPkgAlarmInfor);
	m_Package = BaseMemoryManager::Instance().News<uint8_t>(m_PackageSize);
	if (NULL == m_Package.GetPtr())
	{
		SYS_ERROR("alarm: m_Package news fail, len %d\n", m_PackageSize);
		return GMI_OUT_OF_MEMORY;
	}		

	m_AlarmSession = BaseMemoryManager::Instance().New<AlarmSession>(GMI_SYS_SERVER_C_WARING_PORT, GMI_SDK_S_WARING_PORT);
    if (NULL == m_AlarmSession.GetPtr())
    {
    	SYS_ERROR("m_AlarmSession new fail\n");
    	return GMI_OUT_OF_MEMORY;
    }

    GMI_RESULT Result = m_AlarmSession->Initialize();
    if (FAILED(Result))
    {
    	SYS_ERROR("Alarm Session Initialize fail, Result = 0x%lx\n", Result);
    	return Result;
    }   

    return GMI_SUCCESS;
}


GMI_RESULT SysAlarmInfoReport::Deinitialize()
{
	m_AlarmSession->Deinitialize();
	m_AlarmSession = NULL;
	return GMI_SUCCESS;
}


GMI_RESULT SysAlarmInfoReport::GSMT_Pakage(SysPkgAlarmInfor *SysAlarmInfor, uint8_t *Buffer, size_t BufferSize, size_t *TotalLen)
{
	memset(Buffer, 0, BufferSize);
	SysPkgHeader *Header = (SysPkgHeader*)Buffer;
	memcpy(Header->s_SysMsgTag, GMI_SYS_MESSAGE_TAG, GMI_SYS_MESSAGE_TAG_LENGTH);
	Header->s_AttrCount = HOST_TO_NETWORK_USHORT(1);
	Header->s_Code      = HOST_TO_NETWORK_USHORT(SYSCODE_GET_ALARM_INFO_RSP);
	Header->s_HeaderLen = sizeof(SysPkgHeader);
	Header->s_SeqNum    = HOST_TO_NETWORK_USHORT(1);
	Header->s_SessionId = HOST_TO_NETWORK_USHORT(1);
	Header->s_Version   = SYS_COMM_VERSION;
	Header->s_TotalLen  = HOST_TO_NETWORK_USHORT(sizeof(SysPkgHeader) + sizeof(SysPkgAttrHeader) + sizeof(SysPkgAlarmInfor));

	SysPkgAttrHeader *AttrHeader = (SysPkgAttrHeader*)(Buffer + sizeof(SysPkgHeader));
	AttrHeader->s_Length         = HOST_TO_NETWORK_USHORT(sizeof(SysPkgAttrHeader) + sizeof(SysPkgAlarmInfor));
	AttrHeader->s_Type           = HOST_TO_NETWORK_USHORT(TYPE_WARING_INFO);

	SysPkgAlarmInfor *AlarmInfoBody = (SysPkgAlarmInfor*)(Buffer +  sizeof(SysPkgHeader) + sizeof(SysPkgAttrHeader));
	memcpy(AlarmInfoBody, SysAlarmInfor, sizeof(SysPkgAlarmInfor));
	AlarmInfoBody->s_WaringId = htonll(AlarmInfoBody->s_WaringId);
	AlarmInfoBody->s_WaringType = HOST_TO_NETWORK_UINT(AlarmInfoBody->s_WaringType);
	AlarmInfoBody->s_WaringLevel = HOST_TO_NETWORK_UINT(AlarmInfoBody->s_WaringLevel);
	AlarmInfoBody->s_OnOff = HOST_TO_NETWORK_UINT(AlarmInfoBody->s_OnOff);
	AlarmInfoBody->s_ExtraInfo.s_IoNum = HOST_TO_NETWORK_UINT(AlarmInfoBody->s_ExtraInfo.s_IoNum);
	*TotalLen = BufferSize;

	return GMI_SUCCESS;
}


void SysAlarmInfoReport::Report(SysPkgAlarmInfor * SysAlarmInfor)
{
	size_t TotalLen;
	GMI_RESULT Result = GSMT_Pakage(SysAlarmInfor, m_Package.GetPtr(), m_PackageSize, &TotalLen);
	if (FAILED(Result))
	{
		SYS_ERROR("GSMT_Pakage fail, Result = 0x%lx\n", Result);
		return;
	}

	size_t Transferred;
	Result = m_AlarmSession->Send(m_Package.GetPtr(), TotalLen, &Transferred);
	if (FAILED(Result))
	{
		SYS_ERROR("send alarm info to sdk_server fail, Result = 0x%lx", Result);
		return;
	}
	
	return;
}


