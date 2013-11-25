#ifndef _GMI_SYSTEM_H__
#define _GMI_SYSTEM_H__

#include "gmi_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

    /*=======================================================
    name				:	GMI_ApplicationQuit
    function			:  Application Daemon exit funciton
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     no
    return				:    no
    ******************************************************************************/
    void GMI_ApplicationQuit(int32_t AppId);

    /*=======================================================
    name				:	GMI_ApplicationUpdateQuit
    function			:  Application Daemon exit funciton
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     no
    return				:    no
    ******************************************************************************/
    void GMI_ApplicationUpdateQuit(void);

    /*=======================================================
    name				:	GMI_StartUpSystemService
    function			:  Application Daemon exit funciton
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     no
    return				:    no
    ******************************************************************************/
    GMI_RESULT GMI_StartUpSystemService(const SystemApplicationCfg_t *SystemApplicationCfg, int32_t SequenceId);

    /*=======================================================
    name				:	GMI_ApplicationDaemonExit
    function			:  Application Daemon exit funciton
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     no
    return				:    no
    ******************************************************************************/
    void GMI_ApplicationDaemonExit(void);

    /*==============================================================
    name				:	GMI_SystemInitial
    function			:  System initial function
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     no
    return				:    FAIL:  ERROR
                                         SUCCESS : OK
    ******************************************************************************/
    GMI_RESULT GMI_SystemInitial(void);

    /*==============================================================
    name				:	GMI_SystemUninitial
    function			:  System uninit function
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     no
    return				:    FAIL:  ERROR
                                         SUCCESS : OK
    ******************************************************************************/
    void GMI_SystemUninitial(void);

#ifdef __cplusplus
}
#endif

#endif

