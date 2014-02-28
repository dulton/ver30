#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>
#include "log.h"
#include "system_service_manager.h"
#include "factory_setting_operation.h"
#include "daemon.h"
#include "ipc_fw_v3.x_setting.h"
#include "ipc_fw_v3.x_resource.h"
#include "gmi_system_headers.h"
#include "gmi_daemon_heartbeat_api.h"
#include "gmi_media_ctrl.h"
#include "stream_control.h"
#include "rtsp_stream_control.h"
#include "des.h"

SystemServiceManager::SystemServiceManager(void)
    : m_ThreadExitFlag(false)
    , m_ServerMonitorThread()
    , m_ConfigFileManagerPtr()
    , m_PtzControlPtr()
    , m_StreamCenterClientPtr()
    , m_BoardManagerPtr()
    , m_UserManagerPtr()
    , m_MediaLocalClientPort(GMI_MEDIA_CENTER_CLIENT_COMMAND_PORT)
    , m_MediaLocalServerPort(GMI_MEDIA_CENTER_SERVER_COMMAND_PORT)
    , m_RTSP_ServerPort(GMI_RTSP_SERVER_TCP_PORT)
    , m_UDPSessionBufferSize(8192)
    , m_VideoStreamNum(VIDEO_ENCODE_STREAM_NUM)
    , m_VideoEncParamPtr()
    , m_VideoCodecHandle()
    , m_AudioEncParamPtr()
    , m_VideoInOutHandle()
    , m_ImageHandle()
    , m_VideoOsdParamPtr()
    , m_ImageParamPtr()
    , m_ImageAdvanceParamPtr()
    , m_VideoSourcePtr()
    , m_SupportPtz(false)
    , m_PresetsInfo_InnerPtr()
    , m_FocusMode(0)
{
}


SystemServiceManager::~SystemServiceManager()
{
}

GMI_RESULT SystemServiceManager::Initialize()
{
    SYS_INFO("#%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "#%s in..........\n", __func__);
    GMI_RESULT Result = GMI_SUCCESS;

    //codec initialize
    //IPC files including capabilities 、resource、user default config、user config
    //should get media local port、media server port、RTSP port、Session buffer size from resource file
    //should get video encode parameter from user config file

    Result = OsalResourceInitial();
    if (FAILED(Result))
    {
        SYS_ERROR("OsalResourceInitial fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsalResourceInitial fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = MediaInitial();
    if (FAILED(Result))
    {
        SYS_ERROR("MediaInitial fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaInitial fail, Result = 0x%lx\n", Result);
        OsalResourceDeinitial();
        return Result;
    }

    Result = MediaParamLoad();
    if (FAILED(Result))
    {
        MediaDeinitial();
        OsalResourceDeinitial();
        SYS_ERROR("MediaParamConfig fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaParamConfig fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = PTZ_Initial();
    if (FAILED(Result))
    {
        MediaParamUnLoad();
        MediaDeinitial();
        OsalResourceDeinitial();
        SYS_ERROR("PTZ_Initial fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PTZ_Initial fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = MiscInitial();
    if (FAILED(Result))
    {
        PTZ_Deinitial();
        MediaParamUnLoad();
        MediaDeinitial();
        OsalResourceDeinitial();
        SYS_ERROR("MiscInitial fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaParamConfig fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SYS_INFO("#%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "#%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::Deinitialize()
{
    SYS_INFO("#%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "#%s in..........\n", __func__);

    GMI_RESULT Result;

    Result = MiscDeinitial();
    if (FAILED(Result))
    {
        return Result;
    }

    Result = PTZ_Deinitial();
    if (FAILED(Result))
    {
        return Result;
    }

    Result = MediaParamUnLoad();
    if (FAILED(Result))
    {
        return Result;
    }

    Result = MediaDeinitial();
    if (FAILED(Result))
    {
        return Result;
    }

    Result = OsalResourceDeinitial();
    if (FAILED(Result))
    {
        return Result;
    }

    SYS_INFO("#%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "#%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::AlarmInitial()
{
	m_AlarmPtr = BaseMemoryManager::Instance().New<Alarm>();
	if (NULL == m_AlarmPtr.GetPtr())
	{		
		SYS_ERROR("new alarm fail\n");
		return GMI_OUT_OF_MEMORY;
	}

	GMI_RESULT Result = m_AlarmPtr->Initialize();
	if (FAILED(Result))
	{
		SYS_ERROR("alarm initial fail, Result = 0x%lx\n", Result);
		return Result;
	}	

	//get alarm in config
	for (int32_t i = 0; i < MAX_ALARM_IN_PORT; i++)
	{
		Result = m_ConfigFileManagerPtr->GetAlarmConfig(SYS_DETECTOR_ID_ALARM_INPUT, i, &m_SysAlarmInCfg[i], sizeof(SysPkgAlarmInConfig));
		if (FAILED(Result))
		{		
			SYS_ERROR("GetAlarmInConfig from file fail, Result = 0x%lx\n", Result);
			m_AlarmPtr->Deinitialize();
			return Result;
		}
	}

	//get alarm out config
	for (int32_t i = 0; i < MAX_ALARM_OUT_PORT; i++)
	{
		Result = m_ConfigFileManagerPtr->GetAlarmConfig(SYS_PROCESSOR_ID_ALARM_OUTPUT, i, &m_SysAlarmOutCfg[i], sizeof(SysPkgAlarmOutConfig));
		if (FAILED(Result))
		{		
			SYS_ERROR("GetAlarmOutConfig from file fail, Result = 0x%lx\n", Result);
			m_AlarmPtr->Deinitialize();
			return Result;
		}
	}

	//get PIR config
	Result = m_ConfigFileManagerPtr->GetAlarmConfig(SYS_DETECTOR_ID_PIR, 0, &m_SysAlarmPIRCfg, sizeof(SysPkgAlarmEventConfig));
	if (FAILED(Result))
	{		
		SYS_ERROR("GetAlarmPIRConfig from file fail, Result = 0x%lx\n", Result);
		m_AlarmPtr->Deinitialize();
		return Result;
	}

	//get alarm in schedule 
	for (int32_t i = 0; i < MAX_ALARM_IN_PORT; i++)
	{		
		Result = m_ConfigFileManagerPtr->GetAlarmSchedule(SYS_SCHEDULE_TIME_ID_ALARM_IN, i, &m_SysAlarmInScheduleTime[i]);
		if (FAILED(Result))
		{		
			SYS_ERROR("GetAlarmInSchedule from file fail, Result = 0x%lx\n", Result);
			m_AlarmPtr->Deinitialize();
			return Result;
		}		
	}

	//get alarm out schedule
	for (int32_t i = 0; i < MAX_ALARM_OUT_PORT; i++)
	{		
		Result = m_ConfigFileManagerPtr->GetAlarmSchedule(SYS_SCHEDULE_TIME_ID_ALARM_OUT, i, &m_SysAlarmOutScheduleTime[i]);
		if (FAILED(Result))
		{		
			SYS_ERROR("GetAlarmOutSchedule from file fail, Result = 0x%lx\n", Result);
			m_AlarmPtr->Deinitialize();
			return Result;
		}		
	}

	//get PIR schedule 
	Result = m_ConfigFileManagerPtr->GetAlarmSchedule(SYS_SCHEDULE_TIME_ID_PIR_DETECT, 0, &m_SysAlarmPIRScheduleTime);
	if (FAILED(Result))
	{		
		SYS_ERROR("GetAlarmPIRSchedule from file fail, Result = 0x%lx\n", Result);
		m_AlarmPtr->Deinitialize();
		return Result;
	}	

 	////Set To Alarm
	//set alarm in config 
	for (int32_t i = 0; i < MAX_ALARM_IN_PORT; i++)
	{
		Result = m_AlarmPtr->Config(SYS_DETECTOR_ID_ALARM_INPUT, i, &m_SysAlarmInCfg[i], sizeof(SysPkgAlarmInConfig));
		if (FAILED(Result))
		{		
			SYS_ERROR("SetAlarmInConfig to event module fail, Result = 0x%lx\n", Result);
			m_AlarmPtr->Deinitialize();
			return Result;
		}
	}
	
	//set alarm out config
	for (int32_t i = 0; i < MAX_ALARM_OUT_PORT; i++)
	{
		Result = m_AlarmPtr->Config(SYS_PROCESSOR_ID_ALARM_OUTPUT, i, &m_SysAlarmOutCfg[i], sizeof(SysPkgAlarmOutConfig));
		if (FAILED(Result))
		{		
			SYS_ERROR("SetAlarmOutConfig to event module fail, Result = 0x%lx\n", Result);
			m_AlarmPtr->Deinitialize();
			return Result;
		}
	}	

	//set PIR config
	Result = m_AlarmPtr->Config(SYS_DETECTOR_ID_PIR, 0, &m_SysAlarmPIRCfg, sizeof(SysPkgAlarmEventConfig));
	if (FAILED(Result))
	{		
		SYS_ERROR("SetAlarmPIRConfig to event module fail, Result = 0x%lx\n", Result);
		m_AlarmPtr->Deinitialize();
		return Result;
	}
	
	//set alarm in schedule
	for (int32_t i = 0; i < MAX_ALARM_IN_PORT; i++)
	{
		Result = m_AlarmPtr->Schedule(SYS_SCHEDULE_TIME_ID_ALARM_IN, i, &m_SysAlarmInScheduleTime[i], sizeof(SysPkgAlarmScheduleTime));
		if (FAILED(Result))
		{		
			SYS_ERROR("SetAlarmInSchedule to event module fail, Result = 0x%lx\n", Result);
			m_AlarmPtr->Deinitialize();
			return Result;
		}
	}

	//set alarm out schedule
	for (int32_t i = 0; i < MAX_ALARM_OUT_PORT; i++)
	{
		Result = m_AlarmPtr->Schedule(SYS_SCHEDULE_TIME_ID_ALARM_OUT, i, &m_SysAlarmOutScheduleTime[i], sizeof(SysPkgAlarmScheduleTime));
		if (FAILED(Result))
		{		
			SYS_ERROR("SetAlarmOutSchedule to event module fail, Result = 0x%lx\n", Result);
			m_AlarmPtr->Deinitialize();
			return Result;
		}
	}
	
	//set PIR schedule
	Result = m_AlarmPtr->Schedule(SYS_SCHEDULE_TIME_ID_PIR_DETECT, 0, &m_SysAlarmPIRScheduleTime, sizeof(SysPkgAlarmScheduleTime));
	if (FAILED(Result))
	{		
		SYS_ERROR("SetAlarmPIRSchedule to event module fail, Result = 0x%lx\n", Result);
		m_AlarmPtr->Deinitialize();
		return Result;
	}	
	
	return GMI_SUCCESS;
}

GMI_RESULT SystemServiceManager::AlarmDeinitial()
{
	GMI_RESULT Result = m_AlarmPtr->Deinitialize();
	if (FAILED(Result))
	{
		SYS_ERROR("alarm Deinitial fail, Result = 0x%lx\n", Result);
		return Result;
	}

	return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::MiscInitial()
{
    SYS_INFO("##%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "##%s in..........\n", __func__);

    GMI_RESULT Result;

    //user
    m_UserManagerPtr = BaseMemoryManager::Instance().New<UserManager>();
    if (NULL == m_UserManagerPtr.GetPtr())
    {
        SYS_ERROR("m_UserManagerPtr new fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_UserManagerPtr new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    //user initialize
    Result = m_UserManagerPtr->Initialize(GMI_USERS_FILE_NAME, GMI_USERS_TABLE_NAME);
    if (FAILED(Result))
    {
        m_UserManagerPtr = NULL;
        SYS_ERROR("Initialize fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Initialize fail, Result = 0x%lx\n", Result);
        return Result;
    }

    //add default admin
    Result = m_UserManagerPtr->AddDefaultAdmin();
    if (FAILED(Result))
    {
        m_UserManagerPtr = NULL;
        SYS_ERROR("AddDefaultAdmin fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "AddDefaultAdmin fail, Result = 0x%lx\n", Result);
    }

    //Board
    m_BoardManagerPtr = BaseMemoryManager::Instance().New<BoardManager>();
    if (NULL == m_BoardManagerPtr.GetPtr())
    {
        m_UserManagerPtr->Deinitialize();
        m_UserManagerPtr = NULL;
        SYS_ERROR("m_BoardManagerPtr new fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_BoardManagerPtr new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    //board inilial
    Result = m_BoardManagerPtr->Initialize();
    if (FAILED(Result))
    {
        m_BoardManagerPtr = NULL;
        m_UserManagerPtr->Deinitialize();
        m_UserManagerPtr = NULL;
        SYS_ERROR("m_BoardManagerPtr Initialize fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_BoardManagerPtr Initialize fail, Result = 0x%lx\n", Result);
        return GMI_OUT_OF_MEMORY;
    }

    //user log initial
    Result = UserLogQuery::Initialize();
    if (FAILED(Result))
    {
    	m_BoardManagerPtr = NULL;
        m_UserManagerPtr->Deinitialize();
        m_UserManagerPtr = NULL;
    	SYS_ERROR("UserLogQuery::Initialize fail, Result = 0x%lx\n", Result);
    	return Result;
    }

	//alarm init
	Result = AlarmInitial();
	if (FAILED(Result))
	{
		UserLogQuery::Deinitialize();
		m_BoardManagerPtr = NULL;
        m_UserManagerPtr->Deinitialize();
        m_UserManagerPtr = NULL;
		SYS_ERROR("AlarmInitial fail, Result = 0x%lx\n", Result);
		return Result;
	}		
	
    //set time type and ntp server ip
    SysPkgDateTimeType SysTimeType;
    memset(&SysTimeType, 0, sizeof(SysPkgDateTimeType));
    Result = m_ConfigFileManagerPtr->GetSysTimeType(&SysTimeType);
    if (FAILED(Result))
    {
        SYS_ERROR("GetSysTimeType fail from config file, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "GetSysTimeType fail from config file, Result = 0x%lx\n", Result);
    }

    boolean_t NtpEnable = ((SYS_TIME_TYPE_NTP == SysTimeType.s_Type) ? true : false);
    Result = m_BoardManagerPtr->SetNtp(NtpEnable, SysTimeType.s_NtpInterval);
    if (FAILED(Result))
    {
    	AlarmDeinitial();
    	UserLogQuery::Deinitialize();
        m_BoardManagerPtr = NULL;
        m_UserManagerPtr->Deinitialize();
        m_UserManagerPtr = NULL;
        SYS_ERROR("SetNtp fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetNtp fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SysPkgNtpServerInfo SysNtpServerInfo;
    memset(&SysNtpServerInfo, 0, sizeof(SysPkgNtpServerInfo));
    Result = m_ConfigFileManagerPtr->GetNtpServerInfo(&SysNtpServerInfo);
    if (FAILED(Result))
    {
        SYS_ERROR("GetNtpServerInfo fail from config file, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "GetNtpServerInfo fail from config file, Result = 0x%lx\n", Result);
    }

    Result = m_BoardManagerPtr->SetNtpServer(SysNtpServerInfo.s_NtpAddr_1);
    if (FAILED(Result))
    {
    	AlarmDeinitial();
    	UserLogQuery::Deinitialize();
        m_BoardManagerPtr = NULL;
        m_UserManagerPtr->Deinitialize();
        m_UserManagerPtr = NULL;
        SYS_ERROR("m_BoardManagerPtr SetNtpServer fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_BoardManagerPtr SetNtpServer fail, Result = 0x%lx\n", Result);
        return GMI_FAIL;
    }

    //get network port
    memset(&m_SysNetWorkPort, 0, sizeof(SysPkgNetworkPort));
    Result = m_ConfigFileManagerPtr->GetExternNetworkPort(&m_SysNetWorkPort);
    if (FAILED(Result))
    {
    	AlarmDeinitial();
    	UserLogQuery::Deinitialize();
    	m_BoardManagerPtr = NULL;
        m_UserManagerPtr->Deinitialize();
        m_UserManagerPtr = NULL;
    	SYS_ERROR("get extern network port failed from config file, Result = 0x%lx\n", Result);
    	DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "get extern network port failed from config file, Result = 0x%lx\n", Result);
    	return Result;
    }

	//get device info
	memset(&m_SysDeviceInfo, 0, sizeof(SysPkgDeviceInfo));
	Result = m_ConfigFileManagerPtr->GetDeviceInfo(&m_SysDeviceInfo);
	if (FAILED(Result))
	{
		AlarmDeinitial();
		UserLogQuery::Deinitialize();
		m_BoardManagerPtr = NULL;
        m_UserManagerPtr->Deinitialize();
        m_UserManagerPtr = NULL;
    	SYS_ERROR("get device info failed from config file, Result = 0x%lx\n", Result);
    	DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "get device info failed from config file, Result = 0x%lx\n", Result);
		return Result;
	}

	//get ntp server info
	memset(&m_SysNtpServerInfo, 0, sizeof(SysPkgNtpServerInfo));
    Result = m_ConfigFileManagerPtr->GetNtpServerInfo(&m_SysNtpServerInfo);
    if (FAILED(Result))
    {
    	AlarmDeinitial();
    	UserLogQuery::Deinitialize();
    	m_BoardManagerPtr = NULL;
        m_UserManagerPtr->Deinitialize();
        m_UserManagerPtr = NULL;
    	SYS_ERROR("get ntp server info fail, Result = 0x%lx\n", Result);
    	DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "get ntp server info fail, Result = 0x%lx\n", Result);
    	return Result;
    }

	//get device software capability
	m_CapabilitiesMessagePtr = BaseMemoryManager::Instance().News<char_t>(MAX_MESSAGE_LENGTH);
	if (NULL == m_CapabilitiesMessagePtr.GetPtr())
	{
		AlarmDeinitial();
		UserLogQuery::Deinitialize();
		m_BoardManagerPtr = NULL;
        m_UserManagerPtr->Deinitialize();
        m_UserManagerPtr = NULL;
    	SYS_ERROR("m_CapabilitiesMessagePtr new %d fail\n", MAX_MESSAGE_LENGTH);
    	DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_CapabilitiesMessagePtr new %d fail\n", MAX_MESSAGE_LENGTH);
		return GMI_OUT_OF_MEMORY;
	}

	memset(m_CapabilitiesMessagePtr.GetPtr(), 0, MAX_MESSAGE_LENGTH);
	memset(&m_SysCapability, 0, sizeof(SysPkgXml));
	Result = m_ConfigFileManagerPtr->GetCapabilities(MAX_MESSAGE_LENGTH, m_CapabilitiesMessagePtr.GetPtr(), &m_SysCapability);
	if (FAILED(Result))
	{
		AlarmDeinitial();
		UserLogQuery::Deinitialize();
		memset(&m_SysCapability, 0, sizeof(SysPkgXml));
		m_CapabilitiesMessagePtr = NULL;
		m_BoardManagerPtr = NULL;
        m_UserManagerPtr->Deinitialize();
        m_UserManagerPtr = NULL;
    	SYS_ERROR("GetCapabilities fail, %d\n", MAX_MESSAGE_LENGTH);
    	DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "GetCapabilities fail, %d\n", MAX_MESSAGE_LENGTH);		
	}
	
    SYS_INFO("##%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "##%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::MiscDeinitial()
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    GMI_RESULT Result = AlarmDeinitial();
    if (FAILED(Result))
    {
    	SYS_ERROR("AlarmDeinitial fail, Result = 0x%lx\n", Result);
    	DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "AlarmDeinitial fail, Result = 0x%lx\n", Result);
    	return Result;
    }
    
	Result = UserLogQuery::Deinitialize();
	if (FAILED(Result))
	{
		SYS_ERROR("UserLogQuery::Deinitialize() fail, Result = 0x%lx\n", Result);
		DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "UserLogQuery::Deinitialize() fail, Result = 0x%lx\n", Result);
		return Result;
	}
	
    Result = m_BoardManagerPtr->Deinitialize();
    if (FAILED(Result))
    {
        SYS_ERROR("m_BoardManagerPtr Deinitialize fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_BoardManagerPtr Deinitialize fail, Result = 0x%lx\n", Result);
        return Result;
    }

    m_BoardManagerPtr = NULL;

    Result = m_UserManagerPtr->Deinitialize();
    if (FAILED(Result))
    {
        SYS_ERROR("m_UserManagerPtr Deinitialize fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_UserManagerPtr Deinitialize fail, Result = 0x%lx\n", Result);
        return Result;
    }
    m_UserManagerPtr = NULL;

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::OsalResourceInitial(void)
{
    SYS_INFO("##%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "##%s in..........\n", __func__);

    //read&write lock initialize
    int32_t Ret = pthread_rwlock_init(&m_Lock, NULL);
    if (Ret < 0)
    {
        SYS_ERROR("pthread_rwlock_init fail, Ret = 0x%x\n", Ret);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "pthread_rwlock_init fail, Ret = 0x%x\n", Ret);
        return GMI_FAIL;
    }

    SYS_INFO("##%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "##%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::OsalResourceDeinitial(void)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    //read&write lock initialize
    GMI_RESULT Result = pthread_rwlock_destroy(&m_Lock);
    if (Result < 0)
    {
        SYS_ERROR("pthread_rwlock_destroy fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "pthread_rwlock_destroy fail, Result = 0x%lx\n", Result);
        return GMI_FAIL;
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


void_t* SystemServiceManager::MaintainSystemThread(void * Argument)
{
	SystemServiceManager *Manager = reinterpret_cast<SystemServiceManager*> (Argument);
    return Manager->MaintainSystem();
}


void_t* SystemServiceManager::MaintainSystem(void)
{
	while (1)
	{
		GMI_Sleep(VIDIN_BLOCKED_TIMES*1000);
		
		if (m_VidInBlocked)
		{
			SYS_ERROR("SetVidInVidOut is blocked, so should reboot\n");
			DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetVidInVidOut is blocked, so should reboot\n");
			GMI_RESULT Result = DaemonReboot();
			if (FAILED(Result))
			{
				DaemonReboot();
			}
		}
		else
		{
			SYS_INFO("break from MaintainSystem!!!\n");
			DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "MaintainSystem normal exit!!!\n");
			break;
		}
	}
	
	return (void *) GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::StartStreamMonitor()
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    m_ThreadExitFlag  = false;
    GMI_RESULT Result = m_ServerMonitorThread.Create(NULL, 0, ServerMonitorThread, this);
    if (FAILED(Result))
    {
        SYS_ERROR("m_ServerMonitorThread.Create fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ServerMonitorThread.Create fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_ServerMonitorThread.Start();
    if (FAILED(Result))
    {
        m_ServerMonitorThread.Destroy();
        SYS_ERROR("m_ServerMonitorThread.Start fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ServerMonitorThread.Start fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::StopStreamMonitor()
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    SYS_INFO("ServerMonitor is exiting ......\n");
    m_ThreadExitFlag  = true;
    while (true == m_ThreadExitFlag)
    {
        GMI_Sleep(500);
    }
    SYS_INFO("ServerMonitor have exited ......\n");

    GMI_RESULT Result = m_ServerMonitorThread.Destroy();
    if (FAILED(Result))
    {
        SYS_ERROR("m_ServerMonitorThread.Destroy() fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ServerMonitorThread.Destroy() fail, Result = 0x%lx\n", Result);
        return GMI_FAIL;
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::MediaInitial(void)
{
    SYS_INFO("##%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "##%s in..........\n", __func__);

    int32_t    Id;
    GMI_RESULT Result = GMI_SUCCESS;

    m_ConfigFileManagerPtr = BaseMemoryManager::Instance().New<ConfigFileManager>();
    if (NULL == m_ConfigFileManagerPtr.GetPtr())
    {
        SYS_ERROR("m_ConfigFileManagerPtr new fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ConfigFileManagerPtr new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    //configFileManager initialize
    Result = m_ConfigFileManagerPtr->Initialize();
    if (FAILED(Result))
    {
        SYS_ERROR("Initialize fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Initialize fail, Result = 0x%lx\n", Result);
        return Result;
    }

    //get video stream num
    Result = m_ConfigFileManagerPtr->GetVideoEncodeStreamNum(&m_VideoStreamNum);
    if (FAILED(Result))
    {
        SYS_ERROR("GetVideoEncodeStreamNum fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetVideoEncodeStreamNum fail, Result = 0x%lx\n", Result);
        m_ConfigFileManagerPtr->Deinitialize();
        return Result;
    }

    if (m_VideoStreamNum > MAX_VIDEO_STREAM_NUM)
    {
        SYS_ERROR("m_VideoStreamNum %d error\n", m_VideoStreamNum);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_VideoStreamNum %d error\n", m_VideoStreamNum);
        m_ConfigFileManagerPtr->Deinitialize();
        return Result;
    }

    //allocate VideoEncodeParam
    m_VideoEncParamPtr = BaseMemoryManager::Instance().News<VideoEncodeParam>(MAX_VIDEO_STREAM_NUM);
    if (NULL == m_VideoEncParamPtr.GetPtr())
    {
        SYS_ERROR("m_VideoEncParamPtr news fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_VideoEncParamPtr news fail\n");
        m_ConfigFileManagerPtr->Deinitialize();
        return GMI_OUT_OF_MEMORY;
    }
    memset(m_VideoEncParamPtr.GetPtr(), 0, sizeof(VideoEncodeParam)*MAX_VIDEO_STREAM_NUM);

    m_VideoStreamTypePtr = BaseMemoryManager::Instance().News<int32_t>(MAX_VIDEO_STREAM_NUM);
    if (NULL == m_VideoStreamTypePtr.GetPtr())
    {
        SYS_ERROR("m_VideoStreamTypePtr news fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_VideoStreamTypePtr news fail\n");
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return GMI_OUT_OF_MEMORY;
    }
    memset(m_VideoStreamTypePtr.GetPtr(), VIDEO_ENCODE_STREAM_TYPE, sizeof(int32_t)*MAX_VIDEO_STREAM_NUM);

    //allocate video handle
    m_VideoCodecHandle    = BaseMemoryManager::Instance().News<FD_HANDLE>(MAX_VIDEO_STREAM_NUM);
    if (NULL == m_VideoCodecHandle.GetPtr())
    {
        SYS_ERROR("m_VideoCodecHandle News fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_VideoCodecHandle News fail\n");
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return GMI_OUT_OF_MEMORY;
    }

    //get stream combine
    SysPkgEncStreamCombine SysEncStreamCombine;
    Result = m_ConfigFileManagerPtr->GetStreamCombine(&SysEncStreamCombine);
    if (FAILED(Result))
    {
        SYS_ERROR("get video stream combine fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "get video stream combine fail, Result = 0x%lx\n", Result);
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return Result;
    }

    //get video stream settings
    Result = m_ConfigFileManagerPtr->GetVideoEncodeSettings(0xff, &SysEncStreamCombine, m_VideoEncParamPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("GetVideoEncodeSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetVideoEncodeSettings fail, Result = 0x%lx\n", Result);
        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return Result;
    }

    for (int32_t Id = 0; Id < MAX_VIDEO_STREAM_NUM; Id++)
    {
        Result = m_ConfigFileManagerPtr->GetVideoStreamType(Id, &((m_VideoStreamTypePtr.GetPtr())[Id]));
        if (FAILED(Result))
        {
            SYS_ERROR("GetVideoStreamType fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetVideoStreamType fail, Result = 0x%lx\n", Result);
            m_VideoCodecHandle = NULL;
            m_VideoStreamTypePtr = NULL;
            m_VideoEncParamPtr = NULL;
            m_ConfigFileManagerPtr->Deinitialize();
            return Result;
        }
    }

    //allocate StreamCenterClient
    m_StreamCenterClientPtr = BaseMemoryManager::Instance().New<StreamCenterClient>();
    if (NULL == m_StreamCenterClientPtr.GetPtr())
    {
        SYS_ERROR("m_StreamCenterClientPtr new fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_StreamCenterClientPtr new fail\n");
        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return GMI_OUT_OF_MEMORY;
    }

    //StreamCenterClient Initialize
    Result = m_StreamCenterClientPtr->Initialize(m_MediaLocalClientPort, m_MediaLocalServerPort, m_RTSP_ServerPort, m_UDPSessionBufferSize);
    if (FAILED(Result))
    {
        SYS_ERROR("Initialize fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Initialize fail, Result = 0x%lx\n", Result);
        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return Result;
    }

    //set general param;
    SysPkgComponents SysComponents;
    memset(&SysComponents, 0, sizeof(SysPkgComponents));
    Result = m_ConfigFileManagerPtr->GetHwAutoDetectInfo(&SysComponents);
    if (FAILED(Result))
    {
    	SYS_ERROR("GetHwAutoDetectInfo fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetHwAutoDetectInfo fail, Result = 0x%lx\n", Result);
        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return Result;
    }    
    
    Result = m_StreamCenterClientPtr->GeneralParamSet(GEN_PARAM_AUTO, (void_t*)&SysComponents);
    if (FAILED(Result))
    {
    	SYS_ERROR("GeneralParamSet GEN_PARAM_AUTO fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GeneralParamSet GEN_PARAM_AUTO fail, Result = 0x%lx\n", Result);
        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return Result;
    }

	GeneralParam_Ircut FacotryIrcut;
    memset(&FacotryIrcut, 0, sizeof(GeneralParam_Ircut));
    Result = m_ConfigFileManagerPtr->GetFactoryIrcut(&FacotryIrcut);
    if (FAILED(Result))
    {
    	SYS_ERROR("GetFactoryIrcut fail, Result = 0x%lx. system also start\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "GetFactoryIrcut fail, Result = 0x%lx. system also start\n", Result);        
    }
    else
    {
	    Result = m_StreamCenterClientPtr->GeneralParamSet(GEN_PARAM_IRCUT, (void_t*)&FacotryIrcut);
		if (FAILED(Result))
	    {
	    	SYS_ERROR("GeneralParamSet GEN_PARAM_IRCUT fail, Result = 0x%lx. system also start\n", Result);
	        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "GeneralParamSet GEN_PARAM_IRCUT fail, Result = 0x%lx\n. system also start\n", Result);	        
	    }
    }

	//14/2/12. guoqiang.lu:it unlikely appears "wait for bootup" that lead to IPC is blocked, when upgrade from FW2.0->FW3.0.
	//So, we create maintenance thread to monitor this blocked situation.
	Result = m_MaintainSystemThread.Create(NULL, 0, MaintainSystemThread, this);
	if (FAILED(Result))
	{
		SYS_ERROR("m_MaintainSystemThread.Create fail, Result = 0x%lx\n", Result);
		DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MaintainSystemThread.Create fail, Result = 0x%lx\n", Result);
		m_StreamCenterClientPtr->Deinitialize();
        m_StreamCenterClientPtr = NULL;
        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
		return Result;
	}
	Result = m_MaintainSystemThread.Start();
	if (FAILED(Result))
	{
		SYS_ERROR("m_MaintainSystemThread.Start fail, Result = 0x%lx\n", Result);
		DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MaintainSystemThread.Create fail, Result = 0x%lx\n", Result);
		m_StreamCenterClientPtr->Deinitialize();
        m_StreamCenterClientPtr = NULL;
        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
		return Result;
	}

	int32_t Time1 = TimeStamp();
	m_VidInBlocked = true;
    //OpenVideoInOutDevice
    Result = m_StreamCenterClientPtr->OpenVideoInOutDevice(0, 0, &m_VideoInOutHandle);
    if (FAILED(Result))
    {
        SYS_ERROR("OpenVideoInOutDevice fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OpenVideoInOutDevice fail, Result = 0x%lx\n", Result);
        m_VidInBlocked = false;
        m_StreamCenterClientPtr->Deinitialize();
        m_StreamCenterClientPtr = NULL;
        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return Result;
    }

    //allocate video source
    m_VideoSourcePtr = BaseMemoryManager::Instance().New<SysPkgVideoSource>();
    if (NULL == m_VideoSourcePtr.GetPtr())
    {
        SYS_ERROR("m_VideoSourcePtr new fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_VideoSourcePtr new fail\n");
        m_VidInBlocked = false;
        m_StreamCenterClientPtr->CloseVideoInOutDevice(m_VideoInOutHandle);
        m_StreamCenterClientPtr->Deinitialize();
        m_StreamCenterClientPtr = NULL;

        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();

        return GMI_OUT_OF_MEMORY;
    }
    memset(m_VideoSourcePtr.GetPtr(), 0, sizeof(SysPkgVideoSource));

    //get video source parameter
    Result = m_ConfigFileManagerPtr->GetVideoSourceSettings(0, m_VideoSourcePtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("GetVideoSourceSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetVideoSourceSettings fail, Result = 0x%lx\n", Result);
        m_VidInBlocked = false;
        m_VideoSourcePtr = NULL;
        m_StreamCenterClientPtr->CloseVideoInOutDevice(m_VideoInOutHandle);
        m_StreamCenterClientPtr->Deinitialize();
        m_StreamCenterClientPtr = NULL;

        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return Result;
    }

    //set video in & out param
    VideoInParam  Vin;
    VideoOutParam Vout;
    Result = m_StreamCenterClientPtr->GetVinVoutConfiguration(m_VideoInOutHandle, &Vin, &Vout);
    if (FAILED(Result))
    {
        SYS_ERROR("GetVinVoutConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetVinVoutConfiguration fail, Result = 0x%lx\n", Result);
        m_VidInBlocked = false;
        m_VideoSourcePtr = NULL;
        m_StreamCenterClientPtr->CloseVideoInOutDevice(m_VideoInOutHandle);
        m_StreamCenterClientPtr->Deinitialize();
        m_StreamCenterClientPtr = NULL;

        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return Result;
    }
	
    Vin.s_VinFrameRate     = m_VideoSourcePtr.GetPtr()->s_SrcFps;
    Vin.s_VinMirrorPattern = m_VideoSourcePtr.GetPtr()->s_Mirror;
    Result = m_StreamCenterClientPtr->SetVinVoutConfiguration(m_VideoInOutHandle, &Vin, &Vout);
    if (FAILED(Result))
    {
        SYS_ERROR("SetVinVoutConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetVinVoutConfiguration fail, Result = 0x%lx\n", Result);
        m_VidInBlocked = false;
        m_VideoSourcePtr = NULL;
        m_StreamCenterClientPtr->CloseVideoInOutDevice(m_VideoInOutHandle);
        m_StreamCenterClientPtr->Deinitialize();
        m_StreamCenterClientPtr = NULL;

        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return Result;
    }
    m_VidInBlocked = false;
	SYS_INFO("==================>set vidinout waste time %d\n", TimeStamp() - Time1);

    //start video
    for (Id = 0; Id < m_VideoStreamNum; Id++)
    {
        Result = m_StreamCenterClientPtr->Create(0, Id, MEDIA_VIDEO, (m_VideoEncParamPtr.GetPtr())[Id].s_EncodeType, true, &((m_VideoEncParamPtr.GetPtr())[Id]), sizeof(VideoEncodeParam), &((m_VideoCodecHandle.GetPtr())[Id]));
        if (FAILED(Result))
        {
            SYS_ERROR("Create fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Create fail, Result = 0x%lx\n", Result);
            while (--Id)
            {
                m_StreamCenterClientPtr->Destroy((m_VideoCodecHandle.GetPtr())[Id]);
            }

            m_VideoSourcePtr = NULL;
            m_StreamCenterClientPtr->CloseVideoInOutDevice(m_VideoInOutHandle);
            m_StreamCenterClientPtr->Deinitialize();
            m_StreamCenterClientPtr = NULL;

            m_VideoCodecHandle = NULL;
            m_VideoStreamTypePtr = NULL;
            m_VideoEncParamPtr = NULL;
            m_ConfigFileManagerPtr->Deinitialize();

            return Result;
        }
    }

    for (Id = 0; Id < m_VideoStreamNum; Id++)
    {
        Result = m_StreamCenterClientPtr->Start2((m_VideoCodecHandle.GetPtr())[Id]);
        if (FAILED(Result))
        {
            SYS_ERROR("Start fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Start fail, Result = 0x%lx\n", Result);
            while (--Id)
            {
                m_StreamCenterClientPtr->Stop2((m_VideoCodecHandle.GetPtr())[Id]);
                m_StreamCenterClientPtr->Destroy((m_VideoCodecHandle.GetPtr())[Id]);
            }

            m_VideoSourcePtr = NULL;
            m_StreamCenterClientPtr->CloseVideoInOutDevice(m_VideoInOutHandle);
            m_StreamCenterClientPtr->Deinitialize();
            m_StreamCenterClientPtr = NULL;

            m_VideoCodecHandle = NULL;
            m_VideoStreamTypePtr = NULL;
            m_VideoEncParamPtr = NULL;
            m_ConfigFileManagerPtr->Deinitialize();

            return Result;
        }
    }

    //OpenImageDevice
    Result = m_StreamCenterClientPtr->OpenImageDevice(0, 0, &m_ImageHandle);
    if (FAILED(Result))
    {
        SYS_ERROR("OpenImageDevice fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OpenImageDevice fail, Result = 0x%lx\n", Result);
        while (--Id)
        {
            m_StreamCenterClientPtr->Stop((m_VideoCodecHandle.GetPtr())[Id]);
        }

        m_VideoSourcePtr = NULL;
        m_StreamCenterClientPtr->CloseVideoInOutDevice(m_VideoInOutHandle);
        m_StreamCenterClientPtr->Deinitialize();
        m_StreamCenterClientPtr = NULL;

        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return Result;
    }

    //start audio encode
    m_AudioEncParamPtr     = BaseMemoryManager::Instance().New<AudioEncParam>();
    if (NULL == m_AudioEncParamPtr.GetPtr())
    {
        SYS_ERROR("m_AudioEncParam new fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_AudioEncParam new fail\n");
        while (--Id)
        {
            m_StreamCenterClientPtr->Stop((m_VideoCodecHandle.GetPtr())[Id]);
        }

        m_VideoSourcePtr = NULL;
        m_StreamCenterClientPtr->CloseVideoInOutDevice(m_VideoInOutHandle);
        m_StreamCenterClientPtr->Deinitialize();
        m_StreamCenterClientPtr = NULL;

        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return GMI_OUT_OF_MEMORY;
    }
    memset(m_AudioEncParamPtr.GetPtr(), 0, sizeof(AudioEncParam));

    m_AudioDecParamPtr     = BaseMemoryManager::Instance().New<AudioDecParam>();
    if (NULL == m_AudioDecParamPtr.GetPtr())
    {
        SYS_ERROR("m_AudioDecParamPtr new fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_AudioDecParamPtr new fail\n");
        m_AudioEncParamPtr = NULL;
        while (--Id)
        {
            m_StreamCenterClientPtr->Stop((m_VideoCodecHandle.GetPtr())[Id]);
        }

        m_VideoSourcePtr = NULL;
        m_StreamCenterClientPtr->CloseVideoInOutDevice(m_VideoInOutHandle);
        m_StreamCenterClientPtr->Deinitialize();
        m_StreamCenterClientPtr = NULL;

        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return GMI_OUT_OF_MEMORY;
    }
    memset(m_AudioDecParamPtr.GetPtr(), 0, sizeof(AudioDecParam));

    SysPkgAudioEncodeCfg SysAudioEncodeCfg;
    Result = m_ConfigFileManagerPtr->GetAudioEncodeSettings(&SysAudioEncodeCfg);
    if (FAILED(Result))
    {
        m_AudioEncParamPtr.GetPtr()->s_Codec = CODEC_G711A;
        SYS_ERROR("GetAudioEncodeSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "GetAudioEncodeSettings fail, Result = 0x%lx\n", Result);
    }
    else
    {
        SvrSysAudioEncodeTransferMediaAudio(&SysAudioEncodeCfg, m_AudioEncParamPtr.GetPtr(), m_AudioDecParamPtr.GetPtr());
    }

    boolean_t AudVidStreamExist = false;
    AudVidStreamIsExis(&AudVidStreamExist);
    if (AudVidStreamExist)
    {
        Result = m_StreamCenterClientPtr->Start(0, 0, MEDIA_AUDIO, m_AudioEncParamPtr.GetPtr()->s_Codec, true, m_AudioEncParamPtr.GetPtr(), sizeof(AudioEncParam), &m_AudioEncHandle);
        if (FAILED(Result))
        {
            SYS_ERROR("Start fail audio, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Start fail audio, Result = 0x%lx\n", Result);
        }
    }

    //sdk stream control
    m_SdkStreamCtlPtr = BaseMemoryManager::Instance().New<SdkStreamControl>(SYS_SERVER_TO_SDK_PORT, SDK_TO_SYS_SERVER_PORT);
    if (NULL == m_SdkStreamCtlPtr.GetPtr())
    {
        SYS_ERROR("m_SdkStreamCtlPtr new fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_SdkStreamCtlPtr new fail\n");
        if (NULL != m_AudioEncHandle)
        {
            m_StreamCenterClientPtr->Stop(m_AudioEncHandle);
        }
        m_AudioEncParamPtr = NULL;

        while (--Id)
        {
            m_StreamCenterClientPtr->Stop((m_VideoCodecHandle.GetPtr())[Id]);
        }

        m_VideoSourcePtr = NULL;
        m_StreamCenterClientPtr->CloseVideoInOutDevice(m_VideoInOutHandle);
        m_StreamCenterClientPtr->Deinitialize();
        m_StreamCenterClientPtr = NULL;

        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return GMI_OUT_OF_MEMORY;
    }

    Result = m_SdkStreamCtlPtr->Initialize();
    if (FAILED(Result))
    {
        SYS_ERROR("m_SdkStreamCtlPtr Initialize fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_SdkStreamCtlPtr Initialize fail, Result = 0x%lx\n", Result);
        m_SdkStreamCtlPtr = NULL;

        if (NULL != m_AudioEncHandle)
        {
            m_StreamCenterClientPtr->Stop(m_AudioEncHandle);
        }
        m_AudioEncParamPtr = NULL;

        while (--Id)
        {
            m_StreamCenterClientPtr->Stop((m_VideoCodecHandle.GetPtr())[Id]);
        }

        m_VideoSourcePtr = NULL;
        m_StreamCenterClientPtr->CloseVideoInOutDevice(m_VideoInOutHandle);
        m_StreamCenterClientPtr->Deinitialize();
        m_StreamCenterClientPtr = NULL;

        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return Result;
    }

    //rtsp stream control
    m_RtspStreamCtlPtr = BaseMemoryManager::Instance().New<RtspStreamControl>();
    if (NULL == m_RtspStreamCtlPtr.GetPtr())
    {
        SYS_ERROR("m_RtspStreamCtlPtr new fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_RtspStreamCtlPtr new fail\n");
        m_SdkStreamCtlPtr->Deinitialize();
        m_SdkStreamCtlPtr = NULL;
        if (NULL != m_AudioEncHandle)
        {
            m_StreamCenterClientPtr->Stop(m_AudioEncHandle);
        }
        m_AudioEncParamPtr = NULL;

        while (--Id)
        {
            m_StreamCenterClientPtr->Stop((m_VideoCodecHandle.GetPtr())[Id]);
        }

        m_VideoSourcePtr = NULL;
        m_StreamCenterClientPtr->CloseVideoInOutDevice(m_VideoInOutHandle);
        m_StreamCenterClientPtr->Deinitialize();
        m_StreamCenterClientPtr = NULL;

        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return GMI_OUT_OF_MEMORY;
    }

    Result = m_RtspStreamCtlPtr->Initialize();
    if (FAILED(Result))
    {
        SYS_ERROR("m_RtspStreamCtlPtr Initialize fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_RtspStreamCtlPtr Initialize fail, Result = 0x%lx\n", Result);
        m_RtspStreamCtlPtr = NULL;
        m_SdkStreamCtlPtr->Deinitialize();
        m_SdkStreamCtlPtr = NULL;
        if (NULL != m_AudioEncHandle)
        {
            m_StreamCenterClientPtr->Stop(m_AudioEncHandle);
        }
        m_AudioEncParamPtr = NULL;

        while (--Id)
        {
            m_StreamCenterClientPtr->Stop((m_VideoCodecHandle.GetPtr())[Id]);
        }

        m_VideoSourcePtr = NULL;
        m_StreamCenterClientPtr->CloseVideoInOutDevice(m_VideoInOutHandle);
        m_StreamCenterClientPtr->Deinitialize();
        m_StreamCenterClientPtr = NULL;

        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return Result;
    }

    Result = StartStreamMonitor();
    if (FAILED(Result))
    {

        SYS_ERROR("m_SdkStreamCtlPtr Initialize fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_SdkStreamCtlPtr Initialize fail, Result = 0x%lx\n", Result);
        m_SdkStreamCtlPtr->Deinitialize();
        m_SdkStreamCtlPtr = NULL;
        m_AudioEncParamPtr = NULL;
        while (--Id)
        {
            m_StreamCenterClientPtr->Stop((m_VideoCodecHandle.GetPtr())[Id]);
        }

        m_VideoSourcePtr = NULL;
        m_StreamCenterClientPtr->CloseVideoInOutDevice(m_VideoInOutHandle);
        m_StreamCenterClientPtr->Deinitialize();
        m_StreamCenterClientPtr = NULL;

        m_VideoCodecHandle = NULL;
        m_VideoStreamTypePtr = NULL;
        m_VideoEncParamPtr = NULL;
        m_ConfigFileManagerPtr->Deinitialize();
        return Result;
    }

    SYS_INFO("##%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "##%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::MediaDeinitial(void)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    GMI_RESULT Result = StopStreamMonitor();
    if (FAILED(Result))
    {
        SYS_ERROR("StopStreamMonitor  fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StopStreamMonitor  fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_RtspStreamCtlPtr->Deinitialize();
    if (FAILED(Result))
    {
        SYS_ERROR("m_RtspStreamCtlPtr Deinitialize fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_RtspStreamCtlPtr Deinitialize fail, Result = 0x%lx\n", Result);
        return Result;
    }
    m_RtspStreamCtlPtr = NULL;

    Result = m_SdkStreamCtlPtr->Deinitialize();
    if (FAILED(Result))
    {
        SYS_ERROR("m_SdkStreamCtlPtr Deinitialize fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_SdkStreamCtlPtr Deinitialize fail, Result = 0x%lx\n", Result);
        return Result;
    }
    m_SdkStreamCtlPtr = NULL;

    Result = m_StreamCenterClientPtr->CloseImageDevice(m_ImageHandle);
    if (FAILED(Result))
    {
        SYS_ERROR("CloseImageDevice fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "CloseImageDevice fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = DestroyVideoCodec();
    if (FAILED(Result))
    {
        SYS_ERROR("DestroyVideoCodec fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "DestroyVideoCodec fail, Result = 0x%lx\n", Result);
        return Result;
    }

    m_VideoSourcePtr = NULL;
    Result = m_StreamCenterClientPtr->CloseVideoInOutDevice(m_VideoInOutHandle);
    if (FAILED(Result))
    {
        SYS_ERROR("CloseVideoInOutDevice fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "CloseVideoInOutDevice fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_StreamCenterClientPtr->Deinitialize();
    if (FAILED(Result))
    {
        SYS_ERROR("m_StreamCenterClient Deinitialize fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_StreamCenterClient Deinitialize fail, Result = 0x%lx\n", Result);
        return Result;
    }
    m_StreamCenterClientPtr = NULL;

    m_VideoCodecHandle = NULL;
    m_VideoEncParamPtr = NULL;
    Result = m_ConfigFileManagerPtr->Deinitialize();
    if (FAILED(Result))
    {
        SYS_ERROR("m_ConfigFileManager Deinitialize fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ConfigFileManager Deinitialize fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::MediaParamLoad(void)
{
    SYS_INFO("##%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "##%s in..........\n", __func__);

    GMI_RESULT Result = GMI_SUCCESS;

    //allocate video osd parameter
    m_VideoOsdParamPtr	   = BaseMemoryManager::Instance().News<VideoOSDParam>(MAX_VIDEO_STREAM_NUM);
    if (NULL == m_VideoOsdParamPtr.GetPtr())
    {
        SYS_ERROR("m_VideoOsdParamPtr new fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_VideoOsdParamPtr new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    memset(m_VideoOsdParamPtr.GetPtr(), 0, sizeof(VideoOSDParam)*MAX_VIDEO_STREAM_NUM);

    //get video osd parameter
    Result = m_ConfigFileManagerPtr->GetOsdSettings(0xff, m_VideoStreamNum, m_VideoOsdParamPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("GetOsdSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetOsdSettings fail, Result = 0x%lx\n", Result);
        m_VideoOsdParamPtr = NULL;
        return Result;
    }

    //set osd parameter
    Result = OsdsInit(m_VideoStreamNum, m_VideoEncParamPtr.GetPtr(), m_VideoOsdParamPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("OSD init fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OSD init fail, Result = 0x%lx\n", Result);
        m_VideoOsdParamPtr = NULL;
        return Result;
    }


    //allocate Image parameter
    m_ImageParamPtr = BaseMemoryManager::Instance().New<ImageBaseParam>();
    if (NULL == m_ImageParamPtr.GetPtr())
    {
        SYS_ERROR("m_ImageParamPtr new fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ImageParamPtr new fail\n");
        m_VideoOsdParamPtr = NULL;
        return GMI_OUT_OF_MEMORY;
    }
    memset(m_ImageParamPtr.GetPtr(), 0, sizeof(ImageBaseParam));

    //get video source image parameter
    Result = m_ConfigFileManagerPtr->GetImageSettings(0, 0, m_ImageParamPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("GetImageSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetImageSettings fail, Result = 0x%lx\n", Result);
        m_ImageParamPtr = NULL;
        m_VideoOsdParamPtr = NULL;
        return Result;
    }

    //set imaging parameter
    Result = m_StreamCenterClientPtr->SetImageConfiguration(m_ImageHandle, m_ImageParamPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("SetImageConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetImageConfiguration fail, Result = 0x%lx\n", Result);
        m_ImageParamPtr = NULL;
        m_VideoOsdParamPtr = NULL;
        return Result;
    }

    //new m_ImageAdvanceParamPtr
    m_ImageAdvanceParamPtr = BaseMemoryManager::Instance().New<ImageAdvanceParam>();
    if (NULL == m_ImageAdvanceParamPtr.GetPtr())
    {
        SYS_ERROR("m_ImageAdvanceParamPtr New fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ImageAdvanceParamPtr New fail\n");
        m_ImageParamPtr = NULL;
        m_VideoOsdParamPtr = NULL;
        return GMI_OUT_OF_MEMORY;
    }
    memset(m_ImageAdvanceParamPtr.GetPtr(), 0, sizeof(ImageAdvanceParam));


    //get ImageAdvanceSettings
    Result = m_ConfigFileManagerPtr->GetImageAdvanceSettings(0, 0, m_ImageAdvanceParamPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("GetImageAdvanceSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetImageAdvanceSettings fail, Result = 0x%lx\n", Result);
        m_ImageAdvanceParamPtr = NULL;
        m_ImageParamPtr = NULL;
        m_VideoOsdParamPtr = NULL;
        return Result;
    }

    //set ImageAdvanceSettings
    Result = m_StreamCenterClientPtr->SetImageAdvanceConfiguration(m_ImageHandle, m_ImageAdvanceParamPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("SetImageAdvanceConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetImageAdvanceConfiguration fail, Result = 0x%lx\n", Result);
        m_ImageAdvanceParamPtr = NULL;
        m_ImageParamPtr = NULL;
        m_VideoOsdParamPtr = NULL;
        return Result;
    }

    //set whitebalance
    m_ImageWbParamPtr = BaseMemoryManager::Instance().New<ImageWbParam>();
    if (NULL == m_ImageWbParamPtr.GetPtr())
    {
        SYS_ERROR("m_ImageWbParamPtr New fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ImageWbParamPtr New fail\n");
        m_ImageAdvanceParamPtr = NULL;
        m_ImageParamPtr = NULL;
        m_VideoOsdParamPtr = NULL;
        return GMI_OUT_OF_MEMORY;
    }
    memset(m_ImageWbParamPtr.GetPtr(), 0, sizeof(ImageWbParam));

    Result = m_ConfigFileManagerPtr->GetWhiteBalanceSettings(0, m_ImageWbParamPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("GetWhiteBalanceSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetWhiteBalanceSettings fail, Result = 0x%lx\n", Result);
        m_ImageWbParamPtr = NULL;
        m_ImageAdvanceParamPtr = NULL;
        m_ImageParamPtr = NULL;
        m_VideoOsdParamPtr = NULL;
        return Result;
    }

    Result = m_StreamCenterClientPtr->SetWhiteBalanceConfiguration(m_ImageHandle, m_ImageWbParamPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("SetWhiteBalanceConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetWhiteBalanceConfiguration fail, Result = 0x%lx\n", Result);
        m_ImageWbParamPtr = NULL;
        m_ImageAdvanceParamPtr = NULL;
        m_ImageParamPtr = NULL;
        m_VideoOsdParamPtr = NULL;
        return Result;
    }

    //set daynight
    m_ImageDnParamPtr = BaseMemoryManager::Instance().New<ImageDnParam>();
    if (NULL == m_ImageDnParamPtr.GetPtr())
    {
        SYS_ERROR("image daynight Ptr New fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "image daynight Ptr New fail\n");
        m_ImageWbParamPtr = NULL;
        m_ImageAdvanceParamPtr = NULL;
        m_ImageParamPtr = NULL;
        m_VideoOsdParamPtr = NULL;
        return GMI_OUT_OF_MEMORY;
    }
    memset(m_ImageDnParamPtr.GetPtr(), 0, sizeof(ImageDnParam));

    Result = m_ConfigFileManagerPtr->GetDaynightSettings(0, m_ImageDnParamPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("GetDaynightSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetDaynightSettings fail, Result = 0x%lx\n", Result);
        m_ImageDnParamPtr = NULL;
        m_ImageWbParamPtr = NULL;
        m_ImageAdvanceParamPtr = NULL;
        m_ImageParamPtr = NULL;
        m_VideoOsdParamPtr = NULL;
        return Result;
    }

    Result = m_StreamCenterClientPtr->SetDaynightConfiguration(m_ImageHandle, m_ImageDnParamPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("Apply daynight config set to media control module fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Apply daynight config set to media control module fail, Result = 0x%lx\n", Result);
        m_ImageDnParamPtr = NULL;
        m_ImageWbParamPtr = NULL;
        m_ImageAdvanceParamPtr = NULL;
        m_ImageParamPtr = NULL;
        m_VideoOsdParamPtr = NULL;
        return Result;
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::MediaParamUnLoad(void)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    m_ImageAdvanceParamPtr = NULL;
    m_ImageParamPtr = NULL;
    m_VideoOsdParamPtr = NULL;

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrStop3A(void)
{
	GMI_RESULT Result = GMI_SUCCESS;
	
	if (NULL != m_ZoomHandle)
	{
		Result = m_StreamCenterClientPtr->CloseZoomDevice(m_ZoomHandle);
		if (FAILED(Result))
		{
			SYS_ERROR("CloseZoomDevice fail, Result = 0x%lx\n", Result);
			return Result;
		}		
		else
		{
			SYS_ERROR("CloseZoomDevice OK\n");
		}
		
		m_ZoomHandle = NULL;
	}
	else
	{
		SYS_INFO("CloseZoomDevice has done\n");
	}

	if (NULL != m_AutoFocusHandle)
	{
		Result = m_StreamCenterClientPtr->StopAutoFocusDevice(m_AutoFocusHandle);
		if (FAILED(Result))
		{
			SYS_ERROR("StopAutoFocusDevice fail, Result = 0x%lx\n", Result);
			return Result;
		}
		else
		{
			SYS_ERROR("StopAutoFocusDevice OK\n");
		}

		Result = m_StreamCenterClientPtr->CloseAutoFocusDevice(m_AutoFocusHandle);
		if (FAILED(Result))
		{
			SYS_ERROR("CloseAutoFocusDevice fail, Result = 0x%lx\n", Result);
			return Result;
		}
		else
		{
			SYS_ERROR("CloseAutoFocusDevice OK\n");
		}

		m_AutoFocusHandle = NULL; 
	}
	else
	{
		SYS_INFO("CloseAutoFocusDevice has done\n");
	}

	if (NULL != m_ImageHandle)
	{
		Result = m_StreamCenterClientPtr->CloseImageDevice(m_ImageHandle);
		if (FAILED(Result))
		{
			SYS_ERROR("CloseImageDevice fail, Result = 0x%lx\n", Result);
			DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "CloseImageDevice fail, Result = 0x%lx\n", Result);
			return Result;
		}
		else
		{
			 SYS_ERROR("CloseImageDevice OK\n");
			 m_ImageHandle = NULL;
		}
	}
	else
	{
		SYS_INFO("CloseImageDevice has done\n");
	}

	return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::OsdsInit(int32_t VideoCnt, VideoEncodeParam VideoEncodeParam[], VideoOSDParam VideoOsdParam[])
{
    GMI_RESULT Result = m_StreamCenterClientPtr->SetOsdConfiguration((m_VideoCodecHandle.GetPtr())[0], &VideoOsdParam[0]);
    if (FAILED(Result))
    {
        SYS_ERROR("SetOsdConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetOsdConfiguration fail, Result = 0x%lx\n", Result);
        return Result;
    }

    for (int32_t Id = 1; Id < VideoCnt; Id++)
    {
        if (VideoEncodeParam[Id].s_EncodeWidth != VideoEncodeParam[Id-1].s_EncodeWidth
                || VideoEncodeParam[Id].s_EncodeHeight != VideoEncodeParam[Id-1].s_EncodeHeight)
        {
            Result = m_StreamCenterClientPtr->SetOsdConfiguration((m_VideoCodecHandle.GetPtr())[Id], &VideoOsdParam[Id]);
            if (FAILED(Result))
            {
                SYS_ERROR("SetOsdConfiguration fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetOsdConfiguration fail, Result = 0x%lx\n", Result);
                return Result;
            }
        }
    }

    return GMI_SUCCESS;
}


void SystemServiceManager::AudVidStreamIsExis(boolean_t *Exit)
{
    int32_t Id;
    for (Id = 0; Id < m_VideoStreamNum; Id++)
    {
        if (SYS_AUDIO_VIDEO_STREAM == (m_VideoStreamTypePtr.GetPtr())[Id])
        {
            *Exit = true;
            break;
        }
    }

    if (Id >= m_VideoStreamNum)
    {
        *Exit = false;
    }

    return;
}


GMI_RESULT SystemServiceManager::PTZ_Initial()
{
    SYS_INFO("##%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "##%s in..........\n", __func__);

    SysPkgComponents SysComponents;

    GMI_RESULT Result = m_ConfigFileManagerPtr->GetHwAutoDetectInfo(&SysComponents);
    if (FAILED(Result))
    {
        SYS_ERROR("GetHwAutoDetectInfo fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetHwAutoDetectInfo fail, Result = 0x%lx\n", Result);
        return Result;
    }

    //dome
    if (SysComponents.s_ZoomLensId != e_ZOOM_LENS_NONE)
    {
        m_SupportPtz = true;
    }

    if (m_SupportPtz)
    {
        //ptz control init
        //should get ptz parameter from user config file
        //fill out default parameter
        m_PresetsInfo_InnerPtr =  BaseMemoryManager::Instance().News<SysPkgPresetInfo_Inner>(MAX_PRESETS);
        if (NULL == m_PresetsInfo_InnerPtr.GetPtr())
        {
            SYS_ERROR("m_PresetsInfo_InnerPtr new fail\n");
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_PresetsInfo_InnerPtr new fail\n");
            return GMI_OUT_OF_MEMORY;
        }
        memset(m_PresetsInfo_InnerPtr.GetPtr(), 0, sizeof(SysPkgPresetInfo_Inner)*MAX_PRESETS);

        //get presets info
        Result = m_ConfigFileManagerPtr->GetPresetsInfo(m_PresetsInfo_InnerPtr.GetPtr());
        if (FAILED(Result))
        {
            SYS_ERROR("GetPresetsInfo new fail\n");
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetPresetsInfo new fail\n");
            //return Result;
        }

        m_PtzUartCfg.s_Address	   = PTZ_UART_ADDRESS;
        m_PtzUartCfg.s_Bauderate   = PTZ_UART_BAUDE_RATE;
        m_PtzUartCfg.s_Databits	   = PTZ_UART_DATA_BITS;
        m_PtzUartCfg.s_Parity	   = PTZ_UART_PARITY;
        m_PtzUartCfg.s_Stopbits	   = PTZ_UART_SOTP_BITS;
        m_PtzUartCfg.s_FlowControl = PTZ_UART_FLOW_CONTROL;
        m_PtzUartCfg.s_Protocol	   = PTZ_PROTOCOL_D_PROTOCOL;

        m_PtzControlPtr = BaseMemoryManager::Instance().New<PtzControl>();
        if (m_PtzControlPtr.GetPtr() == NULL)
        {
            m_PresetsInfo_InnerPtr = NULL;
            SYS_ERROR("m_PtzControlPtr new fail\n");
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_PtzControlPtr new fail\n");
            return GMI_OUT_OF_MEMORY;
        }

        Result = m_PtzControlPtr->Initialize(GMI_SETTING_CONFIG_FILE_NAME, PTZ_UART_CONFIG_PATH, PTZ_PRESET_INFO_PATH, &m_PtzUartCfg);
        if (FAILED(Result))
        {
            m_PtzControlPtr	= NULL;
            m_PresetsInfo_InnerPtr = NULL;
            SYS_ERROR("Initialize fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Initialize fail, Result = 0x%lx\n", Result);
            return Result;
        }

        //AF
        Result = m_StreamCenterClientPtr->OpenAutoFocusDevice(0, &m_AutoFocusHandle);
        if (FAILED(Result))
        {
            m_PtzControlPtr	= NULL;
            m_PresetsInfo_InnerPtr = NULL;
            SYS_ERROR("OpenAutoFocusDevice fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OpenAutoFocusDevice fail, Result = 0x%lx\n", Result);
            return Result;
        }

		//particular lens, particular process
		if (e_ZOOM_LENS_ICRJZ9 == SysComponents.s_ZoomLensId)
		{
			//zoom
	        Result = m_StreamCenterClientPtr->OpenZoomDevice(0, &m_ZoomHandle);
	        if (FAILED(Result))
	        {	           
	            m_StreamCenterClientPtr->CloseAutoFocusDevice(m_AutoFocusHandle);	            
	            SYS_ERROR("OpenZoomDevice fail, Result = 0x%lx\n", Result);
	            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OpenZoomDevice fail, Result = 0x%lx\n", Result);
	            return Result;
	        }

	        int32_t ZoomPos;
	        Result = m_ConfigFileManagerPtr->GetCurrentZoomPos(&ZoomPos);
	        if (FAILED(Result))
	        {
	            ZoomPos = PTZ_CURRENT_ZOOM;
	            SYS_ERROR("GetCurrentZoomPos fail, warning, Result = 0x%lx\n", Result);
	            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "GetCurrentZoomPos fail, Result = 0x%lx\n", Result);
	            //return Result;
	        }

	        Result = m_StreamCenterClientPtr->SetZoomPosition(m_ZoomHandle, ZoomPos);
	        if (FAILED(Result))
	        {	            
	        	m_StreamCenterClientPtr->CloseZoomDevice(m_ZoomHandle);
	            m_StreamCenterClientPtr->CloseAutoFocusDevice(m_AutoFocusHandle);	           
	            SYS_ERROR("SetZoomPosition fail, Result = 0x%lx\n", Result);
	            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetZoomPosition fail, Result = 0x%lx\n", Result);
	            return Result;
	        }	        

	        Result = m_StreamCenterClientPtr->StartAutoFocusDevice(m_AutoFocusHandle);
	        if (FAILED(Result))
	        {	     
	        	m_StreamCenterClientPtr->CloseZoomDevice(m_ZoomHandle);
	        	m_StreamCenterClientPtr->CloseAutoFocusDevice(m_AutoFocusHandle);
	            SYS_ERROR("StartAutoFocusDevice fail, Result = 0x%lx\n", Result);
	            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StartAutoFocusDevice fail, Result = 0x%lx\n", Result);
	            return Result;
	        }

	        Result = m_ConfigFileManagerPtr->GetAutoFocusMode(&m_FocusMode);
	        if (FAILED(Result))
	        {	        	
	        	m_StreamCenterClientPtr->StopAutoFocusDevice(m_AutoFocusHandle);
	        	m_StreamCenterClientPtr->CloseZoomDevice(m_ZoomHandle);
	            m_StreamCenterClientPtr->CloseAutoFocusDevice(m_AutoFocusHandle);	            
	            SYS_ERROR("m_ConfigFileManagerPtr GetAutoFocusMode fail, Result = 0x%lx\n", Result);
	            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ConfigFileManagerPtr GetAutoFocusMode fail, Result = 0x%lx\n", Result);
	            return Result;
	        }

	        Result = m_StreamCenterClientPtr->SetAutoFocusMode(m_AutoFocusHandle, m_FocusMode);
	        if (FAILED(Result))
	        {
	        	m_StreamCenterClientPtr->StopAutoFocusDevice(m_AutoFocusHandle);
	        	m_StreamCenterClientPtr->CloseZoomDevice(m_ZoomHandle);
	            m_StreamCenterClientPtr->CloseAutoFocusDevice(m_AutoFocusHandle);	           
	            SYS_ERROR("m_ConfigFileManagerPtr GetAutoFocusMode fail, Result = 0x%lx\n", Result);
	            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ConfigFileManagerPtr GetAutoFocusMode fail, Result = 0x%lx\n", Result);
	            return Result;
	        }

	        //start record user last zoom postion
	        Result = StartRecordZoomPos();
	        if (FAILED(Result))
	        {
	            m_StreamCenterClientPtr->StopAutoFocusDevice(m_AutoFocusHandle);
	        	m_StreamCenterClientPtr->CloseZoomDevice(m_ZoomHandle);
	            m_StreamCenterClientPtr->CloseAutoFocusDevice(m_AutoFocusHandle);           
	            SYS_ERROR("SetZoomPosition fail, Result = 0x%lx\n", Result);
	            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetZoomPosition fail, Result = 0x%lx\n", Result);
	            return Result;
	        }
		}
		else
		{
			Result = m_StreamCenterClientPtr->StartAutoFocusDevice(m_AutoFocusHandle);
	        if (FAILED(Result))
	        {	      	        	
	        	m_StreamCenterClientPtr->CloseAutoFocusDevice(m_AutoFocusHandle);
	            SYS_ERROR("StartAutoFocusDevice fail, Result = 0x%lx\n", Result);
	            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StartAutoFocusDevice fail, Result = 0x%lx\n", Result);
	            return Result;
	        }

	        Result = m_ConfigFileManagerPtr->GetAutoFocusMode(&m_FocusMode);
	        if (FAILED(Result))
	        {
	            m_StreamCenterClientPtr->CloseAutoFocusDevice(m_AutoFocusHandle);	    
	            SYS_ERROR("m_ConfigFileManagerPtr GetAutoFocusMode fail, Result = 0x%lx\n", Result);
	            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ConfigFileManagerPtr GetAutoFocusMode fail, Result = 0x%lx\n", Result);
	            return Result;
	        }

	        Result = m_StreamCenterClientPtr->SetAutoFocusMode(m_AutoFocusHandle, m_FocusMode);
	        if (FAILED(Result))
	        {
	            m_StreamCenterClientPtr->CloseAutoFocusDevice(m_AutoFocusHandle);	       
	            SYS_ERROR("m_ConfigFileManagerPtr GetAutoFocusMode fail, Result = 0x%lx\n", Result);
	            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ConfigFileManagerPtr GetAutoFocusMode fail, Result = 0x%lx\n", Result);
	            //return Result;
	        }

	        //zoom
	        Result = m_StreamCenterClientPtr->OpenZoomDevice(0, &m_ZoomHandle);
	        if (FAILED(Result))
	        {
	            m_StreamCenterClientPtr->StopAutoFocusDevice(m_AutoFocusHandle);
	            m_StreamCenterClientPtr->CloseAutoFocusDevice(m_AutoFocusHandle);	      
	            SYS_ERROR("OpenZoomDevice fail, Result = 0x%lx\n", Result);
	            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OpenZoomDevice fail, Result = 0x%lx\n", Result);
	            return Result;
	        }

	        int32_t ZoomPos;
	        Result = m_ConfigFileManagerPtr->GetCurrentZoomPos(&ZoomPos);
	        if (FAILED(Result))
	        {
	            ZoomPos = PTZ_CURRENT_ZOOM;
	            SYS_ERROR("GetCurrentZoomPos fail, warning, Result = 0x%lx\n", Result);
	            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "GetCurrentZoomPos fail, Result = 0x%lx\n", Result);
	            //return Result;
	        }

	        Result = m_StreamCenterClientPtr->SetZoomPosition(m_ZoomHandle, ZoomPos);
	        if (FAILED(Result))
	        {
	        	m_StreamCenterClientPtr->CloseZoomDevice(m_ZoomHandle);
	            m_StreamCenterClientPtr->StopAutoFocusDevice(m_AutoFocusHandle);
	            m_StreamCenterClientPtr->CloseAutoFocusDevice(m_AutoFocusHandle);	        
	            SYS_ERROR("SetZoomPosition fail, Result = 0x%lx\n", Result);
	            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetZoomPosition fail, Result = 0x%lx\n", Result);
	            return Result;
	        }

	        //start record user last zoom postion
	        Result = StartRecordZoomPos();
	        if (FAILED(Result))
	        {
	            m_StreamCenterClientPtr->CloseZoomDevice(m_ZoomHandle);
	            m_StreamCenterClientPtr->StopAutoFocusDevice(m_AutoFocusHandle);
	            m_StreamCenterClientPtr->CloseAutoFocusDevice(m_AutoFocusHandle);           
	            SYS_ERROR("SetZoomPosition fail, Result = 0x%lx\n", Result);
	            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetZoomPosition fail, Result = 0x%lx\n", Result);
	            return Result;
	        }
		}        
    }

    SYS_INFO("##%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "##%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::PTZ_Deinitial()
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    if (m_SupportPtz)
    {
        GMI_RESULT Result = StopRecordZoomPos();
        if (FAILED(Result))
        {
            SYS_ERROR("StopRecordZoomPos fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StopRecordZoomPos fail, Result = 0x%lx\n", Result);
        }

        Result = m_StreamCenterClientPtr->CloseZoomDevice(m_ZoomHandle);
        if (FAILED(Result))
        {
            SYS_ERROR("CloseZoomDevice fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "CloseZoomDevice fail, Result = 0x%lx\n", Result);
        }
        m_ZoomHandle = NULL;

        Result = m_StreamCenterClientPtr->StopAutoFocusDevice(m_AutoFocusHandle);
        if (FAILED(Result))
        {
            SYS_ERROR("StopAutoFocusDevice fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StopAutoFocusDevice fail, Result = 0x%lx\n", Result);
        }

        Result = m_StreamCenterClientPtr->CloseAutoFocusDevice(m_AutoFocusHandle);
        if (FAILED(Result))
        {
            SYS_ERROR("CloseAutoFocusDevice fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "CloseAutoFocusDevice fail, Result = 0x%lx\n", Result);
        }

        m_AutoFocusHandle = NULL;       

        Result = m_PtzControlPtr->Deinitialize();
        if (FAILED(Result))
        {
            SYS_ERROR("Deinitialize fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Deinitialize fail, Result = 0x%lx\n", Result);
            return Result;
        }
        m_PtzControlPtr   = NULL;
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::StartRecordZoomPos()
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    GMI_RESULT Result = m_RecordZoomNotify.Create(NULL);
    if (FAILED(Result))
    {
        SYS_ERROR("m_RecordZoomNotify.Create fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_RecordZoomNotify.Create fail, Result = 0x%lx\n", Result);
        return Result;
    }

    m_RecordZoomPosThreadExitFlag  = false;
    Result = m_RecordZoomPosThread.Create(NULL, 0, RecordZoomPosThread, this);
    if (FAILED(Result))
    {
        m_RecordZoomNotify.Destroy();
        SYS_ERROR("m_RecordZoomPosThread.Create fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_RecordZoomPosThread.Create fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_RecordZoomPosThread.Start();
    if (FAILED(Result))
    {
        m_RecordZoomPosThread.Destroy();
        m_RecordZoomNotify.Destroy();
        SYS_ERROR("m_RecordZoomPosThread.Start fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_RecordZoomPosThread.Start fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::StopRecordZoomPos()
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    SYS_INFO("RecordZoomPos is exiting ......\n");
    m_RecordZoomPosThreadExitFlag = true;
    while (true == m_RecordZoomPosThreadExitFlag)
    {
        GMI_Sleep(500);
    }
    SYS_INFO("RecordZoomPos have exited......\n");

    GMI_RESULT Result = m_RecordZoomPosThread.Destroy();
    if (FAILED(Result))
    {
        SYS_ERROR("m_RecordZoomPosThread.Destroy() fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_RecordZoomPosThread.Destroy() fail, Result = 0x%lx\n", Result);
        return GMI_FAIL;
    }

    Result = m_RecordZoomNotify.Destroy();
    if (FAILED(Result))
    {
        SYS_ERROR("m_RecordZoomNotify.Destroy() fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_RecordZoomNotify.Destroy() fail, Result = 0x%lx\n", Result);
        return GMI_FAIL;
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


void_t* SystemServiceManager::RecordZoomPosThread(void *Argument)
{
    SystemServiceManager *Manager = reinterpret_cast<SystemServiceManager*> (Argument);
    return Manager->RecordZoomPos();
}


void_t* SystemServiceManager::RecordZoomPos(void)
{
	int32_t ZoomPos = PTZ_CURRENT_ZOOM;
	
    while (!m_RecordZoomPosThreadExitFlag)
    {

        GMI_RESULT Result = m_RecordZoomNotify.Wait(TIMEOUT_INFINITE);
        if (FAILED(Result))
        {
            SYS_ERROR("m_RecordZoomNotify wait fail, Result = 0x%lx\n", Result);
            continue;
        }

        m_StreamCenterClientPtr->GetZoomPosition(m_ZoomHandle, &ZoomPos);		
        SYS_INFO("save current zoom positon %d\n", ZoomPos);
        Result = m_ConfigFileManagerPtr->SetCurrentZoomPos(ZoomPos);
        if (FAILED(Result))
        {
            SYS_ERROR("Save CurrentZoomPos fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Save CurrentZoomPos fail, Result = 0x%lx\n", Result);
            SYS_ERROR("Exit thread %s\n", __func__);
            break;
        }
    }

    m_RecordZoomPosThreadExitFlag = false;

    return (void *) GMI_SUCCESS;
}



void_t* SystemServiceManager::ServerMonitorThread(void *Argument)
{
    SystemServiceManager *Manager = reinterpret_cast<SystemServiceManager*> (Argument);
    return Manager->ServerMonitor();
}


GMI_RESULT SystemServiceManager::SdkServerMonitor(boolean_t *Start)
{
#define MAX_AUDIO_NUM  (1)
    SysPkgEncodeCfg SysEncodeCfg[MAX_VIDEO_STREAM_NUM];
    SysPkgAudioEncodeCfg SysAudioEncodeCfg[MAX_AUDIO_NUM];

    *Start = false;

    int32_t   Started;
    uint16_t  Status;
    GMI_RESULT Result = DaemonQueryServerStatus(SDK_SERVER_ID, &Status);
    if (FAILED(Result))
    {
        SYS_ERROR("DaemonQueryServerStatus fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "DaemonQueryServerStatus fail, Result = 0x%lx\n", Result);
    }
    else
    {
        if (APPLICATION_STATUS_ONLINE == Status)
        {
            Result = m_SdkStreamCtlPtr->Query(5, &Started);
            if (FAILED(Result))
            {
                SYS_ERROR("Query SDK Server fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Query SDK Server fail, Result = 0x%lx\n", Result);
            }
            else if (SDK_STREAM_STATE_STOP == Started || SDK_STREAM_STATE_PAUSE == Started)
            {
                Result = SvrGetVideoEncodeSettings(0xff, SysEncodeCfg);
                if (FAILED(Result))
                {
                    SYS_ERROR("SvrGetVideoEncodeSettings fail\n");
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SvrGetVideoEncodeSettings fail\n");
                    return Result;
                }

                Result = SvrGetAudioEncodeSetting(1, SysAudioEncodeCfg);
                if (FAILED(Result))
                {
                    SYS_ERROR("SvrGetAudioEncodeSetting fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SvrGetAudioEncodeSetting fail, Result = 0x%lx\n", Result);
                }

                switch (Started)
                {
                case 0:
                    Result = m_SdkStreamCtlPtr->Start(SysEncodeCfg, m_VideoStreamNum, SysAudioEncodeCfg, 1, 5);
                    if (FAILED(Result))
                    {
                        SYS_ERROR("Start SDK Server fail, Result = 0x%lx\n", Result);
                        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Start SDK Server fail, Result = 0x%lx\n", Result);
                    }
                    break;
                case 1:
                    Result = m_SdkStreamCtlPtr->Resume(SysEncodeCfg, m_VideoStreamNum, SysAudioEncodeCfg, 1, 5);
                    if (FAILED(Result))
                    {
                        SYS_ERROR("Start SDK Server fail, Result = 0x%lx\n", Result);
                        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Start SDK Server fail, Result = 0x%lx\n", Result);
                    }
                    break;
                default:
                    SYS_ERROR("Query SDK Server Started %d error\n", Started);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Query SDK Server Started %d error\n", Started);
                    break;
                }
            }
        }
    }

    if (2 == Started)
    {
        *Start = true;
        //SYS_INFO("Good news! SDK Stream Transfer have been started successfully\n");
    }
    else
    {
        SYS_INFO("Bad news! SDK Stream Transfer have not been started\n");
    }

    return GMI_SUCCESS;
}

GMI_RESULT SystemServiceManager::RtspServerMonitor(boolean_t *Start)
{
#define MAX_AUDIO_NUM  (1)
    SysPkgEncodeCfg SysEncodeCfg[MAX_VIDEO_STREAM_NUM];
    SysPkgAudioEncodeCfg SysAudioEncodeCfg[MAX_AUDIO_NUM];

    *Start = false;

    int32_t   Started;
    uint16_t  Status;
    GMI_RESULT Result = DaemonQueryServerStatus(TRANSPORT_SERVER_ID, &Status);
    if (FAILED(Result))
    {
        SYS_ERROR("DaemonQueryServerStatus fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "DaemonQueryServerStatus fail, Result = 0x%lx\n", Result);
    }
    else
    {
        if (APPLICATION_STATUS_ONLINE == Status)
        {
            Result = m_RtspStreamCtlPtr->Query(5, m_VideoStreamNum, &Started);
            if (FAILED(Result))
            {
                SYS_ERROR("Query RTSP Server fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Query RTSP Server fail, Result = 0x%lx\n", Result);
            }
            else if (RTSP_STREAM_STATE_STOP == Started)
            {
                Result = SvrGetVideoEncodeSettings(0xff, SysEncodeCfg);
                if (FAILED(Result))
                {
                    SYS_ERROR("SvrGetVideoEncodeSettings fail\n");
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SvrGetVideoEncodeSettings fail\n");
                    return Result;
                }

                Result = SvrGetAudioEncodeSetting(1, SysAudioEncodeCfg);
                if (FAILED(Result))
                {
                    SYS_ERROR("SvrGetAudioEncodeSetting fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SvrGetAudioEncodeSetting fail, Result = 0x%lx\n", Result);
                }

                Result = m_RtspStreamCtlPtr->Start(SysEncodeCfg, m_VideoStreamNum, SysAudioEncodeCfg, 1, 5);
                if (FAILED(Result))
                {
                    SYS_ERROR("Start RTSP Server fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Start RTSP Server fail, Result = 0x%lx\n", Result);
                }
            }
        }
    }

    if (2 == Started)
    {
        *Start = true;
        //SYS_INFO("Good news! RTSP Stream Transfer have been started successfully\n");
    }
    else
    {
        SYS_INFO("Bad news! RTSP Stream Transfer have not been started\n");
    }

    return GMI_SUCCESS;
}


void_t* SystemServiceManager::ServerMonitor(void)
{
    boolean_t SdkStart = false;
    boolean_t RtspStart = false;

    while (!m_ThreadExitFlag)
    {
        pthread_rwlock_wrlock(&m_Lock);
        GMI_RESULT Result = SdkServerMonitor(&SdkStart);
        if (FAILED(Result))
        {
            SYS_ERROR("SdkServerMonitor fail, Result = 0x%lx\n", Result);
        }

        Result = RtspServerMonitor(&RtspStart);
        if (FAILED(Result))
        {
            SYS_ERROR("RtspServerMonitor fail, Result = 0x%lx\n", Result);
        }
        pthread_rwlock_unlock(&m_Lock);

        if (RtspStart && SdkStart )
        {
            GMI_Sleep(10*1000);
        }
        else
        {
            GMI_Sleep(2000);
        }
    }

    m_ThreadExitFlag = false;

    return (void *) GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrStartAudioDecode(int32_t AudioId, SysPkgAudioDecParam *SysAudioDecParamPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (NULL == SysAudioDecParamPtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    SysPkgAudioDecParam SysAudioDecParam;
    AudioDecParam DecParam;

    memset(&SysAudioDecParam, 0, sizeof(SysPkgAudioDecParam));
    memcpy(&SysAudioDecParam, SysAudioDecParamPtr, sizeof(SysPkgAudioDecParam));
    memset(&DecParam, 0, sizeof(AudioDecParam));
    DecParam.s_AudioId      = 0;
    switch (SysAudioDecParam.s_Codec)
    {
    case SYS_AUDIO_COMP_G711A:
        DecParam.s_Codec  = CODEC_G711A;
        break;
    default:
        SYS_ERROR("not support this codec %d\n", m_AudioEncParamPtr.GetPtr()->s_Codec);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "not support this codec %d\n", m_AudioEncParamPtr.GetPtr()->s_Codec);
        return GMI_NOT_SUPPORT;
    }
    DecParam.s_BitRate      = SysAudioDecParam.s_BitRate;
    DecParam.s_AecDelayTime = SysAudioDecParam.s_AecDelayTime;
    DecParam.s_AecFlag      = SysAudioDecParam.s_AecFlag;
    DecParam.s_BitWidth     = SysAudioDecParam.s_BitWidth;
    DecParam.s_ChannelNum   = SysAudioDecParam.s_ChannelNum;
    DecParam.s_FrameRate    = SysAudioDecParam.s_FrameRate;
    DecParam.s_SampleFreq   = SysAudioDecParam.s_SampleFreq;
    DecParam.s_Volume       = SysAudioDecParam.s_Volume;

    int32_t Time1 = TimeStamp();
    GMI_RESULT Result = m_StreamCenterClientPtr->Start(0, 0, MEDIA_AUDIO, DecParam.s_Codec, false, &DecParam, sizeof(AudioDecParam), &m_AudioDecHandle);
    if (FAILED(Result))
    {
        SYS_ERROR("Start audio decode fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Start audio decode fail, Result = 0x%lx\n", Result);
        return Result;
    }
    SYS_INFO("Start audio decode wast time %d\n", TimeStamp()-Time1);

    Time1 = TimeStamp();
    Result = StartAudioEncode();
    if (FAILED(Result))
    {
        m_StreamCenterClientPtr->Stop(m_AudioDecHandle);
        SYS_ERROR("Start audio encode fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Start audio encode fail, Result = 0x%lx\n", Result);
        return Result;
    }
    SYS_INFO("Start audio encode wast time %d\n", TimeStamp()-Time1);

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrStopAudioDecode(int32_t AudioId)
{
    int32_t Time1 = TimeStamp();
    GMI_RESULT Result = m_StreamCenterClientPtr->Stop(m_AudioDecHandle);
    if (FAILED(Result))
    {
        SYS_ERROR("stop audio decode fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "stop audio decode fail, Result = 0x%lx\n", Result);
        return Result;
    }
    m_AudioDecHandle = NULL;
    SYS_INFO("Stop audio decode wast time %d\n", TimeStamp()-Time1);

    Time1 = TimeStamp();
    StopAudioEncode();
    SYS_INFO("Stop audio encode wast time %d\n", TimeStamp()-Time1);

    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetVideoStreamNum(int32_t *StreamNum)
{
    *StreamNum = m_VideoStreamNum;
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetVideoEncodeSettings(int StreamId, SysPkgEncodeCfg *SysEncodeCfgPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (StreamId != 0xff
            && StreamId >= m_VideoStreamNum)
    {
        return GMI_INVALID_PARAMETER;
    }

    VideoEncodeParam EncParam;
    memset(&EncParam, 0, sizeof(VideoEncodeParam));
    size_t  ParamLength = sizeof(VideoEncodeParam);
    int32_t Id;

    if (StreamId == 0xff)
    {
        for (Id = 0; Id < m_VideoStreamNum; Id++)
        {
            SysEncodeCfgPtr[Id].s_StreamType = m_VideoStreamTypePtr.GetPtr()[Id];

            memset(&EncParam, 0, ParamLength);
            memcpy(&EncParam, &m_VideoEncParamPtr.GetPtr()[Id], ParamLength);
            switch (EncParam.s_EncodeType)
            {
            case 1://h264
                SysEncodeCfgPtr[Id].s_Compression = SYS_COMP_H264;
                break;
            case 2://mjpeg
                SysEncodeCfgPtr[Id].s_Compression = SYS_COMP_MJPEG;
                break;
            default:
                SYS_ERROR("EncParam.s_EncodeType %d\n", EncParam.s_EncodeType);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "EncParam.s_EncodeType %d\n", EncParam.s_EncodeType);
                return GMI_INVALID_PARAMETER;
            }

            switch (EncParam.s_BitRateType)
            {
            case 1://cbr
                SysEncodeCfgPtr[Id].s_BitrateCtrl = SYS_BRC_CBR;
                break;
            case 2://vbr
                SysEncodeCfgPtr[Id].s_BitrateCtrl = SYS_BRC_VBR;
                break;
            default:
                SYS_ERROR("EncParam.s_BitRateType %d\n", EncParam.s_BitRateType);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "EncParam.s_BitRateType %d\n", EncParam.s_BitRateType);
                return GMI_INVALID_PARAMETER;
            }

            SysEncodeCfgPtr[Id].s_BitRateAverage = EncParam.s_BitRateAverage;
            SysEncodeCfgPtr[Id].s_BitRateUp      = EncParam.s_BitRateUp;
            SysEncodeCfgPtr[Id].s_BitRateDown    = EncParam.s_BitRateDown;
            SysEncodeCfgPtr[Id].s_FPS            = EncParam.s_FrameRate;
            SysEncodeCfgPtr[Id].s_Gop            = EncParam.s_FrameInterval;
            SysEncodeCfgPtr[Id].s_PicHeight      = EncParam.s_EncodeHeight;
            SysEncodeCfgPtr[Id].s_PicWidth       = EncParam.s_EncodeWidth;
            SysEncodeCfgPtr[Id].s_Quality        = FloatToInt((float_t)(EncParam.s_EncodeQulity * 6) / 100);
            SysEncodeCfgPtr[Id].s_Rotate         = EncParam.s_Rotate;
            SysEncodeCfgPtr[Id].s_Flag           = Id;
            SysEncodeCfgPtr[Id].s_VideoId        = 1;
        }
    }
    else
    {
        memcpy(&EncParam, &m_VideoEncParamPtr.GetPtr()[StreamId], ParamLength);

        switch (EncParam.s_EncodeType)
        {
        case 1://h264
            SysEncodeCfgPtr->s_Compression = SYS_COMP_H264;
            break;
        case 2://mjpeg
            SysEncodeCfgPtr->s_Compression = SYS_COMP_MJPEG;
            break;
        default:
            return GMI_INVALID_PARAMETER;
        }

        switch (EncParam.s_BitRateType)
        {
        case 1://cbr
            SysEncodeCfgPtr->s_BitrateCtrl = SYS_BRC_CBR;
            break;
        case 2://vbr
            SysEncodeCfgPtr->s_BitrateCtrl = SYS_BRC_VBR;
            break;
        default:
            return GMI_INVALID_PARAMETER;
        }
        SysEncodeCfgPtr->s_BitRateAverage = EncParam.s_BitRateAverage;
        SysEncodeCfgPtr->s_BitRateUp      = EncParam.s_BitRateUp;
        SysEncodeCfgPtr->s_BitRateDown    = EncParam.s_BitRateDown;
        SysEncodeCfgPtr->s_FPS            = EncParam.s_FrameRate;
        SysEncodeCfgPtr->s_Gop            = EncParam.s_FrameInterval;
        SysEncodeCfgPtr->s_PicHeight      = EncParam.s_EncodeHeight;
        SysEncodeCfgPtr->s_PicWidth       = EncParam.s_EncodeWidth;
        SysEncodeCfgPtr->s_Quality        = FloatToInt((float_t)(EncParam.s_EncodeQulity * 6) / 100);
        SysEncodeCfgPtr->s_Rotate         = EncParam.s_Rotate;
        SysEncodeCfgPtr->s_Flag           = StreamId;
        SysEncodeCfgPtr->s_VideoId        = 1;
    }

    const char_t*  UserData = "get video encode config successfully";
    USER_LOG(g_DefaultLogClient, SYS_LOGMAJOR_OPERATION, SYS_LOGMINOR_REMOTE_GET_PARM, USER_NAME, strlen(USER_NAME), UserData, strlen(UserData));
    
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetVideoEncodeSettingOptions()
{
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::DestroyVideoCodec()
{
    int32_t Id;
    GMI_RESULT Result;

    for (Id = m_VideoStreamNum-1; Id >= 0; Id--)
    {
        Result = m_StreamCenterClientPtr->Stop2((m_VideoCodecHandle.GetPtr())[Id]);
        if (GMI_WAIT_TIMEOUT == Result)//miss the error of "TIMEOUT"
        {
            SYS_ERROR("Stop video codec%d timeout, Result = 0x%lx\n", Id, Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Stop video codec %d timeout, Result = 0x%lx\n", Id, Result);
        }
        else if (FAILED(Result))
        {
            SYS_ERROR("Stop video codec%d fail, Result = 0x%lx\n", Id, Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Stop video codec fail, Result = 0x%lx\n", Result);
            return Result;
        }
    }

    for (Id = m_VideoStreamNum-1; Id >= 0; Id--)
    {
        Result = m_StreamCenterClientPtr->Destroy((m_VideoCodecHandle.GetPtr())[Id]);
        if (GMI_WAIT_TIMEOUT == Result)//miss the error of "TIMEOUT"
        {
            SYS_ERROR("Destroy video codec%d timeout, Result = 0x%lx\n", Id, Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Destroy video codec%d timeout, Result = 0x%lx\n", Id, Result);
        }
        else if (FAILED(Result))
        {
            SYS_ERROR("Destroy video codec%d fail, Result = 0x%lx\n", Id, Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Destroy video codec%d fail, Result = 0x%lx\n", Id, Result);
            return Result;
        }
    }

    return GMI_SUCCESS;
}


//StreamId = 0xff, rebuild all
GMI_RESULT SystemServiceManager::RecreateVideoCodec(int32_t StreamId, VideoEncodeParam *EncParamPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
	//1/16/2014, fix the bug that modify the bitrate of substream will result in the third four stream disapperaed.       
    GMI_RESULT Result = DestroyVideoCodec();
    if (FAILED(Result))
    {
        SYS_ERROR("Destroy video all code fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Destroy video all code fail, Result = 0x%lx\n", Result);
        return Result;
    }

    //create and start       
    int32_t Id;
    for (Id = 0; Id < m_VideoStreamNum; Id++)
    {
    	if (Id == StreamId)
    	{
			Result = m_StreamCenterClientPtr->Create(0, Id, MEDIA_VIDEO, EncParamPtr->s_EncodeType, true, EncParamPtr, sizeof(VideoEncodeParam), &((m_VideoCodecHandle.GetPtr())[Id]));
			if (GMI_WAIT_TIMEOUT == Result)//miss the error of "TIMEOUT"
			{
				SYS_ERROR("Create video codec%d timeout, Result = 0x%lx\n", Id, Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Create video codec%d timeout, Result = 0x%lx\n", Id, Result);
			}				
    	}
    	else
    	{
            Result = m_StreamCenterClientPtr->Create(0, Id, MEDIA_VIDEO, (m_VideoEncParamPtr.GetPtr())[Id].s_EncodeType, true, &((m_VideoEncParamPtr.GetPtr())[Id]), sizeof(VideoEncodeParam), &((m_VideoCodecHandle.GetPtr())[Id]));
            if (GMI_WAIT_TIMEOUT == Result)//miss the error of "TIMEOUT"
            {
                SYS_ERROR("Create video codec%d timeout, Result = 0x%lx\n", Id, Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Create video codec%d timeout, Result = 0x%lx\n", Id, Result);
            }	          
        }

		if (FAILED(Result) && GMI_WAIT_TIMEOUT != Result)
		{
			while (Id--)
			{
			    m_StreamCenterClientPtr->Destroy((m_VideoCodecHandle.GetPtr())[Id]);
			}
			SYS_ERROR("Create codec%d fail, Result = 0x%lx\n", Id, Result);
			DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Create codec%d fail, Result = 0x%lx\n", Id, Result);
			return Result;
		}
    }

    for (Id = 0; Id < m_VideoStreamNum; Id++)
    {
        Result = m_StreamCenterClientPtr->Start2((m_VideoCodecHandle.GetPtr())[Id]);
        if (GMI_WAIT_TIMEOUT == Result)//miss the error of "TIMEOUT"
        {
            SYS_ERROR("Start2 video codec%d timeout, Result = 0x%lx\n", Id, Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Start2 video codec%d timeout, Result = 0x%lx\n", Id, Result);
        }
        else if (FAILED(Result))
        {
            while (Id--)
            {
                m_StreamCenterClientPtr->Stop2((m_VideoCodecHandle.GetPtr())[Id]);
                m_StreamCenterClientPtr->Destroy((m_VideoCodecHandle.GetPtr())[Id]);
            }
            SYS_ERROR("Start2 codec%d fail, Result = 0x%lx\n", Id, Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Start2 codec%d fail, Result = 0x%lx\n", Id, Result);
            return Result;
        }
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetVideoEncodeSetting(int32_t StreamId, SysPkgEncodeCfg *SysEncodeCfgPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (StreamId >= m_VideoStreamNum
            || 1 != SysEncodeCfgPtr->s_VideoId)
    {
        return GMI_INVALID_PARAMETER;
    }

    VideoEncodeParam EncParam;
    memset(&EncParam, 0, sizeof(VideoEncodeParam));
    size_t ParamLength = sizeof(VideoEncodeParam);

    //compression
    switch (SysEncodeCfgPtr->s_Compression)
    {
    case SYS_COMP_H264:
        EncParam.s_EncodeType = 1;
        break;
    case SYS_COMP_MJPEG:
        EncParam.s_EncodeType = 2;
        break;
    case SYS_COMP_MPEG4:
    default:
        SYS_ERROR("compression %d not support\n", SysEncodeCfgPtr->s_Compression);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "compression %d not support\n", SysEncodeCfgPtr->s_Compression);
        return GMI_NOT_SUPPORT;
    }
    //bitrate type
    switch (SysEncodeCfgPtr->s_BitrateCtrl)
    {
    case SYS_BRC_CBR:
        EncParam.s_BitRateType = 1;
        break;
    case SYS_BRC_VBR:
        EncParam.s_BitRateType = 2;
        break;
    default:
        SYS_ERROR("BitrateCtrl %d not support\n", SysEncodeCfgPtr->s_BitrateCtrl);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "BitrateCtrl %d not support\n", SysEncodeCfgPtr->s_BitrateCtrl);
        return GMI_NOT_SUPPORT;
    }

    if (SYS_BRC_CBR == SysEncodeCfgPtr->s_BitrateCtrl)
    {
        int32_t BitRate;
        int32_t BitRateUp;
        int32_t BitRateDown;
        GMI_RESULT Result = m_ConfigFileManagerPtr->GetBitrates(SysEncodeCfgPtr->s_PicWidth, SysEncodeCfgPtr->s_PicHeight, &BitRate, &BitRateUp, &BitRateDown);
        if (FAILED(Result))
        {
            SYS_ERROR("get bitrates from config file\n");
            return Result;
        }

        SysEncodeCfgPtr->s_BitRateUp   = BitRateUp;
        SysEncodeCfgPtr->s_BitRateDown = BitRateDown;
    }

    EncParam.s_BitRateAverage  = SysEncodeCfgPtr->s_BitRateAverage;
    EncParam.s_BitRateUp       = SysEncodeCfgPtr->s_BitRateUp;
    EncParam.s_BitRateDown     = SysEncodeCfgPtr->s_BitRateDown;
    EncParam.s_FrameRate       = SysEncodeCfgPtr->s_FPS;
    EncParam.s_FrameInterval   = SysEncodeCfgPtr->s_Gop;
    EncParam.s_EncodeHeight    = SysEncodeCfgPtr->s_PicHeight;
    EncParam.s_EncodeWidth     = SysEncodeCfgPtr->s_PicWidth;
    EncParam.s_EncodeQulity    = FloatToInt((float_t)(SysEncodeCfgPtr->s_Quality * 100) / 6);
    EncParam.s_Rotate          = SysEncodeCfgPtr->s_Rotate;

    GMI_RESULT Result = m_StreamCenterClientPtr->CheckVideoEncodeConfiguration(&EncParam);
    if (FAILED(Result))
    {
        SYS_ERROR("check video encode param fail, Result = 0x%lx\n", Result);
        return Result;
    }

	
    GMI_RESULT RetCode = GMI_SUCCESS;
    do
    {
        pthread_rwlock_wrlock(&m_Lock);
        //pause sdk stream server
        Result = m_SdkStreamCtlPtr->Pause(3);
        if (FAILED(Result))
        {
            SYS_ERROR("PauseStreamTransfer fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PauseStreamTransfer fail, Result = 0x%lx\n", Result);
            RetCode = Result;
            break;
        }

        Result = m_RtspStreamCtlPtr->Stop(3);
        if (FAILED(Result))
        {
            SYS_ERROR("stop rtsp fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "stop rtsp fail, Result = 0x%lx\n", Result);
            RetCode = Result;
            break;
        }

        boolean_t OsdInit = false;
        //if frameRate  bitrate  bitrateType is changed, need recreate encode
        if (((BIT_CBR == EncParam.s_BitRateType && m_VideoEncParamPtr.GetPtr()[StreamId].s_BitRateAverage != EncParam.s_BitRateAverage)
                || m_VideoEncParamPtr.GetPtr()[StreamId].s_FrameRate != EncParam.s_FrameRate)
                || (BIT_VBR == EncParam.s_BitRateType && m_VideoEncParamPtr.GetPtr()[StreamId].s_BitRateUp != EncParam.s_BitRateUp)
                || (m_VideoEncParamPtr.GetPtr()[StreamId].s_BitRateType != EncParam.s_BitRateType && (m_VideoEncParamPtr.GetPtr()[StreamId].s_BitRateAverage != EncParam.s_BitRateUp || m_VideoEncParamPtr.GetPtr()[StreamId].s_BitRateUp != EncParam.s_BitRateAverage)))
        {
            Result = RecreateVideoCodec(StreamId, &EncParam);
            if (FAILED(Result))
            {
                pthread_rwlock_unlock(&m_Lock);
                SYS_ERROR("RecreateVideoCodec fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RecreateVideoCodec fail, Result = 0x%lx\n", Result);
                return Result;
            }
            OsdInit = true;
        }
        else
        {
            Result = m_StreamCenterClientPtr->SetCodecConfiguration((m_VideoCodecHandle.GetPtr())[StreamId], &EncParam, ParamLength);
            if (GMI_WAIT_TIMEOUT == Result)//miss the error of "TIMEOUT"
            {
                SYS_ERROR("Set Video Encode setting timeout, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Set Video Encode setting timeout, Result = 0x%lx\n", Result);
            }
            else if (FAILED(Result))
            {
                pthread_rwlock_unlock(&m_Lock);
                SYS_ERROR("Set Video Encode setting fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Set Video Encode setting fail, Result = 0x%lx\n", Result);
                return Result;
            }
        }

        if (SUCCEEDED(Result))
        {
            //copy to global encode pointer
            memcpy(&(m_VideoEncParamPtr.GetPtr()[StreamId]), &EncParam, ParamLength);
            //save
            Result = m_ConfigFileManagerPtr->SetVideoEncodeSetting(StreamId, &EncParam);
            if (FAILED(Result))//save file failed, just waring,should gurantee video transfer normally.
            {
                SYS_ERROR("config file SetVideoEncodeSetting fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "config file SetVideoEncodeSetting fail, Result = 0x%lx\n", Result);
            }

            //streamtype is audio_video_stream, then start audio encode
            if (SYS_AUDIO_VIDEO_STREAM == SysEncodeCfgPtr->s_StreamType)
            {
                Result = StartAudioEncode();
                if (FAILED(Result))
                {
                    SYS_ERROR("start audio encode fail, Result = 0x%lx\n", Result);
                }
                else
                {
                    m_VideoStreamTypePtr.GetPtr()[StreamId] = SysEncodeCfgPtr->s_StreamType;
                    Result = m_ConfigFileManagerPtr->SetVideoStreamType(StreamId, SysEncodeCfgPtr->s_StreamType);
                    if (FAILED(Result))//save file failed, just waring,should gurantee video transfer normally.
                    {
                        SYS_ERROR("config file SetVideoStreamType fail, Result = 0x%lx\n", Result);
                        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "config file SetVideoStreamType fail, Result = 0x%lx\n", Result);
                    }
                }
            }
            else //streamtype just video, then stop audio encode
            {
                //process paticular, stop audio encode when all stream is just video by checking m_VideoStreamTypePtr
                int32_t Tmp = m_VideoStreamTypePtr.GetPtr()[StreamId];
                m_VideoStreamTypePtr.GetPtr()[StreamId] = SYS_VIDEO_STREAM;
                Result = StopAudioEncode();
                if (FAILED(Result))
                {
                    SYS_ERROR("stop audio encode fail, Result = 0x%lx\n", Result);
                    m_VideoStreamTypePtr.GetPtr()[StreamId] = Tmp;
                }
                else
                {
                    m_VideoStreamTypePtr.GetPtr()[StreamId] = SysEncodeCfgPtr->s_StreamType;
                    Result = m_ConfigFileManagerPtr->SetVideoStreamType(StreamId, SysEncodeCfgPtr->s_StreamType);
                    if (FAILED(Result))//save file failed, just waring,should gurantee video transfer normally.
                    {
                        SYS_ERROR("config file SetVideoStreamType fail, Result = 0x%lx\n", Result);
                        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "config file SetVideoStreamType fail, Result = 0x%lx\n", Result);
                    }
                }
            }
        }

        if (OsdInit)
        {
            //Osd init
            Result = OsdsInit(m_VideoStreamNum, m_VideoEncParamPtr.GetPtr(), m_VideoOsdParamPtr.GetPtr());
            if (FAILED(Result))
            {
                RetCode = Result;
                SYS_ERROR("OSD init fail, Result = 0x%lx\n", Result);
                break;
            }
        }
    }
    while (0);

    //allocate SysPkgEncodeCfg
    ReferrencePtr<SysPkgEncodeCfg, DefaultObjectsDeleter>SysEncCfgPtr(BaseMemoryManager::Instance().News<SysPkgEncodeCfg>(m_VideoStreamNum));
    if (NULL == SysEncCfgPtr.GetPtr())
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SysEncCfgPtr news fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysEncCfgPtr news fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    //get all video encode parameter
    Result = SvrGetVideoEncodeSettings(0xff, SysEncCfgPtr.GetPtr());
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SvrGetVideoEncodeSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SvrGetVideoEncodeSettings fail, Result = 0x%lx\n", Result);
        return Result;
    }

    //allocate SysPkgAudioEncodeCfg
    ReferrencePtr<SysPkgAudioEncodeCfg>SysAudioEncodeCfgPtr(BaseMemoryManager::Instance().New<SysPkgAudioEncodeCfg>());
    if (NULL == SysAudioEncodeCfgPtr.GetPtr())
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("audio encode config fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "audio encode config fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    memset(SysAudioEncodeCfgPtr.GetPtr(), 0, sizeof(SysPkgAudioEncodeCfg));

    //get all audio encode parameter
    Result = SvrGetAudioEncodeSetting(1, SysAudioEncodeCfgPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("Svr get audio encode config fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Svr get audio encode config fail, Result = 0x%lx\n", Result);
    }

    //restart sdk stream server
    Result = m_SdkStreamCtlPtr->Resume(SysEncCfgPtr.GetPtr(), m_VideoStreamNum, SysAudioEncodeCfgPtr.GetPtr(), 1, 3);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("ResumeStreamTransfer fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ResumeStreamTransfer fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_RtspStreamCtlPtr->Start(SysEncCfgPtr.GetPtr(), m_VideoStreamNum, SysAudioEncodeCfgPtr.GetPtr(), 1, 3);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("start rtsp fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "start rtsp fail, Result = 0x%lx\n", Result);
        return Result;
    }
    pthread_rwlock_unlock(&m_Lock);

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);   
    
    if (FAILED(Result))
    {
        return Result;
    }
    else
    {
    	const char_t*  UserData = "set video encode config successfully";
    	USER_LOG(g_DefaultLogClient, SYS_LOGMAJOR_OPERATION, SYS_LOGMINOR_REMOTE_CFG_PARM, USER_NAME, strlen(USER_NAME), UserData, strlen(UserData));
        return RetCode;
    }
}


GMI_RESULT SystemServiceManager::StartAudioEncode(void)
{
    if (NULL == m_AudioEncHandle)
    {
        GMI_RESULT Result = m_StreamCenterClientPtr->Start(0, 0, MEDIA_AUDIO, m_AudioEncParamPtr.GetPtr()->s_Codec, true, m_AudioEncParamPtr.GetPtr(), sizeof(AudioEncParam), &m_AudioEncHandle);
        if (FAILED(Result))
        {
            SYS_ERROR("Start audio encode fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Start audio encode fail, Result = 0x%lx\n", Result);
            return Result;
        }
    }

    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::StopAudioEncode(void)
{
    boolean_t AudVidStreamExist;

    AudVidStreamIsExis(&AudVidStreamExist);

    //all stream is just only video and talk close,then it can close audio encode.
    if (!AudVidStreamExist && NULL == m_AudioDecHandle)
    {
        if (NULL != m_AudioEncHandle)
        {
            GMI_RESULT Result = m_StreamCenterClientPtr->Stop(m_AudioEncHandle);
            if (FAILED(Result))
            {
                SYS_ERROR("Stop audio encode fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Stop audio encode fail, Result = 0x%lx\n", Result);
                return Result;
            }
            m_AudioEncHandle = NULL;
        }
    }

    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetVideoEncStreamCombine(SysPkgEncStreamCombine *SysEncStreamCombinePtr)
{
    SysPkgEncStreamCombine SysEncStreamCombine;
    memset(&SysEncStreamCombine, 0, sizeof(SysPkgEncStreamCombine));

    GMI_RESULT Result = m_ConfigFileManagerPtr->GetStreamCombine(&SysEncStreamCombine);
    if (FAILED(Result))
    {
        SYS_ERROR("get video stream combine fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "get video stream combine fail, Result = 0x%lx\n", Result);
        return Result;
    }

    memcpy(SysEncStreamCombinePtr, &SysEncStreamCombine, sizeof(SysPkgEncStreamCombine));
    
    const char_t*  UserData = "get video stream combine successfully";
    USER_LOG(g_DefaultLogClient, SYS_LOGMAJOR_OPERATION, SYS_LOGMINOR_REMOTE_GET_PARM, USER_NAME, strlen(USER_NAME), UserData, strlen(UserData));

    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::CheckShutterAndChange(int32_t VidInFps)
{
    SYS_INFO("%s in.......\n", __func__);

    switch(VidInFps)
    {
    case 20:
        VidInFps = 25;
        break;
    }

    VidInFps = (1/(float_t)VidInFps) * 1000000;

    if (m_ImageParamPtr.GetPtr()->s_ExposureValueMax != VidInFps)
    {
        ImageBaseParam Imaging;
        memcpy(&Imaging, m_ImageParamPtr.GetPtr(), sizeof(ImageBaseParam));
        Imaging.s_ExposureValueMax = VidInFps;
        SYS_INFO("Imaging.s_ExposureValueMax %d\n", Imaging.s_ExposureValueMax);
        GMI_RESULT Result = m_StreamCenterClientPtr->SetImageConfiguration(m_ImageHandle, &Imaging);
        if (FAILED(Result))
        {
            SYS_ERROR("SetImageConfiguration fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetImageConfiguration fail, Result = 0x%lx\n", Result);
            return Result;
        }

        m_ImageParamPtr.GetPtr()->s_ExposureValueMax = Imaging.s_ExposureValueMax;
        Result = m_ConfigFileManagerPtr->SetImageSettings(0, 0, &Imaging);
        if (FAILED(Result))
        {
            SYS_ERROR("SetImageSettings fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetImageSettings fail, Result = 0x%lx\n", Result);
            return Result;
        }
    }
    SYS_INFO("%s normal out.......\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetVideoEncStreamCombine(SysPkgEncStreamCombine *SysEncStreamCombinePtr)
{
    SysPkgEncStreamCombine ToSet_SysEncStreamCombine;
    SysPkgEncStreamCombine Current_SysEncStreamCombine;

    memset(&ToSet_SysEncStreamCombine, 0, sizeof(SysPkgEncStreamCombine));
    memcpy(&ToSet_SysEncStreamCombine, SysEncStreamCombinePtr, sizeof(SysPkgEncStreamCombine));

    //get current stream combine
    GMI_RESULT Result = m_ConfigFileManagerPtr->GetStreamCombine(&Current_SysEncStreamCombine);
    if (FAILED(Result))
    {
        SYS_ERROR("get video stream combine fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "get video stream combine fail, Result = 0x%lx\n", Result);
        return Result;
    }

    if (Current_SysEncStreamCombine.s_EnableStreamNum == ToSet_SysEncStreamCombine.s_EnableStreamNum
            && Current_SysEncStreamCombine.s_StreamCombineNo == ToSet_SysEncStreamCombine.s_StreamCombineNo)
    {
        return GMI_SUCCESS;
    }

    SYS_INFO("s_EnableStreamNum %d,  s_StreamCombineNo %d\n", ToSet_SysEncStreamCombine.s_EnableStreamNum, ToSet_SysEncStreamCombine.s_StreamCombineNo);

    //get video encode config accroding to stream combine config
    VideoEncodeParam EncParam[MAX_VIDEO_STREAM_NUM];
    Result = m_ConfigFileManagerPtr->GetVideoEncodeSettingsDefault(0xff, &ToSet_SysEncStreamCombine, EncParam);
    if (FAILED(Result))
    {
        SYS_ERROR("GetVideoEncodeSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetVideoEncodeSettings fail, Result = 0x%lx\n", Result);
        return Result;
    }

    GMI_RESULT RetCode = GMI_SUCCESS;

    do
    {
        pthread_rwlock_wrlock(&m_Lock);
        //pause sdk stream server
        Result = m_SdkStreamCtlPtr->Pause(3);
        if (FAILED(Result))
        {
            SYS_ERROR("PauseStreamTransfer fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PauseStreamTransfer fail, Result = 0x%lx\n", Result);
            RetCode = Result;
            break;
        }

        Result = m_RtspStreamCtlPtr->Stop(3);
        if (FAILED(Result))
        {
            SYS_ERROR("stop rtsp fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "stop rtsp fail, Result = 0x%lx\n", Result);
            RetCode = Result;
            break;
        }

        //delete all current  video codec
        Result = DestroyVideoCodec();
        if (FAILED(Result))
        {
            pthread_rwlock_unlock(&m_Lock);
            return Result;
        }

        //set vidin
        int32_t MaxFrameRate  = 0;
        for (int32_t Id = 0; Id < ToSet_SysEncStreamCombine.s_EnableStreamNum; Id++)
        {
            if (EncParam[Id].s_FrameRate > MaxFrameRate)
            {
                MaxFrameRate = EncParam[Id].s_FrameRate;
            }
        }

        if (m_VideoSourcePtr.GetPtr()->s_SrcFps != MaxFrameRate)
        {
            VideoInParam  Vin;
            VideoOutParam Vout;
            Result = m_StreamCenterClientPtr->GetVinVoutConfiguration(m_VideoInOutHandle, &Vin, &Vout);
            if (FAILED(Result))
            {
                pthread_rwlock_unlock(&m_Lock);
                SYS_ERROR("GetVinVoutConfiguration fail, Result = 0x%lx\n", Result);
                return Result;
            }

            Vin.s_VinFrameRate = MaxFrameRate;
            Vin.s_VinMirrorPattern = m_VideoSourcePtr.GetPtr()->s_Mirror;
            Result = m_StreamCenterClientPtr->SetVinVoutConfiguration(m_VideoInOutHandle, &Vin, &Vout);
            if (FAILED(Result))
            {
                pthread_rwlock_unlock(&m_Lock);
                SYS_ERROR("SetVinVoutConfiguration fail, Result = 0x%lx\n", Result);
                return Result;
            }

            m_VideoSourcePtr.GetPtr()->s_SrcFps = MaxFrameRate;
            Result = m_ConfigFileManagerPtr->SetVideoSourceSettings(1, m_VideoSourcePtr.GetPtr());
            if (FAILED(Result))//save file failed, just waring,should gurantee video transfer normally.
            {
                SYS_ERROR("Save System video source setting fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Save System video source setting fail, Result = 0x%lx\n", Result);
            }
        }

		//create users video codec
		int32_t Id;
		for (Id = 0; Id < ToSet_SysEncStreamCombine.s_EnableStreamNum; Id++)
		{			
			EncParam[Id].s_Rotate = (m_VideoEncParamPtr.GetPtr())[Id].s_Rotate;//set combine, rotate should keep original.
		    Result = m_StreamCenterClientPtr->Create(0, Id, MEDIA_VIDEO, EncParam[Id].s_EncodeType, true, &EncParam[Id], sizeof(VideoEncodeParam), &((m_VideoCodecHandle.GetPtr())[Id]));
	    	if (FAILED(Result))
		    {
		    	while (Id--)
		    	{
		    		m_StreamCenterClientPtr->Destroy((m_VideoCodecHandle.GetPtr())[Id]);
		    	}
		    	pthread_rwlock_unlock(&m_Lock);
		        SYS_ERROR("Create codec%d fail, Result = 0x%lx\n", Id, Result);
		        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Create codec%d fail, Result = 0x%lx\n", Id, Result);
		        return Result;
		    }
		}

        for (Id = 0; Id < ToSet_SysEncStreamCombine.s_EnableStreamNum; Id++)
        {
            Result = m_StreamCenterClientPtr->Start2((m_VideoCodecHandle.GetPtr())[Id]);
            if (FAILED(Result))
            {
                while (Id--)
                {
                    m_StreamCenterClientPtr->Stop2((m_VideoCodecHandle.GetPtr())[Id]);
                    m_StreamCenterClientPtr->Destroy((m_VideoCodecHandle.GetPtr())[Id]);
                }
                pthread_rwlock_unlock(&m_Lock);
                SYS_ERROR("Start2 codec%d fail, Result = 0x%lx\n", Id, Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Start2 codec%d fail, Result = 0x%lx\n", Id, Result);
                return Result;
            }
        }

        //copy to golbal var
        m_VideoStreamNum = ToSet_SysEncStreamCombine.s_EnableStreamNum;
        memcpy(m_VideoEncParamPtr.GetPtr(), EncParam, sizeof(VideoEncodeParam)*m_VideoStreamNum);

        //set current streamnum to file
        Result = m_ConfigFileManagerPtr->SetVideoEncodeStreamNum(m_VideoStreamNum);
        if (FAILED(Result))
        {
            pthread_rwlock_unlock(&m_Lock);
            return Result;
        }

        //set stream combine to file
        Result = m_ConfigFileManagerPtr->SetStreamCombine(&ToSet_SysEncStreamCombine);
        if (FAILED(Result))
        {
            pthread_rwlock_unlock(&m_Lock);
            return Result;
        }

        //set video encode to file
        for (int32_t i = 0; i < m_VideoStreamNum; i++)
        {
            Result = m_ConfigFileManagerPtr->SetVideoEncodeSetting(i, &(m_VideoEncParamPtr.GetPtr()[i]));
            if (FAILED(Result))
            {
                pthread_rwlock_unlock(&m_Lock);
                return Result;
            }
        }

        //check shutter time, and change it
        Result = CheckShutterAndChange(MaxFrameRate);
        if (FAILED(Result))
        {
            SYS_ERROR("CheckShutterAndChange fail, Result = 0x%lx\n", Result);
        }

        //osd init again, this task schedule after encoder create & start, gurantee osd fail do not affect video
        VideoOSDParam VideoOsdParam[MAX_VIDEO_STREAM_NUM];
        Result = m_ConfigFileManagerPtr->GetOsdSettings(0xff, ToSet_SysEncStreamCombine.s_EnableStreamNum, VideoOsdParam);
        if (FAILED(Result))
        {
            SYS_ERROR("GetOsdSettings fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetOsdSettings fail, Result = 0x%lx\n", Result);
            RetCode = Result;
            break;
        }

        Result = OsdsInit(m_VideoStreamNum, m_VideoEncParamPtr.GetPtr(), VideoOsdParam);
        if (FAILED(Result))
        {
            RetCode = Result;
            SYS_ERROR("osds init fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "osds init fail, Result = 0x%lx\n", Result);
            break;
        }
        memcpy(m_VideoOsdParamPtr.GetPtr(), VideoOsdParam, sizeof(VideoOSDParam)*m_VideoStreamNum);
    }
    while (0);

    //allocate SysPkgEncodeCfg
    ReferrencePtr<SysPkgEncodeCfg, DefaultObjectsDeleter>SysEncCfgPtr(BaseMemoryManager::Instance().News<SysPkgEncodeCfg>(m_VideoStreamNum));
    if (NULL == SysEncCfgPtr.GetPtr())
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SysEncCfgPtr news fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysEncCfgPtr news fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    //get all video encode parameter
    Result = SvrGetVideoEncodeSettings(0xff, SysEncCfgPtr.GetPtr());
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SvrGetVideoEncodeSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SvrGetVideoEncodeSettings fail, Result = 0x%lx\n", Result);
        return Result;
    }

    //allocate SysPkgAudioEncodeCfg
    ReferrencePtr<SysPkgAudioEncodeCfg>SysAudioEncodeCfgPtr(BaseMemoryManager::Instance().New<SysPkgAudioEncodeCfg>());
    if (NULL == SysAudioEncodeCfgPtr.GetPtr())
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("audio encode config fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "audio encode config fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    memset(SysAudioEncodeCfgPtr.GetPtr(), 0, sizeof(SysPkgAudioEncodeCfg));

    //get all audio encode parameter
    Result = SvrGetAudioEncodeSetting(1, SysAudioEncodeCfgPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("Svr get audio encode config fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Svr get audio encode config fail, Result = 0x%lx\n", Result);
    }

    //restart sdk stream server
    Result = m_SdkStreamCtlPtr->Resume(SysEncCfgPtr.GetPtr(), m_VideoStreamNum, SysAudioEncodeCfgPtr.GetPtr(), 1, 3);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("ResumeStreamTransfer fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ResumeStreamTransfer fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_RtspStreamCtlPtr->Start(SysEncCfgPtr.GetPtr(), m_VideoStreamNum, SysAudioEncodeCfgPtr.GetPtr(), 1, 3);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("start rtsp fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "start rtsp fail, Result = 0x%lx\n", Result);
        return Result;
    }
    pthread_rwlock_unlock(&m_Lock);

    const char_t*  UserData = "set video stream combine successfully";
    USER_LOG(g_DefaultLogClient, SYS_LOGMAJOR_OPERATION, SYS_LOGMINOR_REMOTE_CFG_PARM, USER_NAME, strlen(USER_NAME), UserData, strlen(UserData));
    
    if (FAILED(Result))
    {
        return Result;
    }
    else
    {
        return RetCode;
    }
}


GMI_RESULT SystemServiceManager::SvrSysAudioEncodeTransferMediaAudio(SysPkgAudioEncodeCfg *SysEncodeCfgPtr, AudioEncParam *AudioEncodePtr, AudioDecParam *AudioDecodePtr)
{
    switch (SysEncodeCfgPtr->s_EncodeType)
    {
    case SYS_AUDIO_COMP_G711A:
        AudioEncodePtr->s_Codec = CODEC_G711A;
        AudioDecodePtr->s_Codec = CODEC_G711A;
        break;
    default:
        SYS_ERROR("not support this codec %d\n", SysEncodeCfgPtr->s_EncodeType);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "not support this codec %d\n", SysEncodeCfgPtr->s_EncodeType);
        return GMI_NOT_SUPPORT;
    }
    //encode
    AudioEncodePtr->s_AudioId      = 0;
    AudioEncodePtr->s_ChannelNum   = SysEncodeCfgPtr->s_Chan;
    AudioEncodePtr->s_BitWidth     = SysEncodeCfgPtr->s_BitsPerSample;
    AudioEncodePtr->s_SampleFreq   = SysEncodeCfgPtr->s_SamplesPerSec;
    AudioEncodePtr->s_Volume       = SysEncodeCfgPtr->s_CapVolume;
    AudioEncodePtr->s_AecFlag      = SysEncodeCfgPtr->s_AecFlag;
    AudioEncodePtr->s_AecDelayTime = SysEncodeCfgPtr->s_AecDelayTime;
    AudioEncodePtr->s_FrameRate    = SysEncodeCfgPtr->s_FrameRate;
    AudioEncodePtr->s_BitRate      = SysEncodeCfgPtr->s_BitRate;
    //decode
    AudioDecodePtr->s_AudioId      = 0;
    AudioDecodePtr->s_Volume       = SysEncodeCfgPtr->s_PlayVolume;
    AudioDecodePtr->s_ChannelNum   = SysEncodeCfgPtr->s_Chan;
    AudioDecodePtr->s_BitWidth     = SysEncodeCfgPtr->s_BitsPerSample;
    AudioDecodePtr->s_SampleFreq   = SysEncodeCfgPtr->s_SamplesPerSec;
    AudioDecodePtr->s_AecFlag      = SysEncodeCfgPtr->s_AecFlag;
    AudioDecodePtr->s_AecDelayTime = SysEncodeCfgPtr->s_AecDelayTime;
    AudioDecodePtr->s_FrameRate    = SysEncodeCfgPtr->s_FrameRate;
    AudioDecodePtr->s_BitRate      = SysEncodeCfgPtr->s_BitRate;

    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrMeidaAudioTransferSysAudioEncode(AudioEncParam *AudioEncodePtr, AudioDecParam *AudioDecodePtr, SysPkgAudioEncodeCfg *SysEncodeCfgPtr)
{
    SysEncodeCfgPtr->s_AudioId = 1;
    switch (AudioEncodePtr->s_Codec)
    {
    case CODEC_G711A:
        SysEncodeCfgPtr->s_EncodeType = SYS_AUDIO_COMP_G711A;
        break;
    default:
        SYS_ERROR("not support this codec %d\n", AudioEncodePtr->s_Codec);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "not support this codec %d\n", AudioEncodePtr->s_Codec);
        return GMI_NOT_SUPPORT;
    }
    SysEncodeCfgPtr->s_Chan          = AudioEncodePtr->s_ChannelNum;
    SysEncodeCfgPtr->s_BitsPerSample = AudioEncodePtr->s_BitWidth;
    SysEncodeCfgPtr->s_SamplesPerSec = AudioEncodePtr->s_SampleFreq;
    SysEncodeCfgPtr->s_CapVolume     = AudioEncodePtr->s_Volume;
    SysEncodeCfgPtr->s_PlayVolume    = AudioDecodePtr->s_Volume;
    SysEncodeCfgPtr->s_PlayEnable    = 0;
    SysEncodeCfgPtr->s_AecFlag       = AudioEncodePtr->s_AecFlag;
    SysEncodeCfgPtr->s_AecDelayTime  = AudioEncodePtr->s_AecDelayTime;
    SysEncodeCfgPtr->s_FrameRate     = AudioEncodePtr->s_FrameRate;
    SysEncodeCfgPtr->s_BitRate       = AudioEncodePtr->s_BitRate;

    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetAudioEncodeSetting(int32_t AudioId, SysPkgAudioEncodeCfg *SysEncodeCfgPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (NULL == SysEncodeCfgPtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    SysPkgAudioEncodeCfg SysAudioEncodeCfg;
    memset(&SysAudioEncodeCfg, 0, sizeof(SysPkgAudioEncodeCfg));
    GMI_RESULT Result = SvrMeidaAudioTransferSysAudioEncode(m_AudioEncParamPtr.GetPtr(), m_AudioDecParamPtr.GetPtr(), &SysAudioEncodeCfg);
    if (FAILED(Result))
    {
        SYS_ERROR("SvrMeidaAudioTransferSysAudioEncode fail, Result = 0x%lx\n", Result);
        return Result;
    }

    memcpy(SysEncodeCfgPtr, &SysAudioEncodeCfg, sizeof(SysPkgAudioEncodeCfg));

    const char_t*  UserData = "get audio encode config successfully";
    USER_LOG(g_DefaultLogClient, SYS_LOGMAJOR_OPERATION, SYS_LOGMINOR_REMOTE_GET_PARM, USER_NAME, strlen(USER_NAME), UserData, strlen(UserData));
    
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetAudioEncodeSetting(int32_t AudioId, SysPkgAudioEncodeCfg *SysEncodeCfgPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    pthread_rwlock_wrlock(&m_Lock);
    SysPkgAudioEncodeCfg SysAudioEncodeCfg;
    memcpy(&SysAudioEncodeCfg, SysEncodeCfgPtr, sizeof(SysPkgAudioEncodeCfg));
    AudioEncParam AudioEncodeParam;
    AudioDecParam AudioDecodeParam;
    GMI_RESULT Result = SvrSysAudioEncodeTransferMediaAudio(&SysAudioEncodeCfg, &AudioEncodeParam, &AudioDecodeParam);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SvrSysAudioEncodeTransferMediaAudio fail, Result = 0x%lx\n", Result);
        return Result;
    }

    if (NULL != m_AudioEncHandle)
    {
        Result = m_StreamCenterClientPtr->SetCodecConfiguration(m_AudioEncHandle, &AudioEncodeParam, sizeof(AudioEncParam));
        if (FAILED(Result))
        {
            pthread_rwlock_unlock(&m_Lock);
            SYS_ERROR("Set audio encode configuration fail, Result = 0x%lx\n", Result);
            return Result;
        }
    }

    if (NULL != m_AudioDecHandle)
    {
        Result = m_StreamCenterClientPtr->SetCodecConfiguration(m_AudioDecHandle, &AudioDecodeParam, sizeof(AudioDecParam));
        if (FAILED(Result))
        {
            if (NULL != m_AudioEncHandle)
            {
                m_StreamCenterClientPtr->SetCodecConfiguration(m_AudioEncHandle, m_AudioEncParamPtr.GetPtr(), sizeof(AudioEncParam));
            }
            pthread_rwlock_unlock(&m_Lock);
            SYS_ERROR("Set audio decode configuration fail, Result = 0x%lx\n", Result);
            return Result;
        }
    }

    memcpy(m_AudioEncParamPtr.GetPtr(), &AudioEncodeParam, sizeof(AudioEncParam));
    memcpy(m_AudioDecParamPtr.GetPtr(), &AudioDecodeParam, sizeof(AudioDecParam));

    Result = m_ConfigFileManagerPtr->SetAudioEncodeSettings(&SysAudioEncodeCfg);
    if (FAILED(Result))
    {
        SYS_ERROR("save audio config fail, Result = 0x%lx\n", Result);
        //return Result;
    }
    pthread_rwlock_unlock(&m_Lock);
    
    const char_t*  UserData = "set audio encode config successfully";
    USER_LOG(g_DefaultLogClient, SYS_LOGMAJOR_OPERATION, SYS_LOGMINOR_REMOTE_CFG_PARM, USER_NAME, strlen(USER_NAME), UserData, strlen(UserData));
    
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrForceIdrFrame(int32_t StreamId)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (StreamId >= m_VideoStreamNum)
    {
        SYS_ERROR("StreamId %d error\n", StreamId);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StreamId %d error\n", StreamId);
        return GMI_INVALID_PARAMETER;
    }

    pthread_rwlock_wrlock(&m_Lock);
    GMI_RESULT Result = m_StreamCenterClientPtr->ForceGenerateIdrFrame((m_VideoCodecHandle.GetPtr())[StreamId]);
    if (FAILED(Result))
    {
        SYS_ERROR("ForceGenerateIdrFrame Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ForceGenerateIdrFrame Result = 0x%lx\n", Result);
        pthread_rwlock_unlock(&m_Lock);
        return Result;
    }
    pthread_rwlock_unlock(&m_Lock);

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetShowCfg(int StreamId, SysPkgShowCfg *SysShowCfgPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    int32_t Id;

    if (NULL == SysShowCfgPtr)
    {
        SYS_ERROR("SysShowCfg is null\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysShowCfg is null\n");
        return GMI_INVALID_PARAMETER;
    }

    if (StreamId != 0xff
            && StreamId >= m_VideoStreamNum)
    {
        SYS_ERROR("StreamId = %d error\n", StreamId);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StreamId = %d error\n", StreamId);
        return GMI_INVALID_PARAMETER;
    }

    if (NULL == m_VideoOsdParamPtr.GetPtr())
    {
        SYS_ERROR("m_VideoOsdParamPtr.GetPtr() is null\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_VideoOsdParamPtr.GetPtr() is null\n");
        return GMI_NOT_SUPPORT;
    }

    if (0xff == StreamId)
    {
        pthread_rwlock_rdlock(&m_Lock);
        for (Id = 0; Id < m_VideoStreamNum; Id++)
        {
            SysShowCfgPtr[Id].s_VideoId                   = 1;
            SysShowCfgPtr[Id].s_Flag                      = Id;
            SysShowCfgPtr[Id].s_TimeInfo.s_Enable         = (m_VideoOsdParamPtr.GetPtr())[Id].s_TimeDisplay.s_Flag;
            SysShowCfgPtr[Id].s_TimeInfo.s_Language       = (m_VideoOsdParamPtr.GetPtr())[Id].s_Language;
            SysShowCfgPtr[Id].s_TimeInfo.s_DisplayX       = (m_VideoOsdParamPtr.GetPtr())[Id].s_TimeDisplay.s_DisplayX;
            SysShowCfgPtr[Id].s_TimeInfo.s_DisplayY       = (m_VideoOsdParamPtr.GetPtr())[Id].s_TimeDisplay.s_DisplayY;
            SysShowCfgPtr[Id].s_TimeInfo.s_DateStyle      = (m_VideoOsdParamPtr.GetPtr())[Id].s_TimeDisplay.s_DateStyle;
            SysShowCfgPtr[Id].s_TimeInfo.s_TimeStyle      = (m_VideoOsdParamPtr.GetPtr())[Id].s_TimeDisplay.s_TimeStyle;
            SysShowCfgPtr[Id].s_TimeInfo.s_FontColor      = (m_VideoOsdParamPtr.GetPtr())[Id].s_TimeDisplay.s_FontColor;
            SysShowCfgPtr[Id].s_TimeInfo.s_FontSize       = (m_VideoOsdParamPtr.GetPtr())[Id].s_TimeDisplay.s_FontStyle;
            SysShowCfgPtr[Id].s_TimeInfo.s_FontBlod       = (m_VideoOsdParamPtr.GetPtr())[Id].s_TimeDisplay.s_FontBlod;
            SysShowCfgPtr[Id].s_TimeInfo.s_FontRotate     = (m_VideoOsdParamPtr.GetPtr())[Id].s_TimeDisplay.s_Rotate;
            SysShowCfgPtr[Id].s_TimeInfo.s_FontItalic     = (m_VideoOsdParamPtr.GetPtr())[Id].s_TimeDisplay.s_Italic;
            SysShowCfgPtr[Id].s_TimeInfo.s_FontOutline    = (m_VideoOsdParamPtr.GetPtr())[Id].s_TimeDisplay.s_Outline;
            SysShowCfgPtr[Id].s_ChannelInfo.s_Enable      = (m_VideoOsdParamPtr.GetPtr())[Id].s_TextDisplay[0].s_Flag;
            SysShowCfgPtr[Id].s_ChannelInfo.s_DisplayX    = (m_VideoOsdParamPtr.GetPtr())[Id].s_TextDisplay[0].s_DisplayX;
            SysShowCfgPtr[Id].s_ChannelInfo.s_DisplayY    = (m_VideoOsdParamPtr.GetPtr())[Id].s_TextDisplay[0].s_DisplayY;
            SysShowCfgPtr[Id].s_ChannelInfo.s_FontColor   = (m_VideoOsdParamPtr.GetPtr())[Id].s_TextDisplay[0].s_FontColor;
            SysShowCfgPtr[Id].s_ChannelInfo.s_FontSize    = (m_VideoOsdParamPtr.GetPtr())[Id].s_TextDisplay[0].s_FontStyle;
            SysShowCfgPtr[Id].s_ChannelInfo.s_FontBlod    = (m_VideoOsdParamPtr.GetPtr())[Id].s_TextDisplay[0].s_FontBlod;
            SysShowCfgPtr[Id].s_ChannelInfo.s_FontRotate  = (m_VideoOsdParamPtr.GetPtr())[Id].s_TextDisplay[0].s_Rotate;
            SysShowCfgPtr[Id].s_ChannelInfo.s_FontItalic  = (m_VideoOsdParamPtr.GetPtr())[Id].s_TextDisplay[0].s_Italic;
            SysShowCfgPtr[Id].s_ChannelInfo.s_FontOutline = (m_VideoOsdParamPtr.GetPtr())[Id].s_TextDisplay[0].s_Outline;
            memcpy(SysShowCfgPtr[Id].s_ChannelInfo.s_ChannelName, (char_t*)(m_VideoOsdParamPtr.GetPtr())[Id].s_TextDisplay[0].s_TextContent, sizeof(SysShowCfgPtr[Id].s_ChannelInfo.s_ChannelName));
        }
        pthread_rwlock_unlock(&m_Lock);
    }

    const char_t*  UserData = "get osd config successfully";
    USER_LOG(g_DefaultLogClient, SYS_LOGMAJOR_OPERATION, SYS_LOGMINOR_REMOTE_GET_PARM, USER_NAME, strlen(USER_NAME), UserData, strlen(UserData));
    
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetShowCfg(int StreamId, SysPkgShowCfg *SysShowCfgPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (NULL == SysShowCfgPtr)
    {
        SYS_ERROR("SysShowCfg is null\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysShowCfg is null\n");
        return GMI_INVALID_PARAMETER;
    }

    if (StreamId >= m_VideoStreamNum)
    {
        SYS_ERROR("StreamId %d error\n", StreamId);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StreamId %d error\n", StreamId);
        return GMI_INVALID_PARAMETER;
    }

    VideoOSDParam VidOsdParam;

    memset(&VidOsdParam, 0, sizeof(VideoOSDParam));
    memcpy(&VidOsdParam, &((m_VideoOsdParamPtr.GetPtr())[StreamId]), sizeof(VideoOSDParam));
    VidOsdParam.s_StreamId                        = SysShowCfgPtr->s_Flag;
    VidOsdParam.s_Language                        = SysShowCfgPtr->s_TimeInfo.s_Language;
    VidOsdParam.s_TimeDisplay.s_Flag              = SysShowCfgPtr->s_TimeInfo.s_Enable;
    VidOsdParam.s_TimeDisplay.s_DateStyle         = SysShowCfgPtr->s_TimeInfo.s_DateStyle;
    VidOsdParam.s_TimeDisplay.s_TimeStyle         = SysShowCfgPtr->s_TimeInfo.s_TimeStyle;
    VidOsdParam.s_TimeDisplay.s_FontColor         = SysShowCfgPtr->s_TimeInfo.s_FontColor;
    VidOsdParam.s_TimeDisplay.s_FontStyle         = SysShowCfgPtr->s_TimeInfo.s_FontSize;
    VidOsdParam.s_TimeDisplay.s_FontBlod          = SysShowCfgPtr->s_TimeInfo.s_FontBlod;
    VidOsdParam.s_TimeDisplay.s_Rotate            = SysShowCfgPtr->s_TimeInfo.s_FontRotate;
    VidOsdParam.s_TimeDisplay.s_Italic            = SysShowCfgPtr->s_TimeInfo.s_FontItalic;
    VidOsdParam.s_TimeDisplay.s_Outline           = SysShowCfgPtr->s_TimeInfo.s_FontOutline;
    VidOsdParam.s_TimeDisplay.s_DisplayX          = SysShowCfgPtr->s_TimeInfo.s_DisplayX;
    VidOsdParam.s_TimeDisplay.s_DisplayY          = SysShowCfgPtr->s_TimeInfo.s_DisplayY;
    VidOsdParam.s_TextDisplay[0].s_Flag           = SysShowCfgPtr->s_ChannelInfo.s_Enable;
    VidOsdParam.s_TextDisplay[0].s_FontColor      = SysShowCfgPtr->s_ChannelInfo.s_FontColor;
    VidOsdParam.s_TextDisplay[0].s_FontStyle      = SysShowCfgPtr->s_ChannelInfo.s_FontSize;
    VidOsdParam.s_TextDisplay[0].s_FontBlod       = SysShowCfgPtr->s_ChannelInfo.s_FontBlod;
    VidOsdParam.s_TextDisplay[0].s_Rotate         = SysShowCfgPtr->s_ChannelInfo.s_FontRotate;
    VidOsdParam.s_TextDisplay[0].s_Italic         = SysShowCfgPtr->s_ChannelInfo.s_FontItalic;
    VidOsdParam.s_TextDisplay[0].s_Outline        = SysShowCfgPtr->s_ChannelInfo.s_FontOutline;
    VidOsdParam.s_TextDisplay[0].s_DisplayX       = SysShowCfgPtr->s_ChannelInfo.s_DisplayX;
    VidOsdParam.s_TextDisplay[0].s_DisplayY       = SysShowCfgPtr->s_ChannelInfo.s_DisplayY;
    VidOsdParam.s_TextDisplay[0].s_TextContentLen = strlen(SysShowCfgPtr->s_ChannelInfo.s_ChannelName);
    memcpy(VidOsdParam.s_TextDisplay[0].s_TextContent, SysShowCfgPtr->s_ChannelInfo.s_ChannelName, sizeof(SysShowCfgPtr->s_ChannelInfo.s_ChannelName));

    pthread_rwlock_wrlock(&m_Lock);
    GMI_RESULT Result = m_StreamCenterClientPtr->SetOsdConfiguration((m_VideoCodecHandle.GetPtr())[StreamId], &VidOsdParam);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SetOsdConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetOsdConfiguration fail, Result = 0x%lx\n", Result);
        return Result;
    }

    memcpy(&(m_VideoOsdParamPtr.GetPtr()[StreamId]), &VidOsdParam, sizeof(VideoOSDParam));
    Result = m_ConfigFileManagerPtr->SetOsdSettings(StreamId, &VidOsdParam);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SetOsdSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetOsdSettings fail, Result = 0x%lx\n", Result);
        return Result;
    }
    pthread_rwlock_unlock(&m_Lock);

    const char_t*  UserData = "set osd config successfully";
    USER_LOG(g_DefaultLogClient, SYS_LOGMAJOR_OPERATION, SYS_LOGMINOR_REMOTE_CFG_PARM, USER_NAME, strlen(USER_NAME), UserData, strlen(UserData));
    
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetVideoSourceImageSettings(int32_t SourceId, SysPkgImaging *SysPkgImagingPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (NULL == SysPkgImagingPtr
            || 1 != SourceId)
    {
        SYS_ERROR("SysPkgImagingPtr is null or SourceId %d error\n", SourceId);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysPkgImagingPtr is null or SourceId %d error\n", SourceId);
        return GMI_INVALID_PARAMETER;
    }

    SysPkgImagingPtr->s_VideoId                     = 1;
    SysPkgImagingPtr->s_Brightness                  = FloatToInt((float_t)(m_ImageParamPtr.GetPtr()->s_Brightness + 255) / 2);
    SysPkgImagingPtr->s_Contrast                    = m_ImageParamPtr.GetPtr()->s_Contrast;
    SysPkgImagingPtr->s_Saturation                  = m_ImageParamPtr.GetPtr()->s_Saturation;
    SysPkgImagingPtr->s_Hue                         = FloatToInt((float_t)((m_ImageParamPtr.GetPtr()->s_Hue + 15) * 255) / 30);
    SysPkgImagingPtr->s_Sharpness                   = m_ImageParamPtr.GetPtr()->s_Sharpness;
    SysPkgImagingPtr->s_Exposure.s_ExposureMode     = m_ImageParamPtr.GetPtr()->s_ExposureMode;
    SysPkgImagingPtr->s_Exposure.s_ShutterMax       = m_ImageParamPtr.GetPtr()->s_ExposureValueMax;
    SysPkgImagingPtr->s_Exposure.s_ShutterMin       = m_ImageParamPtr.GetPtr()->s_ExposureValueMin;
    SysPkgImagingPtr->s_Exposure.s_GainMax          = m_ImageParamPtr.GetPtr()->s_GainMax;
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


int32_t SystemServiceManager::FloatToInt(float_t Value)
{
    if (Value > 0.0)
    {
        return (int32_t)(Value + 0.5);
    }
    else
    {
        return (int32_t)(Value - 0.5);
    }
}


GMI_RESULT SystemServiceManager::SvrSetVideoSourceImageSettings(int32_t SourceId, SysPkgImaging *SysPkgImagingPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (NULL == SysPkgImagingPtr
            || 1 != SourceId)
    {
        SYS_ERROR("SysPkgImagingPtr is null or SourceId %d error\n", SourceId);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysPkgImagingPtr is null or SourceId %d error\n", SourceId);
        return GMI_INVALID_PARAMETER;
    }

    ImageBaseParam Imaging;

    SYS_INFO("SysPkgImagingPtr->s_Brightness %d\n", SysPkgImagingPtr->s_Brightness);
    SYS_INFO("SysPkgImagingPtr->s_Contrast   %d\n", SysPkgImagingPtr->s_Contrast);
    SYS_INFO("SysPkgImagingPtr->s_Hue        %d\n", SysPkgImagingPtr->s_Hue);
    SYS_INFO("SysPkgImagingPtr->s_Saturation %d\n", SysPkgImagingPtr->s_Saturation);
    SYS_INFO("SysPkgImagingPtr->s_Sharpness  %d\n", SysPkgImagingPtr->s_Sharpness);
    SYS_INFO("SysPkgImagingPtr->s_Exposure.s_ExposureMode     %d\n", SysPkgImagingPtr->s_Exposure.s_ExposureMode);
    SYS_INFO("SysPkgImagingPtr->s_Exposure.s_ShutterMax       %d\n", SysPkgImagingPtr->s_Exposure.s_ShutterMax);
    SYS_INFO("SysPkgImagingPtr->s_Exposure.s_ShutterMin       %d\n", SysPkgImagingPtr->s_Exposure.s_ShutterMin);
    SYS_INFO("SysPkgImagingPtr->s_Exposure.s_GainMax          %d\n", SysPkgImagingPtr->s_Exposure.s_GainMax);

    memset(&Imaging, 0, sizeof(ImageBaseParam));
    Imaging.s_Brightness       = FloatToInt((float_t)(SysPkgImagingPtr->s_Brightness * 2)) - 255;
    Imaging.s_Contrast         = SysPkgImagingPtr->s_Contrast;
    Imaging.s_Hue              = FloatToInt((float_t)(SysPkgImagingPtr->s_Hue * 30) / 255) - 15;
    Imaging.s_Saturation       = SysPkgImagingPtr->s_Saturation;
    Imaging.s_Sharpness        = SysPkgImagingPtr->s_Sharpness;
    Imaging.s_ExposureMode     = SysPkgImagingPtr->s_Exposure.s_ExposureMode;
    Imaging.s_ExposureValueMax = SysPkgImagingPtr->s_Exposure.s_ShutterMax;
    Imaging.s_ExposureValueMin = SysPkgImagingPtr->s_Exposure.s_ShutterMin;
    Imaging.s_GainMax          = SysPkgImagingPtr->s_Exposure.s_GainMax;

    pthread_rwlock_wrlock(&m_Lock);
    GMI_RESULT Result = m_StreamCenterClientPtr->SetImageConfiguration(m_ImageHandle, &Imaging);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SetImageConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetImageConfiguration fail, Result = 0x%lx\n", Result);
        return Result;
    }
    memcpy(m_ImageParamPtr.GetPtr(), &Imaging, sizeof(ImageBaseParam));
    Result = m_ConfigFileManagerPtr->SetImageSettings(0, 0, &Imaging);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SetImageSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetImageSettings fail, Result = 0x%lx\n", Result);
        return Result;
    }
    pthread_rwlock_unlock(&m_Lock);
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetAdvancedImageSettings(int32_t SourceId, SysPkgAdvancedImaging *SysAdvancedImagingPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (NULL == SysAdvancedImagingPtr
            || 1 != SourceId)
    {
        SYS_ERROR("SysAdvancedImagingPtr is null or SourceId %d error\n", SourceId);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysAdvancedImagingPtr is null or SourceId %d error\n", SourceId);
        return GMI_INVALID_PARAMETER;
    }

    SysAdvancedImagingPtr->s_VideoId           = 1;
    SysAdvancedImagingPtr->s_MeteringMode      = m_ImageAdvanceParamPtr.GetPtr()->s_MeteringMode;
    SysAdvancedImagingPtr->s_BackLightCompFlag = m_ImageAdvanceParamPtr.GetPtr()->s_BackLightCompFlag;
    SysAdvancedImagingPtr->s_DcIrisFlag        = m_ImageAdvanceParamPtr.GetPtr()->s_DcIrisFlag;
    SysAdvancedImagingPtr->s_MctfStrength      = m_ImageAdvanceParamPtr.GetPtr()->s_MctfStrength;
    SysAdvancedImagingPtr->s_DcIrisDuty        = m_ImageAdvanceParamPtr.GetPtr()->s_DcIrisDuty;
    SysAdvancedImagingPtr->s_AeTargetRatio     = m_ImageAdvanceParamPtr.GetPtr()->s_AeTargetRatio;
    switch (m_ImageAdvanceParamPtr.GetPtr()->s_LocalExposure)
    {
    case 0:
    case 1:
        SysAdvancedImagingPtr->s_LocalExposure = m_ImageAdvanceParamPtr.GetPtr()->s_LocalExposure;
        break;
    case 64:
        SysAdvancedImagingPtr->s_LocalExposure = 2;
        break;
    case 128:
        SysAdvancedImagingPtr->s_LocalExposure = 3;
        break;
    case 192:
        SysAdvancedImagingPtr->s_LocalExposure = 4;
        break;
    case 256:
        SysAdvancedImagingPtr->s_LocalExposure = 5;
        break;
    default:
        SYS_ERROR("m_ImageAdvanceParamPtr.GetPtr()->s_LocalExposure %d error\n", m_ImageAdvanceParamPtr.GetPtr()->s_LocalExposure);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ImageAdvanceParamPtr.GetPtr()->s_LocalExposure %d error\n", m_ImageAdvanceParamPtr.GetPtr()->s_LocalExposure);
        return GMI_FAIL;
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetAdvancedImagingSettings(int32_t SourceId, SysPkgAdvancedImaging *SysAdvancedImagingPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    ImageAdvanceParam Imaging;

    if (NULL == SysAdvancedImagingPtr
            || 1 != SourceId)
    {
        SYS_ERROR("SysAdvancedImagingPtr is null or SourceId %d error\n", SourceId);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysAdvancedImagingPtr is null or SourceId %d error\n", SourceId);
        return GMI_INVALID_PARAMETER;
    }

    memset(&Imaging, 0, sizeof(ImageAdvanceParam));
    Imaging.s_MeteringMode       = SysAdvancedImagingPtr->s_MeteringMode;
    Imaging.s_BackLightCompFlag  = SysAdvancedImagingPtr->s_BackLightCompFlag;
    Imaging.s_DcIrisFlag         = SysAdvancedImagingPtr->s_DcIrisFlag;
    Imaging.s_DcIrisDuty         = SysAdvancedImagingPtr->s_DcIrisDuty;
    Imaging.s_MctfStrength       = SysAdvancedImagingPtr->s_MctfStrength;
    Imaging.s_AeTargetRatio      = SysAdvancedImagingPtr->s_AeTargetRatio;
    switch (SysAdvancedImagingPtr->s_LocalExposure)
    {
    case 0:
    case 1:
        Imaging.s_LocalExposure = SysAdvancedImagingPtr->s_LocalExposure;
        break;
    case 2:
        Imaging.s_LocalExposure = 64;
        break;
    case 3:
        Imaging.s_LocalExposure = 64 * 2;
        break;
    case 4:
        Imaging.s_LocalExposure = 64 * 3;
        break;
    case 5:
        Imaging.s_LocalExposure = 64 * 4;
        break;
    default:
        SYS_ERROR("SysAdvancedImagingPtr->s_LocalExposure %d error\n", SysAdvancedImagingPtr->s_LocalExposure);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysAdvancedImagingPtr->s_LocalExposure %d error\n", SysAdvancedImagingPtr->s_LocalExposure);
        return GMI_INVALID_PARAMETER;
    }

    pthread_rwlock_wrlock(&m_Lock);
    GMI_RESULT Result = m_StreamCenterClientPtr->SetImageAdvanceConfiguration(m_ImageHandle, &Imaging);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SetImageAdvanceConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetImageAdvanceConfiguration fail, Result = 0x%lx\n", Result);
        return Result;
    }
    memcpy(m_ImageAdvanceParamPtr.GetPtr(), &Imaging, sizeof(ImageAdvanceParam));
    Result = m_ConfigFileManagerPtr->SetImageAdvanceSettings(0, 0, &Imaging);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SetImageAdvanceSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetImageAdvanceSettings fail, Result = 0x%lx\n", Result);
        return Result;
    }
    pthread_rwlock_unlock(&m_Lock);

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetVideoSourceSettings(int32_t SourceId, SysPkgVideoSource *SysVidSourcePtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    memcpy(SysVidSourcePtr, m_VideoSourcePtr.GetPtr(), sizeof(SysPkgVideoSource));
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetVideoSourceSettings(int32_t SourceId, SysPkgVideoSource *SysVidSourcePtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    if (1 != SourceId
            || NULL == SysVidSourcePtr)
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysVidSourcePtr is null or SourceId %d incorrect\n", SourceId);
        SYS_ERROR("SysVidSourcePtr is null or SourceId %d incorrect\n", SourceId);
        return GMI_INVALID_PARAMETER;
    }

    pthread_rwlock_wrlock(&m_Lock);
    //stop sdk stream transfer
    GMI_RESULT Result = m_SdkStreamCtlPtr->Pause(3);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("PauseStreamTransfer fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PauseStreamTransfer fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_RtspStreamCtlPtr->Stop(3);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("stop rtsp fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "stop rtsp fail, Result = 0x%lx\n", Result);
        return Result;
    }

    //stop encode
    int32_t Id;
    for (Id = 0; Id < m_VideoStreamNum; Id++)
    {
        Result = m_StreamCenterClientPtr->Stop2((m_VideoCodecHandle.GetPtr())[Id]);
        if (GMI_WAIT_TIMEOUT == Result)//miss the error of "TIMEOUT"
        {
            SYS_ERROR("Stop video codec timeout, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Stop video codec timeout, Result = 0x%lx\n", Result);
        }
        else if (FAILED(Result))
        {
            pthread_rwlock_unlock(&m_Lock);
            SYS_ERROR("Stop video codec fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Stop video codec fail, Result = 0x%lx\n", Result);
            return Result;
        }
    }

    //config video source
    VideoInParam  Vin;
    VideoOutParam Vout;
    memset(&Vin, 0, sizeof(VideoInParam));
    memset(&Vout, 0, sizeof(VideoOutParam));
    Result = m_StreamCenterClientPtr->GetVinVoutConfiguration(m_VideoInOutHandle, &Vin, &Vout);
    if (GMI_WAIT_TIMEOUT == Result)//miss the error of "TIMEOUT"
    {
        SYS_ERROR("GetVinVoutConfiguration timeout, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "GetVinVoutConfiguration timeout, Result = 0x%lx\n", Result);
    }
    else if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("GetVinVoutConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetVinVoutConfiguration fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Vin.s_VinMirrorPattern = SysVidSourcePtr->s_Mirror;
    Vin.s_VinFrameRate     = SysVidSourcePtr->s_SrcFps;
    Result = m_StreamCenterClientPtr->SetVinVoutConfiguration(m_VideoInOutHandle, &Vin, &Vout);
    if (GMI_WAIT_TIMEOUT == Result)//miss the error of "TIMEOUT"
    {
        SYS_ERROR("SetVinVoutConfiguration timeout, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "SetVinVoutConfiguration timeout, Result = 0x%lx\n", Result);
    }
    else if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SetVinVoutConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetVinVoutConfiguration fail, Result = 0x%lx\n", Result);
        return Result;
    }

    if (SUCCEEDED(Result))
    {
        memcpy(m_VideoSourcePtr.GetPtr(), SysVidSourcePtr, sizeof(SysPkgVideoSource));
        Result = m_ConfigFileManagerPtr->SetVideoSourceSettings(SourceId, SysVidSourcePtr);
        if (FAILED(Result))//save file failed, just waring,should gurantee video transfer normally.
        {
            SYS_ERROR("Save System video source setting fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Save System video source setting fail, Result = 0x%lx\n", Result);
        }
    }

    //start encode
    for (Id = 0; Id < m_VideoStreamNum; Id++)
    {
        Result = m_StreamCenterClientPtr->Start2((m_VideoCodecHandle.GetPtr())[Id]);
        if (GMI_WAIT_TIMEOUT == Result)//miss the error of "TIMEOUT"
        {
            SYS_ERROR("Start video codec timeout, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Start video codec timeout, Result = 0x%lx\n", Result);
        }
        else if (FAILED(Result) && GMI_WAIT_TIMEOUT != Result)
        {
            pthread_rwlock_unlock(&m_Lock);
            SYS_ERROR("Start video codec fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Start video codec fail, Result = 0x%lx\n", Result);
            return Result;
        }
    }

    //start sdk stream transfer
    ReferrencePtr<SysPkgEncodeCfg, DefaultObjectsDeleter>SysEncCfgPtr(BaseMemoryManager::Instance().News<SysPkgEncodeCfg>(m_VideoStreamNum));
    if (NULL == SysEncCfgPtr.GetPtr())
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SysEncCfgPtr news fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysEncCfgPtr news fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SvrGetVideoEncodeSettings(0xff, SysEncCfgPtr.GetPtr());
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SvrGetVideoEncodeSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SvrGetVideoEncodeSettings fail, Result = 0x%lx\n", Result);
        return Result;
    }
    //allocate SysPkgAudioEncodeCfg
    ReferrencePtr<SysPkgAudioEncodeCfg>SysAudioEncodeCfgPtr(BaseMemoryManager::Instance().New<SysPkgAudioEncodeCfg>());
    if (NULL == SysAudioEncodeCfgPtr.GetPtr())
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("audio encode config fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "audio encode config fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    memset(SysAudioEncodeCfgPtr.GetPtr(), 0, sizeof(SysPkgAudioEncodeCfg));
    //get all audio encode parameter
    Result = SvrGetAudioEncodeSetting(1, SysAudioEncodeCfgPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("Svr get audio encode config fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "Svr get audio encode config fail, Result = 0x%lx\n", Result);
    }
    Result = m_SdkStreamCtlPtr->Resume(SysEncCfgPtr.GetPtr(), m_VideoStreamNum, SysAudioEncodeCfgPtr.GetPtr(), 1, 3);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("ResumeStreamTransfer fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ResumeStreamTransfer fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_RtspStreamCtlPtr->Start(SysEncCfgPtr.GetPtr(), m_VideoStreamNum, SysAudioEncodeCfgPtr.GetPtr(), 1, 3);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("start rtsp fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "start rtsp fail, Result = 0x%lx\n", Result);
        return Result;
    }

    pthread_rwlock_unlock(&m_Lock);
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetWhiteBalanceSettings(int32_t SourceId, SysPkgWhiteBalance *SysWhiteBalancePtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    if (NULL == SysWhiteBalancePtr)
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysWhiteBalancePtr is null\n");
        SYS_ERROR("SysWhiteBalancePtr is null\n");
        return GMI_INVALID_PARAMETER;
    }

    SysWhiteBalancePtr->s_Mode  = m_ImageWbParamPtr.GetPtr()->s_WbMode;
    SysWhiteBalancePtr->s_RGain = m_ImageWbParamPtr.GetPtr()->s_WbRgain;
    SysWhiteBalancePtr->s_BGain = m_ImageWbParamPtr.GetPtr()->s_WbBgain;

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetWhiteBalanceSettings(int32_t SourceId, SysPkgWhiteBalance *SysWhiteBalancePtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    if (NULL == SysWhiteBalancePtr)
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysWhiteBalancePtr is null\n");
        SYS_ERROR("SysWhiteBalancePtr is null\n");
        return GMI_INVALID_PARAMETER;
    }

    ImageWbParam ImgWbParam;
    memset(&ImgWbParam, 0, sizeof(ImageWbParam));
    ImgWbParam.s_WbMode  = SysWhiteBalancePtr->s_Mode;
    ImgWbParam.s_WbRgain = SysWhiteBalancePtr->s_RGain;
    ImgWbParam.s_WbBgain = SysWhiteBalancePtr->s_BGain;

    pthread_rwlock_wrlock(&m_Lock);
    GMI_RESULT Result = m_StreamCenterClientPtr->SetWhiteBalanceConfiguration(m_ImageHandle, &ImgWbParam);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SetWhiteBalanceConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetWhiteBalanceConfiguration fail, Result = 0x%lx\n", Result);
        return Result;
    }

    memcpy(m_ImageWbParamPtr.GetPtr(), &ImgWbParam, sizeof(SysPkgWhiteBalance));
    Result = m_ConfigFileManagerPtr->SetWhiteBalanceSettings(0, &ImgWbParam);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SetWhiteBalanceSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetWhiteBalanceSettings fail, Result = 0x%lx\n", Result);
        return Result;
    }

    pthread_rwlock_unlock(&m_Lock);
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetDaynightSettings(int32_t SourceId, SysPkgDaynight *SysDaynightPr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    if (NULL == SysDaynightPr)
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysDaynightPr is null\n");
        SYS_ERROR("SysDaynightPr is null\n");
        return GMI_INVALID_PARAMETER;
    }

    SysDaynightPr->s_Mode          = m_ImageDnParamPtr.GetPtr()->s_DnMode;
    SysDaynightPr->s_DurationTime  = m_ImageDnParamPtr.GetPtr()->s_DnDurtime;
    SysDaynightPr->s_NightToDayThr = m_ImageDnParamPtr.GetPtr()->s_NightToDayThr;
    SysDaynightPr->s_DayToNightThr = m_ImageDnParamPtr.GetPtr()->s_DayToNightThr;
    for (int32_t i = 0; i < SCHEDULE_WEEK_DAYS; i++)
    {
        SysDaynightPr->s_SchedEnable[i]    = m_ImageDnParamPtr.GetPtr()->s_DnSchedule.s_DnSchedFlag[i];
        SysDaynightPr->s_SchedStartTime[i] = m_ImageDnParamPtr.GetPtr()->s_DnSchedule.s_StartTime[i];
        SysDaynightPr->s_SchedEndTime[i]   = m_ImageDnParamPtr.GetPtr()->s_DnSchedule.s_EndTime[i];
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetDaynightSettings(int32_t SourceId, SysPkgDaynight *SysDaynightPr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    if (NULL == SysDaynightPr)
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysDaynightPr is null\n");
        SYS_ERROR("SysDaynightPr is null\n");
        return GMI_INVALID_PARAMETER;
    }

    ImageDnParam ImgDnParam;
    memset(&ImgDnParam, 0, sizeof(ImageDnParam));
    ImgDnParam.s_DnMode          = SysDaynightPr->s_Mode;
    ImgDnParam.s_DnDurtime       = SysDaynightPr->s_DurationTime;
    ImgDnParam.s_NightToDayThr   = SysDaynightPr->s_NightToDayThr;
    ImgDnParam.s_DayToNightThr   = SysDaynightPr->s_DayToNightThr;
    for (int32_t i = 0; i < SCHEDULE_WEEK_DAYS; i++)
    {
        ImgDnParam.s_DnSchedule.s_DnSchedFlag[i] = SysDaynightPr->s_SchedEnable[i];
        ImgDnParam.s_DnSchedule.s_StartTime[i]   = SysDaynightPr->s_SchedStartTime[i];
        ImgDnParam.s_DnSchedule.s_EndTime[i]     = SysDaynightPr->s_SchedEndTime[i];
    }

    pthread_rwlock_wrlock(&m_Lock);
    GMI_RESULT Result = m_StreamCenterClientPtr->SetDaynightConfiguration(m_ImageHandle, &ImgDnParam);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SetDaynightConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetDaynightConfiguration fail, Result = 0x%lx\n", Result);
        return Result;
    }

    memcpy(m_ImageDnParamPtr.GetPtr(), &ImgDnParam, sizeof(ImageDnParam));
    Result = m_ConfigFileManagerPtr->SetDaynightSettings(0, &ImgDnParam);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SetDaynightSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetDaynightSettings fail, Result = 0x%lx\n", Result);
        return Result;
    }

    pthread_rwlock_unlock(&m_Lock);
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::FactoryResetImaging(void)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    SysPkgImaging SysImaging;
    memset(&SysImaging, 0, sizeof(SysPkgImaging));
    GMI_RESULT Result = m_ConfigFileManagerPtr->GetImageSettingsDefault(0, 0, &SysImaging);
    if (FAILED(Result))
    {
        SYS_ERROR("get image default setting fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "get image default setting fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = SvrSetVideoSourceImageSettings(1, &SysImaging);
    if (FAILED(Result))
    {
        SYS_ERROR("SvrSetVideoSourceImageSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SvrSetVideoSourceImageSettings fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SysPkgAdvancedImaging SysAdvancedImaging;
    memset(&SysAdvancedImaging, 0, sizeof(SysPkgAdvancedImaging));
    Result = m_ConfigFileManagerPtr->GetImageAdvanceSettingsDefault(0, 0, &SysAdvancedImaging);
    if (FAILED(Result))
    {
        SYS_ERROR("get advanced image default setting fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "get advanced image default setting fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = SvrSetAdvancedImagingSettings(1, &SysAdvancedImaging);
    if (FAILED(Result))
    {
        SYS_ERROR("SvrSetAdvancedImagingSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SvrSetAdvancedImagingSettings fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SysPkgWhiteBalance SysWhiteBalance;
    memset(&SysWhiteBalance, 0, sizeof(SysPkgWhiteBalance));
    SysWhiteBalance.s_Mode  = SYS_ENV_DEFAULT_WB_MODE;
    SysWhiteBalance.s_BGain = SYS_ENV_DEFAULT_WB_BGAIN;
    SysWhiteBalance.s_RGain = SYS_ENV_DEFAULT_WB_RGAIN;
    Result = SvrSetWhiteBalanceSettings(0, &SysWhiteBalance);
    if (FAILED(Result))
    {
        SYS_ERROR("SvrSetVideoSourceImageSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SvrSetVideoSourceImageSettings fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SysPkgDaynight SysDaynight;
    memset(&SysDaynight, 0, sizeof(SysPkgDaynight));
    Result = m_ConfigFileManagerPtr->GetDaynightSettingsDefault(1, &SysDaynight);
    if (FAILED(Result))
    {
        SYS_ERROR("get day night default setting fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "get day night default setting fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = SvrSetDaynightSettings(1, &SysDaynight);
    if (FAILED(Result))
    {
        SYS_ERROR("SvrSetDaynightSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SvrSetDaynightSettings fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SysPkgVideoSource SysVideoSource;
    memset(&SysVideoSource, 0, sizeof(SysPkgVideoSource));
    Result = SvrGetVideoSourceSettings(1, &SysVideoSource);
    if (FAILED(Result))
    {
        SYS_ERROR("SvrGetVideoSourceSettings fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SvrGetVideoSourceSettings fail, Result = 0x%lx\n", Result);
        return Result;
    }
    if (SYS_ENV_DEFAULT_SOURCE_MIRROR != SysVideoSource.s_Mirror)
    {
        SysVideoSource.s_Mirror = SYS_ENV_DEFAULT_SOURCE_MIRROR;
        Result = SvrSetVideoSourceSettings(1, &SysVideoSource);
        if (FAILED(Result))
        {
            SYS_ERROR("SvrSetVideoSourceSettings fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SvrSetVideoSourceSettings fail, Result = 0x%lx\n", Result);
            return Result;
        }
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::FactoryResetStreamCombine()
{
    SysPkgEncStreamCombine SysEncStreamCombine;
    GMI_RESULT Result = m_ConfigFileManagerPtr->GetSysEncStreamCombineDefault(&SysEncStreamCombine);
    if (FAILED(Result))
    {
        SYS_ERROR("get stream combine default fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "get stream combine default fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = SvrSetVideoEncStreamCombine(&SysEncStreamCombine);
    if (FAILED(Result))
    {
        SYS_ERROR("set stream combine fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "set stream combine fail, Result = 0x%lx\n", Result);
        return Result;
    }
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::FactoryResetVideoEncode()
{
    SysPkgEncodeCfg SysEncodeCfg[MAX_VIDEO_STREAM_NUM];
    memset(SysEncodeCfg, 0, sizeof(SysEncodeCfg));

    SysPkgEncStreamCombine SysEncStreamCombine;
    GMI_RESULT Result = m_ConfigFileManagerPtr->GetStreamCombine(&SysEncStreamCombine);
    if (FAILED(Result))
    {
        SYS_ERROR("get stream combine fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_ConfigFileManagerPtr->GetVideoEncodeSettingsDefault(0xff, &SysEncStreamCombine, SysEncodeCfg);
    if (FAILED(Result))
    {
        SYS_ERROR("get video encode default setting fail, Result = 0x%lx\n", Result);
        return Result;
    }

    for (int32_t Id = 0; Id < SysEncStreamCombine.s_EnableStreamNum; Id++)
    {
        Result = SvrSetVideoEncodeSetting(Id, &SysEncodeCfg[Id]);
        if (FAILED(Result))
        {
            SYS_ERROR("service set video encode setting fail, Result = 0x%lx\n", Result);
            return Result;
        }
    }

    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrPtzControl(SysPkgPtzCtrl *PtzCtrl )
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    uint32_t Time1 = TimeStamp();

    if (m_SupportPtz)
    {
        GMI_RESULT     Result;
        static int32_t LastPtzCmd;
        SysPkgPtzCtrl  PtzCtrlTmp;
        PtzCtlCmd      PT_CtlCmd;

        if (PtzCtrl == NULL)
        {
            return GMI_INVALID_PARAMETER;
        }

        memcpy(&PtzCtrlTmp, PtzCtrl, sizeof(SysPkgPtzCtrl));
        memset(&PT_CtlCmd, 0, sizeof(PtzCtlCmd));

        // SYS_INFO("Time %u, Cmd %d, Param[0] %d, Param[1] %d, Param[2] %d, Param[3] %d\n",
        //          Time1, PtzCtrlTmp.s_PtzCmd, PtzCtrlTmp.s_Param[0], PtzCtrlTmp.s_Param[1], PtzCtrlTmp.s_Param[2], PtzCtrlTmp.s_Param[3]);
        //DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Time %u, Cmd %d, Param[0] %d, Param[1] %d, Param[2] %d, Param[3] %d\n", 
        //          Time1, PtzCtrlTmp.s_PtzCmd, PtzCtrlTmp.s_Param[0], PtzCtrlTmp.s_Param[1], PtzCtrlTmp.s_Param[2], PtzCtrlTmp.s_Param[3]);
        USER_LOG(g_DefaultLogClient, SYS_LOGMAJOR_OPERATION, SYS_LOGMINOR_REMOTE_PTZCTRL, USER_NAME, strlen(USER_NAME), "ptz control", strlen("ptz control"));

        if (SYS_PTZCMD_LEFT        == PtzCtrlTmp.s_PtzCmd
                || SYS_PTZCMD_RIGHT     == PtzCtrlTmp.s_PtzCmd
                || SYS_PTZCMD_UP        == PtzCtrlTmp.s_PtzCmd
                || SYS_PTZCMD_DOWN      == PtzCtrlTmp.s_PtzCmd
                || SYS_PTZCMD_LEFTUP    == PtzCtrlTmp.s_PtzCmd
                || SYS_PTZCMD_LEFTDOWN  == PtzCtrlTmp.s_PtzCmd
                || SYS_PTZCMD_RIGHTUP   == PtzCtrlTmp.s_PtzCmd
                || SYS_PTZCMD_RIGHTDOWN == PtzCtrlTmp.s_PtzCmd)
        {
            switch (PtzCtrlTmp.s_PtzCmd)
            {
            case SYS_PTZCMD_LEFT:
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_Left;
                PT_CtlCmd.s_CmdParam[0] = FloatToInt((float_t)PtzCtrlTmp.s_Param[0]*100/255);
                break;
            case SYS_PTZCMD_RIGHT:
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_Right;
                PT_CtlCmd.s_CmdParam[0] = FloatToInt((float_t)PtzCtrlTmp.s_Param[0]*100/255);
                break;
            case SYS_PTZCMD_UP:
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_Up;
                PT_CtlCmd.s_CmdParam[0] = FloatToInt((float_t)PtzCtrlTmp.s_Param[0]*100/255);
                break;
            case SYS_PTZCMD_DOWN:
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_Down;
                PT_CtlCmd.s_CmdParam[0] = FloatToInt((float_t)PtzCtrlTmp.s_Param[0]*100/255);
                break;
            case SYS_PTZCMD_LEFTUP:
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_LeftUp;
                PT_CtlCmd.s_CmdParam[0] = FloatToInt((float_t)PtzCtrlTmp.s_Param[0]*100/255);
                PT_CtlCmd.s_CmdParam[1] = FloatToInt((float_t)PtzCtrlTmp.s_Param[0]*100/255);
                break;
            case SYS_PTZCMD_LEFTDOWN:
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_LeftDown;
                PT_CtlCmd.s_CmdParam[0] = FloatToInt((float_t)PtzCtrlTmp.s_Param[0]*100/255);
                PT_CtlCmd.s_CmdParam[1] = FloatToInt((float_t)PtzCtrlTmp.s_Param[0]*100/255);
                break;
            case SYS_PTZCMD_RIGHTUP:
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_RightUp;
                PT_CtlCmd.s_CmdParam[0] = FloatToInt((float_t)PtzCtrlTmp.s_Param[0]*100/255);
                PT_CtlCmd.s_CmdParam[1] = FloatToInt((float_t)PtzCtrlTmp.s_Param[0]*100/255);
                break;
            case SYS_PTZCMD_RIGHTDOWN:
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_RightDown;
                PT_CtlCmd.s_CmdParam[0] = FloatToInt((float_t)PtzCtrlTmp.s_Param[0]*100/255);
                PT_CtlCmd.s_CmdParam[1] = FloatToInt((float_t)PtzCtrlTmp.s_Param[0]*100/255);
                break;
            }

            Result = m_StreamCenterClientPtr->PauseAutoFocus(m_AutoFocusHandle, true);
            if (FAILED(Result))
            {
                SYS_ERROR("PauseAutoFocus fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PauseAutoFocus fail, Result = 0x%lx\n", Result);
                return Result;
            }

            Result = m_PtzControlPtr->Control(PTZ_CONTINUE_CONTROL_MODE, &PT_CtlCmd);
            if (FAILED(Result))
            {
                SYS_ERROR("PTZ Control fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PTZ Control fail, Result = 0x%lx\n", Result);
                return Result;
            }
        }
        else if (SYS_PTZCMD_ZOOM_TELE == PtzCtrlTmp.s_PtzCmd
                 || SYS_PTZCMD_ZOOM_WIDE == PtzCtrlTmp.s_PtzCmd)
        {
            ZoomCmd    Z_CtlCmd;

            memset(&Z_CtlCmd, 0, sizeof(ZoomCmd));
            Z_CtlCmd.s_ZoomMode = ((SYS_PTZCMD_ZOOM_TELE == PtzCtrlTmp.s_PtzCmd) ? ZOOM_MODE_IN : ZOOM_MODE_OUT);
            Result = m_StreamCenterClientPtr->PauseAutoFocus(m_AutoFocusHandle, true);
            if (FAILED(Result))
            {
                SYS_ERROR("PauseAutoFocus fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PauseAutoFocus fail, Result = 0x%lx\n", Result);
                return Result;
            }

            Result = m_StreamCenterClientPtr->ControlZoom(m_ZoomHandle, Z_CtlCmd.s_ZoomMode);
            if (FAILED(Result))
            {
                SYS_ERROR("ControlZoom fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ControlZoom fail, Result = 0x%lx\n", Result);
                return Result;
            }
        }
        else if (SYS_PTZCMD_GMI_MANUAL_FOCUS == PtzCtrlTmp.s_PtzCmd)
        {
            Result = m_StreamCenterClientPtr->AutoFocusGlobalScan(m_AutoFocusHandle);
            if (FAILED(Result))
            {
                SYS_ERROR("AutoFocusGlobalScan fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "AutoFocusGlobalScan fail, Result = 0x%lx\n", Result);
                return Result;
            }
        }
        else if (SYS_PTZCMD_FOCUS_FAR == PtzCtrlTmp.s_PtzCmd
                 || SYS_PTZCMD_FOCUS_NEAR == PtzCtrlTmp.s_PtzCmd)
        {
            if (AF_MODE_MANUAL == m_FocusMode)
            {
                int32_t Mode = SYS_PTZCMD_FOCUS_FAR == PtzCtrlTmp.s_PtzCmd ? AF_DIR_MODE_OUT : AF_DIR_MODE_IN;

                Result = m_StreamCenterClientPtr->ControlAutoFocus(m_AutoFocusHandle, Mode);
                if (FAILED(Result))
                {
                    SYS_ERROR("ControlAutoFocus fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ControlAutoFocus fail, Result = 0x%lx\n", Result);
                    return Result;
                }
            }
            else
            {
                SYS_INFO("not support focus near or far on focusmode %d\n", m_FocusMode);
            }
        }
        else if (SYS_PTZCMD_SETPRESET == PtzCtrlTmp.s_PtzCmd)
        {
        }
        else if (SYS_PTZCMD_GOTOPRESET == PtzCtrlTmp.s_PtzCmd)
        {
            PT_CtlCmd.s_Cmd = e_PTZ_CMD_GotoPreset;
            PT_CtlCmd.s_CmdParam[0] = PtzCtrlTmp.s_Param[0];
            if (1 > PT_CtlCmd.s_CmdParam[0]
                    || 256 < PT_CtlCmd.s_CmdParam[0] )
            {
                return GMI_INVALID_PARAMETER;
            }

            int32_t i;
            int32_t ZoomPos = 0;
            uint32_t Index = PT_CtlCmd.s_CmdParam[0];
            for (i = 0; i < MAX_PRESETS; i++)
            {
                if ((m_PresetsInfo_InnerPtr.GetPtr())[i].s_Index == Index)
                {
                    ZoomPos = (m_PresetsInfo_InnerPtr.GetPtr())[i].s_ZoomPosition;
                    break;
                }
            }

            if ((m_PresetsInfo_InnerPtr.GetPtr())[i].s_Setted)
            {
                //Result = m_StreamCenterClientPtr->PauseAutoFocus(m_AutoFocusHandle, true);
                //if (FAILED(Result))
                //{
                //    SYS_ERROR("PauseAutoFocus fail, Result = 0x%lx\n", Result);
                //    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PauseAutoFocus fail, Result = 0x%lx\n", Result);
                //    return Result;
                //}

                Result = m_PtzControlPtr->Control(PTZ_CONTINUE_CONTROL_MODE, &PT_CtlCmd);
                if (FAILED(Result))
                {
                    SYS_ERROR("PTZ Control fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PTZ Control fail, Result = 0x%lx\n", Result);
                    return Result;
                }

                Result = m_StreamCenterClientPtr->SetZoomPosition(m_ZoomHandle, ZoomPos);
                if (FAILED(Result))
                {
                    SYS_ERROR("SetZoomPosition fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetZoomPosition fail, Result = 0x%lx\n", Result);
                    return Result;
                }

                //Result = m_StreamCenterClientPtr->PauseAutoFocus(m_AutoFocusHandle, false);
                //if (FAILED(Result))
                //{
                //    SYS_ERROR("PauseAutoFocus fail, Result = 0x%lx\n", Result);
                //    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PauseAutoFocus fail, Result = 0x%lx\n", Result);
                //    return Result;
                //}
            }
        }
        else if (SYS_PTZCMD_CLEARPRESET == PtzCtrlTmp.s_PtzCmd)
        {
            PtzCtlCmd  PT_CtlCmd;

            memset(&PT_CtlCmd, 0, sizeof(PtzCtlCmd));

            PT_CtlCmd.s_Cmd = e_PTZ_CMD_RmPreset;
            PT_CtlCmd.s_CmdParam[0] = PtzCtrlTmp.s_Param[0];
            if (1 > PT_CtlCmd.s_CmdParam[0]
                    || 256 < PT_CtlCmd.s_CmdParam[0] )
            {
                return GMI_INVALID_PARAMETER;
            }

            Result = m_PtzControlPtr->Control(PTZ_CONTINUE_CONTROL_MODE, &PT_CtlCmd);
            if (FAILED(Result))
            {
                SYS_ERROR("PTZ Control fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PTZ Control fail, Result = 0x%lx\n", Result);
                return Result;
            }

            int32_t  i;
            uint32_t Index = PT_CtlCmd.s_CmdParam[0];
            for (i = 0; i < MAX_PRESETS; i++)
            {
                if ((m_PresetsInfo_InnerPtr.GetPtr())[i].s_Index == Index)
                {
                    (m_PresetsInfo_InnerPtr.GetPtr())[i].s_Setted = false;
                    break;
                }
            }
            Result = m_ConfigFileManagerPtr->SetPresetsInfo(&((m_PresetsInfo_InnerPtr.GetPtr())[i]));
            if (FAILED(Result))
            {
                SYS_ERROR("m_ConfigFileManagerPtr SetPresetsInfo fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ConfigFileManagerPtr SetPresetsInfo fail, Result = 0x%lx\n", Result);
                return Result;
            }
        }
        else if (SYS_PTZCMD_STOP == PtzCtrlTmp.s_PtzCmd)
        {
            if (SYS_PTZCMD_LEFT        == LastPtzCmd
                    || SYS_PTZCMD_RIGHT     == LastPtzCmd
                    || SYS_PTZCMD_UP        == LastPtzCmd
                    || SYS_PTZCMD_DOWN      == LastPtzCmd
                    || SYS_PTZCMD_LEFTUP    == LastPtzCmd
                    || SYS_PTZCMD_LEFTDOWN  == LastPtzCmd
                    || SYS_PTZCMD_RIGHTUP   == LastPtzCmd
                    || SYS_PTZCMD_RIGHTDOWN == LastPtzCmd)
            {
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_Stop;
                Result = m_PtzControlPtr->Control(PTZ_CONTINUE_CONTROL_MODE, &PT_CtlCmd);
                if (FAILED(Result))
                {
                    SYS_ERROR("PTZ Control fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PTZ Control fail, Result = 0x%lx\n", Result);
                    return Result;
                }                
            }
            else if (SYS_PTZCMD_ZOOM_TELE == LastPtzCmd
                     || SYS_PTZCMD_ZOOM_WIDE == LastPtzCmd)
            {
                Result = m_StreamCenterClientPtr->ControlZoom(m_ZoomHandle, ZOOM_MODE_STOP);
                if (FAILED(Result))
                {
                    SYS_ERROR("ControlZoom fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ControlZoom fail, Result = 0x%lx\n", Result);
                    return Result;
                }

                //signal recordzoompos thread to save current zoom position. 11/7/2013 guoqiang.lu
                m_RecordZoomNotify.Signal();
            }
            else
            {
                if (AF_MODE_MANUAL == m_FocusMode)//on manual focus mode , donot need resume autofocus task
                {
                    Result = m_StreamCenterClientPtr->ControlAutoFocus(m_AutoFocusHandle, AF_DIR_MODE_STOP);
                    if (FAILED(Result))
                    {
                        SYS_ERROR("ControlAutoFocus fail, Result = 0x%lx\n", Result);
                        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ControlAutoFocus fail, Result = 0x%lx\n", Result);
                        return Result;
                    }
                }               
            }

			Result = m_StreamCenterClientPtr->PauseAutoFocus(m_AutoFocusHandle, false);
			if (FAILED(Result))
			{
			    SYS_ERROR("PauseAutoFocus fail, Result = 0x%lx\n", Result);
			    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PauseAutoFocus fail, Result = 0x%lx\n", Result);
			    return Result;
			}
        }
        else
        {
            SYS_ERROR("not support ptz cmd %d\n", PtzCtrlTmp.s_PtzCmd);
            return GMI_NOT_SUPPORT;
        }
        LastPtzCmd = PtzCtrlTmp.s_PtzCmd;
    }

    SYS_INFO("%s waste Time %u, normal out..........\n", __func__, TimeStamp() - Time1);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s waste Time %u, normal out..........\n", __func__, TimeStamp() - Time1);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetAutoFocus(SysPkgAutoFocus *SysAutoFocusPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (!m_SupportPtz)
    {
        SYS_INFO("not support ptz\n");
        return GMI_NOT_SUPPORT;
    }

    SysAutoFocusPtr->s_FocusMode = m_FocusMode;

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetAutoFocus(SysPkgAutoFocus *SysAutoFocusPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    if (!m_SupportPtz)
    {
        SYS_INFO("not support ptz\n");
        return GMI_NOT_SUPPORT;
    }

    if (AF_MODE_MANUAL != SysAutoFocusPtr->s_FocusMode
            && AF_MODE_AUTO != SysAutoFocusPtr->s_FocusMode
            && AF_MODE_ONCEAUTO != SysAutoFocusPtr->s_FocusMode)
    {
        SYS_ERROR("SysAutoFocusPtr->s_FocusMode %d incorrect\n", SysAutoFocusPtr->s_FocusMode);
        return GMI_INVALID_PARAMETER;
    }

    SYS_INFO("==>SysAutoFocusPtr->s_FocusMode %d\n", SysAutoFocusPtr->s_FocusMode);
    pthread_rwlock_rdlock(&m_Lock);
    GMI_RESULT Result = m_StreamCenterClientPtr->SetAutoFocusMode(m_AutoFocusHandle, SysAutoFocusPtr->s_FocusMode);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("m_StreamCenterClientPtr SetAutoFocusMode fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_StreamCenterClientPtr SetAutoFocusMode fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_ConfigFileManagerPtr->SetAutoFocusMode(SysAutoFocusPtr->s_FocusMode);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("m_ConfigFileManagerPtr SetAutoFocusMode fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ConfigFileManagerPtr SetAutoFocusMode fail, Result = 0x%lx\n", Result);
        return Result;
    }

    m_FocusMode = SysAutoFocusPtr->s_FocusMode;
    pthread_rwlock_unlock(&m_Lock);

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetPresetInfo(int32_t *PresetCnt, SysPkgPtzPresetInfo *SysPresetInfoPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    if (!m_SupportPtz)
    {
        SYS_ERROR("%s not support ptz\n", __func__);
        return GMI_NOT_SUPPORT;
    }

    for (uint32_t Id = 0; Id  < MAX_PRESETS; Id ++)
    {
        SysPresetInfoPtr[Id].s_PtzId = 1;
        SysPresetInfoPtr[Id].s_PresetIndex = (m_PresetsInfo_InnerPtr.GetPtr())[Id].s_Index & 0xffff;
        if ((m_PresetsInfo_InnerPtr.GetPtr())[Id].s_Setted)
        {
            SysPresetInfoPtr[Id].s_PresetIndex |= (1 << 16);
        }
        else
        {
            SysPresetInfoPtr[Id].s_PresetIndex &= 0xffff;
        }
        strcpy(SysPresetInfoPtr[Id].s_PresetName, (m_PresetsInfo_InnerPtr.GetPtr())[Id].s_Name);
        //SYS_INFO("preset[%d]=0x%x\n", Id, SysPresetInfoPtr[Id].s_PresetIndex);
    }

    *PresetCnt = MAX_PRESETS;
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetPresetInfo(int32_t Index, SysPkgPtzPresetInfo *SysPresetInfoPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    if (!m_SupportPtz)
    {
        SYS_ERROR("%s not support ptz\n", __func__);
        return GMI_NOT_SUPPORT;
    }

    uint32_t Id;
    for (Id = 0; Id  < MAX_PRESETS; Id ++)
    {
        if ((m_PresetsInfo_InnerPtr.GetPtr())[Id].s_Index == (uint32_t)Index)
        {
            SysPresetInfoPtr->s_PtzId = 1;
            if ((m_PresetsInfo_InnerPtr.GetPtr())[Id].s_Setted)
            {
                SysPresetInfoPtr->s_PresetIndex = ((1 << 16) | Index);
            }
            else
            {
                SysPresetInfoPtr->s_PresetIndex = (0xffff & Index);
            }
            strcpy(SysPresetInfoPtr->s_PresetName, (m_PresetsInfo_InnerPtr.GetPtr())[Id].s_Name);
            SYS_INFO("Index = 0x%x\n", SysPresetInfoPtr->s_PresetIndex);
            SYS_INFO("Name  = %s\n", SysPresetInfoPtr->s_PresetName);
            break;
        }
    }

    if (Id > MAX_PRESETS)
    {
        SYS_ERROR("index%d not find\n", Index);
        return GMI_NOT_SUPPORT;
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetPresetInfo(SysPkgPtzPresetInfo *SysPresetInfoPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    if (!m_SupportPtz)
    {
        return GMI_NOT_SUPPORT;
    }

    uint32_t Index = SysPresetInfoPtr->s_PresetIndex & 0xffff;
    if (0 > Index || 256 < Index )
    {
        return GMI_INVALID_PARAMETER;
    }

    PtzCtlCmd  PT_CtlCmd;
    PT_CtlCmd.s_Cmd = e_PTZ_CMD_SetPreset;
    PT_CtlCmd.s_CmdParam[0] = Index;

    GMI_RESULT Result = m_PtzControlPtr->Control(PTZ_CONTINUE_CONTROL_MODE, &PT_CtlCmd);
    if (FAILED(Result))
    {
        SYS_ERROR("PTZ Control fail, Result = 0x%lx\n", Result);
        return Result;
    }
    int32_t ZoomPos;
    Result = m_StreamCenterClientPtr->GetZoomPosition(m_ZoomHandle, &ZoomPos);
    if (FAILED(Result))
    {
        PT_CtlCmd.s_Cmd = e_PTZ_CMD_RmPreset;
        PT_CtlCmd.s_CmdParam[0] = Index;
        m_PtzControlPtr->Control(PTZ_CONTINUE_CONTROL_MODE, &PT_CtlCmd);
        SYS_ERROR("GetZoomPosition fail, Result = 0x%lx\n", Result);
        return Result;
    }

    //force name
    if (0 == strlen(SysPresetInfoPtr->s_PresetName))
    {
        sprintf(SysPresetInfoPtr->s_PresetName, "%d", Index);
    }

    //overwrite
    int32_t i;
    for (i = 0; i < MAX_PRESETS; i++)
    {
        if (0 == strcmp((m_PresetsInfo_InnerPtr.GetPtr())[i].s_Name, SysPresetInfoPtr->s_PresetName))
        {
            strcpy((m_PresetsInfo_InnerPtr.GetPtr())[i].s_Name, SysPresetInfoPtr->s_PresetName);
            (m_PresetsInfo_InnerPtr.GetPtr())[i].s_Index        = Index;
            (m_PresetsInfo_InnerPtr.GetPtr())[i].s_Setted       = true;
            (m_PresetsInfo_InnerPtr.GetPtr())[i].s_ZoomPosition = ZoomPos;
            Result = m_ConfigFileManagerPtr->SetPresetsInfo(&((m_PresetsInfo_InnerPtr.GetPtr())[i]));
            if (FAILED(Result))
            {
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_RmPreset;
                PT_CtlCmd.s_CmdParam[0] = Index;
                m_PtzControlPtr->Control(PTZ_CONTINUE_CONTROL_MODE, &PT_CtlCmd);
                (m_PresetsInfo_InnerPtr.GetPtr())[i].s_Setted   = false;
                SYS_ERROR("m_ConfigFileManagerPtr SetPresetsInfo fail, Result = 0x%lx\n", Result);
                return Result;
            }
            break;
        }
    }

    //create new
    if (i >= MAX_PRESETS)
    {
        int32_t j;
        for (j = 0; j < MAX_PRESETS; j++)
        {
            if (!(m_PresetsInfo_InnerPtr.GetPtr())[j].s_Setted)
            {
                strcpy((m_PresetsInfo_InnerPtr.GetPtr())[j].s_Name, SysPresetInfoPtr->s_PresetName);
                (m_PresetsInfo_InnerPtr.GetPtr())[j].s_Index        = Index;
                (m_PresetsInfo_InnerPtr.GetPtr())[j].s_Setted       = true;
                (m_PresetsInfo_InnerPtr.GetPtr())[j].s_ZoomPosition = ZoomPos;
                Result = m_ConfigFileManagerPtr->SetPresetsInfo(&((m_PresetsInfo_InnerPtr.GetPtr())[j]));
                if (FAILED(Result))
                {
                    PT_CtlCmd.s_Cmd = e_PTZ_CMD_RmPreset;
                    PT_CtlCmd.s_CmdParam[0] = Index;
                    m_PtzControlPtr->Control(PTZ_CONTINUE_CONTROL_MODE, &PT_CtlCmd);
                    (m_PresetsInfo_InnerPtr.GetPtr())[j].s_Setted   = false;
                    SYS_ERROR("m_ConfigFileManagerPtr SetPresetsInfo fail, Result = 0x%lx\n", Result);
                    return Result;
                }
                break;
            }
        }
        if (j >= MAX_PRESETS)
        {
            SYS_ERROR("all presets have been setted\n");
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "all presets have been setted\n");
            return GMI_FAIL;
        }
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return Result;
}


GMI_RESULT SystemServiceManager::GetCompileNum(int32_t *CompilesPtr)
{
    FILE  *Fp = NULL;

    Fp = fopen(VERSION_COMPILE_FILE_NAME, "rb");
    if (NULL == Fp)
    {
        SYS_ERROR("fopen %s error\n", VERSION_COMPILE_FILE_NAME);
        return GMI_FAIL;
    }

    char_t Compiles[64];
    memset(Compiles, 0, sizeof(Compiles));
    while (1)
    {
        char Str[64];
        char Head[20];
        bzero(Str, sizeof(Str));
        bzero(Head, sizeof(Head));
        if (0 == fgets(Str, sizeof(Str), Fp))
            break;
        if (0 == sscanf(Str, "%s", Head))
            continue;
        if (0 == strcmp(Head , COMPILE_NUM_KEY))
        {
            sscanf(Str , "%*s = %s" , Compiles);
        }
    }

    *CompilesPtr = atoi(Compiles);

    if (NULL != Fp)
    {
        fclose(Fp);
        Fp = NULL;
    }

    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::GetVersion(char_t FwVer [64])
{
    int32_t	Year, Month, Day;
    char_t	StrMon[4];
    const char   *StrMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    sscanf(__DATE__, "%s%d%d", StrMon, &Day, &Year);
    for (Month = 0; Month < 12; Month++)
    {
        if (strcmp(StrMon, StrMonths[Month]) == 0)
        {
            break;
        }
    }
    Month++;

    int32_t Milestone = 1;
    int32_t Compiles  = 1;

    GMI_RESULT Result = GetCompileNum(&Compiles);
    if (FAILED(Result))
    {
        SYS_ERROR("get compile num fail, Result = 0x%lx\n", Result);
        Compiles = 1;
    }

    sprintf(FwVer, "V3.0.00%02d.%04d%02d%02d%03d", Milestone, Year, Month, Day, Compiles);

    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetTimeType(SysPkgDateTimeType *SysPkgDateTimePtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (NULL == SysPkgDateTimePtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    SysPkgDateTimeType SysTimeType;

    memset(&SysTimeType, 0, sizeof(SysPkgDateTimeType));
    memcpy(&SysTimeType, SysPkgDateTimePtr, sizeof(SysPkgDateTimeType));

    pthread_rwlock_wrlock(&m_Lock);
    boolean_t NtpEnable = ((SYS_TIME_TYPE_NTP == SysTimeType.s_Type) ? true : false);
    GMI_RESULT Result = m_BoardManagerPtr->SetNtp(NtpEnable, SysTimeType.s_NtpInterval);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SetNtp fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetNtp fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_ConfigFileManagerPtr->SetSysTimeType(&SysTimeType);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SetSysTimeType fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetSysTimeType fail, Result = 0x%lx\n", Result);
        return Result;
    }
    pthread_rwlock_unlock(&m_Lock);
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetTimeType(SysPkgDateTimeType *SysPkgDateTimePtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (NULL == SysPkgDateTimePtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    memset(SysPkgDateTimePtr, 0, sizeof(SysPkgDateTimeType));

    pthread_rwlock_rdlock(&m_Lock);
    GMI_RESULT Result = m_ConfigFileManagerPtr->GetSysTimeType(SysPkgDateTimePtr);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("GetSysTimeType fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetSysTimeType fail, Result = 0x%lx\n", Result);
        return Result;
    }
    pthread_rwlock_unlock(&m_Lock);
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetTime(SysPkgSysTime *SysPkgTimePtr)

{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (NULL == SysPkgTimePtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Result;
    struct tm NowTime;

    NowTime.tm_year   = SysPkgTimePtr->s_Year  - 1900;
    NowTime.tm_mon    = SysPkgTimePtr->s_Month - 1;
    NowTime.tm_mday   = SysPkgTimePtr->s_Day;
    NowTime.tm_hour   = SysPkgTimePtr->s_Hour;
    NowTime.tm_min    = SysPkgTimePtr->s_Minute;
    NowTime.tm_sec    = SysPkgTimePtr->s_Second;

    pthread_rwlock_wrlock(&m_Lock);
    Result = m_BoardManagerPtr->SetTime(&NowTime);
    if (FAILED(Result))
    {
        SYS_ERROR("SetTime fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetTime fail, Result = 0x%lx\n", Result);
        pthread_rwlock_unlock(&m_Lock);
        return Result;
    }
    pthread_rwlock_unlock(&m_Lock);
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetTime(SysPkgSysTime *SysPkgTimePtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (NULL == SysPkgTimePtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    struct tm NowTime;

    pthread_rwlock_rdlock(&m_Lock);
    GMI_RESULT Result = m_BoardManagerPtr->GetTime(&NowTime);
    if (FAILED(Result))
    {
        SYS_ERROR("GetTime fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetTime fail, Result = 0x%lx\n", Result);
        pthread_rwlock_unlock(&m_Lock);
        return Result;
    }
    pthread_rwlock_unlock(&m_Lock);

    SysPkgTimePtr->s_Year   = NowTime.tm_year + 1900;
    SysPkgTimePtr->s_Month  = NowTime.tm_mon  + 1;
    SysPkgTimePtr->s_Day    = NowTime.tm_mday;
    SysPkgTimePtr->s_Hour   = NowTime.tm_hour;
    SysPkgTimePtr->s_Minute = NowTime.tm_min;
    SysPkgTimePtr->s_Second = NowTime.tm_sec;
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetTimezone(SysPkgTimeZone *SysTimezonePtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    if (NULL == SysTimezonePtr)
    {
        SYS_ERROR("SysTimezonePtr is null\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysTimezonePtr is null\n");
        return GMI_INVALID_PARAMETER;
    }

    FILE   *Fp = NULL;
    int32_t Timezone;
    char_t  Buffer[128];
    char_t *ToGetPtr = NULL;

    memset(Buffer, 0, sizeof(Buffer));
    Fp = popen("date -R", "r");
    if (NULL == Fp)
    {
        SYS_ERROR("popen fail\n");
        return GMI_FAIL;
    }

    if (NULL == fgets(Buffer, sizeof(Buffer), Fp))
    {
        SYS_ERROR("fgets fail\n");
        if (NULL != Fp)
        {
            pclose(Fp);
        }
        return GMI_FAIL;
    }

    memset(SysTimezonePtr, 0, sizeof(SysPkgTimeZone));
    if (NULL != (ToGetPtr = strchr(Buffer, '+')))
    {
        sscanf(ToGetPtr+1, "%d", &Timezone);
        Timezone /= 100;
        sprintf(SysTimezonePtr->s_TimzeZoneName, "CST+%02d:00:00", Timezone);
        SysTimezonePtr->s_TimeZone = Timezone;
    }
    else if (NULL != (ToGetPtr = strchr(Buffer, '-')))
    {
        sscanf(ToGetPtr+1, "%d", &Timezone);
        Timezone /= 100;
        sprintf(SysTimezonePtr->s_TimzeZoneName, "CST-%02d:00:00", Timezone);
        Timezone |= (1 << 31);
        SysTimezonePtr->s_TimeZone = Timezone;
    }
    else
    {
        if (NULL != Fp)
        {
            pclose(Fp);
        }
        return GMI_NOT_SUPPORT;
    }

    SYS_INFO("[%s]: timezone=%d\n", __func__, Timezone);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "[%s]: timezone=%d\n", __func__, Timezone);

    if (NULL != Fp)
    {
        pclose(Fp);
    }
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetTimezone(SysPkgTimeZone *SysTimezonePtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    char_t Zone[32];

    switch (SysTimezonePtr->s_TimeZone)
    {
    case 8:
        strcpy(Zone, "Shanghai");
        break;
    case 0:
        strcpy(Zone, "UTC");
        break;
    default:
        memset(Zone, 0, sizeof(Zone));
        break;
    }

    GMI_RESULT Result;

    if (strlen(Zone))
    {
        Result = m_BoardManagerPtr->SetZone(Zone);
        if (FAILED(Result))
        {
            SYS_ERROR("SetZone fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetZone fail, Result = 0x%lx\n", Result);
            return Result;
        }
    }
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetNtpServerInfo(SysPkgNtpServerInfo *SysNtpServerInfoPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (NULL == SysNtpServerInfoPtr)
    {
        return GMI_INVALID_PARAMETER;
    }
    
	memcpy(SysNtpServerInfoPtr, &m_SysNtpServerInfo, sizeof(SysPkgNtpServerInfo));   
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetNtpServerInfo(SysPkgNtpServerInfo *SysNtpServerInfoPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (NULL == SysNtpServerInfoPtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    SysPkgNtpServerInfo SysNtpServerInfo;
    memset(&SysNtpServerInfo, 0, sizeof(SysPkgNtpServerInfo));
    memcpy(&SysNtpServerInfo, SysNtpServerInfoPtr, sizeof(SysPkgNtpServerInfo));

    pthread_rwlock_wrlock(&m_Lock);
    GMI_RESULT Result = m_BoardManagerPtr->SetNtpServer(SysNtpServerInfo.s_NtpAddr_1);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("m_BoardManagerPtr SetNtpServer fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_BoardManagerPtr SetNtpServer fail, Result = 0x%lx\n", Result);
        return GMI_FAIL;
    }

    Result = m_ConfigFileManagerPtr->SetNtpServerInfo(&SysNtpServerInfo);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("save NtpServerInfo to file fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "save NtpServerInfo to file fail, Result = 0x%lx\n", Result);
        return GMI_FAIL;
    }
    pthread_rwlock_unlock(&m_Lock);
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrNetReadIpInfo( SysPkgIpInfo *SysPkgIpInfoPtr )
{
    GMI_RESULT Result = GMI_SUCCESS;
    IpInfo IpInfoTmp;
    char_t Mac[32];

    if (SysPkgIpInfoPtr == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    memset(Mac, 0, sizeof(Mac));
    memset(&IpInfoTmp, 0, sizeof(IpInfo));

    pthread_rwlock_rdlock(&m_Lock);
    Result = NetReadIP(0, &IpInfoTmp);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        return Result;
    }
    Result = NetReadMac(0, Mac);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        return Result;
    }
    pthread_rwlock_unlock(&m_Lock);

    struct in_addr Addr1, Addr2, Addr3;

    SysPkgIpInfoPtr->s_NetId = IpInfoTmp.s_Eth + 1;
    SysPkgIpInfoPtr->s_Dhcp  = (uint8_t)IpInfoTmp.s_Dhcp;
    Addr1.s_addr = IpInfoTmp.s_Dns1;
    Addr2.s_addr = IpInfoTmp.s_Dns2;
    Addr3.s_addr = IpInfoTmp.s_Dns3;
    sprintf(SysPkgIpInfoPtr->s_Dns, "%s", inet_ntoa(Addr1));
    sprintf(SysPkgIpInfoPtr->s_Dns + strlen(SysPkgIpInfoPtr->s_Dns), " %s", inet_ntoa(Addr2));
    sprintf(SysPkgIpInfoPtr->s_Dns + strlen(SysPkgIpInfoPtr->s_Dns), " %s", inet_ntoa(Addr3));

    Addr1.s_addr = IpInfoTmp.s_GateWay;
    sprintf(SysPkgIpInfoPtr->s_GateWay, "%s", inet_ntoa(Addr1));
    Addr1.s_addr = IpInfoTmp.s_IpAddr;
    sprintf(SysPkgIpInfoPtr->s_IpAddress, "%s", inet_ntoa(Addr1));
    Addr1.s_addr = IpInfoTmp.s_NetMask;
    sprintf(SysPkgIpInfoPtr->s_SubNetMask, "%s", inet_ntoa(Addr1));
    sprintf(SysPkgIpInfoPtr->s_InterfName, "eth%d", IpInfoTmp.s_Eth);
    memcpy(SysPkgIpInfoPtr->s_HwAddress, Mac, sizeof(SysPkgIpInfoPtr->s_HwAddress));

    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrNetWriteIpInfo(SysPkgIpInfo *SysPkgIpInfoPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    GMI_RESULT Result = GMI_SUCCESS;
    IpInfo IpInfoTmp;

    if ( SysPkgIpInfoPtr == NULL )
    {
        return GMI_INVALID_PARAMETER;
    }

    memset(&IpInfoTmp, 0, sizeof(IpInfo) );

    SYS_INFO("NetId        = %d\n", SysPkgIpInfoPtr->s_NetId);
    SYS_INFO("IP           = %s\n", SysPkgIpInfoPtr->s_IpAddress);
    SYS_INFO("s_GateWay    = %s\n", SysPkgIpInfoPtr->s_GateWay);
    SYS_INFO("s_NetMask    = %s\n", SysPkgIpInfoPtr->s_SubNetMask);
    SYS_INFO("Mac          = %s\n", SysPkgIpInfoPtr->s_HwAddress);
    SYS_INFO("Dns          = %s\n", SysPkgIpInfoPtr->s_Dns);

    Result = CheckIPv4String(SysPkgIpInfoPtr->s_IpAddress);
    if (FAILED(Result))
    {
        SYS_ERROR("set Ip %s incorrect\n", SysPkgIpInfoPtr->s_IpAddress);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "set Ip %s incorrect\n", SysPkgIpInfoPtr->s_IpAddress);
        return Result;
    }

    Result = CheckIPv4MaskString(SysPkgIpInfoPtr->s_SubNetMask);
    if (FAILED(Result))
    {
        SYS_ERROR("set netmask %s incorrect\n", SysPkgIpInfoPtr->s_SubNetMask);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "set netmask %s incorrect\n", SysPkgIpInfoPtr->s_SubNetMask);
        return Result;
    }

    Result = CheckIPv4Config(SysPkgIpInfoPtr->s_IpAddress, SysPkgIpInfoPtr->s_SubNetMask, SysPkgIpInfoPtr->s_GateWay);
    if (FAILED(Result))
    {
        SYS_ERROR("ip:%s netmask:%s gateway:%s improper\n", SysPkgIpInfoPtr->s_IpAddress, SysPkgIpInfoPtr->s_SubNetMask, SysPkgIpInfoPtr->s_GateWay);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ip:%s netmask:%s gateway:%s improper\n", SysPkgIpInfoPtr->s_IpAddress, SysPkgIpInfoPtr->s_SubNetMask, SysPkgIpInfoPtr->s_GateWay);
        return Result;
    }

    if (0 != strlen(SysPkgIpInfoPtr->s_HwAddress))
    {
        Result = CheckMacString(SysPkgIpInfoPtr->s_HwAddress);
        if (FAILED(Result))
        {
            SYS_ERROR("set mac %s incorrect\n", SysPkgIpInfoPtr->s_HwAddress);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "set mac %s incorrect\n", SysPkgIpInfoPtr->s_HwAddress);
            return Result;
        }
    }

    IpInfoTmp.s_Eth     = SysPkgIpInfoPtr->s_NetId - 1;
    IpInfoTmp.s_Dhcp    = (int32_t)SysPkgIpInfoPtr->s_Dhcp;
    IpInfoTmp.s_IpAddr  = inet_addr(SysPkgIpInfoPtr->s_IpAddress);
    IpInfoTmp.s_GateWay = inet_addr(SysPkgIpInfoPtr->s_GateWay);
    IpInfoTmp.s_NetMask = inet_addr(SysPkgIpInfoPtr->s_SubNetMask);

    char_t Dns1[32];
    char_t Dns2[32];
    char_t Dns3[32];
    sscanf(SysPkgIpInfoPtr->s_Dns, "%s %s %s", Dns1, Dns2, Dns3);
    IpInfoTmp.s_Dns1 = inet_addr(Dns1);
    IpInfoTmp.s_Dns2 = inet_addr(Dns2);
    IpInfoTmp.s_Dns3 = inet_addr(Dns3);

    pthread_rwlock_wrlock(&m_Lock );
    Result = NetWriteIP(&IpInfoTmp);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock( &m_Lock );
        return Result;
    }

    SYS_INFO("%s:HwAddress %s\n", __func__, SysPkgIpInfoPtr->s_HwAddress);
    //if (0 != strlen(SysPkgIpInfoPtr->s_HwAddress))
    //{
    //    Result = NetWriteMac(0, SysPkgIpInfoPtr->s_HwAddress);
    //    if (FAILED(Result))
    //    {
    //        pthread_rwlock_unlock( &m_Lock );
    //        return Result;
    //    }
    //}
    pthread_rwlock_unlock(&m_Lock);

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrNetReadMacInfo( char_t *MacPtr )
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    GMI_RESULT Result = GMI_SUCCESS;

    if ( MacPtr == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    pthread_rwlock_rdlock( &m_Lock );
    Result = NetReadMac( 0, MacPtr);
    if ( FAILED(Result) )
    {
        pthread_rwlock_unlock( &m_Lock );
        return Result;
    }
    pthread_rwlock_unlock( &m_Lock );
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);

    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::FactoryResetNetInfo(void)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    SysPkgIpInfo IpInfo;

    memset(&IpInfo, 0, sizeof(SysPkgIpInfo));
    IpInfo.s_Dhcp  = NET_DEFAULT_DHCP;
    IpInfo.s_NetId = 1;
    sprintf(IpInfo.s_InterfName, "eth%d", NET_DEFAULT_ETH);
    strcpy(IpInfo.s_IpAddress, NET_DEFAULT_IP);
    strcpy(IpInfo.s_SubNetMask, NET_DEFAULT_MASK);
    strcpy(IpInfo.s_GateWay, NET_DEFAULT_GATEWAY);
    strcpy(IpInfo.s_Dns, NET_DEFAULT_DNS);
    GMI_RESULT Result = SvrNetWriteIpInfo(&IpInfo);
    if (FAILED(Result))
    {
        SYS_ERROR("Svr write ip fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Svr write ip fail, Result = 0x%lx\n", Result);
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);

    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetDevinfo(SysPkgDeviceInfo *SysDeviceInfoPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (NULL == SysDeviceInfoPtr)
    {
        SYS_ERROR("SysDeviceInfoPtr is null\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysDeviceInfoPtr is null\n");
        return GMI_INVALID_PARAMETER;
    }

    memcpy(SysDeviceInfoPtr, &m_SysDeviceInfo, sizeof(SysPkgDeviceInfo));
    GMI_RESULT Result = GetVersion(SysDeviceInfoPtr->s_DeviceFwVer);
    if (FAILED(Result))
    {
    	SYS_ERROR("Get Version fail, Result = 0x%lx\n", Result);
        return Result;
    }
    
    //sn set to hostname
    if (0 != strlen(SysDeviceInfoPtr->s_DeviceSerialNum))
    {
        char_t CmdBuff[128];
        memset(CmdBuff, 0, sizeof(CmdBuff));
        sprintf(CmdBuff, "hostname %s", SysDeviceInfoPtr->s_DeviceSerialNum);
        system(CmdBuff);
    }    

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetDeviceInfo(SysPkgDeviceInfo *SysDeviceInfoPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (NULL == SysDeviceInfoPtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    pthread_rwlock_wrlock(&m_Lock);
    GMI_RESULT Result = m_ConfigFileManagerPtr->SetDeviceInfo(SysDeviceInfoPtr);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        return Result;
    }
    memcpy(&m_SysDeviceInfo, SysDeviceInfoPtr, sizeof(SysPkgDeviceInfo));
    pthread_rwlock_unlock(&m_Lock);
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetUserNum(uint32_t *UserNumPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    if (NULL == UserNumPtr)
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "invalid param\n");
        return GMI_INVALID_PARAMETER;
    }

    pthread_rwlock_rdlock(&m_Lock);
    GMI_RESULT Result = m_UserManagerPtr->GetUserNum(UserNumPtr);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        return Result;
    }
    pthread_rwlock_unlock(&m_Lock);

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetAllUsers(SysPkgUserInfo * UserInfoPtr, uint32_t UserInfoNum, uint32_t *RealUserNum)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    pthread_rwlock_rdlock(&m_Lock);
    GMI_RESULT Result = m_UserManagerPtr->GetAllUsers(UserInfoPtr, UserInfoNum, RealUserNum);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        return Result;
    }
    pthread_rwlock_unlock(&m_Lock);

    //encrypt
    int32_t CiperLen = 0;
    char_t  Passwd[LEN_DES_PASSWORD];
    char_t  Key[8];
    char_t  Name[32];

    memset(Name, 0, sizeof(Name));
    sprintf(Name, "eth0");
    memset(Key, 0, sizeof(Key));
    if (0 > NetReadMacChar(Name, Key))
    {
        return GMI_FAIL;
    }

    for (uint32_t i = 0; i < *RealUserNum; i++)
    {
        memset(Passwd, 0, sizeof(Passwd));
        strcpy(Passwd, UserInfoPtr[i].s_UserPass);
        if (0 > DES_Encrypt(Passwd, sizeof(Passwd), Key, UserInfoPtr[i].s_UserPass, &CiperLen))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "DES_Encrypt fail\n");
            SYS_ERROR("DES_Encrypt fail\n");
            return GMI_FAIL;
        }

        SYS_INFO("Users[%d].Name %s\n", i, UserInfoPtr[i].s_UserName);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Users[%d].Name %s\n", i, UserInfoPtr[i].s_UserName);
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetUser(SysPkgUserInfo * UserInfoPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    //decrypt
    int32_t PlainLen = 0;
    char_t  Passwd[LEN_DES_PASSWORD];
    char_t  Key[8];
    char_t  Name[32];

	if (NULL == UserInfoPtr)
	{
		SYS_ERROR("user info is null\n");
		DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user info is null\n");
		return GMI_INVALID_PARAMETER;
	}
	
	if (sizeof(Name) < strlen(UserInfoPtr->s_UserName)
		|| LEN_DES_PASSWORD < strlen(UserInfoPtr->s_UserPass))
	{
		SYS_ERROR("username %s or userpass %s is too long\n", UserInfoPtr->s_UserName, UserInfoPtr->s_UserPass);
		DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "username %s or userpass %s is too long\n", UserInfoPtr->s_UserName, UserInfoPtr->s_UserPass);
		return GMI_INVALID_PARAMETER;
	}
	
    memset(Name, 0, sizeof(Name));
    sprintf(Name, "eth0");
    memset(Key, 0, sizeof(Key));
    if (0 > NetReadMacChar(Name, Key))
    {
        return GMI_FAIL;
    }

    memset(Passwd, 0, sizeof(Passwd));
    memcpy(Passwd, UserInfoPtr->s_UserPass, sizeof(Passwd));
    if (0 > DES_Decrypt(Passwd, sizeof(Passwd), Key, UserInfoPtr->s_UserPass, &PlainLen))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "DES_Decrypt fail\n");
        SYS_ERROR("DES_Decrypt fail\n");
        return GMI_FAIL;
    }
    UserInfoPtr->s_UserPass[LEN_DES_PASSWORD] = '\0';

    pthread_rwlock_wrlock(&m_Lock);
    GMI_RESULT Result = m_UserManagerPtr->SetUser(UserInfoPtr);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        return Result;
    }
    pthread_rwlock_unlock(&m_Lock);

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrDelUser(SysPkgUserInfo * UserInfoPtr)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    pthread_rwlock_wrlock(&m_Lock);
    GMI_RESULT Result = m_UserManagerPtr->DeleteUser(UserInfoPtr);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        return Result;
    }
    pthread_rwlock_unlock(&m_Lock);

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetNetworkPort(SysPkgNetworkPort *SysNetworkPort)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    if (SysNetworkPort->s_HTTP_Port < 0
            || SysNetworkPort->s_RTSP_Port < 0
            || SysNetworkPort->s_SDK_Port < 0
            || SysNetworkPort->s_ONVIF_Port < 0)
    {
        SYS_ERROR("SysNetworkPort less than 0, HTTP_Port %d, RTSP_Port %d, SDK_Port %d, ONVIF_Port %d\n", \
        		SysNetworkPort->s_HTTP_Port, SysNetworkPort->s_RTSP_Port, SysNetworkPort->s_SDK_Port, SysNetworkPort->s_ONVIF_Port);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetExternNetworkPort less than 0, HTTP_Port %d, RTSP_Port %d, SDK_Port %d, ONVIF_Port %d\n", \
        		SysNetworkPort->s_HTTP_Port, SysNetworkPort->s_RTSP_Port, SysNetworkPort->s_SDK_Port, SysNetworkPort->s_ONVIF_Port);
        return GMI_INVALID_PARAMETER;
    }

    if ((SysNetworkPort->s_HTTP_Port == SysNetworkPort->s_RTSP_Port)
            || (SysNetworkPort->s_HTTP_Port == SysNetworkPort->s_SDK_Port)
            || (SysNetworkPort->s_RTSP_Port == SysNetworkPort->s_SDK_Port)
            || (SysNetworkPort->s_ONVIF_Port == SysNetworkPort->s_HTTP_Port)) //temp, ajust onvif port to http port finally.8/14,guoqiang.
    {
        SYS_ERROR("SysNetworkPort Equal each other, HTTP_Port %d, RTSP_Port %d, SDK_Port %d, ONVIF_Port %d\n", \
        		SysNetworkPort->s_HTTP_Port, SysNetworkPort->s_RTSP_Port, SysNetworkPort->s_SDK_Port, SysNetworkPort->s_ONVIF_Port);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysNetworkPort Equal each other, HTTP_Port %d, RTSP_Port %d, SDK_Port %d, ONVIF_Port %d\n", \
        		SysNetworkPort->s_HTTP_Port, SysNetworkPort->s_RTSP_Port, SysNetworkPort->s_SDK_Port, SysNetworkPort->s_ONVIF_Port);
        return GMI_INVALID_PARAMETER;
    }

    if (SysNetworkPort->s_SDK_Port < 1024
            || SysNetworkPort->s_SDK_Port > 65500)
    {
        SYS_ERROR("SDK port %d incorrect\n", SysNetworkPort->s_SDK_Port);
        return GMI_INVALID_PARAMETER;
    }

    if (SysNetworkPort->s_HTTP_Port < 0
            || SysNetworkPort->s_HTTP_Port > 65535)
    {
        SYS_ERROR("HTTP port %d incorrect\n", SysNetworkPort->s_HTTP_Port);
        return GMI_INVALID_PARAMETER;
    }

    if (SysNetworkPort->s_Upgrade_Port < 0
            || SysNetworkPort->s_Upgrade_Port > 65535)
    {
        SYS_ERROR("Upgrade port %d incorrect\n", SysNetworkPort->s_Upgrade_Port);
        return GMI_INVALID_PARAMETER;
    }

    if (SysNetworkPort->s_ONVIF_Port < 0
    	|| SysNetworkPort->s_ONVIF_Port > 65535)
    {
    	SYS_ERROR("ONVIF port %d incorrect\n", SysNetworkPort->s_ONVIF_Port);
        return GMI_INVALID_PARAMETER;
    }

    pthread_rwlock_wrlock(&m_Lock);
    GMI_RESULT Result = m_ConfigFileManagerPtr->SetExternNetworkPort(SysNetworkPort);
    if (FAILED(Result))
    {
        pthread_rwlock_unlock(&m_Lock);
        SYS_ERROR("SetExternNetworkPort fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetExternNetworkPort fail, Result = 0x%lx\n", Result);
        return Result;
    }
    memcpy(&m_SysNetWorkPort, SysNetworkPort, sizeof(SysPkgNetworkPort));
    pthread_rwlock_unlock(&m_Lock);

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetNetworkPort(SysPkgNetworkPort *SysNetworkPort)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);   
    memcpy(SysNetworkPort, &m_SysNetWorkPort, sizeof(SysPkgNetworkPort));    
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrSetSystemDefault(int32_t SysCtrlCmd, int32_t ConfigModule)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    GMI_RESULT Result = GMI_SUCCESS;

    SYS_INFO("SysCtrlCmd %d, ConfigModule %d\n", SysCtrlCmd, ConfigModule);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "SysCtrlCmd %d, ConfigModule %d\n", SysCtrlCmd, ConfigModule);

    if (SYS_SYSTEM_CTRL_DEFAULT_HARD == SysCtrlCmd)
    {
        if (SYS_CONFIG_MODULE_IMAGING == ConfigModule)
        {
            Result = FactoryResetImaging();
            if (FAILED(Result))
            {
                SYS_ERROR("FactoryResetImaging fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "FactoryResetImaging fail, Result = 0x%lx\n", Result);
                return Result;
            }
        }
        else if (SYS_CONFIG_MODULE_STREAM_COMBINE == ConfigModule)
        {
            Result = FactoryResetStreamCombine();
            if (FAILED(Result))
            {
                SYS_ERROR("FactoryResetStreamCombine fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "FactoryResetStreamCombine fail, Result = 0x%lx\n", Result);
                return Result;
            }
        }
        else if (SYS_CONFIG_MODULE_VIDEO_ENCODE == ConfigModule)
        {
            Result = FactoryResetVideoEncode();
            if (FAILED(Result))
            {
                SYS_ERROR("FactoryResetVideoEncode fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "FactoryResetVideoEncode fail, Result = 0x%lx\n", Result);
                return Result;
            }
        }
        else if (SYS_CONFIG_MODULE_ALL == ConfigModule)
        {
            //setting and resources
            Result = m_ConfigFileManagerPtr->FactoryDefault();
            if (FAILED(Result))
            {
                SYS_ERROR("ProcessFactoryDefault fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ProcessFactoryDefault fail, Result = 0x%lx\n", Result);
                return Result;
            }

            //users
            Result = m_UserManagerPtr->FactoryDefault();
            if (FAILED(Result))
            {
                SYS_ERROR("FactoryDefault fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "FactoryDefault fail, Result = 0x%lx\n", Result);
                return Result;
            }

            //network
            Result = FactoryResetNetInfo();
            if (FAILED(Result))
            {
                SYS_ERROR("NetFacotryDefault fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "NetFacotryDefault fail, Result = 0x%lx\n", Result);
                return Result;
            }

            //delay reboot
            Result = DaemonReboot(FACTORY_DEFAULT_REBOOT_DELAY_TIMES);
            if (FAILED(Result))
            {
                DaemonReboot(FACTORY_DEFAULT_REBOOT_DELAY_TIMES);
            }
        }
        else
        {
            SYS_ERROR("not support module %d default\n", ConfigModule);
            return GMI_NOT_SUPPORT;
        }
    }
    else if (SYS_SYSTEM_CTRL_DEFAULT_SOFT == SysCtrlCmd)
    {
        SYS_INFO("Simple factory default\n");
        //setting and resources
        Result = m_ConfigFileManagerPtr->FactoryDefault();
        if (FAILED(Result))
        {
            SYS_ERROR("ProcessFactoryDefault fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ProcessFactoryDefault fail, Result = 0x%lx\n", Result);
            return Result;
        }

        //users
        //Result = m_UserManagerPtr->FactoryDefault();
        //if (FAILED(Result))
        //{
        //    SYS_ERROR("FactoryDefault fail, Result = 0x%lx\n", Result);
        //    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "FactoryDefault fail, Result = 0x%lx\n", Result);
        //    return Result;
        //}

        //delay reboot
        Result = DaemonReboot(FACTORY_DEFAULT_REBOOT_DELAY_TIMES);
        if (FAILED(Result))
        {
            DaemonReboot(FACTORY_DEFAULT_REBOOT_DELAY_TIMES);
        }
    }
    else
    {
        return GMI_NOT_SUPPORT;
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::GetCapabilities(int32_t CapabilityCategory, int32_t CapabilityBufferLength, char_t* Capability, SysPkgXml *SysCapabilities)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    if (SYS_CAPABILITY_CATEGORY_ALL > CapabilityCategory
            || SYS_CAPABILITY_CATEGORY_PTZ < CapabilityCategory)
    {
        return GMI_INVALID_PARAMETER;
    }

	FILE *Fp = NULL;
    Fp = fopen(CAPABILITY_SW_FILE_NAME, "rb");
    if (NULL == Fp)
    {
    	SYS_ERROR("%s not exist\n", CAPABILITY_SW_FILE_NAME);
        return GMI_INVALID_OPERATION;
    }
    if (NULL != Fp)
    {
    	fclose(Fp);
    	Fp = NULL;
    }

	int32_t ReadSize = (CapabilityBufferLength >= m_SysCapability.s_ContentLength ? m_SysCapability.s_ContentLength : CapabilityBufferLength);
    memcpy(Capability, m_CapabilitiesMessagePtr.GetPtr(), ReadSize);
    memcpy(SysCapabilities, &m_SysCapability, sizeof(SysPkgXml));   	

    SYS_INFO("SysCapabilities->s_ContentLength %d\n", SysCapabilities->s_ContentLength);
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::GetWorkState(int32_t WorkStateBufferLength, char_t* WorkState, SysPkgXml *SysWorkState)
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);

    SysWorkState->s_Encrypt = 0;

    int32_t Len = sprintf(WorkState,"<?xml version=\"1.0\"?>");
    Len += sprintf(WorkState + Len, "<WorkState>");

    Len += sprintf(WorkState + Len, "<State>%d</State>", SYS_STATE_DEVICE_NORMAL);

    Len += sprintf(WorkState + Len, "<StreamState>");
    Len += sprintf(WorkState + Len, "<ActiveStreamNum>%d</ActiveStreamNum>", m_VideoStreamNum);
    for (uint16_t i = 0; i < m_VideoStreamNum; i++)
    {
        Len += sprintf(WorkState + Len, "<Stream%d>", i);
        Len += sprintf(WorkState + Len, "<Record>%d</Record>", SYS_STATE_NORECORD);
        Len += sprintf(WorkState + Len, "<VideoSignal>%d</VideoSignal>", SYS_STATE_VIDEO_SIGNAL_NORMAL);
        if (1 == (m_VideoEncParamPtr.GetPtr())[i].s_EncodeType)
        {
            Len += sprintf(WorkState + Len, "<EncodeType>H264</EncodeType>");
        }
        else if (2 == (m_VideoEncParamPtr.GetPtr())[i].s_EncodeType)
        {
            Len += sprintf(WorkState + Len, "<EncodeType>MJPEG</EncodeType>");
        }
        else
        {
            Len += sprintf(WorkState + Len, "<EncodeType>NONE</EncodeType>");
        }
        Len += sprintf(WorkState + Len, "<EncodeWidth>%d</EncodeWidth>", (m_VideoEncParamPtr.GetPtr())[i].s_EncodeWidth);
        Len += sprintf(WorkState + Len, "<EncodeHeight>%d</EncodeHeight>", (m_VideoEncParamPtr.GetPtr())[i].s_EncodeHeight);
        if (1 == (m_VideoEncParamPtr.GetPtr())[i].s_BitRateType)
        {
            Len += sprintf(WorkState + Len, "<BitRateType>CBR</BitRateType>");
        }
        else
        {
            Len += sprintf(WorkState + Len, "<BitRateType>VBR</BitRateType>");
        }
        Len += sprintf(WorkState + Len, "<BitRate>%d</BitRate>", (m_VideoEncParamPtr.GetPtr())[i].s_BitRateAverage);
        Len += sprintf(WorkState + Len, "<BitRateDown>%d</BitRateDown>", (m_VideoEncParamPtr.GetPtr())[i].s_BitRateDown);
        Len += sprintf(WorkState + Len, "<BitRateUp>%d</BitRateUp>", (m_VideoEncParamPtr.GetPtr())[i].s_BitRateUp);
        Len += sprintf(WorkState + Len, "<FrameInterval>%d</FrameInterval>", (m_VideoEncParamPtr.GetPtr())[i].s_FrameInterval);
        Len += sprintf(WorkState + Len, "<FrameRate>%d</FrameRate>", (m_VideoEncParamPtr.GetPtr())[i].s_FrameRate);
        Len += sprintf(WorkState + Len, "<EncodeQuality>%d</EncodeQuality>", (m_VideoEncParamPtr.GetPtr())[i].s_EncodeQulity);
        Len += sprintf(WorkState + Len, "<Rotate>%d</Rotate>", (m_VideoEncParamPtr.GetPtr())[i].s_Rotate);
        //Len += sprintf(WorkState + Len, "<LinkNum>2</LinkNum>");
        //Len += sprintf(WorkState + Len, "<ClientIP1>0.0.0.0</ClientIP1>");
        Len += sprintf(WorkState + Len, "</Stream%d>", i);
    }
    Len += sprintf(WorkState + Len, "</StreamState>");

    //Len += sprintf(WorkState + Len, "<DiskState>");
    //Len += sprintf(WorkState + Len, "<Volume>0</Volume>");
    //Len += sprintf(WorkState + Len, "<FreeSpaces>0</FreeSpaces>");
    //Len += sprintf(WorkState + Len, "<DiskState>%d</DiskState>", SYS_STATE_DISK_ACTIVE);
    //Len += sprintf(WorkState + Len, "</DiskState>");

    Len += sprintf(WorkState + Len, "<AlarmInState>");
    Len += sprintf(WorkState + Len, "<AlarmInNum>1</AlarmInNum>");
    Len += sprintf(WorkState + Len, "<AlarmIn0>%d</AlarmIn0>", SYS_STATE_ALARM_IN_OFF);
    Len += sprintf(WorkState + Len, "</AlarmInState>");

    Len += sprintf(WorkState + Len, "<AlarmOutState>");
    Len += sprintf(WorkState + Len, "<AlarmOutNum>1</AlarmOutNum>");
    Len += sprintf(WorkState + Len, "<AlarmOut0>%d</AlarmOut0>", SYS_STATE_ALARM_OUT_OFF);
    Len += sprintf(WorkState + Len, "</AlarmOutState>");

    Len += sprintf(WorkState + Len, "<LocalDisplay>%d</LocalDisplay>", SYS_STATE_DISPLAY_OFF);

    Len += sprintf(WorkState + Len, "</WorkState>");

    //padding
    int32_t Paddings = 4-Len%4;
    for (int32_t i = 0; i < Paddings; i++)
    {
        WorkState[Len + i] = '\0';
    }
    Len += Paddings;

    SysWorkState->s_ContentLength = Len;

    SYS_INFO("SysWorkState->s_ContentLength %d\n", SysWorkState->s_ContentLength);
    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::ExcuteImportConfigFile(SysPkgConfigFileInfo *SysConfigFilePtr)
{
    FactorySettingOperation FactoryOperation;

	FactoryOperation.Initialize();
    GMI_RESULT Result = FactoryOperation.ExcuteImportFile(SysConfigFilePtr);
    if (FAILED(Result))
    {
    	FactoryOperation.Deinitialize();
    	return Result;
    }
    FactoryOperation.Deinitialize();
    return Result;
}


/*=============alarm==================*/
GMI_RESULT SystemServiceManager::SvrGetAlarmConfig(int32_t AlarmId,  int32_t Index, void_t *Parameter, size_t ParameterLength)
{
	GMI_RESULT Result = GMI_SUCCESS;
	switch (AlarmId)
	{
	case SYS_DETECTOR_ID_ALARM_INPUT:
		if (Index >= MAX_ALARM_IN_PORT)
		{
			SYS_ERROR("Index %d exceed, but Max Port num %d\n", Index, MAX_ALARM_IN_PORT);
			return GMI_INVALID_PARAMETER;
		} 
		memcpy(Parameter, &m_SysAlarmInCfg[Index], sizeof(SysPkgAlarmInConfig));
		break;
	case SYS_DETECTOR_ID_PIR:
		memcpy(Parameter, &m_SysAlarmPIRCfg, sizeof(SysPkgAlarmEventConfig));
		break;
	case SYS_PROCESSOR_ID_ALARM_OUTPUT:
		if (Index >= MAX_ALARM_OUT_PORT)
		{
			SYS_ERROR("Index %d exceed, but Max Port num %d\n", Index, MAX_ALARM_OUT_PORT);
			return GMI_INVALID_PARAMETER;
		}
		memcpy(Parameter, &m_SysAlarmOutCfg[Index], sizeof(SysPkgAlarmOutConfig));
		break;
	default:
		return GMI_NOT_SUPPORT;
	}
	
	return Result;
}


GMI_RESULT SystemServiceManager::SvrSetAlarmConfig(int32_t AlarmId, int32_t Index, const void_t *Parameter, size_t ParameterLength)
{
	GMI_RESULT Result = GMI_SUCCESS;
	
	Result = m_AlarmPtr->Config(AlarmId, Index, Parameter, ParameterLength);
	if (FAILED(Result))
	{
		SYS_ERROR("set alarm id %d config fail\n", AlarmId);
		return Result;
	}

	Result = m_ConfigFileManagerPtr->SetAlarmConfig(AlarmId, Index, (void_t*)Parameter, ParameterLength);
	if (FAILED(Result))
	{
		SYS_ERROR("save alarm id %d config fail\n", AlarmId);
		return Result;
	}
	
	switch (AlarmId)
	{
	case SYS_DETECTOR_ID_ALARM_INPUT:
		memcpy(&m_SysAlarmInCfg[Index], Parameter, ParameterLength);
		break;
	case SYS_DETECTOR_ID_PIR:
		memcpy(&m_SysAlarmPIRCfg, Parameter, ParameterLength);
		break;
	case SYS_PROCESSOR_ID_ALARM_OUTPUT:
		memcpy(&m_SysAlarmOutCfg[Index], Parameter, ParameterLength);
		break;
	default:
		return GMI_NOT_SUPPORT;
	}
	
	return GMI_SUCCESS;
}


GMI_RESULT SystemServiceManager::SvrGetAlmScheduleTime(int32_t ScheduleId, int32_t Index, void_t *Parameter, size_t ParameterLength)
{
	GMI_RESULT Result = GMI_SUCCESS;
	
	switch (ScheduleId)
	{
	case SYS_SCHEDULE_TIME_ID_ALARM_IN:
		if (Index >= MAX_ALARM_IN_PORT)
		{
			SYS_ERROR("Index %d exceed, but Max Port num %d\n", Index, MAX_ALARM_IN_PORT);
			return GMI_INVALID_PARAMETER;
		} 
		memcpy(Parameter, &m_SysAlarmInScheduleTime[Index], sizeof(SysPkgAlarmScheduleTime));		
		break;
	case SYS_SCHEDULE_TIME_ID_PIR_DETECT:
		memcpy(Parameter, &m_SysAlarmPIRScheduleTime, sizeof(SysPkgAlarmScheduleTime));
		break;
	case SYS_SCHEDULE_TIME_ID_ALARM_OUT:
		if (Index >= MAX_ALARM_OUT_PORT)
		{
			SYS_ERROR("Index %d exceed, but Max Port num %d\n", Index, MAX_ALARM_OUT_PORT);
			return GMI_INVALID_PARAMETER;
		}
		memcpy(Parameter, &m_SysAlarmOutScheduleTime[Index], sizeof(SysPkgAlarmScheduleTime));
		break;
	default:
		return GMI_NOT_SUPPORT;
	}
	
	return Result;
}


GMI_RESULT SystemServiceManager::SvrSetAlmScheduleTime(int32_t ScheduleId, int32_t Index, const void_t *Parameter, size_t ParameterLength)
{
	GMI_RESULT Result = GMI_SUCCESS;

	Result = m_AlarmPtr->Schedule(ScheduleId, Index, Parameter, ParameterLength);	
	if (FAILED(Result))
	{
		SYS_ERROR("set Sechdule id %d config fail\n", ScheduleId);
		return Result;
	}

	Result = m_ConfigFileManagerPtr->SetAlarmSchedule(ScheduleId, Index, (SysPkgAlarmScheduleTime*)Parameter);
	if (FAILED(Result))
	{
		SYS_ERROR("save Sechdule id %d config fail\n", ScheduleId);
		return Result;
	}

	switch (ScheduleId)
	{
	case SYS_SCHEDULE_TIME_ID_ALARM_IN:
		memcpy(&m_SysAlarmInScheduleTime[Index], Parameter, ParameterLength);		
		break;
	case SYS_SCHEDULE_TIME_ID_PIR_DETECT:
		memcpy(&m_SysAlarmPIRScheduleTime, Parameter, ParameterLength);
		break;
	case SYS_SCHEDULE_TIME_ID_ALARM_OUT:
		memcpy(&m_SysAlarmOutScheduleTime[Index], Parameter, ParameterLength);
		break;
	default:
		return GMI_NOT_SUPPORT;
	}
	
	return GMI_SUCCESS;
}
