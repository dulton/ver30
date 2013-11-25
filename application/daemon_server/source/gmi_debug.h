#ifndef _GMI_DEBUG_H_
#define _GMI_DEBUG_H_

#include "gmi_system_headers.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DISP_BLACK      "\033[30m"
#define DISP_RED        "\033[31m"
#define DISP_GREEN      "\033[32m"
#define DISP_YELLOW     "\033[33m"
#define DISP_BLUE       "\033[34m"
#define DISP_PURPLE     "\033[35m"
#define DISP_LIGHT_BLUE "\033[36m"
#define DISP_WHITE      "\033[37m"
#define DISP_RESET      "\033[0m"

#define COLOR_VERBOSE   DISP_BLUE
#define COLOR_ASSERT    DISP_RED
#define COLOR_ERROR     DISP_PURPLE
#define COLOR_WARNING   DISP_YELLOW
#define COLOR_INFO      DISP_GREEN


//#define DEBUG_LOG
#ifdef DEBUG_LOG
#define DAEMON_PRINT_LOG(lv, args...) \
     do { \
    	 printf(COLOR_##lv"%7s %20s line %05d: ", #lv, __FILE__, __LINE__); \
    	 printf(args); \
    	 printf(DISP_RESET"\n"); \
     } while(0)
#else
#define DAEMON_PRINT_LOG(lv, ...) \
     do { \
     } while(0)
#endif

#define LOCALFMT				100
#define MAX_LOG_LINE			1000

#define LOG_FILE1				"/opt/log/daemon_log_1"
#define LOG_FILE1_OLD			"/opt/log/daemon_log_1_old"


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

