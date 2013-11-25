
#ifndef __DAEMON_H__
#define __DAEMON_H__

#include "gmi_system_headers.h"

GMI_RESULT DaemonRegister(void);

    void DaemonUnregister(void);

    GMI_RESULT DaemonKeepAlive(uint32_t *BootFlag);
    
    GMI_RESULT DaemonQueryServerStatus(int32_t ServerId, uint16_t *StatusPtr);

    GMI_RESULT DaemonReboot(void);

//DelayTime:uint s
GMI_RESULT DaemonReboot(uint16_t DelayTime);
GMI_RESULT DaemonReportIpChanged(void);
GMI_RESULT DaemonReportIpChanged(int32_t EthId, uint16_t DelayTime);

#endif


