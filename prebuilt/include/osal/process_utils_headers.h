// process_utils_headers.h
#if !defined( GMI_PROCESS_UTILS_HEADERS )
#define GMI_PROCESS_UTILS_HEADERS

#if defined( __linux__ )

#include "linux_anonymous_pipe.h"
#include "linux_event_ipc.h"
#include "linux_message_queue.h"
#include "linux_mutex_ipc.h"
#include "linux_named_pipe.h"
#include "linux_process.h"
#include "linux_semaphore_ipc.h"
#include "linux_share_memory.h"

typedef LinuxAnonymousPipe     GMI_AnonymousPipe;
typedef LinuxEventIPC          GMI_EventIPC;
typedef LinuxMessageQueue      GMI_MessageQueue;
typedef LinuxMutexIPC          GMI_MutexIPC;
typedef LinuxNamedPipe         GMI_NamedPipe;
typedef LinuxProcess           GMI_Process;
typedef LinuxSemaphoreIPC      GMI_SemaphoreIPC;
typedef LinuxShareMemory       GMI_ShareMemory;

#elif defined( _WIN32 )

#include "windows_anonymous_pipe.h"
#include "windows_event_ipc.h"
#include "windows_message_queue.h"
#include "windows_mutex_ipc.h"
#include "windows_named_pipe.h"
#include "windows_process.h"
#include "windows_semaphore_ipc.h"
#include "windows_share_memory.h"

typedef WindowsAnonymousPipe   GMI_AnonymousPipe;
typedef WindowsEventIPC        GMI_EventIPC;
typedef WindowsMessageQueue    GMI_MessageQueue;
typedef WindowsMutexIPC        GMI_MutexIPC;
typedef WindowsNamedPipe       GMI_NamedPipe;
typedef WindowsProcess         GMI_Process;
typedef WindowsSemaphoreIPC    GMI_SemaphoreIPC;
typedef WindowsShareMemory     GMI_ShareMemory;

#else
#error not support current platform
#endif

#define GMI_GetCurrentProcessId BaseProcess::GetCurrentId

#endif//GMI_PROCESS_UTILS_HEADERS
