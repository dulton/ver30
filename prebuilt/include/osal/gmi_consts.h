// gmi_consts.h
// define const for some module
#if !defined( GMI_CONSTS )
#define GMI_CONSTS

#include "gmi_type_definitions.h"

#if defined( __linux__ )
#define TIMEOUT_INFINITE    (long_t)0xFFFFFFFF
#elif defined( _WIN32 )
#define TIMEOUT_INFINITE    (long_t)INFINITE
#else
#error not support current platform
#endif

// 2147483647L on windows
#define LONG_MAX_VALUE      (long_t)0x7FFFFFFF
#define INVALID_FD          (long_t)0xFFFFFFFF
#define MAX_PATH_LENGTH     512
#define INVALID_SIZE        -1

#endif//GMI_CONSTS
