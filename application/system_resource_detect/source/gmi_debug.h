#ifndef _GMI_DEBUG_H_
#define _GMI_DEBUG_H_

#include "gmi_system_headers.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOCALFMT				100
#define MAX_LOG_LINE			4000
#define MAX_BUFFER_LENGTH           255


#define LOG_FILE1				"/opt/log/system_resource_detect_log"
#define LOG_FILE1_OLD			"/opt/log/system_resource_detect_log_old"


    /*==============================================================
    name				:	GMI_DeBugPrint
    function			:  Write debug log to file
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     format.
    return				:    no
    ******************************************************************************/
    void GMI_DeBugPrint(const char *format, ...) ;

    /*==============================================================
    name				:	GMI_DebugLog2FileInitial
    function			:  Debug log file to file
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     no
    return				:    no
    ******************************************************************************/
    void GMI_DebugLog2FileInitial();


#ifdef __cplusplus
}
#endif

#endif

