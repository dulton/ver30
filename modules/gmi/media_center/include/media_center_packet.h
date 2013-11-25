// media_center_packet.h

#if !defined( MEDIA_CENTER_PACKET )
#define MEDIA_CENTER_PACKET

#include "application_packet_header.h"
#include "gmi_system_headers.h"

// ------------------------------------------------------------------------ //

#define GMI_MEDIA_CENTER_MESSAGE_TAG             "GMMT"

#define GMI_MEDIA_CENTER_MESSAGE_MAJOR_VERSION   1
#define GMI_MEDIA_CENTER_MESSAGE_MINOR_VERSION   0

// command
#define GMI_MEDIA_CENTER_START                   1
#define GMI_MEDIA_CENTER_STOP                    2
#define GMI_MEDIA_CENTER_RESTART                 3
#define GMI_MEDIA_CENTER_EXIT                    4

// vin and vout
#define GMI_OPEN_VIN_VOUT_DEVICE                 6
#define GMI_CLOSE_VIN_VOUT_DEVICE                7
#define GMI_GET_VIN_VOUT_CONFIGURATION           8
#define GMI_SET_VIN_VOUT_CONFIGURATION           9
// video/audio codec
#define GMI_CREATE_CODEC                         10
#define GMI_DESTROY_CODEC                        11
#define GMI_START_CODEC                          12
#define GMI_STOP_CODEC                           13
#define GMI_GET_CODEC_CONFIGURARTION             14
#define GMI_SET_CODEC_CONFIGURARTION             15
#define GMI_GET_OSD_CONFIGURARTION               16
#define GMI_SET_OSD_CONFIGURARTION               17
#define GMI_FORCE_GENERATE_IDR_FRAME             18
// image
#define GMI_OPEN_IMAGE_DEVICE                    20
#define GMI_CLOSE_IMAGE_DEVICE                   21
#define GMI_GET_BASE_IMAGE_CONFIGURARTION        22
#define GMI_SET_BASE_IMAGE_CONFIGURARTION        23
#define GMI_GET_ADVANCED_IMAGE_CONFIGURARTION    24
#define GMI_SET_ADVANCED_IMAGE_CONFIGURARTION    25
#define GMI_GET_AUTO_FOCUS_CONFIGURARTION        26
#define GMI_SET_AUTO_FOCUS_CONFIGURARTION        27
#define GMI_GET_DAYNIGHT_CONFIGURARTION          28
#define GMI_SET_DAYNIGHT_CONFIGURARTION          29
#define GMI_GET_WHITE_BALANCE_CONFIGURARTION     30
#define GMI_SET_WHITE_BALANCE_CONFIGURARTION     31
// auto focus
#define GMI_OPEN_AUTO_FOCUS_DEVICE               40
#define GMI_CLOSE_AUTO_FOCUS_DEVICE              41
#define GMI_START_AUTO_FOCUS                     42
#define GMI_STOP_AUTO_FOCUS                      43
#define GMI_PAUSE_AUTO_FOCUS                     44
#define GMI_AUTO_FOCUS_GLOBAL_SCAN               45
#define GMI_SET_AUTO_FOCUS_MODE                  46
#define GMI_NOTIFY_AUTO_FOCUS                    47
#define GMI_GET_FOCUS_POSITION                   48
#define GMI_SET_FOCUS_POSITION                   49
#define GMI_GET_FOCUS_RANGE                      50
#define GMI_RESET_FOCUS_MOTOR                    51
#define GMI_CONTROL_AUTO_FOCUS                   52
#define GMI_SET_AUTO_FOCUS_STEP                  53
// zoom
#define GMI_OPEN_ZOOM_DEVICE                     60
#define GMI_CLOSE_ZOOM_DEVICE                    61
#define GMI_GET_ZOOM_POSITION                    62
#define GMI_SET_ZOOM_POSITION                    63
#define GMI_GET_ZOOM_RANGE                       64
#define GMI_RESET_ZOOM_MOTOR                     65
#define GMI_CONTROL_ZOOM                         66
#define GMI_SET_ZOOM_STEP                        67

#define GMI_START_ZOOM                           100
#define GMI_STOP_ZOOM                            101

// notify

// ------------------------------------------------------------------------ //

// vin and vout
struct OpenVinVoutDevice : public BaseMessageHeader
{
    uint16_t     s_SensorId;
    uint16_t     s_ChannelId;
};

struct OpenVinVoutDeviceReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Token;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct CloseVinVoutDevice : public BaseMessageHeader
{
    uint32_t     s_Token;
};

struct CloseVinVoutDeviceReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct GetVinVoutConfiguration : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint16_t     s_VinParameterLength;
    uint16_t     s_VoutParameterLength;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct GetVinVoutConfigurationReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint16_t     s_VinParameterLength;
    uint8_t      s_VinParameter[1];
    uint16_t     s_VoutParameterLength;
    uint8_t      s_VoutParameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct SetVinVoutConfiguration : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint16_t     s_VinParameterLength;
    uint8_t      s_VinParameter[1];
    uint16_t     s_VoutParameterLength;
    uint8_t      s_VoutParameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct SetVinVoutConfigurationReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

// codec
struct StartCodec : public BaseMessageHeader
{
    uint8_t	     s_CodecMode;           // 1: encode; 0: decode
    uint8_t      s_Reserved[3];
    uint32_t     s_SourceId;
    uint32_t     s_MediaId;
    uint32_t     s_MediaType;
    uint32_t     s_CodecType;
    uint16_t     s_CodecParameterLength;
    uint8_t      s_CodecParameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct StartCodecReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Token;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct StopCodec : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct StopCodecReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct GetCodecConfiguration : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint16_t     s_ParameterLength;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct GetCodecConfigurationReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint16_t     s_ParameterLength;
    uint8_t      s_Parameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct SetCodecConfiguration : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint16_t     s_ParameterLength;
    uint8_t      s_Parameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct SetCodecConfigurationReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct GetOsdConfiguration : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint16_t     s_ParameterLength;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct GetOsdConfigurationReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint16_t     s_ParameterLength;
    uint8_t      s_Parameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct SetOsdConfiguration : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint16_t     s_ParameterLength;
    uint8_t      s_Parameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct SetOsdConfigurationReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct ForceGenerateIdrFrame : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct ForceGenerateIdrFrameReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

// image
struct OpenImageDevice : public BaseMessageHeader
{
    uint16_t     s_SensorId;
    uint16_t     s_ChannelId;
};

struct OpenImageDeviceReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Token;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct CloseImageDevice : public BaseMessageHeader
{
    uint32_t     s_Token;
};

struct CloseImageDeviceReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct GetBaseImageConfiguration : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint16_t     s_ParameterLength;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct GetBaseImageConfigurationReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint16_t     s_ParameterLength;
    uint8_t      s_Parameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct SetBaseImageConfiguration : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint16_t     s_ParameterLength;
    uint8_t      s_Parameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct SetBaseImageConfigurationReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct GetAdvancedImageConfiguration : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint16_t     s_ParameterLength;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct GetAdvancedImageConfigurationReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint16_t     s_ParameterLength;
    uint8_t      s_Parameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct SetAdvancedImageConfiguration : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint16_t     s_ParameterLength;
    uint8_t      s_Parameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct SetAdvancedImageConfigurationReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct GetAutoFocusConfiguration : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint16_t     s_ParameterLength;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct GetAutoFocusConfigurationReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint16_t     s_ParameterLength;
    uint8_t      s_Parameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct SetAutoFocusConfiguration : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint16_t     s_ParameterLength;
    uint8_t      s_Parameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct SetAutoFocusImageConfigurationReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct GetDaynightConfiguration : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint16_t     s_ParameterLength;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct GetDaynightImageConfigurationReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint16_t     s_ParameterLength;
    uint8_t      s_Parameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct SetDaynightConfiguration : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint16_t     s_ParameterLength;
    uint8_t      s_Parameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct SetDaynightConfigurationReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct GetWhiteBalanceConfiguration : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint16_t     s_ParameterLength;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct GetWhiteBalanceImageConfigurationReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint16_t     s_ParameterLength;
    uint8_t      s_Parameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct SetWhiteBalanceConfiguration : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint16_t     s_ParameterLength;
    uint8_t      s_Parameter[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct SetWhiteBalanceConfigurationReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

// auto focus
struct OpenAutoFocusDeviceInfo : public BaseMessageHeader
{
    uint16_t     s_SensorId;
};

struct OpenAutoFocusDeviceInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Token;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct CloseAutoFocusDeviceInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
};

struct CloseAutoFocusDeviceInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct StartAutoFocusInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
};

struct StartAutoFocusInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct StopAutoFocusInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
};

struct StopAutoFocusInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct PauseAutoFocusInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint32_t     s_ControlStatus;
};

struct PauseAutoFocusInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct AutoFocusGlobalScanInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
};

struct AutoFocusGlobalScanInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct SetAutoFocusModeInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint32_t     s_AFMode;
};

struct SetAutoFocusModeInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct NotifyAutoFocusInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint32_t     s_EventType;
    uint32_t     s_ExtDataLength;
    uint8_t      s_ExtData[1];
    uint8_t      s_Paddings[1];         //if required, use paddings to keep four bytes aligned
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct NotifyAutoFocusInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct GetFocusPositionInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
};

struct GetFocusPositionInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_FocusPos;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct SetFocusPositionInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint32_t     s_FocusPos;
};

struct SetFocusPositionInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct GetFocusRangeInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
};

struct GetFocusRangeInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_MinFocusPos;
    uint32_t     s_MaxFocusPos;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct ResetFocusMotorInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
};

struct ResetFocusMotorInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct ControlAutoFocusInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint32_t     s_AfDirMode;
};

struct ControlAutoFocusInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct SetAutoFocusStepInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint32_t     s_AfStep;
};

struct SetAutoFocusStepInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

// zoom
struct OpenZoomDeviceInfo : public BaseMessageHeader
{
    uint16_t     s_SensorId;
};

struct OpenZoomDeviceInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Token;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct CloseZoomDeviceInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
};

struct CloseZoomDeviceInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct GetZoomPositionInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
};

struct GetZoomPositionInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_ZoomPos;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct SetZoomPositionInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint32_t     s_ZoomPos;
};

struct SetZoomPositionInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct GetZoomRangeInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
};

struct GetZoomRangeInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_MinZoomPos;
    uint32_t     s_MaxZoomPos;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct ResetZoomMotorInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
};

struct ResetZoomMotorInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct ControlZoomInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint32_t     s_ZoomMode;
};

struct ControlZoomInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct SetZoomStepInfo : public BaseMessageHeader
{
    uint32_t     s_Token;
    uint32_t     s_ZoomStep;
};

struct SetZoomStepInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct StartZoomInfo : public BaseMessageHeader
{
    uint32_t     s_AutoFocusToken;
    uint32_t     s_AutoFocusControlStatus;
    uint32_t     s_ZoomToken;
    uint32_t     s_ZoomMode;
};

struct StartZoomInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct StopZoomInfo : public BaseMessageHeader
{
    uint32_t     s_AutoFocusToken;
    uint32_t     s_AutoFocusControlStatus;
    uint32_t     s_ZoomToken;
    uint32_t     s_ZoomMode;
};

struct StopZoomInfoReply : public BaseMessageHeader
{
    uint32_t     s_Result;
    uint32_t     s_ZoomPos;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

#endif//MEDIA_CENTER_PACKET
