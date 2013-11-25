// thread_auxiliary.h
#if !defined( GMI_THREAD_AUXILIARY )
#define GMI_THREAD_AUXILIARY

#include "cross_platform_headers.h"

#if defined( __linux__ )

#define GMI_Sleep(x) usleep(1000*x)

#elif defined( _WIN32 )

#define GMI_Sleep(x) ::Sleep(x)

#else
#error not support current platform
#endif

#endif//GMI_THREAD_AUXILIARY
