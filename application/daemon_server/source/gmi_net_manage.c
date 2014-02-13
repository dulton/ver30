/******************************************************************************
modules 	:	 network manage
name		:	 gmi_net_manage.c
function	:	 Read System config file ,if no config file.
                         Use default value  and active system network.
author	:	 minchao.wang
version 	:	 1.0.0.0.0
---------------------------------------------------------------
modification	:
	date		version 	 author 	modification
	8/12/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
#include "gmi_net_manage.h"
#include "gmi_netconfig_api.h"
#include "gmi_includes.h"
#include "gmi_system.h"
#include "gmi_debug.h"
#include "gmi_hardware_monitor.h"
#include "gmi_struct.h"
#include "gmi_config.h"
#include "gmi_config.h"
#include "gmi_common.h"
#include "ipc_fw_v3.x_setting.h"

#define NETWORK_CONFIG_FILE                 "/etc/network/interfaces"

static pthread_mutex_t l_ConfigFile  =  PTHREAD_MUTEX_INITIALIZER;

/*============================================================================
name				:	GMI_ReadNetCfg
function			:  Read system default network file IP value.
algorithm implementation	:	no
global variable 		:	no
parameter declaration	:    IpInfoConfig: network config data
return				:	 success: GMI_SUCCESS
						    fail; ERROR CODE
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	8/13/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_ReadNetCfg(IpInfo *IpInfoConfig)
{

    LOCK(&l_ConfigFile);

    FILE *Fp = NULL;
    Fp = fopen(NETWORK_CONFIG_FILE, "r");
    if(NULL == Fp)
    {
        UNLOCK(&l_ConfigFile);
        return GMI_FAIL;
    }

    for(;;)
    {
        char_t Str[MIN_BUFFER_LENGTH];
        char_t Head[MIN_BUFFER_LENGTH];
        char_t Tmp[MIN_BUFFER_LENGTH];

        bzero(Tmp, sizeof(Tmp));
        bzero(Str, sizeof(Str));
        bzero(Head, sizeof(Head));
        if( fgets(Str, sizeof(Str), Fp) == 0 )
            break;
        if( strlen(Str) == 0 || Str[0] == 0x0a )
            continue;
        if(Str[0] == ';')
            continue;
        if( sscanf(Str, "%s", Head) == 0)
            continue;

        if( strcmp( Head , "address") == 0 )
        {
            sscanf( Str , "%*s%s" , Tmp );
            IpInfoConfig->s_IpAddr = inet_addr(Tmp);
        }
        else if( strcmp( Head , "netmask") == 0 )
        {
            sscanf( Str , "%*s%s", Tmp);
            IpInfoConfig->s_NetMask = inet_addr(Tmp);
        }
        else if( strcmp( Head , "gateway") == 0 )
        {
            sscanf( Str , "%*s%s" , Tmp);
            IpInfoConfig->s_GateWay = inet_addr(Tmp);
        }
    }

    if(Fp != NULL)
    {
        fclose(Fp);
        Fp = NULL;
    }
    UNLOCK(&l_ConfigFile);

    return GMI_SUCCESS;

}

/*============================================================================
name				:	GMI_WriteNetCfg
function			:  Set system default network file IP value.
algorithm implementation	:	no
global variable 		:	no
parameter declaration	:    IpInfoConfig: network config data
return				:	 success: GMI_SUCCESS
						    fail; ERROR CODE
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	8/13/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_WriteNetCfg(const IpInfo *IpInfoConfig)
{

    if(NULL == IpInfoConfig)
    {
        return GMI_INVALID_PARAMETER;
    }

    LOCK(&l_ConfigFile);

    FILE       *Fp = NULL;
    Fp = fopen(NETWORK_CONFIG_FILE, "wb");
    if(NULL == Fp)
    {
        UNLOCK(&l_ConfigFile);
        return GMI_FAIL;
    }

    fprintf(Fp, "# Configure Loopback\n");
    fprintf(Fp, "auto lo\n");
    fprintf(Fp, "iface lo inet loopback\n");
    fprintf(Fp, "auto eth0\n");
    fprintf(Fp, "iface eth0 inet static\n");
    fprintf(Fp, "address      %s\n", GMI_IPAddressTransform(IpInfoConfig->s_IpAddr));
    fprintf(Fp, "netmask      %s\n", GMI_IPAddressTransform(IpInfoConfig->s_NetMask));
    fprintf(Fp, "gateway      %s\n", GMI_IPAddressTransform(IpInfoConfig->s_GateWay));

    if(Fp != NULL)
    {
        fclose(Fp);
        Fp = NULL;
    }
    UNLOCK(&l_ConfigFile);

    return GMI_SUCCESS;
}

/*============================================================================
name				:	GMI_SetSystemDefaultIp
function			:  Set system default value to network data.
algorithm implementation	:	no
global variable 		:	no
parameter declaration	:    IpInfoConfig: network config data
return				:	 success: GMI_SUCCESS
						    fail; ERROR CODE
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	8/12/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_SetSystemDefaultIp(IpInfo *IpInfoConfig)
{
    IpInfoConfig->s_Eth = NET_DEFAULT_ETH;
    IpInfoConfig->s_Dhcp = NET_DEFAULT_DHCP;
    IpInfoConfig->s_IpAddr = inet_addr(NET_DEFAULT_IP);
    IpInfoConfig->s_NetMask = inet_addr(NET_DEFAULT_MASK);
    IpInfoConfig->s_GateWay = inet_addr(NET_DEFAULT_GATEWAY);
    IpInfoConfig->s_Dns1 = inet_addr(NET_DEFAULT_DNS);
    IpInfoConfig->s_Dns2 = inet_addr(NET_DEFAULT_DNS);
    IpInfoConfig->s_Dns3 = inet_addr(NET_DEFAULT_DNS);

    return GMI_SUCCESS;
}

/*============================================================================
name				:	GMI_ReadIpConfig
function			:  Read system config file, if no config file.
algorithm implementation	:	no
global variable 		:	no
parameter declaration	:    IpInfoConfig: network config data
return				:	 success: GMI_SUCCESS
						    fail; ERROR CODE
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	8/12/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT  GMI_ReadIpConfig(IpInfo *IpInfoConfig)
{
    GMI_RESULT Result = GMI_FAIL;
    IpInfo DefaultIpInfoConfig;
    memset(&DefaultIpInfoConfig, 0 ,sizeof(IpInfo));

    Result = GMI_SetSystemDefaultIp(&DefaultIpInfoConfig);
    if(FAILED(Result))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_ReadIpConfig Error  ! ! ");
    }

    Result = GMI_ReadOnlyNetIpConfig(NET_CONFIG_FILE, NET_IPINFO_0_CONFIG_PATH, &DefaultIpInfoConfig, IpInfoConfig);
    if(FAILED(Result))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_ReadNetIpConfig Error	! ! ");
        return Result;
    }

    return Result;
}

/*============================================================================
name				:	GMI_NetActive
function			:  Active system network.
algorithm implementation	:	no
global variable 		:	no
parameter declaration	:    IpInfoConfig: network config data
return				:	 success: GMI_SUCCESS
						    fail; ERROR CODE
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	8/12/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT  GMI_NetActive(const IpInfo *IpInfoConfig)
{
    if(NULL == IpInfoConfig)
    {
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Result = GMI_FAIL;

    Result = GMI_ActivateNet(IpInfoConfig);
    if(FAILED(Result))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_NetActive Error  ! ! ");
        return Result;
    }

    return Result;
}

/*============================================================================
name				:	GMI_NetManageInit
function			:  Read System config file ,if no config file.
                         Use default value  and active system network.
algorithm implementation	:	no
global variable 		:	no
parameter declaration	:    no
return				:	 success: GMI_SUCCESS
						    fail; ERROR CODE
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	8/12/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT  GMI_NetManageInit(void)
{

    GMI_RESULT Result = GMI_FAIL;

    Result = GMI_SetSystemDefaultIp(g_IpInfo);
    if(FAILED(Result))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_SetSystemDefaultIp Error  ! ! ");
    }

    Result = GMI_ReadNetCfg(g_IpInfo);
    if(FAILED(Result))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_ReadNetCfg Error	! ! ");
    }

#if 0
    char_t CmdBuffer[MIN_BUFFER_LENGTH];
    memset(CmdBuffer, 0,  sizeof(CmdBuffer));
    snprintf(CmdBuffer, MIN_BUFFER_LENGTH, "ifdown -a");
    if (system(CmdBuffer) <  0)
    {
        DAEMON_PRINT_LOG(ERROR,"ifdown -a System exe Error! ! ");
        GMI_DeBugPrint("[%s][%d] ifdown -a System exe Error ",__func__,__LINE__);
    }

    memset(CmdBuffer, 0,  sizeof(CmdBuffer));
    snprintf(CmdBuffer, MIN_BUFFER_LENGTH, "ifup -a");
    if (system(CmdBuffer) <  0)
    {
        DAEMON_PRINT_LOG(ERROR,"ifup -a System exe Error! ! ");
        GMI_DeBugPrint("[%s][%d] ifup -a System exe Error ",__func__,__LINE__);
    }

    //}
    else if (SUCCEEDED(Result))
    {
        //read system config file
        Result = GMI_ReadIpConfig(g_IpInfo);
        if(FAILED(Result))
        {
            // read default Ip
            Result = GMI_SetSystemDefaultIp(g_IpInfo);
            if(FAILED(Result))
            {
                DAEMON_PRINT_LOG(ERROR,"GMI_SetSystemDefaultIp Error  ! ! ");
            }
        }
        else if(SUCCEEDED(Result))
        {
            IpInfo DefaultIpInfo;
            memset(&DefaultIpInfo, 0, sizeof(IpInfo));

            Result = GMI_ReadNetCfg(&DefaultIpInfo);
            if(FAILED(Result))
            {
                DAEMON_PRINT_LOG(ERROR,"GMI_ReadNetCfg Error  ! ! ");
            }
            else if(SUCCEEDED(Result))
            {
                if((DefaultIpInfo.s_IpAddr) != (g_IpInfo->s_IpAddr))
                {
                    Result = GMI_WriteNetCfg(g_IpInfo);
                    if(FAILED(Result))
                    {
                        DAEMON_PRINT_LOG(ERROR,"GMI_WriteNetCfg Error  ! ! ");
                    }
                }
            }
        }
    }
    //Active system network
    Result =  GMI_NetActive(g_IpInfo);
    if(FAILED(Result))
    {
        //read default Ip
        DAEMON_PRINT_LOG(ERROR,"GMI_NetActive Error  ! ! ");
        return Result;
    }
#endif


    return Result;
}


