#ifndef __SERVICE_PTZ_H__
#define __SERVICE_PTZ_H__

//max ptz num
#define DEFAULT_PTZ_NUM                            1
//include Meachnical and digital PTZ space
#define DEFAULT_PTZ_SPACE_NUM                      2
//max ptz presets num
#define MAX_PTZ_PRESET_NUM                         256

//ptz_0
#define PTZ_0                                      (0)
#define PTZ_NAME_0                                 "PTZ_0"
#define PTZ_TOKEN_0                                "pToken_PTZ_0"
#define PTZ_NODE_TOKEN_0                           "pNodeToken_PTZ_0"
#define PTZ_NODE_NAME_0                            "PTZ_NODE_0"
//space
#define ABSOLUTE_PANTILT_POSITION_SPACE            "http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace"
#define ABSOLUTE_ZOOM_POSITION_SPACE               "http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace"
#define RELATIVE_PANTILT_TRANSLATION_SPACE         "http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace"
#define RELATIVE_ZOOM_TRANSLATION_SPACE            "http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace"
#define CONTINUOUS_PANTILT_VELOCITY_SPACE          "http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace"
#define CONTINUOUS_ZOOM_VELOCITY_SPACE             "http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace"
#define GENERIC_SPEED_SPACE                        "http://www.onvif.org/ver10/tptz/PanTiltSpaces/GenericSpeedSpace"
#define GENERIC_ZOOM_SPEED_SPACE                   "http://www.onvif.org/ver10/tptz/ZoomSpaces/ZoomGenericSpeedSpace"
//digital ptz space
#define ABSOLUTE_PANTILT_DIGITAL_POSITION_SPACE    "http://www.onvif.org/ver10/tptz/PanTiltSpaces/DigitalPositionSpace"
#define ABSOLUTE_ZOOM_DIGITAL_POSITON_SPACE        "http://www.onvif.org/ver10/tptz/ZoomSpaces/NormalizedDigitalPositionSpace"
#define RELATIVE_PANTILT_DIGITAL_TRANSLATION_SPACE "http://www.onvif.org/ver10/tptz/PanTiltSpaces/DigitalTranslationSpace"
#define RELATIVE_ZOOM_DIGITAL_TRANSLATION_SPACE    "http://www.onvif.org/ver10/tptz/ZoomSpaces/NormalizedDigitalTranslationSpace"
#define CONTINUOUS_PANTILT_DIGITAL_SPACE           "http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocitySpaceFOV"
#define CONTINUOUS_ZOOM_DIGITAL_SPACE              "http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocitySpaceMillimeter"

#define PTZ_NAME_PREFIX "PTZ_"
#define PTZ_TOKEN_PREFIX "pToken_PTZ_"
#define PTZ_NODE_TOKEN_PREFIX "pNodeToken_PTZ_"

#define PTZ_PRESET_TOKEN_PREFIX                    "PresetToken_"

// mesuare unit : ms
//Really, Actual value should acquire from sys_control_server
#define DEFAULT_PTZ_TIMEOUT         (60000)
#define DEFAULT_MIN_PTZ_TIMEOUT     (1000)
#define DEFAULT_MAX_PTZ_TIMEOUT     (600000)

//PTZ Speed Rage
#define PANTILT_XRAGE                            (1.0)
#define PANTILT_YRAGE                            (1.0)
#define ZOOM_XRAGE                               (1.0)
#define PANTILT_XRAGE_MAX                        (1.0)
#define PANTILT_XRAGE_MIN                        (-1.0)
#define PANTILT_YRAGE_MAX                        (1.0)
#define PANTILT_YRAGE_MIN                        (-1.0)
#define ZOOM_XRAGE_MAX                           (1.0)
#define ZOOM_XRAGE_MIN                           (-1.0)
#define CONTINOUS_DIGITIAL_PANTILT_XRANGE_MAX    (7.0)
#define CONTINOUS_DIGITIAL_PANTILT_XRANGE_MIN    (-7.0)
#define CONTINOUS_DIGITIAL_PANTILT_YRANGE_MAX    (7.0)
#define CONTINOUS_DIGITIAL_PANTILT_YRANGE_MIN    (-7.0)
#define CONTINOUS_DIGITIAL_ZOOM_XRANGE_MAX       (7.0)
#define CONTINOUS_DIGITIAL_ZOOM_XRANGE_MIN       (-7.0)


//vector 2D
struct tt_Vector2D
{
    float         x;
    float         y;
    const char_t *space;
};

//vector 1D
struct tt_Vector1D
{
    float         x;
    const char_t *space;
};

//space 2D
typedef struct
{
    const char_t  *s_URI;
    float          s_XRangeMin;
    float          s_XRangeMax;
    float          s_YRangeMin;
    float          s_YRangeMax;
} Space2DDescription;

//space 1D
typedef struct
{
    const char_t  *s_URI;
    float          s_XRangeMin;
    float          s_XRangeMax;
} Space1DDescription;

//ptz speed
typedef struct tagPTZSpeed
{
    struct tt_Vector2D s_PanTilt;
    struct tt_Vector1D s_Zoom;
} PTZ_Speed;

//PanTilt Limits
typedef struct tagPanTiltLimits
{
    Space2DDescription s_Range;
} PanTiltLimits;

//Zoom Limits
typedef struct tagZoomLimits
{
    Space1DDescription s_Range;
} ZoomLimits;

//ptz configuration
typedef struct tagPTZConfiguration
{
    const char_t *s_Name;
    int32_t       s_UseCount;
    const char_t *s_Token;
    const char_t *s_NodeToken;
    const char_t *s_DefaultAbsolutePantTiltPositionSpace;
    const char_t *s_DefaultAbsoluteZoomPositionSpace;
    const char_t *s_DefaultRelativePanTiltTranslationSpace;
    const char_t *s_DefaultRelativeZoomTranslationSpace;
    const char_t *s_DefaultContinuousPanTiltVelocitySpace;
    const char_t *s_DefaultContinuousZoomVelocitySpace;
    PTZ_Speed     s_PTZ_Speed;
    longlong_t    s_DefaultTimeout;
    PanTiltLimits s_PanTiltLimits;
    ZoomLimits    s_ZoomLimits;
} PTZ_Configuration;

//ptz configuration options
typedef struct tagPTZConfigurationOptions
{
    Space2DDescription        s_AbsolutePanTiltPositionSpace[DEFAULT_PTZ_SPACE_NUM];
    Space1DDescription        s_AbsoluteZoomPositionSpace[DEFAULT_PTZ_SPACE_NUM];
    Space2DDescription        s_RelativePanTiltTranslationSpace[DEFAULT_PTZ_SPACE_NUM];
    Space1DDescription        s_RelativeZoomTranslationSpace[DEFAULT_PTZ_SPACE_NUM];
    Space2DDescription        s_ContinuousPanTiltVelocitySpace[DEFAULT_PTZ_SPACE_NUM];
    Space1DDescription        s_ContinuousZoomVelocitySpace[DEFAULT_PTZ_SPACE_NUM];
    Space1DDescription        s_PanTiltSpeedSpace;
    Space1DDescription        s_ZoomSpeedSpace;
    struct tt__DurationRange  s_PTZTimeoutRange;
} PTZ_ConfigurationOptions;

//PTZ NODE
typedef struct tagPTZNode
{
    char_t 					  s_token[TOKEN_LENGTH];
    char_t 					  s_Name[TOKEN_LENGTH];
    Space2DDescription        s_AbsolutePanTiltPositionSpace[DEFAULT_PTZ_SPACE_NUM];
    Space1DDescription        s_AbsoluteZoomPositionSpace[DEFAULT_PTZ_SPACE_NUM];
    Space2DDescription        s_RelativePanTiltTranslationSpace[DEFAULT_PTZ_SPACE_NUM];
    Space1DDescription        s_RelativeZoomTranslationSpace[DEFAULT_PTZ_SPACE_NUM];
    Space2DDescription        s_ContinuousPanTiltVelocitySpace[DEFAULT_PTZ_SPACE_NUM];
    Space1DDescription        s_ContinuousZoomVelocitySpace[DEFAULT_PTZ_SPACE_NUM];
    Space1DDescription        s_PanTiltSpeedSpace;
    Space1DDescription        s_ZoomSpeedSpace;
    int32_t                   s_MaximumNumberOfPresets;
    enum xsd__boolean         s_HomeSupported;
} PTZ_Node;


//PTZ Preset
typedef struct tagPTZPreset
{
    char_t    s_Token[TOKEN_LENGTH];
    char_t    s_Name[TOKEN_LENGTH];
    int       s_Index;
    boolean_t s_Setted;
} PTZ_Preset;

//func declaration
GMI_RESULT __tptz__Initialize();
void __tptz__Deinitialize();


//extern ptz
extern PTZ_Configuration g_PTZ_Coniguration[DEFAULT_PTZ_NUM];

#endif

