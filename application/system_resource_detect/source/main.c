#include "machine.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "gmi_system_headers.h"
#include "gmi_debug.h"
#include "gmi_brdwrapperdef.h"
#include "gmi_brdwrapper.h"

//#define SYSTEM_DEBUG_LOG
#define NETWORK_DOWN      0
#define NETWORK_UP            1
#define GMI_NETWORK_RX_PACKETS_CMD "ifconfig eth0 | grep \"RX packets:\" | awk '{print $2}' | tr -d 'packets:'"

int machine_init(struct statics * statics);
void get_system_info(struct system_info *info);
GMI_RESULT GMI_GetNetWorkRxPackets(const char_t *Cmd, char_t *Buf, int32_t Length)
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

int32_t main(void)
{
    struct statics Stat;
    struct system_info Info;

    machine_init(&Stat);
    GMI_DebugLog2FileInitial();

    int32_t IdleCpu = 0;
    int32_t TmpLen=0;
    int32_t RxLength=0;
    GMI_RESULT  Result = GMI_FAIL;
    char_t RxBuffer[8];
    uint8_t Link = 0;
    uint8_t EthId = 0;

    for(;;)
    {
        get_system_info(&Info);
#ifdef SYSTEM_DEBUG_LOG
        printf("Used CPU:%.1f%%\n",(float_t)Info.cpustates[0]/10);
        printf("Nice CPU:%.1f%%\n",(float_t)Info.cpustates[1]/10);
        printf("System CPU:%.1f%%\n",(float_t)Info.cpustates[2]/10);
        printf("Idle CPU:%.1f%%\n",(float_t)Info.cpustates[3]/10);
        printf("total memroy:%d\n", Info.memory[0]);
        printf("free memroy:%d\n", Info.memory[1]);
        printf("buffers:%d\n", Info.memory[2]);
        printf("cached:%d\n", Info.memory[3]);
        printf("total swap:%d\n", Info.memory[4]);
        printf("free swap:%d\n", Info.memory[5]);
#endif
        IdleCpu = Info.cpustates[3]/10;
        if(IdleCpu < 20)
        {
            GMI_DeBugPrint("System  CPU  Use  Is  %d %",(100-IdleCpu));
        }

        if(Info.memory[1] < 21000)
        {
            GMI_DeBugPrint("System Free Memroy Is  %d Kb",Info.memory[1]);
        }

        Result  = GMI_BrdGetEthLinkStat(EthId, &Link);
        if (SUCCEEDED(Result))
        {
            if (NETWORK_DOWN == Link)
            {
                GMI_DeBugPrint("System NetWork Status Is Down");
            }
            else if ( NETWORK_UP == Link )
            {
                memset(RxBuffer, 0 ,sizeof(RxBuffer));
                Result = GMI_GetNetWorkRxPackets(GMI_NETWORK_RX_PACKETS_CMD, RxBuffer, sizeof(RxBuffer));
                if (SUCCEEDED(Result))
                {
                    TmpLen = atoi(RxBuffer) - RxLength;
                    RxLength = atoi(RxBuffer);
                    if (0 == TmpLen)
                    {
                        GMI_DeBugPrint("System NetWork Status Is UP ,But recieve packets number is 0");
                    }
                }
                else
                {
                    GMI_DeBugPrint("System  GetNetWorkRxPackets Fail");
                }
            }
        }
        else
        {
            GMI_DeBugPrint("System  GetEthLinkStat Fail");
        }

        sleep(2);
    }
    return 0;
}
