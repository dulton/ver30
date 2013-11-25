#ifndef __DISCOVERY_H__
#define __DISCOVERY_H__

#include "gmi_system_headers.h"

#define TYPE_HELLO        0x00
#define TYPE_BYE          0x01

#ifdef __cplusplus
extern "C"
{
#endif

    GMI_RESULT DevStatusVaryNotifyService(int NotifyType);

    GMI_RESULT OnvifDevProbeServiceStart(void);

    GMI_RESULT OnvifDevProbeServiceStop();


#ifdef __cplusplus
}
#endif

#endif

