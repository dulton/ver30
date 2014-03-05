#ifndef __GMI_NETCONFIG_API_H__
#define __GMI_NETCONFIG_API_H__

#include "gmi_system_headers.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SHORT_STRING_SIZE    32
#define MAX_MIDDLE_STRING_SIZE   64
#define MAX_LONG_STRING_SIZE       512


// Device IP Information
    typedef struct _IpInfo
    {
        int32_t   s_Eth;             //eth number
        int32_t   s_Dhcp;              // 0 : Static, 1 : Dhcp
        ulong_t    s_IpAddr;               //IP   Addr
        ulong_t    s_NetMask;     //Mask Addr
        ulong_t    s_GateWay;    //Get Way
        ulong_t    s_Dns1;           //DNS1 Addr
        ulong_t    s_Dns2;           //DNS1 Addr
        ulong_t    s_Dns3;           //DNS1 Addr
        char_t    s_Reserve[MAX_SHORT_STRING_SIZE];      //Reserver
    } IpInfo;

//Device Smtp Information
    typedef struct _SmtpInfo
    {
        int32_t   s_Enable; // 0 : disable, 1 : enable
        char_t    s_SmtpAddr[MAX_SHORT_STRING_SIZE];
        int32_t   s_SmtpPort;
        int32_t   s_AuthEnable; // 0 : disable, 1 : enable
        char_t    s_AuthName[MAX_SHORT_STRING_SIZE];
        char_t    s_AuthPwd[MAX_SHORT_STRING_SIZE];
        int32_t   s_AuthMode; // 0 : login, 1 : plain
        char_t    s_Sender[MAX_MIDDLE_STRING_SIZE];
        char_t    s_Receiver[MAX_MIDDLE_STRING_SIZE];
        char_t    s_Cc[MAX_MIDDLE_STRING_SIZE];
        char_t    s_Subject[MAX_MIDDLE_STRING_SIZE];
        char_t    s_Content[MAX_LONG_STRING_SIZE];
        char_t    s_Reserve[MAX_SHORT_STRING_SIZE];
    } SmtpInfo;

//Device Ftp Information
    typedef struct _FtpInfo
    {
        int32_t  s_FtpEnable; // 0 : disable, 1 : enable
        char_t   s_FtpAddr[MAX_SHORT_STRING_SIZE];
        int32_t  s_FtpPort;
        char_t   s_FtpUser[MAX_SHORT_STRING_SIZE];
        char_t   s_FtpPwd[MAX_SHORT_STRING_SIZE];
        char_t    s_Reserve[MAX_SHORT_STRING_SIZE];
    } FtpInfo;

//Device Upnp Information
    typedef struct _UpnpInfo
    {
        int32_t  s_UpnpEnable;
        int32_t  s_UpnpPort;
        char_t   s_UpnpName[MAX_SHORT_STRING_SIZE];
        char_t    s_Reserve[MAX_SHORT_STRING_SIZE];
    } UpnpInfo;

//Device DDNS Information
    typedef struct _DdnsInfo
    {
        int32_t   s_DdnsEnable;
        char_t    s_DdnsServer[MAX_MIDDLE_STRING_SIZE];
        int32_t   s_DdnsPort;
        int32_t   s_DdnsTimeout;
        char_t    s_DdnsUser[MAX_SHORT_STRING_SIZE];
        char_t    s_DdnsPwd[MAX_SHORT_STRING_SIZE];
        char_t    s_DdnsDomain[MAX_MIDDLE_STRING_SIZE];
        int32_t    s_DdnsType;
        char_t    s_Reserve[MAX_SHORT_STRING_SIZE];
    } DdnsInfo;

//Device Alarm Server Center  Setting Information
    typedef struct _AlarmServerCenterInfo
    {
        int32_t   s_Center1Enable;
        int32_t   s_Center1Port;
        char_t    s_Center1IpAddr[MAX_SHORT_STRING_SIZE];
        int32_t   s_Center2Enable;
        int32_t   s_Center2Port;
        char_t    s_Center2IpAddr[MAX_SHORT_STRING_SIZE];
        int32_t   s_Center3Enable;
        int32_t   s_Center3Port;
        char_t    s_Center3IpAddr[MAX_SHORT_STRING_SIZE];
        char_t    s_Reserve[MAX_SHORT_STRING_SIZE];
    } AlarmServerCenterInfo;

    /*===============================================================
    func name:GMI_IPAddressTransform
    func: Unsigned long type Ip Addr transform char type
    input:
    return:char type  IP addr pointer
    **********************************************************************************/
    char_t *GMI_IPAddressTransform(ulong_t ulIP);

    /*===============================================================
    func name:GMI_CheckIPStr
    func: Check IP String is invalid, String It could not be something this is number or '.
    input:
    return:0: string is vaild.  -1; String is invaild
    **********************************************************************************/
    GMI_RESULT GMI_CheckIPStr(const char_t *str);

    /*===============================================================
    func name:GMI_CheckNetmaskStr
    func: Check IP String is invalid, String It could not be something this is number or '.' And the last character
    	 It could not be the number 255.
    input:
    return:0: string is vaild.  -1; String is invaild
    **********************************************************************************/
    GMI_RESULT GMI_CheckNetmaskStr(const char_t *str);

    /*===============================================================
    func name:GMI_CheckIPConfig
    func: Check IP Addr and IP Mask and GateWay is invalid, IP and getway must be in one net
    input:  ip:  IP Addrs
    	   netmask: Ip Mask
    	   gateway: IP GateWay
    return:0: string is vaild.  -1; String is invaild
    **********************************************************************************/
    GMI_RESULT GMI_CheckIPConfig(ulong_t ip, ulong_t netmask, ulong_t gateway);

    /*===============================================================
    func name:GMI_CheckMacInvalid
    func: Check MAC addrs is invalid
    input: MAC addrs char
    return:0: string is vaild.  -1; String is invaild
    **********************************************************************************/
    GMI_RESULT GMI_CheckMacInvalid(const char_t *MacAddrs);

    /*===============================================================
    func name:GMI_ReadMacConfig
    func: Read Device MAC addr
    input:  FileName:Config file name
             ItemPath: MAC addr Item Save dircation
             DefMac: default value
             Mac: MAC addr return string
    return:SUCCESS: string is vaild.  ERORR CODE; String is invaild
    **********************************************************************************/
    GMI_RESULT GMI_ReadMacConfig(const char_t * FileName, const char_t *ItemPath, const char_t *DefMac, char_t *Mac);

    /*===============================================================
    func name:GMI_WriteMacConfig
    func: Write Device MAC addr
    input:  FileName:Config file name
             ItemPath: MAC addr Item Save dircation
             Mac: MAC addr value
    return:SUCCESS: string is vaild.  ERORR CODE; String is invaild
    **********************************************************************************/

    GMI_RESULT GMI_WriteMacConfig(const char_t * FileName, const char_t *ItemPath, char_t *Mac);
    /*===============================================================
    func name:GMI_ReadNetIpConfig
    func: Read Device IP addr
    input:  FileName:Config file name
             ItemPath: IP addr Item Save dircation
             DefInfoIp: default value
             NetIpInfo: IP addr return
    return:SUCCESS: string is vaild.  ERORR CODE; String is invaild
    **********************************************************************************/
    GMI_RESULT GMI_ReadNetIpConfig(const char_t * FileName, const char_t *ItemPath, const IpInfo *DefInfoIp, IpInfo *NetIpInfo);

    /*===============================================================
    func name:GMI_ReadOnlyNetIpConfig
    func: Read Device IP addr
    input:	FileName:Config file name
    	   ItemPath: IP addr Item Save dircation
    	   DefInfoIp: default value
    	   NetIpInfo: IP addr return
    return:SUCCESS: string is vaild.  ERORR CODE; String is invaild
    **********************************************************************************/
    GMI_RESULT GMI_ReadOnlyNetIpConfig(const char_t * FileName, const char_t *ItemPath, const IpInfo *DefInfo, IpInfo *NetIpInfo);

    /*===============================================================
    func name:GMI_WriteNetIpConfig
    func: Write Device IP addr
    input:  FileName:Config file name
             ItemPath: IP addr Item Save dircation
             NetIpInfo: IP addr return
    return:SUCCESS: string is vaild.  ERORR CODE; String is invaild
    **********************************************************************************/
    GMI_RESULT GMI_WriteNetIpConfig(const char_t * FileName, const char_t *ItemPath, IpInfo *NetIpInfo);

    /*===============================================================
    func name:GMI_ActivateNet
    func:Enable Net Device
    input:
    return:SUCCESS: string is vaild.  ERORR CODE; String is invaild
    **********************************************************************************/
    GMI_RESULT GMI_ActivateNet(const IpInfo *NetIpInfo);

/*============================================================================
name				:	GMI_ReadNetCfg
function			:  Read system default network file IP value.
algorithm implementation	:	no
global variable 		:	no
parameter declaration	:    IpInfoConfig: network config data
return				:	 success: GMI_SUCCESS
						    fail; ERROR CODE
******************************************************************************/
GMI_RESULT GMI_ReadNetWorkCfg(IpInfo *IpInfoConfig);

/*============================================================================
name				:	GMI_WriteNetCfg
function			:  Set system default network file IP value.
algorithm implementation	:	no
global variable 		:	no
parameter declaration	:    IpInfoConfig: network config data
return				:	 success: GMI_SUCCESS
						    fail; ERROR CODE
******************************************************************************/
GMI_RESULT GMI_WriteNetWorkCfg(const IpInfo *IpInfoConfig);

GMI_RESULT GMI_SnmpServerInit(void);

GMI_RESULT GMI_SnmpServerStart(void);

GMI_RESULT GMI_SnmpServerStop(void);


#ifdef __cplusplus
}
#endif

#endif

