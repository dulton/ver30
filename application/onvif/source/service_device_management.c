
#include "soapH.h"
#include "log.h"
#include "discovery.h"
#include "service_utilitly.h"
#include "service_device_management.h"
#include "service_media_configuration.h"
#include "service_ptz.h"
#include "sys_client.h"
#include "sys_env_types.h"
#include "gmi_system_headers.h"


static DevNetworkInterface l_NetworkInterfaceInfo[DEFAULT_VALID_NETWORK_NUM] =
{
    {
        DEFAULT_NETWORK_NAME,
        NETWORK_TOKEN_0,
        "0.0.0.0",
        "0.0.0.0",
        "0.0.0.0"
    }
};

//security
static DevSecurity l_Security =
{
    xsd__boolean__false_,
    xsd__boolean__false_,
    xsd__boolean__false_,
    xsd__boolean__false_,
    xsd__boolean__false_,
    xsd__boolean__false_,
    xsd__boolean__false_,
    xsd__boolean__false_,
    xsd__boolean__false_,
    xsd__boolean__false_,
    xsd__boolean__false_
};

//network protocols
static NetworkProtocols l_NetworkProtocols[DEFAULT_NETWORK_PROTOCOLS] =
{
    {
        tt__NetworkProtocolType__HTTP,
        xsd__boolean__true_,
        DEFAULT_SERVER_PORT
    },
    {
        tt__NetworkProtocolType__RTSP,
        xsd__boolean__true_,
        DEFAULT_RTSP_PORT
    }
};

//host name, used for authen
#ifndef SWITCH_FROM_SERVER
static char_t l_HostName[128] = "IPCamera";
#endif

//dns
static DnsInformation l_DnsInformation =
{
    false,
    "8.8.8.8",
    "domain.name"
};

enum tt__DiscoveryMode g_DiscoveryMode = tt__DiscoveryMode__Discoverable;
enum xsd__boolean g_LocalAddressFlag = xsd__boolean__false_;
enum xsd__boolean g_DhcpFlag = xsd__boolean__false_;
int32_t g_ChangeScopes = 0;

#define DEFAULT_VALID_SCOPE_NUM    12
#define DEFAULT_USED_SCOPE_NUM     6
DevScopes g_DeviceScopes[DEFAULT_VALID_SCOPE_NUM] =
{
    {tt__ScopeDefinition__Fixed, "onvif://www.onvif.org/type/video_encoder"},
    {tt__ScopeDefinition__Fixed, "onvif://www.onvif.org/type/ptz"},
    {tt__ScopeDefinition__Fixed, "onvif://www.onvif.org/type/audio_encoder"},
    {tt__ScopeDefinition__Fixed, "onvif://www.onvif.org/hardware/IPCamera"},
    {tt__ScopeDefinition__Fixed, "onvif://www.onvif.org/location/country/china"},
    {tt__ScopeDefinition__Fixed, "onvif://www.onvif.org/name/IPC"},
    {tt__ScopeDefinition__Configurable, ""},
    {tt__ScopeDefinition__Configurable, ""},
    {tt__ScopeDefinition__Configurable, ""},
    {tt__ScopeDefinition__Configurable, ""},
    {tt__ScopeDefinition__Configurable, ""},
    {tt__ScopeDefinition__Configurable, ""}
};


int min(int x, int y)
{
	return ((x>y)?y:x);
}


void SaveTestValueBeforeReboot()
{
	FILE   *fp = NULL;
	int n_i = 0;

	fp = fopen("/usr/local/bin/onvifTmpFile", "wb");
    if (fp)
    {
        fprintf(fp, "DiscoveryMode %d\n", g_DiscoveryMode);
		fprintf(fp, "LocalAddressFlag %d\n", g_LocalAddressFlag);
		fprintf(fp, "DhcpFlag %d\n", g_DhcpFlag);
		fprintf(fp, "ChangeScopes %d\n", g_ChangeScopes);

		for(n_i = DEFAULT_USED_SCOPE_NUM; n_i < DEFAULT_VALID_SCOPE_NUM; n_i++)
		{
			if(strlen(g_DeviceScopes[n_i].s_ScopeUrl) > 0)
			{
				fprintf(fp, "ScopeUrl %s\n", g_DeviceScopes[n_i].s_ScopeUrl);
			}
		}		
        fclose(fp);
        fp = NULL;
    }
	
	return;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServices(struct soap *soap_ptr, struct _tds__GetServices *tds__GetServices, struct _tds__GetServicesResponse *tds__GetServicesResponse)
{
    uint8_t    ValidInterfacesNum = 0;
    uint8_t    No                 = 0;
    GMI_RESULT Result             = GMI_SUCCESS;
    int32_t    HttpPort           = 0;
    DevNetworkInterface  *NetInterface = NULL;

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);

    //user auth
    //Result = Soap_WSSE_Authentication(soap_ptr);
    //if (FAILED(Result))
    //{
    //	ONVIF_ERROR("user auth failed\n");
    //	SoapResult = SOAP_USER_ERROR;
    //	goto errExit;
    //}

    //should get device capabilties
    //get network num
    //default interface num   : 1
    //default interface name : "eth0"
    //generally, valid interface num just only one,  eth0 or wifi etc
    boolean_t Failure = false;

    do
    {
        HttpPort           = DEFAULT_SERVER_PORT;
        NetInterface       = l_NetworkInterfaceInfo;
        ValidInterfacesNum = DEFAULT_VALID_NETWORK_NUM;
        for (No = 0; No < ValidInterfacesNum; No++)
        {
            Result = NET_GetIpInfo(NetInterface[No].s_InterfaceName, NetInterface[No].s_IPAddr);
            if (FAILED(Result))
            {
                Failure = true;
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "NET_GetIpInfo fail, Result = 0x%lx\n", Result);
                break;
            }
        }
        if (Failure)
        {
            break;
        }

        tds__GetServicesResponse->__sizeService = g_WsdlCount;
        tds__GetServicesResponse->Service =  (struct tds__Service *)soap_malloc_zero(soap_ptr, sizeof(struct tds__Service) * tds__GetServicesResponse->__sizeService);
        if (NULL == tds__GetServicesResponse->Service)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "soap_malloc_zero fail, Result = 0x%lx\n", Result);
            break;
        }

        for ( No = 0; No < tds__GetServicesResponse->__sizeService; No++)
        {
            tds__GetServicesResponse->Service[No].XAddr          = (char *)soap_malloc_zero( soap_ptr, sizeof(char) * INFO_LENGTH);
            tds__GetServicesResponse->Service[No].Namespace      = soap_strdup(soap_ptr, g_WsdlArry[No].Wsdl);
            sprintf( tds__GetServicesResponse->Service[No].XAddr, "http://%s:%d/%s", NetInterface[0].s_IPAddr, HttpPort, g_WsdlArry[No].SvrName );
            tds__GetServicesResponse->Service[No].Capabilities   = NULL;
            tds__GetServicesResponse->Service[No].Version        = (struct tt__OnvifVersion *)soap_malloc_zero(soap_ptr, sizeof(struct tt__OnvifVersion));
            tds__GetServicesResponse->Service[No].Version->Major = ONVIF_VERSION_MAJOR;
            tds__GetServicesResponse->Service[No].Version->Minor = ONVIF_VERSION_MINOR;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServiceCapabilities(struct soap *soap_ptr, struct _tds__GetServiceCapabilities *tds__GetServiceCapabilities, struct _tds__GetServiceCapabilitiesResponse *tds__GetServiceCapabilitiesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDeviceInformation(struct soap *soap_ptr, struct _tds__GetDeviceInformation *tds__GetDeviceInformation, struct _tds__GetDeviceInformationResponse *tds__GetDeviceInformationResponse)
{
    uint16_t SessionId = 0;
    uint32_t AuthValue = 0;
    GMI_RESULT Result = GMI_SUCCESS;
    SysPkgDeviceInfo SysPkgDevInfo;

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);

    //user auth
    do
    {
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Soap_WSSE_Authentication fail, Result = 0x%lx\n", Result);
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        Result = SysGetDeviceInfo(SessionId, AuthValue, &SysPkgDevInfo);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetDeviceInfo fail, Result = 0x%lx.........\n", __func__, Result);
            break;
        }

        tds__GetDeviceInformationResponse->Model           = soap_strdup(soap_ptr, SysPkgDevInfo.s_DeviceModel);
        tds__GetDeviceInformationResponse->Manufacturer    = soap_strdup(soap_ptr, SysPkgDevInfo.s_DeviceManufactuer);
        tds__GetDeviceInformationResponse->HardwareId      = soap_strdup(soap_ptr, SysPkgDevInfo.s_DeviceHwVer);
        tds__GetDeviceInformationResponse->SerialNumber    = soap_strdup(soap_ptr, SysPkgDevInfo.s_DeviceSerialNum);
        tds__GetDeviceInformationResponse->FirmwareVersion = soap_strdup(soap_ptr, SysPkgDevInfo.s_DeviceFwVer);

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnoraml Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetSystemDateAndTime(struct soap *soap_ptr, struct _tds__SetSystemDateAndTime *tds__SetSystemDateAndTime, struct _tds__SetSystemDateAndTimeResponse *tds__SetSystemDateAndTimeResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t SessionId = 0;
    uint32_t AuthValue = 0;
    GMI_RESULT Result = GMI_SUCCESS;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user auth fail, Result = 0x%lx\n", Result);
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "DateTimeType       = %d\n", tds__SetSystemDateAndTime->DateTimeType);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "DaylightSavings    = %d\n", tds__SetSystemDateAndTime->DaylightSavings);
        if (tds__SetSystemDateAndTime->TimeZone
                && tds__SetSystemDateAndTime->TimeZone->TZ
                && tds__SetSystemDateAndTime->UTCDateTime
                && tds__SetSystemDateAndTime->UTCDateTime->Date
                && tds__SetSystemDateAndTime->UTCDateTime->Time)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TimeZone           = %s\n", tds__SetSystemDateAndTime->TimeZone->TZ);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Year               = %d\n", tds__SetSystemDateAndTime->UTCDateTime->Date->Year);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Moth               = %d\n", tds__SetSystemDateAndTime->UTCDateTime->Date->Month);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Day                = %d\n", tds__SetSystemDateAndTime->UTCDateTime->Date->Day);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Hour               = %d\n", tds__SetSystemDateAndTime->UTCDateTime->Time->Hour);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Min                = %d\n", tds__SetSystemDateAndTime->UTCDateTime->Time->Minute);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Second             = %d\n", tds__SetSystemDateAndTime->UTCDateTime->Time->Second);
        }

        //timezone
        if (tds__SetSystemDateAndTime->TimeZone
                && tds__SetSystemDateAndTime->TimeZone->TZ)
        {
            if (0 == strcmp(tds__SetSystemDateAndTime->TimeZone->TZ, "INVALIDTIMEZONE"))
            {
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "invalid timezone\n");
                ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:InvalidTimeZone", "The argument value is invalid");
                break;
            }
        }

        //datetime
        if (tds__SetSystemDateAndTime->UTCDateTime
                && tds__SetSystemDateAndTime->UTCDateTime->Date
                && tds__SetSystemDateAndTime->UTCDateTime->Time)
        {
            if ((tds__SetSystemDateAndTime->UTCDateTime->Time->Hour > 24)
                    || (tds__SetSystemDateAndTime->UTCDateTime->Time->Minute > 60)
                    || (tds__SetSystemDateAndTime->UTCDateTime->Time->Second > 60)
                    || (tds__SetSystemDateAndTime->UTCDateTime->Date->Month > 12)
                    || (tds__SetSystemDateAndTime->UTCDateTime->Date->Day > 31))
            {
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "invalid datetime\n");
                ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:InvalidDateTime", "The argument value is invalid");
                break;
            }
        }


        SysPkgSysTime SysTime;
        SysPkgTimeZone SysTimezone;
        SysPkgDateTimeType SysDateTimeType;
        SysPkgNtpServerInfo SysNtpServerInfo;

        Result = SysGetTime(SessionId, AuthValue, &SysDateTimeType, &SysTime, &SysTimezone, &SysNtpServerInfo);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetTime fail, Result = 0x%lx\n", Result);
            break;
        }

        boolean_t Failure = false;

        switch (tds__SetSystemDateAndTime->DateTimeType)
        {
        case tt__SetDateTimeType__Manual:
            struct tm     UTCTime;
            time_t        TimeTmp;
            struct tm    *TodayPtr;

            memset(&SysTime, 0, sizeof(SysPkgSysTime));
            memset(&SysTimezone, 0, sizeof(SysPkgTimeZone));
            UTCTime.tm_year   = tds__SetSystemDateAndTime->UTCDateTime->Date->Year  - 1900;
            UTCTime.tm_mon    = tds__SetSystemDateAndTime->UTCDateTime->Date->Month - 1;
            UTCTime.tm_mday   = tds__SetSystemDateAndTime->UTCDateTime->Date->Day;
            UTCTime.tm_hour   = tds__SetSystemDateAndTime->UTCDateTime->Time->Hour;
            UTCTime.tm_min    = tds__SetSystemDateAndTime->UTCDateTime->Time->Minute;
            UTCTime.tm_sec    = tds__SetSystemDateAndTime->UTCDateTime->Time->Second;
            TimeTmp = mktime(&UTCTime);

            //user set timezone
            if (tds__SetSystemDateAndTime->TimeZone
                    && tds__SetSystemDateAndTime->TimeZone->TZ)
            {
                if (0 == strcmp(tds__SetSystemDateAndTime->TimeZone->TZ, "CST-8")
                        || 0 == strcmp(tds__SetSystemDateAndTime->TimeZone->TZ, "UTC+08:00")
                        || 0 == strcmp(tds__SetSystemDateAndTime->TimeZone->TZ, "ChinaStandardTime-8")
                        || 0 == strcmp(tds__SetSystemDateAndTime->TimeZone->TZ, "SingaporeStandardTime-8")
                        || 0 == strcmp(tds__SetSystemDateAndTime->TimeZone->TZ, "WAustraliaStandardTime-8")
                        || 0 == strcmp(tds__SetSystemDateAndTime->TimeZone->TZ, "TaipeiStandardTime-8")
                        || 0 == strcmp(tds__SetSystemDateAndTime->TimeZone->TZ, "UlaanbaatarStandardTime-8")
                        || 0 == strcmp(tds__SetSystemDateAndTime->TimeZone->TZ, "NorthAsiaEastStandardTime-8Dayli")
                        || strstr(tds__SetSystemDateAndTime->TimeZone->TZ, "+08"))
                {
                    TimeTmp += 8*60*60;
                    SysTimezone.s_TimeZone = 8;
                }
                else
                {
                    SysTimezone.s_TimeZone = 0;
                }
            }
            else//user not set timezone
            {
                switch (SysTimezone.s_TimeZone)
                {
                case 8://CST
                    TimeTmp += 8*60*60;
                    break;
                case 0://UTC
                    TimeTmp += 0;
                    break;
                default:
                    break;
                }
            }

            //local time
            TodayPtr = localtime(&TimeTmp);
            SysTime.s_Year   = TodayPtr->tm_year + 1900;
            SysTime.s_Month  = TodayPtr->tm_mon  + 1;
            SysTime.s_Day    = TodayPtr->tm_mday;
            SysTime.s_Hour   = TodayPtr->tm_hour;
            SysTime.s_Minute = TodayPtr->tm_min;
            SysTime.s_Second = TodayPtr->tm_sec;

            SysDateTimeType.s_Type = SYS_TIME_TYPE_MANUAL;
            Result = SysSetTime(SessionId, AuthValue, &SysDateTimeType, &SysTime, &SysTimezone, &SysNtpServerInfo);
            if (FAILED(Result))
            {
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysSetTime fail, Result = 0x%lx\n", Result);
                Failure = true;
                break;
            }
            //should set to sys control server
            break;
        case tt__SetDateTimeType__NTP:
            SysDateTimeType.s_Type = SYS_TIME_TYPE_NTP;
            Result = SysSetTime(SessionId, AuthValue, &SysDateTimeType, &SysTime, &SysTimezone, &SysNtpServerInfo);
            if (FAILED(Result))
            {
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysSetTime fail, Result = 0x%lx\n", Result);
                Failure = true;
                break;
            }
            break;
        default:
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "not support DateTimeType %d\n", tds__SetSystemDateAndTime->DateTimeType);
            Failure = true;
            break;
        }
        if (Failure)
        {
            break;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemDateAndTime(struct soap *soap_ptr, struct _tds__GetSystemDateAndTime *tds__GetSystemDateAndTime, struct _tds__GetSystemDateAndTimeResponse *tds__GetSystemDateAndTimeResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t SessionId = 0;
    uint32_t AuthValue = 0;
    GMI_RESULT     Result = GMI_SUCCESS;
    SysPkgTimeZone SysTimezone;

    do
    {
        //get system DateAndTime
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime                    = (struct tt__SystemDateTime*)soap_malloc_zero(soap_ptr, sizeof(struct tt__SystemDateTime));
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone          = (struct tt__TimeZone *)soap_malloc_zero(soap_ptr, sizeof(struct tt__TimeZone));
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime       = (struct tt__DateTime *)soap_malloc_zero(soap_ptr, sizeof(struct tt__DateTime));
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time = (struct tt__Time *)soap_malloc_zero(soap_ptr, sizeof(struct tt__Time));
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date = (struct tt__Date *)soap_malloc_zero(soap_ptr, sizeof(struct tt__Date));
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime       = (struct tt__DateTime *)soap_malloc_zero(soap_ptr, sizeof(struct tt__DateTime));
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time = (struct tt__Time *)soap_malloc_zero(soap_ptr, sizeof(struct tt__Time));
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date = (struct tt__Date *)soap_malloc_zero(soap_ptr, sizeof(struct tt__Date));
        if ( NULL == tds__GetSystemDateAndTimeResponse->SystemDateAndTime
                || NULL == tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone
                || NULL == tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime
                || NULL == tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time
                || NULL == tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "soap malloc SystemDateAndTime failed\n");
            break;
        }

        //should get from sys control server

        //get sys time
        //get timezone
        time_t     Time;
        struct tm *TodayPtr;
        //local time
        Time = time(NULL);
        TodayPtr = localtime(&Time);
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Year    = TodayPtr->tm_year + 1900;
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Month   = TodayPtr->tm_mon + 1;
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Day     = TodayPtr->tm_mday;
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Hour    = TodayPtr->tm_hour;
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Minute  = TodayPtr->tm_min;
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Second  = TodayPtr->tm_sec;


        //utc time
        Time = time(NULL);
        TodayPtr = gmtime(&Time);
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Year    = TodayPtr->tm_year + 1900;
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Month   = TodayPtr->tm_mon + 1;
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Day     = TodayPtr->tm_mday;
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Hour    = TodayPtr->tm_hour;
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Minute  = TodayPtr->tm_min;
        tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Second  = TodayPtr->tm_sec;

        //timezone
        Result = SysGetTimezone(SessionId, AuthValue,&SysTimezone);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetTimezone fail, Result = 0x%lx\n", Result);
            break;
        }

        if (8 == SysTimezone.s_TimeZone)
        {
            tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ = (char *)soap_strdup(soap_ptr, "CST-8");
        }
        else
        {
            tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ = (char *)soap_strdup(soap_ptr, SysTimezone.s_TimzeZoneName);
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetSystemFactoryDefault(struct soap *soap_ptr, struct _tds__SetSystemFactoryDefault *tds__SetSystemFactoryDefault, struct _tds__SetSystemFactoryDefaultResponse *tds__SetSystemFactoryDefaultResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t SessionId = 0;
    uint32_t AuthValue = 0;
    GMI_RESULT Result = GMI_SUCCESS;

    do
    {
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user authen fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        boolean_t Failure = false;

        switch (tds__SetSystemFactoryDefault->FactoryDefault)
        {
        case tt__FactoryDefaultType__Hard:
            Result = FactoryDefaultAll(SessionId, AuthValue);
            if (FAILED(Result))
            {
                Failure = true;
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SYS_SYSTEM_CTRL_DEFAULT_HARD fail, Result = 0x%lx\n", Result);
                break;
            }
            break;
        case tt__FactoryDefaultType__Soft:
            Result = FactorySimpleDefaultAll(SessionId, AuthValue);
            if (FAILED(Result))
            {
                Failure = true;
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SYS_SYSTEM_CTRL_DEFAULT_SOFT fail, Result = 0x%lx\n", Result);
                break;
            }
            break;
        default:
            Failure = true;
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "not support this FactoryDefault %d\n", tds__SetSystemFactoryDefault->FactoryDefault);
            break;
        }

        if (Failure)
        {
            break;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}



SOAP_FMAC5 int SOAP_FMAC6 __tds__UpgradeSystemFirmware(struct soap *soap_ptr, struct _tds__UpgradeSystemFirmware *tds__UpgradeSystemFirmware, struct _tds__UpgradeSystemFirmwareResponse *tds__UpgradeSystemFirmwareResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SystemReboot(struct soap *soap_ptr, struct _tds__SystemReboot *tds__SystemReboot, struct _tds__SystemRebootResponse *tds__SystemRebootResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t SessionId = 0;
    uint32_t AuthValue = 0;
    GMI_RESULT Result = GMI_SUCCESS;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user authen fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        Result = SysReboot(SessionId, AuthValue);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysCtrlSystem fail, Result = 0x%lx\n", Result);
            break;
        }
        tds__SystemRebootResponse->Message = soap_strdup(soap_ptr, REBOOT_MESSAGE);

        if(tt__DiscoveryMode__Discoverable == g_DiscoveryMode)
		{
			DevStatusVaryNotifyService(TYPE_BYE);
		}

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__RestoreSystem(struct soap *soap_ptr, struct _tds__RestoreSystem *tds__RestoreSystem, struct _tds__RestoreSystemResponse *tds__RestoreSystemResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemBackup(struct soap *soap_ptr, struct _tds__GetSystemBackup *tds__GetSystemBackup, struct _tds__GetSystemBackupResponse *tds__GetSystemBackupResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemLog(struct soap *soap_ptr, struct _tds__GetSystemLog *tds__GetSystemLog, struct _tds__GetSystemLogResponse *tds__GetSystemLogResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    GMI_RESULT Result                  = GMI_SUCCESS;
    char_t TestSysLog[INFO_LENGTH]     = "test system log";
    char_t TestAccessLog[INFO_LENGTH]  = "test Access log";


    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user authen fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        boolean_t Failure = false;
        //get log info
        //log information should be get from sys control server
        switch (tds__GetSystemLog->LogType)
        {
        case tt__SystemLogType__System:
            tds__GetSystemLogResponse->SystemLog = (struct tt__SystemLog*)soap_malloc_zero(soap_ptr, sizeof(struct tt__SystemLog));
            tds__GetSystemLogResponse->SystemLog->String = soap_strdup(soap_ptr, TestSysLog);
            break;
        case tt__SystemLogType__Access:
            tds__GetSystemLogResponse->SystemLog = (struct tt__SystemLog*)soap_malloc_zero(soap_ptr, sizeof(struct tt__SystemLog));
            tds__GetSystemLogResponse->SystemLog->String = soap_strdup(soap_ptr, TestAccessLog);
            break;
        default:
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "not support LogType %d\n", tds__GetSystemLog->LogType);
            Failure = true;
            break;
        }
        if (Failure)
        {
            break;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemSupportInformation(struct soap *soap_ptr, struct _tds__GetSystemSupportInformation *tds__GetSystemSupportInformation, struct _tds__GetSystemSupportInformationResponse *tds__GetSystemSupportInformationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetScopes(struct soap *soap_ptr, struct _tds__GetScopes *tds__GetScopes, struct _tds__GetScopesResponse *tds__GetScopesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    int scopesCount = 0;
    int n_i = 0;

    do
    {
        for (n_i = 0; n_i < DEFAULT_VALID_SCOPE_NUM; n_i++)
        {
            if (0 != strlen(g_DeviceScopes[n_i].s_ScopeUrl))
            {
                scopesCount++;
            }
        }

        tds__GetScopesResponse->__sizeScopes = scopesCount;
        tds__GetScopesResponse->Scopes = (struct tt__Scope *)soap_malloc_zero(soap_ptr, sizeof(struct tt__Scope) * tds__GetScopesResponse->__sizeScopes);

        if (NULL == tds__GetScopesResponse->Scopes)
        {
            ONVIF_ERROR("Scopes soap malloc failed\n");
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Scopes soap malloc failed\n");
            break;
        }

        for (n_i = 0; n_i < tds__GetScopesResponse->__sizeScopes; n_i++)
        {
            tds__GetScopesResponse->Scopes[n_i].ScopeDef = g_DeviceScopes[n_i].s_ScopeDefinitionFlag;
            tds__GetScopesResponse->Scopes[n_i].ScopeItem = soap_strdup(soap_ptr, g_DeviceScopes[n_i].s_ScopeUrl);
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetScopes(struct soap *soap_ptr, struct _tds__SetScopes *tds__SetScopes, struct _tds__SetScopesResponse *tds__SetScopesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);    
	int ScopesNum = 0;
	int n_i = 0;
	int n_k = 0;
	int IsHaveUrl = 0;

    do
    {
		if (NULL == tds__SetScopes)
		{			
			break;
		}

		ScopesNum = tds__SetScopes->__sizeScopes;

		if (0 == ScopesNum)
		{
			g_ChangeScopes = 0;
			for(n_k=DEFAULT_USED_SCOPE_NUM; n_k<DEFAULT_VALID_SCOPE_NUM; n_k++)
			{
				memset(g_DeviceScopes[n_k].s_ScopeUrl, 0, sizeof(g_DeviceScopes[n_k].s_ScopeUrl));
			}
			SaveTestValueBeforeReboot();
			break;
		}

		for (n_i = 0; n_i < ScopesNum; n_i++)
		{
			IsHaveUrl = 0;
			if(n_i > (DEFAULT_VALID_SCOPE_NUM-1))
			{	
				break;
			}	

			for(n_k=0; n_k<DEFAULT_VALID_SCOPE_NUM; n_k++)
			{
				if((0 != strlen(g_DeviceScopes[n_k].s_ScopeUrl)) 
					&& (NULL != tds__SetScopes->Scopes[n_i])
					&& (0 == strncmp(g_DeviceScopes[n_k].s_ScopeUrl, tds__SetScopes->Scopes[n_i], strlen(g_DeviceScopes[n_k].s_ScopeUrl))))
				{
					IsHaveUrl = 1;
					break;
				}
			}
		}

		if (1 == IsHaveUrl)
		{
			ONVIF_Fault(soap_ptr, "ter:OperationProhibited", "ScopeOverwrite", "Scope parameter overwrites fixed device scope setting, command rejected.");
			break;
		}

		n_i = 0;
		for (n_k = 0; n_k < DEFAULT_VALID_SCOPE_NUM; n_k++)
		{
			if(0 != strlen(g_DeviceScopes[n_k].s_ScopeUrl))
			{
				n_i++;
			}
		}
		if ((ScopesNum+n_i) > DEFAULT_VALID_SCOPE_NUM)
		{			
			break;
		}

		n_i = 0;
		for (n_k = 0; n_k < DEFAULT_VALID_SCOPE_NUM; n_k++)
		{
			if((0 == strlen(g_DeviceScopes[n_k].s_ScopeUrl))
				&& (NULL != tds__SetScopes->Scopes[n_i]))
			{
				memcpy(g_DeviceScopes[n_k].s_ScopeUrl, tds__SetScopes->Scopes[n_i], min(strlen(tds__SetScopes->Scopes[n_i]), sizeof(g_DeviceScopes[n_k].s_ScopeUrl)-1));	
				n_i++;
				if(n_i >= ScopesNum)
				{
					break;
				}
			}
		}

		if (n_i > 0)
		{
			g_ChangeScopes = 1;
			SaveTestValueBeforeReboot();			
		}
	    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
	    ONVIF_INFO("%s normal out.........\n", __func__);
	    return SOAP_OK;
    }
    while (0);

	DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__AddScopes(struct soap *soap_ptr, struct _tds__AddScopes *tds__AddScopes, struct _tds__AddScopesResponse *tds__AddScopesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    int ScopesCount = 0;
	int n_i = 0;
	int n_k = 0;
	int AddScopeNum = 0;

	for (n_i = 0; n_i < DEFAULT_VALID_SCOPE_NUM; n_i++)
	{
		if(0 != strlen(g_DeviceScopes[n_i].s_ScopeUrl))
		{
			ScopesCount++;
		}
	}

	do
	{
		AddScopeNum = tds__AddScopes->__sizeScopeItem;
		if ((AddScopeNum+ScopesCount) > DEFAULT_VALID_SCOPE_NUM)
		{			
			break;
		}

		for (n_i = 0; n_i < DEFAULT_VALID_SCOPE_NUM; n_i++)
		{
			if((0 == strlen(g_DeviceScopes[n_i].s_ScopeUrl))
				&& (NULL != tds__AddScopes->ScopeItem[n_k]))
			{
				memcpy(g_DeviceScopes[n_i].s_ScopeUrl, tds__AddScopes->ScopeItem[n_k], min(strlen(tds__AddScopes->ScopeItem[n_k]), sizeof(g_DeviceScopes[n_i].s_ScopeUrl)-1));	
				n_k++;
				if(n_k >= AddScopeNum)
				{
					break;
				}
			}
		}

		if (n_k > 0)
		{
			g_ChangeScopes = 1;
			SaveTestValueBeforeReboot();
			if(tt__DiscoveryMode__Discoverable == g_DiscoveryMode)
			{
				DevStatusVaryNotifyService(TYPE_HELLO);
			}
		}
	    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
	    ONVIF_INFO("%s normal out.........\n", __func__);
	    return SOAP_OK;
    }
    while(0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveScopes(struct soap *soap_ptr, struct _tds__RemoveScopes *tds__RemoveScopes, struct _tds__RemoveScopesResponse *tds__RemoveScopesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    int n_k = 0;
	int n_i = 0;	
	int IsHaveUrl = 0;

	do
	{
		if (0 >= tds__RemoveScopes->__sizeScopeItem)
		{
			break;
		}

		for (n_i = 0; n_i < tds__RemoveScopes->__sizeScopeItem; n_i++)
		{
			for (n_k = DEFAULT_USED_SCOPE_NUM; n_k < DEFAULT_VALID_SCOPE_NUM; n_k++)
			{
				if((0 != strlen(g_DeviceScopes[n_k].s_ScopeUrl)) 
					&& (NULL != tds__RemoveScopes->ScopeItem[n_i])
					&&(strlen(g_DeviceScopes[n_k].s_ScopeUrl) == strlen(tds__RemoveScopes->ScopeItem[n_i]))
					&&(0 == strncmp(g_DeviceScopes[n_k].s_ScopeUrl, tds__RemoveScopes->ScopeItem[n_i], strlen(tds__RemoveScopes->ScopeItem[n_i]))))
				{
					memset(g_DeviceScopes[n_k].s_ScopeUrl, 0, sizeof(g_DeviceScopes[n_k].s_ScopeUrl));
					IsHaveUrl = 1;
				}
			}
		}

		if(1 == IsHaveUrl)
		{
			g_ChangeScopes = 1;
			SaveTestValueBeforeReboot();
			if(tt__DiscoveryMode__Discoverable == g_DiscoveryMode)
			{
				DevStatusVaryNotifyService(TYPE_HELLO);
			}
		}
		
		DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
	    ONVIF_INFO("%s normal out.........\n", __func__);
	    return SOAP_OK;
	}
	while(0);
	
	DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
	ONVIF_INFO("%s Abnormal out.........\n", __func__);
	return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDiscoveryMode(struct soap *soap_ptr, struct _tds__GetDiscoveryMode *tds__GetDiscoveryMode, struct _tds__GetDiscoveryModeResponse *tds__GetDiscoveryModeResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    tds__GetDiscoveryModeResponse->DiscoveryMode = g_DiscoveryMode;
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDiscoveryMode(struct soap *soap_ptr, struct _tds__SetDiscoveryMode *tds__SetDiscoveryMode, struct _tds__SetDiscoveryModeResponse *tds__SetDiscoveryModeResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    g_DiscoveryMode= tds__SetDiscoveryMode->DiscoveryMode;
	SaveTestValueBeforeReboot();
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteDiscoveryMode(struct soap *soap_ptr, struct _tds__GetRemoteDiscoveryMode *tds__GetRemoteDiscoveryMode, struct _tds__GetRemoteDiscoveryModeResponse *tds__GetRemoteDiscoveryModeResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteDiscoveryMode(struct soap *soap_ptr, struct _tds__SetRemoteDiscoveryMode *tds__SetRemoteDiscoveryMode, struct _tds__SetRemoteDiscoveryModeResponse *tds__SetRemoteDiscoveryModeResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDPAddresses(struct soap *soap_ptr, struct _tds__GetDPAddresses *tds__GetDPAddresses, struct _tds__GetDPAddressesResponse *tds__GetDPAddressesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetEndpointReference(struct soap *soap_ptr, struct _tds__GetEndpointReference *tds__GetEndpointReference, struct _tds__GetEndpointReferenceResponse *tds__GetEndpointReferenceResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteUser(struct soap *soap_ptr, struct _tds__GetRemoteUser *tds__GetRemoteUser, struct _tds__GetRemoteUserResponse *tds__GetRemoteUserResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteUser(struct soap *soap_ptr, struct _tds__SetRemoteUser *tds__SetRemoteUser, struct _tds__SetRemoteUserResponse *tds__SetRemoteUserResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetUsers(struct soap *soap_ptr, struct _tds__GetUsers *tds__GetUsers, struct _tds__GetUsersResponse *tds__GetUsersResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t SessionId = 0;
    uint32_t AuthValue = 0;
    uint32_t        Cnt;
    uint32_t        RealUserCnt;
    GMI_RESULT      Result   = GMI_SUCCESS;
    SysPkgUserInfo *SysUsersPtr = NULL;

    do
    {
        SysUsersPtr = (SysPkgUserInfo*)malloc(MAX_USERS * sizeof(SysPkgUserInfo));
        if (NULL == SysUsersPtr)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysUsersPtr malloc failed\n");
            break;
        }

        Result = SysGetUsers(SessionId, AuthValue, SysUsersPtr, MAX_USERS, &RealUserCnt);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetUsers fail, Result = 0x%lx\n", Result);
            break;
        }

        tds__GetUsersResponse->__sizeUser = RealUserCnt;
        tds__GetUsersResponse->User = (struct tt__User*)soap_malloc_zero(soap_ptr, (RealUserCnt * sizeof(struct tt__User)));
        for (Cnt = 0; Cnt < RealUserCnt; Cnt++)
        {
            tds__GetUsersResponse->User[Cnt].Username  = soap_strdup(soap_ptr, SysUsersPtr[Cnt].s_UserName);
            switch (SysUsersPtr[Cnt].s_UserFlag)
            {
            case 1:
                tds__GetUsersResponse->User[Cnt].UserLevel = tt__UserLevel__Administrator;
                break;
            case 2:
                tds__GetUsersResponse->User[Cnt].UserLevel = tt__UserLevel__Operator;
                break;
            case 3:
                tds__GetUsersResponse->User[Cnt].UserLevel = tt__UserLevel__User;
                break;
            default:
                tds__GetUsersResponse->User[Cnt].UserLevel = tt__UserLevel__Anonymous;
                break;
            }
            tds__GetUsersResponse->User[Cnt].Extension = NULL;
        }

        if (NULL != SysUsersPtr)
        {
            free(SysUsersPtr);
            SysUsersPtr = NULL;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    if (NULL != SysUsersPtr)
    {
        free(SysUsersPtr);
        SysUsersPtr = NULL;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateUsers(struct soap *soap_ptr, struct _tds__CreateUsers *tds__CreateUsers, struct _tds__CreateUsersResponse *tds__CreateUsersResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t SessionId = 0;
    uint32_t AuthValue = 0;
    int32_t        Cnt;
    GMI_RESULT     Result  = GMI_SUCCESS;
    SysPkgUserInfo SysUserInfo;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user authen fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        boolean_t Failure = false;

        for (Cnt = 0; Cnt < tds__CreateUsers->__sizeUser; Cnt++)
        {
            if (strlen(tds__CreateUsers->User[Cnt].Username) > USER_LENGTH)
            {
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "tds__CreateUsers->User[Cnt].Username length %d exceed\n", strlen(tds__CreateUsers->User[Cnt].Username));
                ONVIF_Fault(soap_ptr, "ter:OperationProhibited", "ter:UsernameTooLong", "The requested operation is not permitted by the device");
                Failure = true;
                break;
            }

            if (strlen(tds__CreateUsers->User[Cnt].Password) > PASSWD_LENGTH)
            {
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "tds__CreateUsers->User[Cnt].Password length %d exceed\n", strlen(tds__CreateUsers->User[Cnt].Password));
                ONVIF_Fault(soap_ptr, "ter:OperationProhibited", "ter:PasswordTooLong", "The requested operation is not permitted by the device");
                Failure = true;
                break;
            }

            if (tds__CreateUsers->User[Cnt].UserLevel > 4
                    || tds__CreateUsers->User[Cnt].UserLevel < 0)
            {
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "UserLevel %d error\n", tds__CreateUsers->User[Cnt].UserLevel);
                ONVIF_Fault(soap_ptr, "ter:OperationProhibited", "ter:AnonymousNotAllowed", "The requested operation is not permitted by the device");
                Failure = true;
                break;
            }
        }

        if (Failure)
        {
            break;
        }

        for (Cnt = 0; Cnt < tds__CreateUsers->__sizeUser; Cnt++)
        {
            memset(&SysUserInfo, 0, sizeof(SysPkgUserInfo));
            memcpy(SysUserInfo.s_UserName, tds__CreateUsers->User[Cnt].Username, sizeof(SysUserInfo.s_UserName));
            memcpy(SysUserInfo.s_UserPass, tds__CreateUsers->User[Cnt].Password, sizeof(SysUserInfo.s_UserPass));
            switch (tds__CreateUsers->User[Cnt].UserLevel)
            {
            case tt__UserLevel__Administrator:
                SysUserInfo.s_UserFlag = 1;
                break;
            case tt__UserLevel__Operator:
                SysUserInfo.s_UserFlag = 2;
                break;
            case tt__UserLevel__User:
                SysUserInfo.s_UserFlag = 3;
                break;
            default:
                SysUserInfo.s_UserFlag = 0xffff;
                break;
            }

            Result = SysSetUsers(SessionId, AuthValue, &SysUserInfo);
            if (FAILED(Result))
            {
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysSetUsers fail, Result = 0x%lx\n", Result);
                Failure = true;
                break;
            }
        }

        if (Failure)
        {
            break;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteUsers(struct soap *soap_ptr, struct _tds__DeleteUsers *tds__DeleteUsers, struct _tds__DeleteUsersResponse *tds__DeleteUsersResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t SessionId = 0;
    uint32_t AuthValue = 0;
    int32_t        Cnt;
    GMI_RESULT     Result  = GMI_SUCCESS;
    SysPkgUserInfo SysUserInfo;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user authen fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        boolean_t Failure = false;

        for (Cnt = 0; Cnt < tds__DeleteUsers->__sizeUsername; Cnt++)
        {
            if ((strlen(tds__DeleteUsers->Username[Cnt])) == 0)
            {
                ONVIF_Fault(soap_ptr, "ter:OperationProhibited", "ter:AnonymousNotAllowed", "The requested operation is not permitted by the device");
                Failure = true;
                break;
            }

            memset(&SysUserInfo, 0, sizeof(SysPkgUserInfo));
            memcpy(SysUserInfo.s_UserName, tds__DeleteUsers->Username[Cnt], sizeof(SysUserInfo.s_UserName));

            Result = SysDelUsers(SessionId, AuthValue, &SysUserInfo);
            if (FAILED(Result))
            {
                Failure = true;
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysSetUsers fail, Result = 0x%lx\n", Result);
                break;
            }
        }

        if (Failure)
        {
            break;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while(0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetUser(struct soap *soap_ptr, struct _tds__SetUser *tds__SetUser, struct _tds__SetUserResponse *tds__SetUserResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t SessionId = 0;
    uint32_t AuthValue = 0;
    int32_t        Cnt;
    GMI_RESULT     Result  = GMI_SUCCESS;
    SysPkgUserInfo SysUserInfo;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user authen fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        boolean_t Failure = false;

        for (Cnt = 0; Cnt < tds__SetUser->__sizeUser; Cnt++)
        {
            if (strlen(tds__SetUser->User[Cnt].Username) > USER_LENGTH)
            {
                ONVIF_Fault(soap_ptr, "ter:OperationProhibited", "ter:UsernameTooLong", "The requested operation is not permitted by the device");
                Failure = true;
                break;
            }

            if (strlen(tds__SetUser->User[Cnt].Password) > PASSWD_LENGTH)
            {
                ONVIF_Fault(soap_ptr, "ter:OperationProhibited", "ter:PasswordTooLong", "The requested operation is not permitted by the device");
                Failure = true;
                break;
            }

            if (strlen(tds__SetUser->User[Cnt].Password) == 0)
            {
                ONVIF_Fault(soap_ptr, "ter:OperationProhibited", "ter:PasswordTooWeak", "The requested operation is not permitted by the device");
                Failure = true;
                break;
            }

            if (tds__SetUser->User[Cnt].UserLevel == 0
                    || tds__SetUser->User[Cnt].UserLevel > 4
                    || tds__SetUser->User[Cnt].UserLevel < 0)
            {
                ONVIF_Fault(soap_ptr, "ter:OperationProhibited", "ter:AnonymousNotAllowed", "The requested operation is not permitted by the device");
                Failure = true;
                break;
            }
        }

        if (Failure)
        {
            break;
        }

        for (Cnt = 0; Cnt < tds__SetUser->__sizeUser; Cnt++)
        {
            memset(&SysUserInfo, 0, sizeof(SysPkgUserInfo));
            memcpy(SysUserInfo.s_UserName, tds__SetUser->User[Cnt].Username, sizeof(SysUserInfo.s_UserName));
            memcpy(SysUserInfo.s_UserPass, tds__SetUser->User[Cnt].Password, sizeof(SysUserInfo.s_UserPass));
            switch (tds__SetUser->User[Cnt].UserLevel)
            {
            case tt__UserLevel__Administrator:
                SysUserInfo.s_UserFlag = 1;
                break;
            case tt__UserLevel__Operator:
                SysUserInfo.s_UserFlag = 2;
                break;
            case tt__UserLevel__User:
                SysUserInfo.s_UserFlag = 3;
                break;
            default:
                SysUserInfo.s_UserFlag = 0xffff;
                break;
            }

            Result = SysSetUsers(SessionId, AuthValue, &SysUserInfo);
            if (FAILED(Result))
            {
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysSetUsers fail, Result = 0x%lx\n", Result);
                Failure = true;
                break;
            }
        }

        if (Failure)
        {
            break;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Normal.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetWsdlUrl(struct soap *soap_ptr, struct _tds__GetWsdlUrl *tds__GetWsdlUrl, struct _tds__GetWsdlUrlResponse *tds__GetWsdlUrlResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint8_t   Id;
    uint32_t  Len      = 0;
    uint32_t  Offset   = 0;
    uint8_t   SpaceLen = 2;
    char_t    *WsdlUrl = NULL;
    //GMI_RESULT Result  = GMI_SUCCESS;

    //user auth
    //Result = Soap_WSSE_Authentication(soap_ptr);
    //if (FAILED(Result))
    //{
    //	ONVIF_ERROR("user auth failed\n");
    //	goto errExit;
    //}

    do
    {
        for (Id = 0; Id < g_WsdlCount; Id++)
        {
            Len += strlen(g_WsdlArry[Id].Wsdl);
        }
        Len += Id*SpaceLen;

        WsdlUrl = (char_t*)malloc(Len);
        if (WsdlUrl == NULL)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "malloc fail\n", __func__);
            break;
        }
        memset( WsdlUrl, 0, Len );

        for (Id = 0; Id < g_WsdlCount; Id++)
        {
            sprintf((WsdlUrl + Offset), "%s", g_WsdlArry[Id].Wsdl);
            Offset += SpaceLen;
            sprintf((WsdlUrl + Offset), "%s", "");
            Offset += strlen(g_WsdlArry[Id].Wsdl);
        }

        tds__GetWsdlUrlResponse->WsdlUrl = soap_strdup(soap_ptr, WsdlUrl);

        if (NULL != WsdlUrl)
        {
            free(WsdlUrl);
            WsdlUrl = NULL;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    if (WsdlUrl != NULL)
    {
        free(WsdlUrl);
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Normal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCapabilities(struct soap *soap_ptr, struct _tds__GetCapabilities *tds__GetCapabilities, struct _tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t SessionId = 0;
    uint32_t AuthValue = 0;
    uint8_t      Id                                 = 0;
    char_t       InterfaceName[NETWORK_INFO_LENGTH] = {0};
    char_t       IP[NETWORK_INFO_LENGTH]            = {0};
    uint32_t     StreamNum                          = 0;
    uint8_t      AlmInputNum                        = 0;
    uint8_t      AlmOutputNum                       = 0;
    uint8_t      VideoSources                       = 0;
    uint8_t      VideoOutputs                       = 0;
    uint8_t      AudioSources                       = 0;
    uint8_t      AudioOutputs                       = 0;
    uint8_t      RelayOutputs                       = 0;
    int32_t      _Category                          = 0;
    int32_t      HttpPort                           = 0;
    GMI_RESULT   Result                             = GMI_SUCCESS;
    struct soap *soap                               = soap_ptr;
    DevSecurity *SecurityPtr                        = NULL;

    do
    {
        //should get from sys control server
        //get device capabilities , media capabilities, network info
        //device capabilities include  network num, ptz num, alarm input num, alarm output num, video source, video outputs, audio sources, audio outputs,
        //network info include IP
        //media capabilities include stream num
        StreamNum     = DEFAULT_STREAM_NUM;
        AlmInputNum   = DEFAULT_ALARM_INPUTS;
        AlmOutputNum  = DEFAULT_ALARM_OUTPUTS;
        VideoSources  = DEFAULT_VIDEO_SOURCES;
        VideoOutputs  = DEFAULT_VIDEO_OUTPUTS;
        AudioSources  = DEFAULT_AUDIO_SOURCES;
        AudioOutputs  = DEFAULT_AUIDO_OUTPUTS;
        memcpy(InterfaceName, DEFAULT_NETWORK_NAME, NETWORK_INFO_LENGTH);
        HttpPort      = DEFAULT_SERVER_PORT;

        Result = NET_GetIpInfo(InterfaceName, IP);
        if (FAILED(Result))
        {
            ONVIF_ERROR("NET_GetIpInfo failed\n");
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "NET_GetIpInfo fail, Result = 0x%lx\n", Result);
            break;
        }

        if (tds__GetCapabilities->Category == NULL)
        {
            _Category = tt__CapabilityCategory__All;
        }
        else
        {
            _Category = *tds__GetCapabilities->Category;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "_Category    = %d\n", _Category);

        Result = SysGetEncodeStreamNum(SessionId, AuthValue, &StreamNum);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetEncodeStreamNum fail, Result = 0x%lx\n", __func__);
            break;
        }

        tds__GetCapabilitiesResponse->Capabilities = (struct tt__Capabilities*)soap_malloc_zero(soap, sizeof(struct tt__Capabilities));

        //device
        if (_Category == tt__CapabilityCategory__All
                || _Category == tt__CapabilityCategory__Device)
        {
            tds__GetCapabilitiesResponse->Capabilities->Device            = (struct tt__DeviceCapabilities*)soap_malloc_zero(soap, sizeof(struct tt__DeviceCapabilities));
            tds__GetCapabilitiesResponse->Capabilities->Device->XAddr     = (char *) soap_malloc_zero(soap, sizeof(char) * INFO_LENGTH);
            sprintf(tds__GetCapabilitiesResponse->Capabilities->Device->XAddr, "http://%s:%d/%s", IP, HttpPort, g_WsdlArry[DEVICE_WSDL_ID].SvrName);
            tds__GetCapabilitiesResponse->Capabilities->Device->Extension = NULL;

            //network
            tds__GetCapabilitiesResponse->Capabilities->Device->Network                    = (struct tt__NetworkCapabilities*)soap_malloc_zero(soap, sizeof(struct tt__NetworkCapabilities));
            tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter          = (enum xsd__boolean*)soap_malloc_zero(soap, sizeof(int));
            tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration = (enum xsd__boolean*)soap_malloc_zero(soap, sizeof(int));
            tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6        = (enum xsd__boolean*)soap_malloc_zero(soap, sizeof(int));
            tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS            = (enum xsd__boolean*)soap_malloc_zero(soap, sizeof(int));

            *tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter          = xsd__boolean__false_;
            *tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration = xsd__boolean__false_;
            *tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6        = xsd__boolean__false_;
            *tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS            = xsd__boolean__true_;

            tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension                      = (struct tt__NetworkCapabilitiesExtension*)soap_malloc_zero(soap, sizeof(struct tt__NetworkCapabilitiesExtension));
            tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->Dot11Configuration  = (enum xsd__boolean*)soap_malloc_zero(soap, sizeof(int));

            *tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->Dot11Configuration = xsd__boolean__false_;
            tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->Extension           = NULL;
            tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->__size              = 0;
            tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->__any               = NULL;

            //system
            tds__GetCapabilitiesResponse->Capabilities->Device->System = (struct tt__SystemCapabilities*)soap_malloc_zero(soap, sizeof(struct tt__SystemCapabilities));
            tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryResolve        = xsd__boolean__false_;
            tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryBye            = xsd__boolean__false_;
            tds__GetCapabilitiesResponse->Capabilities->Device->System->RemoteDiscovery         = xsd__boolean__true_;
            tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemLogging           = xsd__boolean__false_;
            tds__GetCapabilitiesResponse->Capabilities->Device->System->FirmwareUpgrade         = xsd__boolean__false_;//xsd__boolean__true_;
            tds__GetCapabilitiesResponse->Capabilities->Device->System->__sizeSupportedVersions = ONVIF_SUPPORT_VER_NUM;
            tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions       = (struct tt__OnvifVersion*)soap_malloc_zero(soap, sizeof(struct tt__OnvifVersion)*tds__GetCapabilitiesResponse->Capabilities->Device->System->__sizeSupportedVersions);
            for (Id = 0; Id < tds__GetCapabilitiesResponse->Capabilities->Device->System->__sizeSupportedVersions; Id++)
            {
                tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions[Id].Major = ONVIF_VERSION_MAJOR;
                tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions[Id].Minor = Id;
            }
            tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension = (struct tt__SystemCapabilitiesExtension*)soap_malloc_zero(soap, sizeof(struct tt__SystemCapabilitiesExtension));
            tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemBackup = (enum xsd__boolean *)soap_malloc_zero(soap, sizeof(int));
            *tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemBackup = xsd__boolean__false_;
            tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpFirmwareUpgrade = (enum xsd__boolean *)soap_malloc_zero(soap, sizeof(int));
            *tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpFirmwareUpgrade = xsd__boolean__false_;
            tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemLogging = (enum xsd__boolean *)soap_malloc_zero(soap, sizeof(int));
            *tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemLogging = xsd__boolean__false_;
            tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSupportInformation = (enum xsd__boolean *)soap_malloc_zero(soap, sizeof(int));
            *tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSupportInformation = xsd__boolean__false_;
            tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->Extension = NULL;

            //IO
            tds__GetCapabilitiesResponse->Capabilities->Device->IO = (struct tt__IOCapabilities*)soap_malloc_zero(soap, sizeof(struct tt__IOCapabilities));
            tds__GetCapabilitiesResponse->Capabilities->Device->IO->InputConnectors  = (int*)soap_malloc_zero(soap, sizeof(int));
            tds__GetCapabilitiesResponse->Capabilities->Device->IO->RelayOutputs     = (int*)soap_malloc_zero(soap, sizeof(int));
            *tds__GetCapabilitiesResponse->Capabilities->Device->IO->InputConnectors = AlmInputNum;
            *tds__GetCapabilitiesResponse->Capabilities->Device->IO->RelayOutputs    = AlmOutputNum;
            tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension        = NULL;//(struct tt__IOCapabilitiesExtension*)soap_malloc_zero(soap, sizeof(struct tt__IOCapabilitiesExtension));

            //security
            SecurityPtr = &l_Security;
            tds__GetCapabilitiesResponse->Capabilities->Device->Security                       = (struct tt__SecurityCapabilities *)soap_malloc_zero(soap, sizeof(struct tt__SecurityCapabilities));
            tds__GetCapabilitiesResponse->Capabilities->Device->Security->__size               = 1;
            tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension            = (struct tt__SecurityCapabilitiesExtension*)soap_malloc_zero(soap, tds__GetCapabilitiesResponse->Capabilities->Device->Security->__size*sizeof(struct tt__SecurityCapabilitiesExtension));
            tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension->Extension = (struct tt__SecurityCapabilitiesExtension2*)soap_malloc_zero(soap, sizeof(struct tt__SecurityCapabilitiesExtension2));
            tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension->Extension->__sizeSupportedEAPMethod = 1;
            tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension->Extension->SupportedEAPMethod = (int*)soap_malloc_zero(soap, sizeof(int)*tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension->Extension->__sizeSupportedEAPMethod);
            tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e1          = (enum xsd__boolean)SecurityPtr->s_TLS1_x002e1;
            tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e2          = (enum xsd__boolean)SecurityPtr->s_TLS1_x002e2;
            tds__GetCapabilitiesResponse->Capabilities->Device->Security->OnboardKeyGeneration = (enum xsd__boolean)SecurityPtr->s_OnboardKeyGeneration;
            tds__GetCapabilitiesResponse->Capabilities->Device->Security->AccessPolicyConfig   = (enum xsd__boolean)SecurityPtr->s_AccessPolicyConfig;
            tds__GetCapabilitiesResponse->Capabilities->Device->Security->X_x002e509Token      = (enum xsd__boolean)SecurityPtr->s_X_x002e509Token;
            tds__GetCapabilitiesResponse->Capabilities->Device->Security->SAMLToken            = (enum xsd__boolean)SecurityPtr->s_SAMLToken;
            tds__GetCapabilitiesResponse->Capabilities->Device->Security->KerberosToken        = (enum xsd__boolean)SecurityPtr->s_KerberosToken;
            tds__GetCapabilitiesResponse->Capabilities->Device->Security->RELToken             = (enum xsd__boolean)SecurityPtr->s_RELToken;
            tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension->TLS1_x002e0            = (enum xsd__boolean)SecurityPtr->s_TLS1_x002e0;
            tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension->Extension->Dot1X       = (enum xsd__boolean)SecurityPtr->s_Dot1X;
            tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension->Extension->RemoteUserHandling = (enum xsd__boolean)SecurityPtr->s_RemoteUserHandling;
            *tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension->Extension->SupportedEAPMethod= 0;
        }

        //media
        if (_Category == tt__CapabilityCategory__All
                || _Category == tt__CapabilityCategory__Media)
        {
            tds__GetCapabilitiesResponse->Capabilities->Media        = (struct tt__MediaCapabilities*)soap_malloc_zero(soap, sizeof(struct tt__MediaCapabilities));
            tds__GetCapabilitiesResponse->Capabilities->Media->XAddr = (char*)soap_malloc_zero(soap, sizeof(char)*INFO_LENGTH);
            sprintf(tds__GetCapabilitiesResponse->Capabilities->Media->XAddr, "http://%s:%d/%s", IP, HttpPort, g_WsdlArry[MEDIA_WSDL_ID].SvrName);

            tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities = (struct tt__RealTimeStreamingCapabilities*)soap_malloc_zero(soap, sizeof(struct tt__RealTimeStreamingCapabilities));
            tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast              = ( enum xsd__boolean * )soap_malloc_zero(soap, sizeof(int));
            tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP             = ( enum xsd__boolean * )soap_malloc_zero(soap, sizeof(int));
            tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP  = ( enum xsd__boolean * )soap_malloc_zero(soap, sizeof(int));
            *tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast             = xsd__boolean__false_;
            *tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP            = xsd__boolean__true_;
            *tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = xsd__boolean__true_;
            tds__GetCapabilitiesResponse->Capabilities->Media->Extension = (struct tt__MediaCapabilitiesExtension*)soap_malloc_zero(soap, sizeof(struct tt__MediaCapabilitiesExtension));
            tds__GetCapabilitiesResponse->Capabilities->Media->Extension->ProfileCapabilities = (struct tt__ProfileCapabilities*)soap_malloc_zero(soap, sizeof(struct tt__ProfileCapabilities));
            tds__GetCapabilitiesResponse->Capabilities->Media->Extension->ProfileCapabilities->MaximumNumberOfProfiles = StreamNum;
        }

        //ptz
        if (_Category == tt__CapabilityCategory__All
                || _Category == tt__CapabilityCategory__PTZ)
        {
            tds__GetCapabilitiesResponse->Capabilities->PTZ = (struct tt__PTZCapabilities*)soap_malloc_zero(soap, sizeof(struct tt__PTZCapabilities));
            tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr = (char*)soap_malloc_zero(soap, sizeof(char*)*INFO_LENGTH);
            sprintf(tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr, "http://%s:%d/%s", IP, HttpPort, g_WsdlArry[PTZ_WSDL_ID].SvrName);
        }

        //image
        if (_Category == tt__CapabilityCategory__All
                || _Category == tt__CapabilityCategory__Imaging)
        {
            tds__GetCapabilitiesResponse->Capabilities->Imaging = NULL;//(struct tt__ImagingCapabilities*)soap_malloc(soap, sizeof(struct tt__ImagingCapabilities));
            //tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr = (char *)soap_malloc_zero(soap, sizeof(char) * INFO_LENGTH);
            //sprintf(tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr, "http://%s:%d/%s", IP, HttpPort, g_WsdlArry[IMAGING_WSDL_ID].SvrName);
        }

        //events
        if (_Category == tt__CapabilityCategory__All
                || _Category == tt__CapabilityCategory__Events)
        {
            tds__GetCapabilitiesResponse->Capabilities->Events = NULL;//(struct tt__EventCapabilities*)soap_malloc(soap, sizeof(struct tt__EventCapabilities));
            //tds__GetCapabilitiesResponse->Capabilities->Events->XAddr = (char *)soap_malloc_zero(soap, sizeof(char) * INFO_LENGTH);
            //sprintf(tds__GetCapabilitiesResponse->Capabilities->Events->XAddr, "http://%s:%d/%s", IP, HttpPort, g_WsdlArry[EVENTS_WSDL_ID].SvrName);
            //tds__GetCapabilitiesResponse->Capabilities->Events->WSSubscriptionPolicySupport = xsd__boolean__true_;
            //tds__GetCapabilitiesResponse->Capabilities->Events->WSPullPointSupport = xsd__boolean__true_;
            //tds__GetCapabilitiesResponse->Capabilities->Events->WSPausableSubscriptionManagerInterfaceSupport = xsd__boolean__false_;
        }

        //analytics
        if (_Category == tt__CapabilityCategory__All
                || _Category == tt__CapabilityCategory__Analytics)
        {
            tds__GetCapabilitiesResponse->Capabilities->Analytics = NULL;//(struct tt__AnalyticsCapabilities*)soap_malloc(soap, sizeof(struct tt__AnalyticsCapabilities));
            //tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr = (char *)soap_malloc_zero(soap, sizeof(char) * INFO_LENGTH);
            //sprintf(tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr, "http://%s:%d/%s", IP, HttpPort, g_WsdlArry[ANALYTICS_WSDL_ID].SvrName);
            //tds__GetCapabilitiesResponse->Capabilities->Analytics->RuleSupport = xsd__boolean__false_;
            //tds__GetCapabilitiesResponse->Capabilities->Analytics->AnalyticsModuleSupport = xsd__boolean__false_;
        }

        //extension
        tds__GetCapabilitiesResponse->Capabilities->Extension = (struct tt__CapabilitiesExtension*)soap_malloc_zero(soap, sizeof(struct tt__CapabilitiesExtension));
        tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO               = (struct tt__DeviceIOCapabilities*)soap_malloc_zero(soap, sizeof(struct tt__DeviceIOCapabilities));
        tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->XAddr        = (char *)soap_malloc_zero(soap, sizeof(char) * INFO_LENGTH);
        sprintf(tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->XAddr, "http://%s:%d/%s", IP, HttpPort, g_WsdlArry[DEVICE_IO_WSDL_ID].SvrName);
        tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->VideoSources = VideoSources;
        tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->VideoOutputs = VideoOutputs;
        tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->AudioSources = AudioSources;
        tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->AudioOutputs = AudioOutputs;
        tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->RelayOutputs = RelayOutputs;
        tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->__size       = 0;
        tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->__any        = NULL;
        tds__GetCapabilitiesResponse->Capabilities->Extension->Display                = NULL;
        tds__GetCapabilitiesResponse->Capabilities->Extension->Recording              = NULL;
        tds__GetCapabilitiesResponse->Capabilities->Extension->Search                 = NULL;
        tds__GetCapabilitiesResponse->Capabilities->Extension->Replay                 = NULL;
        tds__GetCapabilitiesResponse->Capabilities->Extension->Receiver               = NULL;
        tds__GetCapabilitiesResponse->Capabilities->Extension->AnalyticsDevice        = NULL;
        tds__GetCapabilitiesResponse->Capabilities->Extension->Extensions             = NULL;
        tds__GetCapabilitiesResponse->Capabilities->Extension->__size                 = 0;
        tds__GetCapabilitiesResponse->Capabilities->Extension->__any                  = NULL;

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDPAddresses(struct soap *soap_ptr, struct _tds__SetDPAddresses *tds__SetDPAddresses, struct _tds__SetDPAddressesResponse *tds__SetDPAddressesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetHostname(struct soap *soap_ptr, struct _tds__GetHostname *tds__GetHostname, struct _tds__GetHostnameResponse *tds__GetHostnameResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    GMI_RESULT Result = GMI_SUCCESS;
    char_t HostName[128];

    do
    {
        memset(HostName, 0, sizeof(HostName));
        Result = SysGetHostName(HostName);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetHostName fail, Result = 0x%lx\n", __func__);
            break;
        }

        tds__GetHostnameResponse->HostnameInformation = (struct tt__HostnameInformation*)soap_malloc_zero(soap_ptr, sizeof(struct tt__HostnameInformation));
        memset(tds__GetHostnameResponse->HostnameInformation, 0, sizeof(struct tt__HostnameInformation));
        tds__GetHostnameResponse->HostnameInformation->Name     = soap_strdup(soap_ptr, HostName);
        tds__GetHostnameResponse->HostnameInformation->FromDHCP = xsd__boolean__false_;

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostname(struct soap *soap_ptr, struct _tds__SetHostname *tds__SetHostname, struct _tds__SetHostnameResponse *tds__SetHostnameResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    GMI_RESULT Result = GMI_SUCCESS;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user auhten fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        if (NULL == tds__SetHostname->Name)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "tds__SetHostname->Name is null\n");
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:InvalidDnsName", "The argument value is invalid");
            break;
        }

        Result = NET_CheckHostname(tds__SetHostname->Name);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "tds__SetHostname->Name %s incorrect\n", tds__SetHostname->Name);
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:InvalidHostname", "The argument value is invalid");
            break;
        }

        Result = SysSetHostName(tds__SetHostname->Name);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysSetHostName fail, Result = 0x%lx\n", Result);
            break;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostnameFromDHCP(struct soap *soap_ptr, struct _tds__SetHostnameFromDHCP *tds__SetHostnameFromDHCP, struct _tds__SetHostnameFromDHCPResponse *tds__SetHostnameFromDHCPResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDNS(struct soap *soap_ptr, struct _tds__GetDNS *tds__GetDNS, struct _tds__GetDNSResponse *tds__GetDNSResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    int32_t      *PtrTmp      = NULL;
    char_t       *Dns         = NULL;
    char_t       *Domain      = NULL;

    tds__GetDNSResponse->DNSInformation = (struct tt__DNSInformation*)soap_malloc_zero(soap_ptr, sizeof(struct tt__DNSInformation));
    tds__GetDNSResponse->DNSInformation->FromDHCP        = (l_DnsInformation.s_FromDhcp ? xsd__boolean__true_ : xsd__boolean__false_);
    tds__GetDNSResponse->DNSInformation->__sizeSearchDomain = 1;
    PtrTmp     = (int32_t*)soap_malloc_zero(soap_ptr, sizeof(int32_t*));
    Domain     = (char_t*)soap_malloc_zero(soap_ptr, sizeof(l_DnsInformation.s_SearchDomain));
    *PtrTmp    = (int32_t)Domain;
    memcpy(Domain, l_DnsInformation.s_SearchDomain, sizeof(l_DnsInformation.s_SearchDomain));
    tds__GetDNSResponse->DNSInformation->SearchDomain = (char_t**)PtrTmp;

    if (tds__GetDNSResponse->DNSInformation->FromDHCP)
    {
        tds__GetDNSResponse->DNSInformation->__sizeDNSFromDHCP = 1;
        tds__GetDNSResponse->DNSInformation->DNSFromDHCP       =  (struct tt__IPAddress *)soap_malloc_zero(soap_ptr, sizeof( struct tt__IPAddress));
        tds__GetDNSResponse->DNSInformation->DNSFromDHCP->Type = tt__IPType__IPv4;
        PtrTmp     = (int32_t*)soap_malloc_zero(soap_ptr, sizeof(int32_t*));
        Dns        = (char_t*)soap_malloc_zero(soap_ptr, NETWORK_INFO_LENGTH);
        *PtrTmp    = (int32_t)Dns;
        memcpy(Dns, l_DnsInformation.s_ManualIpv4Addr, NETWORK_INFO_LENGTH);
        tds__GetDNSResponse->DNSInformation->DNSFromDHCP->IPv4Address = (char_t**)PtrTmp;
    }
    else
    {
        tds__GetDNSResponse->DNSInformation->__sizeDNSManual = 1;
        tds__GetDNSResponse->DNSInformation->DNSManual       =  (struct tt__IPAddress *)soap_malloc_zero(soap_ptr, sizeof( struct tt__IPAddress));
        tds__GetDNSResponse->DNSInformation->DNSManual->Type = tt__IPType__IPv4;
        PtrTmp     = (int32_t*)soap_malloc_zero(soap_ptr, sizeof(int32_t*));
        Dns        = (char_t*)soap_malloc_zero(soap_ptr, NETWORK_INFO_LENGTH);
        *PtrTmp    = (int32_t)Dns;
        memcpy(Dns, l_DnsInformation.s_ManualIpv4Addr, NETWORK_INFO_LENGTH);
        tds__GetDNSResponse->DNSInformation->DNSManual->IPv4Address = (char_t**)PtrTmp;
    }

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDNS(struct soap *soap_ptr, struct _tds__SetDNS *tds__SetDNS, struct _tds__SetDNSResponse *tds__SetDNSResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);

    do
    {
        if (tds__SetDNS->FromDHCP)
        {
            l_DnsInformation.s_FromDhcp = (tds__SetDNS->FromDHCP ? true : false);
        }
        else
        {
            if (tds__SetDNS->__sizeSearchDomain)
            {
                memcpy(l_DnsInformation.s_SearchDomain, *tds__SetDNS->SearchDomain, sizeof(l_DnsInformation.s_SearchDomain));
            }

            if (tds__SetDNS->__sizeDNSManual > 0)
            {
                if (tds__SetDNS->DNSManual->Type != tt__IPType__IPv4)
                {
                    ONVIF_Fault(soap_ptr, "ter:NotSupported", "ter:InvalidIPv6Address", "The argument value is invalid");
                    break;
                }

                if (NET_CheckIPString(*tds__SetDNS->DNSManual->IPv4Address))
                {
                    ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:InvalidIPv4Address", "The argument value is invalid");
                    break;
                }
                memcpy(l_DnsInformation.s_ManualIpv4Addr, *tds__SetDNS->DNSManual->IPv4Address, NETWORK_INFO_LENGTH);
            }
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNTP(struct soap *soap_ptr, struct _tds__GetNTP *tds__GetNTP, struct _tds__GetNTPResponse *tds__GetNTPResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t SessionId = 0;
    uint32_t AuthValue = 0;
    GMI_RESULT Result = GMI_SUCCESS;

    do
    {
        SysPkgSysTime SysTime;
        SysPkgTimeZone SysTimezone;
        SysPkgDateTimeType SysDateTimeType;
        SysPkgNtpServerInfo SysNtpServerInfo;
        Result = SysGetTime(SessionId, AuthValue, &SysDateTimeType, &SysTime, &SysTimezone, &SysNtpServerInfo);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetTime fail, Result = 0x%lx\n", Result);
            break;
        }

        SysPkgIpInfo  SysIpInfo;
        memset(&SysIpInfo, 0, sizeof(SysPkgIpInfo));
        Result = SysGetDeviceIP(SessionId, AuthValue, &SysIpInfo);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetDeviceIP fail, Result = 0x%lx\n", Result);
            break;
        }

        tds__GetNTPResponse->NTPInformation = (struct tt__NTPInformation*)soap_malloc_zero(soap_ptr, sizeof(struct tt__NTPInformation));
        tds__GetNTPResponse->NTPInformation->FromDHCP = (SysIpInfo.s_Dhcp ? xsd__boolean__true_ : xsd__boolean__false_);
        if (tds__GetNTPResponse->NTPInformation->FromDHCP) //FromDHCP
        {
            tds__GetNTPResponse->NTPInformation->__sizeNTPFromDHCP           = 1;
            tds__GetNTPResponse->NTPInformation->NTPFromDHCP                 = (struct tt__NetworkHost*)soap_malloc_zero(soap_ptr, sizeof(struct tt__NetworkHost));
            tds__GetNTPResponse->NTPInformation->NTPFromDHCP->Type           = tt__NetworkHostType__IPv4;
            tds__GetNTPResponse->NTPInformation->NTPFromDHCP->IPv4Address    = (char **)soap_malloc_zero(soap_ptr, sizeof(char *));
            tds__GetNTPResponse->NTPInformation->NTPFromDHCP->IPv4Address[0] = (char *)soap_malloc_zero(soap_ptr, sizeof(char) * NETWORK_INFO_LENGTH);
            strcpy(tds__GetNTPResponse->NTPInformation->NTPFromDHCP->IPv4Address[0], SysNtpServerInfo.s_NtpAddr_1);
            tds__GetNTPResponse->NTPInformation->NTPFromDHCP->IPv6Address    = NULL;
            tds__GetNTPResponse->NTPInformation->NTPFromDHCP->DNSname        = NULL;
            tds__GetNTPResponse->NTPInformation->NTPFromDHCP->Extension      = NULL;

            tds__GetNTPResponse->NTPInformation->__sizeNTPManual             = 0;
            tds__GetNTPResponse->NTPInformation->NTPManual                   = NULL;
        }
        else // NTPManual || Static
        {
            tds__GetNTPResponse->NTPInformation->__sizeNTPManual             = 1;
            tds__GetNTPResponse->NTPInformation->NTPManual                   = ((struct tt__NetworkHost *)soap_malloc(soap_ptr, sizeof(struct tt__NetworkHost)));
            tds__GetNTPResponse->NTPInformation->NTPManual->Type             = tt__NetworkHostType__IPv4;
            tds__GetNTPResponse->NTPInformation->NTPManual->IPv4Address      = (char **)soap_malloc_zero(soap_ptr, sizeof(char *));
            tds__GetNTPResponse->NTPInformation->NTPManual->IPv4Address[0]   = (char *)soap_malloc_zero(soap_ptr, sizeof(char) * NETWORK_INFO_LENGTH);
            strcpy(tds__GetNTPResponse->NTPInformation->NTPManual->IPv4Address[0], SysNtpServerInfo.s_NtpAddr_1);
            tds__GetNTPResponse->NTPInformation->NTPManual->IPv6Address      = NULL;
            tds__GetNTPResponse->NTPInformation->NTPManual->DNSname          = NULL;
            tds__GetNTPResponse->NTPInformation->NTPManual->Extension        = NULL;

            tds__GetNTPResponse->NTPInformation->__sizeNTPFromDHCP           = 0;
            tds__GetNTPResponse->NTPInformation->NTPFromDHCP                 = NULL;

        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNTP(struct soap *soap_ptr, struct _tds__SetNTP *tds__SetNTP, struct _tds__SetNTPResponse *tds__SetNTPResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t SessionId = 0;
    uint32_t AuthValue = 0;
    GMI_RESULT Result = GMI_SUCCESS;

    do
    {
        if (tds__SetNTP->FromDHCP)
        {
            ONVIF_Fault(soap_ptr, "ter:NotSupported", "ter:SetDHCPNotAllowed", "the service is not supported");
            break;
        }

        if (NULL != tds__SetNTP->NTPManual->IPv6Address)
        {
            ONVIF_Fault(soap_ptr, "ter:NotSupported", "ter:IPv6AddressNotAllowed", "the service is not supported");
            break;
        }

        if (NULL != tds__SetNTP->NTPManual->IPv4Address)
        {
            Result = NET_CheckIPString(tds__SetNTP->NTPManual->IPv4Address[0]);
            if (FAILED(Result))
            {
                ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:InvalidIPv4Address", "The argument value is invalid");
                break;
            }

            SysPkgSysTime SysTime;
            SysPkgTimeZone SysTimezone;
            SysPkgDateTimeType SysDateTimeType;
            SysPkgNtpServerInfo SysNtpServerInfo;
            Result = SysGetTime(SessionId, AuthValue, &SysDateTimeType, &SysTime, &SysTimezone, &SysNtpServerInfo);
            if (FAILED(Result))
            {
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetTime fail, Result = 0x%lx\n", Result);
                break;
            }

            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Ntp Address %s\n", tds__SetNTP->NTPManual->IPv4Address[0]);
            memcpy(SysNtpServerInfo.s_NtpAddr_1, tds__SetNTP->NTPManual->IPv4Address[0], sizeof(SysNtpServerInfo.s_NtpAddr_1));
            Result = SysSetTime(SessionId, AuthValue, &SysDateTimeType, &SysTime, &SysTimezone, &SysNtpServerInfo);
            if (FAILED(Result))
            {
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysSetTime fail, Result = 0x%lx\n", Result);
                break;
            }
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDynamicDNS(struct soap *soap_ptr, struct _tds__GetDynamicDNS *tds__GetDynamicDNS, struct _tds__GetDynamicDNSResponse *tds__GetDynamicDNSResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDynamicDNS(struct soap *soap_ptr, struct _tds__SetDynamicDNS *tds__SetDynamicDNS, struct _tds__SetDynamicDNSResponse *tds__SetDynamicDNSResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkInterfaces(struct soap *soap_ptr, struct _tds__GetNetworkInterfaces *tds__GetNetworkInterfaces, struct _tds__GetNetworkInterfacesResponse *tds__GetNetworkInterfacesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint8_t    Id                                 = 0;
    uint8_t    Interfaces                         = 0;
    char_t     IP[NETWORK_INFO_LENGTH]            = {0};
    char_t     InterfaceName[NETWORK_INFO_LENGTH] = {0};
    char_t     InterfaceToken[NETWORK_INFO_LENGTH]= {0};
    boolean_t  Dhcp                               = false;
    char_t     Mac[32]                            = {0};
    GMI_RESULT Result                             = GMI_SUCCESS;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user authen fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        //should get from sys control server
        //get device capabilities and networkinfo
        //get netInterfaceinfo
        Interfaces = DEFAULT_VALID_NETWORK_NUM;
        memcpy(InterfaceName, DEFAULT_NETWORK_NAME, sizeof(InterfaceName));
        memcpy(InterfaceToken, DEFAULT_NETWORK_TOKEN, sizeof(InterfaceToken));

        Result = NET_GetIpInfo(InterfaceName, IP);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "NET_GetIpInfo fail, Result = 0x%lx\n", Result);
            break;
        }

        Result = NET_GetMacInfo(InterfaceName, Mac);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "NET_GetMacInfo fail, Result = 0x%lx\n", Result);
            break;
        }

        tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces = Interfaces;
        tds__GetNetworkInterfacesResponse->NetworkInterfaces = (struct tt__NetworkInterface*)soap_malloc_zero( soap_ptr, tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces*sizeof(struct tt__NetworkInterface) );
        for (Id = 0; Id < tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces; Id++)
        {
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].token  = soap_strdup( soap_ptr, InterfaceToken);
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].Enabled =  xsd__boolean__true_;

            //info
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].Info            = (struct tt__NetworkInterfaceInfo*)soap_malloc_zero(soap_ptr, sizeof(struct tt__NetworkInterfaceInfo));
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].Info->Name      = (char_t*)soap_strdup(soap_ptr, InterfaceName);
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].Info->HwAddress = (char_t*)soap_strdup(soap_ptr, Mac);
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].Info->MTU       = (int*)soap_malloc_zero(soap_ptr, sizeof(int));
            *(tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].Info->MTU )   = 1500;

            //link
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].Link                = (struct tt__NetworkInterfaceLink*)soap_malloc_zero(soap_ptr, sizeof(struct tt__NetworkInterfaceLink));
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].Link->AdminSettings = (struct tt__NetworkInterfaceConnectionSetting*)soap_malloc_zero(soap_ptr, sizeof(struct tt__NetworkInterfaceConnectionSetting));
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].Link->OperSettings  = (struct tt__NetworkInterfaceConnectionSetting*)soap_malloc_zero(soap_ptr, sizeof(struct tt__NetworkInterfaceConnectionSetting));
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].Link->AdminSettings->AutoNegotiation = xsd__boolean__true_;
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].Link->AdminSettings->Speed           = 100;
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].Link->AdminSettings->Duplex          = tt__Duplex__Full;
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].Link->OperSettings->AutoNegotiation  = xsd__boolean__true_;
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].Link->OperSettings->Speed            = 100;
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].Link->OperSettings->Duplex           = tt__Duplex__Full;
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].Link->InterfaceType                  = 0;

            //IPv4
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].IPv4 = ( struct tt__IPv4NetworkInterface* )soap_malloc_zero( soap_ptr, sizeof( struct tt__IPv4NetworkInterface) );
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].IPv4->Enabled = xsd__boolean__true_;
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].IPv4->Config = ( struct tt__IPv4Configuration* )soap_malloc_zero( soap_ptr, sizeof( struct tt__IPv4Configuration) );
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].IPv4->Config->__sizeManual = 1;
            if (Dhcp == false)
            {
                tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].IPv4->Config->Manual = ( struct tt__PrefixedIPv4Address* )soap_malloc_zero( soap_ptr, sizeof( struct tt__PrefixedIPv4Address) );
                tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].IPv4->Config->Manual->Address = soap_strdup(soap_ptr, IP);
                tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].IPv4->Config->Manual->PrefixLength = 24;
                tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].IPv4->Config->DHCP = xsd__boolean__false_;
            }
            else
            {
                tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].IPv4->Config->FromDHCP = ( struct tt__PrefixedIPv4Address* )soap_malloc_zero( soap_ptr, sizeof( struct tt__PrefixedIPv4Address) );
                tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].IPv4->Config->FromDHCP->Address = soap_strdup( soap_ptr, IP);
                tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].IPv4->Config->Manual->PrefixLength = 24;
                tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].IPv4->Config->DHCP = xsd__boolean__true_;
            }

            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].IPv6 = ( struct tt__IPv6NetworkInterface* )soap_malloc_zero( soap_ptr, sizeof( struct tt__IPv6NetworkInterface) );
            tds__GetNetworkInterfacesResponse->NetworkInterfaces[Id].IPv6->Enabled = xsd__boolean__false_;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkInterfaces(struct soap *soap_ptr, struct _tds__SetNetworkInterfaces *tds__SetNetworkInterfaces, struct _tds__SetNetworkInterfacesResponse *tds__SetNetworkInterfacesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t SessionId = 0;
    uint32_t AuthValue = 0;
    char_t      *Ip                                  = NULL;
    char_t       InterfaceToken[NETWORK_INFO_LENGTH] = {0};
    GMI_RESULT   Result                              = GMI_SUCCESS;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user authen fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        //should get form sys control server.
        //get valid network interface info
        memcpy(InterfaceToken, DEFAULT_NETWORK_TOKEN, sizeof(InterfaceToken));

        if (NULL == tds__SetNetworkInterfaces->InterfaceToken)
        {
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:InvalidNetworkInterface", "The argument value is invalid");
            break;
        }

        if (strcmp(tds__SetNetworkInterfaces->InterfaceToken, DEFAULT_NETWORK_TOKEN) != 0)
        {
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:InvalidNetworkInterface", "The argument value is invalid");
            break;
        }

        if (NULL != tds__SetNetworkInterfaces->NetworkInterface->IPv6)
        {
            if (NULL != tds__SetNetworkInterfaces->NetworkInterface->IPv6->Enabled)
            {
                if (*tds__SetNetworkInterfaces->NetworkInterface->IPv6->Enabled)
                {
                    ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:IPv6NotSupported", "the service is not supported");
                    break;
                }
            }
        }

        if (tds__SetNetworkInterfaces->NetworkInterface->IPv4 == NULL)
        {
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:IPv4ValuesMissing", "The argument value is invalid");
            break;
        }

        if (NULL != tds__SetNetworkInterfaces->NetworkInterface->IPv4->Enabled)
        {
            if (tds__SetNetworkInterfaces->NetworkInterface->IPv4->__sizeManual)
            {
                SysPkgIpInfo SysPkgIpInfoTmp                     = {0};

                int32_t PrefixLength = tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual->PrefixLength;
                Ip = tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual->Address;

                ONVIF_INFO("PrefixLength  = %d\n", PrefixLength);
                ONVIF_INFO("IP            = %s\n", Ip);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "PrefixLength  = %d\n", PrefixLength);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "IP            = %s\n", Ip);

                //check IP
                Result = NET_CheckIPString(Ip);
                if (FAILED(Result))
                {
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "IP invalid\n");
                    ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:InvalidIPv4Address", "The argument value is invalid");
                    break;
                }

                Result = SysGetDeviceIP(SessionId, AuthValue, &SysPkgIpInfoTmp );
                if (FAILED(Result))
                {
                    ONVIF_ERROR("SysGetDeviceIP fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetDeviceIP fail, Result = 0x%lx\n", Result);
                    break;
                }

                memcpy(SysPkgIpInfoTmp.s_IpAddress, Ip, sizeof(SysPkgIpInfoTmp.s_IpAddress));
                Result = SysSetDeviceIP(SessionId, AuthValue,  &SysPkgIpInfoTmp);
                if (FAILED(Result))
                {
                    ONVIF_ERROR("SysSetDeviceIP fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysSetDeviceIP fail, Result = 0x%lx\n", Result);
                    break;
                }
            }
            else
            {
                if (tds__SetNetworkInterfaces->NetworkInterface->IPv4->DHCP != NULL)
                {
                    if (*(tds__SetNetworkInterfaces->NetworkInterface->IPv4->DHCP))
                    {
                        //
                    }
                    else
                    {
                        //
                    }
                }
            }
        }

        tds__SetNetworkInterfacesResponse->RebootNeeded = xsd__boolean__false_;
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkProtocols(struct soap *soap_ptr, struct _tds__GetNetworkProtocols *tds__GetNetworkProtocols, struct _tds__GetNetworkProtocolsResponse *tds__GetNetworkProtocolsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t SessionId = 0;
    uint32_t AuthValue = 0;
    int32_t           Id;
    GMI_RESULT        Result = GMI_SUCCESS;
    NetworkProtocols *NetworkProtocolsPtr = NULL;
    SysPkgNetworkPort SysNetworkPort;

    //should get from system control server
    do
    {
        memset(&SysNetworkPort, 0, sizeof(SysPkgNetworkPort));
        Result = SysGetNetworkPort(SessionId, AuthValue, &SysNetworkPort);
        if (FAILED(Result))
        {
            ONVIF_ERROR("SysGetNetworkPort fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetNetworkPort fail, Result = 0x%lx\n", Result);
            break;
        }

        tds__GetNetworkProtocolsResponse->__sizeNetworkProtocols = DEFAULT_NETWORK_PROTOCOLS;
        NetworkProtocolsPtr = l_NetworkProtocols;

        tds__GetNetworkProtocolsResponse->NetworkProtocols       = (struct tt__NetworkProtocol*)soap_malloc_zero(soap_ptr, sizeof(struct tt__NetworkProtocol)*tds__GetNetworkProtocolsResponse->__sizeNetworkProtocols);
        if (NULL == tds__GetNetworkProtocolsResponse->NetworkProtocols)
        {
            ONVIF_ERROR("malloc NetworkProtocols failed\n");
            break;
        }

        NetworkProtocolsPtr[0].s_Port = SysNetworkPort.s_HTTP_Port ;
        NetworkProtocolsPtr[1].s_Port = SysNetworkPort.s_RTSP_Port;

        for (Id = 0; Id < tds__GetNetworkProtocolsResponse->__sizeNetworkProtocols; Id++)
        {
            tds__GetNetworkProtocolsResponse->NetworkProtocols[Id].Name       = NetworkProtocolsPtr[Id].s_Name;
            tds__GetNetworkProtocolsResponse->NetworkProtocols[Id].Enabled    = NetworkProtocolsPtr[Id].s_Enabled;
            tds__GetNetworkProtocolsResponse->NetworkProtocols[Id].__sizePort = 1;
            tds__GetNetworkProtocolsResponse->NetworkProtocols[Id].Port       = (int*)soap_malloc_zero(soap_ptr, sizeof(int)*tds__GetNetworkProtocolsResponse->NetworkProtocols[Id].__sizePort);
            *(tds__GetNetworkProtocolsResponse->NetworkProtocols[Id].Port)    = NetworkProtocolsPtr[Id].s_Port;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkProtocols(struct soap *soap_ptr, struct _tds__SetNetworkProtocols *tds__SetNetworkProtocols, struct _tds__SetNetworkProtocolsResponse *tds__SetNetworkProtocolsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t           SessionId = 0;
    uint32_t           AuthValue = 0;
    int32_t            Id;
    int32_t            NetWorkProtocols    = DEFAULT_NETWORK_PROTOCOLS;
    GMI_RESULT         Result              = GMI_SUCCESS;
    NetworkProtocols  *NetworkProtocolsPtr = NULL;
    SysPkgNetworkPort  SysNetworkPort;

    do
    {
        if (tds__SetNetworkProtocols->NetworkProtocols->Name == tt__NetworkProtocolType__HTTPS)
        {
            ONVIF_ERROR("unsupport tt__NetworkProtocolType__HTTPS\n");
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:ServiceNotSupported", "the service is not supported");
            break;
        }

        NetworkProtocolsPtr = l_NetworkProtocols;
        for (Id = 0; Id < NetWorkProtocols; Id++)
        {
            if (tds__SetNetworkProtocols->NetworkProtocols->Name == NetworkProtocolsPtr[Id].s_Name)
            {
                NetworkProtocolsPtr[Id].s_Enabled   = tds__SetNetworkProtocols->NetworkProtocols->Enabled;
                NetworkProtocolsPtr[Id].s_Port      = (int32_t)*(tds__SetNetworkProtocols->NetworkProtocols->Port);
            }
        }

        memset(&SysNetworkPort, 0, sizeof(SysPkgNetworkPort));
        Result = SysGetNetworkPort(SessionId, AuthValue, &SysNetworkPort);
        if (FAILED(Result))
        {
            ONVIF_ERROR("SysGetNetworkPort fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetNetworkPort fail, Result = 0x%lx\n", Result);
            break;
        }

        SysNetworkPort.s_HTTP_Port = NetworkProtocolsPtr[0].s_Port;
        SysNetworkPort.s_RTSP_Port = NetworkProtocolsPtr[1].s_Port;

        Result = SysSetNetworkPort(SessionId, AuthValue, &SysNetworkPort);
        if (FAILED(Result))
        {
            ONVIF_ERROR("SysSetNetworkPort fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysSetNetworkPort fail, Result = 0x%lx\n", Result);
            break;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkDefaultGateway(struct soap *soap_ptr, struct _tds__GetNetworkDefaultGateway *tds__GetNetworkDefaultGateway, struct _tds__GetNetworkDefaultGatewayResponse *tds__GetNetworkDefaultGatewayResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t      SessionId = 0;
    uint32_t      AuthValue = 0;
    GMI_RESULT    Result      = GMI_SUCCESS;
    char_t       *GateWayPtr  = NULL;
    int32_t      *PtrTmp      = NULL;
    SysPkgIpInfo SysIpInfo   = {0};

    do
    {
        Result = SysGetDeviceIP(SessionId, AuthValue, &SysIpInfo);
        if (FAILED(Result))
        {
            ONVIF_ERROR("SysGetDeviceIP failed\n");
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetDeviceIP fail, Result = 0x%lx\n", Result);
            break;
        }

        tds__GetNetworkDefaultGatewayResponse->NetworkGateway = (struct tt__NetworkGateway*)soap_malloc_zero(soap_ptr, sizeof(struct tt__NetworkGateway));
        tds__GetNetworkDefaultGatewayResponse->NetworkGateway->__sizeIPv4Address = 1;
        PtrTmp     = (int32_t*)soap_malloc_zero(soap_ptr, sizeof(int32_t*));
        GateWayPtr = (char_t*)soap_malloc_zero(soap_ptr, NETWORK_INFO_LENGTH);
        *PtrTmp    = (int32_t)GateWayPtr;
        strcpy(GateWayPtr, SysIpInfo.s_GateWay);
        tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address = (char_t**)PtrTmp;
        ONVIF_INFO("IPv4Address %s\n", *tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address );
        ONVIF_INFO("%s normal exit.........\n", __func__);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkDefaultGateway(struct soap *soap_ptr, struct _tds__SetNetworkDefaultGateway *tds__SetNetworkDefaultGateway, struct _tds__SetNetworkDefaultGatewayResponse *tds__SetNetworkDefaultGatewayResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t      SessionId = 0;
    uint32_t      AuthValue = 0;
    GMI_RESULT    Result      = GMI_SUCCESS;

    do
    {
        if (tds__SetNetworkDefaultGateway->__sizeIPv6Address > 0)
        {
            ONVIF_Fault(soap_ptr, "ter:NotSupported", "ter:InvalidIPv6Address", "the service is not supported");
            break;
        }

        if (0 == tds__SetNetworkDefaultGateway->__sizeIPv4Address)
        {
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:InvalidGatewayAddress", "The argument value is invalid");
            break;
        }

        Result = NET_CheckIPString(*tds__SetNetworkDefaultGateway->IPv4Address);
        if (FAILED(Result))
        {
            ONVIF_ERROR("gateway invalid\n");
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:InvalidGatewayAddress", "The argument value is invalid");
            break;
        }

        ONVIF_INFO("IPv4Address %s\n", *tds__SetNetworkDefaultGateway->IPv4Address);

        SysPkgIpInfo SysIpInfo;

        Result = SysGetDeviceIP(SessionId, AuthValue, &SysIpInfo);
        if (FAILED(Result))
        {
            ONVIF_ERROR("SysGetDeviceIP failed\n");
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetDeviceIP fail, Result = 0x%lx\n", Result);
            break;
        }

        memcpy(SysIpInfo.s_GateWay, *tds__SetNetworkDefaultGateway->IPv4Address, NETWORK_INFO_LENGTH);
        Result = SysSetDeviceIP(SessionId, AuthValue, &SysIpInfo);
        if (FAILED(Result))
        {
            ONVIF_ERROR("SysSetDeviceIP fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysSetDeviceIP fail, Result = 0x%lx\n", Result);
            break;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetZeroConfiguration(struct soap *soap_ptr, struct _tds__GetZeroConfiguration *tds__GetZeroConfiguration, struct _tds__GetZeroConfigurationResponse *tds__GetZeroConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    tds__GetZeroConfigurationResponse->ZeroConfiguration = (struct tt__NetworkZeroConfiguration*)soap_malloc_zero(soap_ptr, sizeof(struct tt__NetworkZeroConfiguration));
    tds__GetZeroConfigurationResponse->ZeroConfiguration->InterfaceToken = soap_strdup(soap_ptr, DEFAULT_NETWORK_TOKEN);
    tds__GetZeroConfigurationResponse->ZeroConfiguration->Enabled = xsd__boolean__true_;
    tds__GetZeroConfigurationResponse->ZeroConfiguration->__sizeAddresses = 2;
    tds__GetZeroConfigurationResponse->ZeroConfiguration->Addresses = (char **)soap_malloc_zero(soap_ptr, sizeof(char *)*tds__GetZeroConfigurationResponse->ZeroConfiguration->__sizeAddresses);

    tds__GetZeroConfigurationResponse->ZeroConfiguration->Addresses[0]= soap_strdup(soap_ptr, "169.254.10.200");
    tds__GetZeroConfigurationResponse->ZeroConfiguration->Addresses[1]= soap_strdup(soap_ptr, "169.254.10.100");
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetZeroConfiguration(struct soap *soap_ptr, struct _tds__SetZeroConfiguration *tds__SetZeroConfiguration, struct _tds__SetZeroConfigurationResponse *tds__SetZeroConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetIPAddressFilter(struct soap *soap_ptr, struct _tds__GetIPAddressFilter *tds__GetIPAddressFilter, struct _tds__GetIPAddressFilterResponse *tds__GetIPAddressFilterResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetIPAddressFilter(struct soap *soap_ptr, struct _tds__SetIPAddressFilter *tds__SetIPAddressFilter, struct _tds__SetIPAddressFilterResponse *tds__SetIPAddressFilterResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__AddIPAddressFilter(struct soap *soap_ptr, struct _tds__AddIPAddressFilter *tds__AddIPAddressFilter, struct _tds__AddIPAddressFilterResponse *tds__AddIPAddressFilterResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveIPAddressFilter(struct soap *soap_ptr, struct _tds__RemoveIPAddressFilter *tds__RemoveIPAddressFilter, struct _tds__RemoveIPAddressFilterResponse *tds__RemoveIPAddressFilterResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetAccessPolicy(struct soap *soap_ptr, struct _tds__GetAccessPolicy *tds__GetAccessPolicy, struct _tds__GetAccessPolicyResponse *tds__GetAccessPolicyResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetAccessPolicy(struct soap *soap_ptr, struct _tds__SetAccessPolicy *tds__SetAccessPolicy, struct _tds__SetAccessPolicyResponse *tds__SetAccessPolicyResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateCertificate(struct soap *soap_ptr, struct _tds__CreateCertificate *tds__CreateCertificate, struct _tds__CreateCertificateResponse *tds__CreateCertificateResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificates(struct soap *soap_ptr, struct _tds__GetCertificates *tds__GetCertificates, struct _tds__GetCertificatesResponse *tds__GetCertificatesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificatesStatus(struct soap *soap_ptr, struct _tds__GetCertificatesStatus *tds__GetCertificatesStatus, struct _tds__GetCertificatesStatusResponse *tds__GetCertificatesStatusResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetCertificatesStatus(struct soap *soap_ptr, struct _tds__SetCertificatesStatus *tds__SetCertificatesStatus, struct _tds__SetCertificatesStatusResponse *tds__SetCertificatesStatusResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteCertificates(struct soap *soap_ptr, struct _tds__DeleteCertificates *tds__DeleteCertificates, struct _tds__DeleteCertificatesResponse *tds__DeleteCertificatesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetPkcs10Request(struct soap *soap_ptr, struct _tds__GetPkcs10Request *tds__GetPkcs10Request, struct _tds__GetPkcs10RequestResponse *tds__GetPkcs10RequestResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificates(struct soap *soap_ptr, struct _tds__LoadCertificates *tds__LoadCertificates, struct _tds__LoadCertificatesResponse *tds__LoadCertificatesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetClientCertificateMode(struct soap *soap_ptr, struct _tds__GetClientCertificateMode *tds__GetClientCertificateMode, struct _tds__GetClientCertificateModeResponse *tds__GetClientCertificateModeResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetClientCertificateMode(struct soap *soap_ptr, struct _tds__SetClientCertificateMode *tds__SetClientCertificateMode, struct _tds__SetClientCertificateModeResponse *tds__SetClientCertificateModeResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRelayOutputs(struct soap *soap_ptr, struct _tds__GetRelayOutputs *tds__GetRelayOutputs, struct _tds__GetRelayOutputsResponse *tds__GetRelayOutputsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRelayOutputSettings(struct soap *soap_ptr, struct _tds__SetRelayOutputSettings *tds__SetRelayOutputSettings, struct _tds__SetRelayOutputSettingsResponse *tds__SetRelayOutputSettingsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRelayOutputState(struct soap *soap_ptr, struct _tds__SetRelayOutputState *tds__SetRelayOutputState, struct _tds__SetRelayOutputStateResponse *tds__SetRelayOutputStateResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SendAuxiliaryCommand(struct soap *soap_ptr, struct _tds__SendAuxiliaryCommand *tds__SendAuxiliaryCommand, struct _tds__SendAuxiliaryCommandResponse *tds__SendAuxiliaryCommandResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCACertificates(struct soap *soap_ptr, struct _tds__GetCACertificates *tds__GetCACertificates, struct _tds__GetCACertificatesResponse *tds__GetCACertificatesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificateWithPrivateKey(struct soap *soap_ptr, struct _tds__LoadCertificateWithPrivateKey *tds__LoadCertificateWithPrivateKey, struct _tds__LoadCertificateWithPrivateKeyResponse *tds__LoadCertificateWithPrivateKeyResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificateInformation(struct soap *soap_ptr, struct _tds__GetCertificateInformation *tds__GetCertificateInformation, struct _tds__GetCertificateInformationResponse *tds__GetCertificateInformationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCACertificates(struct soap *soap_ptr, struct _tds__LoadCACertificates *tds__LoadCACertificates, struct _tds__LoadCACertificatesResponse *tds__LoadCACertificatesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateDot1XConfiguration(struct soap *soap_ptr, struct _tds__CreateDot1XConfiguration *tds__CreateDot1XConfiguration, struct _tds__CreateDot1XConfigurationResponse *tds__CreateDot1XConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDot1XConfiguration(struct soap *soap_ptr, struct _tds__SetDot1XConfiguration *tds__SetDot1XConfiguration, struct _tds__SetDot1XConfigurationResponse *tds__SetDot1XConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot1XConfiguration(struct soap *soap_ptr, struct _tds__GetDot1XConfiguration *tds__GetDot1XConfiguration, struct _tds__GetDot1XConfigurationResponse *tds__GetDot1XConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot1XConfigurations(struct soap *soap_ptr, struct _tds__GetDot1XConfigurations *tds__GetDot1XConfigurations, struct _tds__GetDot1XConfigurationsResponse *tds__GetDot1XConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteDot1XConfiguration(struct soap *soap_ptr, struct _tds__DeleteDot1XConfiguration *tds__DeleteDot1XConfiguration, struct _tds__DeleteDot1XConfigurationResponse *tds__DeleteDot1XConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Capabilities(struct soap *soap_ptr, struct _tds__GetDot11Capabilities *tds__GetDot11Capabilities, struct _tds__GetDot11CapabilitiesResponse *tds__GetDot11CapabilitiesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Status(struct soap *soap_ptr, struct _tds__GetDot11Status *tds__GetDot11Status, struct _tds__GetDot11StatusResponse *tds__GetDot11StatusResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__ScanAvailableDot11Networks(struct soap *soap_ptr, struct _tds__ScanAvailableDot11Networks *tds__ScanAvailableDot11Networks, struct _tds__ScanAvailableDot11NetworksResponse *tds__ScanAvailableDot11NetworksResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemUris(struct soap *soap_ptr, struct _tds__GetSystemUris *tds__GetSystemUris, struct _tds__GetSystemUrisResponse *tds__GetSystemUrisResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__StartFirmwareUpgrade(struct soap *soap_ptr, struct _tds__StartFirmwareUpgrade *tds__StartFirmwareUpgrade, struct _tds__StartFirmwareUpgradeResponse *tds__StartFirmwareUpgradeResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__StartSystemRestore(struct soap *soap_ptr, struct _tds__StartSystemRestore *tds__StartSystemRestore, struct _tds__StartSystemRestoreResponse *tds__StartSystemRestoreResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}



