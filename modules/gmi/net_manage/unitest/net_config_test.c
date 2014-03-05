#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include "gmi_system_headers.h"
#include "gmi_netconfig_api.h"

int main()
{
    int32_t nRet;
    int32_t Max = 10;
#if 0
    char_t file[]="netconfig.xml";
    const char_t *path="/system/netconfig/ipinfo/";
    char_t read[32];
    char_t DefMac[32];
    char_t NewMac[32];

  
    memset(read, 0, sizeof(read));
    nRet = GMI_ReadMacConfig(file, path, DefMac, read);

    strcpy(NewMac, "20:C7:56:52:A3:49");

    nRet = GMI_WriteMacConfig(file,path,NewMac);
#endif
   printf("=============>[%s][%d]\n",__func__,__LINE__);

   GMI_SnmpServerInit();
   printf("=============>[%s][%d]\n",__func__,__LINE__);

   GMI_SnmpServerStart();
   printf("=============>[%s][%d]\n",__func__,__LINE__);

    while(1)
    {
          sleep(1);
          if(Max == 0)
          {
	      printf("GMI_SnmpServerStop  [%s][%d]\n",__func__,__LINE__);
	      GMI_SnmpServerStop();
          }
          Max--;
    }
	printf("=============>[%s][%d]\n",__func__,__LINE__);

    return nRet;

}



