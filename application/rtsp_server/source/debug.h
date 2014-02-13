#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>

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

#define PRINT_LOG(lv, ...) \
    do { \
        printf(COLOR_##lv"%7s %20s line %05d: ", #lv, __FILE__, __LINE__); \
        printf(__VA_ARGS__); \
        printf(DISP_RESET"\n"); \
    } while(0)

#define HALT *(int *)NULL = 0

#define ASSERT(cond, ...) \
    do { \
        if (cond) break; \
        PRINT_LOG(ASSERT, "[%s] MUST be true", #cond); \
        PRINT_LOG(ASSERT, __VA_ARGS__); \
        HALT; \
    } while(0)

#endif // __DEBUG_H__

