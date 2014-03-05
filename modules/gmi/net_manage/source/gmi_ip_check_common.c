#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include "gmi_system_headers.h"
#include "gmi_netconfig_api.h"

/*===============================================================
func name:GMI_IPAddressTransform
func: Unsigned long type Ip Addr transform char type
input:
return:char type  IP addr pointer
--------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/3/5       1.0                 minchao.wang          create
**********************************************************************************/
char_t *GMI_IPAddressTransform(ulong_t ulIP)
{
    struct in_addr addr;
    addr.s_addr = ulIP;
    return inet_ntoa(addr);
}

/*===============================================================
func name:GMI_CheckIPStr
func: Check IP String is invalid, String It could not be something this is number or '.'
input:
return:0: string is vaild.  -1; String is invaild
--------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/3/5       1.0                 minchao.wang          create
**********************************************************************************/
GMI_RESULT GMI_CheckIPStr(const char_t *str)
{
    uint8_t a;
    int32_t dot=0;
    int32_t a3,a2,a1,a0,i=0;

    if(strlen(str)==0)
    {
        goto ERROR;
    }

    while((a = str[i++]) != '\0')
    {
        if( (a==' ')||(a=='.')||((a>='0')&&(a<='9')) )
        {
            if(a=='.')
            {
                dot++;
            }
        }
        else
        {
            goto ERROR;
        }
    }

    if(dot!=3)
    {
        goto ERROR;
    }
    else
    {
        sscanf(str, "%d.%d.%d.%d", &a3,&a2,&a1,&a0);
        if( (a0>=255)||(a1>255)||(a2>255)||(a3>255) )
        {
            goto ERROR;
        }
        return GMI_SUCCESS;
    }

    return GMI_SUCCESS;

ERROR:
    return GMI_FAIL;

}

/*===============================================================
func name:GMI_CheckNetmaskStr
func: Check IP String is invalid, String It could not be something this is number or '.' And the last character
       It could not be the number 255.
input:
return:0: string is vaild.  -1; String is invaild
--------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/3/5       1.0                 minchao.wang          create
**********************************************************************************/
GMI_RESULT GMI_CheckNetmaskStr(const char_t *str)
{
    uint8_t a;
    uint32_t netmask;
    int32_t dot=0;
    int32_t a3,a2,a1,a0;
    int32_t i=0, j=0;

    if(strlen(str)==0)
    {
        goto ERROR;
    }

    while((a = str[i++]) != '\0')
    {
        if( (a==' ')||(a=='.')||((a>='0')&&(a<='9')) )
        {
            if(a=='.')
            {
                dot++;
            }
        }
        else
        {
            goto ERROR;
        }
    }

    if(dot!=3)
    {
        goto ERROR;
    }
    else
    {
        sscanf(str, "%d.%d.%d.%d", &a3,&a2,&a1,&a0);

        if( (a0>255)||(a1>255)||(a2>255)||(a3>255) )
        {
            goto ERROR;
        }

        netmask = (a3<<24) | (a2<<16) | (a1<<8) | a0;
        if(0 == netmask)
        {
            goto ERROR;
        }

        for(i=0; i<32; i++)
        {
            if(netmask & (1<<i))
            {
                j = i;
                break;
            }
        }
        for(; j<32; j++)
        {
            if(netmask & (1<<j))
            {
                continue;
            }
            else
            {
                goto ERROR;
            }
        }
        return GMI_SUCCESS;
    }

    return GMI_SUCCESS;

ERROR:
    return GMI_FAIL;
}

/*===============================================================
func name:GMI_CheckIPConfig
func: Check IP Addr and IP Mask and GateWay is invalid, IP and getway must be in one net
input:  ip:  IP Addrs
         netmask: Ip Mask
         gateway: IP GateWay
return:0: string is vaild.  -1; String is invaild
--------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/3/5       1.0                 minchao.wang          create
**********************************************************************************/
GMI_RESULT GMI_CheckIPConfig(ulong_t ip, ulong_t netmask, ulong_t gateway)
{
    if(0 == gateway)
    {
        return GMI_SUCCESS;
    }

    if(((netmask&ip) != 0) && ((netmask&gateway) != 0))
    {
        if((netmask&ip) == (netmask&gateway))
        {
            return GMI_SUCCESS;
        }
        else
        {
            goto ERROR;
        }
    }
    else
    {
        goto ERROR;
    }

    return GMI_SUCCESS;

ERROR:
    return GMI_FAIL;

}

/*===============================================================
func name:GMI_CheckMacInvalid
func: Check MAC addrs is invalid
input: MAC addrs char
return:0: string is vaild.  -1; String is invaild
--------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/3/5       1.0                 minchao.wang          create
**********************************************************************************/
GMI_RESULT GMI_CheckMacInvalid(const char_t *MacAddrs)
{
    char_t Mac[32];
    char_t *strtmp;
    int32_t MacLen = 0;

    if(MacAddrs == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    if(strlen(MacAddrs) != 17)
        goto ERROR;

    strcpy(Mac , MacAddrs);

    strtmp = strtok(Mac,":");

    while(strtmp != NULL)
    {
        if(strlen(strtmp) != 2)
            goto ERROR;

        if((strtmp[0] >= '0' && strtmp[0] <= '9') || (strtmp[0] >= 'A' && strtmp[0] <= 'F') || (strtmp[0] >= 'a' && strtmp[0] <= 'f'))
        {
            if((strtmp[1] >= '0' && strtmp[1] <= '9') || (strtmp[1] >= 'A' && strtmp[1] <= 'F') || (strtmp[1] >= 'a' && strtmp[1] <= 'f'))
            {
                MacLen ++;
                strtmp = strtok(NULL,":");
            }
            else
            {
                goto ERROR;
            }
        }
        else
        {
            goto ERROR;
        }

    }

    if(MacLen != 6)
        goto ERROR;

    return GMI_SUCCESS;

ERROR:
    return GMI_FAIL;
}


