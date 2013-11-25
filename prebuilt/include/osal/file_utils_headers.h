// file_utils_headers.h
#if !defined( GMI_FILE_UTILS_HEADERS )
#define GMI_FILE_UTILS_HEADERS

#if defined( __linux__ )

#include "linux_directory.h"
#include "linux_file.h"

typedef LinuxDirectory     GMI_Directory;
typedef LinuxFile          GMI_File;

#elif defined( _WIN32 )

#include "windows_directory.h"
#include "windows_file.h"

typedef WindowsDirectory   GMI_Directory;
typedef WindowsFile        GMI_File;

#else
#error not support current platform
#endif

#endif//GMI_FILE_UTILS_HEADERS
