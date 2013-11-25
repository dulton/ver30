#ifndef __SERVICE_MEDIA_CONFIGURATION_H__
#define __SERVICE_MEDIA_CONFIGURATION_H__

#include "soapH.h"
#include "service_ptz.h"
#include "service_device_management.h"
#include "gmi_system_headers.h"


//video source 1
#define VIDEO_SOURCE_CONFIG_NAME            "Video_Source_Config_1"
#define VIDEO_SOURCE_CONFIG_TOKEN  			"Video_Source_Token_1"
#define VIDEO_SOURCE_CONFIG_SOURCE_TOKEN    "Video_Source_1"

//video source 2
#define VIDEO_SOURCE_2_CONFIG_NAME          "Video_Source_Config_2"
#define VIDEO_SOURCE_2_CONFIG_TOKEN         "Video_Source_Token_2"
#define VIDEO_SOURCE_CONFIG_SOURCE_2_TOKEN  "Video_Source_2"


//profiles1
#define PROFILE_TOKEN_1                     "Profile_1"
#define PROFILE_NAME_1                      "Stream_1"
//video encoder1
#define VIDEO_ENCODER_NAME_1                "Video_Encoder_1"
#define VIDEO_ENCODER_TOKEN_1               "Video_Encoder_Token_1"

//profiles2
#define PROFILE_TOKEN_2                     "Profile_2"
#define PROFILE_NAME_2                      "Stream_2"
//video encoder2
#define VIDEO_ENCODER_NAME_2                "Video_Encoder_2"
#define VIDEO_ENCODER_TOKEN_2               "Video_Encoder_Token_2"

//profiles3
#define PROFILE_TOKEN_3                     "Profile_3"
#define PROFILE_NAME_3                      "Stream_3"
//video encoder3
#define VIDEO_ENCODER_NAME_3                "Video_Encoder_3"
#define VIDEO_ENCODER_TOKEN_3               "Video_Encoder_Token_3"

//profiles4
#define PROFILE_TOKEN_4                     "Profile_4"
#define PROFILE_NAME_4                      "Stream_4"
//video encoder4
#define VIDEO_ENCODER_NAME_4                "Video_Encoder_4"
#define VIDEO_ENCODER_TOKEN_4               "Video_Encoder_Token_4"


//default media stream
#define MAX_STREAM_NUM                      (4)
#define MAX_SOURCE_NUM                      (1)
#define DEFAULT_STREAM_NUM                  (2)
#define DEFAULT_VIDEO_SOURCES               (1)
#define DEFAULT_VIDEO_OUTPUTS               (0)
#define DEFAULT_AUDIO_SOURCES               (0)
#define DEFAULT_AUIDO_OUTPUTS               (0)

//support streaming protocl
#define SUPPORT_RTP_USCORETCP               (1)
#define SUPPORT_RTP_USCORERTSP_USCORETCP    (1)

//Video source resolution
#define DEFAULT_VIDEO_SOURCE_RES_WIDTH      (1920)
#define DEFAULT_VIDEO_SOURCE_RES_HEIGHT     (1080)
#define DEFAULT_VIDEO_SOURCE_FRAME_RATE     (25)

//Video streams
#define DEFAULT_VIDEO_MAIN_STREAM_RES_WIDTH    (1280)
#define DEFAULT_VIDEO_MAIN_STREAM_RES_HEIGHT   (720)
#define DEFAULT_VIDEO_MAIN_STREAM_FRAME_RATE   (DEFAULT_VIDEO_SOURCE_FRAME_RATE)
#define DEFAULT_VIDEO_MAIN_STREAM_BITRATE      (4000)
#define DEFAULT_VIDEO_MAIN_STREAM_I_INTERVAL   (30)
#define DEFAULT_VIDEO_SECOND_STREAM_RES_WIDTH  (720)
#define DEFAULT_VIDEO_SECOND_STREAM_RES_HEIGHT (576)
#define DEFAULT_VIDEO_SECOND_STREAM_FRAME_RATE (DEFAULT_VIDEO_SOURCE_FRAME_RATE)
#define DEFAULT_VIDEO_SECOND_STREAM_BITRATE    (1500)
#define DEFAULT_VIDEO_SECOND_STREAM_I_INTERVAL (30)
#define DEFAULT_VIDEO_SESSION_TIMEOUT          (5000)

//frame rate range
#define MAX_FRAME_RATE                          (25)
#define MIN_FRAME_RATE                          (1)
//I Interval
#define MAX_I_INTERVAL                          (150)
#define MIN_I_INTERVAL                           (2)
//Encoding Interval
#define MAX_ENCODING_INTERVAL                    (25)
#define MIN_ENCODING_INTERVAL                    (1)
//quality
#define MAX_QUALITY                              (6)
#define MIN_QUALITY                              (1)
//resolutions
#define RES_1080P_WIDTH                          (1920)
#define RES_1080P_HEIGHT                         (1080)
#define RES_720P_WIDTH                           (1280)
#define RES_720_HEIGHT                           (720)
#define RES_D1_WIDTH                             (720)
#define RES_D1_HEIGHT                            (576)
#define RES_CIF_WIDTH                            (352)
#define RES_CIF_HEIGHT                           (288)
#define RES_QCIF_WIDTH                           (192)
#define RES_QCIF_HEIGHT                          (144)

//imaging
typedef struct tagImaging
{
    float                             s_Brightness;
    float                             s_ColorSaturation;
    float                             s_Contrast;
    float                             s_Sharpness;
    enum tt__IrCutFilterMode          s_IrCutFilter;
    struct tt__Exposure               s_Exposure;
    struct tt__FocusConfiguration     s_Focus;
    struct tt__BacklightCompensation  s_BacklightCompensation;
    struct tt__WideDynamicRange       s_WideDynamicRange;
    struct tt__WhiteBalance           s_WhiteBalance;
} Imaging;


//video source
typedef struct tagMediaVideoSource
{
    char_t  s_VideoConfigName[TOKEN_LENGTH];
    char_t  s_VideoConfigToken[TOKEN_LENGTH];
    char_t  s_VideoSourceToken[TOKEN_LENGTH];
    int32_t s_SrcWidth;
    int32_t s_SrcHeight;
    int32_t s_SrcFrameRate;
    Imaging s_Imaging;
} MediaVideoSource;


//video source configuration
typedef struct tagVideoSourceConfiguration
{
    char_t   s_Name[TOKEN_LENGTH];
    int32_t  s_UseCount;
    char_t   s_Token[TOKEN_LENGTH];
    char_t   s_SourceToken[TOKEN_LENGTH];
    struct tt__IntRectangle s_Bounds;
} VideoSourceConfiguration;


//multicast
typedef struct tt_MulticastConfiguration
{
    IPAddress         s_Address;
    int32_t           s_Port;
    int32_t           s_TTL;
    enum xsd__boolean s_AutoStart;
} MulticastConfiguration;


//VideoEncoderConfiguration
typedef struct tagVideoEncoderConfiguration
{
    char_t                            s_Name[TOKEN_LENGTH];
    int32_t                           s_UseCount;
    char_t                            s_Token[TOKEN_LENGTH];
    enum tt__VideoEncoding            s_Encoding;
    float                             s_Quality;
    struct tt__VideoResolution        s_Resolution;
    struct tt__VideoRateControl       s_RateControl;
    struct tt__Mpeg4Configuration     s_MPEG4;
    struct tt__H264Configuration      s_H264;
    MulticastConfiguration            s_Multicast;
    longlong_t                        s_SessionTimeout;
} VideoEncoderConfiguration;


//H264Options
typedef struct tagH264Options
{
    int                        s_sizeResolutionsAvailable;
    struct tt__VideoResolution s_ResolutionsAvailable[16];
    struct tt__IntRange        s_GovLengthRange;
    struct tt__IntRange        s_FrameRateRange;
    struct tt__IntRange        s_EncodingIntervalRange;
    int                        s_sizeH264ProfilesSupported;
    enum tt__H264Profile       s_H264ProfilesSupported[4];
} H264Options;


//VideoEncoderConfigurationOptions
typedef struct tagVideoEncoderConfigurationOptions
{
    struct tt__IntRange     s_QualityRange;
    H264Options             s_H264Options;
} VideoEncoderConfigurationOptions;


//profiles
typedef struct
{
    enum xsd__boolean                 s_Fixed;
    char_t                            s_Name[TOKEN_LENGTH];
    char_t                            s_Token[TOKEN_LENGTH];
    VideoSourceConfiguration          s_VideoSourceConfiguration;
    VideoEncoderConfiguration         s_VideoEncoderConfiguration;
    PTZ_Configuration                 s_PTZ_Configuration;
} Profile;

#endif

