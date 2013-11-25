#pragma once

#include "file_logger.h"
#include "gmi_system_headers.h"
#include "log_header.h"

#if !defined( USER_LOG_CALLBACK )
#define USER_LOG_CALLBACK
typedef void_t (*UserLogCallback)( uint16_t Type, uint16_t Subtype, uint64_t Time, const char_t *UserName, uint32_t UserNameLength, const void_t *SpecificData, uint32_t SpecificDataLength );
#endif//USER_LOG_CALLBACK

#if !defined( DEBUG_LOG_CALLBACK )
#define DEBUG_LOG_CALLBACK
typedef void_t (*DebugLogCallback)( uint16_t Level, uint64_t Time, uint64_t ProcessId, uint64_t ThreadId, uint32_t ModuleId, const char_t *ModuleName,
                                    const char_t *FileName, const char_t *FunctionName, uint32_t LineNumber, const void_t *SpecificData, uint32_t SpecificDataLength );
#endif//DEBUG_LOG_CALLBACK

class PipeSession;

class LogClient
{
public:
    LogClient(void);
    ~LogClient(void);

    GMI_RESULT	Initialize( uint32_t ModuleId, const char_t *ModuleName, const char_t *ModulePipeName, long_t ModulePipeMutexId, const char_t *PeerPipeName, long_t PeerPipeMutexId, const char_t *ServerPipeName, long_t ServerPipeMutexId, const char_t *LogFilePath,
                            uint32_t DebugLogLevel );
    GMI_RESULT  Deinitialize();

    GMI_RESULT  UserLog( uint16_t Type, uint16_t Subtype, const char_t *UserName, uint32_t UserNameLength, const void_t *SpecificData, uint32_t SpecificDataLength );
    GMI_RESULT  DebugLog( uint16_t Level, const char_t *FileName, const char_t *FunctionName, uint32_t LineNumber, const void_t *SpecificData, uint32_t SpecificDataLength );
    GMI_RESULT  DebugLogV( uint16_t Level, const char_t *FileName, const char_t *FunctionName, uint32_t LineNumber, const char_t *Format, ... );

    GMI_RESULT  ConfigLog( enum LogType Type, enum LogTransportMode TransportMode, uint16_t StorageMediaType, enum LogStorageLimitMode StorageLimitMode, uint32_t StorageLimitParameter );
    GMI_RESULT  SetupDebugLog( boolean_t Enable, uint16_t WarningLevel, uint32_t BufferingRecordNumber );
    GMI_RESULT  QueryUserLog( uint16_t Type, uint16_t Subtype, uint64_t StartTime, uint64_t EndTime, uint8_t *Reply, uint32_t *ReplyLength );
    GMI_RESULT  QueryDebugLog( uint64_t StartTime, uint64_t EndTime, uint8_t *Reply, uint32_t *ReplyLength );

private:
    GMI_RESULT  GetSystemTime( uint32_t& Second, uint32_t& Microsecond );

private:
    FileLogger                                                          m_LogClientOperationFile;
    boolean_t                                                           m_LogClientOperationFileOwner;
    uint32_t                                                            m_ModuleId;
    SafePtr< char_t, DefaultObjectsDeleter >                            m_ModuleName;
    SafePtr< char_t, DefaultObjectsDeleter >                            m_ServerPipeName;
    long_t                                                              m_ServerPipeMutexId;
    uint32_t                                                            m_DebugLogLevel;

    ReferrencePtr<PipeSession,DefaultObjectDeleter,MultipleThreadModel> m_PipeSession;
    uint16_t                                                            m_SequenceNumber;
    char_t                                                              m_LogBuffer[FILE_LOGGER_MAX_LOG_BUFFER];
};

#if !defined( DEBUG_LOG_DEFINITION )
#define DEBUG_LOG_DEFINITION 1
#define USER_LOG( Client, LogLevel, Id, UserName, UserNameLength, SpecificData, SpecificDataLength )  if (NULL!=Client) (Client)->UserLog( LogLevel, Id, UserName, UserNameLength, SpecificData, SpecificDataLength )
#define DEBUG_LOG( Client, LogLevel, Format, ... )                                                    if (NULL!=Client) (Client)->DebugLogV( LogLevel, __FILE__, __FUNCTION__, __LINE__, Format, ##__VA_ARGS__ )
#endif//DEBUG_LOG_DEFINITION

extern  LogClient* g_DefaultLogClient;
