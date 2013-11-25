#ifndef _GMI_STRUCT_H_
#define _GMI_STRUCT_H_

#include "gmi_system_headers.h"
#include "gmi_netconfig_api.h"


#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////Struct and Data for Application daemon ////////////////////////////////////

#define GMI_RUDP_TIMEOUT                                 2000
#define GMI_RUDP_BUFFER_LENGTH                      32

//daemon application timer value. default number.
#define MAX_RETURN_VALUE                               120

//daemon application timer value. default number.
#define MAX_BUFFER_LENGTH                             255
#define MIN_BUFFER_LENGTH                               64
#define FILE_BUFFER_LENGTH                               1024

#define SYSTEM_START_TIME_RECORD "/opt/bin/start_time.sh"

#define LOG_SERVER_APPLICATION  "/opt/bin/log_server &"
#define MEDIA_SERVER_APPLICATION  "/opt/bin/media_center_server &"
#define CONTROL_SERVER_APPLICATION "/opt/bin/sys_server &"
#define TRANSPORT_SERVER_APPLICATION "/opt/bin/rtsp_server &"
#define GB28181_SERVER_APPLICATION "/opt/bin/gb28181 -r 10.0.2.237:5060  &"
#define ONVIF_SERVER_APPLICATON "/opt/bin/onvif &"
#define SDK_SERVER_APPLICATION "/opt/bin/sdk_server &"
#define AUTH_SERVER_APPLICATION "/opt/bin/auth_server &"
#define CONFIG_TOOL_SERVER_APPLICATION "/opt/bin/config_tool &"


#define LOG_SERVER_APPLICATION_KILL  "killall -9  log_server"
#define MEDIA_SERVER_APPLICATION_KILL  "killall -9  media_center_server"
#define CONTROL_SERVER_APPLICATION_KILL "killall -9  sys_server"
#define TRANSPORT_SERVER_APPLICATION_KILL "killall -9  rtsp_server"
#define GB28181_SERVER_APPLICATION_KILL "killall -9  gb28181"
#define ONVIF_SERVER_APPLICATON_KILL "killall -9  onvif"
#define SDK_SERVER_APPLICATION_KILL "killall -9 sdk_server"
#define AUTH_SERVER_APPLICATION_KILL  "killall -9 auth_server"
#define CONFIG_TOOL_SERVER_APPLICATION_KILL "killall -9  config_tool"


//daemon application manage app
    typedef struct
    {
        int32_t  s_LogServer;                  //log Server
        int32_t  s_MediaServer;               //media Server
        int32_t  s_TransportServer;         //transportServer
        int32_t  s_WebServer;                 //   Web Server
        int32_t  s_ControlServer;            // control Server
        int32_t  s_Gb28181Server;         // GB 28181 Server
        int32_t  s_OnVifServer;              //OnVif Server
        int32_t  s_SdkServer;		//Sdk Server
        int32_t  s_AuthServer;		//Auth Server
        int32_t  s_ConfigToolServer;		//config_tool Server
        int32_t s_LogServerTimer ;    //logServer Timer
        int32_t s_MediaServerTimer ;  //MediaServe Timer
        int32_t s_TransportServerTimer ;   //TransportServer Timer
        int32_t s_ControlServerTimer ;   //ControlServer Timer
        int32_t s_GbServerTimer;     //GbServer Timer
        int32_t s_OnvifServerTimer;   //OnvifServer Timer
        int32_t s_WebServerTimer;   //WebServe Timer
        int32_t s_SdkServerTimer;    //SdkServer Timer
        int32_t s_AuthServerTimer;  //AuthServer Timer
        int32_t  s_ConfigToolServertimer;		//config_tool timer
        int32_t  s_Reserve[8];                 //reserver m_usReserve
    } SystemApplicationCfg_t, SYSTEM_APPLICATION_CFG, *PSYSTEM_APPLICATION_CFG;

//daemon application is Open Flags
    typedef struct _ApplicationOpenFlags {
        boolean_t s_LogServerOpenFlags;
        boolean_t s_MediaServerOpenFlags;
        boolean_t s_TransportServerOpenFlags;
        boolean_t s_ControlServerOpenFlags;
        boolean_t s_Gb28181ServerOpenFlags;
        boolean_t s_OnVifServerOpenFlags;
        boolean_t s_SdkServerOpenFlags;
        boolean_t s_AuthServerOpenFlags;
        boolean_t s_ConfigToolOpenFlags;
        boolean_t s_Reserve[8]; 				//reserver m_usReserve
    } ApplicationOpenFlags;

    typedef struct _ApplicationIPChangeFlags {
        boolean_t s_LogServerIPChangeFlags;
        boolean_t s_MediaServerIPChangeFlags;
        boolean_t s_TransportServerIPChangeFlags;
        boolean_t s_ControlServerIPChangeFlags;
        boolean_t s_Gb28181ServerIPChangeFlags;
        boolean_t s_OnVifServerIPChangeFlags;
        boolean_t s_SdkServerIPChangeFlags;
        boolean_t s_AuthServerIPChangeFlags;
        boolean_t s_ConfigToolIPChangeFlags;
        boolean_t s_Reserve[8]; 				//reserver m_usReserve
    } ApplicationIPChangeFlags;

//daemon application is daemon register flags
    typedef struct _ApplicationRegisterFlags {
        boolean_t s_LogRegisterFlag;
        boolean_t s_MediaRegisterFlag;
        boolean_t s_TransPortRegisterFlag;
        boolean_t s_ControlRegisterFlag;
        boolean_t s_GbRegisterFlag;
        boolean_t s_OnvifRegisterFlag;
        boolean_t s_SdkRegisterFlag;
        boolean_t s_AuthRegisterFlag;
        boolean_t s_ConfigToolRegisterFlag;
        boolean_t s_Reserve[8]; 				//reserver m_usReserve
    } ApplicationRegisterFlags;

//daemon application is daemon register flags
    typedef struct _ApplicationDaemonTime {
        int32_t s_LogDaeminTime;
        int32_t s_MediaDaeminTime;
        int32_t s_TransPortDaeminTime;
        int32_t s_ControlDaeminTime;
        int32_t s_GbDaeminTime;
        int32_t s_OnvifDaeminTime;
        int32_t s_SdkDaeminTime;
        int32_t s_AuthDaeminTime;
        int32_t s_ConfigToolDaeminTime;
        int32_t s_Reserve[8]; 				//reserver m_usReserve
    } ApplicationDaemonTime ;

//application quit flag
    typedef struct _ApplicationQuitFlag {
        boolean_t s_AllAppQuitFlag;
        boolean_t s_LogServerQuitFlag;
        boolean_t s_MediaQuitFlag;
        boolean_t s_TransPortQuitFlag;
        boolean_t s_ControlQuitFlag;
        boolean_t s_GbQuitFlag;
        boolean_t s_OnvifQuitFlag;
        boolean_t s_SdkQuitFlag;
        boolean_t s_AuthQuitFlag;
        boolean_t s_ConfigToolQuitFlag;
        boolean_t s_Reserve[8];				//reserver m_usReserve
    } ApplicationQuitFlag ;

//application Online flag
    typedef struct _ApplicationOnlineFlag {
        boolean_t s_LogServerOnlineFlag;
        boolean_t s_MediaOnlineFlag;
        boolean_t s_TransPortOnlineFlag;
        boolean_t s_ControlOnlineFlag;
        boolean_t s_GbOnlineFlag;
        boolean_t s_OnvifOnlineFlag;
        boolean_t s_SdkOnlineFlag;
        boolean_t s_AuthOnlineFlag;
        boolean_t s_ConfigToolOnlineFlag;
        boolean_t s_Reserve[8];				//reserver m_usReserve
    } ApplicationOnlineFlag ;

//////////////////////////Struct and Data for hardware monitor ////////////////////////////////////
//Hardware monitor struct
    typedef struct _HardwareMonitor
    {
        char_t s_CPU[16];        //CPU information
        char_t s_VideoIn[16];   //VideoIn information
        char_t s_MainBoard[16];   //Board information
        char_t s_Len[16];           // Len type
        char_t s_Reserve[32];
    } HardwareConfig;

//Gpio Control struct
    typedef struct _GpioControl
    {
        uint32_t id;   //GPIO port ,for example .70 ,GPOO70
        uint32_t data;  //Set or Read GPIO port data value
    } GpioControl;

//CPU ID
    typedef enum {
        IAV_CHIP_ID_A5S_UNKNOWN = -1,
        IAV_CHIP_ID_A5S_33,
        IAV_CHIP_ID_A5S_55,
        IAV_CHIP_ID_A5S_66,
        IAV_CHIP_ID_A5S_70,
        IAV_CHIP_ID_A5S_88,
        IAV_CHIP_ID_A5S_99,
    } IavChipId;

//////////////////////////Struct and Data for System Update ////////////////////////////////////

//For System Update Message
#define GMI_UPDATE_MESSAGE_TAG             "GUMT"
#define GMI_UPDATE_MESSAGE_TAG_LENGTH      4

#define GMI_UPDATE_MESSAGE_MAJOR_VERSION   1
#define GMI_UPDATE_MESSAGE_MINOR_VERSION   0

#define GMI_UPDATE_MESSAGE_TYPE_REQUEST    1  //
#define GMI_UPDATE_MESSAGE_TYPE_REPLY      2    //
#define GMI_UPDATE_MESSAGE_TYPE_NOTIFY     4   //
#define GMI_UPDATE_MESSAGE_TYPE_PARTIAL    8  //

// command
#define GMI_UPDATE_LIST_SEND			0x10001
#define GMI_UPDATE_FILE_DOWN			0x10002
#define GMI_UPDATE_NOTIFY_STATUS	0x10003

// Update Base
    typedef struct _UpdateBaseMessage
    {
        uint8_t      s_MessageTag[GMI_UPDATE_MESSAGE_TAG_LENGTH];
        uint8_t      s_HeaderFlags;  //0
        uint8_t      s_HeaderExtensionSize;//one implies four byte unit in extension section;
        uint8_t      s_MajorVersion;
        uint8_t      s_MinorVersion;
        uint16_t     s_SequenceNumber;   //
        uint8_t      s_PaddingByteNumber;//7-8 bits for header extension padding byte number, 5-6 bits for message padding byte number
        uint8_t      s_MessageType;
        uint16_t     s_MessageId;
        uint16_t     s_MessageLength;
    } UpdateBaseMessage;

//update message
    typedef struct _UpdateMessage
    {
        UpdateBaseMessage s_UpdateHearder;
        char_t *XmlContent;
    } UpdateMessage;

//System data
    typedef struct _SystemUpdateData
    {
        char_t s_ServerIp[32];
        int32_t s_type;
        char_t s_Version[32];
        int32_t s_Count;
        int32_t s_TotalSize;
        int32_t s_KernelFlags;
        char_t s_KernelName[32];
        int32_t s_KernelSize;
        char_t s_KernelTime[16];
        char_t s_KernelVersion[16];
        char_t s_KernelMd5[32];
        int32_t s_RootfsFlags;
        char_t s_RootfsName[32];
        int32_t s_RootfsSize;
        char_t s_RootfsTime[16];
        char_t s_RootfsVersion[16];
        char_t s_RootfsMd5[32];
        int32_t s_AppFlags;
        char_t s_AppName[32];
        int32_t s_AppSize;
        char_t s_AppTime[16];
        char_t s_AppVersion[16];
        char_t s_AppMd5[32];
    } SystemUpdateData;

///////////////////////////////////////
//For System Daemon Message
#define GMI_DAEMON_MESSAGE_TAG             "GDMT"
#define GMI_DAEMON_MESSAGE_TAG_LENGTH      4

//s_MajorVersion
#define GMI_DAEMON_MESSAGE_MAJOR_VERSION   1
#define GMI_DAEMON_MESSAGE_MINOR_VERSION   0

// s_MessageId
#define GMI_DAEMON_APPLICATION_STATUS_QUIRY			    0x01
#define GMI_DAEMON_APPLICATION_START			             0x02
#define GMI_DAEMON_APPLICATION_STOP		                    0x03
#define GMI_DAEMON_APPLICATION_SYSTEM_REBOOT                0x04
#define GMI_DAEMON_APPLICATION_IP_CHANGE                      0x05

//Application Id for system server


#define  ALL_SERVER_ID                                                    0x00
#define  LOG_SERVER_ID                                                   0x01
#define  MEDIA_SERVER_ID                                                0x02
#define  TRANSPORT_SERVER_ID                                         0x03
#define  CONTROL_SERVER_ID                                            0x04
#define  WEB_SERVER_ID                                                   0x05
#define  GB28181_SERVER_ID                                             0x06
#define  ONVIF_SERVER_ID                                                 0x07
#define  SDK_SERVER_ID                                                    0x08
#define  AUTH_SERVER_ID			                                 0x09
#define  CONFIG_TOOL_SERVER_ID                                     0x0A


//Application running status
#define   APPLICATION_STATUS_OFFLINE                              0x00
#define   APPLICATION_STATUS_ONLINE                               0x01

#define   APPLICATION_QUIT_FLAGS                                     "QUIT"
#define   APPLICATION_LINK_FLAGS                                       "LINK"
#define   APPLICATION_IPCHANGE_FLAGS                             "IPCHANGE"


#define   LOG_SERVER_REGISTER_MESSAGE                            "LogServer"
#define   MEDIA_SERVER_REGISTER_MESSAGE                         "MeidaServer"
#define   TRANSPORT_SERVER_REGISTER_MESSAGE                  "TransportServer"
#define   CONTROL_SERVER_REGISTER_MESSAGE                     "ControlServer"
#define   WEB_SERVER_REGISTER_MESSAGE                             "WebServer"
#define   GB28181_SERVER_REGISTER_MESSAGE                      "Gb28181Server"
#define   ONVIF_SERVER_REGISTER_MESSAGE                          "OnvifServer"
#define   SDK_SERVER_REGISTER_MESSAGE                              "SdkServer"
#define   AUTH_SERVER_REGISTER_MESSAGE                           "AuthServer"
#define   CONFIG_TOOL_SERVER_REGISTER_MESSAGE             "ConfigToolServer"


#define   LOG_SERVER_HEARTBEAT_MESSAGE                           "LogServer Link"
#define   MEDIA_SERVER_HEARTBEAT_MESSAGE                       "MeidaServer Link"
#define   TRANSPORT_SERVER_HEARTBEAT_MESSAGE                "TransportServer Link"
#define   CONTROL_SERVER_HEARTBEAT_MESSAGE                   "ControlServer Link"
#define   WEB_SERVER_HEARTBEAT_MESSAGE                            "WebServer Link"
#define   GB28181_SERVER_HEARTBEAT_MESSAGE                    "Gb28181Server Link"
#define   ONVIF_SERVER_HEARTBEAT_MESSAGE                        "OnvifServer Link"
#define   SDK_SERVER_HEARTBEAT_MESSAGE                            "SdkServer Link"
#define   AUTH_SERVER_HEARTBEAT_MESSAGE                           "AuthServer Link"
#define   CONFIG_TOOL_SERVER_HEARTBEAT_MESSAGE             "ConfigToolServer Link"

#define   LOG_SERVER_UNREGISTER_MESSAGE                             "UnLogServer"
#define   MEDIA_SERVER_UNREGISTER_MESSAGE                          "UnMeidaServer"
#define   TRANSPORT_SERVER_UNREGISTER_MESSAGE                   "UnTransportServer"
#define   CONTROL_SERVER_UNREGISTER_MESSAGE                      "UnControlServer"
#define   WEB_SERVER_UNREGISTER_MESSAGE                              "UnWebServer"
#define   GB28181_SERVER_UNREGISTER_MESSAGE                       "UnGb28181Server"
#define   ONVIF_SERVER_UNREGISTER_MESSAGE                           "UnOnvifServer"
#define   SDK_SERVER_UNREGISTER_MESSAGE                               "UnSdkServer"
#define   AUTH_SERVER_UNREGISTER_MESSAGE                            "UnAuthServer"
#define   CONFIG_TOOL_SERVER_UNREGISTER_MESSAGE              "UnConfigToolServer"

// Daemon Base Message
    typedef struct _DaemonBaseMessage
    {
        uint8_t      s_MessageTag[GMI_UPDATE_MESSAGE_TAG_LENGTH];
        uint8_t      s_HeaderFlags;  //0
        uint8_t      s_HeaderExtensionSize;//one implies four byte unit in extension section;
        uint8_t      s_MajorVersion;
        uint8_t      s_MinorVersion;
        uint16_t     s_SequenceNumber;   //
        uint8_t      s_PaddingByteNumber;//7-8 bits for header extension padding byte number, 5-6 bits for message padding byte number
        uint8_t      s_MessageType;
        uint16_t     s_MessageId;
        uint16_t     s_MessageLength;
    } DaemonBaseMessage;

//Daemon  Message
    typedef struct _DaemonMessage
    {
        DaemonBaseMessage s_DaemonHearder;
        uint16_t s_LocalServerId;        //local Server ID
        uint16_t s_AppId;                  //inquiry server ID
        uint16_t s_AppStatus;            //inquiry server running status
        uint16_t s_Reserve[1];
    } DaemonMessage;

////////extern global variable///////

//System application configs struct
    extern SystemApplicationCfg_t  *g_pSystemApplicationCfg;
//Application Open Flags
    extern ApplicationOpenFlags *g_ApplicationOpenFlags;
//Application Daemon Register Flage
    extern ApplicationRegisterFlags *g_ApplicationRegisterFlags;
//Application Quit Flag
    extern ApplicationQuitFlag  *g_ApplicationQuitFlag;
//Application Online Flag
    extern ApplicationOnlineFlag *g_ApplicationOnlineFlag;
//System hardware monitor config
    extern HardwareConfig *g_Hardware;
//System update struct data
    extern SystemUpdateData *g_UpMessage;
//System daemon time
    extern ApplicationDaemonTime *g_ApplicationDaemonTime;
//System Ip addr info
    extern IpInfo                                       *g_IpInfo;
//System IP change report flags
    extern ApplicationIPChangeFlags        *g_ApplicationIPChangeFlags;


    extern int32_t g_UpdatePort;

#ifdef __cplusplus
}
#endif

#endif

