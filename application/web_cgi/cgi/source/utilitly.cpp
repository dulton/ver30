#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include "utilitly.h"
#include "log.h"
#include "gmi_system_headers.h"
#include "sys_client.h"
#include "gmi_config_api.h"
#include "ipc_fw_v3.x_resource.h"
#include "ipc_fw_v3.x_setting.h"
#include "sys_info_readonly.h"


static struct hsearch_data htab;


static void Unescape(char_t *s)
{
    uint32_t c;

    while ((s = strpbrk(s, "%+")))
    {
        /* Parse %xx */
        if (*s == '%')
        {
            sscanf(s + 1, "%02x", &c);
            *s++ = (char) c;
            strncpy(s, s + 2, strlen(s) + 1);
        }
        /* Space is special */
        else if (*s == '+')
        {
            *s++ = ' ';
        }
    }

    return;
}


char_t *GetCgi(std::string Name)
{
    ENTRY e, *ep;

    if (!htab.table)
    {
        CGI_INFO("[%s]%d\n", __func__, __LINE__);
        return NULL;
    }

    e.key = (char_t*)Name.c_str();
    hsearch_r(e, FIND, &ep, &htab);
    if (ep)
    {
        // CGI_INFO("ep->data %s\n", (char_t *)ep->data);
        return  (char_t*)ep->data;
    }
    else
    {
        // CGI_INFO("NULL\n");
        //CGI_INFO("3 ep->name %s, ep->data %s\n", Name, (char_t *)ep->data);
        // CGI_INFO("3 e.name %s\n", Name);
        return NULL;
    }

    //return (char_t*)(ep ? ep->data : NULL);
}


static GMI_RESULT SetCgi(char_t *Name, char_t *Value)
{
    ENTRY e, *ep;

    if (!htab.table)
        return GMI_FAIL;

    e.key = Name;
    hsearch_r(e, FIND, &ep, &htab);
    if (ep)
    {
        ep->data = Value;
        // CGI_INFO("1 ep->name %s, ep->data %s\n", Name, (char_t *)ep->data);
    }
    else
    {
        e.data = Value;
        hsearch_r(e, ENTER, &ep, &htab);
        // CGI_INFO("2 ep->name %s, ep->data %s\n", Name, (char_t *)ep->data);
    }
    //assert(ep);

    return GMI_SUCCESS;
}


GMI_RESULT InitCgiCmd(char_t *Query)
{
    char_t *QueryTmp;
    char_t *Name;
    char_t *Value;
    int32_t Len;
    GMI_RESULT Result;

    if (Query == NULL)
    {
        CGI_ERROR("invalid parameter\n");
        return GMI_INVALID_PARAMETER;
    }

    QueryTmp = Query;
    Len = strlen(Query);
    while (strsep(&QueryTmp, "&"));
    //CGI_INFO("QueryTmp %s, Query %s, Nel %d\n", QueryTmp, Query, Nel);

    for (QueryTmp = Query; QueryTmp < (Query+Len);)
    {
        Unescape(Name=Value=QueryTmp);
        for (QueryTmp+=strlen(QueryTmp); QueryTmp<(Query+Len)&&!*QueryTmp; QueryTmp++);
        Name = strsep(&Value, "=");

        CGI_INFO("[%s]: Name=%s Value=%s\n", __func__, Name, Value);

        if (Value)
        {
            Result = SetCgi(Name, Value);
            if (FAILED(Result))
            {
                CGI_ERROR("SetCgi fail\n");
                return Result;
            }
        }
    }

    return GMI_SUCCESS;
}


GMI_RESULT InitCgiContent(char_t *Query)
{
    if (Query == NULL)
    {
        CGI_ERROR("invalid parameter\n");
        return GMI_INVALID_PARAMETER;
    }

    char_t *QueryTmp;
    char_t *Postion1;
    char_t *Postion2;
    char_t *Name;
    char_t *Value;
    int32_t Len;
    GMI_RESULT Result;

    Len = strlen(Query);

    Postion1 = strchr(Query, '{');
    if (NULL == Postion1)
    {
        CGI_ERROR("invalid parameter\n");
        return GMI_INVALID_PARAMETER;
    }

    Postion2 = strchr(Query, '}');
    if (NULL == Postion1)
    {
        CGI_ERROR("invalid parameter\n");
        return GMI_INVALID_PARAMETER;
    }

    *Postion1 = '\0';
    *Postion2 = '\0';
    Query += 1;
    QueryTmp = Query;
    for (uint32_t i = 0; i < strlen(QueryTmp); i++)
    {
        if (*(QueryTmp+i) == '"')
        {
            for (uint32_t j = i; j < strlen(QueryTmp); j++)
            {
                *(QueryTmp+j) = *(QueryTmp+j+1);
            }
        }
    }

    while (strsep(&QueryTmp, ","));

    for (QueryTmp = Query; QueryTmp < (Query+Len);)
    {
        Unescape(Name=Value=QueryTmp);

        for (QueryTmp+=strlen(QueryTmp); QueryTmp<(Query+Len)&&!*QueryTmp; QueryTmp++);

        Name = strsep(&Value, ":");

        CGI_INFO("[%s]: Name=%s Value=%s\n", __func__, Name, Value);

        if (Value)
        {
            Result = SetCgi(Name, Value);
            if (FAILED(Result))
            {
                CGI_ERROR("SetCgi fail\n");
                //return Result;
            }
        }
    }

    return GMI_SUCCESS;
}


GMI_RESULT InitCgi(char_t *Query)
{
    uint32_t KeyNum;
    uint32_t Cnt;

    if (!Query)
    {
        hdestroy_r(&htab);
        return GMI_INVALID_PARAMETER;
    }

    //statics key's totoal count
    KeyNum = 1;
    for (Cnt = 0; Cnt < strlen(Query); Cnt++)
    {
        if (*(Query+Cnt) == '&')
        {
            KeyNum++;
        }
        else if (*(Query+Cnt) == ',')
        {
            KeyNum++;
        }
    }

    //create hash table
    hcreate_r(KeyNum, &htab);

    char_t *ContentPtr       = NULL;
    const char_t *ContentKey = "&Content=";
    uint32_t KeyLen          = strlen(ContentKey);

    ContentPtr = strstr(Query, ContentKey);
    if (NULL == ContentPtr)
    {
        CGI_ERROR("Query no Content\n");
        hdestroy_r(&htab);
        return GMI_INVALID_PARAMETER;
    }

    //parse cgi command and set to hash table
    *ContentPtr = '\0';
    GMI_RESULT Result = InitCgiCmd(Query);
    if (FAILED(Result))
    {
        CGI_ERROR("InitCgiCmd fail\n");
        hdestroy_r(&htab);
        return Result;
    }

    //parse cgi content and set to hash table
    ContentPtr += KeyLen;
    Result = InitCgiContent(ContentPtr);
    if (FAILED(Result))
    {
        CGI_ERROR("InitCgiContent fail\n");
        hdestroy_r(&htab);
        return Result;
    }

    return GMI_SUCCESS;
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

GMI_RESULT GMI_StrRpl(char_t* DstOut, char_t* SrcIn, const char_t* SrcRpl, const char_t* DstRpl)
{
    if (NULL==SrcRpl ||NULL==DstRpl)
    {
        return GMI_INVALID_PARAMETER;
    }

    char_t* PSrcIn = SrcIn;
    char_t* PSrcOut = DstOut;

    int32_t SrcRplLen = strlen(SrcRpl);
    int32_t DstRplLen = strlen(DstRpl);

    char_t *PTmpSrc = NULL;
    int32_t Lenght = 0;

    do
    {
        //Find next node
        PTmpSrc = strstr(PSrcIn, SrcRpl);

        if(PTmpSrc != NULL)
        {
            //Copy first node and next node string
            Lenght = PTmpSrc - PSrcIn;
            memcpy(PSrcOut, PSrcIn, Lenght);
            //Copy must replace string
            memcpy(PSrcOut + Lenght, DstRpl, DstRplLen);
        }
        else
        {
            strcpy(PSrcOut, PSrcIn);
            //if no must copy string , while break
            break;
        }

        PSrcIn = PTmpSrc + SrcRplLen;
        PSrcOut = PSrcOut + Lenght + DstRplLen;
    } while (PTmpSrc != NULL);

    return GMI_SUCCESS;
}

GMI_RESULT GMI_SysCheckSessionId(uint16_t SessionId)
{
    GMI_RESULT Result = GMI_FAIL;
    boolean_t QuitFlag = false;

    Result =  SysCheckSessionId(SessionId, &QuitFlag);
    if (FAILED(Result))
    {
        return Result;
    }

    if (!QuitFlag)
    {
        return RETCODE_ERRSESSIONID;
    }

    return Result;
}


GMI_RESULT GMI_CheckAutoFlags(boolean_t *AutoFlags)
{
    GMI_RESULT Result = GMI_FAIL;
    Result = SysInfoReadInitialize();
    if (FAILED(Result))
    {
       return Result;
    }
    
    FD_HANDLE Handle;
    Result = SysInfoOpen(CAPABILITY_AUTO_FILE_NAME, &Handle);
    if (FAILED(Result))
    {
       SysInfoReadDeinitialize();
       return Result;
    }
    
    char_t  Value[64] = {"0"};
    Result = SysInfoRead(Handle, HW_AUTO_DETECT_INFO_PATH, HW_LENS_KEY, "DF003", Value);
    if (FAILED(Result))
    {
        SysInfoClose(Handle);      
        SysInfoReadDeinitialize();
        return Result;
    }
    else if (SUCCEEDED(Result))
    {
       if (strcmp("DF003", Value) == 0)
        {
            *AutoFlags = true;
        }
        else
        {
            *AutoFlags = false;
        }
    }

    SysInfoClose(Handle);
    SysInfoReadDeinitialize();
    return Result;
}

GMI_RESULT CheckIpExist(const char_t* InData)
{
    if (NULL == InData)
    {
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Result = GMI_FAIL;
    char_t Cmd[256] = {0}, CmdResult[256] = {0};

    sprintf(Cmd, "ping -W 4 -c 5 %s|grep packets|awk \'{print $4}\'", InData);

    FILE* fp=popen(Cmd, "r");
    fread(CmdResult, 1, 256, fp);
    pclose(fp);

    if (strcmp(CmdResult, "") == 0)
    {
        Result = RETCODE_IP_INVAILD;
    }
    else if (atoi(CmdResult) >= 3)
    {
        Result = RETCODE_IP_CONFLICT;
    }
    else
    {
        Result = RETCODE_OK;
    }

    return Result;
}


GMI_RESULT GMI_GetUpdatePort(int32_t  *UpgradePort)
{

    GMI_RESULT Result = GMI_FAIL;

    Result = SysInfoReadInitialize();
    if (FAILED(Result))
    {
       return Result;
    }
    
    FD_HANDLE Handle;
    
    Result = SysInfoOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if (FAILED(Result))
    {
       SysInfoReadDeinitialize();
       return Result;
    }
        
    Result = SysInfoRead(Handle, GMI_EXTERN_NETWORK_PORT_PATH, "UpdatePort", GMI_DAEMON_UPDATE_SERVER_PORT, UpgradePort);
    if (FAILED(Result))
    {
        SysInfoClose(Handle);      
        SysInfoReadDeinitialize();
        return Result;
    }
    
    SysInfoClose(Handle);
    SysInfoReadDeinitialize();
    return Result;
}

