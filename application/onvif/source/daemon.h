#ifndef __DAEMON_H__
#define __DAEMON_H__

#include "gmi_system_headers.h"

#ifdef __cpluscplus
extern "C"
{
#endif

    GMI_RESULT DaemonRegister(void);

    void DaemonUnregister(void);

    GMI_RESULT DaemonStart();

    void DaemonStop();


#ifdef __cpluscplus
}
#endif

#endif

