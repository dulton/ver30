
// ipc_fw_v3x_setting.h
// define setting for ipc_fw_v3x project
// note: these value are only default value, program can use other value.

#if !defined( IPC_FW_V3X_SETTING )
#define IPC_FW_V3X_SETTING

/************************ log config ************************/
#define GMI_IPC_LOG_FILE_PATH                               "/opt/log"

/************************ udp port ************************/

// onvif rtsp client
// ONVIF_MEDIA_SERVER_RTSP_CONFIG_PATH is defined in ipc_fw_v3.x_resource.h
//#define ONVIF_MEDIA_SERVER_RTSP_CONFIG_PATH               "/Config/onvif_media_server/rtsp/"
#define ONVIF_MEDIA_SERVER_RTSP_CONFIG_CLIENT_RTP_UDP_PORT  "client_rtp_udp_port"
#define ONVIF_MEDIA_SERVER_RTSP_CONFIG_CLIENT_RTCP_UDP_PORT "client_rtcp_udp_port"
#define GMI_ONVIF_RTSP_CLIENT_ENCODE_VIDEO1_RTP             2000
#define GMI_ONVIF_RTSP_CLIENT_ENCODE_VIDEO1_RTCP            2001
#define GMI_ONVIF_RTSP_CLIENT_ENCODE_VIDEO2_RTP             2002
#define GMI_ONVIF_RTSP_CLIENT_ENCODE_VIDEO2_RTCP            2003
#define GMI_ONVIF_RTSP_CLIENT_ENCODE_VIDEO3_RTP             2004
#define GMI_ONVIF_RTSP_CLIENT_ENCODE_VIDEO3_RTCP            2005
#define GMI_ONVIF_RTSP_CLIENT_ENCODE_VIDEO4_RTP             2006
#define GMI_ONVIF_RTSP_CLIENT_ENCODE_VIDEO4_RTCP            2007
#define GMI_ONVIF_RTSP_CLIENT_ENCODE_AUDIO1_RTP             2008
#define GMI_ONVIF_RTSP_CLIENT_ENCODE_AUDIO1_RTCP            2009
#define GMI_ONVIF_RTSP_CLIENT_DECODE_AUDIO1_RTP             2010
#define GMI_ONVIF_RTSP_CLIENT_DECODE_AUDIO1_RTCP            2011

//capabilities software
#define CAPABILITY_SW_FILE_NAME_BAK                "/opt/config/capability_sw.xml.bak"
#define CAPABILITY_SW_FILE_NAME                    "/opt/config/capability_sw.xml"
#define CAPABILITY_AUTO_FILE_NAME                  "/opt/config/capability_auto.xml"
#define CAPABILITY_CONFIGURABLE_FILE_NAME          "/opt/config/capability_configurable.xml"
//stream combine 
#define COMBINE_720P_FILE_NAME                     "/opt/config/720p_combine.xml"
#define COMBINE_1080P_FILE_NAME                    "/opt/config/1080p_combine.xml"
//limits
#define LIMITS_FILE_NAME                           "/opt/config/Limits.xml"

//user file
#define GMI_USERS_FILE_NAME                        "/opt/config/gmi_users.db"
//users table
#define GMI_USERS_TABLE_NAME                       "Users"
#define GMI_USERS_TABLE_VALUES                     "(ID INTEGER PRIMARY KEY AUTOINCREMENT, Name TEXT, Password TEXT, Role INTEGER, Popedom INTEGER, Encrypt INTEGER)"
#define GMI_SUPER_USER_NAME                        "root"
#define GMI_SUPER_USER_PASSWD                      "Gmi_Ihdnc&_200_"
#define GMI_ADMIN_USER_NAME1                       "admin"
#define GMI_ADMIN_USER_NAME1_PASSWD                "admin"
#define GMI_ADMIN_USER_NAME2                       "inc"
#define GMI_ADMIN_USER_NAME2_PASSWD                "inc"
#define GMI_ADMIN_AUTH_VALUE                       0x1ff 


//netconfig file
#define NETWORK_CONFIG_FILE                         "/etc/network/interfaces"
#define NET_CONFIG_FILE                             "/opt/config/net_config.xml"
#define NET_IPINFO_0_CONFIG_PATH                    "/net_info/ip_info_0/"
#define NET_DEFAULT_ETH                              0
#define NET_DEFAULT_DHCP                             0
#define NET_DEFAULT_MAC                             "7A:C5:54:52:03:A8"
#define NET_DEFAULT_IP                              "10.0.0.2"
#define NET_DEFAULT_MASK                            "255.255.255.0"
#define NET_DEFAULT_GATEWAY                         "10.0.0.1"
#define NET_DEFAULT_DNS                             "0.0.0.0"

//setting file
#define GMI_SETTING_CONFIG_FILE_NAME                "/opt/config/gmi_setting.xml"
#define GMI_DEFAULT_SETTING_CONFIG_FILE_NAME        "/opt/config/gmi_default_setting.xml"
#define GMI_FACTORY_SETTING_CONFIG_FILE_NAME        "/opt/config/gmi_factory_setting.xml"
//hw auto detect info
#define SENSOR_ID_2715                              "ov2715"
#define SENSOR_ID_9715                              "ov9715"
#define SENSOR_ID_IMX122                            "imx122"
#define SENSOR_ID_34041                             "mn34041pl"
#define SENSOR_ID_TW9910                            "tw9910"
#define CPU_ID_A55                                  "A5S_55"
#define CPU_ID_A66                                  "A5S_66"
#define CPU_ID_A88                                  "A5S_88"
#define LENS_NONE                                   "NONE"
#define LENS_DF003                                  "DF003"
#define LENS_YB22                                   "YB22"
#define BOARD_NORMAL                                "NORMAL"
#define BOARD_LARK                                  "LARK"

//hw value
#define HW_AUTO_DETECT_INFO_PATH                    "/Capability/"
#define HW_CPU                                      "A5S_55"
#define HW_SENSOR                                   "ov9715"
#define HW_LENS                                     "NONE"
#define HW_MAINBOARD                                "NORMAL"
//hw key s_MainBoard
#define HW_CPU_KEY                                  "CPU"
#define HW_SENSOR_KEY                               "VideoIn"
#define HW_LENS_KEY                                 "Lens"
#define HW_MAINBOARD_KEY                            "MainBoard"
//software info
#define CAPABILITY_SW_MEDIA_PATH                    "/Capabilities/Device/Media/"
#define MAX_PIC_WIDTH_KEY                           "MaxPicWidth"
#define MAX_PIC_HEIGHT_KEY                          "MaxPicHeight"
#define MAX_STREAM_NUM_KEY                          "MaxStreamNum"


/*========ptz==========*/
#define PTZ_UART_CONFIG_PATH                        "/Config/PTZ/uart_config/"
#define PTZ_UART_BAUDE_RATE                         9600
#define PTZ_UART_DATA_BITS                           3
#define PTZ_UART_PARITY                              0
#define PTZ_UART_SOTP_BITS                           0
#define PTZ_UART_FLOW_CONTROL                        0
#define PTZ_UART_ADDRESS                             1
#define PTZ_AUTO_FOCUS_CONFIG_PATH                  "/Config/PTZ/Focus/"
//default value 1-auto, 0-manual, 2-onceauto
#define PTZ_AUTO_FOCUS_MODE                          1
#define PTZ_AUTO_FOCUS_MODE_KEY                     "FocusMode"
#define PTZ_CURRENT_ZOOM_CONFIG_PATH                "/Config/PTZ/Zoom/"
#define PTZ_CURRENT_ZOOM_KEY                        "CurrentZoom"
#define PTZ_CURRENT_ZOOM                            1
//speed map
#define PTZ_SHIELD_NAME_PATH                        "/Config/PTZ/"
#define PTZ_SHIELD_NAME_KEY                         "Name"
#define PTZ_SPEED_MAP_PATH                          "/Config/PTZ/Speed/"
#define PTZ_V_SPEED_KEY                             "VspeedMap%d"
#define PTZ_H_SPEED_KEY                             "HspeedMap%d"

//ptz presets patrols file
#define GMI_PTZ_PRESETS_PATROLS_FILE_NAME           "/opt/config/ipc_presets_setting.xml"
#define PTZ_PRESET_INFO_PATH                        "/Config/PTZ/preset_info/Id%d/"
#define PTZ_PRESET_INDEX_KEY                        "Index"
#define PTZ_PRESET_NAME_KEY                         "Name"
#define PTZ_PRESET_ZOOM_POSITION_KEY                "Zoom"
#define PTZ_PRESET_SETTED_KEY                       "Setted"

/*========video==========*/
/*========video source value==========*/
#define VIDEO_SOURCE_PATH                           "/Config/Video/Source/"
//source-576p,720p,1080p
#define VIDEO_SOURCE_WIDTH                          1280
#define VIDEO_SOURCE_HEIGHT                         720
#define VIDEO_SOURCE_FRAME_RATE                     25
#define VIDEO_SOURCE_MIRROR                         0
//video source key
#define VIDEO_SOURCE_WIDTH_KEY                      "SourceWidth"
#define VIDEO_SOURCE_HEIGHT_KEY                     "SourceHeight"
#define VIDEO_SOURCE_FRAME_RATE_KEY                 "FPS"
#define VIDEO_SOURCE_MIRROR_KEY                     "Mirror"

/////video image value
#define VIDEO_SOURCE_IMAGE_PATH                     "/Config/Video/Source/Image/"
//0-50Hz,1-60Hz,2-auto
#define VIDEO_SOURCE_IMAGE_EXPOSURE_MODE            0
//125us~40000us
#define VIDEO_SOURCE_IMAGE_EXPOSURE_VALUE_MIN       125
//125us~133333us
#define VIDEO_SOURCE_IMAGE_EXPOSURE_VALUE_MAX       40000
//36,42,48,54,60db
#define VIDEO_SOURCE_IMAGE_GAIN_MAX                 36
//-255~255
#define VIDEO_SOURCE_IMAGE_BRIGHTNESS               0
//0~255
#define VIDEO_SOURCE_IMAGE_CONTRAST                 128
//0~255
#define VIDEO_SOURCE_IMAGE_SATURATION               64
//-15~15
#define VIDEO_SOURCE_IMAGE_HUE                      0
//0~255
#define VIDEO_SOURCE_IMAGE_SHARPNESS                128

#define VIDEO_SOURCE_ADVANCED_IMAGE_PATH            "/Config/Video/Source/AdvancedImage/"
//0-spot,1-center,2-average,3-custom
#define VIDEO_IMAGE_ADVANCE_METERING_MODE           1
//0-disable,1-enable
#define VIDEO_IMAGE_ADVANCE_BACKLIGHTCOMP_FLAG      0
//0-disable,1-enable
#define VIDEO_IMAGE_ADVANCE_DCIRIS_FLAG             0
//0-disable,1-auto,128-2x,192-3x,256-4x
#define VIDEO_IMAGE_ADVANCE_LOCAL_EXPOSURE          0
//0-disable,32-1x,64-2x,96-3x,128-4x,160-5x,...512-16x
#define VIDEO_IMAGE_ADVANCE_MCTF_STRLENGTH          32
//100~999
#define VIDEO_IMAGE_ADVANCE_DCIRIS_DUTY             100
//25~400,unit in percentage
#define VIDEO_IMAGE_ADVANCE_AETARGETRATIO           110

#define VIDEO_IMAGE_WHITE_BALANCE_PATH              "/Config/Video/Source/WhiteBalance/"
//0-auto,1-incandesent,2-4000,3-5000,4-sunny,5-cloudy,6-flash
#define VIDEO_IMAGE_WHITE_BALANCE_MODE              0
//0~1023
#define VIDEO_IMAGE_WHITE_BALANCE_RGAIN             512
//0~1023
#define VIDEO_IMAGE_WHITE_BALANCE_BGAIN             512

#define VIDEO_IMAGE_DAY_NIGHT_PATH                  "/Config/Video/Source/DayNight/"
//0-day,1-night,2-auto,3-timing
#define VIDEO_IMAGE_DAY_NIGHT_MODE                  0
//3-~30s
#define VIDEO_IMAGE_DAY_NIGHT_DURTIME               5
//0~100
#define VIDEO_IMAGE_DAY_TO_NIGHT_THRESHOLD          40
//0~100
#define VIDEO_IMAGE_NIGHT_TO_DAY_THRESHOLD          60
//0-disable,1-enable
#define VIDEO_IMAGE_DAY_NIGHT_SCHED_ENABLE          0
//0~24*3600
#define VIDEO_IMAGE_DAY_NIGHT_SCHED_START_TIME      0
//0~24*3600
#define VIDEO_IMAGE_DAY_NIGHT_SCHED_END_TIME        0
typedef enum 
{
	DN_MODE_DAY = 0,
	DN_MODE_NIGHT,
	DN_MODE_AUTO,
	DN_MODE_TIMING
}DayNightMode;

#define VIDEO_IRCUT_NAME_PATH                       "/Config/IRCUT/"
#define VIDEO_IRCUT_NAME_KEY                        "Name"
#define VIDEO_IRCUT_MODE_PATH                       "/Config/IRCUT/Mode/"
#define VIDEO_IRCUT_MODE                            0
#define VIDEO_IRCUT_ADC_MODE                        0
#define VIDEO_IRCUT_IRLIGHT_MODE                    0
#define VIDEO_IRCUT_DAYTONIGHT_THR                  40
#define VIDEO_IRCUT_NIGHTTODAY_THR                  60
#define VIDEO_IRCUT_LUX5_NIGHT_PATH                 "/Config/IRCUT/Lux5/Night/"
#define VIDEO_LUX5_NIGHT_MIN                        105
#define VIDEO_LUX5_NIGHT_MAX                        105
#define VIDEO_LUX5_NIGHT_AVG                        105
#define VIDEO_IRCUT_LUX5_NIGHT_ADJUST_PATH          "/Config/IRCUT/Lux5/NightAdjust/"
#define VIDEO_LUX5_NIGHT_ADJUST_MIN                 105
#define VIDEO_LUX5_NIGHT_ADJUST_MAX                 105
#define VIDEO_LUX5_NIGHT_ADJUST_AVG                 105
#define VIDEO_IRCUT_LUX10_DAY_PATH                  "/Config/IRCUT/Lux10/Day/"
#define VIDEO_LUX10_DAY_MIN                         230
#define VIDEO_LUX10_DAY_MAX                         230
#define VIDEO_LUX10_DAY_AVG                         230
#define VIDEO_IRCUT_LUX10_DAY_ADJUST_PATH           "/Config/IRCUT/Lux10/DayAdjust/"
#define VIDEO_LUX10_DAY_ADJUST_MIN                  230
#define VIDEO_LUX10_DAY_ADJUST_MAX                  230
#define VIDEO_LUX10_DAY_ADJUST_AVG                  230
typedef enum
{
	IRCUT_MODE_AE = 0,
	IRCUT_MODE_ADC,
	IRCUT_MODE_AE_EX_INTENSITY
}IrcutMode;

//video image key
#define VIDEO_SOURCE_IMAGE_EXPOSURE_MODE_KEY        "ExposureMode"
#define VIDEO_SOURCE_IMAGE_EXPOSURE_VALUE_MIN_KEY   "ExposureValueMin"
#define VIDEO_SOURCE_IMAGE_EXPOSURE_VALUE_MAX_KEY   "ExposureValueMax"
#define VIDEO_SOURCE_IMAGE_GAIN_MAX_KEY             "GainMax"
#define VIDEO_SOURCE_IMAGE_BRIGHTNESS_KEY           "Brightness"
#define VIDEO_SOURCE_IMAGE_CONTRAST_KEY             "Contrast"
#define VIDEO_SOURCE_IMAGE_SATURATION_KEY           "Saturation"
#define VIDEO_SOURCE_IMAGE_HUE_KEY                  "Hue"
#define VIDEO_SOURCE_IMAGE_SHARPNESS_KEY            "Sharpness"

#define VIDEO_IMAGE_ADVANCE_METERING_MODE_KEY       "MeteringMode"     
#define VIDEO_IMAGE_ADVANCE_BACKLIGHTCOMP_KEY       "BacklightCompFlag"
#define VIDEO_IMAGE_ADVANCE_DCIRIS_FLAG_KEY         "DcIrisFlag"
#define VIDEO_IMAGE_ADVANCE_LOCAL_EXPOSURE_KEY      "LocalExposure"
#define VIDEO_IMAGE_ADVANCE_MCTF_STRLENGTH_KEY      "MctfStrength"
#define VIDEO_IMAGE_ADVANCE_DCIRIS_DUTY_KEY         "DcIrisDuty"
#define VIDEO_IMAGE_ADVANCE_AETARGETRATIO_KEY       "AeTargetRatio"

#define VIDEO_IMAGE_WHITE_BALANCE_MODE_KEY          "WhiteBalanceMode"
#define VIDEO_IMAGE_WHITE_BALANCE_RGAIN_KEY         "WB_Rgain"
#define VIDEO_IMAGE_WHITE_BALANCE_BGAIN_KEY         "WB_Bgain"

#define VIDEO_IMAGE_DAY_NIGHT_MODE_KEY              "DnMode"
#define VIDEO_IMAGE_DAY_NIGHT_DURTIME_KEY           "DnDurTime"
#define VIDEO_IMAGE_DAY_TO_NIGHT_THRESHOLD_KEY      "DayToNightThreshold"
#define VIDEO_IMAGE_NIGHT_TO_DAY_THRESHOLD_KEY      "NightToDayThreshold"
#define VIDEO_IMAGE_DAY_NIGHT_SCHED_ENABLE_KEY      "Day%d_DnEnable"
#define VIDEO_IMAGE_DAY_NIGHT_SCHED_START_TIME_KEY  "Day%d_DnStartTime"
#define VIDEO_IMAGE_DAY_NIGHT_SCHED_END_TIME_KEY    "Day%d_DnEndTime"

#define VIDEO_IRCUT_MODE_KEY                            "IrcutMode"
#define VIDEO_IRCUT_ADC_MODE_KEY                        "AdcMode"
#define VIDEO_IRCUT_IRLIGHT_MODE_KEY                    "IrLightMode"
#define VIDEO_IRCUT_DAYTONIGHT_THR_KEY                  "DayToNightThr"
#define VIDEO_IRCUT_NIGHTTODAY_THR_KEY                  "NightToDayThr"
#define VIDEO_LUX5_NIGHT_MIN_KEY                        "Min"
#define VIDEO_LUX5_NIGHT_MAX_KEY                        "Max"
#define VIDEO_LUX5_NIGHT_AVG_KEY                        "Avg"
#define VIDEO_LUX5_NIGHT_ADJUST_MIN_KEY                 "Min"
#define VIDEO_LUX5_NIGHT_ADJUST_MAX_KEY                 "Max"
#define VIDEO_LUX5_NIGHT_ADJUST_AVG_KEY                 "Avg"
#define VIDEO_LUX10_DAY_MIN_KEY                         "Min"
#define VIDEO_LUX10_DAY_MAX_KEY                         "Max"
#define VIDEO_LUX10_DAY_AVG_KEY                         "Avg"
#define VIDEO_LUX10_DAY_ADJUST_MIN_KEY                  "Min"
#define VIDEO_LUX10_DAY_ADJUST_MAX_KEY                  "Max"
#define VIDEO_LUX10_DAY_ADJUST_AVG_KEY                  "Avg"


/*========video encode value==========*/
#define VIDEO_ENCODE_PATH                           "/Config/Video/Encode/"
#define VIDEO_ENCODE_STREAM_PATH                    "/Config/Video/Encode/Stream%d/"
// 1~4
#define VIDEO_ENCODE_STREAM_NUM                      2
//1-h264,3-mjpeg
#define VIDEO_ENCODE_CODEC                           1
// 1-video, 2-audio&video
#define VIDEO_ENCODE_STREAM_TYPE                     1
//1080p25+576p25
#define VIDEO_ENCODE_1080P_TWOSTREAM_COMBINE_NO      0
//720p25+576p25
#define VIDEO_ENCODE_720P_TWOSTREAM_COMBINE_NO       0
#define RES_1080P_WIDTH                              1920
#define RES_1080P_HEIGHT                             1080
#define RES_720P_WIDTH                               1280
#define RES_720P_HEIGHT                              720
#define RES_D1_PAL_WIDTH                             720
#define RES_D1_PAL_HEIGHT                            576
#define RES_D1_NTSC_WIDTH                            720
#define RES_D1_NTSC_HEIGHT                           480
#define RES_CIF_PAL_WIDTH                            352
#define RES_CIF_PAL_HEIGHT                           288
#define RES_CIF_NTSC_WIDTH                           352
#define RES_CIF_NTSC_HEIGHT                          240
#define RES_QCIF_PAL_WIDTH                           176
#define RES_QCIF_PAL_HEIGHT                          144
#define RES_QVGA_WIDTH                               320
#define RES_QVGA_HEIGHT                              240

//1-CBR,2-VBR,3-CBRQ,4-VBRQ,5-CVBRQ
#define VIDEO_ENCODE_STREAM_BITRATE_TYPE             1
//bitrates
#define VIDEO_ENCODE_STREAM_BITRATE                 2000
#define VIDEO_ENCODE_1080P_STREAM_BITRATE           4000
#define VIDEO_ENCODE_1080P_STREAM_BITRATE_UP        8000
#define VIDEO_ENCODE_1080P_STREAM_BITRATE_DOWN      1000
#define VIDEO_ENCODE_720P_STREAM_BITRATE            2000
#define VIDEO_ENCODE_720P_STREAM_BITRATE_UP         4000
#define VIDEO_ENCODE_720P_STREAM_BITRATE_DOWN       1000
#define VIDEO_ENCODE_576P_STREAM_BITRATE            1500
#define VIDEO_ENCODE_576P_STREAM_BITRATE_UP         2000
#define VIDEO_ENCODE_576P_STREAM_BITRATE_DOWN       500
#define VIDEO_ENCODE_CIF_STREAM_BITRATE             1000
#define VIDEO_ENCODE_CIF_STREAM_BITRATE_UP          2000
#define VIDEO_ENCODE_CIF_STREAM_BITRATE_DOWN        100
#define VIDEO_ENCODE_QCIF_STREAM_BITRATE            500
#define VIDEO_ENCODE_QCIF_STREAM_BITRATE_UP         1000
#define VIDEO_ENCODE_QCIF_STREAM_BITRATE_DOWN       100

//osd font size
#define OSD_FONT_SIZE_BIGNESS                       48
#define OSD_FONT_SIZE_BIG                           40
#define OSD_FONT_SIZE_MEDIUM                        32
#define OSD_FONT_SIZE_SMALL                         24
#define OSD_FONT_SIZE_SMALLNESS                     16

//1~60
#define VIDEO_ENCODE_STREAM_FRAME_RATE              25
//25,30,50,60,100
#define VIDEO_ENCODE_STREAM_GOP                     25
//1~100
#define VIDEO_ENCODE_STREAM_QUALITY                 50
//0,1,2,3
#define VIDEO_ENCODE_STREAM_ROTATE                  0
//video encode key
#define VIDEO_ENCODE_STREAM_NUM_KEY                  "StreamNum"
#define VIDEO_ENCODE_COMBINE_NO_KEY                  "CombineNo"
#define VIDEO_ENCODE_CODEC_KEY                       "EncodeType"
#define VIDEO_ENCODE_STREAM_TYPE_KEY                 "StreamType"
#define VIDEO_ENCODE_STREAM_WIDTH_KEY                "EncodeWidth"
#define VIDEO_ENCODE_STREAM_HEIGHT_KEY               "EncodeHeight"
#define VIDEO_ENCODE_STREAM_BITRATE_TYPE_KEY         "BitRateType"
#define VIDEO_ENCODE_STREAM_BITRATE_KEY              "BitRateAverage"
#define VIDEO_ENCODE_STREAM_BITRATEUP_KEY            "BitRateUp"
#define VIDEO_ENCODE_STREAM_BITRATEDOWN_KEY          "BitRateDown"
#define VIDEO_ENCODE_STREAM_FRAME_RATE_KEY           "FrameRate"
#define VIDEO_ENCODE_STREAM_GOP_KEY                  "FrameInterval"
#define VIDEO_ENCODE_STREAM_QUALITY_KEY              "EncodeQulity"
#define VIDEO_ENCODE_STREAM_ROTATE_KEY               "Rotate"

/*========osd==========*/
//osd value
#define OSD_PATH                                   "/Config/Video/Osd%d/"
#define OSD_TIME_PATH                              "/Config/Video/Osd%d/Time/"
#define OSD_TEXT_PATH                              "/Config/Video/Osd%d/Text%d/"
//0-opacity&no-blink,1-hyaline&no-blink,2-opacity&blink,3-hyaline&blink
#define OSD_DISPLAY_TYPE                             0
//0/1-English,2-chinese,3-korean
#define OSD_LANGUAGE                                 2
//  Max num:4
#define OSD_DEFAULT_TEXT_NUM                         1
//0-disable, 1-enable
#define OSD_ENABLE                                   1
//0-disable, 1-enable
#define OSD_TIME_ENABLE                              1
//0-year-month-day, 1-month-day-year
#define OSD_TIME_DATE_STYLE                          1
//0-hour:minute:second
#define OSD_TIME_STYLE                               0
//0-black, 1-red, 2-blue,3-green,4-yellow,5-magenta,6-cyan,7-white,8-auto(self-adaption according to backgroud,range between white and balck), default-7
#define OSD_TIME_FONT_COLOR                          7
//Font size, range 16,24,32,40,48, auto-0, default auto
#define OSD_TIME_FONT_STYLE                          0
//0-no-blod, 1-blod, default-0
#define OSD_TIME_FONT_BLOD                           0
//0-no-rotate, 1-rotate, default-0
#define OSD_TIME_ROTATE                              0
//0-no-italic, 1-italic, default-0
#define OSD_TIME_ITALIC                              0
//0~10
#define OSD_TIME_OUTLINE                             0
//0~100
#define OSD_TIME_DISPLAY_X                           0
//0~100
#define OSD_TIME_DISPLAY_Y                           0
//0~100
#define OSD_TIME_DISPLAY_H                           100
//0~100
#define OSD_TIME_DISPLAY_W                           100
//0-disable, 1-enable
#define OSD_TEXT_ENABLE                              1
//0~10
#define OSD_TEXT_OUTLINE                             0
//0-no-italic, 1-italic, default-0
#define OSD_TEXT_ITALIC                              0
//0-black, 1-red, 2-blue,3-green,4-yellow,5-magenta,6-cyan,7-white,8-auto(self-adaption according to backgroud,range between white and balck), default-7
#define OSD_TEXT_FONT_COLOR                          7
//0~100
#define OSD_TEXT_DISPLAY_X                           88
//0~100
#define OSD_TEXT_DISPLAY_Y                           90
//Font size, range 16,24,32,40,48,auto-0, default auto
#define OSD_TEXT_FONT_STYLE                          0
//0~100
#define OSD_TEXT_DISPLAY_H                           100
//0~100
#define OSD_TEXT_DISPLAY_W                           100
//0-no-blod, 1-blod, default-0
#define OSD_TEXT_FONT_BLOD                           0
//0-no-rotate, 1-rotate, default-0
#define OSD_TEXT_ROTATE                              0
#define OSD_TEXT_CONTENT                             "IPCamera"
#define OSD_TEXT_SIZE                                48
//osd key
#define OSD_DISPLAY_TYPE_KEY                         "DisplayType"
#define OSD_LANGUAGE_KEY                             "Language"
#define OSD_TEXT_NUM_KEY                             "TextNum"
#define OSD_ENABLE_KEY                               "Enable"
#define OSD_TIME_ENABLE_KEY                          "TimeEnable"
#define OSD_TIME_DATE_STYLE_KEY                      "DateStyle"
#define OSD_TIME_STYLE_KEY                           "TimeStyle"
#define OSD_TIME_FONT_COLOR_KEY                      "TimeFontColor"
#define OSD_TIME_FONT_STYLE_KEY                      "TimeFontStyle1223"
#define OSD_TIME_FONT_BLOD_KEY                       "TimeFontBlod"
#define OSD_TIME_ROTATE_KEY                          "TimeRotate"
#define OSD_TIME_ITALIC_KEY                          "TimeItalic"
#define OSD_TIME_OUTLINE_KEY                         "TimeOutline"
#define OSD_TIME_DISPLAY_X_KEY                       "TimeDisplayX"
#define OSD_TIME_DISPLAY_Y_KEY                       "TimeDisplayY"
#define OSD_TIME_DISPLAY_H_KEY                       "TimeDisplayH"
#define OSD_TIME_DISPLAY_W_KEY                       "TimeDisplayW"
#define OSD_TEXT_ENABLE_KEY                          "TextEnable"
#define OSD_TEXT_OUTLINE_KEY                         "TextOutline"
#define OSD_TEXT_ITALIC_KEY                          "TextItalic"
#define OSD_TEXT_FONT_COLOR_KEY                      "TextFontColor"
#define OSD_TEXT_DISPLAY_X_KEY                       "TextDisplayX"
#define OSD_TEXT_DISPLAY_Y_KEY                       "TextDisplayY"
#define OSD_TEXT_FONT_STYLE_KEY                      "TextFontStyle1223"
#define OSD_TEXT_DISPLAY_H_KEY                       "TextDisplayH"
#define OSD_TEXT_DISPLAY_W_KEY                       "TextDisplayW"
#define OSD_TEXT_FONT_BLOD_KEY                       "TextFontBlod"
#define OSD_TEXT_ROTATE_KEY                          "TextRotate"
#define OSD_TEXT_CONTENT_KEY                         "TextContent"


/*========audio==========*/
#define AUDIO_ENCODE_PATH                           "/Config/Audio/Encode/"
#define AUDIO_ENCODE_TYPE                           1
#define AUDIO_ENCODE_CHANNEL                        1
#define AUDIO_ENCODE_BITS_PERSAMPLE                 16
#define AUDIO_ENCODE_SAMPLES_PERSEC                 8000
#define AUDIO_ENCODE_FRAME_RATE                     8
#define AUDIO_ENCODE_CAP_VOLUME                     80
#define AUDIO_ENCODE_PLAY_VOLUME                    80
#define AUDIO_ENCODE_BIT_RATE                       64
#define AUDIO_ENCODE_PLAY_ENABLE                    0
#define AUDIO_ENCODE_AEC_ENABLE                     0
#define AUDIO_ENCODE_AEC_DELAY_TIME                 0

#define AUDIO_ENCODE_TYPE_KEY                       "EncodeType"
#define AUDIO_ENCODE_CHANNEL_KEY                    "Channel"
#define AUDIO_ENCODE_BITS_PERSAMPLE_KEY             "BitsPerSample"
#define AUDIO_ENCODE_SAMPLES_PERSEC_KEY             "SamplesPerSec"
#define AUDIO_ENCODE_FRAME_RATE_KEY                 "FPS"
#define AUDIO_ENCODE_CAP_VOLUME_KEY                 "CapVolume"
#define AUDIO_ENCODE_PLAY_VOLUME_KEY                "PlayVolume"
#define AUDIO_ENCODE_BIT_RATE_KEY                   "BitRate"
#define AUDIO_ENCODE_PLAY_ENABLE_KEY                "PlayEnable"
#define AUDIO_ENCODE_AEC_ENABLE_KEY                 "AecEnable"
#define AUDIO_ENCODE_AEC_DELAY_TIME_KEY             "AecDelayTime"


/*========device info==========*/
//device info value
#define DEVICE_INFO_PATH                            "/Config/Device/DeviceInfo/"
#define DEVICE_NAME                                 "IPCamera"
#define DEVICE_ID                                   1
#define DEVICE_MODEL                                "iHDNC"
#define DEVICE_MANUFACTUER                          "inc"
#define DEVICE_FWVER                                "0.0"
#define DEVICE_HWVER                                "V2.0"
#define DEVICE_SN                                   "34789345"
//device info key
#define DEVICE_NAME_KEY                             "Name"
#define DEVICE_ID_KEY                               "Id"
#define DEVICE_MODEL_KEY                            "Model"
#define DEVICE_MANUFACTUER_KEY                      "Manufactuer"
#define DEVICE_FWVER_KEY                            "Fwver"
#define DEVICE_HWVER_KEY                            "Hwver"
#define DEVICE_SN_KEY                               "Sn"

//ntp
#define NTP_INFO_PATH                               "/Config/Device/Net/NtpInfo/"
#define NTP_INFO_1                                  "0.0.0.0"
#define NTP_INFO_2                                  "0.0.0.0"
#define NTP_INFO_3                                  "0.0.0.0"
#define NTP_INFO_1_KEY                              "Ntp1"
#define NTP_INFO_2_KEY                              "Ntp2"
#define NTP_INFO_3_KEY                              "Ntp3"

//time type
#define DATE_TIME_PATH                              "/Config/Device/Time/"
#define DATE_TIME_TYPE                              0
#define DATE_TIME_NTP_INTERVAL                      0
#define DATE_TIME_TYPE_KEY                          "DateTimeType"
#define DATE_TIME_NTP_INTERVAL_KEY                  "NtpInterval"

//version, compile num
#define VERSION_COMPILE_FILE_NAME                   "/opt/bin/CompileNumber"
#define COMPILE_NUM_KEY                             "Num"


/*========alarm==========*/
#define ALARM_IN_CONFIG_PATH                        "/Config/Alarm/AlarmIn%d/"
#define ALARM_IN_ENABLE_KEY                         "Enable"
#define ALARM_IN_INPUT_NO_KEY                       "InputNo"
#define ALARM_IN_NAME_KEY                           "Name"
#define ALARM_IN_CHECK_TIME_KEY                     "CheckTime"
#define ALARM_IN_NORMAL_STATUS_KEY                  "NormalStatus"
#define ALARM_IN_LINK_STRATEGY_KEY                  "LinkStrategy"
#define ALARM_IN_LINK_ALARM_OUT_NO_KEY              "LinkAlarmOutNo"
#define ALARM_IN_LINK_PTZ_FUNC_KEY                  "LinkPTZFuncNo"
#define ALARM_IN_LINK_PTZ_SEQ_KEY                   "LinkPTZSeqNo"
#define ALARM_IN_ENABLE                         	(0)
#define ALARM_IN_INPUT_NO                       	(0)
#define ALARM_IN_NAME                           	("AlarmIn")
//300ms
#define ALARM_IN_CHECK_TIME                    		(300)
//normal open
#define ALARM_IN_NORMAL_STATUS                 		(1)
#define ALARM_IN_LINK_STRATEGY                  	(0x00)
#define ALARM_IN_LINK_ALARM_OUT_NO              	(0)
#define ALARM_IN_LINK_PTZ_FUNC                  	(0)
#define ALARM_IN_LINK_PTZ_SEQ                   	(0)

#define ALARM_OUT_CONFIG_PATH                       "/Config/Alarm/AlarmOut%d/"
#define ALARM_OUT_ENABLE_KEY                        "Enable"
#define ALARM_OUT_OUTPUT_NO_KEY                     "OutputNo"
#define ALARM_OUT_NAME_KEY                          "Name"
#define ALARM_OUT_NORMAL_STATUS_KEY                 "NormalStatus"
#define ALARM_OUT_DELAY_TIME_KEY                    "DelayTime"
#define ALARM_OUT_ENABLE                            (0)
#define ALARM_OUT_OUTPUT_NO                     	(0)
#define ALARM_OUT_NAME                          	"AlarmOut"
//open
#define ALARM_OUT_NORMAL_STATUS                 	(1)
//5S
#define ALARM_OUT_DELAY_TIME                    	(5)

#define ALARM_PIR_CONFIG_PATH                       "/Config/Alarm/PIR/"
#define ALARM_PIR_ALARM_ID_KEY                      "ID"
#define ALARM_PIR_ENABLE_KEY                        "Enable"
#define ALARM_PIR_CHECK_TIME_KEY                    "CheckTime"
#define ALARM_PIR_LINK_STARTEGY_KEY                 "LinkStrategy"
#define ALARM_PIR_SENSITIVE_KEY                     "Sensitive"
#define ALARM_PIR_LINK_ALARM_OUT_NO_KEY             "LinkAlarmOutNo"
#define ALARM_PIR_LINK_WHITE_LIGHT_DELAY_TIME_KEY   "LinkWhiteLightDelayTime" 
#define ALARM_PIR_ALARM_ID                          (2)
//enable
#define ALARM_PIR_ENABLE                            (1)
#define ALARM_PIR_CHECK_TIME                        (300)
//link white light
#define ALARM_PIR_LINK_STARTEGY                     (0x100)
//0~100
#define ALARM_PIR_SENSITIVE                         (50)
#define ALARM_PIR_LINK_ALARM_OUT_NO             	(0)
//5S
#define ALARM_PIR_LINK_WHITE_LIGHT_DELAY_TIME  		(5)

#define ALARM_IN_SCHEDULE_TIME_PATH                "/Config/Alarm/AlarmIn%d_Schedule/"
#define ALARM_IN_SCHEDULE_ID_KEY                   "ScheduleId"
#define ALARM_IN_SCHEDULE_INDEX_KEY                "Index"
#define ALARM_IN_SCHEDULE_START_TIME_KEY           "Week%d_StartTime%d"
#define ALARM_IN_SCHEDULE_END_TIME_KEY             "Week%d_EndTime%d"
#define ALARM_IN_SCHEDULE_ID                       (1)
#define ALARM_IN_SCHEDULE_INDEX                    (0)
#define ALARM_IN_SCHEDULE_START_TIME               (0)
#define ALARM_IN_SCHEDULE_END_TIME                 (24*60)

#define ALARM_OUT_SCHEDULE_TIME_PATH                "/Config/Alarm/AlarmOut%d_Schedule/"
#define ALARM_OUT_SCHEDULE_ID_KEY                   "ScheduleId"
#define ALARM_OUT_SCHEDULE_INDEX_KEY                "Index"
#define ALARM_OUT_SCHEDULE_START_TIME_KEY           "Week%d_StartTime%d"
#define ALARM_OUT_SCHEDULE_END_TIME_KEY             "Week%d_EndTime%d"
#define ALARM_OUT_SCHEDULE_ID                       (1)
#define ALARM_OUT_SCHEDULE_INDEX                    (0)
#define ALARM_OUT_SCHEDULE_START_TIME               (0)
#define ALARM_OUT_SCHEDULE_END_TIME                 (24*60)

#define PIR_SCHEDULE_TIME_PATH                		"/Config/Alarm/PIR_Schedule/"
#define PIR_SCHEDULE_ID_KEY                   		"ScheduleId"
#define PIR_SCHEDULE_START_TIME_KEY           		"Week%d_StartTime%d"
#define PIR_SCHEDULE_END_TIME_KEY             		"Week%d_EndTime%d"
#define PIR_SCHEDULE_ID                       		(3)
#define PIR_SCHEDULE_START_TIME               		(0)
#define PIR_SCHEDULE_END_TIME                 		(24*60)


// log server heartbeat interval
#define GMI_LOG_SERVER_HEARTBEAT_INTERVAL          1000 // ms unit

// media center server heartbeat interval
#define GMI_MEDIA_CENTER_SERVER_HEARTBEAT_INTERVAL 1000 // ms unit

// hardware monitor config
#define GMI_HARDWARE_CONFIG_FILE                    "/opt/config/capability_auto.xml"
#define GMI_HARDWARE_PATH                           "/Capability/"
#define GMI_DAEMON_CONFIG_FILE                      "/opt/config/ipc_daemon_config.xml"
#define GMI_DAEMON_PATH                             "/Capability/daemon/"

// log module id, name and debug log level definition
#define GMI_LOG_MODULE_DEFAULT_DEBUG_LOG_LEVEL                0
#define GMI_LOG_MODULE_DAEMON_ID                              1
#define GMI_LOG_MODULE_DAEMON_NAME                            "daemon"
#define GMI_LOG_MODULE_DAEMON_DEBUG_LOG_LEVEL                 GMI_LOG_MODULE_DEFAULT_DEBUG_LOG_LEVEL
#define GMI_LOG_MODULE_UPGRADE_ID                             2
#define GMI_LOG_MODULE_UPGRADE_NAME                           "upgrade"
#define GMI_LOG_MODULE_UPGRADE_DEBUG_LOG_LEVEL                GMI_LOG_MODULE_DEFAULT_DEBUG_LOG_LEVEL
#define GMI_LOG_MODULE_LOG_ID                                 3
#define GMI_LOG_MODULE_LOG_NAME                               "log"
// filter out log by log server
#define GMI_LOG_MODULE_LOG_DEBUG_LOG_LEVEL                    GMI_LOG_MODULE_DEFAULT_DEBUG_LOG_LEVEL
#define GMI_LOG_MODULE_MEDIA_CENTER_SERVER_ID                 4
#define GMI_LOG_MODULE_MEDIA_CENTER_SERVER_NAME               "media_center_server"
#define GMI_LOG_MODULE_MEDIA_CENTER_SERVER_DEBUG_LOG_LEVEL    GMI_LOG_MODULE_DEFAULT_DEBUG_LOG_LEVEL
#define GMI_LOG_MODULE_CONTROL_CENTER_SERVER_ID               5
#define GMI_LOG_MODULE_CONTROL_CENTER_SERVER_NAME             "control_center_server"
#define GMI_LOG_MODULE_CONTROL_CENTER_SERVER_DEBUG_LOG_LEVEL  GMI_LOG_MODULE_DEFAULT_DEBUG_LOG_LEVEL
#define GMI_LOG_MODULE_GB28181_ID                             6
#define GMI_LOG_MODULE_GB28181_NAME                           "gb28181"
#define GMI_LOG_MODULE_GB28181_DEBUG_LOG_LEVEL                GMI_LOG_MODULE_DEFAULT_DEBUG_LOG_LEVEL
#define GMI_LOG_MODULE_ONVIF_ID                               7
#define GMI_LOG_MODULE_ONVIF_NAME                             "onvif"
#define GMI_LOG_MODULE_ONVIF_DEBUG_LOG_LEVEL                  GMI_LOG_MODULE_DEFAULT_DEBUG_LOG_LEVEL
#define GMI_LOG_MODULE_PRIVATE_STREAM_ID                      8
#define GMI_LOG_MODULE_PRIVATE_STREAM_NAME                    "private_stream"
#define GMI_LOG_MODULE_PRIVATE_STREAM_DEBUG_LOG_LEVEL         GMI_LOG_MODULE_DEFAULT_DEBUG_LOG_LEVEL
#define GMI_LOG_MODULE_STORAGE_ID                             9
#define GMI_LOG_MODULE_STORAGE_NAME                           "storage"
#define GMI_LOG_MODULE_STORAGE_DEBUG_LOG_LEVEL                GMI_LOG_MODULE_DEFAULT_DEBUG_LOG_LEVEL
#define GMI_LOG_MODULE_WEB_ID                                 10
#define GMI_LOG_MODULE_WEB_NAME                               "web"
#define GMI_LOG_MODULE_WEB_DEBUG_LOG_LEVEL                    GMI_LOG_MODULE_DEFAULT_DEBUG_LOG_LEVEL
#define GMI_LOG_MODULE_AUTHENTICATION_ID                      11
#define GMI_LOG_MODULE_AUTHENTICATION_NAME                    "authentication"
#define GMI_LOG_MODULE_AUTHENTICATION_DEBUG_LOG_LEVEL         GMI_LOG_MODULE_DEFAULT_DEBUG_LOG_LEVEL
#define GMI_LOG_MODULE_GB_RTP_ID                              12
#define GMI_LOG_MODULE_GB_RTP_NAME                            "gb_rtp"
#define GMI_LOG_MODULE_GB_RTP_DEBUG_LOG_LEVEL                 GMI_LOG_MODULE_DEFAULT_DEBUG_LOG_LEVEL
#define GMI_LOG_MODULE_ONVIF_RTSP_ID                          13
#define GMI_LOG_MODULE_ONVIF_RTSP_NAME                        "onvif_rtsp"
#define GMI_LOG_MODULE_ONVIF_RTSP_DEBUG_LOG_LEVEL             GMI_LOG_MODULE_DEFAULT_DEBUG_LOG_LEVEL
#define GMI_LOG_MODULE_SDK_ID                                 14
#define GMI_LOG_MODULE_SDK_NAME                               "sdk"
#define GMI_LOG_MODULE_SDK_DEBUG_LOG_LEVEL                    GMI_LOG_MODULE_DEFAULT_DEBUG_LOG_LEVEL

#define LOG_SERVER_CONFIG_SHARE_MEMORY_SIZE                   "share_memory_size"
#define GMI_LOG_DEFAULT_SHARE_MEMORY_SIZE                     (1024*1024)

#define GMI_LOG_DEFAULT_USER_LOG_FILE_PATH                    "/opt/log/gmi_user.log"
// 1 indicates e_LogStorageLimitMode_RecordNumber
#define GMI_LOG_DEFAULT_USER_LOG_STORAGE_LIMIT_MODE           1
#define GMI_LOG_DEFAULT_USER_LOG_STORAGE_LIMIT_PARAMETER      2000
#define GMI_LOG_DEFAULT_USER_LOG_SHARE_MEMORY_SIZE            (512*1024)

#define GMI_H264_VIDEO_MONITOR_CONFIG_PATH                              "/Config/media_center/h264_video_monitor/"
#define GMI_H264_VIDEO_MONITOR_CONFIG_ENABLE_KEY_NAME                   "enable"
#define GMI_H264_VIDEO_MONITOR_CONFIG_ENABLE_VALUE                      1
#define GMI_H264_VIDEO_MONITOR_CONFIG_FRAME_CHECK_INTERVAL_KEY_NAME     "frame_check_interval"
#define GMI_H264_VIDEO_MONITOR_CONFIG_FRAME_CHECK_INTERVAL_VALUE        1500

#define GMI_DECODE_SOURCE_MONITOR_CONFIG_PATH                           "/Config/media_center/decode_source_monitor/"
#define GMI_DECODE_SOURCE_MONITOR_CONFIG_ENABLE_KEY_NAME                "enable"
#define GMI_DECODE_SOURCE_MONITOR_CONFIG_ENABLE_VALUE                   1
#define GMI_DECODE_SOURCE_MONITOR_CONFIG_FRAME_CHECK_INTERVAL_KEY_NAME  "frame_check_interval"
#define GMI_DECODE_SOURCE_MONITOR_CONFIG_FRAME_CHECK_INTERVAL_VALUE     1500

#define GMI_HARDWARE_DOG_CONFIG_PATH                                    "/Config/media_center/hardware_dog/"
#define GMI_HARDWARE_DOG_CONFIG_ENABLE_KEY_NAME                         "enable"
#define GMI_HARDWARE_DOG_CONFIG_ENABLE_VALUE                            0
#define GMI_HARDWARE_DOG_CONFIG_GUARD_TIME_KEY_NAME                     "guard_time"
#define GMI_HARDWARE_DOG_CONFIG_GUARD_TIME_VALUE                        60
#define GMI_HARDWARE_DOG_CONFIG_FEED_DOG_INTERVAL_KEY_NAME              "feed_dog_interval"
#define GMI_HARDWARE_DOG_CONFIG_FEED_DOG_INTERVAL_VALUE                 1

#endif//IPC_FW_V3X_SETTING 
