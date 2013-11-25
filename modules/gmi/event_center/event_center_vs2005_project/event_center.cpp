
#include "event_center.h"

#include "event_detect_center.h"
#include "event_process_center.h"
#include "log_client.h"

EventCenter::EventCenter(void)
: m_DetectCenter( NULL )
, m_ProcessCenter( NULL )
{
}

EventCenter::~EventCenter(void)
{
}

GMI_RESULT EventCenter::Initialize()
{
	DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "EventCenter::Initialize begin \n" );

	m_ProcessCenter = BaseMemoryManager::Instance().New<EventProcessCenter>();
	if ( NULL == m_ProcessCenter.GetPtr() )
	{
		DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "EventCenter::Initialize, allocating EventProcessCenter object failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
		return GMI_OUT_OF_MEMORY;
	}

	GMI_RESULT Result = m_ProcessCenter->Initialize();
	if ( FAILED( Result ) )
	{
		m_ProcessCenter = NULL;
		DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "EventCenter::Initialize, EventProcessCenter::Initialize return %x \n", (uint32_t) Result );
		return GMI_OUT_OF_MEMORY;
	}

	m_DetectCenter = BaseMemoryManager::Instance().New<EventDetectCenter>();
	if ( NULL == m_DetectCenter.GetPtr() )
	{
		m_ProcessCenter->Deinitialize();
		m_ProcessCenter = NULL;
		DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "EventCenter::Initialize, allocating EventDetectCenter object failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
		return GMI_OUT_OF_MEMORY;
	}

	Result = m_DetectCenter->Initialize();
	if ( FAILED( Result ) )
	{
		m_DetectCenter = NULL;
		m_ProcessCenter->Deinitialize();
		m_ProcessCenter = NULL;
		DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "EventCenter::Initialize, EventDetectCenter::Initialize return %x \n", (uint32_t) Result );
		return GMI_OUT_OF_MEMORY;
	}

	DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "EventCenter::Initialize end, function return %x \n", (uint32_t) Result );
	return Result;
}

GMI_RESULT EventCenter::Deinitialize()
{
	DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "EventCenter::Deinitialize begin \n" );

	GMI_RESULT Result = m_DetectCenter->Deinitialize();
	if ( FAILED( Result ) )
	{
		DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "EventCenter::Deinitialize, EventDetectCenter::Deinitialize return %x \n", (uint32_t) Result );
		return Result;
	}
	m_DetectCenter = NULL;

	Result = m_ProcessCenter->Deinitialize();
	if ( FAILED( Result ) )
	{
		DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "EventCenter::Deinitialize, EventProcessCenter::Deinitialize return %x \n", (uint32_t) Result );
		return Result;
	}
	m_ProcessCenter = NULL;

	DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "EventCenter::Deinitialize end, function return %x \n", (uint32_t) Result );
	return Result;
}
