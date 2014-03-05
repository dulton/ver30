#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "gmi_system_headers.h"
#include "gmi_config_api.h"
#include "gmi_netconfig_api.h"
#include "ipc_fw_v3.x_resource.h"
#include "sys_info_readonly.h"

#define GET_MAC_ADDR_CMD        "ifconfig eth0 | grep HWaddr | awk '{ print $5 }' | tr -d '\n'"
#define MAX_BUFFER_LENGTH                             64
#define MIN_BUFFER_LENGTH                               32
#define MAX_CHAR_BUF_LEN                             512

#define LOCK(lock)                                  {pthread_mutex_lock(lock);}
#define UNLOCK(lock)                                {pthread_mutex_unlock(lock);}

#define NETWORK_CONFIG_FILE                 "/etc/network/interfaces"

static pthread_mutex_t l_ConfigFile  =  PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t l_Snmp  =  PTHREAD_MUTEX_INITIALIZER;

static boolean_t l_SnmpFlags = false;
static uint16_t l_ServerPort = GMI_SNMP_SERVER_PORT;
static int32_t l_SockFdPtr = -1;
static uint8_t l_SnmpRequest[]= {0x30,0x27,0x02,0x01,0x00,0x04,0x06,0x70,0x75,0x62,0x6c,0x69,0x63,0xa0,0x1a,0x02,0x02,0x3c,0x2f,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x0e,0x30,0x0c,0x06,0x08,0x2b,0x06,0x01,0x02,0x01,0x01,0x05,0x00,0x05,0x00};
static uint8_t l_SnmpResponse[] = {0x30,0x32,0x02,0x01,0x00,0x04,0x06,0x70,0x75,0x62,0x6c,0x69,0x63,0xa2,0x25,0x02,0x02,0x3c,0x2f,0x02,0x01,0x00,0x02,0x01,0x00,0x30,0x19,0x30,0x17,0x06,0x08,0x2b,0x06,0x01,0x02,0x01,0x01,0x05,0x00,0x04,0x0b,0x43,0x46,0x42,0x31,0x32,0x30,0x33,0x2d,0x31,0x34,0x31};

GMI_RESULT GMI_GetMacAddr(const char_t *Cmd, char_t *Buf, int32_t Length)
{
    if(NULL == Cmd)
    {
        return GMI_INVALID_PARAMETER;
    }

    FILE *Fp = popen(Cmd, "r");
    if ( NULL == Fp )
    {
        return GMI_FAIL;
    }

    int32_t Retval = fread(Buf, 1, Length, Fp);
    pclose(Fp);
    if ( Retval < 0 )
    {
        return GMI_FAIL;
    }

    Buf[Retval] = '\0';

    return GMI_SUCCESS;
}

/*===============================================================
func name:GMI_ReadMacConfig
func: Read Device MAC addr
input:  FileName:Config file name
         ItemPath: MAC addr Item Save dircation
         DefMac: default value
         Mac: MAC addr return string
return:SUCCESS: string is vaild.  ERORR CODE; String is invaild
--------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/3/9       1.0                 minchao.wang          create
2013/8/13       1.0                 minchao.wang          modify
**********************************************************************************/
GMI_RESULT GMI_ReadMacConfig(const char_t * FileName, const char_t *ItemPath, const char_t *DefMac, char_t *Mac)
{
    if((NULL==FileName) || (NULL == ItemPath) || (NULL == DefMac))
    {
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Result = GMI_FAIL;
    char_t  Key[MAX_BUFFER_LENGTH]= {"0"};
    char_t Value[MAX_CHAR_BUF_LEN];
    char_t  SystemMac[MIN_BUFFER_LENGTH];
    FD_HANDLE  Handle;

    Result = GMI_XmlOpen(FileName, &Handle);
    if(FAILED(Result))
    {
        printf("GMI_XmlOpen xml file Error!!\n");
    }

    Result = GMI_GetMacAddr(GET_MAC_ADDR_CMD, SystemMac, sizeof(SystemMac));
    if(FAILED(Result))
    {
        printf("Get System Mac error!!\n");
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "s_Mac");

    Result = GMI_XmlRead(Handle, ItemPath, Key, DefMac, Value, GMI_CONFIG_READ_ONLY);
    if(SUCCEEDED(Result))
    {
        if(strcmp(Value, SystemMac) != 0)
        {
            strcpy(Mac,SystemMac);
        }
        else
        {
            strcpy(Mac,Value);
        }
    }
    else
    {
          strcpy(Mac ,SystemMac);
    }

    Result = GMI_XmlFileSave(Handle);
    if(FAILED(Result))
    {
        printf("GMI_XmlFileSave xml file Error!!\n");
        return Result;
    }

    return Result;
}

/*===============================================================
func name:GMI_WriteMacConfig
func: Write Device MAC addr
input:  FileName:Config file name
         ItemPath: MAC addr Item Save dircation
         Mac: MAC addr value
return:SUCCESS: string is vaild.  ERORR CODE; String is invaild
--------------------------------------------------------------------
modify notes:
date              version            author                notes
2013/3/9       1.0                 minchao.wang          create
2013/8/13       1.0                 minchao.wang          modify
**********************************************************************************/
GMI_RESULT GMI_WriteMacConfig(const char_t * FileName, const char_t *ItemPath, char_t *Mac)
{

    if(NULL==Mac || NULL==FileName)
    {
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Result = GMI_FAIL;
    char_t  Key[MAX_BUFFER_LENGTH];
    char_t  SystemMac[MIN_BUFFER_LENGTH];
    char_t CmdBuffer[MAX_BUFFER_LENGTH];

    FD_HANDLE  Handle;

    Result = GMI_XmlOpen(FileName, &Handle);
    if(FAILED(Result))
    {
        printf("GMI_XmlOpen xml file Error!!\n");
    }

    Result = GMI_GetMacAddr(GET_MAC_ADDR_CMD, SystemMac, sizeof(SystemMac));
    if(FAILED(Result))
    {
        printf("Get System Mac error!!\n");
        goto ERROR;
    }

    Result = GMI_CheckMacInvalid(SystemMac);
    if(FAILED(Result))
    {
        printf("Get System Mac error!!\n");
        goto ERROR;
    }

    Result = GMI_CheckMacInvalid(Mac);
    if(FAILED(Result))
    {
        printf("Get System Mac error!!\n");
        goto ERROR;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "s_Mac");
    if(strcmp(Mac, SystemMac) != 0)
    {
        Result = GMI_XmlWrite(Handle, ItemPath, Key, Mac);
        if(SUCCEEDED(Result))
        {
            memset(CmdBuffer, 0, sizeof(CmdBuffer));
            snprintf(CmdBuffer, sizeof(CmdBuffer), "nandwrite -E %s", Mac);
            if(system(CmdBuffer))
            {
                goto ERROR;
            }
        }
    }
    else
    {
        Result = GMI_XmlWrite(Handle, ItemPath, Key, Mac);
        if(FAILED(Result))
        {
            printf("Write Mac error!!\n");
            goto ERROR;
        }
    }

    Result = GMI_XmlFileSave(Handle);
    if(FAILED(Result))
    {
        printf("GMI_XmlFileSave xml file Error!!\n");
        goto ERROR;
    }

    return GMI_SUCCESS;

ERROR:
    return GMI_FAIL;
}

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
GMI_RESULT GMI_ReadNetWorkCfg(IpInfo *IpInfoConfig)
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
GMI_RESULT GMI_WriteNetWorkCfg(const IpInfo *IpInfoConfig)
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

GMI_RESULT GMI_ActivateNet(const IpInfo *NetIpInfo)
{
     return GMI_SUCCESS;
}

GMI_RESULT GMI_SnmpPortRead(uint16_t *ServerPort)
{
    GMI_RESULT Result = GMI_FAIL; 

    Result = SysInfoReadInitialize();
    if (FAILED(Result))
    {
        printf("SysInfoReadInitialize is Fail ! ! \n");
    }

    FD_HANDLE Handle;
    Result = SysInfoOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if (FAILED(Result))
    {
        printf("SysInfoOpen is Fail ! ! \n");
    }
    
    Result = SysInfoRead(Handle, GMI_EXTERN_NETWORK_PORT_PATH, GMI_SNMP_SERVER_PORT_KEY, GMI_SNMP_SERVER_PORT, ServerPort);
    if (FAILED(Result))
    {
	*ServerPort = GMI_SNMP_SERVER_PORT;
    }

    SysInfoClose(Handle);
    
    SysInfoReadDeinitialize();

    return GMI_SUCCESS;
}

GMI_RESULT GMI_OpenSocket(uint16_t ServerPort, int32_t *SockFdPtr)
{
    int32_t SockFd = -1;
    int32_t Result;
    struct sockaddr_in  LocalAddr;	
    
    LocalAddr.sin_family      = AF_INET;
    LocalAddr.sin_port        = htons(ServerPort);
    LocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&(LocalAddr.sin_zero), 0, 8);
    
    SockFd = socket(AF_INET, SOCK_DGRAM, 0);  
    if (SockFd < 0)
    {
        return GMI_FAIL;
    }
    
    if (-1 != SockFd) 
    {
        int32_t Loop = 1;
        
        setsockopt(SockFd, SOL_SOCKET, SO_REUSEADDR, &Loop, sizeof(Loop));    
        Result = bind(SockFd, (struct sockaddr*)&LocalAddr, sizeof(struct sockaddr));
        if (0 > Result)
        {
            return GMI_FAIL;
        }
    }
    
    *SockFdPtr = SockFd;
    return GMI_SUCCESS;
}

GMI_RESULT GMI_ServerRunning(boolean_t Flag)
{

    if (!Flag)
    {
        return GMI_SUCCESS;
    }
    printf("=============>[%s][%d]\n",__func__,__LINE__);

    int32_t    i;
    fd_set       FdSet; 	
    uint8_t      Buffin[1500];
    int32_t      Readlen;	
    struct sockaddr_in ClientAddr;
    struct timeval     Timeout;
    socklen_t SockAddrLen  = sizeof(struct sockaddr_in);
    int32_t Result;
    
    Timeout.tv_sec	= 1;
    Timeout.tv_usec = 0;
    FD_ZERO(&FdSet);
    FD_SET(l_SockFdPtr, &FdSet); 
    if (0 < select(l_SockFdPtr+1, &FdSet, NULL, NULL, &Timeout)) 
    {
   	if (FD_ISSET(l_SockFdPtr, &FdSet)) 
   	{
   		memset(Buffin, 0, sizeof(Buffin));
   		Readlen = recvfrom(l_SockFdPtr, Buffin, sizeof(Buffin), 0, (struct sockaddr*)&ClientAddr, &SockAddrLen);	
   		if (0 < Readlen) 
   		{
   			if (Readlen == sizeof(l_SnmpRequest))
   			{
   				for (i = 0; i < 5; i++)
   				{
   					if (Buffin[i] != l_SnmpRequest[i])
   					{
   						break;
   					}
   				}
   				
   				if (i < 5)
   				{																			   
   					printf("recvfrom buffer content incorrect\n");
   					for (i = 0; i < Readlen; i++)
   					{
   						printf("0x%x\t", Buffin[i]);
   					}
   					printf("\n");
   				}
   				else
   				{
   					Result = sendto(l_SockFdPtr, l_SnmpResponse, sizeof(l_SnmpResponse), 0, (struct sockaddr*)&ClientAddr, sizeof(struct sockaddr));
   					if (Result < 0)
   					{
   						printf("sendto error, Result = %d\n", Result);								 
   					}
   				}
   			}
   			else
   			{
   				//printf("recvfrom error, Readlen = %d, sizeof(l_SnmpRequest) = %ld, not equal \n", Readlen, sizeof(l_SnmpRequest));
   			}  
   		}
   		else
   		{
   			 printf("recvfrom error, Readlen = 0x%x\n", Readlen);
   		}				
   	}				   
   }

     return GMI_SUCCESS;
}

static  void* GMI_SNMPServer(void* unused)
{

    pthread_detach(pthread_self());

    for(;;)
    {
	GMI_ServerRunning(l_SnmpFlags);
        sleep(1); 	    	    
    }

    pthread_exit(NULL);
    return NULL;

}

GMI_RESULT GMI_SnmpServerInit(void)
{

    GMI_RESULT Result = GMI_SUCCESS;
    
    Result = GMI_SnmpPortRead(&l_ServerPort);
    if(FAILED(Result))
    {
    	printf("GMI_SnmpPortRead is Fail ! ! \n");
    }
    
    Result = GMI_OpenSocket(l_ServerPort, &l_SockFdPtr);
    if(FAILED(Result))
    {
    	printf("GMI_OpenSocket is Fail ! ! \n");
    }
    
    pthread_t PthreadId =-1;

    LOCK(&l_Snmp);
    l_SnmpFlags = false;  
    UNLOCK(&l_Snmp);

    pthread_create(&PthreadId,NULL,GMI_SNMPServer,NULL);
 
    return Result;
}

GMI_RESULT GMI_SnmpServerStart(void)
{

    LOCK(&l_Snmp);
    l_SnmpFlags = true;  
    UNLOCK(&l_Snmp);
  
    return GMI_SUCCESS;
}

GMI_RESULT GMI_SnmpServerStop(void)
{
    LOCK(&l_Snmp);
    l_SnmpFlags = false;  
    UNLOCK(&l_Snmp);

    return GMI_SUCCESS;
}


