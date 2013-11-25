// cross_platform_headers.h
// define cross-platform header file
#if !defined( CROSS_PLATFORM_HEADERS )
#define CROSS_PLATFORM_HEADERS

#include <assert.h>

#if defined( __cplusplus )
#include <deque>
#include <list>
#include <map>
#include <queue>
#include <stack>
#include <stdarg.h>
#include <vector>
#endif

#if defined( __linux__ )

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#elif defined( _WIN32 )

#include <errno.h>
#include <stdio.h>
#if !defined( _WIN32_WCE )
#include <sys/timeb.h>
#endif
#include <time.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#else
#error not support current platform
#endif

#include "gmi_auxiliary_macros.h"
#include "gmi_consts.h"
#include "gmi_errors.h"
#include "gmi_type_definitions.h"

#endif//CROSS_PLATFORM_HEADERS
