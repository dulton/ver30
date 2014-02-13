#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "gmi_system_headers.h"
#include "gmi_network_reboot_times.h"

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
GMI_RESULT GMI_ReadRebootTimes(SystemNetWorkRebootTimes *NetWorkRebootTimes)
{

    FILE *Fp = NULL;
    Fp = fopen(NETWORK_REBOOT_FILE, "r");
    if(NULL == Fp)
    {
        return GMI_FAIL;
    }

    for(;;)
    {
        char_t Str[BUFFER_LENGTH];
        char_t Head[BUFFER_LENGTH];
        char_t Tmp[BUFFER_LENGTH];

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

        if( strcmp( Head , "Times") == 0 )
        {
            sscanf( Str , "%*s%s" , Tmp );
            NetWorkRebootTimes->s_Times= atoi(Tmp);
        }
        else if( strcmp( Head , "RebootTime") == 0 )
        {
            sscanf( Str , "%*s%s", Tmp);
            NetWorkRebootTimes->s_RebootTime= atoi(Tmp);
        }

    }

    if(Fp != NULL)
    {
        fclose(Fp);
        Fp = NULL;
    }

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
GMI_RESULT GMI_WriteRebootTimes(const SystemNetWorkRebootTimes *NetWorkRebootTimes)
{

    if(NULL == NetWorkRebootTimes)
    {
        return GMI_INVALID_PARAMETER;
    }

    FILE       *Fp = NULL;
    Fp = fopen(NETWORK_REBOOT_FILE, "wb");
    if(NULL == Fp)
    {
        return GMI_FAIL;
    }

    fprintf(Fp, "Times      %d\n", NetWorkRebootTimes->s_Times);
    fprintf(Fp, "RebootTime      %d\n", NetWorkRebootTimes->s_RebootTime);

    if(Fp != NULL)
    {
        fclose(Fp);
        Fp = NULL;
    }
    return GMI_SUCCESS;
}


GMI_RESULT GMI_GetNetWorkPackets(const char_t *Cmd, char_t *Buf, int32_t Length)
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


GMI_RESULT GMI_FileExists(const char_t *FileName)
{
    if (access(FileName ,F_OK) == 0)
    {
        return GMI_SUCCESS;
    }
    else
    {
        return GMI_FAIL;
    }
}

GMI_RESULT GMI_NetWorkDevCheck(boolean_t *dev)
{
    struct ifreq *ifr;   
    struct ifreq ifs[16];    
    struct ifconf ifc;   

    int32_t SockFd;   
    int32_t DevCnt = 0;  

    if ( (SockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)    
    {		 
    	 printf("create ifr socket error[%d]!\n", errno);		 
    	 return GMI_FAIL;   
    }  

    ifc.ifc_len = sizeof(ifs);	
    ifc.ifc_req = ifs;	
    
    if (ioctl(SockFd, SIOCGIFCONF, &ifc) < 0)	
    {		
    	printf("ioctl SIOCGIFCONF error[%d]!\n", errno);		
    	close(SockFd);		  
    	return GMI_FAIL;	  
    }	
    
    DevCnt = ifc.ifc_len/sizeof(struct ifreq);  
    ifr = ifc.ifc_req;	
    
    if ( (1 == DevCnt) && (0 == strcmp(ifr->ifr_name, "lo")) )    
    {		
        *dev = false; 
    }	 
    else
    {
         *dev = true; 
    }
    
    close(SockFd); 
    return GMI_SUCCESS;
}

