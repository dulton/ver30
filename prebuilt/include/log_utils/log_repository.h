#pragma once

#include "gmi_system_headers.h"
#include "log_handler.h"

#define LOG_REPOSITORY_DATA_TYPE_STRING_REFERENCE  1
#define LOG_REPOSITORY_DATA_TYPE_STRING            2

struct LogRepositoryInitializationParameter
{
    union FilePath_t
    {
        const char_t    *u_FilePathReference;
        char_t           u_FilePath[MAX_PATH_LENGTH];
    };

    uint32_t             s_UserLogFilePathType;
    union FilePath_t     s_UserLogFilePath;
    LogStorageLimitMode  s_UserLogStorageLimitMode;
    uint32_t             s_UserLogStorageLimitParameter;
    long_t               s_UserLogShareMemoryKey;
    uint32_t             s_UserLogShareMemorySize;
    long_t               s_UserLogIpcMutexKey;

    uint32_t             s_DebugLogFilePathType;
    union FilePath_t     s_DebugLogFilePath;
    LogStorageLimitMode  s_DebugLogStorageLimitMode;
    uint32_t             s_DebugLogStorageLimitParameter;
    long_t               s_DebugLogShareMemoryKey;
    uint32_t             s_DebugLogShareMemorySize;
    long_t               s_DebugLogIpcMutexKey;
};

class LogRepository : public GMI_LogHandler
{
public:
    LogRepository(void);
    virtual ~LogRepository(void);

    virtual GMI_RESULT  Initialize( const void_t *Parameter, size_t ParameterLength );
    virtual GMI_RESULT  Deinitialize();
    virtual GMI_RESULT  UserLog( uint16_t Type, uint16_t Subtype, uint64_t LogTime, const char_t *UserName, uint32_t UserNameLength, const void_t *SpecificData, uint32_t SpecificDataLength );
    virtual GMI_RESULT  DebugLog( uint16_t Level, uint64_t LogTime, uint64_t ProcessId, uint64_t ThreadId, uint32_t ModuleId, const char_t *ModuleName, const char_t *FileName, const char_t *FunctionName, uint32_t LineNumber, const void_t *SpecificData, uint32_t SpecificDataLength );

private:
    GMI_RESULT  GetLogFileName( char_t *LogFileName, long_t FileNameBufferLength );
    GMI_RESULT  GetUserLogFullPath( char_t *LogFullPath, long_t FullPathBufferLength, const char_t *LogPathName, const char_t *LogFileName );
    GMI_RESULT  GetDebugLogFullPath( char_t *LogFullPath, long_t FullPathBufferLength, const char_t *LogPathName, const char_t *LogFileName );

private:
    LogRepositoryInitializationParameter  m_InitializationgParameter;
};
