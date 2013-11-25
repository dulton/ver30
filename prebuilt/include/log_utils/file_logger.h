#pragma once

#include "gmi_system_headers.h"

#define FILE_LOGGER_MAX_FILE_SIZE   (1024*1024)
#define FILE_LOGGER_MAX_LOG_BUFFER  4096

class FileLogger
{
public:
    FileLogger(void);
    ~FileLogger(void);

    GMI_RESULT	Initialize( uint32_t ModuleId, const char_t *ModuleName, const char_t *LogFilePath );
    GMI_RESULT  Deinitialize();

    GMI_RESULT  DebugLog( uint16_t Level, const char_t *FileName, const char_t *FunctionName, uint32_t LineNumber, const void_t *SpecificData, uint32_t SpecificDataLength );
    GMI_RESULT  DebugLogV( uint16_t Level, const char_t *FileName, const char_t *FunctionName, uint32_t LineNumber, const char_t *Format, ... );

private:
    GMI_RESULT  GetSystemTime( uint32_t& Second, uint32_t& Microsecond );

    GMI_RESULT  GetLogFileName( char_t *LogFileName, long_t FileNameBufferLength );
    GMI_RESULT  GetDebugLogFullPath( char_t *LogFullPath, long_t FullPathBufferLength, const char_t *LogPathName, const char_t *LogFileName, const char_t *ModuleName );

private:
    uint32_t                                  m_ModuleId;
    SafePtr< char_t, DefaultObjectsDeleter >  m_ModuleName;
    SafePtr< char_t, DefaultObjectsDeleter >  m_FileName;
    GMI_Mutex                                 m_OperationLock;
    GMI_File                                  m_LogFile;
    char_t                                    m_LogBuffer[FILE_LOGGER_MAX_LOG_BUFFER];
};

#if !defined( DEBUG_LOG_DEFINITION )
#define DEBUG_LOG_DEFINITION 1
#define USER_LOG( Logger, LogLevel, Id, UserName, UserNameLength, SpecificData, SpecificDataLength )  if (NULL!=Logger) (Logger)->UserLog( LogLevel, Id, UserName, UserNameLength, SpecificData, SpecificDataLength )
#define DEBUG_LOG( Logger, LogLevel, Format, ... )                                                    if (NULL!=Logger) (Logger)->DebugLogV( LogLevel, __FILE__, __FUNCTION__, __LINE__, Format, ##__VA_ARGS__ )
#endif//DEBUG_LOG_DEFINITION

extern  FileLogger* g_DefaultFileLogger;
