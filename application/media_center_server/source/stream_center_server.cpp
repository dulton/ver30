// media_center_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "application_packet.h"
#include "gmi_system_headers.h"
#include "ipc_fw_v3.x_resource.h"
#include "ipc_fw_v3.x_setting.h"
#include "log_client.h"
#include "media_center_service.h"

#include "media_center_open_vin_vout_device_command_executor.h"
#include "media_center_close_vin_vout_device_command_executor.h"
#include "media_center_get_vin_vout_configuration_command_executor.h"
#include "media_center_set_vin_vout_configuration_command_executor.h"

#include "media_center_create_codec_command_executor.h"
#include "media_center_destroy_codec_command_executor.h"
#include "media_center_start_codec_command_executor.h"
#include "media_center_stop_codec_command_executor.h"
#include "media_center_force_generate_idr_frame_command_executor.h"
#include "media_center_get_codec_configuration_command_executor.h"
#include "media_center_get_osd_configuration_command_executor.h"
#include "media_center_set_codec_configuration_command_executor.h"
#include "media_center_set_osd_configuration_command_executor.h"

#include "media_center_open_image_device_command_executor.h"
#include "media_center_close_image_device_command_executor.h"
#include "media_center_close_image_device_command_executor.h"
#include "media_center_get_base_image_configuration_command_executor.h"
#include "media_center_set_base_image_configuration_command_executor.h"
#include "media_center_get_advanced_image_configuration_command_executor.h"
#include "media_center_set_advanced_image_configuration_command_executor.h"
#include "media_center_get_auto_focus_configuration_command_executor.h"
#include "media_center_set_auto_focus_configuration_command_executor.h"
#include "media_center_get_daynight_configuration_command_executor.h"
#include "media_center_set_daynight_configuration_command_executor.h"
#include "media_center_get_white_balance_configuration_command_executor.h"
#include "media_center_set_white_balance_configuration_command_executor.h"

#include "media_center_open_auto_focus_device_command_executor.h"
#include "media_center_close_auto_focus_device_command_executor.h"
#include "media_center_start_auto_focus_command_executor.h"
#include "media_center_stop_auto_focus_command_executor.h"
#include "media_center_pause_auto_focus_command_executor.h"
#include "media_center_auto_focus_global_scan_command_executor.h"
#include "media_center_set_auto_focus_mode_command_executor.h"
#include "media_center_notify_auto_focus_command_executor.h"
#include "media_center_get_focus_position_command_executor.h"
#include "media_center_set_focus_position_command_executor.h"
#include "media_center_get_focus_range_command_executor.h"
#include "media_center_reset_focus_motor_command_executor.h"
#include "media_center_control_auto_focus_command_executor.h"
#include "media_center_set_auto_focus_step_command_executor.h"

#include "media_center_open_zoom_device_command_executor.h"
#include "media_center_close_zoom_device_command_executor.h"
#include "media_center_get_zoom_position_command_executor.h"
#include "media_center_set_zoom_position_command_executor.h"
#include "media_center_get_zoom_range_command_executor.h"
#include "media_center_reset_zoom_motor_command_executor.h"
#include "media_center_control_zoom_command_executor.h"
#include "media_center_set_zoom_step_command_executor.h"

#include "media_center_start_zoom_command_executor.h"
#include "media_center_stop_zoom_command_executor.h"

#if defined( __linux__ )
#include "gmi_config_api.h"
#include "gmi_daemon_heartbeat_api.h"
#endif

void_t* DaemonHeartbeatProc( void_t *Argument );
GMI_RESULT RegisterCommandExecutor( MediaCenterService& Service );
GMI_RESULT GetMediaCenterServerLogConfig( uint32_t *ModuleId, char_t *ModuleName, char_t *ModulePipeName, long_t *ModulePipeMutexId, char_t *PeerPipeName, long_t *PeerPipeMutexId, char_t *ServerPipeName, long_t *ServerPipeMutexId, uint32_t *DebugLogLevel );
GMI_RESULT GetMediaCenterServerAddress( uint32_t *Address );
GMI_RESULT GetMediaCenterServerPort( uint16_t *Port );
GMI_RESULT GetHeartbeatInterval( uint32_t *Interval );

static uint32_t l_Heartbeat_Interval = GMI_MEDIA_CENTER_SERVER_HEARTBEAT_INTERVAL;

#if defined( __linux__ )
void_t SIGPIPE_SignalProcess( int32_t SignalNumber )
{
    printf( "media_center_server: read side of pipe or socket is already close. SignalNumber=%d\n", SignalNumber );
}

void_t SignalProcess( int32_t SignalNumber )
{
    printf( "media_center_server: SignalNumber=%d\n", SignalNumber );
}
#endif

#define TEST_NO_HEARTBEAT_CASE           0
#define TEST_HEARTBEAT_COUNT             30
#define INIT_NETWORK                     1

#define LOOPBACK_IP                      "127.0.0.1"
#define DEFAULT_MEDIA_CENTER_SERVER_IP   "192.168.0.247"

#if INIT_NETWORK
class NetworkInitializer;
#endif

MediaCenterService  g_MediaCenter;

#if defined( _WIN32 )
int32_t _tmain( int32_t argc, _TCHAR* argv[] )
#elif defined( __linux__ )
int32_t main( int32_t argc, char_t* argv[] )
#endif
{
#if defined( __linux__ )
    signal( SIGPIPE , SIGPIPE_SignalProcess );
    //signal( SIGINT,  SignalProcess );
    //signal( SIGTERM, SignalProcess );
#endif

    uint32_t ModuleId = 0;
    char_t   ModuleName[MAX_PATH_LENGTH] = {0};
    char_t   ModulePipeName[MAX_PATH_LENGTH] = {0};
    long_t   ModulePipeMutexId = 0;
    char_t   PeerPipeName[MAX_PATH_LENGTH] = {0};
    long_t   PeerPipeMutexId = 0;
    char_t   ServerPipeName[MAX_PATH_LENGTH] = {0};
    long_t   ServerPipeMutexId = 0;
    uint32_t ServerDebugLogLevel = 0;

    GMI_RESULT Result = GetMediaCenterServerLogConfig( &ModuleId, ModuleName, ModulePipeName, &ModulePipeMutexId, PeerPipeName, &PeerPipeMutexId, ServerPipeName, &ServerPipeMutexId, &ServerDebugLogLevel );
    if ( FAILED( Result ) )
    {
        printf( "Media Center get log client config error \n" );
        return Result;
    }

    LogClient Client;
    Result = Client.Initialize( ModuleId, ModuleName, ModulePipeName, ModulePipeMutexId, PeerPipeName, PeerPipeMutexId, ServerPipeName, ServerPipeMutexId, GMI_IPC_LOG_FILE_PATH, ServerDebugLogLevel );
    if ( FAILED( Result ) )
    {
        printf( "Media Center log client initialization error \n" );
        return Result;
    }

    // log operation is low speed for now, to test other function, we comment it out
    g_DefaultLogClient = &Client;

    //USER_LOG( &Client, e_UserLogType_Operation, 0, "TestUserName", 12, USER_LOG_TEST_STRING, (uint32_t) strlen(USER_LOG_TEST_STRING) );
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, " Media Center Server starting... \n" );

    uint32_t MediaCenterServerAddress = 0;
    Result = GetMediaCenterServerAddress( &MediaCenterServerAddress );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "get media center server address fail, Result=%x \n", (uint32_t) Result );
        Client.Deinitialize();
        return Result;
    }

    uint16_t MediaCenterServerPort = 0;
    Result = GetMediaCenterServerPort( &MediaCenterServerPort );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "get media center server port fail, Result=%x \n", (uint32_t) Result );
        Client.Deinitialize();
        return Result;
    }

    SafePtr<NetworkInitializer> Initializer( BaseMemoryManager::Instance().New<NetworkInitializer>() );
    if ( NULL == Initializer.GetPtr() )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "Initialize network failed \n" );
        Client.Deinitialize();
        return GMI_OUT_OF_MEMORY;
    }

    Result = g_MediaCenter.Initialize( MediaCenterServerAddress, MediaCenterServerPort, UDP_SESSION_BUFFER_SIZE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenter initialize failed, Result=%x \n", (uint32_t) Result );
        Client.Deinitialize();
        return Result;
    }

    Result = RegisterCommandExecutor( g_MediaCenter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "RegisterCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        Client.Deinitialize();
        return Result;
    }

    Result = g_MediaCenter.Run( false );
    if ( FAILED( Result ) )
    {
        g_MediaCenter.Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenter Run failed, Result=%x \n", (uint32_t) Result );
        Client.Deinitialize();
        return Result;
    }

    Result = GetHeartbeatInterval( &l_Heartbeat_Interval );
    if ( FAILED( Result ) )
    {
        g_MediaCenter.Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenter GetHeartbeatInterval failed, Result=%x \n", (uint32_t) Result );
        Client.Deinitialize();
        return Result;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, " Media Center Server started \n" );
    DaemonHeartbeatProc( &g_MediaCenter );

    Result = g_MediaCenter.Deinitialize();
    if ( FAILED( Result ) )
    {
        g_MediaCenter.Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenter Deinitialize failed, Result=%x \n", (uint32_t) Result );
        Client.Deinitialize();
        return Result;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, " Media Center Server exit... " );
    g_DefaultLogClient = NULL;
    Client.Deinitialize();
    printf( "media center server quit because receiving a signal from daemon \n" );

    return 0;
}

void* DaemonHeartbeatProc( void *Argument )
{
#if defined( __linux__ )
    uint32_t Flags = APPLICATION_RUNNING;

    DAEMON_DATA_CFG DaemonData;
    GMI_RESULT Result = GMI_DaemonInit( &DaemonData, MEDIA_SERVER_ID, GMI_DAEMON_HEARDBEAT_SERVER, GMI_DAEMON_HEARDBEAT_MEDIA );
    if ( FAILED( Result ) )
    {
        printf( "media center server initialization fail \n" );
        return (void_t*) GMI_FAIL;
    }

    do
    {
        GMI_RESULT Result = GMI_DaemonRegister(&DaemonData);
        if ( SUCCEEDED( Result ) )
        {
            break;
        }
        GMI_Sleep( 10 );
    }
    while ( 1 );
#endif

#if TEST_NO_HEARTBEAT_CASE
    long_t Count = TEST_HEARTBEAT_COUNT;
#endif

    while ( 1 )
    {
#if defined( __linux__ )
#if TEST_NO_HEARTBEAT_CASE
        if ( 0 < Count )
        {
            GMI_DaemonReport(&DaemonData, &Flags );
            --Count;
            if (APPLICATION_QUIT == Flags)
            {
                break;
            }
        }
#else
        GMI_DaemonReport(&DaemonData, &Flags );
        if (APPLICATION_QUIT == Flags)
        {
            break;
        }
#endif
#endif
        GMI_Sleep( l_Heartbeat_Interval );
    }

#if defined( __linux__ )
    GMI_DaemonUnInit(&DaemonData);
#endif
    return (void_t*) GMI_SUCCESS;
}

GMI_RESULT RegisterCommandExecutor( MediaCenterService& Service )
{
    SafePtr<MediaCenterOpenVinVoutDeviceCommandExecutor> OpenVinVoutDeviceCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterOpenVinVoutDeviceCommandExecutor>() );
    if ( NULL == OpenVinVoutDeviceCommandExecutor.GetPtr() )
    {
        printf( "OpenVinVoutDeviceCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterCloseVinVoutDeviceCommandExecutor> CloseVinVoutDeviceCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterCloseVinVoutDeviceCommandExecutor>() );
    if ( NULL == CloseVinVoutDeviceCommandExecutor.GetPtr() )
    {
        printf( "CloseVinVoutDeviceCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterGetVinVoutConfigurationCommandExecutor> GetVinVoutConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetVinVoutConfigurationCommandExecutor>() );
    if ( NULL == GetVinVoutConfigurationCommandExecutor.GetPtr() )
    {
        printf( "GetVinVoutConfigurationCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterSetVinVoutConfigurationCommandExecutor> SetVinVoutConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetVinVoutConfigurationCommandExecutor>() );
    if ( NULL == SetVinVoutConfigurationCommandExecutor.GetPtr() )
    {
        printf( "SetVinVoutConfigurationCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterCreateCodecCommandExecutor> CreateCodecCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterCreateCodecCommandExecutor>() );
    if ( NULL == CreateCodecCommandExecutor.GetPtr() )
    {
        printf( "CreateCodecCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterDestroyCodecCommandExecutor> DestroyCodecCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterDestroyCodecCommandExecutor>() );
    if ( NULL == DestroyCodecCommandExecutor.GetPtr() )
    {
        printf( "DestroyCodecCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterStartCodecCommandExecutor> StartCodecCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterStartCodecCommandExecutor>() );
    if ( NULL == StartCodecCommandExecutor.GetPtr() )
    {
        printf( "StartCodecCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterStopCodecCommandExecutor> StopCodecCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterStopCodecCommandExecutor>() );
    if ( NULL == StopCodecCommandExecutor.GetPtr() )
    {
        printf( "StopCodecCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterGetCodecConfigurationCommandExecutor> GetCodecConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetCodecConfigurationCommandExecutor>() );
    if ( NULL == GetCodecConfigurationCommandExecutor.GetPtr() )
    {
        printf( "GetCodecConfigurationCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterSetCodecConfigurationCommandExecutor> SetCodecConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetCodecConfigurationCommandExecutor>() );
    if ( NULL == SetCodecConfigurationCommandExecutor.GetPtr() )
    {
        printf( "SetCodecConfigurationCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterGetOsdConfigurationCommandExecutor> GetOsdConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetOsdConfigurationCommandExecutor>() );
    if ( NULL == GetOsdConfigurationCommandExecutor.GetPtr() )
    {
        printf( "GetOsdConfigurationCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterSetOsdConfigurationCommandExecutor> SetOsdConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetOsdConfigurationCommandExecutor>() );
    if ( NULL == SetOsdConfigurationCommandExecutor.GetPtr() )
    {
        printf( "SetOsdConfigurationCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterForceGenerateIdrFrameCommandExecutor> ForceGenerateIdrFrameCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterForceGenerateIdrFrameCommandExecutor>() );
    if ( NULL == ForceGenerateIdrFrameCommandExecutor.GetPtr() )
    {
        printf( "ForceGenerateIdrFrameCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterOpenImageDeviceCommandExecutor> OpenImageDeviceCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterOpenImageDeviceCommandExecutor>() );
    if ( NULL == OpenImageDeviceCommandExecutor.GetPtr() )
    {
        printf( "OpenImageDeviceCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterCloseImageDeviceCommandExecutor> CloseImageDeviceCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterCloseImageDeviceCommandExecutor>() );
    if ( NULL == CloseImageDeviceCommandExecutor.GetPtr() )
    {
        printf( "CloseImageDeviceCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterGetBaseImageConfigurationCommandExecutor> GetBaseImageConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetBaseImageConfigurationCommandExecutor>() );
    if ( NULL == GetBaseImageConfigurationCommandExecutor.GetPtr() )
    {
        printf( "GetBaseImageConfigurationCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterSetBaseImageConfigurationCommandExecutor> SetBaseImageConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetBaseImageConfigurationCommandExecutor>() );
    if ( NULL == SetBaseImageConfigurationCommandExecutor.GetPtr() )
    {
        printf( "SetBaseImageConfigurationCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterGetAdvancedImageConfigurationCommandExecutor> GetAdvancedImageConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetAdvancedImageConfigurationCommandExecutor>() );
    if ( NULL == GetAdvancedImageConfigurationCommandExecutor.GetPtr() )
    {
        printf( "GetAdvancedImageConfigurationCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterSetAdvancedImageConfigurationCommandExecutor> SetAdvancedImageConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetAdvancedImageConfigurationCommandExecutor>() );
    if ( NULL == SetAdvancedImageConfigurationCommandExecutor.GetPtr() )
    {
        printf( "SetAdvancedImageConfigurationCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterGetAutoFocusConfigurationCommandExecutor> GetAutoFocusConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetAutoFocusConfigurationCommandExecutor>() );
    if ( NULL == GetAutoFocusConfigurationCommandExecutor.GetPtr() )
    {
        printf( "GetAutoFocusConfigurationCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterSetAutoFocusConfigurationCommandExecutor> SetAutoFocusConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetAutoFocusConfigurationCommandExecutor>() );
    if ( NULL == SetAutoFocusConfigurationCommandExecutor.GetPtr() )
    {
        printf( "SetAutoFocusConfigurationCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterGetDaynightConfigurationCommandExecutor> GetDaynightConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetDaynightConfigurationCommandExecutor>() );
    if ( NULL == GetDaynightConfigurationCommandExecutor.GetPtr() )
    {
        printf( "GetDaynightConfigurationCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterSetDaynightConfigurationCommandExecutor> SetDaynightConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetDaynightConfigurationCommandExecutor>() );
    if ( NULL == SetDaynightConfigurationCommandExecutor.GetPtr() )
    {
        printf( "SetDaynightConfigurationCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterGetWhiteBalanceConfigurationCommandExecutor> GetWhiteBalanceConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetWhiteBalanceConfigurationCommandExecutor>() );
    if ( NULL == GetWhiteBalanceConfigurationCommandExecutor.GetPtr() )
    {
        printf( "GetWhiteBalanceConfigurationCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterSetWhiteBalanceConfigurationCommandExecutor> SetWhiteBalanceConfigurationCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetWhiteBalanceConfigurationCommandExecutor>() );
    if ( NULL == SetWhiteBalanceConfigurationCommandExecutor.GetPtr() )
    {
        printf( "SetWhiteBalanceConfigurationCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterOpenAutoFocusDeviceCommandExecutor> OpenAutoFocusDeviceCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterOpenAutoFocusDeviceCommandExecutor>() );
    if ( NULL == OpenAutoFocusDeviceCommandExecutor.GetPtr() )
    {
        printf( "OpenAutoFocusDeviceCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterCloseAutoFocusDeviceCommandExecutor> CloseAutoFocusDeviceCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterCloseAutoFocusDeviceCommandExecutor>() );
    if ( NULL == CloseAutoFocusDeviceCommandExecutor.GetPtr() )
    {
        printf( "CloseAutoFocusDeviceCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterStartAutoFocusCommandExecutor> StartAutoFocusCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterStartAutoFocusCommandExecutor>() );
    if ( NULL == StartAutoFocusCommandExecutor.GetPtr() )
    {
        printf( "StartAutoFocusCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterStopAutoFocusCommandExecutor> StopAutoFocusCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterStopAutoFocusCommandExecutor>() );
    if ( NULL == StopAutoFocusCommandExecutor.GetPtr() )
    {
        printf( "StopAutoFocusCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterPauseAutoFocusCommandExecutor> PauseAutoFocusCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterPauseAutoFocusCommandExecutor>() );
    if ( NULL == PauseAutoFocusCommandExecutor.GetPtr() )
    {
        printf( "PauseAutoFocusCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterAutoFocusGlobalScanCommandExecutor> AutoFocusGlobalScanCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterAutoFocusGlobalScanCommandExecutor>() );
    if ( NULL == AutoFocusGlobalScanCommandExecutor.GetPtr() )
    {
        printf( "AutoFocusGlobalScanCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterSetAutoFocusModeCommandExecutor> SetAutoFocusModeCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetAutoFocusModeCommandExecutor>() );
    if ( NULL == SetAutoFocusModeCommandExecutor.GetPtr() )
    {
        printf( "SetAutoFocusModeCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterNotifyAutoFocusCommandExecutor> NotifyAutoFocusCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterNotifyAutoFocusCommandExecutor>() );
    if ( NULL == NotifyAutoFocusCommandExecutor.GetPtr() )
    {
        printf( "NotifyAutoFocusCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterGetFocusPositionCommandExecutor> GetFocusPositionCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetFocusPositionCommandExecutor>() );
    if ( NULL == GetFocusPositionCommandExecutor.GetPtr() )
    {
        printf( "GetFocusPositionCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterSetFocusPositionCommandExecutor> SetFocusPositionCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetFocusPositionCommandExecutor>() );
    if ( NULL == SetFocusPositionCommandExecutor.GetPtr() )
    {
        printf( "SetFocusPositionCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterGetFocusRangeCommandExecutor> GetFocusRangeCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetFocusRangeCommandExecutor>() );
    if ( NULL == GetFocusRangeCommandExecutor.GetPtr() )
    {
        printf( "GetFocusRangeCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterResetFocusMotorCommandExecutor> ResetFocusMotorCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterResetFocusMotorCommandExecutor>() );
    if ( NULL == ResetFocusMotorCommandExecutor.GetPtr() )
    {
        printf( "ResetFocusMotorCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterControlAutoFocusCommandExecutor> ControlAutoFocusCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterControlAutoFocusCommandExecutor>() );
    if ( NULL == ControlAutoFocusCommandExecutor.GetPtr() )
    {
        printf( "ControlAutoFocusCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterSetAutoFocusStepCommandExecutor> SetAutoFocusStepCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetAutoFocusStepCommandExecutor>() );
    if ( NULL == SetAutoFocusStepCommandExecutor.GetPtr() )
    {
        printf( "SetAutoFocusStepCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterOpenZoomDeviceCommandExecutor> OpenZoomDeviceCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterOpenZoomDeviceCommandExecutor>() );
    if ( NULL == OpenZoomDeviceCommandExecutor.GetPtr() )
    {
        printf( "OpenZoomDeviceCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterCloseZoomDeviceCommandExecutor> CloseZoomDeviceCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterCloseZoomDeviceCommandExecutor>() );
    if ( NULL == CloseZoomDeviceCommandExecutor.GetPtr() )
    {
        printf( "CloseZoomDeviceCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterGetZoomPositionCommandExecutor> GetZoomPositionCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetZoomPositionCommandExecutor>() );
    if ( NULL == GetZoomPositionCommandExecutor.GetPtr() )
    {
        printf( "GetZoomPositionCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterSetZoomPositionCommandExecutor> SetZoomPositionCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetZoomPositionCommandExecutor>() );
    if ( NULL == SetZoomPositionCommandExecutor.GetPtr() )
    {
        printf( "SetZoomPositionCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterGetZoomRangeCommandExecutor> GetZoomRangeCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetZoomRangeCommandExecutor>() );
    if ( NULL == GetZoomRangeCommandExecutor.GetPtr() )
    {
        printf( "GetZoomRangeCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterResetZoomMotorCommandExecutor> ResetZoomMotorCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterResetZoomMotorCommandExecutor>() );
    if ( NULL == ResetZoomMotorCommandExecutor.GetPtr() )
    {
        printf( "ResetZoomMotorCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterControlZoomCommandExecutor> ControlZoomCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterControlZoomCommandExecutor>() );
    if ( NULL == ControlZoomCommandExecutor.GetPtr() )
    {
        printf( "ControlZoomCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterSetZoomStepCommandExecutor> SetZoomStepCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetZoomStepCommandExecutor>() );
    if ( NULL == SetZoomStepCommandExecutor.GetPtr() )
    {
        printf( "SetZoomStepCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterStartZoomCommandExecutor> StartZoomCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterStartZoomCommandExecutor>() );
    if ( NULL == StartZoomCommandExecutor.GetPtr() )
    {
        printf( "StartZoomCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    SafePtr<MediaCenterStopZoomCommandExecutor> StopZoomCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterStopZoomCommandExecutor>() );
    if ( NULL == StopZoomCommandExecutor.GetPtr() )
    {
        printf( "StopZoomCommandExecutor failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    OpenVinVoutDeviceCommandExecutor->SetParameter( Service.GetMediaCenter() );
    CloseVinVoutDeviceCommandExecutor->SetParameter ( Service.GetMediaCenter() );
    GetVinVoutConfigurationCommandExecutor->SetParameter( Service.GetMediaCenter() );
    SetVinVoutConfigurationCommandExecutor->SetParameter( Service.GetMediaCenter() );

    CreateCodecCommandExecutor->SetParameter( Service.GetMediaCenter() );
    DestroyCodecCommandExecutor->SetParameter ( Service.GetMediaCenter() );
    StartCodecCommandExecutor->SetParameter( Service.GetMediaCenter() );
    StopCodecCommandExecutor->SetParameter ( Service.GetMediaCenter() );
    GetCodecConfigurationCommandExecutor->SetParameter( Service.GetMediaCenter() );
    SetCodecConfigurationCommandExecutor->SetParameter( Service.GetMediaCenter() );
    GetOsdConfigurationCommandExecutor->SetParameter( Service.GetMediaCenter() );
    SetOsdConfigurationCommandExecutor->SetParameter( Service.GetMediaCenter() );
    ForceGenerateIdrFrameCommandExecutor->SetParameter( Service.GetMediaCenter() );

    OpenImageDeviceCommandExecutor->SetParameter( Service.GetMediaCenter() );
    CloseImageDeviceCommandExecutor->SetParameter( Service.GetMediaCenter() );
    GetBaseImageConfigurationCommandExecutor->SetParameter( Service.GetMediaCenter() );
    SetBaseImageConfigurationCommandExecutor->SetParameter( Service.GetMediaCenter() );
    GetAdvancedImageConfigurationCommandExecutor->SetParameter( Service.GetMediaCenter() );
    SetAdvancedImageConfigurationCommandExecutor->SetParameter( Service.GetMediaCenter() );
    GetAutoFocusConfigurationCommandExecutor->SetParameter( Service.GetMediaCenter() );
    SetAutoFocusConfigurationCommandExecutor->SetParameter( Service.GetMediaCenter() );
    GetDaynightConfigurationCommandExecutor->SetParameter( Service.GetMediaCenter() );
    SetDaynightConfigurationCommandExecutor->SetParameter( Service.GetMediaCenter() );
    GetWhiteBalanceConfigurationCommandExecutor->SetParameter( Service.GetMediaCenter() );
    SetWhiteBalanceConfigurationCommandExecutor->SetParameter( Service.GetMediaCenter() );

    OpenAutoFocusDeviceCommandExecutor->SetParameter( Service.GetMediaCenter() );
    CloseAutoFocusDeviceCommandExecutor->SetParameter( Service.GetMediaCenter() );
    StartAutoFocusCommandExecutor->SetParameter( Service.GetMediaCenter() );
    StopAutoFocusCommandExecutor->SetParameter( Service.GetMediaCenter() );
    PauseAutoFocusCommandExecutor->SetParameter( Service.GetMediaCenter() );
    AutoFocusGlobalScanCommandExecutor->SetParameter( Service.GetMediaCenter() );
    SetAutoFocusModeCommandExecutor->SetParameter( Service.GetMediaCenter() );
    NotifyAutoFocusCommandExecutor->SetParameter( Service.GetMediaCenter() );
    GetFocusPositionCommandExecutor->SetParameter( Service.GetMediaCenter() );
    SetFocusPositionCommandExecutor->SetParameter( Service.GetMediaCenter() );
    GetFocusRangeCommandExecutor->SetParameter( Service.GetMediaCenter() );
    ResetFocusMotorCommandExecutor->SetParameter( Service.GetMediaCenter() );
    ControlAutoFocusCommandExecutor->SetParameter( Service.GetMediaCenter() );
    SetAutoFocusStepCommandExecutor->SetParameter( Service.GetMediaCenter() );

    OpenZoomDeviceCommandExecutor->SetParameter( Service.GetMediaCenter() );
    CloseZoomDeviceCommandExecutor->SetParameter( Service.GetMediaCenter() );
    GetZoomPositionCommandExecutor->SetParameter( Service.GetMediaCenter() );
    SetZoomPositionCommandExecutor->SetParameter( Service.GetMediaCenter() );
    GetZoomRangeCommandExecutor->SetParameter( Service.GetMediaCenter() );
    ResetZoomMotorCommandExecutor->SetParameter( Service.GetMediaCenter() );
    ControlZoomCommandExecutor->SetParameter( Service.GetMediaCenter() );
    SetZoomStepCommandExecutor->SetParameter( Service.GetMediaCenter() );

    StartZoomCommandExecutor->SetParameter( Service.GetMediaCenter() );
    StopZoomCommandExecutor->SetParameter( Service.GetMediaCenter() );

    GMI_RESULT Result = Service.RegisterCommandExecutor( OpenVinVoutDeviceCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor OpenVinVoutDeviceCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( CloseVinVoutDeviceCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor CloseVinVoutDeviceCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( GetVinVoutConfigurationCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor GetVinVoutConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( SetVinVoutConfigurationCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor SetVinVoutConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( CreateCodecCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor CreateCodecCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( DestroyCodecCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor DestroyCodecCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( StartCodecCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor StartCodecCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( StopCodecCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor StopCodecCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( GetCodecConfigurationCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor GetCodecConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( SetCodecConfigurationCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor SetCodecConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( GetOsdConfigurationCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor GetOsdConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( SetOsdConfigurationCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor SetOsdConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( ForceGenerateIdrFrameCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor SetOsdConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( OpenImageDeviceCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor OpenImageDeviceCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( CloseImageDeviceCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor CloseImageDeviceCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( GetBaseImageConfigurationCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor GetBaseImageConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( SetBaseImageConfigurationCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor SetBaseImageConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( GetAdvancedImageConfigurationCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor GetAdvancedImageConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( SetAdvancedImageConfigurationCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor SetAdvancedImageConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( GetAutoFocusConfigurationCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor GetAutoFocusConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( SetAutoFocusConfigurationCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor SetAutoFocusConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( GetDaynightConfigurationCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor GetDaynightConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( SetDaynightConfigurationCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor SetDaynightConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( GetWhiteBalanceConfigurationCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor GetWhiteBalanceConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( SetWhiteBalanceConfigurationCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor SetWhiteBalanceConfigurationCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( OpenAutoFocusDeviceCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor OpenAutoFocusDeviceCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( CloseAutoFocusDeviceCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor CloseAutoFocusDeviceCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( StartAutoFocusCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor StartAutoFocusCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( StopAutoFocusCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor StopAutoFocusCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( PauseAutoFocusCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor PauseAutoFocusCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( AutoFocusGlobalScanCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor AutoFocusGlobalScanCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( SetAutoFocusModeCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor SetAutoFocusModeCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( NotifyAutoFocusCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor NotifyAutoFocusCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( GetFocusPositionCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor GetFocusPositionCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( SetFocusPositionCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor SetFocusPositionCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( GetFocusRangeCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor GetFocusRangeCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( ResetFocusMotorCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor ResetFocusMotorCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( ControlAutoFocusCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor ControlAutoFocusCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( SetAutoFocusStepCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor SetAutoFocusStepCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( OpenZoomDeviceCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor OpenZoomDeviceCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( CloseZoomDeviceCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor CloseZoomDeviceCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( GetZoomPositionCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor GetZoomPositionCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( SetZoomPositionCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor SetZoomPositionCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( GetZoomRangeCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor GetZoomRangeCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( ResetZoomMotorCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor ResetZoomMotorCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( ControlZoomCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor ControlZoomCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( SetZoomStepCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor SetZoomStepCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( StartZoomCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor StartZoomCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = Service.RegisterCommandExecutor( StopZoomCommandExecutor );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter RegisterCommandExecutor StopZoomCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    return GMI_SUCCESS;
}

#define MEDIA_CENTER_SERVER_CONFIG_PATH                "/Config/media_center_server/"
#define MEDIA_CENTER_SERVER_CONFIG_HEARTBEAT_INTERVAL  "heartbeat_interval"
#define MEDIA_CENTER_SERVER_CONFIG_SERVER_ADDRESS      "server_address"
#define MEDIA_CENTER_SERVER_CONFIG_SERVER_PORT         "command_port"

#define MEDIA_CENTER_SERVER_CONFIG_LOG_CLIENT_PIPE_NAME      "log_client_pipe_name"
#define MEDIA_CENTER_SERVER_CONFIG_LOG_CLIENT_PIPE_MUTEX_ID  "log_client_pipe_mutex_id"
#define MEDIA_CENTER_SERVER_CONFIG_LOG_PEER_PIPE_NAME        "log_peer_pipe_name"
#define MEDIA_CENTER_SERVER_CONFIG_LOG_PEER_PIPE_MUTEX_ID    "log_peer_pipe_mutex_id"
#define MEDIA_CENTER_SERVER_CONFIG_LOG_SERVER_PIPE_NAME      "log_server_pipe_name"
#define MEDIA_CENTER_SERVER_CONFIG_LOG_SERVER_PIPE_MUTEX_ID  "log_server_pipe_mutex_id"
#define MEDIA_CENTER_SERVER_CONFIG_DEBUG_LOG_LEVEL           "debug_log_level"

GMI_RESULT GetMediaCenterServerLogConfig( uint32_t *ModuleId, char_t *ModuleName, char_t *ModulePipeName, long_t *ModulePipeMutexId, char_t *PeerPipeName, long_t *PeerPipeMutexId, char_t *ServerPipeName, long_t *ServerPipeMutexId, uint32_t *DebugLogLevel )
{
    *ModuleId = GMI_LOG_MODULE_MEDIA_CENTER_SERVER_ID;
#if defined( __linux__ )
    strcpy( ModuleName, GMI_LOG_MODULE_MEDIA_CENTER_SERVER_NAME );

    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "media center server, GetMediaCenterClientPipeName, Default_MediaCenterClientPipeName=%s \n", LOG_MEDIA_CENTER_DEFAULT_CLIENT_PIPE_NAME );
    Result = GMI_XmlRead(Handle, MEDIA_CENTER_SERVER_CONFIG_PATH, MEDIA_CENTER_SERVER_CONFIG_LOG_CLIENT_PIPE_NAME, LOG_MEDIA_CENTER_DEFAULT_CLIENT_PIPE_NAME, ModulePipeName, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }
    printf( "media center server, GetMediaCenterClientPipeName, Default_MediaCenterClientPipeName=%s, Config_MediaCenterClientPipeName=%s \n", LOG_MEDIA_CENTER_DEFAULT_CLIENT_PIPE_NAME, ModulePipeName );

    printf( "media center server, GetMediaCenterClientPipeMutexId, Default_MediaCenterClientPipeMutexId=%d \n", LOG_MEDIA_CENTER_DEFAULT_CLIENT_PIPE_MUTEX_ID );
    Result = GMI_XmlRead(Handle, MEDIA_CENTER_SERVER_CONFIG_PATH, MEDIA_CENTER_SERVER_CONFIG_LOG_CLIENT_PIPE_MUTEX_ID, LOG_MEDIA_CENTER_DEFAULT_CLIENT_PIPE_MUTEX_ID, (int32_t *) ModulePipeMutexId, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }
    printf( "media center server, GetMediaCenterClientPipeMutexId, Default_MediaCenterClientPipeMutexId=%d, Config_MediaCenterClientPipeMutexId=%ld \n", LOG_MEDIA_CENTER_DEFAULT_CLIENT_PIPE_MUTEX_ID, *ModulePipeMutexId );


    printf( "media center server, GetMediaCenterPeerPipeName, Default_MediaCenterPeerPipeName=%s \n", LOG_MEDIA_CENTER_DEFAULT_PEER_PIPE_NAME );
    Result = GMI_XmlRead(Handle, MEDIA_CENTER_SERVER_CONFIG_PATH, MEDIA_CENTER_SERVER_CONFIG_LOG_PEER_PIPE_NAME, LOG_MEDIA_CENTER_DEFAULT_PEER_PIPE_NAME, PeerPipeName, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }
    printf( "media center server, GetMediaCenterPeerPipeName, Default_MediaCenterPeerPipeName=%s, Config_MediaCenterPeerPipeName=%s \n", LOG_MEDIA_CENTER_DEFAULT_PEER_PIPE_NAME, PeerPipeName );

    printf( "media center server, GetMediaCenterClientPipeMutexId, Default_MediaCenterClientPipeMutexId=%d \n", LOG_MEDIA_CENTER_DEFAULT_PEER_PIPE_MUTEX_ID );
    Result = GMI_XmlRead(Handle, MEDIA_CENTER_SERVER_CONFIG_PATH, MEDIA_CENTER_SERVER_CONFIG_LOG_PEER_PIPE_MUTEX_ID, LOG_MEDIA_CENTER_DEFAULT_PEER_PIPE_MUTEX_ID, (int32_t *) PeerPipeMutexId, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }
    printf( "media center server, GetMediaCenterPeerPipeMutexId, Default_MediaCenterPeerPipeMutexId=%d, Config_MediaCenterPeerPipeMutexId=%ld \n", LOG_MEDIA_CENTER_DEFAULT_PEER_PIPE_MUTEX_ID, *PeerPipeMutexId );


    printf( "media center server, GetMediaCenterServerPipeName, Default_MediaCenterServerPipeName=%s \n", LOG_SERVER_DEFAULT_PIPE_NAME );
    Result = GMI_XmlRead(Handle, MEDIA_CENTER_SERVER_CONFIG_PATH, MEDIA_CENTER_SERVER_CONFIG_LOG_SERVER_PIPE_NAME, LOG_SERVER_DEFAULT_PIPE_NAME, ServerPipeName, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }
    printf( "media center server, GetMediaCenterServerPipeName, Default_MediaCenterServerPipeName=%s, Config_MediaCenterServerPipeName=%s \n", LOG_SERVER_DEFAULT_PIPE_NAME, ServerPipeName );

    printf( "media center server, GetMediaCenterServerPipeMutexId, Default_MediaCenterServerPipeMutexId=%d \n", LOG_SERVER_DEFAULT_PIPE_MUTEX_ID );
    Result = GMI_XmlRead(Handle, MEDIA_CENTER_SERVER_CONFIG_PATH, MEDIA_CENTER_SERVER_CONFIG_LOG_SERVER_PIPE_MUTEX_ID, LOG_SERVER_DEFAULT_PIPE_MUTEX_ID, (int32_t *) ServerPipeMutexId, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }
    printf( "media center server, GetMediaCenterServerPipeMutexId, Default_MediaCenterServerPipeMutexId=%d, Config_MediaCenterServerPipeMutexId=%ld \n", LOG_SERVER_DEFAULT_PIPE_MUTEX_ID, *ServerPipeMutexId );

    printf( "media center server, GetMediaCenterLogLevel, Default_MediaCenterLogLevel=%d \n", GMI_LOG_MODULE_MEDIA_CENTER_SERVER_DEBUG_LOG_LEVEL );
    Result = GMI_XmlRead(Handle, MEDIA_CENTER_SERVER_CONFIG_PATH, MEDIA_CENTER_SERVER_CONFIG_DEBUG_LOG_LEVEL, GMI_LOG_MODULE_MEDIA_CENTER_SERVER_DEBUG_LOG_LEVEL, (int32_t *) DebugLogLevel, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }
    printf( "media center server, GetMediaCenterLogLevel, Default_MediaCenterLogLevel=%d, Config_MediaCenterLogLevel=%d \n", GMI_LOG_MODULE_MEDIA_CENTER_SERVER_DEBUG_LOG_LEVEL, *DebugLogLevel );

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

#elif defined( _WIN32 )
    strcpy_s( ModuleName,     MAX_PATH_LENGTH, GMI_LOG_MODULE_MEDIA_CENTER_SERVER_NAME );

    strcpy_s( ModulePipeName, MAX_PATH_LENGTH, LOG_MEDIA_CENTER_DEFAULT_CLIENT_PIPE_NAME );
    *ModulePipeMutexId = LOG_MEDIA_CENTER_DEFAULT_CLIENT_PIPE_MUTEX_ID;

    strcpy_s( PeerPipeName, MAX_PATH_LENGTH, LOG_MEDIA_CENTER_DEFAULT_PEER_PIPE_NAME );
    *PeerPipeMutexId = LOG_MEDIA_CENTER_DEFAULT_PEER_PIPE_MUTEX_ID;

    strcpy_s( ServerPipeName, MAX_PATH_LENGTH, LOG_SERVER_DEFAULT_PIPE_NAME );
    *ServerPipeMutexId = LOG_SERVER_DEFAULT_PIPE_MUTEX_ID;
    *DebugLogLevel = GMI_LOG_MODULE_MEDIA_CENTER_SERVER_DEBUG_LOG_LEVEL;
#endif
    return GMI_SUCCESS;
}

GMI_RESULT GetHeartbeatInterval( uint32_t *Interval )
{
#if defined( __linux__ )

    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "media center server, GetHeartbeatInterval, Default_Interval=%d \n", GMI_MEDIA_CENTER_SERVER_HEARTBEAT_INTERVAL );

    Result = GMI_XmlRead(Handle, MEDIA_CENTER_SERVER_CONFIG_PATH, MEDIA_CENTER_SERVER_CONFIG_HEARTBEAT_INTERVAL, GMI_MEDIA_CENTER_SERVER_HEARTBEAT_INTERVAL, (int32_t *) Interval, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    printf( "media center server, GetHeartbeatInterval, Default_Interval=%d, Interval=%d \n", GMI_MEDIA_CENTER_SERVER_HEARTBEAT_INTERVAL, *Interval );

#elif defined( _WIN32 )
    *Interval = GMI_MEDIA_CENTER_SERVER_HEARTBEAT_INTERVAL;
#endif
    return GMI_SUCCESS;
}

GMI_RESULT GetMediaCenterServerAddress( uint32_t *Address )
{
#if defined( __linux__ )

    // upgrading server use "0.0.0.0" as upgrading server socket address, we can be the same as it, use 0 directly as log server address
    *Address = inet_addr(LOOPBACK_IP);

    printf( "GetMediaCenterServerAddress, Address=%d \n", *Address );

#elif defined( _WIN32 )
    const size_t ServerIPLength = 128;
    char_t   ServerIP[ServerIPLength];
    memset( ServerIP, 0, ServerIPLength );

    printf( "please input media center server IP, for example: %s\n", DEFAULT_MEDIA_CENTER_SERVER_IP );
#if defined( __linux__ )
    int32_t ScanfResult = scanf( "%s", ServerIP );
#elif defined( _WIN32 )
    int32_t ScanfResult = scanf_s( "%s", ServerIP, ServerIPLength );
#endif
    READ_MYSELF( ScanfResult );

    *Address = inet_addr(ServerIP);
#endif
    return GMI_SUCCESS;
}

GMI_RESULT GetMediaCenterServerPort( uint16_t *Port )
{
#if defined( __linux__ )

    int32_t ServerPort = 0;

    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        printf( "GetMediaCenterServerPort, GMI_XmlOpen, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( "GetMediaCenterServerPort, Default_Port=%d \n", GMI_MEDIA_CENTER_SERVER_COMMAND_PORT );

    Result = GMI_XmlRead(Handle, MEDIA_CENTER_SERVER_CONFIG_PATH, MEDIA_CENTER_SERVER_CONFIG_SERVER_PORT, GMI_MEDIA_CENTER_SERVER_COMMAND_PORT, &ServerPort, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        printf( "GetMediaCenterServerPort, GMI_XmlRead, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        printf( "GetMediaCenterServerPort, GMI_XmlFileSave, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( "GetMediaCenterServerPort, Default_Port=%d, ServerPort=%d \n", GMI_MEDIA_CENTER_SERVER_COMMAND_PORT, ServerPort );

    *Port = htons((uint16_t)ServerPort);
#elif defined( _WIN32 )
    uint32_t ServerPort = 0;

    printf( "please input media center server port, for example: %d\n", GMI_MEDIA_CENTER_SERVER_COMMAND_PORT );
#if defined( __linux__ )
    int32_t ScanfResult = scanf( "%d", &ServerPort );
#elif defined( _WIN32 )
    int32_t ScanfResult = scanf_s( "%d", &ServerPort );
#endif
    READ_MYSELF( ScanfResult );

    *Port = htons((uint16_t)ServerPort);
#endif
    return GMI_SUCCESS;
}

class NetworkInitializer
{
public:
    NetworkInitializer()
    {
#if defined( _WIN32 )
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;

        wVersionRequested = MAKEWORD( 2, 2 );

        err = WSAStartup( wVersionRequested, &wsaData );
        if ( err != 0 )
        {
            /* Tell the user that we could not find a usable */
            /* WinSock DLL.                                  */
            return;
        }

        /* Confirm that the WinSock DLL supports 2.2.*/
        /* Note that if the DLL supports versions greater    */
        /* than 2.2 in addition to 2.2, it will still return */
        /* 2.2 in wVersion since that is the version we      */
        /* requested.                                        */

        if ( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 )
        {
            /* Tell the user that we could not find a usable */
            /* WinSock DLL.                                  */
            WSACleanup();
            return;
        }

        /* The WinSock DLL is acceptable. Proceed. */
#elif defined( __linux__ )
#endif
    }

    ~NetworkInitializer()
    {
#if defined( _WIN32 )
        WSACleanup();
#elif defined( __linux__ )
#endif
    }
};
