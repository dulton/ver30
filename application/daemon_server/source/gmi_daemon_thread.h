#ifndef _DAEMON_THREAD_H_
#define _DAEMON_THREAD_H_

#include "gmi_system_headers.h"

#ifdef __cplusplus
extern "C" {
#endif

    /*=======================================================
    name				:	GMI_DaemonInit
    function			:  Init Daemon thread
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     no
    return				:    FAIL:  -1
                                         SUCCESS : 0
    ******************************************************************************/
    GMI_RESULT GMI_DaemonInit(void);

    /*=======================================================
    name				:	GMI_DaemonUnInit
    function			:  UnInit Daemon thread,free resource
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     no
    return				:    no
    ******************************************************************************/
    void GMI_DaemonUnInit(void);


#ifdef __cplusplus
}
#endif

#endif

