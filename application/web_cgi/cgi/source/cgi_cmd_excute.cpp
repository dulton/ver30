#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "cgi_cmd_entry.h"
#include "cgi_cmd_excute.h"
#include "utilitly.h"
#include "auth_center_api.h"
#include "sys_env_types.h"
#include "gmi_system_headers.h"
#include "ipc_fw_v3.x_resource.h"
#include "gmi_daemon_heartbeat_api.h"
#include "sys_client.h"
#include "sys_info_readonly.h"
#include "log.h"

//#define DEBUG_TEST
#define ID_MOUDLE_REST_WEB			0x4   //WEB
#define SYSTEM_MAX_USER_COUNT          32


//SessionId Check
GMI_RESULT CgiCheckSessionId(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *RetFormat;
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    do
    {
        if  (NULL == SessionId)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        Result = GMI_SysCheckSessionId(atoi(SessionId));
        if (FAILED(Result))
        {
            RetCode = Result;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);

        return GMI_SUCCESS;
    } while(0);

    RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
    return GMI_FAIL;
}

//LogIn System
GMI_RESULT CgiLogIn(const char_t *FncCmd)
{
    char_t *UserName = WEB_GET_VAR("UserName");
    char_t *Password = WEB_GET_VAR("Password");
    char_t *InSessionId = WEB_GET_VAR("SessionId");
    const char_t *RetFormat;
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;
    DaemonData_t DaemonData;

    CGI_ERROR("GMI_DaemonInit start[%d] \n",__LINE__);

    Result = GMI_DaemonInit( &DaemonData, WEB_SERVER_ID, GMI_DAEMON_HEARDBEAT_SERVER, GMI_DAEMON_HEARDBEAT_WEB);
    if (FAILED(Result))
    {
        RetCode = RETCODE_SYSTEM_RNNING;
        CGI_ERROR("Call SysAuthLogin Error Result = %ld\n",Result);
        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        GMI_DaemonUnInit(&DaemonData);
        return GMI_FAIL;
    }

    do
    {
        if  (NULL == UserName
                || NULL == Password
                || NULL == InSessionId)
        {
            RetCode = RETCODE_ERROR;
            CGI_ERROR("CgiLogIn  Error \n");
            break;
        }

        CGI_ERROR("GMI_InquiryServerStatus Start [%d] \n",__LINE__);

        uint16_t Status = 0;
        Result = GMI_InquiryServerStatus(&DaemonData, GMI_DAEMON_HEARTBEAT_STATUS_QUERY,GMI_DAEMON_APPLICATION_STATUS_QUIRY, SDK_SERVER_ID, &Status);
        if (FAILED(Result))
        {
            RetCode = RETCODE_SYSTEM_RNNING;
            CGI_ERROR("Call SysAuthLogin Error Result = %ld\n",Result);
            break;
        }

        CGI_ERROR("GMI_DaemonInit [%d] Status = %d\n",__LINE__, Status);


        if (1 == Status)
        {
            uint32_t Authvalue = 0;
            uint16_t OutSessionId = 0;
            uint8_t UserFlag =0;
            uint16_t SdkPort = 30000;
#if 0
            Result = SysAuthLogin(UserName, Password, atoi(InSessionId), ID_MOUDLE_REST_WEB, &OutSessionId, &UserFlag, &Authvalue);
            if (FAILED(Result))
            {
                RetCode = RETCODE_ERROR;
                CGI_ERROR("Call SysAuthLogin Error Result = %ld\n",Result);
                break;
            }

#endif
            Result = SysAuthLoginExt(UserName, Password, atoi(InSessionId), ID_MOUDLE_REST_WEB, &OutSessionId, &UserFlag, &Authvalue, &SdkPort);
	    if (FAILED(Result))
	    {
		RetCode = RETCODE_ERROR;
		CGI_ERROR("Call SysAuthLogin Error Result = %ld\n",Result);
		break;
	    }

            RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\",\"SessionId\":\"%d\",\"UserFlag\":\"%d\",\"Authvalue\":\"%u\",\"SdkPort\":\"%u\"}";
            fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode, OutSessionId, UserFlag, Authvalue, SdkPort);

            GMI_DaemonUnInit(&DaemonData);
            return GMI_SUCCESS;
        }
        else
        {
            RetCode = RETCODE_SYSTEM_RNNING;
            CGI_ERROR("Call SysAuthLogin Error Result = %ld\n",Result);
            break;
        }

    } while(0);

    RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
    GMI_DaemonUnInit(&DaemonData);

    return GMI_FAIL;
}

//LogOut system
GMI_RESULT CgiLogOut(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *RetFormat;
    int32_t RetCode = RETCODE_OK;
//    GMI_RESULT Result = GMI_FAIL;

    do
    {
        if (NULL == SessionId)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

#if 0
        Result =  SysAuthLogout(atoi(SessionId));
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }
#endif

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
    return GMI_FAIL;
}

//Getting System IP Address
GMI_RESULT CgiGetIpInfo(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
//    char_t *InterfName = "eth0";
//   char_t Ipv4Address[MIN_BUFFER_LENTH];
    //  char_t HwAddress[MIN_BUFFER_LENTH];
    int32_t RetCode   = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }


#ifdef  DEBUG_TEST

        Result = NET_GetIpInfo(InterfName,Ipv4Address);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        Result = NET_GetMacInfo(InterfName,HwAddress);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }
#else
        SysPkgIpInfo SysIpInfo;
        memset(&SysIpInfo, 0,sizeof(SysPkgIpInfo));
        Result = SysGetDeviceIP(atoi(SessionId), atoi(AuthValue), &SysIpInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }
#endif

        RetFormat = "%s={\"RetCode\":\"%d\",\"SessionId\":\"%d\",\"NetId\":\"%d\",\"InterfName\":\"%s\",\"IpAddress\":\"%s\""
                    ",\"SubNetMask\":\"%s\",\"GateWay\":\"%s\",\"Dns\":\"%s\",\"HwAddress\":\"%s\",\"Dhcp\":\"%d\"}";
        fprintf(stdout, RetFormat, Cmd, RetCode, *SessionId,\
                SysIpInfo.s_NetId, \
                SysIpInfo.s_InterfName,\
                SysIpInfo.s_IpAddress,\
                SysIpInfo.s_SubNetMask,\
                SysIpInfo.s_GateWay,\
                SysIpInfo.s_Dns,\
                SysIpInfo.s_HwAddress,\
                SysIpInfo.s_Dhcp
               );

        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

//Setting system IP Address
GMI_RESULT CgiSetIpInfo(const char_t *FncCmd)
{
    const char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *NetId = WEB_GET_VAR("NetId");
    const char_t *InterfName = WEB_GET_VAR("InterfName");
    const char_t *IpAddress = WEB_GET_VAR("IpAddress");
    const char_t *SubNetMask = WEB_GET_VAR("SubNetMask");
    const char_t *GateWay = WEB_GET_VAR("GateWay");
    const char_t *Dns = WEB_GET_VAR("Dns");
    const char_t *HwAddress = WEB_GET_VAR("HwAddress");
    const char_t *Dhcp = WEB_GET_VAR("Dhcp");
    const char_t *RetFormat;

    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode	 = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId  || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgIpInfo SysIpInfo;
        memset(&SysIpInfo, 0, sizeof(SysPkgIpInfo));
        //Call get SysIpInfo api
        Result = SysGetDeviceIP(atoi(SessionId), atoi(AuthValue), &SysIpInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        if ( NULL != NetId)
            SysIpInfo.s_NetId = atoi(NetId);
        if (NULL != InterfName)
            strcpy(SysIpInfo.s_InterfName, InterfName);
        if (NULL != IpAddress)
            strcpy(SysIpInfo.s_IpAddress, IpAddress);
        if (NULL != SubNetMask)
            strcpy(SysIpInfo.s_SubNetMask, SubNetMask);
        if (NULL !=  GateWay)
            strcpy(SysIpInfo.s_GateWay, GateWay);
        if (NULL != Dns)
            strcpy(SysIpInfo.s_Dns, Dns);
        if (NULL != HwAddress)
            strcpy(SysIpInfo.s_HwAddress , HwAddress);
        if (NULL != Dhcp)
            SysIpInfo.s_Dhcp = atoi(Dhcp);
        //Call set Ip api
        Result = SysSetDeviceIP(atoi(SessionId), atoi(AuthValue), &SysIpInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

//system network port information setting and getting
GMI_RESULT CgiGetNetworkPort(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    GMI_RESULT Result = GMI_FAIL;

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

#ifdef DEBUG_TEST
        SysPkgNetworkPort SysNetworkPort;
        memset(&SysNetworkPort, 0, sizeof(SysPkgNetworkPort));
        SysNetworkPort.s_HTTP_Port = 80;
        SysNetworkPort.s_RTSP_Port = 554;
        SysNetworkPort.s_SDK_Port = 30000;
#else
        SysPkgNetworkPort SysNetworkPort;
        memset(&SysNetworkPort, 0, sizeof(SysPkgNetworkPort));
        //Call API get SysPkgNetworkPort
        Result = SysGetNetworkPort(atoi(SessionId), atoi(AuthValue), &SysNetworkPort);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

#endif

        RetFormat = "%s={\"RetCode\":\"%d\",\"SessionId\":\"%d\",\"HTTP_Port\":\"%d\",\"RTSP_Port\":\"%d\",\"SDK_Port\":\"%d\",\"Upgrade_Port\":\"%d\"}";
        fprintf(stdout, RetFormat, Cmd, RetCode, *SessionId,\
                SysNetworkPort.s_HTTP_Port, \
                SysNetworkPort.s_RTSP_Port,\
                SysNetworkPort.s_SDK_Port, \
                SysNetworkPort.s_Upgrade_Port
               );
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

GMI_RESULT CgiSetNetworkPort(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *HTTP_Port = WEB_GET_VAR("HTTP_Port");
    const char_t *RTSP_Port = WEB_GET_VAR("RTSP_Port");
    const char_t *SDK_Port = WEB_GET_VAR("SDK_Port");
    const char_t *Upgrade_Port = WEB_GET_VAR("Upgrade_Port");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    GMI_RESULT Result = GMI_FAIL;

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

#ifndef DEBUG_TEST
        SysPkgNetworkPort SysNetworkPort;
        memset(&SysNetworkPort, 0, sizeof(SysPkgNetworkPort));
        //Call get NetWork
        Result = SysGetNetworkPort(atoi(SessionId), atoi(AuthValue), &SysNetworkPort);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        if (NULL != HTTP_Port)
            SysNetworkPort.s_HTTP_Port = atoi(HTTP_Port);
        if (NULL != RTSP_Port)
            SysNetworkPort.s_RTSP_Port = atoi(RTSP_Port);
        if (NULL != SDK_Port)
            SysNetworkPort.s_SDK_Port = atoi(SDK_Port);
        if (NULL != Upgrade_Port)
            SysNetworkPort.s_Upgrade_Port = atoi(Upgrade_Port);

        //Call network port setting api
        Result = SysSetNetworkPort(atoi(SessionId), atoi(AuthValue), &SysNetworkPort);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

#endif

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

//system device information
GMI_RESULT CgiGetSysDeviceInfo(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

#ifdef DEBUG_TEST
        SysPkgDeviceInfo SysDeviceInfo;
        memset(&SysDeviceInfo, 0, sizeof(SysPkgDeviceInfo));
        strcpy(SysDeviceInfo.s_DeviceName, "IPCamera");
        SysDeviceInfo.s_DeviceId = 1;
        strcpy(SysDeviceInfo.s_DeviceModel, "iHDNC");
        strcpy(SysDeviceInfo.s_DeviceManufactuer, "GMI");
        strcpy(SysDeviceInfo.s_DeviceSerialNum, "123456");
        strcpy(SysDeviceInfo.s_DeviceFwVer, "0.0.160007.20130822001");
        strcpy(SysDeviceInfo.s_DeviceHwVer, "0.0");
#else
        SysPkgDeviceInfo SysDeviceInfo;
        memset(&SysDeviceInfo, 0, sizeof(SysPkgDeviceInfo));
        //Call get System DeviceInfo api
        Result = SysGetDeviceInfo(atoi(SessionId), atoi(AuthValue), &SysDeviceInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }
#endif

        RetFormat = "%s={\"RetCode\":\"%d\",\"SessionId\":\"%d\",\"DeviceName\":\"%s\",\"DeviceId\":\"%d\",\"DeviceModel\":\"%s\""
                    ",\"DeviceManufactuer\":\"%s\",\"DeviceSerialNum\":\"%s\",\"DeviceFwVer\":\"%s\",\"DeviceHwVer\":\"%s\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, *SessionId,\
                SysDeviceInfo.s_DeviceName, \
                SysDeviceInfo.s_DeviceId,\
                SysDeviceInfo.s_DeviceModel,\
                SysDeviceInfo.s_DeviceManufactuer,\
                SysDeviceInfo.s_DeviceSerialNum,\
                SysDeviceInfo.s_DeviceFwVer,\
                SysDeviceInfo.s_DeviceHwVer
               );
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":%d}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

GMI_RESULT CgiSetSysDeviceInfo(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *DeviceName = WEB_GET_VAR("DeviceName");
    const char_t *DeviceId = WEB_GET_VAR("DeviceId");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

#ifndef DEBUG_TEST
        SysPkgDeviceInfo SysDeviceInfo;
        memset(&SysDeviceInfo, 0, sizeof(SysPkgDeviceInfo));
        //Call get System Device info api
        Result = SysGetDeviceInfo(atoi(SessionId), atoi(AuthValue), &SysDeviceInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        if (NULL != DeviceName)
            strcpy(SysDeviceInfo.s_DeviceName, DeviceName);
        if(NULL != DeviceId)
            SysDeviceInfo.s_DeviceId = atoi(DeviceId);
        //Call set System Device info api
        Result = SysSetDeviceInfo(atoi(SessionId), atoi(AuthValue), &SysDeviceInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }
#endif

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

//system NTP server information setting and getting
GMI_RESULT CgiGetNtpServerInfo(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if ( NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgNtpServerInfo SysNtpServerInfo;
        memset(&SysNtpServerInfo, 0 ,sizeof(SysPkgNtpServerInfo));

        //Call NTP server get
        Result = SysGetNtpServer(atoi(SessionId), atoi(AuthValue), &SysNtpServerInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"SessionId\":\"%d\",\"NtpAddr_1\":\"%s\",\"NtpAddr_2\":\"%s\",\"NtpAddr_3\":\"%s\"}";
        fprintf(stdout, RetFormat, Cmd, RetCode, *SessionId,\
                SysNtpServerInfo.s_NtpAddr_1, \
                SysNtpServerInfo.s_NtpAddr_2,\
                SysNtpServerInfo.s_NtpAddr_3
               );
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiSetNtpServerInfo(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *NtpAddr_1 = WEB_GET_VAR("NtpAddr_1");
    const char_t *NtpAddr_2 = WEB_GET_VAR("NtpAddr_2");
    const char_t *NtpAddr_3 = WEB_GET_VAR("NtpAddr_3");
    const char_t *Type = WEB_GET_VAR("Type");
    const char_t *NtpInterval = WEB_GET_VAR("NtpInterval");

    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    GMI_RESULT Result = GMI_FAIL;

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgDateTimeType  SysDateTimeType;
        memset(&SysDateTimeType, 0, sizeof(SysPkgDateTimeType));

        SysPkgSysTime SysSysTime;
        memset(&SysSysTime, 0, sizeof(SysPkgSysTime));

        SysPkgTimeZone SysTimeZone;
        memset(&SysTimeZone, 0, sizeof(SysPkgTimeZone));

        SysPkgNtpServerInfo SysNtpServerInfo;
        memset(&SysNtpServerInfo, 0, sizeof(SysPkgNtpServerInfo));

        //Call Get system Time api
        Result = SysGetTime(atoi(SessionId), atoi(AuthValue), &SysDateTimeType, &SysSysTime, &SysTimeZone, &SysNtpServerInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        if (NULL != NtpAddr_1)
            strcpy(SysNtpServerInfo.s_NtpAddr_1, NtpAddr_1);
        if (NULL != NtpAddr_2)
            strcpy(SysNtpServerInfo.s_NtpAddr_2, NtpAddr_2);
        if (NULL != NtpAddr_3)
            strcpy(SysNtpServerInfo.s_NtpAddr_3, NtpAddr_3);
        if (NULL != Type)
            SysDateTimeType.s_Type = atoi(Type);
        if (NULL != NtpInterval)
            SysDateTimeType.s_NtpInterval = atoi(NtpInterval);

        //Call api set NtpserverInfo
        Result = SysSetTime(atoi(SessionId), atoi(AuthValue), &SysDateTimeType, &SysSysTime, &SysTimeZone, &SysNtpServerInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}
//system Ftp server infomation setting and getting
GMI_RESULT CgiGetFtpServerInfo(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
//    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgFtpServerInfo SysFtpServerInfo;
        memset(&SysFtpServerInfo, 0, sizeof(SysPkgFtpServerInfo));
        //call get FTP Server

        RetFormat = "%s={\"RetCode\":\"%d\",\"SessionId\":\"%d\",\"ServerIp\":\"%s\",\"UserName\":\"%s\",\"PassWord\":\"%s\",\"FolderName\":\"%s\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, *SessionId,\
                SysFtpServerInfo.s_ServerIp, \
                SysFtpServerInfo.s_UserName,\
                SysFtpServerInfo.s_PassWord,\
                SysFtpServerInfo.s_FolderName
               );
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}
GMI_RESULT CgiSetFtpServerInfo(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *ServerIp = WEB_GET_VAR("ServerIp");
    const char_t *UserName = WEB_GET_VAR("UserName");
    const char_t *PassWord = WEB_GET_VAR("PassWord");
    const char_t *FolderName= WEB_GET_VAR("FolderName");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    // GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    do
    {
        if ( NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }
        SysPkgFtpServerInfo SysFtpServerInfo;
        memset(&SysFtpServerInfo, 0, sizeof(SysPkgFtpServerInfo));
        //Call get FtpServer

        if (NULL != ServerIp)
            strcpy(SysFtpServerInfo.s_ServerIp, ServerIp);
        if (NULL != UserName)
            strcpy(SysFtpServerInfo.s_UserName, UserName);
        if (NULL != PassWord)
            strcpy(SysFtpServerInfo.s_PassWord, PassWord);
        if (NULL != FolderName)
            strcpy(SysFtpServerInfo.s_FolderName, FolderName);

        //Call Set FtpServer Info


        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

//system time setting and getting
GMI_RESULT CgiGetTime(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

#ifdef DEBUG_TEST
        SysPkgDateTimeType	SysDateTimeType;
        memset(&SysDateTimeType, 0, sizeof(SysPkgDateTimeType));
        SysDateTimeType.s_Type = 0;
        SysDateTimeType.s_NtpInterval = 0;

        SysPkgSysTime SysSysTime;
        memset(&SysSysTime, 0, sizeof(SysPkgSysTime));
        SysSysTime.s_Year = 2013;
        SysSysTime.s_Month = 8;
        SysSysTime.s_Day = 29;
        SysSysTime.s_Hour = 18;
        SysSysTime.s_Minute = 33;
        SysSysTime.s_Second = 23;

        SysPkgTimeZone SysTimeZone;
        memset(&SysTimeZone, 0, sizeof(SysPkgTimeZone));
        strcpy(SysTimeZone.s_TimzeZoneName, "CST+08:00:00");

        SysPkgNtpServerInfo SysNtpServerInfo;
        memset(&SysNtpServerInfo, 0, sizeof(SysPkgNtpServerInfo));
        strcpy(SysNtpServerInfo.s_NtpAddr_1, "0.0.0.0");

#else
        SysPkgDateTimeType  SysDateTimeType;
        memset(&SysDateTimeType, 0, sizeof(SysPkgDateTimeType));

        SysPkgSysTime SysSysTime;
        memset(&SysSysTime, 0, sizeof(SysPkgSysTime));

        SysPkgTimeZone SysTimeZone;
        memset(&SysTimeZone, 0, sizeof(SysPkgTimeZone));

        SysPkgNtpServerInfo SysNtpServerInfo;
        memset(&SysNtpServerInfo, 0, sizeof(SysPkgNtpServerInfo));

        Result = SysGetTime(atoi(SessionId), atoi(AuthValue), &SysDateTimeType, &SysSysTime, &SysTimeZone, &SysNtpServerInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }
#endif

        RetFormat = "%s={\"RetCode\":\"%d\",\"SessionId\":\"%d\",\"Type\":\"%d\",\"NtpInterval\":\"%d\",\"Year\":\"%d\",\"Month\":\"%d\""
                    ",\"Day\":\"%d\",\"Hour\":\"%d\",\"Minute\":\"%d\",\"Second\":\"%d\",\"TimeZone\":\"%d\",\"TimeZoneName\":\"%s\""
                    ",\"NtpAddr_1\":\"%s\",\"NtpAddr_2\":\"%s\",\"NtpAddr_3\":\"%s\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, *SessionId,\
                SysDateTimeType.s_Type, \
                SysDateTimeType.s_NtpInterval, \
                SysSysTime.s_Year, \
                SysSysTime.s_Month,\
                SysSysTime.s_Day,\
                SysSysTime.s_Hour,\
                SysSysTime.s_Minute,\
                SysSysTime.s_Second,\
                SysTimeZone.s_TimeZone,\
                SysTimeZone.s_TimzeZoneName, \
                SysNtpServerInfo.s_NtpAddr_1, \
                SysNtpServerInfo.s_NtpAddr_2,\
                SysNtpServerInfo.s_NtpAddr_3
               );
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

GMI_RESULT CgiSetTime(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *Type = WEB_GET_VAR("Type");
    const char_t *NtpInterval = WEB_GET_VAR("NtpInterval");
    const char_t *Year = WEB_GET_VAR("Year");
    const char_t *Month = WEB_GET_VAR("Month");
    const char_t *Day = WEB_GET_VAR("Day");
    const char_t *Hour = WEB_GET_VAR("Hour");
    const char_t *Minute = WEB_GET_VAR("Minute");
    const char_t *Second = WEB_GET_VAR("Second");
    const char_t *TimeZoneName = WEB_GET_VAR("TimeZoneName");
    const char_t *NtpAddr_1 = WEB_GET_VAR("NtpAddr_1");
    const char_t *NtpAddr_2 = WEB_GET_VAR("NtpAddr_2");
    const char_t *NtpAddr_3 = WEB_GET_VAR("NtpAddr_3");

    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgDateTimeType  SysDateTimeType;
        memset(&SysDateTimeType, 0, sizeof(SysPkgDateTimeType));

        SysPkgSysTime SysSysTime;
        memset(&SysSysTime, 0, sizeof(SysPkgSysTime));

        SysPkgTimeZone SysTimeZone;
        memset(&SysTimeZone, 0, sizeof(SysPkgTimeZone));

        SysPkgNtpServerInfo SysNtpServerInfo;
        memset(&SysNtpServerInfo, 0, sizeof(SysPkgNtpServerInfo));

        //Call Get system Time api

        Result = SysGetTime(atoi(SessionId), atoi(AuthValue), &SysDateTimeType, &SysSysTime, &SysTimeZone, &SysNtpServerInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        if (NULL != Type)
            SysDateTimeType.s_Type = atoi(Type);
        if (NULL != NtpInterval)
            SysDateTimeType.s_NtpInterval = atoi(NtpInterval);
        if (NULL != Year)
            SysSysTime.s_Year = atoi(Year);
        if (NULL != Month)
            SysSysTime.s_Month = atoi(Month);
        if (NULL != Day)
            SysSysTime.s_Day = atoi(Day);
        if (NULL != Hour)
            SysSysTime.s_Hour = atoi(Hour);
        if (NULL != Minute)
            SysSysTime.s_Minute = atoi(Minute);
        if (NULL != Second)
            SysSysTime.s_Second = atoi(Second);
        if (NULL != TimeZoneName)
            strcpy(SysTimeZone.s_TimzeZoneName, TimeZoneName);
        if (NULL != NtpAddr_1)
            strcpy(SysNtpServerInfo.s_NtpAddr_1, NtpAddr_1);
        if (NULL != NtpAddr_2)
            strcpy(SysNtpServerInfo.s_NtpAddr_2, NtpAddr_2);
        if (NULL != NtpAddr_3)
            strcpy(SysNtpServerInfo.s_NtpAddr_3, NtpAddr_3);

        Result = SysSetTime(atoi(SessionId), atoi(AuthValue), &SysDateTimeType, &SysSysTime, &SysTimeZone, &SysNtpServerInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }
        CGI_ERROR("SysSetTime end[%d] \n",__LINE__);

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}


GMI_RESULT CgiSystemRootCmd(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId ||  NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        //Call SysCtrlSystem
        Result = SysReboot(atoi(SessionId), atoi(AuthValue));
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiSystemDefaultCmd(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        //Call SysCtrlSystem
        Result = FactoryDefaultAll(atoi(SessionId), atoi(AuthValue));
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiSystemSimpleDefaultCmd(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        //Call SysCtrlSystem
        Result = FactorySimpleDefaultAll(atoi(SessionId), atoi(AuthValue));
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}


GMI_RESULT CgiGetUsers(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        char_t *Buffer = NULL;
        Buffer = (char_t*)malloc(32 * sizeof(SysPkgUserInfo));
        if(NULL == Buffer)
        {
            RetCode = RETCODE_ERROR;
            Buffer = NULL;
            break;
        }

        memset(Buffer, 0, SYSTEM_MAX_USER_COUNT * sizeof(SysPkgUserInfo));
        SysPkgUserInfo *SysUserInfo;
        SysUserInfo = (SysPkgUserInfo *)Buffer;

        uint32_t RealUserCnt = 0;
        Result = SysGetUsers(atoi(SessionId), atoi(AuthValue), SysUserInfo, SYSTEM_MAX_USER_COUNT, &RealUserCnt);
        if (FAILED(Result))
        {
            if(NULL != Buffer)
            {
                free(Buffer);
                Buffer = NULL;
            }
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\",\"RealUserCnt\":\"%d\"%s}";

        char_t TmpBuf[MAX_CHAR_BUFFER_LENTH];
        memset(TmpBuf, 0, sizeof(TmpBuf));
        uint32_t i=0, Length= 0;
        for (i=0; i<RealUserCnt; i++)
        {
            Length += sprintf(TmpBuf+Length,",\"UserName%d\":\"%s\",\"UserPass%d\":\"%s\",\"UserLevel%d\":\"%u\",\"UserFlag%d\":\"%u\"",      \
                              i, SysUserInfo[i].s_UserName, i, SysUserInfo[i].s_UserPass,i, SysUserInfo[i].s_UserLevel, i, SysUserInfo[i].s_UserFlag);
        }
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode, RealUserCnt, TmpBuf);

        if(NULL != Buffer)
        {
            free(Buffer);
            Buffer = NULL;
        }
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT  CgiAddUsers(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *UserName = WEB_GET_VAR("UserName");
    const char_t *UserPass = WEB_GET_VAR("UserPass");
    const char_t *UserFlag = WEB_GET_VAR("UserFlag");
    const char_t *UserLevel = WEB_GET_VAR("UserLevel");
    const char_t *RetFormat;
    char_t	Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId
                || NULL == AuthValue
                || NULL == UserName
                || NULL == UserPass
                || NULL == UserFlag
                || NULL == UserLevel)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgUserInfo SysUserInfo;
        memset(&SysUserInfo, 0, sizeof(SysPkgUserInfo));

        strcpy(SysUserInfo.s_UserName, UserName);
        strcpy(SysUserInfo.s_UserPass, UserPass);
        SysUserInfo.s_UserFlag = atoi(UserFlag);
        SysUserInfo.s_UserLevel = atoi(UserLevel);

        //Call CgiSetUsers
        Result = SysAddUsers(atoi(SessionId), atoi(AuthValue), &SysUserInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}


GMI_RESULT  CgiModifyUsers(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *UserName = WEB_GET_VAR("UserName");
    const char_t *UserPass = WEB_GET_VAR("UserPass");
    const char_t *UserFlag = WEB_GET_VAR("UserFlag");
    const char_t *UserLevel = WEB_GET_VAR("UserLevel");
    const char_t *RetFormat;
    char_t	Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId
                || NULL == AuthValue
                ||NULL == UserName
                || NULL == UserPass
                || NULL == UserFlag
                || NULL == UserLevel)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgUserInfo SysUserInfo;
        memset(&SysUserInfo, 0, sizeof(SysPkgUserInfo));

        strcpy(SysUserInfo.s_UserName, UserName);
        strcpy(SysUserInfo.s_UserPass, UserPass);
        SysUserInfo.s_UserFlag = atoi(UserFlag);
        SysUserInfo.s_UserLevel = atoi(UserLevel);

        //Call CgiSetUsers
        Result = SysModifyUsers(atoi(SessionId), atoi(AuthValue), &SysUserInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiSetUsers(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *UserName = WEB_GET_VAR("UserName");
    const char_t *UserPass = WEB_GET_VAR("UserPass");
    const char_t *UserFlag = WEB_GET_VAR("UserFlag");
    const char_t *UserLevel = WEB_GET_VAR("UserLevel");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId
                || NULL == AuthValue
                ||NULL == UserName
                || NULL == UserPass
                || NULL == UserFlag
                || NULL == UserLevel)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgUserInfo SysUserInfo;
        memset(&SysUserInfo, 0, sizeof(SysPkgUserInfo));

        strcpy(SysUserInfo.s_UserName, UserName);
        strcpy(SysUserInfo.s_UserPass, UserPass);
        SysUserInfo.s_UserFlag = atoi(UserFlag);
        SysUserInfo.s_UserLevel = atoi(UserLevel);

        //Call CgiSetUsers
        Result = SysSetUsers(atoi(SessionId), atoi(AuthValue), &SysUserInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiDelUsers(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *UserName = WEB_GET_VAR("UserName");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId
                || NULL == AuthValue
                ||NULL == UserName)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgUserInfo SysUserInfo;
        memset(&SysUserInfo, 0, sizeof(SysPkgUserInfo));
        strcpy(SysUserInfo.s_UserName, UserName);

        //Call SysDelUsers api
        Result = SysDelUsers(atoi(SessionId), atoi(AuthValue), &SysUserInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

//Get Encode string Number
GMI_RESULT CgiGetEncodeStreamNum(const char_t * FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        uint32_t StreamNum = 0;

        Result = SysGetEncodeStreamNum(atoi(SessionId), atoi(AuthValue), &StreamNum);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\",\"StreamNum\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode, StreamNum);

        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);

    return GMI_FAIL;
}

GMI_RESULT CgiGetEncodeCfg(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;
    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        uint32_t StreamNum = 0;
        Result = SysGetEncodeStreamNum(atoi(SessionId), atoi(AuthValue), &StreamNum);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        char_t *Buffer = NULL;
        Buffer = (char_t*)malloc(StreamNum * sizeof(SysPkgEncodeCfg));
        if(NULL == Buffer)
        {
            RetCode = RETCODE_ERROR;
            Buffer = NULL;
            break;
        }

        memset(Buffer, 0, StreamNum*sizeof(SysPkgEncodeCfg));
        SysPkgEncodeCfg *SysEncodeCfg;
        SysEncodeCfg = (SysPkgEncodeCfg *)Buffer;

        //Call GetSysPkgEncodeCfg api
        uint32_t RspEncodeCfgNum = 0;
        Result = SysGetEncodeCfg(atoi(SessionId), atoi(AuthValue), SysEncodeCfg, StreamNum, &RspEncodeCfgNum);
        if (FAILED(Result))
        {
            if(NULL != Buffer)
            {
                free(Buffer);
                Buffer = NULL;
            }
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"RspEncodeCfgNum\":\"%u\"%s}";

        uint32_t Length = 0, i = 0;
        char_t TmpBuf[MAX_CHAR_BUFFER_LENTH];
        memset(TmpBuf, 0, sizeof(TmpBuf));
        for(i=0; i<RspEncodeCfgNum; i++)
        {
            Length += sprintf(TmpBuf+Length,",\"VideoId%d\":\"%d\",\"StreamType%d\":\"%d\",\"Compression%d\":\"%d\",\"PicWidth%d\":\"%d\",\"PicHeight%d\":\"%d\""
                              ",\"BitrateCtrl%d\":\"%d\",\"Quality%d\":\"%d\",\"FPS%d\":\"%d\",\"BitRateAverage%d\":\"%d\",\"BitRateUp%d\":\"%d\""
                              ",\"BitRateDown%d\":\"%d\",\"Gop%d\":\"%d\",\"Rotate%d\":\"%d\",\"Flag%d\":\"%d\"",                                                               \
                              i, SysEncodeCfg[i].s_VideoId, i,SysEncodeCfg[i].s_StreamType,i, SysEncodeCfg[i].s_Compression, i, SysEncodeCfg[i].s_PicWidth,     \
                              i,SysEncodeCfg[i].s_PicHeight,i ,SysEncodeCfg[i].s_BitrateCtrl, i, SysEncodeCfg[i].s_Quality, i, SysEncodeCfg[i].s_FPS,                       \
                              i, SysEncodeCfg[i].s_BitRateAverage, i, SysEncodeCfg[i].s_BitRateUp, i, SysEncodeCfg[i].s_BitRateDown,                                            \
                              i, SysEncodeCfg[i].s_Gop, i, SysEncodeCfg[i].s_Rotate, i, SysEncodeCfg[i].s_Flag);
        }

        fprintf(stdout, RetFormat, Cmd, RetCode, RspEncodeCfgNum, TmpBuf);

        if(NULL != Buffer)
        {
            free(Buffer);
            Buffer = NULL;
        }
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

GMI_RESULT CgiSetEncodeCfg(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *StreamId = WEB_GET_VAR("StreamId");
    const char_t *StreamType = WEB_GET_VAR("StreamType");
    const char_t *Compression = WEB_GET_VAR("Compression");
    const char_t *PicHeight = WEB_GET_VAR("PicHeight");
    const char_t *PicWidth = WEB_GET_VAR("PicWidth");
    const char_t *BitrateCtrl = WEB_GET_VAR("BitrateCtrl");
    const char_t *Quality = WEB_GET_VAR("Quality");
    const char_t *FPS= WEB_GET_VAR("FPS");
    const char_t *BitRateAverage = WEB_GET_VAR("BitRateAverage");
    const char_t *BitRateUp = WEB_GET_VAR("BitRateUp");
    const char_t *BitRateDown = WEB_GET_VAR("BitRateDown");
    const char_t *Gop = WEB_GET_VAR("Gop");
    const char_t *Rotate = WEB_GET_VAR("Rotate");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    CGI_ERROR("CgiSetEncodeCfg start[%d] \n",__LINE__);

    do
    {

        if (NULL == SessionId || NULL == StreamId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgEncodeCfg SysEncodeCfg;
        memset(&SysEncodeCfg, 0, sizeof(SysPkgEncodeCfg));

        SysEncodeCfg.s_VideoId = 1;
        SysEncodeCfg.s_Compression = atoi(Compression);
        SysEncodeCfg.s_StreamType = atoi(StreamType);
        SysEncodeCfg.s_PicHeight = atoi(PicHeight);
        SysEncodeCfg.s_PicWidth= atoi(PicWidth);
        SysEncodeCfg.s_BitrateCtrl = atoi(BitrateCtrl);
        SysEncodeCfg.s_Quality = atoi(Quality);
        SysEncodeCfg.s_FPS= atoi(FPS);
        SysEncodeCfg.s_BitRateAverage = atoi(BitRateAverage);
        SysEncodeCfg.s_BitRateUp = atoi(BitRateUp);
        SysEncodeCfg.s_BitRateDown = atoi(BitRateDown);
        SysEncodeCfg.s_Gop = atoi(Gop);
        SysEncodeCfg.s_Rotate = atoi(Rotate);
        SysEncodeCfg.s_Flag = atoi(StreamId);

        //Call SysPkgEncodeCfg api
        Result = SysSetEncodeCfg(atoi(SessionId), atoi(AuthValue), atoi(StreamId), &SysEncodeCfg);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}


GMI_RESULT CgiFactoryDefaultStreamCombine(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {

        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        //Call SysPkgEncodeCfg api
        Result = FactoryDefaultStreamCombine(atoi(SessionId), atoi(AuthValue));
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiFactoryDefaultVideoEncode(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {

        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        //Call SysPkgEncodeCfg api
        Result = FactoryDefaultVideoEncode(atoi(SessionId), atoi(AuthValue));
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiGetImageCfg(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgImaging SysImaging;
        memset(&SysImaging, 0, sizeof(SysPkgImaging));

        //Call SysGetImaging
        Result = SysGetImaging(atoi(SessionId), atoi(AuthValue), &SysImaging);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"SessionId\":\"%d\",\"VideoId\":\"%d\",\"Brightness\":\"%d\",\"Contrast\":\"%d\""
                    ",\"Saturation\":\"%d\",\"Hue\":\"%d\",\"Sharpness\":\"%d\",\"ExposureMode\":\"%d\",\"ShutterMin\":\"%d\""
                    ",\"ShutterMax\":\"%d\",\"GainMax\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, *SessionId,\
                SysImaging.s_VideoId, \
                SysImaging.s_Brightness,\
                SysImaging.s_Contrast,\
                SysImaging.s_Saturation,\
                SysImaging.s_Hue,\
                SysImaging.s_Sharpness,\
                SysImaging.s_Exposure.s_ExposureMode,\
                SysImaging.s_Exposure.s_ShutterMin,\
                SysImaging.s_Exposure.s_ShutterMax,\
                SysImaging.s_Exposure.s_GainMax
               );
        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

GMI_RESULT CgiSetImageCfg(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *VideoId = WEB_GET_VAR("VideoId");
    const char_t *Brightness = WEB_GET_VAR("Brightness");
    const char_t *Contrast = WEB_GET_VAR("Contrast");
    const char_t *Saturation = WEB_GET_VAR("Saturation");
    const char_t *Hue = WEB_GET_VAR("Hue");
    const char_t *Sharpness = WEB_GET_VAR("Sharpness");
    const char_t *ExposureMode = WEB_GET_VAR("ExposureMode");
    const char_t *ShutterMin = WEB_GET_VAR("ShutterMin");
    const char_t *ShutterMax = WEB_GET_VAR("ShutterMax");
    const char_t *GainMax = WEB_GET_VAR("GainMax");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgImaging SysImaging;
        memset(&SysImaging, 0, sizeof(SysPkgImaging));
        //Call GetImageCfg function

        Result = SysGetImaging(atoi(SessionId), atoi(AuthValue), &SysImaging);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        if (NULL != VideoId)
            SysImaging.s_VideoId = atoi(VideoId);
        if (NULL != Brightness)
            SysImaging.s_Brightness = atoi(Brightness);
        if (NULL != Contrast)
            SysImaging.s_Contrast = atoi(Contrast);
        if (NULL != Saturation)
            SysImaging.s_Saturation = atoi(Saturation);
        if (NULL != Hue)
            SysImaging.s_Hue = atoi(Hue);
        if (NULL != Sharpness)
            SysImaging.s_Sharpness = atoi(Sharpness);
        if (NULL != ExposureMode)
            SysImaging.s_Exposure.s_ExposureMode = atoi(ExposureMode);
        if (NULL != ShutterMin)
            SysImaging.s_Exposure.s_ShutterMin = atoi(ShutterMin);
        if (NULL != ShutterMax)
            SysImaging.s_Exposure.s_ShutterMax = atoi(ShutterMax);
        if (NULL != GainMax)
            SysImaging.s_Exposure.s_GainMax = atoi(GainMax);

        //Call SetImageCfg function
        Result = SysSetImaging(atoi(SessionId), atoi(AuthValue), &SysImaging);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

GMI_RESULT CgiGetAdvancedImaging(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgAdvancedImaging SysAdvancedImaging;
        memset(&SysAdvancedImaging, 0, sizeof(SysPkgAdvancedImaging));
        Result = SysGetAdvancedImaging(atoi(SessionId), atoi(AuthValue), &SysAdvancedImaging);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        //Call GetAdvancedImaging function

        RetFormat = "%s={\"RetCode\":\"%d\",\"SessionId\":\"%d\",\"VideoId\":\"%d\",\"MeteringMode\":\"%d\",\"BackLightCompFlag\":\"%d\""
                    ",\"DcIrisFlag\":\"%d\",\"LocalExposure\":\"%d\",\"MctfStrength\":\"%d\",\"DcIrisDuty\":\"%d\",\"AeTargetRatio\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, *SessionId,\
                SysAdvancedImaging.s_VideoId, \
                SysAdvancedImaging.s_MeteringMode,\
                SysAdvancedImaging.s_BackLightCompFlag,\
                SysAdvancedImaging.s_DcIrisFlag,\
                SysAdvancedImaging.s_LocalExposure,\
                SysAdvancedImaging.s_MctfStrength,\
                SysAdvancedImaging.s_DcIrisDuty,\
                SysAdvancedImaging.s_AeTargetRatio
               );
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

GMI_RESULT CgiSetAdvancedImaging(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *VideoId = WEB_GET_VAR("VideoId");
    const char_t *MeteringMode = WEB_GET_VAR("MeteringMode");
    const char_t *BackLightCompFlag = WEB_GET_VAR("BackLightCompFlag");
    const char_t *DcIrisFlag = WEB_GET_VAR("DcIrisFlag");
    const char_t *LocalExposure = WEB_GET_VAR("LocalExposure");
    const char_t *MctfStrength = WEB_GET_VAR("MctfStrength");
    const char_t *DcIrisDuty = WEB_GET_VAR("DcIrisDuty");
    const char_t *AeTargetRatio = WEB_GET_VAR("AeTargetRatio");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;

    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgAdvancedImaging SysAdvancedImaging;
        memset(&SysAdvancedImaging, 0, sizeof(SysPkgAdvancedImaging));

        Result = SysGetAdvancedImaging(atoi(SessionId), atoi(AuthValue), &SysAdvancedImaging);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        if (NULL != VideoId)
            SysAdvancedImaging.s_VideoId = atoi(VideoId);
        if (NULL != MeteringMode)
            SysAdvancedImaging.s_MeteringMode = atoi(MeteringMode);
        if (NULL != BackLightCompFlag)
            SysAdvancedImaging.s_BackLightCompFlag = atoi(BackLightCompFlag);
        if (NULL != DcIrisFlag)
            SysAdvancedImaging.s_DcIrisFlag = atoi(DcIrisFlag);
        if (NULL != LocalExposure)
            SysAdvancedImaging.s_LocalExposure = atoi(LocalExposure);
        if (NULL != MctfStrength)
            SysAdvancedImaging.s_MctfStrength = atoi(MctfStrength);
        if (NULL != DcIrisDuty)
            SysAdvancedImaging.s_DcIrisDuty = atoi(DcIrisDuty);
        if (NULL != AeTargetRatio)
            SysAdvancedImaging.s_AeTargetRatio = atoi(AeTargetRatio);

        //Call SetAdvancedImaging function
        Result = SysSetAdvancedImaging(atoi(SessionId), atoi(AuthValue), &SysAdvancedImaging);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

GMI_RESULT CgiSetImagingDefaultMode(const char_t *FncCmd)
{

    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;

    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        //Call SetAdvancedImaging function
        Result = FactoryDefaultImaging(atoi(SessionId), atoi(AuthValue));
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        return GMI_SUCCESS;
    } while(0);


    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

GMI_RESULT CgiGetCapabilities(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t	Cmd[CMD_BUFFER_LENTH];
    char_t *XmlBuf =  NULL;
    char_t *BufIn =  NULL;
    char_t *BufOut =  NULL;

    int32_t RetCode = RETCODE_OK;

    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_XML);
    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgXml SysXml;
        memset(&SysXml, 0, sizeof(SysPkgXml));

        XmlBuf = (char_t *)malloc(MAX_XML_BUFFER_LENTH);
        if(NULL == XmlBuf)
        {
            XmlBuf = NULL;
            RetCode = RETCODE_ERROR;
            break;
        }
        memset(XmlBuf, 0, sizeof(XmlBuf));

        Result = SysGetCapabilities(atoi(SessionId), atoi(AuthValue), XmlBuf,MAX_XML_BUFFER_LENTH,&SysXml);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        const char_t XmlHeader[MAX_BUFFER_LENTH]	= "<?xml version=\"1.0\"?>";
        int32_t XmlHeaderLenght = 0;
        XmlHeaderLenght = strlen(XmlHeader);

        char_t *XmlBufTmp = NULL;
        XmlBufTmp = (char_t *)malloc(MAX_XML_BUFFER_LENTH);
        if(NULL == XmlBufTmp)
        {
            XmlBufTmp = NULL;
            RetCode = RETCODE_ERROR;
            free(XmlBuf);
            XmlBuf = NULL;
            break;
        }

        memset(XmlBufTmp, 0, sizeof(XmlBufTmp));
        memcpy(XmlBufTmp, XmlBuf+XmlHeaderLenght, SysXml.s_ContentLength - XmlHeaderLenght);

        const char_t SrcRpl[MIN_BUFFER_LENTH]="<";
        const char_t DstRpl[MIN_BUFFER_LENTH]="<;";

        BufIn = (char_t *)malloc(MAX_XML_BUFFER_LENTH);
        if(NULL == BufIn)
        {
            BufIn = NULL;
            RetCode = RETCODE_ERROR;
            free(XmlBuf);
            XmlBuf = NULL;
            free(XmlBufTmp);
            XmlBufTmp = NULL;
            break;
        }
        memset(BufIn, 0, sizeof(BufIn));
        Result = GMI_StrRpl(BufIn, XmlBufTmp, SrcRpl, DstRpl);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        const char_t SrcRpl2[MIN_BUFFER_LENTH] = ">";
        const char_t DstRpl2[MIN_BUFFER_LENTH] = ">;";

        BufOut = (char_t *)malloc(MAX_XML_BUFFER_LENTH);
        if(NULL == BufOut)
        {
            BufOut = NULL;
            RetCode = RETCODE_ERROR;
            free(XmlBuf);
            XmlBuf = NULL;
            free(XmlBufTmp);
            XmlBufTmp = NULL;
            free(BufIn);
            BufIn = NULL;
            break;
        }

        Result = GMI_StrRpl(BufOut, BufIn, SrcRpl2, DstRpl2);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"SessionId\":\"%d\",\"XmlBuf\":\"%s\"}";
        fprintf(stdout, RetFormat, Cmd, RetCode, *SessionId, BufOut);

        if(NULL != XmlBuf)
        {
            free(XmlBuf);
            XmlBuf = NULL;
        }
        if(NULL != XmlBufTmp)
        {
            free(XmlBufTmp);
            XmlBufTmp = NULL;
        }
        if(NULL != BufIn)
        {
            free(BufIn);
            BufIn = NULL;
        }
        if(NULL != BufOut)
        {
            free(BufOut);
            BufOut = NULL;
        }

        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\",\"XmlBuf\":\"%s\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode,NULL);
    return GMI_FAIL;

}

GMI_RESULT CgiGetWorkState(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t Cmd[CMD_BUFFER_LENTH];
    char_t XmlBuf[MAX_CHAR_BUFFER_LENTH] = {"0"};
    char_t BufIn[MAX_CHAR_BUFFER_LENTH] = {"0"};
    char_t BufOut[MAX_CHAR_BUFFER_LENTH] = {"0"};
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_XML);
    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

#ifndef DEBUG_TEST
        //Call GetAdvancedImaging function
        SysPkgXml SysXml;
        memset(&SysXml, 0, sizeof(SysPkgXml));
        memset(XmlBuf, 0, sizeof(XmlBuf));

        Result = SysGetWorkState(atoi(SessionId), atoi(AuthValue), XmlBuf,MAX_CHAR_BUFFER_LENTH,&SysXml);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        const char_t XmlHeader[MAX_BUFFER_LENTH]	= "<?xml version=\"1.0\"?>";
        int32_t XmlHeaderLenght = 0;
        XmlHeaderLenght = strlen(XmlHeader);
        char_t XmlBufTmp[MAX_CHAR_BUFFER_LENTH] = {"0"};

        memset(XmlBufTmp, 0, sizeof(XmlBufTmp));
        memcpy(XmlBufTmp, XmlBuf+XmlHeaderLenght, SysXml.s_ContentLength - XmlHeaderLenght);

        const char_t SrcRpl[MIN_BUFFER_LENTH]="<";
        const char_t DstRpl[MIN_BUFFER_LENTH]="<;";
        Result = GMI_StrRpl(BufIn, XmlBufTmp, SrcRpl, DstRpl);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        const char_t SrcRpl2[MIN_BUFFER_LENTH] = ">";
        const char_t DstRpl2[MIN_BUFFER_LENTH] = ">;";
        Result = GMI_StrRpl(BufOut, BufIn, SrcRpl2, DstRpl2);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }
#endif

        RetFormat = "%s={\"RetCode\":\"%d\",\"SessionId\":\"%d\",\"XmlBuf\":\"%s\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, *SessionId, BufOut);

        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\",\"XmlBuf\":\"%s\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode,NULL);
    return GMI_FAIL;

}

GMI_RESULT CgiGetAutoFocusMode(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgAutoFocus SysAutoFocusPtr;
        memset(&SysAutoFocusPtr, 0, sizeof(SysPkgAutoFocus));

        Result =  SysGetAutoFocusCfg(atoi(SessionId), atoi(AuthValue), &SysAutoFocusPtr);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"SessionId\":\"%d\",\"FocusMode\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, *SessionId, SysAutoFocusPtr.s_FocusMode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiSetAutoFocusMode(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *FocusMode= WEB_GET_VAR("FocusMode");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue ||NULL == FocusMode)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgAutoFocus SysAutoFocusPtr;
        memset(&SysAutoFocusPtr, 0, sizeof(SysPkgAutoFocus));

        SysAutoFocusPtr.s_FocusMode = atoi(FocusMode);

        Result =  SysSetAutoFocusCfg(atoi(SessionId), atoi(AuthValue), &SysAutoFocusPtr);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiFocusGlobalScan(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        Result = SysFocusGlobalScan(atoi(SessionId), atoi(AuthValue));
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiGetSystemAutoFocusIsValid(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
//    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        boolean_t AutoFlags = true;
        GMI_CheckAutoFlags(&AutoFlags);

        RetFormat = "%s={\"RetCode\":\"%d\",\"AutoFlags\":\"%u\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, AutoFlags);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}


GMI_RESULT CgiGetShowInfo(const char_t *FncCmd)
{

    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t	Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        uint32_t StreamNum = 0;
        Result = SysGetEncodeStreamNum(atoi(SessionId), atoi(AuthValue), &StreamNum);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        char_t *Buffer = NULL;
        Buffer = (char_t*)malloc(StreamNum * sizeof(SysPkgShowCfg));
        if(NULL == Buffer)
        {
            RetCode = RETCODE_ERROR;
            Buffer = NULL;
            break;
        }

        memset(Buffer, 0, StreamNum*sizeof(SysPkgShowCfg));
        SysPkgShowCfg *SysShowCfg;
        SysShowCfg = (SysPkgShowCfg *)Buffer;

        //Call GetSysPkgEncodeCfg api
        uint32_t RspShowNum = 0;
        Result = SysGetShowInfo(atoi(SessionId), atoi(AuthValue), SysShowCfg, StreamNum, &RspShowNum);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            Buffer = NULL;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"RspShowNum\":\"%u\"%s}";

        uint32_t Length = 0, i = 0;
        char_t TmpBuf[4096];
        memset(TmpBuf, 0, 4096);
        for (i=0; i < RspShowNum; i++)
        {
            Length += sprintf(TmpBuf+Length,",\"Flag%d\":\"%d\",\"TimeEnable%d\":\"%d\",\"Language%d\":\"%d\""
                              ",\"TimeDisplayX%d\":\"%d\",\"TimeDisplayY%d\":\"%d\",\"DateStyle%d\":\"%d\",\"TimeStyle%d\":\"%d\",\"TimeFontColor%d\":\"%d\""
                              ",\"TimeFontSize%d\":\"%d\",\"TimeFontBlod%d\":\"%d\",\"TimeFontRotate%d\":\"%d\",\"TimeFontItalic%d\":\"%d\",\"FontOutline%d\":\"%d\""
                              ",\"ChannelEnable%d\":\"%d\",\"ChannelDisplayX%d\":\"%d\",\"ChannelDisplayY%d\":\"%d\",\"ChannelFontColor%d\":\"%d\",\"ChannelFontSize%d\":\"%d\""
                              ",\"ChannelFontBlod%d\":\"%d\",\"ChannelFontRotate%d\":\"%d\",\"ChannelFontItalic%d\":\"%d\",\"ChannelOutline%d\":\"%d\",\"ChannelName%d\":\"%s\""
                              ,i , SysShowCfg[i].s_Flag, i , SysShowCfg[i].s_TimeInfo.s_Enable , i ,SysShowCfg[i].s_TimeInfo.s_Language                                                          \
                              ,i , SysShowCfg[i].s_TimeInfo.s_DisplayX, i, SysShowCfg[i].s_TimeInfo.s_DisplayY, i, SysShowCfg[i].s_TimeInfo.s_DateStyle                                 \
                              ,i , SysShowCfg[i].s_TimeInfo.s_TimeStyle ,i , SysShowCfg[i].s_TimeInfo.s_FontColor, i, SysShowCfg[i].s_TimeInfo.s_FontSize                              \
                              ,i , SysShowCfg[i].s_TimeInfo.s_FontBlod, i, SysShowCfg[i].s_TimeInfo.s_FontRotate,i , SysShowCfg[i].s_TimeInfo.s_FontItalic                             \
                              ,i , SysShowCfg[i].s_TimeInfo.s_FontOutline, i, SysShowCfg[i].s_ChannelInfo.s_Enable, i,SysShowCfg[i].s_ChannelInfo.s_DisplayX                        \
                              ,i , SysShowCfg[i].s_ChannelInfo.s_DisplayY, i, SysShowCfg[i].s_ChannelInfo.s_FontColor, i, SysShowCfg[i].s_ChannelInfo.s_FontSize                  \
                              ,i , SysShowCfg[i].s_ChannelInfo.s_FontBlod, i, SysShowCfg[i].s_ChannelInfo.s_FontRotate, i, SysShowCfg[i].s_ChannelInfo.s_FontItalic               \
                              ,i , SysShowCfg[i].s_ChannelInfo.s_FontOutline , i, SysShowCfg[i].s_ChannelInfo.s_ChannelName);
        }

        fprintf(stdout, RetFormat, Cmd, RetCode, RspShowNum, TmpBuf);

        if(NULL != Buffer)
        {
            free(Buffer);
            Buffer = NULL;
        }
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

GMI_RESULT CgiSetShowInfo(const char_t *FncCmd)
{

    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *Flag = WEB_GET_VAR("Flag");
    const char_t *TimeEnable = WEB_GET_VAR("TimeEnable");
    //const char_t *Language = WEB_GET_VAR("Language");
    const char_t *TimeDisplayX = WEB_GET_VAR("TimeDisplayX");
    const char_t *TimeDisplayY = WEB_GET_VAR("TimeDisplayY");
    const char_t *DateStyle = WEB_GET_VAR("DateStyle");
    const char_t *TimeStyle = WEB_GET_VAR("TimeStyle");
    const char_t *TimeFontColor = WEB_GET_VAR("TimeFontColor");
    const char_t *TimeFontSize = WEB_GET_VAR("TimeFontSize");
    const char_t *TimeFontBlod = WEB_GET_VAR("TimeFontBlod");
    //const char_t *TimeFontRotate = WEB_GET_VAR("TimeFontRotate");
    const char_t *TimeFontItalic = WEB_GET_VAR("TimeFontItalic");
    const char_t *FontOutline = WEB_GET_VAR("FontOutline");
    const char_t *ChannelEnable = WEB_GET_VAR("ChannelEnable");
    const char_t *ChannelDisplayX = WEB_GET_VAR("ChannelDisplayX");
    const char_t *ChannelDisplayY = WEB_GET_VAR("ChannelDisplayY");
    const char_t *ChannelFontColor = WEB_GET_VAR("ChannelFontColor");
    const char_t *ChannelFontSize = WEB_GET_VAR("ChannelFontSize");
    const char_t *ChannelFontBlod = WEB_GET_VAR("ChannelFontBlod");
    //const char_t *ChannelFontRotate = WEB_GET_VAR("ChannelFontRotate");
    const char_t *ChannelFontItalic = WEB_GET_VAR("ChannelFontItalic");
    const char_t *ChannelOutline = WEB_GET_VAR("ChannelOutline");
    const char_t *ChannelName = WEB_GET_VAR("ChannelName");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgShowCfg SysShowCfg;
        memset(&SysShowCfg, 0, sizeof(SysPkgShowCfg));
        SysShowCfg.s_VideoId = 1;
        SysShowCfg.s_Flag = atoi(Flag);
        SysShowCfg.s_TimeInfo.s_Enable = atoi(TimeEnable);
        SysShowCfg.s_TimeInfo.s_Language = 2;
        SysShowCfg.s_TimeInfo.s_DisplayX = atoi(TimeDisplayX);
        SysShowCfg.s_TimeInfo.s_DisplayY = atoi(TimeDisplayY);
        SysShowCfg.s_TimeInfo.s_TimeStyle = atoi(TimeStyle);
        SysShowCfg.s_TimeInfo.s_DateStyle = atoi(DateStyle);
        SysShowCfg.s_TimeInfo.s_FontColor= atoi(TimeFontColor);
        SysShowCfg.s_TimeInfo.s_FontSize = atoi(TimeFontSize);
        SysShowCfg.s_TimeInfo.s_FontBlod = atoi(TimeFontBlod);
        SysShowCfg.s_TimeInfo.s_FontRotate = 0;
        SysShowCfg.s_TimeInfo.s_FontItalic = atoi(TimeFontItalic);
        SysShowCfg.s_TimeInfo.s_FontOutline = atoi(FontOutline);
        SysShowCfg.s_ChannelInfo.s_Enable = atoi(ChannelEnable);
        SysShowCfg.s_ChannelInfo.s_DisplayX = atoi(ChannelDisplayX);
        SysShowCfg.s_ChannelInfo.s_DisplayY = atoi(ChannelDisplayY);
        SysShowCfg.s_ChannelInfo.s_FontColor = atoi(ChannelFontColor);
        SysShowCfg.s_ChannelInfo.s_FontSize = atoi(ChannelFontSize);
        SysShowCfg.s_ChannelInfo.s_FontBlod = atoi(ChannelFontBlod);
        SysShowCfg.s_ChannelInfo.s_FontRotate = 0;
        SysShowCfg.s_ChannelInfo.s_FontItalic = atoi(ChannelFontItalic);
        SysShowCfg.s_ChannelInfo.s_FontOutline = atoi(ChannelOutline);
        strcpy(SysShowCfg.s_ChannelInfo.s_ChannelName, ChannelName);

        Result = SysSetShowInfo(atoi(SessionId), atoi(AuthValue), &SysShowCfg);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }
        RetFormat = "%s={\"RetCode\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}


GMI_RESULT CgiGetWhiteBalanceMode(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgWhiteBalance SysWhiteBalance;
        memset(&SysWhiteBalance, 0, sizeof(SysPkgWhiteBalance));

        //Call WhiteBalanceMode
        Result =  SysGetWhiteBalance(atoi(SessionId), atoi(AuthValue), &SysWhiteBalance);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"SessionId\":\"%d\",\"Mode\":\"%d\",\"RGain\":\"%d\",\"BGain\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, *SessionId,\
                SysWhiteBalance.s_Mode, \
                SysWhiteBalance.s_RGain,\
                SysWhiteBalance.s_BGain);

        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

GMI_RESULT CgiSetWhiteBalanceMode(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *Mode = WEB_GET_VAR("Mode");
    const char_t *RGain = WEB_GET_VAR("RGain");
    const char_t *BGain = WEB_GET_VAR("BGain");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgWhiteBalance SysWhiteBalance;
        memset(&SysWhiteBalance, 0, sizeof(SysPkgWhiteBalance));

        //Call WhiteBalanceMode
        Result =  SysGetWhiteBalance(atoi(SessionId), atoi(AuthValue), &SysWhiteBalance);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        if (NULL != Mode)
            SysWhiteBalance.s_Mode = atoi(Mode);
        if (NULL != RGain)
            SysWhiteBalance.s_RGain = atoi(RGain);
        if (NULL != BGain)
            SysWhiteBalance.s_BGain = atoi(BGain);

        Result =  SysSetWhiteBalance(atoi(SessionId), atoi(AuthValue), &SysWhiteBalance);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode);
        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiGetDayNightMode(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgDaynight SysDaynight;
        memset(&SysDaynight, 0, sizeof(SysPkgDaynight));

        //Call WhiteBalanceMode
        Result = SysGetDaynight(atoi(SessionId), atoi(AuthValue), &SysDaynight);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"SessionId\":\"%d\",\"Mode\":\"%d\",\"DurationTime\":\"%d\",\"NightToDayThr\":\"%d\",\"DayToNightThr\":\"%d\""
                    ",\"SchedEnable0\":\"%d\",\"SchedEnable1\":\"%d\",\"SchedEnable2\":\"%d\",\"SchedEnable3\":\"%d\",\"SchedEnable4\":\"%d\",\"SchedEnable5\":\"%d\",\"SchedEnable6\":\"%d\",\"SchedEnable7\":\"%d\""
                    ",\"SchedStartTime0\":\"%u\",\"SchedStartTime1\":\"%u\",\"SchedStartTime2\":\"%u\",\"SchedStartTime3\":\"%u\",\"SchedStartTime4\":\"%u\",\"SchedStartTime5\":\"%u\",\"SchedStartTime6\":\"%u\",\"SchedStartTime7\":\"%u\""
                    ",\"SchedEndTime0\":\"%u\",\"SchedEndTime1\":\"%u\",\"SchedEndTime2\":\"%u\",\"SchedEndTime3\":\"%u\",\"SchedEndTime4\":\"%u\",\"SchedEndTime5\":\"%u\",\"SchedEndTime6\":\"%u\",\"SchedEndTime7\":\"%u\""
                    "}";

        fprintf(stdout, RetFormat, Cmd, RetCode, *SessionId,\
                SysDaynight.s_Mode, \
                SysDaynight.s_DurationTime,\
                SysDaynight.s_NightToDayThr,\
                SysDaynight.s_DayToNightThr,\
                SysDaynight.s_SchedEnable[0],\
                SysDaynight.s_SchedEnable[1],\
                SysDaynight.s_SchedEnable[2],\
                SysDaynight.s_SchedEnable[3],\
                SysDaynight.s_SchedEnable[4],\
                SysDaynight.s_SchedEnable[5],\
                SysDaynight.s_SchedEnable[6],\
                SysDaynight.s_SchedEnable[7],\
                SysDaynight.s_SchedStartTime[0],\
                SysDaynight.s_SchedStartTime[1],\
                SysDaynight.s_SchedStartTime[2],\
                SysDaynight.s_SchedStartTime[3],\
                SysDaynight.s_SchedStartTime[4],\
                SysDaynight.s_SchedStartTime[5],\
                SysDaynight.s_SchedStartTime[6],\
                SysDaynight.s_SchedStartTime[7],\
                SysDaynight.s_SchedEndTime[0],\
                SysDaynight.s_SchedEndTime[1],\
                SysDaynight.s_SchedEndTime[2],\
                SysDaynight.s_SchedEndTime[3],\
                SysDaynight.s_SchedEndTime[4],\
                SysDaynight.s_SchedEndTime[5],\
                SysDaynight.s_SchedEndTime[6],\
                SysDaynight.s_SchedEndTime[7]);

        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

GMI_RESULT CgiSetDayNightMode(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *Mode = WEB_GET_VAR("Mode");
    const char_t *DurationTime = WEB_GET_VAR("DurationTime");
    const char_t *NightToDayThr = WEB_GET_VAR("NightToDayThr");
    const char_t *DayToNightThr = WEB_GET_VAR("DayToNightThr");
    const char_t *SchedEnable0 = WEB_GET_VAR("SchedEnable0");
    const char_t *SchedEnable1 = WEB_GET_VAR("SchedEnable1");
    const char_t *SchedEnable2 = WEB_GET_VAR("SchedEnable2");
    const char_t *SchedEnable3 = WEB_GET_VAR("SchedEnable3");
    const char_t *SchedEnable4 = WEB_GET_VAR("SchedEnable4");
    const char_t *SchedEnable5 = WEB_GET_VAR("SchedEnable5");
    const char_t *SchedEnable6 = WEB_GET_VAR("SchedEnable6");
    const char_t *SchedEnable7 = WEB_GET_VAR("SchedEnable7");
    const char_t *SchedStartTime0 = WEB_GET_VAR("SchedStartTime0");
    const char_t *SchedStartTime1 = WEB_GET_VAR("SchedStartTime1");
    const char_t *SchedStartTime2 = WEB_GET_VAR("SchedStartTime2");
    const char_t *SchedStartTime3 = WEB_GET_VAR("SchedStartTime3");
    const char_t *SchedStartTime4 = WEB_GET_VAR("SchedStartTime4");
    const char_t *SchedStartTime5 = WEB_GET_VAR("SchedStartTime5");
    const char_t *SchedStartTime6 = WEB_GET_VAR("SchedStartTime6");
    const char_t *SchedStartTime7 = WEB_GET_VAR("SchedStartTime7");
    const char_t *SchedEndTime0 = WEB_GET_VAR("SchedEndTime0");
    const char_t *SchedEndTime1 = WEB_GET_VAR("SchedEndTime1");
    const char_t *SchedEndTime2 = WEB_GET_VAR("SchedEndTime2");
    const char_t *SchedEndTime3 = WEB_GET_VAR("SchedEndTime3");
    const char_t *SchedEndTime4 = WEB_GET_VAR("SchedEndTime4");
    const char_t *SchedEndTime5 = WEB_GET_VAR("SchedEndTime5");
    const char_t *SchedEndTime6 = WEB_GET_VAR("SchedEndTime6");
    const char_t *SchedEndTime7 = WEB_GET_VAR("SchedEndTime7");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgDaynight SysDaynight;
        memset(&SysDaynight, 0, sizeof(SysPkgDaynight));

        //Call WhiteBalanceMode
        Result = SysGetDaynight(atoi(SessionId), atoi(AuthValue), &SysDaynight);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        if (NULL != Mode)
            SysDaynight.s_Mode = atoi(Mode);
        if (NULL != DurationTime)
            SysDaynight.s_DurationTime = atoi(DurationTime);
        if (NULL != NightToDayThr)
            SysDaynight.s_NightToDayThr = atoi(NightToDayThr);
        if (NULL != DayToNightThr)
            SysDaynight.s_DayToNightThr = atoi(DayToNightThr);
        if (NULL != SchedEnable0)
            SysDaynight.s_SchedEnable[0] = atoi(SchedEnable0);
        if (NULL != SchedEnable1)
            SysDaynight.s_SchedEnable[1] = atoi(SchedEnable1);
        if (NULL != SchedEnable2)
            SysDaynight.s_SchedEnable[2] = atoi(SchedEnable2);
        if (NULL != SchedEnable3)
            SysDaynight.s_SchedEnable[3] = atoi(SchedEnable3);
        if (NULL != SchedEnable4)
            SysDaynight.s_SchedEnable[4] = atoi(SchedEnable4);
        if (NULL != SchedEnable5)
            SysDaynight.s_SchedEnable[5] = atoi(SchedEnable5);
        if (NULL != SchedEnable6)
            SysDaynight.s_SchedEnable[6] = atoi(SchedEnable6);
        if (NULL != SchedEnable7)
            SysDaynight.s_SchedEnable[7] = atoi(SchedEnable7);

        if (NULL != SchedStartTime0)
            SysDaynight.s_SchedStartTime[0] = atoi(SchedStartTime0);
        if (NULL != SchedStartTime1)
            SysDaynight.s_SchedStartTime[1] = atoi(SchedStartTime1);
        if (NULL != SchedStartTime2)
            SysDaynight.s_SchedStartTime[2] = atoi(SchedStartTime2);
        if (NULL != SchedStartTime3)
            SysDaynight.s_SchedStartTime[3] = atoi(SchedStartTime3);
        if (NULL != SchedStartTime4)
            SysDaynight.s_SchedStartTime[4] = atoi(SchedStartTime4);
        if (NULL != SchedStartTime5)
            SysDaynight.s_SchedStartTime[5] = atoi(SchedStartTime5);
        if (NULL != SchedStartTime6)
            SysDaynight.s_SchedStartTime[6] = atoi(SchedStartTime6);
        if (NULL != SchedStartTime7)
            SysDaynight.s_SchedStartTime[7] = atoi(SchedStartTime7);

        if (NULL != SchedEndTime0)
            SysDaynight.s_SchedEndTime[0] = atoi(SchedEndTime0);
        if (NULL != SchedEndTime1)
            SysDaynight.s_SchedEndTime[1] = atoi(SchedEndTime1);
        if (NULL != SchedEndTime2)
            SysDaynight.s_SchedEndTime[2] = atoi(SchedEndTime2);
        if (NULL != SchedEndTime3)
            SysDaynight.s_SchedEndTime[3] = atoi(SchedEndTime3);
        if (NULL != SchedEndTime4)
            SysDaynight.s_SchedEndTime[4] = atoi(SchedEndTime4);
        if (NULL != SchedEndTime5)
            SysDaynight.s_SchedEndTime[5] = atoi(SchedEndTime5);
        if (NULL != SchedEndTime6)
            SysDaynight.s_SchedEndTime[6] = atoi(SchedEndTime6);
        if (NULL != SchedEndTime7)
            SysDaynight.s_SchedEndTime[7] = atoi(SchedEndTime7);

        Result = SysSetDaynight(atoi(SessionId), atoi(AuthValue), &SysDaynight);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"SessionId\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, *SessionId);
        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiCheckIPExist(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *IpAddr = WEB_GET_VAR("IpAddr");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    do
    {
        if (NULL == SessionId || NULL == AuthValue || NULL == IpAddr)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetCode = CheckIpExist(IpAddr);

        RetFormat = "%s={\"RetCode\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode);
        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

GMI_RESULT CgiSetVideoSourceMirror(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *Mirror = WEB_GET_VAR("Mirror");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    do
    {
        if (NULL == SessionId || NULL == AuthValue || NULL == Mirror)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        Result =  SysSetVideoSourceMirror(atoi(SessionId), atoi(AuthValue), atoi(Mirror));
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode);
        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}


GMI_RESULT CgiGetVideoSourceMirror(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    int32_t Mirror = 0;
    const char_t *RetFormat;
    char_t	Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        Result =  SysGetVideoSourceMirror(atoi(SessionId), atoi(AuthValue), &Mirror);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"Mirror\":\"%u\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, Mirror);
        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

GMI_RESULT CgiGetVideoEncodeMirror(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *StreamId = WEB_GET_VAR("StreamId");
    int32_t Mirror = 0;
    const char_t *RetFormat;
    char_t	Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        Result =  SysGetVideoEncodeMirror(atoi(SessionId), atoi(AuthValue), atoi(StreamId), &Mirror);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"Mirror\":\"%u\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, Mirror);
        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiSetVideoEncodeMirror(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *StreamId = WEB_GET_VAR("StreamId");
    const char_t *Mirror = WEB_GET_VAR("Mirror");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    do
    {
        if (NULL == SessionId || NULL == AuthValue || NULL == StreamId ||NULL == Mirror)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        Result =  SysSetVideoEncodeMirror(atoi(SessionId), atoi(AuthValue), atoi(StreamId), atoi(Mirror));
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode);
        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}


GMI_RESULT CgiGetVideoEncStreamCombine(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgEncStreamCombine SysEncStreamCombinePtrl;
        memset(&SysEncStreamCombinePtrl, 0 ,sizeof(SysPkgEncStreamCombine));
        Result =  SysGetVideoEncStreamCombine(atoi(SessionId), atoi(AuthValue), &SysEncStreamCombinePtrl);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"EnableStreamNum\":\"%d\",\"StreamCombineNo\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, SysEncStreamCombinePtrl.s_EnableStreamNum, SysEncStreamCombinePtrl.s_StreamCombineNo);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiSetVideoEncStreamCombine(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *EnableStreamNum = WEB_GET_VAR("EnableStreamNum");
    const char_t *StreamCombineNo = WEB_GET_VAR("StreamCombineNo");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgEncStreamCombine SysEncStreamCombinePtrl;
        Result =  SysGetVideoEncStreamCombine(atoi(SessionId), atoi(AuthValue), &SysEncStreamCombinePtrl);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        //VideoId defoult value 1
        SysEncStreamCombinePtrl.s_VideoId = 1;

        if (NULL != EnableStreamNum)
            SysEncStreamCombinePtrl.s_EnableStreamNum = atoi(EnableStreamNum);
        if (NULL != StreamCombineNo)
            SysEncStreamCombinePtrl.s_StreamCombineNo = atoi(StreamCombineNo);

        Result =  SysSetVideoEncStreamCombine(atoi(SessionId), atoi(AuthValue), &SysEncStreamCombinePtrl);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode);
        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}


GMI_RESULT CgiGetAudioEncCfg(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        uint32_t AudioId = 0;
        uint8_t EncodeType = 0;
        uint16_t CapVolume = 0, PlayVolume = 0;
        Result =  SysGetAudioEncCfg(atoi(SessionId), atoi(AuthValue), &AudioId, &EncodeType, &CapVolume, &PlayVolume);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"EncodeType\":\"%u\",\"CapVolume\":\"%u\",\"PlayVolume\":\"%u\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, EncodeType, CapVolume, PlayVolume);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiSetAudioEncCfg(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *EncodeType = WEB_GET_VAR("EncodeType");
    const char_t *CapVolume = WEB_GET_VAR("CapVolume");
    const char_t *PlayVolume = WEB_GET_VAR("PlayVolume");
    const char_t *RetFormat;
    char_t	Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        uint8_t EncodeTypeValue = 0;
        uint16_t CapVolumeValue = 0;
        uint16_t PlayVolumeValue = 0;

        if (NULL != EncodeType)
            EncodeTypeValue = atoi(EncodeType);
        if (NULL != CapVolume)
            CapVolumeValue = atoi(CapVolume);
        if (NULL != EncodeType)
            PlayVolumeValue = atoi(PlayVolume);

        Result =  SysSetAudioEncCfg(atoi(SessionId), atoi(AuthValue), 1, EncodeTypeValue, CapVolumeValue, PlayVolumeValue);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiGetUpdateCfg(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        int32_t  UpgradePort = 0;
        Result =  GMI_GetUpdatePort(&UpgradePort);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"UpgradePort\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, UpgradePort);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiSystemRebootCmd(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    DaemonData_t DaemonData;

    Result = GMI_DaemonInit( &DaemonData, WEB_SERVER_ID, GMI_DAEMON_HEARDBEAT_SERVER, GMI_DAEMON_HEARDBEAT_WEB);
    if(FAILED(Result))
    {
        RetCode = RETCODE_SYSTEM_RNNING;
        CGI_ERROR("Call SysAuthLogin Error Result = %ld\n",Result);
        RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
        GMI_DaemonUnInit(&DaemonData);
        return GMI_FAIL;
    }

    do
    {
        if  (NULL == SessionId
                || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            CGI_ERROR("CgiSystemRebootCmd	Error \n");
            break;
        }

        Result = GMI_SystemReboot(&DaemonData, GMI_DAEMON_HEARTBEAT_STATUS_QUERY);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            CGI_ERROR("Call GMI_SystemReboot Error Result = %ld\n",Result);
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode);

        GMI_DaemonUnInit(&DaemonData);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "cgiFncCmd=%s&cgiContentType=%s&Content={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, FncCmd, CONTENT_TYPE_JSON, RetCode);
    GMI_DaemonUnInit(&DaemonData);

    return GMI_FAIL;
}

GMI_RESULT CgiFactoryDefault(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }
        Result = FactorySimpleDefaultAllLocal(atoi(SessionId), atoi(AuthValue));
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, Cmd, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiFactoryDefaultAll(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        Result = FactoryDefaultAllLocal(atoi(SessionId), atoi(AuthValue));
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, Cmd, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}


GMI_RESULT CgiSysSearchPtzPresetInfo(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *PresetIndex = WEB_GET_VAR("PresetIndex");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue || NULL == PresetIndex)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        boolean_t Setted = false;
        char_t PresetName[256];
        memset(PresetName, 0, sizeof(PresetName));
        Result = SysSearchPtzPresetInfo(atoi(SessionId), atoi(AuthValue), atoi(PresetIndex), &Setted, PresetName);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"PresetIndex\":\"%d\",\"Setted\":\"%u\",\"PresetName\":\"%s\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, atoi(PresetIndex), Setted, PresetName);
        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}


GMI_RESULT CgiSysGetDeviceStartedTime(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }


        SysPkgSysTime DeviceStartedTimePtr;
        memset(&DeviceStartedTimePtr, 0, sizeof(SysPkgSysTime));
        Result = SysGetDeviceStartedTime(&DeviceStartedTimePtr);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"Year\":\"%d\",\"Month\":\"%d\",\"Day\":\"%d\",\"Hour\":\"%d\",\"Minute\":\"%d\",\"Second\":\"%d\"}";

        fprintf(stdout, RetFormat, Cmd, RetCode, DeviceStartedTimePtr.s_Year, DeviceStartedTimePtr.s_Month, DeviceStartedTimePtr.s_Day, DeviceStartedTimePtr.s_Hour, DeviceStartedTimePtr.s_Minute, DeviceStartedTimePtr.s_Second);
        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}



GMI_RESULT CgiConfigToolGetDeviceInfo(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgDeviceInfo SysDeviceInfo;
        memset(&SysDeviceInfo, 0, sizeof(SysPkgDeviceInfo));
        //Call get System DeviceInfo api
        Result = SysGetDeviceInfo(atoi(SessionId), atoi(AuthValue), &SysDeviceInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgIpInfo SysIpInfo;
        memset(&SysIpInfo, 0,sizeof(SysPkgIpInfo));
        Result = SysGetDeviceIP(atoi(SessionId), atoi(AuthValue), &SysIpInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"DeviceFwVer\":\"%s\",\"DeviceSerialNum\":\"%s\",\"DeviceHwVer\":\"%s\",\"HwAddress\":\"%s\"}";
        fprintf(stdout, RetFormat, Cmd, RetCode, SysDeviceInfo.s_DeviceFwVer, SysDeviceInfo.s_DeviceSerialNum, SysDeviceInfo.s_DeviceHwVer, SysIpInfo.s_HwAddress);
        return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

GMI_RESULT CgiConfigToolSetDeviceInfo(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *DeviceSerialNum = WEB_GET_VAR("DeviceSerialNum");
    const char_t *DeviceHwVer = WEB_GET_VAR("DeviceHwVer");
//    const char_t *HwAddress = WEB_GET_VAR("HwAddress");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        SysPkgDeviceInfo SysDeviceInfo;
        memset(&SysDeviceInfo, 0, sizeof(SysPkgDeviceInfo));
        //Call get System DeviceInfo api
        Result = SysGetDeviceInfo(atoi(SessionId), atoi(AuthValue), &SysDeviceInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        if (NULL != DeviceSerialNum)
            strcpy(SysDeviceInfo.s_DeviceSerialNum, DeviceSerialNum);
        if (NULL != DeviceHwVer)
            strcpy(SysDeviceInfo.s_DeviceHwVer, DeviceHwVer);

        Result = SysSetDeviceInfo(atoi(SessionId), atoi(AuthValue), &SysDeviceInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, Cmd, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiConfigToolSetIrCutStatus(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
//    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }


        RetFormat = "%s={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, Cmd, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}

GMI_RESULT CgiConfigToolGetRtcTime(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }
        SysPkgDateTimeType  SysDateTimeType;
        memset(&SysDateTimeType, 0, sizeof(SysPkgDateTimeType));

        SysPkgSysTime SysSysTime;
        memset(&SysSysTime, 0, sizeof(SysPkgSysTime));

        SysPkgTimeZone SysTimeZone;
        memset(&SysTimeZone, 0, sizeof(SysPkgTimeZone));

        SysPkgNtpServerInfo SysNtpServerInfo;
        memset(&SysNtpServerInfo, 0, sizeof(SysPkgNtpServerInfo));

        Result = SysGetTime(atoi(SessionId), atoi(AuthValue), &SysDateTimeType, &SysSysTime, &SysTimeZone, &SysNtpServerInfo);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"Hour\":\"%d\",\"Minute\":\"%d\",\"Second\":\"%d\"}";
        fprintf(stdout, RetFormat, Cmd, RetCode, SysSysTime.s_Hour, SysSysTime.s_Minute, SysSysTime.s_Second);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiConfigToolTestWatchdog(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        Result = GMI_ConfigToolWatchDogTest(atoi(SessionId), atoi(AuthValue));
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, Cmd, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiConfigToolOpenDcIris(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        Result = GMI_ConfigToolOpenDcIris(atoi(SessionId), atoi(AuthValue));
        if(FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, Cmd, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiConfigToolCloseDcIris(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t	Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        Result = GMI_ConfigToolCloseDcIris(atoi(SessionId), atoi(AuthValue));
        if(FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\"}";
        fprintf(stdout, RetFormat, Cmd, RetCode);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}


GMI_RESULT CgiConfigToolTestAFConfigFile(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        int32_t FileFlags = -1;
        Result = GMI_ConfigToolAfConfigDetect(atoi(SessionId), atoi(AuthValue), &FileFlags);
        if (FAILED(Result))
        {
            RetCode = RETCODE_ERROR;
            break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"FileFlags\":\"%d\"}";
        fprintf(stdout, RetFormat, Cmd, RetCode, FileFlags);
        return GMI_SUCCESS;
    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}


GMI_RESULT CgiConfigToolGetMac(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t	Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;
    
    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    
    do
    {
    	if (NULL == SessionId || NULL == AuthValue)
    	{
    		RetCode = RETCODE_ERROR;
    		break;
    	}
    
	SysPkgIpInfo SysIpInfo;
	memset(&SysIpInfo, 0,sizeof(SysPkgIpInfo));
	Result = SysGetDeviceIP(atoi(SessionId), atoi(AuthValue), &SysIpInfo);
	if (FAILED(Result))
	{
		RetCode = RETCODE_ERROR;
		break;
	}
    
    	RetFormat = "%s={\"RetCode\":\"%d\",\"HwAddress\":\"%s\"}";
    	fprintf(stdout, RetFormat, Cmd, RetCode,SysIpInfo.s_HwAddress);
    	return GMI_SUCCESS;
    } while(0);
    
    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

GMI_RESULT CgiConfigToolSetMac(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *HwAddress = WEB_GET_VAR("HwAddress");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue || NULL==HwAddress)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

	if (NULL != HwAddress)
        {		
             Result = GMI_SetSystemNetworkMac(HwAddress);
             if (FAILED(Result))
             {
                 RetCode = RETCODE_ERROR;
                 break;
             }
         }


	RetFormat = "%s={\"RetCode\":\"%d\"}";
	
	fprintf(stdout, RetFormat, Cmd, RetCode);
	return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}


GMI_RESULT CgiConfigToolIrCutOpen(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

	Result =  GMI_ConfigToolIrCultOpen(atoi(SessionId), atoi(AuthValue));
	if (FAILED(Result))
	{
		RetCode = RETCODE_ERROR;
		break;
	}
	
	RetFormat = "%s={\"RetCode\":\"%d\"}";
	
	fprintf(stdout, RetFormat, Cmd, RetCode);
	return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;


}

GMI_RESULT CgiConfigToolIrCutClose(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;

    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);

    do
    {
        if (NULL == SessionId || NULL == AuthValue)
        {
            RetCode = RETCODE_ERROR;
            break;
        }

	Result =  GMI_ConfigToolIrCultClose(atoi(SessionId), atoi(AuthValue));
	if (FAILED(Result))
	{
		RetCode = RETCODE_ERROR;
		break;
	}
	
	RetFormat = "%s={\"RetCode\":\"%d\"}";
	
	fprintf(stdout, RetFormat, Cmd, RetCode);
	return GMI_SUCCESS;

    } while(0);

    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}


GMI_RESULT CgiConfigToolGetSystemInfo(const char_t *FncCmd)
{

    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *RetFormat;
    char_t	Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;
    
    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    
    do
    {
    	if (NULL == SessionId || NULL == AuthValue)
    	{
    		RetCode = RETCODE_ERROR;
    		break;
    	}

        Result = SysInfoReadInitialize();
	if (FAILED(Result))
	{
		RetCode = RETCODE_ERROR;
		break;
	}
	
	FD_HANDLE Handle;

	Result = SysInfoOpen(CAPABILITY_AUTO_FILE_NAME, &Handle);
	if (FAILED(Result))
	{
		RetCode = RETCODE_ERROR;
		break;
	}

        //read cpu name
        char_t CpuInfo[32] = "0";
        
	Result = SysInfoRead(Handle, HW_AUTO_DETECT_INFO_PATH, HW_CPU_KEY, HW_CPU, CpuInfo);
	if (FAILED(Result))
	{
		SysInfoClose(Handle);
		break;
	}

	//read sensor name
	char_t Sensor[32] = "0";

	Result = SysInfoRead(Handle, HW_AUTO_DETECT_INFO_PATH, HW_SENSOR_KEY, HW_SENSOR, Sensor);
	if (FAILED(Result))
	{
		SysInfoClose(Handle);
		break;
	}

        SysInfoClose(Handle);
	
	Result = SysInfoOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
	if (FAILED(Result))
	{
		RetCode = RETCODE_ERROR;
		break;
	}
		
	//read software version
	char_t SoftwareVersion[64] = "0";
#if 0
	Result = SysInfoRead(Handle, CAPABILITY_SW_MEDIA_PATH, MAX_STREAM_NUM_KEY,  0, &StreamNum);
	if (FAILED(Result))
	{
		SysInfoClose(Handle);
		break;
	}
#endif

	//read hardware version
	char_t HardwareVersion[64] = "0";
	Result = SysInfoRead(Handle, DEVICE_INFO_PATH, DEVICE_HWVER_KEY, "NULL", HardwareVersion);
	if (FAILED(Result))
	{
		SysInfoClose(Handle);
		break;
	}
	
	//read alarm state, true-enable, false-disable
	boolean_t AlarmEnabled = 0;
#if 0
	Result = SysInfoRead(Handle, CAPABILITY_SW_MEDIA_PATH, MAX_STREAM_NUM_KEY,  0, &StreamNum);
	if (FAILED(Result))
	{
		SysInfoClose(Handle);
		break;
	}
#endif

	
	//read language
	char_t Language[32] = "0";
        strcpy(Language, "chinese");
#if 0
	Result = SysInfoRead(Handle, CAPABILITY_SW_MEDIA_PATH, MAX_STREAM_NUM_KEY,	0, &StreamNum);
	if (FAILED(Result))
	{
		SysInfoClose(Handle);
		break;
	}
#endif
	
	//read GB28181 state, true-enable, false-disable
	boolean_t GbEnabled = 0;
#if 0
	Result = SysInfoRead(Handle, CAPABILITY_SW_MEDIA_PATH, MAX_STREAM_NUM_KEY,	0, &StreamNum);
	if (FAILED(Result))
	{
		SysInfoClose(Handle);
		break;
	}
#endif
    
        SysInfoClose(Handle);

	Result = SysInfoOpen(CAPABILITY_SW_FILE_NAME, &Handle);
	if (FAILED(Result))
	{
		RetCode = RETCODE_ERROR;
		break;
	}

	//read max resolution
	int32_t Width = 0;
	Result = SysInfoRead(Handle, CAPABILITY_SW_MEDIA_PATH, MAX_PIC_WIDTH_KEY, 0, &Width);
	if (FAILED(Result))
	{
		SysInfoClose(Handle);
		break;
	}
	
	int32_t Height = 0;
	Result = SysInfoRead(Handle, CAPABILITY_SW_MEDIA_PATH, MAX_PIC_HEIGHT_KEY, 0, &Height);
	if (FAILED(Result))
	{
		SysInfoClose(Handle);
		break;
	}
	
	//read max stream num
		int32_t StreamNum = 0;
	Result = SysInfoRead(Handle, CAPABILITY_SW_MEDIA_PATH, MAX_STREAM_NUM_KEY,	0, &StreamNum);
	if (FAILED(Result))
	{
		SysInfoClose(Handle);
		break;
	}

	SysInfoClose(Handle);


	Result = SysInfoOpen(GMI_FACTORY_SETTING_CONFIG_FILE_NAME, &Handle);
	if (FAILED(Result))
	{
		RetCode = RETCODE_ERROR;
		break;
	}

		//read ircut name
	char_t IRcut[32] = "0";
	Result = SysInfoRead(Handle, VIDEO_IRCUT_NAME_PATH, VIDEO_IRCUT_NAME_KEY, "NULL", IRcut);
	if (FAILED(Result))
	{
		SysInfoClose(Handle);
		break;
	}
	
	//read shield name
	char_t Shield[32] = "0";
	Result = SysInfoRead(Handle, PTZ_SHIELD_NAME_PATH, PTZ_SHIELD_NAME_KEY, "NULL", Shield);
	if (FAILED(Result))
	{
		SysInfoClose(Handle);
		break;
	}
        SysInfoClose(Handle);

	SysInfoReadDeinitialize();

    	RetFormat = "%s={\"RetCode\":\"%d\",\"CpuInfo\":\"%s\",\"Sensor\":\"%s\",\"IRcut\":\"%s\",\"Shield\":\"%s\",\"Width\":\"%d\",\"Height\":\"%d\",\"StreamNum\":\"%d\",\"SoftwareVersion\":\"%s\",\"HardwareVersion\":\"%s\",\"AlarmEnabled\":\"%d\",\"Language\":\"%s\",\"GbEnabled\":\"%d\"}";
    	fprintf(stdout, RetFormat, Cmd, RetCode, CpuInfo, Sensor, IRcut, Shield, Width, Height, StreamNum, SoftwareVersion, HardwareVersion, AlarmEnabled, Language, GbEnabled);
    	return GMI_SUCCESS;
    
    } while(0);
    
    SysInfoReadDeinitialize();
    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;
}


GMI_RESULT CgiSysGetLogInfo(const char_t *FncCmd)
{
    char_t *SessionId = WEB_GET_VAR("SessionId");
    const char_t *AuthValue = WEB_GET_VAR("AuthValue");
    const char_t *SelectMode = WEB_GET_VAR("SelectMode");
    const char_t *MajorType = WEB_GET_VAR("MajorType");
    const char_t *MinorType = WEB_GET_VAR("MinorType");
    const char_t *StartTime = WEB_GET_VAR("StartTime");
    const char_t *StopTime = WEB_GET_VAR("StopTime");
    const char_t *Offset = WEB_GET_VAR("Offset");
    const char_t *MaxNum = WEB_GET_VAR("MaxNum");
    const char_t *RetFormat;
    char_t  Cmd[CMD_BUFFER_LENTH];
    int32_t RetCode = RETCODE_OK;
    GMI_RESULT Result = GMI_FAIL;
    
    sprintf(Cmd, CMD_STRING, FncCmd, CONTENT_TYPE_JSON);
    
    do
    {
        if (NULL == SessionId || NULL == AuthValue           \
            || NULL == SelectMode ||NULL == MajorType    \
            || NULL == MinorType || NULL == StartTime      \
            || NULL == StopTime || NULL == Offset              \
            || NULL == MaxNum)
        {
	     CGI_ERROR("CgiSysGetLogInfo start[%d] \n",__LINE__);
             RetCode = RETCODE_ERROR;
             break;
        }
        
        char_t *Buffer = NULL;
        Buffer = (char_t*)malloc(atoi(MaxNum) * sizeof(SysPkgShowCfg));
        if(NULL == Buffer)
        {
            RetCode = RETCODE_ERROR;
            Buffer = NULL;
	    CGI_ERROR("CgiSysGetLogInfo start[%d] \n",__LINE__);
            break;
        }

        memset(Buffer, 0, atoi(MaxNum)*sizeof(SysPkgShowCfg));
        SysPkgLogInfo *SysLogInfo;
        SysLogInfo = (SysPkgLogInfo *)Buffer;

        SysPkgLogInfoSearch SysLogInfoSearch;
        SysPkgLogInfoInt SysLogInfoInt;
        memset(&SysLogInfoSearch, 0, sizeof(SysPkgLogInfoSearch));
        memset(&SysLogInfoInt, 0, sizeof(SysPkgLogInfoInt));	

	SysLogInfoSearch.s_SelectMode = atoi(SelectMode);
	SysLogInfoSearch.s_MajorType = atoi(MajorType);
        SysLogInfoSearch.s_MinorType = atoi(MinorType);
        strcpy(SysLogInfoSearch.s_StartTime, StartTime);
        strcpy(SysLogInfoSearch.s_StopTime, StopTime);
	SysLogInfoSearch.s_Offset = atoi(Offset);
        SysLogInfoSearch.s_MaxNum = atoi(MaxNum);

        Result = SysGetLogInfo(atoi(SessionId), atoi(AuthValue), &SysLogInfoSearch, &SysLogInfoInt, SysLogInfo);
        if (FAILED(Result))
        {
             RetCode = RETCODE_ERROR;
             if(NULL != Buffer)
             {
            	 free(Buffer);
            	 Buffer = NULL;
             }
             break;
        }

        RetFormat = "%s={\"RetCode\":\"%d\",\"Total\":\"%d\",\"Count\":\"%d\"%s}";

        int32_t Length = 0, i = 0;
        char_t TmpBuf[16384];
        memset(TmpBuf, 0, 16384);
        for (i=0; i < SysLogInfoInt.s_Count; i++)
        {
            Length += sprintf(TmpBuf+Length,",\"LogId%d\":\"%llu\",\"MajorType%d\":\"%d\",\"MinorType%d\":\"%d\""
                             ",\"LogTime%d\":\"%s\",\"UserName%d\":\"%s\",\"RemoteHostIp%d\":\"%s \",\"LogData%d\":\"%s\""
                             ,i , SysLogInfo[i].s_LogId, i , SysLogInfo[i].s_MajorType, i ,SysLogInfo[i].s_MinorType ,i , SysLogInfo[i].s_LogTime               \
                             ,i , SysLogInfo[i].s_UserName, i, SysLogInfo[i].s_RemoteHostIp,i , SysLogInfo[i].s_LogData);
        }

        fprintf(stdout, RetFormat, Cmd, RetCode, SysLogInfoInt.s_Total, SysLogInfoInt.s_Count ,TmpBuf);

        if(NULL != Buffer)
        {
            free(Buffer);
            Buffer = NULL;
        }

        return GMI_SUCCESS;      
    }while(0);
    
    RetFormat = "%s={\"RetCode\":\"%d\"}";
    fprintf(stdout, RetFormat, Cmd, RetCode);
    return GMI_FAIL;

}

