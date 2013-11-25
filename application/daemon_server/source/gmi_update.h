#ifndef _GMI_UPDATE_H_
#define _GMI_UPDATE_H_

#include "gmi_system_headers.h"
#include "gmi_struct.h"


#ifdef __cplusplus
extern "C" {
#endif

#include "gmi_includes.h"

#define NETWORK_TIMEOUT	       0x0000000AUL

//#define UPDATE_SPACE_FILE   "/opt/bin/"
#define UPDATE_SPACE_FILE   "/tmp/"
#define UPDATE_KERNEL_BIN  "/opt/bin/Update.sh  kernel"
#define UPDATE_ROOTFS_BIN  "/opt/bin/Update.sh  rootfs"
#define UPDATE_APP_BIN        "/opt/bin/Update.sh  user"

#define SYSTEM_PLATFORM_AMBA   "amba"

#define GMI_UPDATE_CMD_LIST                        0
#define GMI_UPDATE_CMD_REBOOT                  1
#define GMI_UPDATE_CMD_GET_INFO               2

#define GMI_UPDATE_CMD_OK                        1
#define GMI_UPDATE_CMD_ERROR                   0

#define GMI_UPDATE_START_FLAGS                  1
#define GMI_UPDATE_END_FLAGS                    0

#define  GMI_UPDATE_FILE_TYPE_KENREL           0
#define  GMI_UPDATE_FILE_TYPE_ROOTFS          1
#define  GMI_UPDATE_FILE_TYPE_APP		2
#define  GMI_UPDATE_FILE_TYPE_UBOOT		3
#define  GMI_UPDATE_FILE_TYPE_PTZ 		4
#define  GMI_UPDATE_FILE_TYPE_NOTSUPPORT  5


#define GMI_REPORT_UPDATE_OK                     1
#define GMI_REPORT_UPDATE_ERROR                20001025

#define GMI_UPDATE_STATUS_DOWNLOAD  	0
#define GMI_UPDATE_STATUS_UPDATING  	        1
#define GMI_UPDATE_STATUS_UPDATED		2
#define GMI_UPDATE_STATUS_REBOOTING 	3
#define GMI_UPDATE_STATUS_RETRY			4
#define GMI_UPDATE_STATUS_DISCONNECT	5
#define GMI_UPDATE_STATUS_DONE	                6
#define GMI_UPDATE_STATUS_UPDATE_ERROR	7
#define GMI_UPDATE_STATUS_MESSAGE_ERROR	8


#define MAX_MSG_LEN      0x00000800UL  //2048

    /*=======================================================
    name				:	GMI_SystemUpdateServerInit
    function			:  Create System update Server
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     no
    return				:    no
    ---------------------------------------------------------------
    modification	:
    	date		version		 author 	modification
    	7/10/2013	1.0.0.0.0     minchao.wang         establish
    ******************************************************************************/
    GMI_RESULT  GMI_SystemUpdateServerInit(void);


#ifdef __cplusplus
}
#endif

#endif

