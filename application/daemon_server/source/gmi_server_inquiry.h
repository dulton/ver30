#ifndef _GMI_SERVER_INQUIRY_H_
#define _GMI_SERVER_INQUIRY_H_

#include "gmi_system_headers.h"

#ifdef __cplusplus
extern "C" {
#endif

    /*=======================================================
    name				:	GMI_InitServerQuiry
    function			:  Init ServerQuiry resource thread
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     no
    return				:    FAIL:  -1
                                         SUCCESS : 0
    ******************************************************************************/
    GMI_RESULT GMI_InitServerQuiry(void);

    /*=======================================================
    name				:	GMI_UnInitServerQuiry
    function			:  UnInit ServerQuiry resource
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     no
    return				:    FAIL:  -1
                                         SUCCESS : 0
    ******************************************************************************/
    void GMI_UnInitServerQuiry(void);

#ifdef __cplusplus
}
#endif

#endif


