/*
 *  Communication protocl for system control server
 *  Copyright (c) 2013/03/02 by guoqiang.lu <guoqiang.lu@gm-innovation.com>
 */
#ifndef __SYS_ENV_TYPES_H__
#define __SYS_ENV_TYPES_H__

#include "gmi_system_headers.h" 

#ifdef __cplusplus
extern "C" {
#endif 

//pkg code
#define SYSCODE_SET_IPINFO_REQ            1011
#define SYSCODE_SET_IPINFO_RSP            1012
#define SYSCODE_GET_IPINFO_REQ            1013
#define SYSCODE_GET_IPINFO_RSP            1014
#define SYSCODE_SET_USERINFO_REQ          1021 
#define SYSCODE_SET_USERINFO_RSP          1022
#define SYSCODE_GET_USERINFO_REQ          1023
#define SYSCODE_GET_USERINFO_RSP          1024
#define SYSCODE_DEL_USERINFO_REQ          1025
#define SYSCODE_DEL_USERINFO_RSP          1026
#define SYSCODE_SET_ENCODECFG_REQ         1031
#define SYSCODE_SET_ENCODECFG_RSP         1032
#define SYSCODE_GET_ENCODECFG_REQ         1033
#define SYSCODE_GET_ENCODECFG_RSP         1034
#define SYSCODE_SET_SHOWCFG_REQ           1041
#define SYSCODE_SET_SHOWCFG_RSP           1042
#define SYSCODE_GET_SHOWCFG_REQ           1043
#define SYSCODE_GET_SHOWCFG_RSP           1044
#define SYSCODE_SET_DEVICEINFO_REQ        1051
#define SYSCODE_SET_DEVICEINFO_RSP        1052
#define SYSCODE_GET_DEVICEINFO_REQ        1053
#define SYSCODE_GET_DEVICEINFO_RSP        1054
#define SYSCODE_SET_TIME_REQ              1061
#define SYSCODE_SET_TIME_RSP              1062
#define SYSCODE_GET_TIME_REQ              1063
#define SYSCODE_GET_TIME_RSP              1064
#define SYSCODE_3DPTZ_CTL_REQ             1071
#define SYSCODE_3DPTZ_CTL_RSP             1072
#define SYSCODE_SET_PTZHOME_REQ           1073
#define SYSCODE_SET_PTZHOME_RSP           1074
#define SYSCODE_GET_PTZHOME_REQ           1075
#define SYSCODE_GET_PTZHOME_RSP           1076
#define SYSCODE_CTL_PTZ_REQ               1077
#define SYSCODE_CTL_PTZ_RSP               1078 
#define SYSCODE_SET_PTZPRESET_REQ         1081
#define SYSCODE_SET_PTZPRESET_RSP         1082
#define SYSCODE_GET_PTZPRESET_REQ         1083
#define SYSCODE_GET_PTZPRESET_RSP         1084
#define SYSCODE_SEARCH_PTZPRESET_REQ      1085
#define SYSCODE_SEARCH_PTZPRESET_RSP      1086
#define SYSCODE_CTL_SYSTEM_REQ            1091
#define SYSCODE_CTL_SYSTEM_RSP            1092
#define SYSCODE_FIND_STORAGEFILE_REQ      1101
#define SYSCODE_FIND_STORAGEFILE_RSP      1102
#define SYSCODE_GET_STORAGEFILE_REQ       1103
#define SYSCODE_GET_STORAGEFILE_RSP       1104
#define SYSCODE_SET_HIDEAREA_REQ          1121
#define SYSCODE_SET_HIDEAREA_RSP          1122
#define SYSCODE_GET_HIDEAREA_REQ          1123
#define SYSCODE_GET_HIDEAREA_RSP          1124
#define SYSCODE_GET_PERFORMANCE_REQ       1131
#define SYSCODE_GET_PERFORMANCE_RSP       1132
#define SYSCODE_SET_ALMDEPLOY_REQ         1141
#define SYSCODE_SET_ALMDEPLOY_RSP         1142
#define SYSCODE_GET_ALMDEPLOY_REQ         1143
#define SYSCODE_GET_ALMDEPLOY_RSP         1144
#define SYSCODE_SET_ALMCFG_REQ            1151
#define SYSCODE_SET_ALMCFG_RSP            1152
#define SYSCODE_GET_ALMCFG_REQ            1153
#define SYSCODE_GET_ALMCFG_RSP            1154
#define SYSCODE_EXTERNAL_CTRL_REQ         1161
#define SYSCODE_EXTERNAL_CTRL_RSP         1162
#define SYSCODE_GET_IMAGING_REQ           1171
#define SYSCODE_GET_IMAGING_RSP           1172
#define SYSCODE_SET_IMAGING_REQ           1173
#define SYSCODE_SET_IMAGING_RSP           1174
#define SYSCODE_GET_VIDEO_SOURCE_INFO_REQ 1175
#define SYSCODE_GET_VIDEO_SOURCE_INFO_RSP 1176
#define SYSCODE_GET_ADVANCED_IMAGING_REQ  1177
#define SYSCODE_GET_ADVANCED_IMAGING_RSP  1178
#define SYSCODE_SET_ADVANCED_IMAGING_REQ  1179
#define SYSCODE_SET_ADVANCED_IMAGING_RSP  1180
#define SYSCODE_FORCE_IDR_REQ             1181
#define SYSCODE_FORCE_IDR_RSP             1182
#define SYSCODE_GET_DEVICE_WORK_STATE_REQ 1191
#define SYSCODE_GET_DEVICE_WORK_STATE_RSP 1192
#define SYSCODE_GET_NETWORK_PORT_REQ      1193
#define SYSCODE_GET_NETWORK_PORT_RSP      1194    
#define SYSCODE_SET_NETWORK_PORT_REQ      1195
#define SYSCODE_SET_NETWORK_PORT_RSP      1196
#define SYSCODE_GET_CAPABILITIES_REQ      1197
#define SYSCODE_GET_CAPABILITIES_RSP      1198
#define SYSCODE_GET_ENCSTREAM_COMBINE_REQ 1201
#define SYSCODE_GET_ENCSTREAM_COMBINE_RSP 1202
#define SYSCODE_SET_ENCSTREAM_COMBINE_REQ 1203
#define SYSCODE_SET_ENCSTREAM_COMBINE_RSP 1204
#define SYSCODE_GET_AUDIO_ENCODE_REQ      1205    
#define SYSCODE_GET_AUDIO_ENCODE_RSP      1206
#define SYSCODE_SET_AUDIO_ENCODE_REQ      1207
#define SYSCODE_SET_AUDIO_ENCODE_RSP      1208
#define SYSCODE_GET_AUTOFOCUS_REQ         1211
#define SYSCODE_GET_AUTOFOCUS_RSP         1212
#define SYSCODE_SET_AUTOFOCUS_REQ         1213
#define SYSCODE_SET_AUTOFOCUS_RSP         1214
#define SYSCODE_GET_WHITEBALANCE_REQ      1215
#define SYSCODE_GET_WHITEBALANCE_RSP      1216
#define SYSCODE_SET_WHITEBALANCE_REQ      1217
#define SYSCODE_SET_WHITEBALANCE_RSP      1218
#define SYSCODE_SET_VIDEO_SOURCE_INFO_REQ 1221
#define SYSCODE_SET_VIDEO_SOURCE_INFO_RSP 1222
#define SYSCODE_GET_DAY_NIGHT_REQ         1223
#define SYSCODE_GET_DAY_NIGHT_RSP         1224
#define SYSCODE_SET_DAY_NIGHT_REQ         1225
#define SYSCODE_SET_DAY_NIGHT_RSP         1226
#define SYSCODE_GET_LOGINFO_REQ           1227
#define SYSCODE_GET_LOGINFO_RSP           1228
#define SYSCODE_GET_ALARM_REQ             1229
#define SYSCODE_GET_ALARM_RSP             1230

//inner use
#define SYSCODE_START_AUDIO_DECODE_REQ    2002 
#define SYSCODE_START_AUDIO_DECODE_RSP    2003
#define SYSCODE_STOP_AUDIO_DECODE_REQ     2004
#define SYSCODE_STOP_AUDIO_DECODE_RSP     2005
#define SYSCODE_IMPORT_EXCUTE_REQ         2006
#define SYSCODE_IMPORT_EXCUTE_RSP         2007
#define SYSCODE_STOP_3A_REQ               2008
#define SYSCODE_STOP_3A_RSP               2009
#define SYSCODE_REPORT_CONFIG             2001


//pkg attrubite
#define TYPE_INTVALUE           1
#define TYPE_STRINGVALUE        2
#define TYPE_IPINFOR            10
#define TYPE_NETWORK_INTERFACE  12
#define TYPE_MESSAGECODE        54
#define TYPE_USERINFOR          11
#define TYPE_ENCODECFG          24
#define TYPE_SHOWCFG            13
#define TYPE_IPCNAME            101
#define TYPE_SVCADDR            102
#define TYPE_TIMEZONE           108
#define TYPE_NTPSERVER          107
#define TYPE_SYSTIME            15
#define TYPE_CTLPTZ             17
#define TYPE_PTZPRESET          18
#define TYPE_3DCTLPTZ           19
#define TYPE_STORAGEFILE_SEARCH 63
#define TYPE_STORAGEFILE_DATA   64
#define TYPE_STORAGEFILE        20
#define TYPE_DISKINFOR          21
#define TYPE_LOGINFOR_SEARCH    22
#define TYPE_LOGINFOR           23
#define TYPE_SERIALPORT         25
#define TYPE_CTLSERIALPORT      60
#define TYPE_SERIALPORTDATA     61
#define TYPE_HIDEAREA           31
#define TYPE_PERFORMANCE        32
#define TYPE_ALMDEPLOY          66
#define TYPE_ALMSUBSCRIPTION    67
#define TYPE_ALARM              81
#define TYPE_VIDEODATA          34
#define TYPE_AUDIODATA          35
#define TYPE_DATAACK            39
#define TYPE_CTLALMCFG          82
#define TYPE_EXTERNAL_CTRL      85
#define TYPE_ENVDATA            83
#define TYPE_OSD_SEARCH         14
#define TYPE_PTZPRESET_SEARCH   16
#define TYPE_PTZHOME            93	
#define TYPE_IMAGING            40
#define TYPE_VIDEO_SOURCE       41
#define TYPE_FORCEIDR           42
#define TYPE_TIMETYPE           109
#define TYPE_ADVANCED_IMAGING   110
#define TYPE_WORK_STATE         111
#define TYPE_NETWORK_PORT       112
#define TYPE_XML                113
#define TYPE_ENCSTREAM_COMBINE  114
#define TYPE_AUDIO_ENCODECFG    115
#define TYPE_AUTO_FOCUS         116
#define TYPE_WHITE_BALANCE      117
#define TYPE_DAY_NIGHT          118
#define TYPE_LOGINFO_SEARCH     119
#define TYPE_LOGINFO            120
#define TYPE_LOGINFO_INT        121
#define TYPE_WARING_INFO        122

//inner use
#define TYPE_AUDIO_DECODE       201
#define TYPE_CONFIG_FILE        202

//error code
#define RET_TYPE_SUCCESS         0
#define RET_TYPE_FAIL            1
#define MODULE_TYPE_DEVICE       0
#define MODULE_TYPE_MEDIA        1
#define MODULE_TYPE_PTZ          2
#define MODULE_TYPE_RECORD       3
#define MODULE_TYPE_LOG_INFO     4
#define MODULE_TYPE_EVENT        5
#define MAKE_RETCODE(r, m, e)   ((r<<31)+(m<<16)+e)
#define RETCODE_OK              MAKE_RETCODE(RET_TYPE_SUCCESS, MODULE_TYPE_DEVICE, 0)
#define RETCODE_ERROR           MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_DEVICE, 1)
#define RETCODE_NOUSER          MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_DEVICE, 2)
#define RETCODE_ERRPASSWD       MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_DEVICE, 3)
#define RETCODE_ERRSESSIONID    MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_DEVICE, 4)
#define RETCODE_TOOMANYLINKS_ONEUSER MAKE_RETCODE(RET_TYPE_FAIL, MODULE_TYPE_DEVICE, 5)
#define RETCODE_TOOMANYUSERS    MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_DEVICE, 6)
#define RETCODE_NOSUPPORT       MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_DEVICE, 7)
#define RETCODE_NORIGHT         MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_DEVICE, 8)
#define RETCODE_NOSERIALPORT    MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_DEVICE, 9)
#define RETCODE_NOALARMINPORT   MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_DEVICE, 10)
#define RETCODE_NOALARMOUTPORT  MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_DEVICE, 11)
#define RETCODE_NOVIDEOCHANNEL  MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_DEVICE, 12)
#define RETCODE_ERRORDATA       MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_DEVICE, 13)
#define RETCODE_NODISK          MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_RECORD, 1)
#define RETCODE_NOSPECDISK      MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_RECORD, 2)
#define RETCODE_FORMATING       MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_RECORD, 3)
#define RETCODE_OTHERUSER_FORMAITING MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_RECORD, 4)
#define RETCODE_IP_INVAILD      MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_RECORD, 5)
#define RETCODE_IP_CONFLICT     MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_RECORD, 6)
#define RETCODE_SYSTEM_RNNING   MAKE_RETCODE(RET_TYPE_FAIL,    MODULE_TYPE_RECORD, 7)


#define SYS_COMM_VERSION            1
#define GMI_SYS_MESSAGE_TAG_LENGTH  4
#define GMI_SYS_MESSAGE_TAG         "GSMT"

//use inner
#define GMI_TRANSFER_KEY_TAG_LENGTH 4
#define GMI_TRANSFER_KEY_TAG        "GTKT"
//use inner
typedef struct 
{
	uint8_t  s_TransferKey[4];
	uint16_t s_SessionId;
	uint16_t s_SeqNumber;
	uint32_t s_AuthValue;
}SdkPkgTransferProtocolKey;

//use inner
typedef struct
{
    uint32_t s_PtzId;
	uint32_t s_Index;
	char_t   s_Name[128];
	int32_t  s_Setted; // 1--have setted, 0--not Setted
	int32_t  s_ZoomPosition;
	int32_t  s_Reserved[4];	
}SysPkgPresetInfo_Inner;

//preset total number
#define MAX_PRESETS (256)
//user total number
#define MAX_USERS   (32)

//func module
typedef enum tagConfigModule
{
    SYS_CONFIG_MODULE_ALL = 0,
	SYS_CONFIG_MODULE_IP_INFO,
	SYS_CONFIG_MODULE_USER_INFO,
	SYS_CONFIG_MODULE_IMAGING,
	SYS_CONFIG_MODULE_VIDEO_ENCODE,
	SYS_CONFIG_MODULE_SHOW,
	SYS_CONFIG_MODULE_DEVICE_INFO,
	SYS_CONFIG_MODULE_TIME,
	SYS_CONFIG_MODULE_PTZ_HOME,
	SYS_CONFIG_MODULE_PTZ_PRESET,
	SYS_CONFIG_MODULE_HIDE_AREA,
	SYS_CONFIG_MODULE_STREAM_COMBINE
}SysPkgConfigModule;


typedef struct
{
	char_t  s_FileTmpPath[128];
	char_t  s_FileTargetPath[128];
	int32_t s_Persit; // 1-persit, 0-delete 
	int32_t s_Encrypt;// 0-no encrypt, 2-aes
	uint8_t s_Reserved[8];
}SysPkgConfigFileInfo;


//package header
typedef struct tagSysPkgHeader
{
	uint8_t  s_SysMsgTag[GMI_SYS_MESSAGE_TAG_LENGTH];
	uint8_t  s_Version;
	uint8_t  s_HeaderLen;
	uint16_t s_Code;
	uint16_t s_AttrCount;
	uint16_t s_SeqNum;
	uint16_t s_SessionId;
	uint16_t s_TotalLen;
}SysPkgHeader; 


//package attribute header
typedef struct tagSysPkgAttrHeader
{
	uint16_t s_Type;	
	uint16_t s_Length;
}SysPkgAttrHeader;


//message code
typedef struct tagSysMessageCode
{
	int32_t	 s_MessageCode;
	int32_t  s_MessageLen;
	char_t   s_Message[0];
}SysPkgMessageCode;


/*===device management=== */
//just only superuser(manufactory tools) can write it.
//cpu
typedef enum tagSysCpuId
{
    e_CPU_A5S_55  = 1,
    e_CPU_A5S_66,
	e_CPU_A5S_88
}SysPkgCpuId;


//just only superuser(manufactory tools) can write it. 
//sensor
typedef enum tagSysSensorId
{
    e_SENSOR_MN34041  = 1,
    e_SENSOR_IMX122,
    e_SENSOR_OV2715,
    e_SENSOR_OV9715,
    e_SENSOR_TW9910
}SysPkgSensorId;


//just only superuser(manufactory tools) can write it.
typedef enum tagSysAlarmPortType
{
    e_ALM_PORT_NATIVE_GPIO   = 1,
    e_ALM_PORT_I2C_GPIO
}SysPkgAlarmPortType;


//just only superuser(manufactory tools) can write it.
typedef struct tagSysFuncSupport
{
    uint64_t  s_SupportWDR      : 1; //true -- support wide daynmic range, false -- not support wide daynmic range.
    uint64_t  s_SupportIR       : 1; //true -- support IR, false -- not support IR.
    uint64_t  s_SupportLowLum   : 1; //true -- support low lum, false -- not support low lum;
    uint64_t  s_SupportZoomLens : 1; //true -- support zoom lens,false -- not support zoom lens;
    uint64_t  s_SupportDCIRIS   : 1;
}SysPkgMisceFuncBit;


//just only superuser(manufactory tools) can write it.
typedef enum tagSysIRCutCtrlType
{
    e_IRCUT_CTRL_GPIO  = 1,
    e_IRCUT_CTRL_SPI, 
}SysPkgIRCutCtrlType;


//just only superuser(manufactory tools) can write it.
typedef enum tagSysIRCtrlType
{
	e_IRLED_CTRL_GPIO  = 1,
	e_IRLED_CTRL_PWM,
	e_IRLED_CTRL_SERIAL,
	e_IRLED_CTRL_ALARM_INPUT,
	e_IRLED_CTRL_NONE  = 255
}SysPkgIRCtrlType;


//just only superuser(manufactory tools) can write it.
typedef enum tagSysShieldType
{
	e_SHIELD_TIAN_JING  = 1,
	e_SHIELD_HONG_BEN,
	e_SHIELD_NONE = 255
}SysPkgShieldType;


//just only superuser(manufactory tools) can write it.
typedef struct tagSysNetworkType
{
    uint8_t  s_SupportEth  : 1;
    uint8_t  s_SupportEth1 : 1;
    uint8_t  s_SupportWIFI : 1;
    uint8_t  s_Support3G   : 1;
}SysPkgNetworkTypeBit;


//just only superuser(manufactory tools) can write it.
typedef enum tagSysVideoPortType
{
    e_VIDEO_PORT_CVBS  = 1,
    e_VIDEO_PORT_HDMI,
    e_VIDEO_PORT_VGA,
    e_VIDEO_PORT_DVI,
    e_VIDEO_PORT_NONE  = 255
}SysPkgVideoPortType;


//just only superuser(manufactory tools) can write it.
typedef enum tagSysIRCutMaterialType
{
    e_IRCUT_MATERIAL_GREEN_RED_GLASS = 1,
    e_IRCUT_MATERIAL_GREEN_BLUE_GLASS,
}SysPkgIRCutMaterialType;


typedef enum tagSysZoomLensId
{
	  e_ZOOM_LENS_DF003 = 1,//18x,TAMRON
	  e_ZOOM_LENS_YB22,//22x,FUJI
	  e_ZOOM_LENS_NONE  = 255
}SysPkgZoomLensId;


//just only superuser(manufactory tools) can write it.
typedef struct tagSysInterface
{
    uint8_t               s_AlarmInputNum;//observe from alarm port0 alarm port 1 to use 
    uint8_t               s_AlarmOutputNum;//observe from alarm port0 alarm port 1 to use 
    uint8_t               s_RS232Num;
    uint8_t               s_RS485Num;
    uint8_t               s_PTZNum;
    uint8_t               s_EthPortNum;
    uint8_t               s_DiskNum;
    uint8_t               s_USBNum;
    uint8_t               s_VideoOutNum;
    uint8_t               s_VideoInNum;
    uint8_t               s_AudioInNum;
    uint8_t               s_AudioOutNum;
    uint8_t               s_VideoInType;//e_VIDEO_PORT_NONE/e_VIDEO_PORT_CVBS/e_VIDEO_PORT_HDMI/e_VIDEO_PORT_VGA/e_VIDEO_PORT_DVI
    uint8_t               s_VideoOutType;//e_VIDEO_PORT_NONE/e_VIDEO_PORT_CVBS/e_VIDEO_PORT_HDMI/e_VIDEO_PORT_VGA/e_VIDEO_PORT_DVI
    uint8_t               s_AlarmPortType;//e_ALM_PORT_NATIVE_GPIO/e_ALM_PORT_I2C_GPIO
    SysPkgNetworkTypeBit  s_NetworkTypeSupport;//eth¡¢wifi¡¢3g or all support
    uint8_t               s_Reserved[4];
}SysPkgInterface;


//just only superuser(manufactory tools) can write it.
typedef struct tagSysComponents
{
    uint8_t            s_CpuId;//a5s_66 a5s_55
    uint8_t            s_SensorId;//e_SENSOR_MN34041,e_SENSOR_IMX122,e_SENSOR_OV2715,e_SENSOR_OV9715,e_DECODER_TW9910
    uint8_t            s_EncryptICId;
    uint8_t            s_MotorICId;
    uint8_t            s_ZoomLensId;//e_ZOOM_LENS_DF003/e_ZOOM_LENS_YB22
    uint8_t            s_IRCutMaterialType;//e_IRCUT_MATERIAL_GREEN_RED_GLASS/e_IRCUT_MATERIAL_GREEN_BLUE_GLASS
    uint8_t            s_IRCutCtrlType;//e_IRCUT_CTRL_GPIO/e_IRCUT_CTRL_SPI
	uint8_t            s_IRLedCtrlType;//e_IR_CTRL_GPIO/e_IR_CTRL_PWM/e_IR_CTRL_SERIAL/e_IR_CTRL_ALARM_INPUT/e_IR_CTRL_NONE
	uint8_t            s_ShieldType;//e_SHIELD_TIAN_JING/e_SHIELD_HONG_BEN
    uint8_t            s_Reserved[3];
}SysPkgComponents;


//just only superuser(manufactory tools) can write it.
//device capabilities
typedef struct tagSysDeviceHWCapabilities
{
    SysPkgComponents   s_Components;
    SysPkgInterface    s_Interface;
    SysPkgMisceFuncBit s_MisceFuncSupport;
}SysPkgDeviceHWCapabilities;


//capbility type
typedef enum tagSysCapabilityCategory
{
	SYS_CAPABILITY_CATEGORY_ALL = 0,
	SYS_CAPABILITY_CATEGORY_DEVICE,	
	SYS_CAPABILITY_CATEGORY_MEDIA,
	SYS_CAPABILITY_CATEGORY_PTZ
}SysPkgCapabilityCategory;


typedef struct tagSysXml
{	
	int32_t  s_Encrypt;// 0-no encrypt, 1-md5, 2-aes
	int32_t  s_ContentLength; //s_Capabilities length
	char_t   s_Capabilities[0];
}SysPkgXml;

//capabilities
/*
	<?xml version="1.0"?>
	<Capabilities>
		<Device>
			<Network>
				<IPVersion6>false</IPVersion6>
				<DynDns>true</DynDns>
				<IPFilter>true</IPFilter>
				<ZeroConfiguration>false</ZeroConfiguration>				
			</Network>
			<System>
				<RemoteDiscovery>true</RemoteDiscovery>
				<SystemBackup>false</SystemBackup>
				<SystemLogging>true</SystemLogging>
				<FirmwareUpgrade>true</FirmwareUpgrade>
			</System>
			<DeviceIO>
				<AlarmInPortNum>1</AlarmInPortNum>
				<AlarmOutPortNum>1</AlarmOutPortNum>
				<VideoNum>1</VideoNum>			
				<AudioNum>1</AudioNum>			
				<DiskNum>1</DiskNum>
				<NetworkPortNum>1</NetworkPortNum>
			</DeviceIO>
		</Device>
		<Media>
			<MaxStreamNum>4</MaxStreamNum>
			<MaxPicWidth>1920</MaxPicWidth>
			<MaxPicHeight>1080</MaxPicHeight>	
			<RTP_RTSP_TCP>true</RTP_RTSP_TCP>
			<Multicast>false</Multicast>
		</Media>
		<PTZ>
			<MaxPtzPresets>256</MaxPtzPresets>
			<PatrolCruise>8</PatrolCruise>
		</PTZ>
		<Private>
			<CpuType>A5S_66<CpuType>
		</Private>
	</Capabilities>
*/

//workstate
typedef enum
{
	SYS_STATE_RECORDING  = 0, 
	SYS_STATE_NORECORD
}SysPkgRecordState;
  
typedef enum
{
	SYS_STATE_VIDEO_SIGNAL_NORMAL = 0, 
	SYS_STATE_VIDEO_SIGNAL_LOST
}SysPkgVideoSignalState;
   
typedef enum
{
   SYS_STATE_DISK_ACTIVE = 0, 
   SYS_STATE_DISK_SLEEP, 
   SYS_STATE_DISK_ABNORMAL
}SysPkgDiskState;

typedef enum
{
    SYS_STATE_DISPLAY_ON = 0, 
    SYS_STATE_DISPLAY_OFF 
}SysPkgDisplayState;

typedef enum
{
    SYS_STATE_ALARM_IN_OFF = 0, 
    SYS_STATE_ALARM_IN_ON 
}SysPkgAlarmInState;

typedef enum
{
    SYS_STATE_ALARM_OUT_OFF = 0, 
    SYS_STATE_ALARM_OUT_ON 
}SysPkgAlarmOutState;

typedef enum
{
    SYS_STATE_DEVICE_NORMAL = 0, 
    SYS_STATE_DEVICE_CPU_ABNORMAL, 
    SYS_STATE_DEVICE_HARDWARE_ABNORAML 
}SysPkgDeviceState;


//work state
/*
<?xml version="1.0"?>
	<State>0</State>
	<WorkState>
	<StreamState>
          <ActiveStreamNum> 2</ActiveStreamNum>
          <Stream0>
            <Record>0</Record>
            <VideoSignal>0</VideoSignal>
            <BitRate>2000</BitRate>
            <LinkNum>2</LinkNum>
            <ClientIP1>0.0.0.0</ClientIP1>
         </Stream0>
         <Stream1>
            <Record>0</Record>
            <VideoSignal>0</VideoSignal>
            <BitRate>2000</BitRate>
            <LinkNum>2</LinkNum>
            <ClientIP1>0.0.0.0</ClientIP1>
         </Stream1>
      </StreamState>
      <DiskState>
          <Volume> 0</Volume>
          <FreeSpaces>0</FreeSpaces>
          <DiskState>0</DiskState>
      </DisksState>
      <AlarmInState>
          <AlarmIn0>0</AlarmIn0>
          <AlarmIn1>0</AlarmIn1>
      </AlarmInState>
      <AlarmOutState>
          <AlarmOut0> <AlarmOut0>
          <AlarmOut1><AlarmOut1>
      </AlarmOutState>
      <LocalDisplay>0</LocalDisplay>
</WorkState>
*/


//network protocols
typedef struct tagSysNetworkPort
{
	int32_t  s_HTTP_Port;
	int32_t  s_RTSP_Port;
	int32_t  s_SDK_Port;
	int32_t  s_Upgrade_Port;
	int32_t  s_Reserved[6];	
}SysPkgNetworkPort;


//device info
typedef struct tagSysDeviceInfo
{
	char_t	 s_DeviceName[128];//all can read and write.
	uint32_t s_DeviceId;//all can read and write.
	char_t   s_DeviceModel[32];//superuser(manufactory tools) can write it, but others can read only.
	char_t   s_DeviceManufactuer[64];//superuser(manufactory tools) can write it, but others can read only.
	char_t   s_DeviceSerialNum[64];//superuser(manufactory tools) can write it, but others can read only.
	char_t   s_DeviceFwVer[64];//all can read.
	char_t   s_DeviceHwVer[64];//superuser(manufactory tools) can write it, but others can read only.
}SysPkgDeviceInfo;


//ip info
typedef struct tagSysIpInfo
{
	int32_t  s_NetId;//from 1
	char_t   s_InterfName[32];
	char_t   s_IpAddress[32];
	char_t   s_SubNetMask[32];
	char_t   s_GateWay[32];
	char_t   s_Dns[128];//one or more, use space diff
	char_t   s_HwAddress[32];//rd, but just only superuser(manufactory) can write it
	uint8_t  s_Dhcp;//0-disable,1-enable
	uint8_t  s_Reserved[3];
}SysPkgIpInfo;


//ntp
typedef struct tagSysNtpServerInfo
{
	char_t  s_NtpAddr_1[32];
	char_t  s_NtpAddr_2[32];
	char_t  s_NtpAddr_3[32];
}SysPkgNtpServerInfo;


//ftp 
typedef struct tagSysFtpServerInfo
{
	char_t  s_ServerIp[32];
	char_t  s_UserName[128];
	char_t  s_PassWord[128];
	char_t  s_FolderName[128];
}SysPkgFtpServerInfo;


//smtp
typedef struct tagSysSmtpInfo
{
	char_t  s_ServerIp[32];
	char_t  s_UserName[128];
	char_t  s_PassWord[128];
	uint8_t s_Authentication;
	char_t  s_SenderEmailAddr[256];
	char_t  s_receiverEmailAddr[256];
	char_t  s_CC[256];
	char_t  s_Subject[256];
}SysPkgSmtpInfo;


//timezone
typedef struct tagSysTimeZone
{
	int32_t  s_TimeZone;/*range from -12 ~ +13*/
	char_t   s_TimzeZoneName[32];	
}SysPkgTimeZone;

typedef struct tagSysTimeZoneInnerUse
{
	int32_t  s_TimeZone;
	char_t   s_TimeZoneName1[32];
	char_t   s_TimeZoneName2[32];
}SysPkgTimeZoneInnerUse;

//sysdate and time
typedef struct tagSysTime
{
	int32_t  s_Year;
	int32_t  s_Month;
	int32_t  s_Day;
	int32_t  s_Hour;
	int32_t  s_Minute;
	int32_t  s_Second;	
}SysPkgSysTime;


//time config type
typedef enum tagTimeType
{
	SYS_TIME_TYPE_MANUAL = 0,
	SYS_TIME_TYPE_NTP
}SysPkgTimeType;


//time type
typedef struct tagDateTimeType
{
	int32_t  s_Type; //SysPkgTimeType
	int32_t  s_NtpInterval; // 0:sync once, >0:auto sync by interval
}SysPkgDateTimeType;


//users
typedef struct tagUserInfo
{
	char_t    s_UserName[128];
	char_t    s_UserPass[128];
	uint16_t  s_UserFlag;//admin--1, operator--2, viewer--3
	uint16_t  s_UserLevel;//user auth
}SysPkgUserInfo;


//system ctrl command type
typedef enum tagSysCtrlCmd
{
	SYS_SYSTEM_CTRL_REBOOT  = 1,//reboot
	SYS_SYSTEM_CTRL_DEFAULT_HARD, //hard default
	SYS_SYSTEM_CTRL_DEFAULT_SOFT, //soft default
	SYS_SYSTEM_CTRL_WAKEUP,//wakeup
	SYS_SYSTEM_CTRL_SLEEP//sleep
}SysPkgSystemCtrlCmd;


//serial port config
typedef struct tagSerialPortCfg
{
	int32_t   s_PortId;//default from 1
	uint32_t  s_BaudRate;
	uint16_t  s_DataBit;// 5,6,7,8
	uint16_t  s_Parity;//no parity--0, odd --1,even--2
	uint16_t  s_StopBit;// 1, 2
	uint16_t  s_Mode; // RS485--1, RS232--2
	uint16_t  s_FlowCtrl; //none--0, soft--1, hard--2
	uint16_t  s_WorkMode; //terminal--1, trans channel--2
}SysPkgSerialPortCfg;//sys_pkg_serial_port_cfg;


//device performance 
typedef struct tagPerformance
{
	int32_t  s_Cpu; //high 16bit :integer£¬low 16bit:decimal
	int32_t  s_Mem;  //high 16bit :integer£¬low 16bit:decimal
	int32_t  s_Disk; //high 16bit :integer£¬low 16bit:decimal
	char_t   s_TimeStamp[32];
}SysPkgPerformance;


//log query info
typedef struct tagLogInfoSearch
{
	int32_t  s_SelectMode; //all--0, type--1, time--2, type&time--3
	int32_t  s_MajorType; //all type--0
	int32_t  s_MinorType; //all type --0
	char_t   s_StartTime[32];//"yyyy-mm-dd hh:mm:ss"
	char_t   s_StopTime[32];//"yyyy-mm-dd hh:mm:ss"
    int32_t  s_Offset; //first, offset is 0
    int32_t  s_MaxNum; //show count one page
}SysPkgLogInfoSearch;


//log info
typedef struct tagLogInfo
{
    uint64_t s_LogId;
	int32_t  s_MajorType;
	int32_t  s_MinorType;
	char_t   s_LogTime[32];
    char_t   s_UserName[32];
    char_t   s_RemoteHostIp[32];
	char_t   s_LogData[128];
    char_t   s_Reserved[4];
}SysPkgLogInfo;

//log info num
typedef struct tagLogInfoInt
{
    int32_t s_Total; //total loginfo count
    int32_t s_Count; //loginfo count in preset
}SysPkgLogInfoInt;


//alarm info
typedef struct tagAlarmInfor
{
    uint64_t s_WaringId;
    int32_t  s_WaringType;
    int32_t  s_WaringLevel;//1 for hight level, 5 for low level
    int32_t  s_OnOff; //0 for off, 1 for on;
    uint8_t  s_Time[36];
    uint8_t  s_DevId[64];
    uint8_t  s_Description[128];
    union 
    {
        uint32_t s_IoNum;
    }s_ExtraInfo;
}SysPkgAlarmInfor;


//log major type and minor type
////alarm log
//alarm log major type
#define SYS_LOGMAJOR_ALARM            0x1
//minor type, alarm in
#define SYS_LOGMINOR_ALRAM_IN         0x1
//alarm out
#define SYS_LOGMINOR_ALRAM_OUT        0x2
//motion detect
#define SYS_LOGMINOR_MOTDET_START     0x3
#define SYS_LOGMINOR_MOTDET_STOP      0x4
//hide alarm
#define SYS_LOGMINOR_HIDE_ALARM_START 0x5
#define SYS_LOGMINOR_HIDE_ALARM_STOP  0x6
//selt definition alarm
#define SYS_LOGMINOR_SELF_DEF         0x7
//extend io
#define SYS_LOGMINOR_ENLARGE_ALARM    0x8
//extend 64io
#define SYS_LOGMINOR_ENLARGE_64_ALARM 0x9

////abnormal log
#define SYS_LOGMAJOR_EXCEPTION        0x2
//video lost
#define SYS_LOGMINOR_VI_LOST          0x21
//illegal access
#define SYS_LOGMINOR_ILLEGAL_ACCESS   0x22
//hard disk full
#define SYS_LOGMINOR_HD_FULL          0x23
//hard disk error
#define SYS_LOGMINOR_HD_ERROR         0x24
//IP conflict
#define SYS_LOGMINOR_IP_CONFLICT      0x25
//net broken
#define SYS_LOGMINOR_NET_BROKEN       0x26
//rec error
#define SYS_LOGMINOR_REC_ERROR        0x27


////operate log
#define SYS_LOGMAJOR_OPERATION        0x3
//login
#define SYS_LOGMINOR_REMOTE_LOGIN     0x41
//logout
#define SYS_LOGMINOR_REMOTE_LOGOUT    0x42
//start record
#define SYS_LOGMINOR_REMOTE_START_REC 0x43
//stop record
#define SYS_LOGMINOR_REMOTE_STOP_REC  0x44
//start trans channel
#define SYS_LOGMINOR_START_TRANS_CHAN 0x45
//stop trans channel
#define SYS_LOGMINOR_STOP_TRANS_CHAN  0x46
//get param
#define SYS_LOGMINOR_REMOTE_GET_PARM  0x47
//config param
#define SYS_LOGMINOR_REMOTE_CFG_PARM  0x48
//get status
#define SYS_LOGMINOR_REMOTE_GET_STATUS  0x49
//deploy enable
#define SYS_LOGMINOR_REMOTE_ARM         0x4a
//deploy disable
#define SYS_LOGMINOR_REMOTE_DISARM      0x4b
//reboot
#define SYS_LOGMINOR_REMOTE_REBOOT      0x4c
//start voice talk
#define SYS_LOGMINOR_START_VT           0x4d
//stop voice talk
#define SYS_LOGMINOR_STOP_VT            0x4e
//upgrade
#define SYS_LOGMINOR_REMOTE_UPGRADE     0x4f
//play or download file accroding to file
#define SYS_LOGMINOR_REMOTE_PLAYBYFILE  0x50
//play or download file accroding to time
#define SYS_LOGMINOR_REMOTE_PLAYBYTIME  0x51
//format disk
#define SYS_LOGMINOR_REMOTE_FORMAT_HDD  0x52
//ptz control
#define SYS_LOGMINOR_REMOTE_PTZCTRL     0x53
//shutdown
#define SYS_LOGMINOR_REMOTE_STOP        0x54
//lock file
#define SYS_LOGMINOR_REMOTE_LOCKFILE    0x55
//unlock file
#define SYS_LOGMINOR_REMOTE_UNLOCKFILE  0x56
//config file export
#define SYS_LOGMINOR_REMOTE_CFGFILE_OUTPUT  0x57
//config file import
#define SYS_LOGMINOR_REMOTE_CFGFILE_INPUT   0x58
//recod file export
#define SYS_LOGMINOR_REMOTE_RECFILE_OUTPUT  0x59
//manual delete alarm
#define SYS_LOGMINOR_REMOTE_IPC_ALARM       0x5a


////info log
#define SYS_LOGMAJOR_INFORMATION  0x4
//disk info
#define SYS_LOGMINOR_HDD_INFO     0xa1
//smart info
#define SYS_LOGMINOR_SMART_INFO   0xa2
//record start
#define SYS_LOGMINOR_REC_START    0xa3
//record stop
#define SYS_LOGMINOR_REC_STOP     0xa4
//overdate recording file delete
#define SYS_LOGMINOR_REC_OVERDUE  0xa5


/*===media configuration===*/
//video bitrate control
typedef enum tagEnuBitrateCtrl
{
	SYS_BRC_CBR  = 0,
	SYS_BRC_VBR,
	SYS_BRC_CVBR,
	SYS_BRC_NR  = 255,
}SysPkgEnuBitrateCtrl;


//audio compression
typedef enum tagAudioCompression
{
	SYS_AUDIO_COMP_G711A  = 1,
	SYS_AUDIO_COMP_G711U,
	SYS_AUDIO_COMP_G726
}SysPkgAudioCompression;

//audio config
typedef struct tagAudioEncodeCfg
{   
    uint32_t  s_AudioId; //from 1
	uint8_t   s_EncodeType;//1--G.711A,2--G.711U,3--G.726
	uint8_t   s_Chan;//1--mon£¬2--stero
	uint8_t   s_BitsPerSample;//8bit,16bit
	uint8_t   s_Reserved1;
	uint32_t  s_SamplesPerSec;//8000Hz	
	uint16_t  s_CapVolume;	//10% 20% 30%...100%
	uint16_t  s_PlayVolume;//10% 20% 30%...100%
	uint8_t   s_PlayEnable;//0-disable, 1-enable
	uint8_t   s_AecFlag;//0--disable,1--enable
	uint16_t  s_AecDelayTime;//ms
	uint16_t  s_BitRate;//default 64kbps
	uint16_t  s_FrameRate;//default 50
	uint8_t   s_Reserved[12];
}SysPkgAudioEncodeCfg;


//audio decode param,inner use
typedef struct
{
    uint32_t  s_AudioId;//default from 1
    uint32_t  s_Codec;//1--G.711A,2--G.711U,3--G.726
    uint32_t  s_SampleFreq; //unit:hz, 8000hz, ...
    uint32_t  s_BitWidth;   //unit:bit, 8bit, ...
    uint16_t  s_ChannelNum;//1--mon£¬2--stero
    uint16_t  s_FrameRate;//50fps
    uint16_t  s_BitRate; //default 64kbps
    uint16_t  s_Volume; //10%, 20%, 30% ...100%
    uint16_t  s_AecFlag; //0-disable, 1-enable, Acoustic Echo Cancellation
    int16_t   s_AecDelayTime; //unit:ms
    uint8_t   s_Reserved[12];
}SysPkgAudioDecParam;


//video compression
typedef enum tagEnuVideoCompression
{
	SYS_COMP_H264  = 16,
	SYS_COMP_MPEG4,
	SYS_COMP_MJPEG,
}SysPkgVideoCompression;

typedef enum tagStreamType
{
	SYS_VIDEO_STREAM = 1,
	SYS_AUDIO_VIDEO_STREAM
}SysPkgStreamType;

//video encode configuration
typedef struct tagEncodeCfg
{
	int32_t  s_VideoId; //default from 1
	int32_t  s_StreamType;// 1--video, 2--video & audio
	int32_t  s_Compression;
	int32_t  s_PicWidth;
	int32_t  s_PicHeight;
	int32_t  s_BitrateCtrl;
	int32_t  s_Quality;//1-6, best--6, worst--1
	int32_t  s_FPS;// 1-30
	int32_t  s_BitRateAverage;
	int32_t  s_BitRateUp;
	int32_t  s_BitRateDown;
	int32_t  s_Gop;
	int32_t  s_Rotate;
	int32_t  s_Flag;//main stream--0, first substream--1,sencod substream--2,third substream--3
	uint8_t  s_Reserved[4];
}SysPkgEncodeCfg;


//osd info
typedef struct tagOsdInfo
{
	int32_t  s_OsdIndex;//default from 1
	int32_t  s_OsdX;
	int32_t  s_OsdY;
	int32_t  s_BgColor; //background color 
	int32_t  s_Alpha; //diaphaneity 0-256,default 128
	char_t   s_OsdText[128];/*null means no osd, 
	                         if OsdIndex is 1, means used for datetime display,
	                         if OsdText is @datetime means set datetime display, 
	                         if OsdText is null , datetime not display.
	                         format:"YYYY-MM-DD HH:MM:SS ÐÇÆÚX".                                     
	                         if osdIndex is 1, means used for channel display.
	                         if OsdText include'|',means enter ".*/                  
}SysPkgOsdInfo;


typedef struct tagTimeInfo
{
	int32_t s_Enable; //0-disable, 1-enable
	int32_t s_Language; //1-English, 2-Chinese, 3-Korean
	int32_t s_DisplayX; //(0,0)~(100,100)
	int32_t s_DisplayY; //(0,0)~(100,100)
	int32_t s_DateStyle;//date display style, 0-year-month-day,1-month-day-year etc.
	int32_t s_TimeStyle; //time display style, 0-hour:minute:sencond
	int32_t s_FontColor; //font color type, 0-black, 1-red, 2-blue,3-green,4-yellow,5-magenta,6-cyan,7-white,8-auto(self-adaption according to backgroud,range between white and balck)
	int32_t s_FontSize;//Font size, range in8¡¢ 16¡¢24¡¢32
	int32_t s_FontBlod; //0-no-blod, 1-blod
	int32_t s_FontRotate;   //0-no-rotate, 1-rotate
	int32_t s_FontItalic;   //0-no-italic, 1-italic
	int32_t s_FontOutline;  //font outline width	
}SysPkgTimeInfo;


typedef struct tagChannelInfo
{
	int32_t s_Enable; //0-disable, 1-enable
	int32_t s_DisplayX; //(0,0)~(100,100)  
	int32_t s_DisplayY; //(0,0)~(100,100)
	int32_t s_FontColor; //font color type, 0-black, 1-red, 2-blue,3-green,4-yellow,5-magenta,6-cyan,7-white,8-auto(self-adaption according to backgroud,range between white and balck)
	int32_t s_FontSize;//Font size, range in8¡¢ 16¡¢24¡¢32
	int32_t s_FontBlod; //0-no-blod, 1-blod
	int32_t s_FontRotate; //0-no-rotate, 1-rotate
	int32_t s_FontItalic;   //0-no-italic, 1-italic
	int32_t s_FontOutline;  //font outline width
	char_t  s_ChannelName[128];//null means no osd.	
}SysPkgChannelInfo;


//osd info configuration
typedef struct tagShowCfg
{
	int32_t  s_VideoId;//default from 1
	int32_t  s_Flag; //main stream--0, first substream--1,sencod substream--2,third substream--3
	SysPkgTimeInfo     s_TimeInfo;
	SysPkgChannelInfo  s_ChannelInfo;
}SysPkgShowCfg;


//hide area
typedef struct tagHideArea
{
	int32_t s_VideoId;
	int32_t s_StreamId;
	int32_t s_X;
	int32_t s_Y;
	int32_t s_Width;
	int32_t s_Height;
	uint8_t s_Color; //0--white, 1--black
	uint8_t s_Reserved[3];
}SysPkgHideArea;


#define SYS_ENV_DEFAULT_SOURCE_MIRROR  (0)
//video source
typedef struct tagVideoSource
{
    int32_t s_VideoId;
	int32_t s_SrcHeight;
	int32_t s_SrcWidth;
	int32_t s_SrcFps;	
	int32_t s_Mirror; //default-0(nomal), 1-horizontal flip, 2-vertical flip, 3-horizontal&vertical flip	
	int32_t s_Reserved[4];
}SysPkgVideoSource;


//force idr 
typedef struct tagForceIdr
{
    int32_t s_VideoId;
    int32_t s_Flag;
}SysPkgForceIdr;


typedef struct 
{
   int32_t s_VideoId;//default from 1
   int32_t s_EnableStreamNum;//enable stream
   int32_t s_StreamCombineNo;//get from capabiliteis
   uint8_t s_Reserved[4];
}SysPkgEncStreamCombine;


//just only superuser(manufactory tools) can write it, others can read it.
//media stream capabilities
typedef struct tagMediaStreamCapabilities
{
    int32_t  s_StreamId; //default from 1
    uint8_t  s_CompressionsNum;
    uint8_t  s_Compressions[4];//SYS_COMP_H264/SYS_COMP_MPEG4/SYS_COMP_MJPEG
    uint8_t  s_ResolutionsNum;
    uint8_t  s_Resolutions[32];//SYS_RESOLUTION_D1/SYS_RESOLUTION_CIF/SYS_RESOLUTION_QCIF...
	int32_t  s_MaxResolution;
    int32_t  s_MinResolution;
    int32_t  s_MaxFrameRate;
    int32_t  s_MinFrameRate;
    int32_t  s_MaxBitRate;
    int32_t  s_MinBitRate;
    uint8_t  s_Rserved[2];
}SysPkgMediaStreamCapabilities;

//just only superuser(manufactory tools) can write it, others can read it.
//media channel capabilities
typedef struct tagMediaChannelCapabilities
{
    uint8_t  s_VideoId;//default from 1
    uint8_t  s_StreamNum;
    int32_t  s_MaxOsdNum;
    int32_t  s_MaxTotalResolution;
    int32_t  s_MinTotalResolution;
    int32_t  s_MaxTotalFrameRate;
    int32_t  s_MinTotalFrameRate;
    int32_t  s_MaxTotalBitRate;
    int32_t  s_MinTotalBitRate;
    uint8_t  s_Reserved[2];
    char_t   s_MediaStreamCapabilities[0];
}SysPkgMediaChannelCapabilities;

//just only superuser(manufactory tools) can write it, others can read it.
//media capabilities
typedef struct tagMediaCapabilities
{
    uint8_t  s_VideoInNum;
    uint8_t  s_Reserved[3];
    char_t   s_MediaChannelCapabilities[0];
}SysPkgMediaCapabilities;


/*===ptz=== */
//ptz command
typedef enum tagPtzCtrlCmd  
{
	SYS_PTZCMD_STOP = 0,
	SYS_PTZCMD_ZOOM_TELE,	//s_Param[0]--X, use X realize position rapidly. use 0 realize gradual zoom.
	SYS_PTZCMD_ZOOM_WIDE,   //s_Param[0]--X, use X realize position rapidly. use 0 realize gradual zoom.
	SYS_PTZCMD_FOCUS_AUTO, 
	SYS_PTZCMD_FOCUS_NEAR, 	//s_Param[0]--X, use X realize position rapidly. use 0 realize gradual zoom.
	SYS_PTZCMD_FOCUS_FAR, 	//s_Param[0]--X, use X realize position rapidly. use 0 realize gradual zoom.
	SYS_PTZCMD_IRIS_AUTO, 	
	SYS_PTZCMD_IRIS_LARGE, 	
	SYS_PTZCMD_IRIS_SMALL,  
	SYS_PTZCMD_AUXON,  //windshield wipers on
	SYS_PTZCMD_AUXOFF,  //windshield wipers off
	SYS_PTZCMD_AUTOPANON,   
	SYS_PTZCMD_AUTOPANOFF,  
	SYS_PTZCMD_UP, //s_Param[0]--vertical speed (0~255)
	SYS_PTZCMD_DOWN, //s_Param[0]--vertical speed (0~255)       
	SYS_PTZCMD_LEFT, //s_Param[0]--horizontal speed (0~255)      
	SYS_PTZCMD_RIGHT, //s_Param[0]--horizontal speed (0~255)  
	SYS_PTZCMD_RIGHTUP, //s_Param[0]--speed (0~255)
	SYS_PTZCMD_LEFTUP, //s_Param[0]--speed (0~255)
	SYS_PTZCMD_RIGHTDOWN, //s_Param[0]--speed (0~255)
	SYS_PTZCMD_LEFTDOWN, //s_Param[0]--speed (0~255)
	SYS_PTZCMD_GOTOPRESET,  //s_Param[0]--preset index (1~255)
	SYS_PTZCMD_SETPRESET,  //s_Param[0]--preset index (1~255)
	SYS_PTZCMD_CLEARPRESET, //s_Param[0]--preset index (1~255)
	SYS_PTZCMD_LIGHT_ON,  //light on
	SYS_PTZCMD_LIGHT_OFF,  //light off
	SYS_PTZCMD_STARTTOUR,   
	SYS_PTZCMD_STOPTOUR,    
	SYS_PTZCMD_3DCONTROL = 200,   
	SYS_PTZCMD_GMI_MANUAL_FOCUS,
	SYS_PTZCMD_NR = 255
}SysPkgPtzCtrlCmd;


//ptz ctrl
typedef struct tagPtzCtrl
{
	int32_t  s_PtzId;//default from 1
	int32_t  s_PtzCmd;
	int32_t  s_Param[4];//generally use s_Param[0],s_Param[0] imply ptz speed or ptz index,speed range from 0~255,index range from 0~255.
}SysPkgPtzCtrl;


//home 
typedef struct tagPtzHomeCfg
{
	int32_t  s_PtzId; //default from 1
	int32_t  s_HomeEnable; // enable --1,disable--0
	int32_t  s_PresetIndex; // default from 1
	int32_t  s_HomeTime;// 1-720 s
}SysPkgPtzHomeCfg;


//preset index info
typedef struct tagPtzPresetInfo
{
	int32_t s_PtzId;
	int32_t s_PresetIndex; // 1<<16 :have setted, 0<<16:not setted
	char_t  s_PresetName[128];
}SysPkgPtzPresetInfo;


//preset index search
typedef struct tagPtzPresetSearch
{
	int32_t  s_PtzId;
	int32_t  s_PresetIndex; // 0,requeset all preset
}SysPkgPresetSearch;


//3d ctrl
//EndX > StartX means zoom out;
//Else means zoom in;
typedef struct tagPtz3Dctrl
{
	int32_t  s_PtzId;//default form 1
	int32_t  s_X;
	int32_t  s_Y;
	int32_t  s_Width;
	int32_t  s_Height;
	uint8_t  s_Reserved[4];
}SysPkgPtz3Dctrl;


//auto focus mode
typedef enum
{
	SYS_AUTO_FOCUS_MODE_MANUAL = 0,
	SYS_AUTO_FOCUS_MODE_AUTO,
	SYS_AUTO_FOCUS_MODE_ONCE_AUTO
}SysPkgAutoFocusMode;


//focus config
typedef struct 
{
    int32_t s_FocusMode;
    int32_t s_Reserved[4];
}SysPkgAutoFocus;


//capabilities
typedef struct tagPtzCapabilities
{
	//
}SysPkgPtzCapabilites;


/*===record===*/
//record state
/*typedef struct tagRecordState
{
	int32_t  s_VideoId; //default from 1
	int32_t  s_RecordState; //recording -- 1, no record--0
}SysPkgRecordState;
*/

//disk info
typedef struct tagDiskInfo
{
	int32_t   s_DiskId;
	uint32_t  s_TotalSize;// M
	uint32_t  s_FreeSize;
}SysPkgDiskInfo;


//record file info
typedef struct tagStorageFile
{
	int32_t  s_VideoId;
	int32_t  s_FileType;
	int32_t  s_Size;
	char_t   s_StartTime[32];
	char_t   s_StopTime[32];
	char_t   s_FileName[128];
}SysPkgStorageFile;


//record file search
typedef struct tagStorageFileSearch
{
	int32_t  s_VideoId;
	int32_t  s_FileType; //all--0, normal recording file--1, alarm recording file--2
	char_t   s_StartTime[32];
	char_t   s_StopTime[32];
}SysPkgStorageFileSearch;


//record file data
typedef struct tagStorageFileData
{
	int32_t  s_VideoId;
	int32_t  s_FileType;
	char_t   s_StartTime[32];
	char_t   s_StopTime[32];
	char_t   s_FileName[128];
	char_t   s_PlayURL[256];
	int32_t  s_Compression;
}SysPkgStorageFileData;



/*===event===*/
//alarm state
typedef enum tagAlarmState
{
	SYS_ALMSTATE_NONE = 0,
	SYS_ALMSTATE_ON,
	SYS_ALMSTATE_OFF,
}SysPkgAlmState;//SYS_PKG_ALMSTATE;


//alarm type
typedef enum tagAlarmType
{
	SYS_ALMTYPE_NONE = 0,
	SYS_ALMTYPE_DEVICE_FAIL,   //device invalid
	SYS_ALMTYPE_HIDE_DETECT,   //hide 
	SYS_ALMTYPE_LOSS_DETECT,   //video lost
	SYS_ALMTYPE_MOTION_DETECT, //motion dectect
	SYS_ALMTYPE_DISK_FULL,     //disk full
	SYS_ALMTYPE_DISK_ERROR,    //disk error 
}SysPkgAlarmType;


//motion detect
typedef struct tagAlmctlCfgMotionDetect
{
	int32_t  s_Enable;
	int32_t  s_MotionDetectSensitivity;
	int32_t  s_MotionDetectX;
	int32_t  s_MotionDetectY;
	int32_t  s_MotionDetectWidth;
	int32_t  s_MotionDetectHeight;
}SysPkgAlmctlCfgMotionDetect;


//hide detect
typedef struct tagAlmctlcfgHideDetect
{
	int32_t  s_Enable;
	int32_t  s_HideDetectX;
	int32_t  s_HideDetectY;
	int32_t  s_HideDetectWidth;
	int32_t  s_HideDetectHeight;
}SysAlmctlCfgHideDetect;


//externl device ctrl type
typedef enum tagExternalCtrlType
{
	SYS_EXTERNAL_CTRLTYPE_NONE  = 0,
	SYS_EXTERNAL_CTRLTYPE_CTRL,//on/off device
	SYS_EXTERNAL_CTRLTYPE_STATE,//get device status
}SysPkgExternalCtrlType;



/*===image configuration===*/
#define SYS_ENV_DEFAULT_EXPOSURE_MODE      (0)
#define SYS_ENV_DEFAULT_SHUTTER_MIN        (125)
#define SYS_ENV_DEFAULT_SHUTTER_MAX        (40000)
#define SYS_ENV_DEFAULT_EXPOSURE_GAIN_MAX  (36)

typedef struct tagSysPkgExposure
{
	int32_t s_ExposureMode; //0-flicker 50HZ, 1-flicker 60HZ, 2-auto, default 0
	int32_t s_ShutterMin;  //125us ~ 40000us,default 125
	int32_t s_ShutterMax;  //125us ~ 133333, default 40000
	int32_t s_GainMax;    //unit:db, 30-ISO_3200, 36-ISO_6400, 42-ISO_12800, 48-ISO_25600,
		                 //54-ISO_51200,60-ISO_102400 . default 36db
}SysPkgExposure;


#define SYS_ENV_DEFAULT_BRIGHTNESS   (128)
#define SYS_ENV_DEFAULT_CONTRAST     (128)
#define SYS_ENV_DEFAULT_SATURATION   (64)
#define SYS_ENV_DEFAULT_HUE          (128)
#define SYS_ENV_DEFAULT_SHARPNESS    (128)
typedef struct tagSysPkgImaging
{
    int32_t s_VideoId;
	int32_t s_Brightness;        //valid range 0~255, default value 128
	int32_t s_Contrast;          //valid range 0~255, default value 128
    int32_t s_Saturation;        //valid range 0~255, default value 128
	int32_t s_Hue;               //valid range 0~255, default value 128
	int32_t s_Sharpness;         //valid range 0~255, default value 128
	SysPkgExposure s_Exposure;
}SysPkgImaging;

#define SYS_ENV_DEFAULT_METERING_MODE           (1)
#define SYS_ENV_DEFAULT_BACKLIGHT_COMP_FLAG     (0)
#define SYS_ENV_DEFAULT_DCIRIS_FLAG             (0)
#define SYS_ENV_DEFAULT_LOCAL_EXPOSURE          (0)
#define SYS_ENV_DEFAULT_MCTF_STRENGTH           (32)
#define SYS_ENV_DEFAULT_DCIRIS_DUTY             (100)
#define SYS_ENV_DEFAULT_AE_TARGETRATIO          (110)
typedef struct tagSysPkgAdvancedImaging
{
	int32_t  s_VideoId;
	uint8_t  s_MeteringMode;        //0-spot metering, 1-center metering, 2-average metering,3-custom metering. default 1
	uint8_t  s_BackLightCompFlag;   //0-disable, 1-enable. default 0
	uint8_t  s_DcIrisFlag;          //0-disable, 1-enable. default 0
	uint8_t  s_Reserved1[5];
	uint16_t s_LocalExposure;       //WDR :0-disable, 1-Auto, 2-1x, 3-2x, 4-3x,5-4x, valid range is 0~5. default 0
	uint16_t s_MctfStrength;        //DigitalNoiseReduction:0-disable, valid range is 0~255.default 32
	int16_t  s_DcIrisDuty;          //valid range is 100~999.default 100
	int16_t  s_AeTargetRatio;       //valid range is 25~400,unit in percentage.default 110
	uint8_t  s_Reserver2[16];      
}SysPkgAdvancedImaging;

#define SYS_ENV_DEFAULT_WB_MODE    (0)
#define SYS_ENV_DEFAULT_WB_RGAIN   (512)
#define SYS_ENV_DEFAULT_WB_BGAIN   (512)
typedef struct tagSysPkgWhiteBalance
{
	int32_t  s_Mode; //0-WB_AUTOMATIC,1-WB_INCANDESCENT, 2-WB_D4000,3-WB_D5000,4-WB_SUNNY, 5-WB_CLOUDY,6-WB_FLASH,
	                 //7-WB_FLUORESCENT,8-WB_FLUORESCENT_H, 9-WB_UNDERWATER,10-WB_CUSTOM, 11-WB_OUTDOOR . default 0
	int32_t  s_RGain;//valid range is 0~1023. default 512
	int32_t  s_BGain;//valid range is 0~1023. default 512
	uint8_t  s_Reserved[4];
}SysPkgWhiteBalance;


#define SYS_ENV_DEFAULT_DN_MODE           (0)
#define SYS_ENV_DEFAULT_DN_DURTIME        (5)
#define SYS_ENV_DEFAULT_NIGHT_TO_DAY_THR  (60)
#define SYS_ENV_DEFAULT_DAY_TO_NIGHT_THR  (40)
#define SYS_ENV_DEFAULT_SCHED_ENABLE      (0)
#define SYS_ENV_DEFAULT_START_TIME        (0)
#define SYS_ENV_DEFAULT_END_TIME          (0)
// 0: SUNDAY 1~6 MONDAY~SATDAY 7:all
#define DAY_NIGHT_SCHED_WEEKS  (8)
typedef struct tagSysDaynight
{
	int32_t  s_Mode; //0-day mode,1-night mode, 2-auto ,3- timing mode.default 0
	int32_t  s_DurationTime; //valid range is 3~30s.default 5
	int32_t  s_NightToDayThr;//valid range is 0~100.default 60
	int32_t  s_DayToNightThr;//valid range is 0~100.default 40
	uint32_t s_SchedEnable[DAY_NIGHT_SCHED_WEEKS];//0--disable, 1--enable.default 0
	uint32_t s_SchedStartTime[DAY_NIGHT_SCHED_WEEKS];//0~24*3600s.default 0
	uint32_t s_SchedEndTime[DAY_NIGHT_SCHED_WEEKS];//0~24*3600s.default 0
	uint8_t  s_Reserved[4];
}SysPkgDaynight;

#ifdef __cplusplus
}
#endif

#endif
