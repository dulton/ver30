#include <arpa/inet.h>
#include "net_manager.h"
#include "log.h"
#include "ipc_fw_v3.x_setting.h"
#include "gmi_netconfig_api.h"
#include "daemon.h"


GMI_RESULT CheckIPv4String(char_t *Ip)
{
    if (0 == strlen(Ip))
    {
        return GMI_INVALID_PARAMETER;
    }

    uint8_t t = 0;
    uint8_t i = 0;
    char_t  a;
    int32_t a0, a1, a2, a3;
    while ((a = Ip[i++]) != '\0')
    {
        if ((a == ' ')
                || (a == '.')
                || ((a >= '0') && (a <= '9')))
        {
            if (a == '.')
            {
                t++;
            }
        }
        else
        {
            return GMI_INVALID_PARAMETER;
        }
    }

    if (3 != t)
    {
        return GMI_INVALID_PARAMETER;
    }
    else
    {
        sscanf(Ip, "%d.%d.%d.%d", &a3, &a2, &a1, &a0);
        if (255 <= a0
                || 255 <= a1
                || 255 <= a2
                || 255 <= a3)
        {
            return GMI_INVALID_PARAMETER;
        }
    }

    return GMI_SUCCESS;
}


GMI_RESULT CheckIPv4MaskString(char_t *Mask)
{
    if (0 == strlen(Mask))
    {
        return GMI_INVALID_PARAMETER;
    }

    uint8_t t = 0;
    uint8_t i = 0, j = 0;
    char_t  a;
    int32_t a0, a1, a2, a3;
    uint32_t NetMask;
    while ((a = Mask[i++]) != '\0')
    {
        if ((a == ' ')
                || (a == '.')
                || ((a >= '0') && (a <= '9')))
        {
            if (a == '.')
            {
                t++;
            }
        }
        else
        {
            return GMI_INVALID_PARAMETER;
        }
    }

    if (3 != t)
    {
        return GMI_INVALID_PARAMETER;
    }
    else
    {
        sscanf(Mask, "%d.%d.%d.%d", &a3, &a2, &a1, &a0);
        if (255 < a0
                || 255 < a1
                || 255 < a2
                || 255 < a3)
        {
            return GMI_INVALID_PARAMETER;
        }

        NetMask = (a3 << 24) | (a2 << 16) | (a1 << 8) | a0;
        if (0 == NetMask)
        {
            return GMI_INVALID_PARAMETER;
        }

        for (i = 0; i < 32; i++)
        {
            if (NetMask & ( 1 << i))
            {
                j = i;
                break;
            }
        }

        for (; j < 32; j++)
        {
            if (NetMask & (1 << j))
            {
                continue;
            }
            else
            {
                return GMI_INVALID_PARAMETER;
            }
        }
    }

    return GMI_SUCCESS;
}


GMI_RESULT CheckIPv4Config(char_t *IpString, char_t *MaskString, char_t *GatewayString)
{
    if (0 == strlen(GatewayString))
    {
        return GMI_SUCCESS;
    }

    if (0 == strlen(IpString) || 0 == strlen(MaskString))
    {
        return GMI_INVALID_PARAMETER;
    }

    ulong_t Ip;
    ulong_t Mask;
    ulong_t Gateway;

    Ip      = inet_addr(IpString);
    Mask    = inet_addr(MaskString);
    Gateway = inet_addr(GatewayString);

    if (0 != (Ip & Mask)
            && 0 != (Mask & Gateway))
    {
        if ((Mask & Ip) == (Mask & Gateway))
        {
            return GMI_SUCCESS;
        }
        else
        {
            return GMI_INVALID_PARAMETER;
        }
    }
    else
    {
        return GMI_INVALID_PARAMETER;
    }
}


GMI_RESULT CheckMacString(char_t *MacString)
{
    if (0 == strlen(MacString))
    {
        return GMI_INVALID_PARAMETER;
    }

    uint8_t t = 0;
    uint8_t i = 0;
    char_t  a;
    while ((a = MacString[i++]) != '\0')
    {
        if ((a == ' ')
                || (a == ':')
                || ((a >= '0') && (a <= '9'))
                || ((a >= 'a') && (a <= 'f'))
                || ((a >= 'A') && (a <= 'F')))
        {
            if (a == ':')
            {
                t++;
            }
        }
        else
        {
            return GMI_INVALID_PARAMETER;
        }
    }

    if (5 != t)
    {
        return GMI_INVALID_PARAMETER;
    }

    return GMI_SUCCESS;
}


GMI_RESULT NetReadMacChar(char_t EthName[32], char_t Mac[6])
{
    int32_t            Sock;
    struct sockaddr_in Sin;
    struct ifreq       Ifr;

    if (NULL == Mac)
    {
        return GMI_INVALID_PARAMETER;
    }

    Sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (0 > Sock)
    {
        perror("socket");
        return GMI_FAIL;
    }

    strncpy(Ifr.ifr_name, EthName, IFNAMSIZ);
    Ifr.ifr_name[IFNAMSIZ - 1] = 0;

    memcpy(&Sin, &Ifr.ifr_addr, sizeof(Sin));
    if (0 > ioctl(Sock, SIOCGIFHWADDR, &Ifr))
    {
        perror("ioctl");
        return GMI_FAIL;
    }

    Mac[0] = Ifr.ifr_hwaddr.sa_data[0];
    Mac[1] = Ifr.ifr_hwaddr.sa_data[1];
    Mac[2] = Ifr.ifr_hwaddr.sa_data[2];
    Mac[3] = Ifr.ifr_hwaddr.sa_data[3];
    Mac[4] = Ifr.ifr_hwaddr.sa_data[4];
    Mac[5] = Ifr.ifr_hwaddr.sa_data[5];

    close(Sock);

    return GMI_SUCCESS;
}


GMI_RESULT NetReadMac(long_t NetId, char_t *MacPtr)
{
    if (MacPtr == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    int32_t            Sock;
    struct sockaddr_in Sin;
    struct ifreq       Ifr;

    Sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (Sock < 0)
    {
        perror("socket");
        return GMI_FAIL;
    }

    strncpy(Ifr.ifr_name, "eth0", IFNAMSIZ);
    Ifr.ifr_name[IFNAMSIZ - 1] = 0;

    memcpy(&Sin, &Ifr.ifr_addr, sizeof(Sin));
    if (ioctl(Sock, SIOCGIFHWADDR, &Ifr) < 0)
    {
        perror("ioctl");
        return GMI_FAIL;
    }

    sprintf(MacPtr, "%02x:%02x:%02x:%02x:%02x:%02x", \
            (uint8_t)Ifr.ifr_hwaddr.sa_data[0], (uint8_t)Ifr.ifr_hwaddr.sa_data[1], (uint8_t)Ifr.ifr_hwaddr.sa_data[2], (uint8_t)Ifr.ifr_hwaddr.sa_data[3], (uint8_t)Ifr.ifr_hwaddr.sa_data[4], (uint8_t)Ifr.ifr_hwaddr.sa_data[5]);

    close(Sock);

    return GMI_SUCCESS;
}


GMI_RESULT NetWriteMac(long_t NetId, char_t *MacPtr)
{
    if (MacPtr == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    return GMI_WriteMacConfig((char_t*)NET_CONFIG_FILE, NET_IPINFO_0_CONFIG_PATH, MacPtr);
}


GMI_RESULT NetReadIP(long_t NetId, IpInfo *IpInfoPtr)
{
    if (NULL == IpInfoPtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Result = GMI_ReadNetWorkCfg(IpInfoPtr);
    if (FAILED(Result))
    {
        SYS_ERROR("read IpInfo fail, Result = 0x%lx\n", Result);
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT NetWriteIP(IpInfo *IpInfoPtr)
{
    if (IpInfoPtr == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    IpInfo IpInfoTmp;

    memset(&IpInfoTmp, 0, sizeof(IpInfo));
    memcpy(&IpInfoTmp, IpInfoPtr, sizeof(IpInfo));

    GMI_RESULT Result = GMI_WriteNetWorkCfg(&IpInfoTmp);
    if (FAILED(Result))
    {
        SYS_ERROR("wriet IpInfo fail, Result = 0x%lx\n", Result);
        return Result;
    }

    DaemonReportIpChanged(IpInfoTmp.s_Eth, 5);

    return GMI_SUCCESS;
}


GMI_RESULT NetActivate(long_t NetId)
{
    SYS_INFO("%s in..........\n", __func__);
    GMI_RESULT Result = GMI_SUCCESS;
    IpInfo IpInfoTmp;

    Result = NetReadIP(NetId, &IpInfoTmp );
    if (FAILED(Result))
    {
        return Result;
    }
    SYS_INFO("%s normal out..........\n", __func__);
    return GMI_ActivateNet( &IpInfoTmp );
}

