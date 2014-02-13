#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "log.h"
#include "service_utilitly.h"
#include "stdsoap2.h"
#if 0
ONVIF_Wsdl g_WsdlArry[] =
{
    { DEVICE_WSDL_ID, "http://www.onvif.org/ver10/device/wsdl", "onvif/device_service",  1,  5 },
    { MEDIA_WSDL_ID, "http://www.onvif.org/ver10/media/wsdl", "onvif/Media", 1, 5 },
    { PTZ_WSDL_ID, "http://www.onvif.org/ver20/ptz/wsdl", "onvif/PTZ", 2, 4 },
    { EVENTS_WSDL_ID, "http://www.onvif.org/ver10/events/wsdl", "onvif/Events", 1, 5 },
    { DEVICE_IO_WSDL_ID, "http://www.onvif.org/ver10/deviceIO/wsdl", "onvif/DeviceIO", 1, 4 },
    { IMAGING_WSDL_ID, "http://www.onvif.org/ver20/imaging/wsdl", "onvif/Imaging", 2, 3 },
    { ANALYTICS_WSDL_ID, "http://www.onvif.org/ver20/analytics/wsdl", "onvif/Analytics", 2, 3 },
    { REMOTE_DISCOVERY_WSDL_ID, "http://www.onvif.org/onvif/ver10/network/wsdl", "onvif/remote_discovery", 1, 1},
    /*
       { ANALYTICS_DEVICE_WSDL_ID, "http://www.onvif.org/ver10/analyticsdevice/wsdl", "analytics_device", 1, 1 },
       { DISPLAY_WSDL_ID, "http://www.onvif.org/ver10/display/wsdl", "Display", 1, 1 },
       { RECEIVER_WSDL_ID, "http://www.onvif.org/ver10/receiver/wsdl", "Receiver", 1, 2 }
       { RECORDING_CTRL_WSDL_ID, "http://www.onvif.org/ver10/recording/wsdl", "recording_ctrl", 1, 4 },
       { RECORDING_SEARCH_WSDL_ID, "http://www.onvif.org/ver10/search/wsdl", "recording_search", 1, 3 },
       { REPLAY_WSDL_ID, "http://www.onvif.org/ver10/replay/wsdl", "Replay", 1, 5 },
    */
};
#else
ONVIF_Wsdl g_WsdlArry[] =
{
    { DEVICE_WSDL_ID, "http://www.onvif.org/ver10/device/wsdl", "onvif/device_service",  1,  5 },
    { MEDIA_WSDL_ID, "http://www.onvif.org/ver10/media/wsdl", "onvif/media_service", 1, 5 },
    { PTZ_WSDL_ID, "http://www.onvif.org/ver20/ptz/wsdl", "onvif/ptz_service", 2, 4 },
    { EVENTS_WSDL_ID, "http://www.onvif.org/ver10/events/wsdl", "onvif/event_service", 1, 5 },
    { DEVICE_IO_WSDL_ID, "http://www.onvif.org/ver10/deviceIO/wsdl", "onvif/DeviceIO", 1, 4 },
    { IMAGING_WSDL_ID, "http://www.onvif.org/ver20/imaging/wsdl", "onvif/imaging_service", 2, 3 },
    /*    { ANALYTICS_WSDL_ID, "http://www.onvif.org/ver20/analytics/wsdl", "onvif/analytics_service", 2, 3 },
        { REMOTE_DISCOVERY_WSDL_ID, "http://www.onvif.org/onvif/ver10/network/wsdl", "onvif/remote_discovery", 1, 1},
        { ANALYTICS_DEVICE_WSDL_ID, "http://www.onvif.org/ver10/analyticsdevice/wsdl", "analytics_device", 1, 1 },
        { DISPLAY_WSDL_ID, "http://www.onvif.org/ver10/display/wsdl", "Display", 1, 1 },
        { RECEIVER_WSDL_ID, "http://www.onvif.org/ver10/receiver/wsdl", "Receiver", 1, 2 }
        { RECORDING_CTRL_WSDL_ID, "http://www.onvif.org/ver10/recording/wsdl", "recording_ctrl", 1, 4 },
        { RECORDING_SEARCH_WSDL_ID, "http://www.onvif.org/ver10/search/wsdl", "recording_search", 1, 3 },
        { REPLAY_WSDL_ID, "http://www.onvif.org/ver10/replay/wsdl", "Replay", 1, 5 },
     */
};
#endif
uint32_t g_WsdlCount = sizeof(g_WsdlArry) / sizeof(g_WsdlArry[0]);


void* soap_malloc_zero(struct soap *p_soap, size_t n)
{
    register void *p = NULL;
    p = soap_malloc(p_soap, n);
    if (NULL == p)
    {
        return NULL;
    }

    memset(p, 0, n);

    return p;
}


int GetNetAddress( char_t *EthName, uint8_t *Mac, char_t *Ip)
{
    int32_t Sock;
    struct sockaddr_in Sin;
    struct ifreq Ifr;

    Sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (Sock == -1)
    {
        perror("socket");
        return -1;
    }

    strncpy(Ifr.ifr_name, EthName, IFNAMSIZ);
    Ifr.ifr_name[IFNAMSIZ - 1] = 0;

    if (ioctl(Sock, SIOCGIFADDR, &Ifr) < 0)
    {
        perror("ioctl");
        return -1;
    }

    memcpy(&Sin, &Ifr.ifr_addr, sizeof(Sin));
    printf("%s: %s", EthName, inet_ntoa(Sin.sin_addr));

    if (ioctl(Sock, SIOCGIFHWADDR, &Ifr) < 0)
    {
        perror("ioctl");
        return GMI_FAIL;
    }

    printf("adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x", \
           (unsigned char)Ifr.ifr_hwaddr.sa_data[0], (unsigned char)Ifr.ifr_hwaddr.sa_data[1], (unsigned char)Ifr.ifr_hwaddr.sa_data[2], (unsigned char)Ifr.ifr_hwaddr.sa_data[3], (unsigned char)Ifr.ifr_hwaddr.sa_data[4], (unsigned char)Ifr.ifr_hwaddr.sa_data[5]);

    if (Ip != NULL)
    {
        strcpy(Ip, inet_ntoa(Sin.sin_addr));
    }

    if (Mac != NULL)
    {
        Mac[0] = Ifr.ifr_hwaddr.sa_data[0];
        Mac[1] = Ifr.ifr_hwaddr.sa_data[1];
        Mac[2] = Ifr.ifr_hwaddr.sa_data[2];
        Mac[3] = Ifr.ifr_hwaddr.sa_data[3];
        Mac[4] = Ifr.ifr_hwaddr.sa_data[4];
        Mac[5] = Ifr.ifr_hwaddr.sa_data[5];
    }

    close(Sock);

    return 0;
}


GMI_RESULT NET_GetIpInfo(char_t *EthName, char_t *IP)
{
    int32_t Sock;
    struct sockaddr_in Sin;
    struct ifreq Ifr;

    if (NULL == IP)
    {
        return GMI_INVALID_PARAMETER;
    }

    Sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (Sock < 0)
    {
        perror("socket");
        return GMI_FAIL;
    }

    strncpy(Ifr.ifr_name, EthName, IFNAMSIZ);
    Ifr.ifr_name[IFNAMSIZ - 1] = 0;

    if (ioctl(Sock, SIOCGIFADDR, &Ifr) < 0)
    {
        perror("ioctl");
        return GMI_FAIL;
    }

    memcpy(&Sin, &Ifr.ifr_addr, sizeof(Sin));
    strcpy(IP, inet_ntoa(Sin.sin_addr));

    close(Sock);

    return GMI_SUCCESS;
}


GMI_RESULT NET_GetMacInfo(char_t *EthName, char_t *Mac)
{
    int32_t            Sock;
    struct sockaddr_in Sin;
    struct ifreq       Ifr;

    if (NULL == Mac)
    {
        return GMI_INVALID_PARAMETER;
    }

    Sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (Sock < 0)
    {
        perror("socket");
        return GMI_FAIL;
    }

    strncpy(Ifr.ifr_name, EthName, IFNAMSIZ);
    Ifr.ifr_name[IFNAMSIZ - 1] = 0;

    memcpy(&Sin, &Ifr.ifr_addr, sizeof(Sin));
    if (ioctl(Sock, SIOCGIFHWADDR, &Ifr) < 0)
    {
        perror("ioctl");
        return GMI_FAIL;
    }

    sprintf(Mac, "%02x:%02x:%02x:%02x:%02x:%02x", \
            (uint8_t)Ifr.ifr_hwaddr.sa_data[0], (uint8_t)Ifr.ifr_hwaddr.sa_data[1], (uint8_t)Ifr.ifr_hwaddr.sa_data[2], (uint8_t)Ifr.ifr_hwaddr.sa_data[3], (uint8_t)Ifr.ifr_hwaddr.sa_data[4], (uint8_t)Ifr.ifr_hwaddr.sa_data[5]);

    //Mac[0] = Ifr.ifr_hwaddr.sa_data[0];
    //Mac[1] = Ifr.ifr_hwaddr.sa_data[1];
    //Mac[2] = Ifr.ifr_hwaddr.sa_data[2];
    //Mac[3] = Ifr.ifr_hwaddr.sa_data[3];
    //Mac[4] = Ifr.ifr_hwaddr.sa_data[4];
    //Mac[5] = Ifr.ifr_hwaddr.sa_data[5];

    close(Sock);

    return GMI_SUCCESS;
}


GMI_RESULT NET_CheckIPString(char_t *Str)
{
    uint8_t  a;
    int32_t  dot = 0;
    int32_t  a3,a2,a1,a0,i;

    if (strlen(Str) == 0)
    {
        return GMI_FAIL;
    }

    i = 0;
    while ((a = Str[i++]) != '\0')
    {
        if ((a == ' ')
                || (a == '.')
                || ((a >= '0') && ( a <= '9')))
        {
            if (a == '.')
            {
                dot++;
            }
        }
        else
        {
            return GMI_FAIL;
        }
    }

    if (dot != 3)
    {
        return GMI_FAIL;
    }
    else
    {
        sscanf(Str, "%d.%d.%d.%d", &a3,&a2,&a1,&a0);
        if ((a0 >= 255)
                ||(a1 > 255)
                ||(a2 > 255)
                ||(a3 > 255) )
        {
            return GMI_FAIL;
        }
    }

    return GMI_SUCCESS;
}


GMI_RESULT Soap_WSSE_Authentication(struct soap *soap_ptr)
{   
    char_t  PassWord[128];

    if ((NULL != soap_ptr->header)
            && (NULL != soap_ptr->header->wsse__Security)
            && (NULL != soap_ptr->header->wsse__Security->UsernameToken)
            && (NULL != soap_ptr->header->wsse__Security->UsernameToken->Username)
            && (NULL != soap_ptr->header->wsse__Security->UsernameToken->Password)
            && (0 < strlen(soap_ptr->header->wsse__Security->UsernameToken->Username)))
    {    	
        if (0 == strcmp(soap_ptr->header->wsse__Security->UsernameToken->Username,  "admin")
        	|| 0 == strcmp(soap_ptr->header->wsse__Security->UsernameToken->Username,  "inc"))
        {
            memcpy(PassWord, "admin", sizeof(PassWord));
        }
        else
        {        	
            ONVIF_ERROR("Username %s mismatch.........\n", soap_ptr->header->wsse__Security->UsernameToken->Username);
            goto errExit;
        }

        //should send user information to Authencation Center,
        //then get Authencation Result.
        //if (SOAP_OK != soap_wsse_verify_Password(soap_ptr, PassWord))
        //{
        //	ONVIF_ERROR("password verify error.........\n");
        //	goto errExit;
        //}
    }
    
    return GMI_SUCCESS;
errExit:   
    return GMI_FAIL;
}


GMI_RESULT ONVIF_Fault(struct soap *soap_ptr, const char *Value1, const char *Value2, const char *Reason)
{
    //fault code
    soap_ptr->fault = (struct SOAP_ENV__Fault*)soap_malloc_zero(soap_ptr, (sizeof(struct SOAP_ENV__Fault)));
    soap_ptr->fault->SOAP_ENV__Code = (struct SOAP_ENV__Code*)soap_malloc_zero(soap_ptr, (sizeof(struct SOAP_ENV__Code)));
    if (NULL == soap_ptr->fault->SOAP_ENV__Code)
    {
        return GMI_OUT_OF_MEMORY;
    }
    soap_ptr->fault->SOAP_ENV__Code->SOAP_ENV__Value = soap_strdup(soap_ptr, "SOAP-ENV:Sender");
    //fault subcode
    soap_ptr->fault->SOAP_ENV__Code->SOAP_ENV__Subcode = (struct SOAP_ENV__Code*)soap_malloc_zero(soap_ptr, (sizeof(struct SOAP_ENV__Code)));
    if (NULL == soap_ptr->fault->SOAP_ENV__Code->SOAP_ENV__Subcode)
    {
        return GMI_OUT_OF_MEMORY;
    }
    soap_ptr->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Value = soap_strdup(soap_ptr, Value1);
    //fault subcode subcode
    if (NULL != Value2)
    {
        soap_ptr->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode = (struct SOAP_ENV__Code*)soap_malloc_zero(soap_ptr, (sizeof(struct SOAP_ENV__Code)));
        if (NULL == soap_ptr->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode)
        {
            return GMI_OUT_OF_MEMORY;
        }
        soap_ptr->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode->SOAP_ENV__Value   = soap_strdup(soap_ptr, Value2);
        soap_ptr->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode->SOAP_ENV__Subcode = NULL;
    }
    else
    {
        soap_ptr->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode = NULL;
    }

    soap_ptr->fault->faultcode = NULL;
    soap_ptr->fault->faultstring = NULL;
    soap_ptr->fault->faultactor = NULL;
    soap_ptr->fault->detail = NULL;
    //fault reason
    if (NULL != Reason)
    {
        soap_ptr->fault->SOAP_ENV__Reason = (struct SOAP_ENV__Reason *)soap_malloc_zero(soap_ptr, sizeof(struct SOAP_ENV__Reason));
        if (NULL == soap_ptr->fault->SOAP_ENV__Reason)
        {
            return GMI_OUT_OF_MEMORY;
        }
        soap_ptr->fault->SOAP_ENV__Reason->SOAP_ENV__Text = soap_strdup(soap_ptr, Reason);
    }
    else
    {
        soap_ptr->fault->SOAP_ENV__Reason = NULL;
    }
    soap_ptr->fault->SOAP_ENV__Node = NULL;
    soap_ptr->fault->SOAP_ENV__Role = NULL;
    soap_ptr->fault->SOAP_ENV__Detail = NULL;

    return GMI_SUCCESS;
}


GMI_RESULT NET_CheckHostname (char_t *Str)
{
    if (Str == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    while (*Str != '\0')
    {
        if ((*Str >= 'a' && *Str <= 'z') || (*Str >= 'A' && *Str <= 'Z') || (*Str >= '0' && *Str <= '9') || (*Str == '.') || (*Str == '-') )
        {
            Str++;
        }
        else
        {
            return GMI_FAIL;
        }
    }
    return GMI_SUCCESS;
}



GMI_RESULT IsNum(char_t *Str)
{
    char_t   *StrTmpPtr = NULL;
    uint32_t Len = 0;

    if (Str == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    Len = strlen(Str);
    StrTmpPtr = Str;
    ONVIF_INFO("IsNum Len %d\n", Len);
    while (Len--)
    {
        if (*StrTmpPtr > '9'
                || *StrTmpPtr < '0')
        {
            return GMI_FAIL;
        }

        StrTmpPtr++;
        ONVIF_INFO("IsNum exe\n");
    }

    return GMI_SUCCESS;
}
