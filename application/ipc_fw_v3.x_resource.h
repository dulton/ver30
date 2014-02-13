
// ipc_fw_v3x_resource.h
// define resource id or range for ipc_fw_v3x project
// note: these value are only default value, program can use other value.

#if !defined( IPC_FW_V3X_RESOURCE )
#define IPC_FW_V3X_RESOURCE

#define GMI_RESOURCE_CONFIG_FILE_NAME               "/opt/config/gmi_resource.xml"
#define GMI_RESOURCE_XML                            GMI_RESOURCE_CONFIG_FILE_NAME

#define GMI_EXTERN_NETWORK_PORT_PATH                "/Config/NetworkPort/Extern/"
#define GMI_INNER_NETWORK_PORT_PATH                 "/Config/NetworkPort/Inner/"

//extern network port
#define GMI_RTSP_SERVER_TCP_PORT_KEY                "RTSP_ServerPort"
#define GMI_DAEMON_UPDATE_SERVER_PORT_KEY           "UpdatePort"
#define GMI_HTTP_SERVER_PORT_KEY                    "HTTP_ServerPort"
#define GMI_SDK_SERVER_PORT_KEY                     "SDK_ServerPort"

/********************** unix socket ***********************/

#define GMI_CONTROL_S_UNIX_NAME                     "/tmp/sys_server"

/************************ udp port ************************/

#define GMI_UDP_PORT_BASE                           60000

//daemon server
#define GMI_DAEMON_HEARDBEAT_SERVER                 (GMI_UDP_PORT_BASE+0)
#define GMI_DAEMON_HEARDBEAT_LOG                    (GMI_UDP_PORT_BASE+1)
#define GMI_DAEMON_HEARDBEAT_MEDIA                  (GMI_UDP_PORT_BASE+2)
#define GMI_DAEMON_HEARDBEAT_TRANSPORT              (GMI_UDP_PORT_BASE+4)
#define GMI_DAEMON_HEARDBEAT_CONTROL                (GMI_UDP_PORT_BASE+5)
#define GMI_DAEMON_HEARDBEAT_GB28181                (GMI_UDP_PORT_BASE+6)
#define GMI_DAEMON_HEARDBEAT_ONVIF                  (GMI_UDP_PORT_BASE+7)
#define GMI_DAEMON_HEARDBEAT_WEB                    (GMI_UDP_PORT_BASE+8)
#define GMI_DAEMON_HEARTBEAT_SDK                    (GMI_UDP_PORT_BASE+9)
#define GMI_DAEMON_HEARTBEAT_AUTH                   (GMI_UDP_PORT_BASE+10)
#define GMI_DAEMON_HEARTBEAT_CONFIG_TOOL            (GMI_UDP_PORT_BASE+11)
#define GMI_DAEMON_HEARTBEAT_STATUS_QUERY           (GMI_UDP_PORT_BASE+12)
//onvif server
#define GMI_ONVIF_C_PORT                            (GMI_UDP_PORT_BASE+13)
#define GMI_ONVIF_S_PORT                            (GMI_UDP_PORT_BASE+14)
#define GMI_ONVIF_AUTH_PORT                         (GMI_UDP_PORT_BASE+15)

//sys server
#define GMI_CONTROL_C_PORT                          (GMI_UDP_PORT_BASE+16)
#define GMI_CONTROL_S_PORT                          (GMI_UDP_PORT_BASE+17)
//cgi
#define GMI_CGI_C_PORT_START                        (GMI_UDP_PORT_BASE+18)
#define GMI_CGI_S_PORT_START                        (GMI_UDP_PORT_BASE+19)
#define GMI_CGI_AUTH_PORT                           (GMI_UDP_PORT_BASE+20)
//ntp loacl listen port
#define GMI_NTP_S_PORT                              (GMI_UDP_PORT_BASE+21)
//config_tool
#define GMI_CONFIG_TOOL_C_PORT                      (GMI_UDP_PORT_BASE+22)
#define GMI_CONFIG_TOOL_S_PORT                      (GMI_UDP_PORT_BASE+23)
#define GMI_CONFIG_TOOL_AUTH_PORT                   (GMI_UDP_PORT_BASE+24)
//media center server
#define GMI_MEDIA_CENTER_SERVER_COMMAND_PORT        (GMI_UDP_PORT_BASE+25)
#define GMI_MEDIA_CENTER_CLIENT_COMMAND_PORT        (GMI_UDP_PORT_BASE+26)

#define GMI_SYS_SDK_PORT_PATH                       "/ipc/connect_port_resource"
#define GMI_SYS_SERVER_TO_SDK_PORT_ITEM             "sys_server_to_sdk_port"
#define SYS_SERVER_TO_SDK_PORT                      (GMI_UDP_PORT_BASE+27)
#define GMI_SDK_TO_SYS_SERVER_PORT_ITEM             "sdk_to_sys_server_port"
#define SDK_TO_SYS_SERVER_PORT                      (GMI_UDP_PORT_BASE+28)

#define GMI_ONVIF_RTSP_SERVER_AUTH_PORT             (GMI_UDP_PORT_BASE+29)
//authentication server port
#define GMI_AUTH_SERVER_PORT                        (GMI_UDP_PORT_BASE+30)
#define GMI_SYS_SERVER_TO_SDK_REQ_PORT_ITEM         "sys_server_to_sdk_req_port"
#define GMI_SDK_TO_SYS_SERVER_REQ_PORT_ITEM         "sdk_to_sys_server_req_port"
#define SDK_TO_SYS_SERVER_REQ_PORT                  (GMI_UDP_PORT_BASE+31)
#define SDK_TO_AUTH_SERVER_PORT                     (GMI_UDP_PORT_BASE+32)


//streaming media server
#define MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH            "/Config/media_center_server/ipc_media_data_dispatch/"
#define MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SERVER_UDP_PORT "server_udp_port"
#define GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1    (GMI_UDP_PORT_BASE+50)
#define GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO2    (GMI_UDP_PORT_BASE+51)
#define GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO3    (GMI_UDP_PORT_BASE+52)
#define GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO4    (GMI_UDP_PORT_BASE+53)
#define GMI_STREAMING_MEDIA_SERVER_ENCODE_AUDIO1    (GMI_UDP_PORT_BASE+54)
#define MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_CLIENT_UDP_PORT "client_udp_port"
#define GMI_STREAMING_MEDIA_SERVER_DECODE_AUDIO1    (GMI_UDP_PORT_BASE+55)
//streaming media client - onvif
#define ONVIF_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH             "/Config/onvif_media_server/ipc_media_data_dispatch/"
#define ONVIF_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_CLIENT_UDP_PORT  "client_udp_port"
#define GMI_STREAMING_MEDIA_ONVIF_ENCODE_VIDEO1     (GMI_UDP_PORT_BASE+60)
#define GMI_STREAMING_MEDIA_ONVIF_ENCODE_VIDEO2     (GMI_UDP_PORT_BASE+61)
#define GMI_STREAMING_MEDIA_ONVIF_ENCODE_VIDEO3     (GMI_UDP_PORT_BASE+62)
#define GMI_STREAMING_MEDIA_ONVIF_ENCODE_VIDEO4     (GMI_UDP_PORT_BASE+63)
#define GMI_STREAMING_MEDIA_ONVIF_ENCODE_AUDIO1     (GMI_UDP_PORT_BASE+64)
#define ONVIF_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SERVER_UDP_PORT  "server_udp_port"
#define GMI_STREAMING_MEDIA_ONVIF_DECODE_AUDIO1     (GMI_UDP_PORT_BASE+65)
//streaming media client - gb28181
#define GB_MEDIA_CLIENT_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH                "/Config/gb_media_client/ipc_media_data_dispatch/"
#define GB_MEDIA_CLIENT_IPC_MEDIA_DATA_DISPATCH_CONFIG_CLIENT_UDP_PORT     "client_udp_port"
#define GMI_STREAMING_MEDIA_GB28181_ENCODE_VIDEO1   (GMI_UDP_PORT_BASE+70)
#define GMI_STREAMING_MEDIA_GB28181_ENCODE_VIDEO2   (GMI_UDP_PORT_BASE+71)
#define GMI_STREAMING_MEDIA_GB28181_ENCODE_VIDEO3   (GMI_UDP_PORT_BASE+72)
#define GMI_STREAMING_MEDIA_GB28181_ENCODE_VIDEO4   (GMI_UDP_PORT_BASE+73)
#define GMI_STREAMING_MEDIA_GB28181_ENCODE_AUDIO1   (GMI_UDP_PORT_BASE+74)
#define GB_MEDIA_CLIENT_IPC_MEDIA_DATA_DISPATCH_CONFIG_SERVER_UDP_PORT     "server_udp_port"
#define GMI_STREAMING_MEDIA_GB28181_DECODE_AUDIO1   (GMI_UDP_PORT_BASE+75)
//streaming media client - sdk stream
#define SDK_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH               "/Config/sdk_media_server/ipc_media_data_dispatch/"
#define SDK_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_CLIENT_UDP_PORT    "client_udp_port"
#define GMI_STREAMING_MEDIA_SDK_ENCODE_VIDEO1       (GMI_UDP_PORT_BASE+80)
#define GMI_STREAMING_MEDIA_SDK_ENCODE_VIDEO2       (GMI_UDP_PORT_BASE+81)
#define GMI_STREAMING_MEDIA_SDK_ENCODE_VIDEO3       (GMI_UDP_PORT_BASE+82)
#define GMI_STREAMING_MEDIA_SDK_ENCODE_VIDEO4       (GMI_UDP_PORT_BASE+83)
#define GMI_STREAMING_MEDIA_SDK_ENCODE_AUDIO1       (GMI_UDP_PORT_BASE+84)
#define SDK_MEDIA_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SERVER_UDP_PORT    "server_udp_port"
#define GMI_STREAMING_MEDIA_SDK_DECODE_AUDIO1       (GMI_UDP_PORT_BASE+85)

// these UDP port used by share memory log server and client
#define LOG_SERVER_DEFAULT_SERVER_PORT              (GMI_UDP_PORT_BASE+200)
#define LOG_UPGRADE_DEFAULT_PORT                    (GMI_UDP_PORT_BASE+201)
#define LOG_MEDIA_CENTER_DEFAULT_PORT               (GMI_UDP_PORT_BASE+202)
#define LOG_CONTROL_CENTER_DEFAULT_PORT             (GMI_UDP_PORT_BASE+203)
#define LOG_GB28181_DEFAULT_PORT                    (GMI_UDP_PORT_BASE+204)
#define LOG_ONVIF_DEFAULT_PORT                      (GMI_UDP_PORT_BASE+205)
#define LOG_PRIVATE_STREAM_DEFAULT_PORT             (GMI_UDP_PORT_BASE+206)
#define LOG_STORAGE_DEFAULT_PORT                    (GMI_UDP_PORT_BASE+207)
#define LOG_WEB_DEFAULT_PORT                        (GMI_UDP_PORT_BASE+208)
#define LOG_AUTHENTICATION_DEFAULT_PORT             (GMI_UDP_PORT_BASE+209)
#define LOG_GB_RTP_DEFAULT_PORT                     (GMI_UDP_PORT_BASE+210)
#define LOG_ONVIF_RTSP_DEFAULT_PORT                 (GMI_UDP_PORT_BASE+211)
#define LOG_SDK_DEFAULT_PORT                        (GMI_UDP_PORT_BASE+212)

// onvif rtsp server
#define ONVIF_MEDIA_SERVER_RTSP_CONFIG_PATH                                "/Config/onvif_media_server/rtsp/"
#define ONVIF_MEDIA_SERVER_RTSP_CONFIG_SERVER_MULTICAST_ADDRESS            "server_multicast_address"
#define ONVIF_MEDIA_SERVER_RTSP_CONFIG_SERVER_RTP_UDP_PORT                 "server_rtp_udp_port"
#define ONVIF_MEDIA_SERVER_RTSP_CONFIG_SERVER_RTCP_UDP_PORT                "server_rtcp_udp_port"

#define GMI_ONVIF_RTSP_SERVER_BASE_PORT             2000

#define GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO1_RTP     (GMI_ONVIF_RTSP_SERVER_BASE_PORT+0)
#define GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO1_RTCP    (GMI_ONVIF_RTSP_SERVER_BASE_PORT+1)
#define GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO2_RTP     (GMI_ONVIF_RTSP_SERVER_BASE_PORT+2)
#define GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO2_RTCP    (GMI_ONVIF_RTSP_SERVER_BASE_PORT+3)
#define GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO3_RTP     (GMI_ONVIF_RTSP_SERVER_BASE_PORT+4)
#define GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO3_RTCP    (GMI_ONVIF_RTSP_SERVER_BASE_PORT+5)
#define GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO4_RTP     (GMI_ONVIF_RTSP_SERVER_BASE_PORT+6)
#define GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO4_RTCP    (GMI_ONVIF_RTSP_SERVER_BASE_PORT+7)
#define GMI_ONVIF_RTSP_SERVER_ENCODE_AUDIO1_RTP     (GMI_ONVIF_RTSP_SERVER_BASE_PORT+8)
#define GMI_ONVIF_RTSP_SERVER_ENCODE_AUDIO1_RTCP    (GMI_ONVIF_RTSP_SERVER_BASE_PORT+9)
#define GMI_ONVIF_RTSP_SERVER_DECODE_AUDIO1_RTP     (GMI_ONVIF_RTSP_SERVER_BASE_PORT+10)
#define GMI_ONVIF_RTSP_SERVER_DECODE_AUDIO1_RTCP    (GMI_ONVIF_RTSP_SERVER_BASE_PORT+11)

/************************ tcp port  ************************/

//http server port
#define GMI_HTTP_SERVER_PORT                        80
//rtsp server tcp port
#define GMI_RTSP_SERVER_TCP_PORT                    554
//onvif port
#define GMI_ONVIF_SERVER_PORT                       8080
//sdk server port
#define GMI_SDK_SERVER_PORT                         30000

// log server
// publish server port, later log will use SDK to transmit log information and don't use this tcp port
#define GMI_LOG_PUBLISH_SERVER_PORT                 2000

//daemon update server port
#define GMI_DAEMON_UPDATE_SERVER_PORT               8000

/************************ mutex id ************************/

#define GMI_IPC_MUTEX_BASE_KEY                                       1000

/********* log *********/
#define LOG_SERVER_CONFIG_PATH                                       "/Config/log_server/"
#define LOG_SERVER_CONFIG_SHARE_MEMORY_MUTEX_KEY                     "share_memory_mutex_key"
#define GMI_LOG_SERVER_DEFAULT_SHARE_MEMORY_IPC_MUTEX_KEY            (GMI_IPC_MUTEX_BASE_KEY+0)

//streaming media data dispatch
#define MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH             "/Config/media_center_server/ipc_media_data_dispatch/"
#define MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_IPC_MUTEX_KEY    "ipc_mutex_key"
#define GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1_IPC_MUTEX_KEY       (GMI_IPC_MUTEX_BASE_KEY+2)
#define GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO2_IPC_MUTEX_KEY       (GMI_IPC_MUTEX_BASE_KEY+4)
#define GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO3_IPC_MUTEX_KEY       (GMI_IPC_MUTEX_BASE_KEY+6)
#define GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO4_IPC_MUTEX_KEY       (GMI_IPC_MUTEX_BASE_KEY+8)
#define GMI_STREAMING_MEDIA_SERVER_ENCODE_AUDIO1_IPC_MUTEX_KEY       (GMI_IPC_MUTEX_BASE_KEY+10)
#define GMI_STREAMING_MEDIA_SERVER_DECODE_AUDIO1_IPC_MUTEX_KEY       (GMI_IPC_MUTEX_BASE_KEY+12)

#define GMI_USER_LOG_IPC_MUTEX_KEY                                   (GMI_IPC_MUTEX_BASE_KEY+14)
// not used for now 
#define GMI_DEBUG_LOG_IPC_MUTEX_KEY                                  (GMI_IPC_MUTEX_BASE_KEY+16)

/************************ share memory key ************************/

#define GMI_SHARE_MEMORY_BASE_KEY                                    2000

//log server
#define LOG_SERVER_CONFIG_SHARE_MEMORY_KEY                           "share_memory_key"
#define GMI_LOG_SERVER_DEFAUL_SHARE_MEMORY_KEY                       (GMI_SHARE_MEMORY_BASE_KEY+1)
#define GMI_USER_LOG_SHARE_MEMORY_KEY                                (GMI_SHARE_MEMORY_BASE_KEY+2)
// not used for now 
#define GMI_DEBUG_LOG_SHARE_MEMORY_KEY                               (GMI_SHARE_MEMORY_BASE_KEY+3)

//streaming media center server
// the same as MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_IPC_MUTEX_KEY
//#define MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_PATH             "/Config/media_center_server/ipc_media_data_dispatch/"
#define MEDIA_CENTER_SERVER_IPC_MEDIA_DATA_DISPATCH_CONFIG_SHARE_MEMORY_KEY "share_memory_key"
#define GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1_SHARE_MEMORY_KEY    (GMI_SHARE_MEMORY_BASE_KEY+4)
#define GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO2_SHARE_MEMORY_KEY    (GMI_SHARE_MEMORY_BASE_KEY+5)
#define GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO3_SHARE_MEMORY_KEY    (GMI_SHARE_MEMORY_BASE_KEY+6)
#define GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO4_SHARE_MEMORY_KEY    (GMI_SHARE_MEMORY_BASE_KEY+7)
#define GMI_STREAMING_MEDIA_SERVER_ENCODE_AUDIO1_SHARE_MEMORY_KEY    (GMI_SHARE_MEMORY_BASE_KEY+8)
#define GMI_STREAMING_MEDIA_SERVER_DECODE_AUDIO1_SHARE_MEMORY_KEY    (GMI_SHARE_MEMORY_BASE_KEY+9)

/************************ multicast address ************************/
//onvif rtsp server
#define GMI_ONVIF_RTSP_SERVER_MULTICAST_ADDRESS                      "224.0.0.100"
#define GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO1_MULTICAST_ADDRESS        GMI_ONVIF_RTSP_SERVER_MULTICAST_ADDRESS
#define GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO2_MULTICAST_ADDRESS        GMI_ONVIF_RTSP_SERVER_MULTICAST_ADDRESS
#define GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO3_MULTICAST_ADDRESS        GMI_ONVIF_RTSP_SERVER_MULTICAST_ADDRESS
#define GMI_ONVIF_RTSP_SERVER_ENCODE_VIDEO4_MULTICAST_ADDRESS        GMI_ONVIF_RTSP_SERVER_MULTICAST_ADDRESS
#define GMI_ONVIF_RTSP_SERVER_ENCODE_AUDIO1_MULTICAST_ADDRESS        GMI_ONVIF_RTSP_SERVER_MULTICAST_ADDRESS
// not used for now 
#define GMI_ONVIF_RTSP_SERVER_DECODE_AUDIO1_MULTICAST_ADDRESS        GMI_ONVIF_RTSP_SERVER_MULTICAST_ADDRESS

#endif//IPC_FW_V3X_RESOURCE
