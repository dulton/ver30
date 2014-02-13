#ifndef __SERVICE_UTILITLY_H__
#define __SERVICE_UTILITLY_H__

#include "soapH.h"
#include "gmi_system_headers.h"

#ifdef __cpluscplus
extern "C"
{
#endif

//get from sys control server switch
#define SWITCH_FROM_SERVER
//#define DEBUG_RTSP

//onvif version
#define ONVIF_SUPPORT_VER_NUM   3
#define ONVIF_VERSION_MAJOR     2
#define ONVIF_VERSION_MINOR     2
//default server port
//commd port
#define DEFAULT_SERVER_PORT     8080
//rtsp port
#define DEFAULT_RTSP_PORT       554

//info length
#define INFO_LENGTH              100
#define AJUST_GAP                10
#define TOKEN_LENGTH             64

//wsdl id
#define DEVICE_WSDL_ID           0
#define MEDIA_WSDL_ID            1
#define PTZ_WSDL_ID              2
#define EVENTS_WSDL_ID           3
#define DEVICE_IO_WSDL_ID        4
#define IMAGING_WSDL_ID          5
#define ANALYTICS_WSDL_ID        6
#define REMOTE_DISCOVERY_WSDL_ID 7
#define DISPLAY_WSDL_ID          8
#define RECEIVER_WSDL_ID         9
#define RECORDING_CTRL_WSDL_ID   10
#define RECORDING_SEARCH_WSDL_ID 11
#define REPLAY_WSDL_ID           12
#define ANALYTICS_DEVICE_WSDL_ID 13


    typedef struct
    {
        int32_t Id;
        const char_t *Wsdl;
        const char_t *SvrName;
        int32_t Major;
        int32_t Minor;
    } ONVIF_Wsdl;

    //func declartion
    void* soap_malloc_zero(struct soap *p_soap, size_t n);
    int GetNetAddress( char_t *EthName, uint8_t *Mac, char_t *Ip);
    GMI_RESULT NET_GetIpInfo(char_t * EthName, char_t * IP);
    GMI_RESULT NET_GetMacInfo(char_t * EthName, char_t * Mac);
    GMI_RESULT NET_CheckIPString(char_t *Str);
    GMI_RESULT Soap_WSSE_Authentication(struct soap *soap_ptr);
    GMI_RESULT ONVIF_Fault(struct soap *soap_ptr, const char *Value1, const char *Value2, const char *Reason);
    GMI_RESULT ONVIF_FAULT(struct soap *soap_ptr, const char *Object, const char *Value1, const char *Value2, const char *Reason);
    GMI_RESULT NET_CheckHostname (char_t *Str);
    GMI_RESULT IsNum(char_t *Str);

    extern ONVIF_Wsdl g_WsdlArry[];
    extern uint32_t g_WsdlCount;

#ifdef __cpluscplus
}
#endif
#endif

