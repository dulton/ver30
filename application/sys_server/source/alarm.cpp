#include <time.h>
#include "alarm.h"
#include "ipc_fw_v3.x_resource.h"
#include "event_process_header.h"
#include "log.h"

Alarm::Alarm()
{
}


Alarm::~Alarm()
{
}


GMI_RESULT Alarm::Initialize()
{	
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

    Result = m_EventCenter.Start(NULL, 0, EventProcess, this);
    if (FAILED(Result))
    {
    	m_AlarmSession->Deinitialize();
    	SYS_ERROR("event center start fail, Result = 0x%lx\n", Result);
    	return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT Alarm::Deinitialize()
{
	m_EventCenter.Stop();
	m_AlarmSession->Deinitialize();
	m_AlarmSession = NULL;

	return GMI_SUCCESS;
}


GMI_RESULT Alarm::Report(SysPkgAlarmInfor *SysAlarmInfor)
{	
	size_t Transfered;
	GMI_RESULT Result = m_AlarmSession->Send((uint8_t*)SysAlarmInfor, sizeof(SysPkgAlarmInfor), &Transfered);
	if (FAILED(Result))
	{
		SYS_ERROR("send alarm info to sdk fail, Result = 0x%lx\n", Result);
		return Result;
	}

	return GMI_SUCCESS;
}



void Alarm::EventProcess(void_t *UserData, uint32_t EventId, enum EventType Type, void_t *Parameter, size_t ParameterLength)
{
	struct timeval CurrentTime;
    gettimeofday( &CurrentTime, NULL );
    SYS_INFO( "EventProcess: UserData=%p, EventId=%d, Type=%d, Parameter=%p, ParameterLength=%d, current time=%ld:%06ld \n", UserData, EventId, Type, Parameter, ParameterLength, CurrentTime.tv_sec, CurrentTime.tv_usec );
	return;
}



