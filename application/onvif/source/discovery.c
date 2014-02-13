
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <string.h>
#include <sys/ioctl.h>
#include "service_utilitly.h"


#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define sleep(x) Sleep((x) * 1000)
#define close(x) closesocket(x)
#define socklen_t int
#undef errno
#define errno WSAGetLastError()
#else /* !_WIN32 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
typedef int SOCKET;
#endif /* _WIN32 */
#include "stdsoap2.h"
#include "gmi_system_headers.h"
#include "threads.h"

#include "util.h"
#include "uuid.h"
#include "discovery.h"
#include "log.h"

#ifndef SD_RECEIVE
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02
#endif

typedef struct tagScopes
{
    tt__ScopeDefinition s_ScopeDefinitionFlag;
    char_t s_ScopeUrl[64];
} DevScopes;

#define DEFAULT_VALID_SCOPE_NUM    12
#define DEFAULT_USED_SCOPE_NUM     6

#define ONVIF_MSG_BUF_LEN 4096
static boolean_t l_ProbeServerStop = false;
static char_t    l_DeviceUuid[100] = {0};
extern uint16_t  g_ONVIF_Port;
extern uint16_t  g_RTSP_Port;
extern int32_t   g_ChangeScopes;
extern enum tt__DiscoveryMode g_DiscoveryMode;
extern enum xsd__boolean      g_LocalAddressFlag;
extern enum xsd__boolean      g_DhcpFlag;
extern DevScopes g_DeviceScopes[DEFAULT_VALID_SCOPE_NUM];

//func delcare
static void *DevProbeService();

void GetTmpValueAfterReboot()
{
    FILE *fp = NULL;
    char str[128];
    char head[20];
    int scopesNum = 0;
    fp = fopen("/usr/local/bin/onvifTmpFile", "rb+");
    if (fp)
    {
        while (1)
        {
            bzero(str, sizeof(str));
            bzero(head, sizeof(head));
            if (fgets(str, sizeof(str), fp) == 0 )
                break;
            if (strlen(str) == 0)
                continue;
            if (sscanf(str, "%s", head) == 0)
                continue;
            if (strcmp(head , "DiscoveryMode") == 0 )
            {
                sscanf(str , "%*s %d" , (int*)&g_DiscoveryMode);
            }
            else if (strcmp(head , "LocalAddressFlag") == 0 )
            {
                sscanf(str , "%*s %d" , (int*)&g_LocalAddressFlag);
            }
            else if (strcmp(head , "DhcpFlag") == 0 )
            {
                sscanf(str , "%*s %d" , (int*)&g_DhcpFlag);
            }
            else if (strcmp(head , "ChangeScopes") == 0 )
            {
                sscanf(str , "%*s %d" , &g_ChangeScopes);
            }
            else if (strcmp(head , "ScopeUrl") == 0 )
            {
                sscanf(str , "%*s %s" , g_DeviceScopes[scopesNum+6].s_ScopeUrl);
                scopesNum++;
            }
        }

        fclose(fp);
        fp = NULL;
    }
}


GMI_RESULT OnvifDevProbeServiceStart(void)
{
    THREAD_TYPE TidProbe;

    l_ProbeServerStop = false;
    THREAD_CREATE(&TidProbe, (void*(*)(void*))DevProbeService, (void*)NULL);

    return GMI_SUCCESS;
}


GMI_RESULT OnvifDevProbeServiceStop()
{
    l_ProbeServerStop = true;

    return GMI_SUCCESS;
}


static GMI_RESULT CreateSsdpSockV4(int isbind, SOCKET *ssdpSock)
{
    long_t onOff;
    long_t ret = 0;
    char_t ttl = 4;
    struct in_addr addr;
    struct sockaddr_storage __ss;
    struct sockaddr_in *ssdpAddr4 = (struct sockaddr_in *)&__ss;
    struct ip_mreq ssdpMcastAddr;
    struct linger tcpLinger;

    *ssdpSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (*ssdpSock == -1)
    {
        printf("Error in socket(): %d\n", errno);
        return GMI_FAIL;
    }

    onOff = 1;
    ret = setsockopt(*ssdpSock, SOL_SOCKET, SO_REUSEADDR, (char *)&onOff, sizeof(onOff));
    if (ret == -1)
    {
        printf("Error in setsockopt() SO_REUSEADDR: %d\n", errno);
        goto errExit;
    }

    if (isbind)
    {
        memset(&__ss, 0, sizeof(__ss));
        ssdpAddr4->sin_family = AF_INET;
        ssdpAddr4->sin_addr.s_addr = htonl(INADDR_ANY);
        ssdpAddr4->sin_port = htons(3702);

        ret = bind(*ssdpSock, (struct sockaddr *)ssdpAddr4, sizeof(*ssdpAddr4));
        if (ret == -1)
        {
            printf("Error in bind(), addr=0x%08X\n", INADDR_ANY);
            goto errExit;
        }

        memset( ( void * )&ssdpMcastAddr, 0, sizeof( struct ip_mreq ) );
        ssdpMcastAddr.imr_interface.s_addr = htonl(INADDR_ANY);
        ssdpMcastAddr.imr_multiaddr.s_addr = inet_addr( "239.255.255.250" );

        ret = setsockopt(*ssdpSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&ssdpMcastAddr, sizeof(struct ip_mreq));
        if (ret == -1)
        {
            printf("Error in setsockopt() IP_ADD_MEMBERSHIP (join multicast group): %d\n", errno);
            goto errExit;
        }

        memset((void *)&addr, 0, sizeof(struct in_addr));
        addr.s_addr = htonl(INADDR_ANY);
        ret = setsockopt(*ssdpSock, IPPROTO_IP, IP_MULTICAST_IF, (char *)&addr, sizeof addr);
        if (ret == -1)
        {
            // This is probably not a critical error, so let's continue
        }

        ret = setsockopt(*ssdpSock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

        onOff = 1;
        ret = setsockopt(*ssdpSock, SOL_SOCKET, SO_BROADCAST, (char *)&onOff, sizeof(onOff));
        if (ret == -1)
        {
            printf("Error in setsockopt() SO_BROADCAST (set broadcast): %d\n", errno);
            goto errExit;
        }

        tcpLinger.l_onoff = 1;
        tcpLinger.l_linger = 1;
        ret = setsockopt(*ssdpSock, SOL_SOCKET, SO_LINGER, (char*)&tcpLinger, sizeof(struct linger));
        if (ret == -1)
        {
            printf("set socket LINGER failed !! errno = %d\n", errno);
            goto errExit;

        }

    }//endif if (isbind)

    return GMI_SUCCESS;

errExit:
    shutdown(*ssdpSock, SD_BOTH);
    close(*ssdpSock);

    return GMI_FAIL;
}


static void *DevProbeService()
{
    long_t     Ret;
    GMI_RESULT Result;
    ulong_t ClientPort   = 0;
    char_t ClientIp[20]  = {0};
    char_t *MsgBuf       = NULL;
    char_t HostIp[20]    = {0};
    char_t *MsgPtr1      = NULL;
    char_t *MsgPtr2      = NULL;
    char_t UuidAddr[100] = {0};
    char_t UuidTo[100]   = {0};
    const char *format = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" xmlns:wsd=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\"><SOAP-ENV:Header><wsa:To>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</wsa:To><wsa:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches</wsa:Action><wsa:MessageID>urn:uuid:%s</wsa:MessageID><wsa:RelatesTo>%s</wsa:RelatesTo><wsd:AppSequence MessageNumber=\"%d\" InstanceId=\"2\"/></SOAP-ENV:Header><SOAP-ENV:Body><wsd:ProbeMatches><wsd:ProbeMatch><wsa:EndpointReference><wsa:Address>urn:uuid:%s</wsa:Address></wsa:EndpointReference><wsd:Types>dn:NetworkVideoTransmitter</wsd:Types><wsd:Scopes>onvif://www.onvif.org/type/video_encoder onvif://www.onvif.org/type/ptz onvif://www.onvif.org/type/audio_encoder onvif://www.onvif.org/hardware/IPCamera onvif://www.onvif.org/location/country/china onvif://www.onvif.org/name/IPC</wsd:Scopes><wsd:XAddrs>http://%s:%d/onvif/device_service</wsd:XAddrs><wsd:MetadataVersion>1</wsd:MetadataVersion></wsd:ProbeMatch></wsd:ProbeMatches></SOAP-ENV:Body></SOAP-ENV:Envelope>";
    SOCKET sockfd;
    socklen_t AddrLen;
    fd_set FdSet;
    uuid_upnp uuid;
    struct sockaddr_in RemoteAddr;
    struct timeval TimeOut;
    char_t MessageId[100] = {0};
    char_t MacAddress[64] = {0};
    char_t EthAddr[] = "eth0";
    uuid_upnp NameSpace_MAC =   /* 6ba7b810-9dad-11d1-80b4-00c04fd430c8 */
    {
        0x6ba7b810,
        0x9dad,
        0x11d1,
        0x80, 0xb4, {0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8}
    };

    pthread_detach(pthread_self());

    Ret = CreateSsdpSockV4(1, &sockfd);
    if (GMI_FAIL == Ret)
    {
        printf("create ssdp socket failed, errno %ld\n", Ret);
        goto errExit;
    }

    MsgBuf = (char_t *)malloc(ONVIF_MSG_BUF_LEN * sizeof(char_t));
    if (NULL == MsgBuf)
    {
        fprintf(stderr, "malloc msgbuf failed!\n");
        goto errExit;
    }

    uuid_create(&uuid);
    uuid_unpack(&uuid, MessageId);

    Result = NET_GetMacInfo(EthAddr, MacAddress);
    if (FAILED(Result))
    {
        memcpy(UuidAddr, "6ba7b810-9dad-11d1-80b4-00c04fd430c8", strlen("6ba7b810-9dad-11d1-80b4-00c04fd430c8"));
    }
    else
    {
        memset(&uuid, 0, sizeof(uuid));
        uuid_create(&uuid);
        uuid_create_from_name(&uuid, NameSpace_MAC, (void*)MacAddress, strlen(MacAddress));
        uuid_unpack(&uuid, UuidAddr);
    }

    GetTmpValueAfterReboot();
    memset(l_DeviceUuid, 0, sizeof(l_DeviceUuid));
    memcpy(l_DeviceUuid, UuidAddr, strlen(UuidAddr));
    if (tt__DiscoveryMode__Discoverable == g_DiscoveryMode)
    {
        DevStatusVaryNotifyService(TYPE_HELLO);
    }

    ONVIF_INFO("uuidaddr of device = %s \n", UuidAddr);

    while (l_ProbeServerStop != true)
    {
        FD_ZERO(&FdSet);
        FD_SET(sockfd, &FdSet);

        TimeOut.tv_sec	= 1;
        TimeOut.tv_usec	= 0;
        Ret = select(sockfd+1, &FdSet, NULL, NULL, &TimeOut);
        if (Ret <= 0)
        {
            usleep(10000);
            continue;
        }
        if (FD_ISSET(sockfd,&FdSet) > 0)
        {
            memset(MsgBuf, 0, ONVIF_MSG_BUF_LEN);
            memset(&RemoteAddr, 0, sizeof(RemoteAddr));
            memset(HostIp, 0, sizeof(HostIp));
            memset(ClientIp, 0, sizeof(ClientIp));

            AddrLen = sizeof(struct sockaddr);
            Ret = recvfrom(sockfd, MsgBuf, ONVIF_MSG_BUF_LEN - 1, 0, (struct sockaddr *)&RemoteAddr, &AddrLen);
            if (Ret <= 0)
            {
                continue;
            }

            if (NULL == strstr(MsgBuf, "Probe>"))
            {
                continue;
            }

            if (NULL == (MsgPtr1 = strstr(MsgBuf, "MessageID")))
            {
                printf("probe msg has no MessageID token................\n");
                continue;
            }
            MsgPtr1 += strlen("MessageID");
            if (NULL == (MsgPtr1 = strchr(MsgPtr1, '>')))
            {
                printf("probe msg is invalid, messageid token 1 error..............................\n");
                continue;
            }
            MsgPtr1 += 1;
            if (NULL == (MsgPtr2 = strchr(MsgPtr1, '<')))
            {
                printf("probe msg is invalid, message token 2 error..............................\n");
                continue;
            }

            memset(UuidTo, 0, sizeof(UuidTo));
            memcpy(UuidTo, MsgPtr1, MsgPtr2 - MsgPtr1);
            printf("uuidto_client=%s.......................\n", UuidTo);

            ClientPort = ntohs(RemoteAddr.sin_port);
            memcpy(ClientIp, inet_ntoa(RemoteAddr.sin_addr), sizeof(ClientIp) - 1);
            if (0 == strcmp("0.0.0.0", ClientIp))
            {
                printf("client ip(0.0.0.0) is invalid..................\n");
                continue;
            }

            printf("send probe response, ip=%s, port=%ldu\n", ClientIp, ClientPort);

            memset(MsgBuf, 0, ONVIF_MSG_BUF_LEN);
            get_hostip_by_ip(ClientIp, HostIp);
            snprintf(MsgBuf, ONVIF_MSG_BUF_LEN - 1, format, UuidAddr, UuidTo, time(NULL), UuidAddr, HostIp, g_ONVIF_Port);
            sendto(sockfd, (char *)MsgBuf, strlen(MsgBuf), 0, (struct sockaddr *)&RemoteAddr, sizeof(RemoteAddr));
        }
    }

errExit:
    if (MsgBuf)
    {
        free(MsgBuf);
    }

    if (sockfd != -1)
    {
        close(sockfd);
        sockfd = -1;
    }

    pthread_exit(NULL);
}


GMI_RESULT DevStatusVaryNotifyService(int NotifyType)
{
    char_t *sendBuf = NULL;
    char_t HostIP[64] = {0};
    char_t TmpBuf[512] = {0};
    SOCKET sockfd = -1;
    int ret = -1;
    int option = 0;
    int randomDelayTime = 0;
    int n_i = 0;
    int32_t OnvifPort = g_ONVIF_Port;
    struct ifreq ifr;
    struct sockaddr_in localSockAddr;
    struct sockaddr_in remoteSockAddr;
    const char *helloFormat = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n <SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" xmlns:wsd=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\" > <SOAP-ENV:Header><wsa:MessageID>urn:uuid:%s</wsa:MessageID><wsa:To>urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To><wsa:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/Hello</wsa:Action></SOAP-ENV:Header><SOAP-ENV:Body><wsd:Hello><wsa:EndpointReference><wsa:Address>urn:uuid:%s</wsa:Address></wsa:EndpointReference><wsd:Types>dn:NetworkVideoTransmitter</wsd:Types><wsd:Scopes>onvif://www.onvif.org/type/video_encoder onvif://www.onvif.org/type/ptz onvif://www.onvif.org/type/audio_encoder onvif://www.onvif.org/hardware/IPCamera onvif://www.onvif.org/location/country/china onvif://www.onvif.org/name/IPC %s</wsd:Scopes><wsd:XAddrs>http://%s:%d/onvif/device_service</wsd:XAddrs><wsd:MetadataVersion>9</wsd:MetadataVersion></wsd:Hello></SOAP-ENV:Body></SOAP-ENV:Envelope>";
    const char *byeFormat = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n <SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" xmlns:wsd=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\"> <SOAP-ENV:Header><wsa:MessageID>urn:uuid:%s</wsa:MessageID><wsa:To>urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To><wsa:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/Bye</wsa:Action></SOAP-ENV:Header><SOAP-ENV:Body><wsd:Bye><wsa:EndpointReference><wsa:Address>urn:uuid:%s</wsa:Address></wsa:EndpointReference><wsd:Types>dn:NetworkVideoTransmitter</wsd:Types><wsd:Scopes>onvif://www.onvif.org/type/video_encoder onvif://www.onvif.org/type/ptz onvif://www.onvif.org/type/audio_encoder onvif://www.onvif.org/hardware/IPCamera onvif://www.onvif.org/location/country/china onvif://www.onvif.org/name/IPC</wsd:Scopes><wsd:XAddrs>http://%s:%d/onvif/device_service</wsd:XAddrs><wsd:MetadataVersion>9</wsd:MetadataVersion></wsd:Bye></SOAP-ENV:Body></SOAP-ENV:Envelope>";

    ONVIF_INFO("%s in.........\n", __func__);
    sendBuf = (char_t *)malloc(ONVIF_MSG_BUF_LEN * sizeof(char_t));
    if (NULL == sendBuf)
    {
        fprintf(stderr, "malloc sendBuf failed!\n");
        goto errExit;
    }
    memset(sendBuf, 0, (ONVIF_MSG_BUF_LEN * sizeof(char_t)));

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        fprintf(stderr, "DevStatusVaryNotifyService error in socket(): %d\n", errno);
        goto errExit;
    }

    memset(HostIP, 0, sizeof(HostIP));
    strcpy(ifr.ifr_name, "eth0");
    if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0)
    {
        fprintf(stderr, "DevStatusVaryNotifyService error in ioctl(): %d\n", errno);
        goto errExit;
    }
    sprintf(HostIP, "%s", inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));

    option = 1;
    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(option));
    if(ret < 0)
    {
        fprintf(stderr, "DevStatusVaryNotifyService error in setsockopt SO_REUSEADDR: %d\n", errno);
        goto errExit;
    }

    option = 1;
    ret = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char *)&option, sizeof(option));
    if(ret < 0)
    {
        fprintf(stderr, "DevStatusVaryNotifyService error in setsockopt SO_BROADCAST: %d\n", errno);
        goto errExit;
    }

    memset(&localSockAddr, 0, sizeof(localSockAddr));
    localSockAddr.sin_family = AF_INET;
    localSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(TYPE_HELLO == NotifyType)
    {
        localSockAddr.sin_port = htons(3699);
    }
    else
    {
        localSockAddr.sin_port = htons(3700);
    }

    ret = bind(sockfd, (struct sockaddr *)&localSockAddr, sizeof(localSockAddr));
    if(ret < 0)
    {
        fprintf(stderr, "DevStatusVaryNotifyService error in bind(): %d\n", errno);
        goto errExit;
    }

    remoteSockAddr.sin_family = AF_INET;
    remoteSockAddr.sin_addr.s_addr = inet_addr( "239.255.255.250" );
    remoteSockAddr.sin_port = htons(3702);
    memset(sendBuf, 0, (ONVIF_MSG_BUF_LEN * sizeof(char_t)));
    switch(NotifyType)
    {
    case TYPE_HELLO:
        srand((int)time(0));
        randomDelayTime = 1+(int)(500.0*rand()/(RAND_MAX+1.0));
        usleep(randomDelayTime*1000);
        GetTmpValueAfterReboot();
        if (g_ChangeScopes)
        {
            memset(TmpBuf, 0, sizeof(TmpBuf));
            for(n_i = 6; n_i < 12; n_i++)
            {
                if(0 != strlen(g_DeviceScopes[n_i].s_ScopeUrl))
                {
                    sprintf(TmpBuf+strlen(TmpBuf), "%s ", g_DeviceScopes[n_i].s_ScopeUrl);
                }
            }
            snprintf(sendBuf, ONVIF_MSG_BUF_LEN - 1, helloFormat, l_DeviceUuid, l_DeviceUuid, TmpBuf, HostIP, OnvifPort);
        }
        else
        {
            snprintf(sendBuf, ONVIF_MSG_BUF_LEN - 1, helloFormat, l_DeviceUuid, l_DeviceUuid, "", HostIP, OnvifPort);
        }
        sendto(sockfd, (char *)sendBuf, strlen(sendBuf), 0, (struct sockaddr *)&remoteSockAddr, sizeof(remoteSockAddr));
        break;
    case TYPE_BYE:
        snprintf(sendBuf, ONVIF_MSG_BUF_LEN - 1, byeFormat, l_DeviceUuid, l_DeviceUuid, HostIP, OnvifPort);
        sendto(sockfd, (char *)sendBuf, strlen(sendBuf), 0, (struct sockaddr *)&remoteSockAddr, sizeof(remoteSockAddr));
        break;
    default:
        fprintf(stderr, "DevStatusVaryNotifyService error in NotifyType\n");
        break;
    }
    fprintf(stderr, "DevStatusVaryNotifyService %d ok\n", NotifyType);

errExit:
    if(sockfd >= 0)
    {
        close(sockfd);
        sockfd = -1;
    }

    if(NULL != sendBuf)
    {
        free(sendBuf);
        sendBuf = NULL;
    }

    ONVIF_INFO("%s normal exit.........\n", __func__);
    return GMI_SUCCESS;
}


