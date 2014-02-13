#include "machine.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include<sys/time.h>
#include "gmi_system_headers.h"
#include "gmi_brdwrapperdef.h"
#include "gmi_brdwrapper.h"
#include <syslog.h>
#include "gmi_network_reboot_times.h"

int machine_init(struct statics * statics);
void get_system_info(struct system_info *info);


static uint32_t l_DetectNumber = NETWORK_DETECT_NUMBER_MAX;
static uint32_t l_InterruptsNumber = NETWORK_DETECT_INTERRUPTS_MAX;
SystemNetWorkRebootTimes RebootTimes;
static uint8_t DownFlags = 0;
static uint8_t UpFlags = 0;
static uint8_t PhyFlags = 0;

int32_t main(void)
{
    struct statics Stat;
    struct system_info Info;

    machine_init(&Stat);

    openlog("system_resource_detect", LOG_CONS |LOG_PERROR , LOG_LOCAL0);

    int32_t IdleCpu = 0;
    uint8_t Link = 0;
    uint32_t InterruptsNumber = 0;
    uint32_t OldInterruptsNumber = 0;

    for(;;)
    {
        get_system_info(&Info);

        IdleCpu = Info.cpustates[3]/10;
        if(IdleCpu < 20)
        {
            syslog(LOG_DEBUG,"[%d]System  CPU  Use  Is  %d",__LINE__, (100-IdleCpu));
        }

        if(Info.memory[1] < 21000)
        {
            syslog(LOG_DEBUG,"[%d]System Free Memroy Is  %d Kb",__LINE__, Info.memory[1]);
        }

	GMI_RESULT	Result = GMI_FAIL;

        //detect system Phy is exist
      //  memset(TxBuffer, 0 ,sizeof(TxBuffer));
       // Result = GMI_GetNetWorkPackets(GMI_NETWORK_DEV_CMD, TxBuffer, sizeof(TxBuffer));
	boolean_t dev = false;
	Result = GMI_NetWorkDevCheck(&dev);
        if (SUCCEEDED(Result))
        {
   //         const char_t *tmp = "eth0";
            //if Phy is exist , detect network Line is exist
           // if(memcmp(tmp, TxBuffer,4) == 0)
	    if ( dev )
            {
		syslog(LOG_DEBUG,"[%d]GMI_NetWorkDevCheck Is Exist",__LINE__);
                l_DetectNumber = NETWORK_DETECT_NUMBER_MAX;
                Result  = GMI_BrdGetEthLinkStat(0, &Link);
                if (SUCCEEDED(Result))
                {
                    //Network is Down
                    if (NETWORK_DOWN == Link)
                    {
                        if(DownFlags == 0)
                        {
                            DownFlags = 1;
                            UpFlags = 0;
                            syslog(LOG_DEBUG,"[%d]System NetWork Status Is Down",__LINE__);
                        }
                    }
                    //Network is Up
                    else if (NETWORK_UP == Link)
                    {
                        if(UpFlags == 0)
                        {
                            UpFlags = 1;
                            DownFlags = 0;
                            syslog(LOG_DEBUG,"[%d]System NetWork Status Is Up",__LINE__);
                        }
                    }

                    char_t Buffer[32];
                    Result = GMI_GetNetWorkPackets(GMI_NETWORK_INTERRUPTS_CMD, Buffer, sizeof(Buffer));
                    if (SUCCEEDED(Result))
                    {
                          InterruptsNumber = atoi(Buffer) - OldInterruptsNumber;
                          OldInterruptsNumber = atoi(Buffer);
                          if(InterruptsNumber > NETWORK_INTERRUPTS_NUMBER_MAX)
                          {
				syslog(LOG_DEBUG,"[%d]System NetWork Restart l_InterruptsNumber = %d",__LINE__,l_InterruptsNumber);
                                l_InterruptsNumber--;
                                if(0 == l_InterruptsNumber)
                                {
				        syslog(LOG_DEBUG,"[%d]System NetWork Restart",__LINE__);
				        l_InterruptsNumber = NETWORK_DETECT_INTERRUPTS_MAX;
                                       char_t CmdBuffer[256];
                                       memset(CmdBuffer, 0, sizeof(CmdBuffer));
                                       strcpy(CmdBuffer, GMI_NETWORK_RESTART);
                                       system(CmdBuffer);
                                       //网卡重启，会导致config_tool服务CPU 飙升到60%，重启网卡后，kill config_tool服务。
				       syslog(LOG_DEBUG,"[%d]System NetWork Restart, config_tool server restart",__LINE__);
                                       memset(CmdBuffer, 0, sizeof(CmdBuffer));
                                       strcpy(CmdBuffer, GMI_CONFIT_TOOL_RESTART);
                                       system(CmdBuffer);       
                                }
                          }
                     }
                }
                else
                {
                    syslog(LOG_ERR,"[%d]System  GetEthLinkStat Fail",__LINE__);
                }
            }
            else
            {
		syslog(LOG_DEBUG,"[%d]GMI_NetWorkDevCheck Is not Exist",__LINE__);
                //Phy is not exist ,Detect Phy 2 minute
                l_DetectNumber--;
                if(0 == l_DetectNumber)
                {
                    GMI_RESULT Result = GMI_FAIL;
                    struct timeval tv;
                    struct timezone tz;
                    gettimeofday (&tv , &tz);

                    Result = GMI_FileExists(NETWORK_REBOOT_FILE);
                    if (SUCCEEDED(Result))
                    {
                        Result = GMI_ReadRebootTimes(&RebootTimes);
                        if (SUCCEEDED(Result))
                        {

			    //如果上次找不到PHY的时间和这次相差24小时，那么重新开始计时
                            if ((tv.tv_sec - RebootTimes.s_RebootTime) > 86400)
                            {
				syslog(LOG_DEBUG,"[%d]System NetWork Phy not detect time for 24 hours", __LINE__);
                                RebootTimes.s_Times = 0;
                                RebootTimes.s_RebootTime = tv.tv_sec;
                                Result = GMI_WriteRebootTimes(&RebootTimes);
                                if(FAILED(Result))
                                {
                                    syslog(LOG_ERR,"[%d]Write Reboot file Fail",__LINE__);
                                }
                            }

                            if(RebootTimes.s_Times > 5)
                            {
                                if(PhyFlags == 0)
                                     syslog(LOG_ERR,"[%d]System Phy  is not Exist, System Reboot 5 times",__LINE__);
                            }
                            else
                            {
                                RebootTimes.s_Times += 1;
                                RebootTimes.s_RebootTime = tv.tv_sec;
                                
                                Result = GMI_WriteRebootTimes(&RebootTimes);
                                if(FAILED(Result))
                                {
                                    syslog(LOG_ERR,"[%d]Write Reboot file Fail",__LINE__);
                                }

                                Result = GMI_BrdHwReset();
                                if (FAILED(Result))
                                {
                                    syslog(LOG_ERR,"[%d]GMI_BrdHwReset Call Fail",__LINE__);
                                }
                                else if (SUCCEEDED(Result))
                                {
                                    syslog(LOG_ERR,"[%d]System Phy  is not Exist, GMI_BrdHwReset	for Watchdog",__LINE__);
                                }
                            }
                        }
                    }
                    else if (FAILED(Result))
                    { 
                        syslog(LOG_DEBUG,"[%d]System NetWork Phy First reboot", __LINE__);

                        RebootTimes.s_Times = 1;
                        RebootTimes.s_RebootTime = tv.tv_sec;
                        Result = GMI_WriteRebootTimes(&RebootTimes);
                        if(FAILED(Result))
                        {
                            syslog(LOG_ERR,"[%d]Write Reboot file Fail",__LINE__);
                        }

                        Result = GMI_BrdHwReset();
                        if (FAILED(Result))
                        {
                            syslog(LOG_ERR,"[%d]GMI_BrdHwReset Call Fail",__LINE__);
                        }
                        else if (SUCCEEDED(Result))
                        {
                            syslog(LOG_ERR,"[%d]System Phy is not Exist, GMI_BrdHwReset  for Watchdog",__LINE__);
                        }
                    }
                }
            }
        }
        else
        {
            //Phy is not exist
            syslog(LOG_ERR,"[%d]Get  System   Phy  Fail",__LINE__);
        }
        sleep(30);
    }

    closelog();
    return 0;
}
