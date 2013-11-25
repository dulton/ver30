// media_center_server_unitest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "application_packet.h"
#include "gmi_system_headers.h"
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

#define DEFAULT_MEDIA_CENTER_SERVER_IP   "192.168.0.247"
#define DEFAULT_MEDIA_CENTER_SERVER_PORT 2000

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

GMI_RESULT RegisterCommandExecutor( MediaCenterService& Service );
GMI_RESULT UnregisterCommandExecutor( MediaCenterService& Service );

class NetworkInitializer;

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

    const size_t ServerIPLength = 128;
    char_t   ServerIP[ServerIPLength];
    memset( ServerIP, 0, ServerIPLength );
    uint32_t ServerPort = 0;

    printf( "please input server IP, for example: %s\n", DEFAULT_MEDIA_CENTER_SERVER_IP );
#if defined( __linux__ )
    int32_t ScanfResult = scanf( "%s", ServerIP );
#elif defined( _WIN32 )
    int32_t ScanfResult = scanf_s( "%s", ServerIP, ServerIPLength );
#endif
    READ_MYSELF( ScanfResult );

    printf( "please input server port, for example: %d\n", DEFAULT_MEDIA_CENTER_SERVER_PORT );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &ServerPort );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &ServerPort );
#endif

    SafePtr<NetworkInitializer> Initializer( BaseMemoryManager::Instance().New<NetworkInitializer>() );
    if ( NULL == Initializer.GetPtr() )
    {
        printf( "Initializer failed \n" );
        return -1;
    }

    GMI_RESULT Result = g_MediaCenter.Initialize( inet_addr(ServerIP), htons((uint16_t)ServerPort), UDP_SESSION_BUFFER_SIZE );
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter initialize failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = RegisterCommandExecutor( g_MediaCenter );
    if ( FAILED( Result ) )
    {
        g_MediaCenter.Deinitialize();
        printf( "RegisterCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = g_MediaCenter.Run( false );
    if ( FAILED( Result ) )
    {
        UnregisterCommandExecutor( g_MediaCenter );
        g_MediaCenter.Deinitialize();
        printf( "MediaCenter Run failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    do
    {
        GMI_Sleep( 1000 );
    }
    while ( 1 );

    Result = UnregisterCommandExecutor( g_MediaCenter );
    if ( FAILED( Result ) )
    {
        g_MediaCenter.Deinitialize();
        printf( "UnregisterCommandExecutor failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    Result = g_MediaCenter.Deinitialize();
    if ( FAILED( Result ) )
    {
        printf( "MediaCenter Deinitialize failed, Result=%x \n", (uint32_t) Result );
        return Result;
    }

    return 0;
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
    if ( NULL == CreateCodecCommandExecutor.GetPtr() )
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

GMI_RESULT UnregisterCommandExecutor( MediaCenterService& Service )
{
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
