#ifndef GMI_CONFIG_H
#define GMI_CONFIG_H

#include "gmi_system_headers.h"

#ifdef __cplusplus
extern "C" {
#endif

    /*==============================================================
    name				:	GMI_LoadCfg
    function			:  load daemon application config
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     no
    return				:    FAIL:  -1
                                        SUCCESS : 0
    ******************************************************************************/
    GMI_RESULT GMI_LoadApplicationCfg(void);

    /*==============================================================
    name				:	GMI_MemoryAllocForLoadCFG
    function			:  System Memory alloc
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     no
    return				:    FAIL:  -1
                                         SUCCESS : 0
    ******************************************************************************/
    GMI_RESULT GMI_MemoryAllocForLoadCFG(void);

    /*==============================================================
    name				:	MemoryFreeForLoadCFG
    function			:  System Free Memory
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     no
    return				:
    ******************************************************************************/
    void MemoryFreeForLoadCFG(void);

#ifdef __cplusplus
}
#endif

#endif /* GMI_CONFIG_H */
