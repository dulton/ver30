#include "system_packet.h"
#include "rudp_session_manager.h"
#include "unix_tcp_session_manager.h"
#include "sys_command_processor.h"
#include "sys_get_system_config_cmd.h"
#include "sys_system_ctrl_cmd.h"
#include "sys_ptz_ctrl_cmd.h"
#include "sys_get_ip_info_cmd.h"
#include "sys_set_ip_info_cmd.h"
#include "sys_get_encode_config_cmd.h"
#include "sys_set_encode_config_cmd.h"
#include "sys_get_imaging_cmd.h"
#include "sys_set_imaging_cmd.h"
#include "sys_get_video_source_cmd.h"
#include "sys_get_system_time_cmd.h"
#include "sys_set_system_time_cmd.h"
#include "sys_force_idr_cmd.h"
#include "sys_set_system_config_cmd.h"
#include "sys_get_show_info_cmd.h"
#include "sys_set_show_info_cmd.h"
#include "sys_get_user_info_cmd.h"
#include "sys_set_user_info_cmd.h"
#include "sys_del_user_info_cmd.h"
#include "sys_get_network_port_cmd.h"
#include "sys_set_network_port_cmd.h"
#include "sys_get_advanced_imaging_cmd.h"
#include "sys_set_advanced_imaging_cmd.h"
#include "sys_get_capabilities_cmd.h"
#include "sys_get_work_state_cmd.h"
#include "sys_get_focus_config_cmd.h"
#include "sys_set_focus_config_cmd.h"
#include "sys_set_white_balance_cmd.h"
#include "sys_get_white_balance_cmd.h"
#include "sys_set_video_source_cmd.h"
#include "sys_get_daynight_cmd.h"
#include "sys_set_daynight_cmd.h"
#include "sys_start_audio_decode_cmd.h"
#include "sys_stop_audio_decode_cmd.h"
#include "sys_get_audio_encode_cmd.h"
#include "sys_get_preset_info_cmd.h"
#include "sys_set_preset_info_cmd.h"
#include "sys_get_encstream_combine_cmd.h"
#include "sys_set_encstream_combine_cmd.h"
#include "sys_excute_import_config_file.h"
#include "sys_process_not_support_cmd.h"
#include "sys_search_preset_cmd.h"
#include "log.h"
#include "server_command_pipeline_manager.h"
#include "gmi_system_headers.h"


template<typename T>
GMI_RESULT RegiserCommand( ReferrencePtr<BaseCommandPipelineManager> CommandPipeline )
{
    SafePtr<T> CommandExecutor( BaseMemoryManager::Instance().New<T>() );
    if ( NULL == CommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    GMI_RESULT Result = CommandPipeline->RegisterCommandExecutor( CommandExecutor );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    return GMI_SUCCESS;
}


SysCommandProcessor::SysCommandProcessor( uint16_t LocalRudpCPort, uint16_t LocalRudpSPort, size_t  SessionBufferSize )
    : m_LocalRudpCPort( LocalRudpCPort )
    , m_LocalRudpSPort( LocalRudpSPort )
    , m_SessionBufferSize( SessionBufferSize )
    , m_SessionManager()
    , m_CommandPipeline()
    ,m_Packet()
{
}


SysCommandProcessor::~SysCommandProcessor()
{
}


GMI_RESULT SysCommandProcessor::ContextInitialize()
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    GMI_RESULT Result = GMI_SUCCESS;

    //packet
    m_Packet = BaseMemoryManager::Instance().New<SystemPacket>();
    if (NULL == m_Packet.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SystemPacket new fail\n");
        return -1;
    }

    //command pipeline
    m_CommandPipeline = BaseMemoryManager::Instance().New<ServerCommandPipelineManager>();
    if (NULL == m_CommandPipeline.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ServerCommandPipelineManager new fail\n");
        return -1;
    }

    Result = m_CommandPipeline->Initialize();
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_CommandPipeline->Initialize fail, Result = 0x%lx\n", Result);
        return Result;
    }

    //session manager
    m_SessionManager = BaseMemoryManager::Instance().New<RUDPSessionManager>(m_LocalRudpCPort, m_LocalRudpSPort, m_SessionBufferSize) ;
    if (NULL == m_SessionManager.GetPtr())
    {
        m_CommandPipeline->Deinitialize();
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RUDPSessionManager new fail\n");
        return -1;
    }

    Result = m_SessionManager->Initialize(NULL, 0);
    if (FAILED(Result))
    {
        m_CommandPipeline->Deinitialize();
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_SessionManager->Initialize fail, Result = 0x%lx\n", Result);
        return -1;
    }

    //unix tcp session manager
    m_UnixTcpSessionManager = BaseMemoryManager::Instance().New<UnixTcpSessionManager>(m_SessionBufferSize) ;
    if (NULL == m_UnixTcpSessionManager.GetPtr())
    {
        m_SessionManager->Deinitialize();
        m_CommandPipeline->Deinitialize();
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_UnixTcpSessionManager new fail\n");
        return -1;
    }

    Result = m_UnixTcpSessionManager->Initialize(NULL, 0);
    if (FAILED(Result))
    {
        m_SessionManager->Deinitialize();
        m_CommandPipeline->Deinitialize();
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_UnixTcpSessionManager->Initialize fail, Result = 0x%lx\n", Result);
        return -1;
    }

    //service manager
    m_ServiceManager = BaseMemoryManager::Instance().New<SystemServiceManager>();
    if (NULL == m_ServiceManager.GetPtr())
    {
        m_UnixTcpSessionManager->Deinitialize();
        m_SessionManager->Deinitialize();
        m_CommandPipeline->Deinitialize();
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SystemServiceManager new fail\n");
        return -1;
    }

    Result = m_ServiceManager->Initialize();
    if (FAILED(Result))
    {
        m_UnixTcpSessionManager->Deinitialize();
        m_SessionManager->Deinitialize();
        m_CommandPipeline->Deinitialize();
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ServiceManager->Initialize fail, Result = 0x%lx\n", Result);
        return Result;
    }

    //register packet
    Result = m_CommandPipeline->RegisterPacket(m_Packet);
    if (FAILED(Result))
    {
        m_ServiceManager->Deinitialize();
        m_UnixTcpSessionManager->Deinitialize();
        m_SessionManager->Deinitialize();
        m_CommandPipeline->Deinitialize();
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterPacket fail, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SysCommandProcessor::ContextDeinitialize()
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    GMI_RESULT Result = GMI_SUCCESS;

    Result = m_UnixTcpSessionManager->Deinitialize();
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_UnixTcpSessionManager->Deinitialize fail, Result = 0x%lx\n", Result);
        return Result;
    }
    m_UnixTcpSessionManager = NULL;

    Result = m_SessionManager->Deinitialize();
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_SessionManager->Deinitialize fail, Result = 0x%lx\n", Result);
        return Result;
    }
    m_SessionManager = NULL;

    Result = m_CommandPipeline->Deinitialize();
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_CommandPipeline->Deinitialize fail, Result = 0x%lx\n", Result);
        return Result;
    }
    m_CommandPipeline = NULL;
    m_Packet = NULL;
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SysCommandProcessor::Start()
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    GMI_RESULT Result = GMI_SUCCESS;

    Result = m_CommandPipeline->Start( 1, 1 , 100);
    if (FAILED(Result))
    {
        ContextDeinitialize();
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_CommandPipeline->Start fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_SessionManager->Start(m_CommandPipeline);
    if (FAILED(Result))
    {
        m_CommandPipeline->Stop();
        ContextDeinitialize();
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_SessionManager->Start fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_UnixTcpSessionManager->Start(m_CommandPipeline);
    if (FAILED(Result))
    {
        m_SessionManager->Stop();
        m_CommandPipeline->Stop();
        ContextDeinitialize();
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_SessionManager->Start fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_CommandPipeline->Run(false);
    if (FAILED(Result))
    {
        m_UnixTcpSessionManager->Stop();
        m_SessionManager->Stop();
        ContextDeinitialize();
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_CommandPipeline->Run fail, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SysCommandProcessor::Stop()
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    GMI_RESULT Result = GMI_SUCCESS;

    Result = m_CommandPipeline->Stop();
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_CommandPipeline->Stop fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_SessionManager->Stop();
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_SessionManager->Stop fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_UnixTcpSessionManager->Stop();
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_SessionManager->Stop fail, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}

//this register method need improve, guoqiang, 8/15/2013
GMI_RESULT SysCommandProcessor::RegisterCommand()
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    GMI_RESULT Result = GMI_SUCCESS;

    SafePtr<SysPtzCtrlCommandExecutor> PtzCtrlCommandExecutor( BaseMemoryManager::Instance().New<SysPtzCtrlCommandExecutor>() );
    if (NULL == PtzCtrlCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PtzCtrlCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = PtzCtrlCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(PtzCtrlCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetSysCfgCommandExecutor> GetSysCfgCommandExecutor( BaseMemoryManager::Instance().New<SysGetSysCfgCommandExecutor>() );
    if (NULL == GetSysCfgCommandExecutor.GetPtr())
    {
        SYS_ERROR("GetSysCfgCommandExecutor new fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetSysCfgCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetSysCfgCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        SYS_ERROR("SetParameter fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor( GetSysCfgCommandExecutor );
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSystemCtrlCommandExecutor> SystemCtrlCommandExecutor( BaseMemoryManager::Instance().New<SysSystemCtrlCommandExecutor>() );
    if (NULL == SystemCtrlCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SystemCtrlCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SystemCtrlCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SystemCtrlCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetIpInfoCommandExecutor> GetIpInfoCommandExecutor( BaseMemoryManager::Instance().New<SysGetIpInfoCommandExecutor>() );
    if (NULL == GetIpInfoCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetIpInfoCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetIpInfoCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetIpInfoCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSetIpInfoCommandExecutor> SetIpInfoCommandExecutor( BaseMemoryManager::Instance().New<SysSetIpInfoCommandExecutor>() );
    if (NULL == SetIpInfoCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetIpInfoCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SetIpInfoCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SetIpInfoCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetEncodeConfigCommandExecutor> GetEncodeConfigCommandExecutor( BaseMemoryManager::Instance().New<SysGetEncodeConfigCommandExecutor>() );
    if (NULL == GetEncodeConfigCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetEncodeConfigCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetEncodeConfigCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetEncodeConfigCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSetEncodeConfigCommandExecutor> SetEncodeConfigCommandExecutor( BaseMemoryManager::Instance().New<SysSetEncodeConfigCommandExecutor>() );
    if (NULL == SetEncodeConfigCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetEncodeConfigCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SetEncodeConfigCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SetEncodeConfigCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetImagingCommandExecutor> GetImagingCommandExecutor( BaseMemoryManager::Instance().New<SysGetImagingCommandExecutor>() );
    if (NULL == GetImagingCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetImagingCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetImagingCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetImagingCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSetImagingCommandExecutor> SetImagingCommandExecutor( BaseMemoryManager::Instance().New<SysSetImagingCommandExecutor>() );
    if (NULL == SetImagingCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetImagingCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SetImagingCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SetImagingCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetVideoSourceCommandExecutor> GetVideoSourceCommandExecutor( BaseMemoryManager::Instance().New<SysGetVideoSourceCommandExecutor>() );
    if (NULL == GetVideoSourceCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetVideoSourceCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetVideoSourceCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetVideoSourceCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSetVideoSourceCommandExecutor> SetVideoSourceCommandExecutor( BaseMemoryManager::Instance().New<SysSetVideoSourceCommandExecutor>() );
    if (NULL == SetVideoSourceCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetVideoSourceCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SetVideoSourceCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SetVideoSourceCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetTimeCommandExecutor> GetTimeCommandExecutor( BaseMemoryManager::Instance().New<SysGetTimeCommandExecutor>() );
    if (NULL == GetTimeCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetTimeCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetTimeCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetTimeCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSetTimeCommandExecutor> SetTimeCommandExecutor(BaseMemoryManager::Instance().New<SysSetTimeCommandExecutor>());
    if (NULL == SetTimeCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetTimeCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SetTimeCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SetTimeCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysForceIdrCommandExecutor> ForceIdrCommandExecutor(BaseMemoryManager::Instance().New<SysForceIdrCommandExecutor>());
    if (NULL == ForceIdrCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ForceIdrCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = ForceIdrCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(ForceIdrCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSetSystemConfigCommandExecutor> SetSystemConfigCommandExecutor( BaseMemoryManager::Instance().New<SysSetSystemConfigCommandExecutor>() );
    if (NULL == SetSystemConfigCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetSystemConfigCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SetSystemConfigCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SetSystemConfigCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetShowInfoCommandExecutor> GetShowInfoCommandExecutor( BaseMemoryManager::Instance().New<SysGetShowInfoCommandExecutor>() );
    if (NULL == GetShowInfoCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetShowInfoCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetShowInfoCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetShowInfoCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSetShowInfoCommandExecutor> SetShowInfoCommandExecutor(BaseMemoryManager::Instance().New<SysSetShowInfoCommandExecutor>() );
    if (NULL == SetShowInfoCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetShowInfoCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SetShowInfoCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SetShowInfoCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetUserInfoCommandExecutor> GetUserInfoCommandExecutor(BaseMemoryManager::Instance().New<SysGetUserInfoCommandExecutor>() );
    if (NULL == GetUserInfoCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetUserInfoCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetUserInfoCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetUserInfoCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSetUserInfoCommandExecutor> SetUserInfoCommandExecutor(BaseMemoryManager::Instance().New<SysSetUserInfoCommandExecutor>() );
    if (NULL == SetUserInfoCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetUserInfoCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SetUserInfoCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SetUserInfoCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysDelUserInfoCommandExecutor> DelUserInfoCommandExecutor(BaseMemoryManager::Instance().New<SysDelUserInfoCommandExecutor>() );
    if (NULL == DelUserInfoCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "DelUserInfoCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = DelUserInfoCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(DelUserInfoCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetNetworkPortCommandExecutor> GetNetworkPortCommandExecutor(BaseMemoryManager::Instance().New<SysGetNetworkPortCommandExecutor>() );
    if (NULL == GetNetworkPortCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetNetworkPortCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetNetworkPortCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetNetworkPortCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSetNetworkPortCommandExecutor> SetNetworkPortCommandExecutor(BaseMemoryManager::Instance().New<SysSetNetworkPortCommandExecutor>() );
    if (NULL == SetNetworkPortCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetNetworkPortCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SetNetworkPortCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SetNetworkPortCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetAdvancedImagingCommandExecutor> GetAdvancedImagingCommandExecutor(BaseMemoryManager::Instance().New<SysGetAdvancedImagingCommandExecutor>() );
    if (NULL == GetAdvancedImagingCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetAdvancedImagingCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetAdvancedImagingCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetAdvancedImagingCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSetAdvancedImagingCommandExecutor> SetAdvancedImagingCommandExecutor(BaseMemoryManager::Instance().New<SysSetAdvancedImagingCommandExecutor>() );
    if (NULL == SetAdvancedImagingCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetAdvancedImagingCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SetAdvancedImagingCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SetAdvancedImagingCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetCapabilitiesCommandExecutor> GetCapabilitiesCommandExecutor(BaseMemoryManager::Instance().New<SysGetCapabilitiesCommandExecutor>() );
    if (NULL == GetCapabilitiesCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetCapabilitiesCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetCapabilitiesCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetCapabilitiesCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetWorkStateCommandExecutor> GetWorkStateCommandExecutor(BaseMemoryManager::Instance().New<SysGetWorkStateCommandExecutor>() );
    if (NULL == GetWorkStateCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetWorkStateCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetWorkStateCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetWorkStateCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetFocusConfigCommandExecutor> GetFocusConfigCommandExecutor(BaseMemoryManager::Instance().New<SysGetFocusConfigCommandExecutor>() );
    if (NULL == GetFocusConfigCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetFocusConfigCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetFocusConfigCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetFocusConfigCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSetFocusConfigCommandExecutor> SetFocusConfigCommandExecutor(BaseMemoryManager::Instance().New<SysSetFocusConfigCommandExecutor>() );
    if (NULL == SetFocusConfigCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetFocusConfigCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SetFocusConfigCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SetFocusConfigCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetWhiteBalanceCommandExecutor> GetWhiteBalanceCommandExecutor(BaseMemoryManager::Instance().New<SysGetWhiteBalanceCommandExecutor>() );
    if (NULL == GetWhiteBalanceCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetWhiteBalanceCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetWhiteBalanceCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetWhiteBalanceCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSetWhiteBalanceCommandExecutor> SetWhiteBalanceCommandExecutor(BaseMemoryManager::Instance().New<SysSetWhiteBalanceCommandExecutor>() );
    if (NULL == SetWhiteBalanceCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetWhiteBalanceCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SetWhiteBalanceCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SetWhiteBalanceCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetDaynightCommandExecutor> GetDaynightCommandExecutor(BaseMemoryManager::Instance().New<SysGetDaynightCommandExecutor>() );
    if (NULL == GetDaynightCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetDaynightCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetDaynightCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetDaynightCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSetDaynightCommandExecutor> SetDaynightCommandExecutor(BaseMemoryManager::Instance().New<SysSetDaynightCommandExecutor>() );
    if (NULL == SetDaynightCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetDaynightCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SetDaynightCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SetDaynightCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysStartAudioDecodeCommandExecutor> StartAudioDecodeCommandExecutor(BaseMemoryManager::Instance().New<SysStartAudioDecodeCommandExecutor>());
    if (NULL == StartAudioDecodeCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetDaynightCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = StartAudioDecodeCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(StartAudioDecodeCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysStopAudioDecodeCommandExecutor> StopAudioDecodeCommandExecutor(BaseMemoryManager::Instance().New<SysStopAudioDecodeCommandExecutor>());
    if (NULL == StopAudioDecodeCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetDaynightCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = StopAudioDecodeCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(StopAudioDecodeCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetAudioEncodeCommandExecutor> GetAudioEncodeCommandExecutor(BaseMemoryManager::Instance().New<SysGetAudioEncodeCommandExecutor>());
    if (NULL == GetAudioEncodeCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetDaynightCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetAudioEncodeCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetAudioEncodeCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetPresetInfoCommandExecutor> GetPresetInfoCommandExecutor(BaseMemoryManager::Instance().New<SysGetPresetInfoCommandExecutor>());
    if (NULL == GetPresetInfoCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetDaynightCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetPresetInfoCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetPresetInfoCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSetPresetInfoCommandExecutor> SetPresetInfoCommandExecutor(BaseMemoryManager::Instance().New<SysSetPresetInfoCommandExecutor>());
    if (NULL == SetPresetInfoCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetDaynightCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SetPresetInfoCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SetPresetInfoCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysGetEncStreamCombineCommandExecutor> GetEncStreamCombineCommandExecutor(BaseMemoryManager::Instance().New<SysGetEncStreamCombineCommandExecutor>());
    if (NULL == GetEncStreamCombineCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetEncStreamCombineCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = GetEncStreamCombineCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(GetEncStreamCombineCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSetEncStreamCombineCommandExecutor> SetEncStreamCombineCommandExecutor(BaseMemoryManager::Instance().New<SysSetEncStreamCombineCommandExecutor>());
    if (NULL == SetEncStreamCombineCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetEncStreamCombineCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SetEncStreamCombineCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SetEncStreamCombineCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysExcuteImportConfigFileCommandExecutor> ExcuteImportConfigFileCommandExecutor(BaseMemoryManager::Instance().New<SysExcuteImportConfigFileCommandExecutor>());
    if (NULL == ExcuteImportConfigFileCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ExcuteImportConfigFileCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = ExcuteImportConfigFileCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(ExcuteImportConfigFileCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysSearchPresetCommandExecutor> SearchPresetCommandExecutor(BaseMemoryManager::Instance().New<SysSearchPresetCommandExecutor>());
    if (NULL == SearchPresetCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SearchPresetCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = SearchPresetCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(SearchPresetCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SafePtr<SysProcessNotSupportCommandExecutor> ProcessNotSupportCommandExecutor(BaseMemoryManager::Instance().New<SysProcessNotSupportCommandExecutor>() );
    if (NULL == ProcessNotSupportCommandExecutor.GetPtr())
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ProcessNotSupportCommandExecutor new fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    Result = ProcessNotSupportCommandExecutor->SetParameter(m_ServiceManager, NULL, 0);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetParameter fail, Result = 0x%lx\n", Result);
        return Result;
    }
    Result = m_CommandPipeline->RegisterCommandExecutor(ProcessNotSupportCommandExecutor);
    if (FAILED(Result))
    {
        SYS_ERROR("RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SYS_INFO("%s normal out..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SysCommandProcessor::Initialize(void_t *Argument)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    GMI_RESULT Result = GMI_SUCCESS;

    Result = ContextInitialize();
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ContextInitialize fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = RegisterCommand();
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommand fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = Start();
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Start fail, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


GMI_RESULT SysCommandProcessor::Deinitialize(void)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", __func__);
    GMI_RESULT Result = GMI_SUCCESS;

    Result = Stop();
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "stop fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = ContextDeinitialize();
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ContextDeinitialize fail, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal out..........\n", __func__);
    return GMI_SUCCESS;
}


