// thread_utils_headers.h
#if !defined( GMI_THREAD_UTILS_HEADERS )
#define GMI_THREAD_UTILS_HEADERS

#if defined( __linux__ )

#include "linux_event.h"
#include "linux_mutex.h"
#include "linux_reader_writer_lock.h"
#include "linux_semaphore.h"
#include "linux_thread.h"

typedef LinuxEvent              GMI_Event;
typedef LinuxMutex				GMI_Mutex;
typedef LinuxReaderWriterLock   GMI_ReaderWriterLock;
typedef LinuxSemaphore	        GMI_Semaphore;
typedef LinuxThread             GMI_Thread;

#elif defined( _WIN32 )

#include "windows_event.h"
#include "windows_mutex.h"
#include "windows_reader_writer_lock.h"
#include "windows_semaphore.h"
#include "windows_thread.h"

typedef WindowsEvent            GMI_Event;
typedef WindowsMutex            GMI_Mutex;
typedef WindowsReaderWriterLock GMI_ReaderWriterLock;
typedef WindowsSemaphore        GMI_Semaphore;
typedef WindowsThread           GMI_Thread;

#else
#error not support current platform
#endif

#include "thread_auxiliary.h"

#define GMI_GetCurrentThreadId BaseThread::GetCurrentId

#endif//GMI_THREAD_UTILS_HEADERS
