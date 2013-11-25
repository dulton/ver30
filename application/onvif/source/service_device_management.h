#ifndef __SERVICE_DEVICE_MANAGEMENT_H__
#define __SERVICE_DEVICE_MANAGEMENT_H__

#include "gmi_system_headers.h"


//network
#define DEFAULT_VALID_NETWORK_NUM     1
#define DEFAULT_NETWORK_NAME         "eth0"
#define NETWORK_TOKEN_0              "eth0"
#define DEFAULT_NETWORK_TOKEN        "eth0"
#define NETWORK_INFO_LENGTH           32

//reboot message
#define REBOOT_MESSAGE               "Rebooting in 90 seconds"

//alarm io
#define DEFAULT_ALARM_INPUTS       1
#define DEFAULT_ALARM_OUTPUTS      1
#define DEFAULT_RELAY_OUTPUTS      1

//network protocl
#define DEFAULT_NETWORK_PROTOCOLS  2

//user length
#define USER_LENGTH                64
//passwd length
#define PASSWD_LENGTH              64
//user count
#define USER_NUM                   16

typedef struct tt_IPAddress
{
    enum tt__IPType Type;
    char IPv4Address[32];
    char IPv6Address[48];
} IPAddress;

//interface
typedef struct tagDevNetworkInterface
{
    char_t  s_InterfaceName[NETWORK_INFO_LENGTH];
    char_t  s_InterfaceToken[NETWORK_INFO_LENGTH];
    char_t  s_IPAddr[32];
    char_t  s_Mask[32];
    char_t  s_Gateway[32];
    uint8_t s_MacAddr[32];
} DevNetworkInterface;

//security
typedef struct tagSecurity
{
    int32_t s_TLS1_x002e1          : 1;
    int32_t s_TLS1_x002e2          : 1;
    int32_t s_OnboardKeyGeneration : 1;
    int32_t s_AccessPolicyConfig   : 1;
    int32_t s_X_x002e509Token      : 1;
    int32_t s_SAMLToken            : 1;
    int32_t s_KerberosToken        : 1;
    int32_t s_RELToken             : 1;
    int32_t s_TLS1_x002e0          : 1;
    int32_t s_Dot1X                : 1;
    int32_t s_RemoteUserHandling   : 1;
} DevSecurity;

//network protocols
typedef struct tagNetworkProtocols
{
    enum tt__NetworkProtocolType s_Name;
    enum xsd__boolean            s_Enabled;
    int32_t                      s_Port;
} NetworkProtocols;

//dns
typedef struct tagDnsInformation
{
    boolean_t s_FromDhcp;
    char_t    s_ManualIpv4Addr[32];
    char_t    s_SearchDomain[128];
} DnsInformation;

//ntp
typedef struct tagNtpInformaiton
{
    boolean_t s_FromDhcp;
    char_t    s_ManualIpv4Addr[32];
    char_t    s_DnsName[128];
} NtpInformaiton;

typedef struct
{
    char_t    s_UserName[64];
    char_t    s_UserPass[64];
    uint16_t  s_UserLevel;//operator 0-9
} Users;


typedef struct tagScopes
{
    tt__ScopeDefinition s_ScopeDefinitionFlag;
    char_t s_ScopeUrl[64];
} DevScopes;


#endif

