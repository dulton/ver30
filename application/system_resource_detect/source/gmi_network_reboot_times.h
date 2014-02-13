#ifndef GMI_NETWORK_REBOOT_TIMES_H
#define GMI_NETWORK_REBOOT_TIMES_H

#include "gmi_system_headers.h"

#define NETWORK_REBOOT_FILE                 "/opt/log/networkTimes"
#define BUFFER_LENGTH                               64
#define NETWORK_DOWN      0
#define NETWORK_UP            1
#define NETWORK_DETECT_NUMBER_MAX   10
#define NETWORK_DETECT_INTERRUPTS_MAX   5
#define NETWORK_INTERRUPTS_NUMBER_MAX  300000
#define GMI_NETWORK_RX_PACKETS_CMD "ifconfig eth0 | grep \"RX packets:\" | awk '{print $2}' | tr -d 'packets:'"
#define GMI_NETWORK_TX_PACKETS_CMD "ifconfig eth0 | grep \"TX packets:\" | awk '{print $2}' | tr -d 'packets:'"
#define GMI_NETWORK_DEV_CMD  "ifconfig | grep \"eth0\" | awk '{print $1}'"
#define GMI_NETWORK_INTERRUPTS_CMD "cat /proc/interrupts | grep eth0 | awk '{print $2}'"
#define GMI_NETWORK_RESTART "/etc/init.d/S40network restart &"
#define GMI_CONFIT_TOOL_RESTART "killall -9  config_tool"


typedef struct _SystemNetWorkRebootTimes
{
    uint8_t    s_Times;			   //IP   Addr
    uint8_t     s_Reserve[3];	  //Mask Addr
    uint32_t     s_RebootTime;
} SystemNetWorkRebootTimes;


#ifdef __cplusplus
extern "C" {
#endif

    GMI_RESULT GMI_ReadRebootTimes(SystemNetWorkRebootTimes *NetWorkRebootTimes);

    GMI_RESULT GMI_WriteRebootTimes(const SystemNetWorkRebootTimes *NetWorkRebootTimes);

    GMI_RESULT GMI_GetNetWorkPackets(const char_t *Cmd, char_t *Buf, int32_t Length);

    GMI_RESULT GMI_FileExists(const char_t *FileName);

    GMI_RESULT GMI_NetWorkDevCheck(boolean_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* GMI_CONFIG_H */

