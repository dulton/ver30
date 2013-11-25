#ifndef _HEARDBEAT_H_
#define _HEARDBEAT_H_

#include "gmi_system_headers.h"
#include "gmi_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

    extern pthread_mutex_t g_LockRudpAuthPort;
    extern pthread_mutex_t g_LockRudpLogPort;
    extern pthread_mutex_t g_LockRudpMediaPort;
    extern pthread_mutex_t g_LockRudpSystemPort;
    extern pthread_mutex_t g_LockRudpOnvifPort;
    extern pthread_mutex_t g_LockRudpGBPort;
    extern pthread_mutex_t g_LockRudpSdkPort;
    extern pthread_mutex_t g_LockRudpWebPort;
    extern pthread_mutex_t g_LockRudpTransportPort;
    extern pthread_mutex_t g_LockRudpConfigToolPort;


    GMI_RESULT GMI_SystemAppRunning(const SystemApplicationCfg_t *SystemApplicationCfg, ApplicationRegisterFlags *AppRegisterFlags, int32_t SequenceId);
    /*=======================================================
    name				:	InitHeardbeat
    function			:  Init heardbeat for message quene
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:
    return				:    0:success
                                           -1: fail
    ******************************************************************************/
    FD_HANDLE GMI_InitHeartbeat();

    /*=======================================================
    name				:	GMI_QueryHeardbeatMassge
    function			:  Init heardbeat for message quene
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     AppId : daemon application Id,Define heardbeat.h file
                                                  SendKey: return value;
                                                  ReadKey: return value
    return				:    0:success
                                           -1: fail
    ******************************************************************************/
    GMI_RESULT GMI_QueryHeartbeatMassge(FD_HANDLE SockFd);

    /*=======================================================
    name				:	GMI_UnInitHeartbeat
    function			:  Init heardbeat for message quene
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     AppId : daemon application Id,Define heardbeat.h file
                                                  SendKey: return value;
                                                  ReadKey: return value
    return				:    0:success
                                           -1: fail
    ******************************************************************************/
    void GMI_UnInitHeartbeat(FD_HANDLE SockFd);

    /*=======================================================
    name				:	GMI_InitSystemHeard
    function			:  Get Message key Id
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:
    return				:   no
    ******************************************************************************/
    FD_HANDLE GMI_InitSystemHeard();

#ifdef __cplusplus
}
#endif

#endif

