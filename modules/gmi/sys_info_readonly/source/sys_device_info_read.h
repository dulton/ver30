#ifndef __SYS_DEVICE_INFO_READ_H__
#define __SYS_DEVICE_INFO_READ_H__

#include <pthread.h>
#include "file_lock.h"

typedef struct tagFileInfo
{
   char_t s_Name[128];
   pthread_mutex_t s_Lock;  
   boolean_t       s_Created; //true -- filelock have init, false --filelock not init
   CSemaphoreMutex s_FileLock;   
}SI_FileInfo;


typedef struct tagSysInfoReadHandle
{
	FD_HANDLE    s_XmlHandle;
	SI_FileInfo  *s_FileInfo;	
}SysInfoReadHandle;


#endif


