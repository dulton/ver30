#ifndef _DAEMON_HEART_BEAT_H_
#define _DAEMON_HEART_BEAT_H_


#include "gmi_system_headers.h"

#ifdef __cplusplus
extern "C" {
#endif

//SERVER ID
#define  ALL_SERVER_ID                                      0x00
#define  LOG_SERVER_ID                                      0x01
#define  MEDIA_SERVER_ID                                    0x02
#define  TRANSPORT_SERVER_ID                                0x03
#define  CONTROL_SERVER_ID                                  0x04
#define  WEB_SERVER_ID                                      0x05
#define  GB28181_SERVER_ID                                  0x06
#define  ONVIF_SERVER_ID                                    0x07
#define  SDK_SERVER_ID                                      0x08
#define  AUTH_SERVER_ID                                     0x09
#define  CONFIG_TOOL_SERVER_ID                                     0x0A

#define   APPLICATION_QUIT                                  0x00
#define   APPLICATION_RUNNING                               0x01
#define   SYSTEM_IP_CHANGEING                               0x02

//CMD
#define  GMI_DAEMON_APPLICATION_STATUS_QUIRY                0x01
#define  GMI_DAEMON_APPLICATION_START                       0x02
#define  GMI_DAEMON_APPLICATION_STOP                        0x03
#define  GMI_DAEMON_APPLICATION_SYSTEM_REBOOT               0x04
#define  GMI_DAEMON_APPLICATION_IP_CHANGE               0x05

//Application running status
#define   APPLICATION_STATUS_OFFLINE                        0x00
#define   APPLICATION_STATUS_ONLINE                         0x01
#define   APPLICATION_STATUS_REBOOT                         0x02
#define   APPLICATION_STATUS_ERROR                          0x03

#define   APPLICATION_QUIT_FLAGS                            "QUIT"
#define   APPLICATION_LINK_FLAGS                            "LINK"
#define   APPLICATION_IPCHANGE_FLAGS                   "IPCHANGE"

#define   LOG_SERVER_REGISTER_MESSAGE                       "LogServer"
#define   MEDIA_SERVER_REGISTER_MESSAGE                     "MeidaServer"
#define   TRANSPORT_SERVER_REGISTER_MESSAGE                 "TransportServer"
#define   CONTROL_SERVER_REGISTER_MESSAGE                   "ControlServer"
#define   WEB_SERVER_REGISTER_MESSAGE                       "WebServer"
#define   GB28181_SERVER_REGISTER_MESSAGE                   "Gb28181Server"
#define   ONVIF_SERVER_REGISTER_MESSAGE                     "OnvifServer"
#define   SDK_SERVER_REGISTER_MESSAGE                       "SdkServer"
#define   AUTH_SERVER_REGISTER_MESSAGE                      "AuthServer"
#define   CONFIG_TOOL_SERVER_REGISTER_MESSAGE             "ConfigToolServer"

#define   LOG_SERVER_HEARTBEAT_MESSAGE                      "LogServer Link"
#define   MEDIA_SERVER_HEARTBEAT_MESSAGE                    "MeidaServer Link"
#define   TRANSPORT_SERVER_HEARTBEAT_MESSAGE                "TransportServer Link"
#define   CONTROL_SERVER_HEARTBEAT_MESSAGE                  "ControlServer Link"
#define   WEB_SERVER_HEARTBEAT_MESSAGE                      "WebServer Link"
#define   GB28181_SERVER_HEARTBEAT_MESSAGE                  "Gb28181Server Link"
#define   ONVIF_SERVER_HEARTBEAT_MESSAGE                    "OnvifServer Link"
#define   SDK_SERVER_HEARTBEAT_MESSAGE                      "SdkServer Link"
#define   AUTH_SERVER_HEARTBEAT_MESSAGE                     "AuthServer Link"
#define   CONFIG_TOOL_SERVER_HEARTBEAT_MESSAGE             "ConfigToolServer Link"

#define   LOG_SERVER_UNREGISTER_MESSAGE                     "UnLogServer"
#define   MEDIA_SERVER_UNREGISTER_MESSAGE                   "UnMeidaServer"
#define   TRANSPORT_SERVER_UNREGISTER_MESSAGE               "UnTransportServer"
#define   CONTROL_SERVER_UNREGISTER_MESSAGE                 "UnControlServer"
#define   WEB_SERVER_UNREGISTER_MESSAGE                     "UnWebServer"
#define   GB28181_SERVER_UNREGISTER_MESSAGE                 "UnGb28181Server"
#define   ONVIF_SERVER_UNREGISTER_MESSAGE                   "UnOnvifServer"
#define   SDK_SERVER_UNREGISTER_MESSAGE                     "UnSdkServer"
#define   AUTH_SERVER_UNREGISTER_MESSAGE                    "UnAuthServer"
#define   CONFIG_TOOL_SERVER_UNREGISTER_MESSAGE              "UnConfigToolServer"

    typedef struct
    {
        long_t   s_ServerPort;
        int32_t  s_AppId;
        FD_HANDLE s_SockFd;
    } DaemonData_t, DAEMON_DATA_CFG, *PDAEMON_DATA_CFG;

    /*=======================================================
    name				:	GMI_DaemonInit
    function			:  Init Heardbeat .Get Application Socket Handle
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     DaemonData : Daemon struct data;
                                                  AppId : System server Id;
                                                  ServerPort: daemon  server Port;
                                                  LocalPort: System  server Port;

    return				:    FD_HANDLE: Application Socket handle
    ******************************************************************************/
    GMI_RESULT GMI_DaemonInit(DaemonData_t *DaemonData, int32_t AppId, long_t ServerPort, long_t LocalPort);

    /*=======================================================
    name				:	UnInitHeardbeat
    function			:  Delete Application Handle
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:      DaemonData : Daemon struct data;

    return				:    no
    ******************************************************************************/
    void GMI_DaemonUnInit(DaemonData_t *DaemonData);

    /*=======================================================
    name				:	GMI_DaemonRegister
    function			:  Register Application to Daemon Server
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:      DaemonData : Daemon struct data;
    return				:     0:success
                                            fail:  ERROR CODE
    ******************************************************************************/
    GMI_RESULT GMI_DaemonRegister(DaemonData_t *DaemonData);

    /*=======================================================
    name				:	GMI_DaemonUnRegister
    function			:  UnRegister Application to Daemon Server
    algorithm implementation	:	no
    global variable 		:	no
    parameter declaration		:	   DaemonData : Daemon struct data;
    return				:	  0:success
    										fail:  ERROR CODE
    ******************************************************************************/
    GMI_RESULT GMI_DaemonUnRegister(DaemonData_t *DaemonData);

    /*=======================================================
    name				:	GMI_DaemonReport
    function			:  Report Heardbeat to Daemon Server
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     DaemonData : Daemon struct data;
    return				:    0:success
                                           fail:  ERROR CODE
    ******************************************************************************/
    GMI_RESULT GMI_DaemonReport(DaemonData_t *DaemonData, uint32_t *BootFlags);

    /*=======================================================
    name				:	GMI_InquriyServerStatus
    function			:  Inquriy Server Status form daemon server
    algorithm implementation	:	no
    global variable 		:	no
    parameter declaration		:	  PInquriyData : Daemon struct data;
                                         Cmd : the Cmmmond for server ,for example. GMI_DAEMON_APPLICATION_STATUS_QUIRY or GMI_DAEMON_APPLICATION_SYSTEM_REBOOT
                                         Status:Server running status, for example. APPLICATION_STATUS_OFFLINE
    return				:	 SUCCESS :GMI_SUCCESS
                                   FAIL:   GMI_FAIL;
    ******************************************************************************/
    GMI_RESULT GMI_InquiryServerStatus(DaemonData_t *DaemonData, long_t RemotePort, int32_t Cmd, int32_t AppId, uint16_t *Status);

    /*=======================================================
    name				:	GMI_SystemReboot
    function			:  server reboot system api
    algorithm implementation	:	no
    global variable 		:	no
    parameter declaration		:	  PInquriyData : Daemon struct data;
                                         RemotePort : Remote server port
    return				:	 SUCCESS :GMI_SUCCESS
                                   FAIL:   GMI_FAIL;
    ******************************************************************************/
    GMI_RESULT  GMI_SystemReboot(DaemonData_t *DaemonData, long_t RemotePort);

    /*=======================================================
    name				:	GMI_SystemIPChangeReport
    function			:  server Ip change report
    algorithm implementation	:	no
    global variable 		:	no
    parameter declaration		:	  PInquriyData : Daemon struct data;
                                         RemotePort : Remote server port
    return				:	 SUCCESS :GMI_SUCCESS
                                   FAIL:   GMI_FAIL;
    ******************************************************************************/
    GMI_RESULT  GMI_SystemIPChangeReport(DaemonData_t *DaemonData, long_t RemotePort);

#ifdef __cplusplus
}
#endif

#endif

