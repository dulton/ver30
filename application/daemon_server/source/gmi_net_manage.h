#ifndef _GMI_NET_MANAGE_H_
#define _GMI_NET_MANAGE_H_

#include "gmi_system_headers.h"
#include "gmi_netconfig_api.h"


#ifdef __cplusplus
extern "C" {
#endif

    /*============================================================================
    name				:	GMI_NetManageInit
    function			:  Read System config file ,if no config file.
                             Use default value  and active system network.
    algorithm implementation	:	no
    global variable 		:	no
    parameter declaration	:    no
    return				:	 success: GMI_SUCCESS
    						    fail; ERROR CODE
    ******************************************************************************/
    GMI_RESULT  GMI_NetManageInit(void);


#ifdef __cplusplus
}
#endif

#endif


