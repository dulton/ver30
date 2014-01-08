#pragma once

#include "gmi_system_headers.h"

#define USER_LOG_SOURCE_TYPE_FILE_PATH_REFERENCE  1
#define USER_LOG_SOURCE_TYPE_FILE_PATH            2
#define USER_LOG_SOURCE_TYPE_SHARE_MEMORY         4

struct UserLogQueryerInitializationParameter
{
    union LogSource_t
    {
        const char_t   *u_FilePathReference;
        char_t         u_FilePath[MAX_PATH_LENGTH];
        long_t         u_ShareMemoryKey;
    };

    uint32_t           s_SourceType;
    union LogSource_t  s_Source;
    long_t             s_SourceIpcMutexKey;
};

#define USER_LOG_NAME_MAX_LENGTH           32
#define USER_LOG_SPECIFIC_DATA_MAX_LENGTH  128
// used to save to local storage and provided to IPC sdk
struct UserLogStorageInfo
{
    uint64_t  s_Index;// used to identify log item, and increased monotonously and can be reversed back to reuse.
    uint16_t  s_Type;
    uint16_t  s_Subtype;
    uint32_t  s_Reserved;
    uint64_t  s_LogTime;
    char_t    s_UserName[USER_LOG_NAME_MAX_LENGTH];
    uint8_t   s_SpecificData[USER_LOG_SPECIFIC_DATA_MAX_LENGTH];
};

// log file meta data
#define USER_LOG_META_DATA_SIZE        4096
#define USER_LOG_VALID_META_DATA_SIZE  16
struct UserLogMetaData
{
    uint64_t  s_LogNumber;
    uint64_t  s_LatestLogIndex;
};

class UserLogQueryer
{
public:
    UserLogQueryer(void);
    virtual ~UserLogQueryer(void);

    GMI_RESULT  Initialize   ( const UserLogQueryerInitializationParameter *Parameter, size_t ParameterLength );
    GMI_RESULT  Deinitialize ();
    GMI_RESULT  Query( uint16_t Type, uint16_t Subtype, uint64_t StartTime, uint64_t EndTime, UserLogStorageInfo *UserLogBuffer, uint32_t *UserLogNumber );

private:
    GMI_RESULT  LookForUserLog ( const uint8_t *Buffer, uint64_t BufferLength, uint16_t Type, uint16_t Subtype, uint64_t StartTime, uint64_t EndTime, UserLogStorageInfo *UserLogBuffer, uint32_t *UserLogNumber );

private:
    UserLogQueryerInitializationParameter      m_InitializationgParameter;
    GMI_MutexIPC                               m_OperationLock;
    SafePtr< uint8_t, DefaultObjectsDeleter >  m_UserLogBuffer;
    uint64_t                                   m_UserLogBufferLength;
};
