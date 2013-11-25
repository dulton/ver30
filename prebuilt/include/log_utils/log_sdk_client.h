#pragma once

#include "application_packet.h"
#include "file_logger.h"
#include "gmi_system_headers.h"
#include "log_header.h"
#include "log_tcp_session.h"

#define FILE_LOGGER_CLIENT_MODULE_ID   0
#define FILE_LOGGER_CLIENT_MODULE_NAME "log_sdk_client"

#if !defined( USER_LOG_CALLBACK )
#define USER_LOG_CALLBACK
typedef void_t (*UserLogCallback)( uint16_t Type, uint16_t Subtype, uint64_t Time, const char_t *UserName, uint32_t UserNameLength, const void_t *SpecificData, uint32_t SpecificDataLength );
#endif//USER_LOG_CALLBACK

#if !defined( DEBUG_LOG_CALLBACK )
#define DEBUG_LOG_CALLBACK
typedef void_t (*DebugLogCallback)( uint16_t Level, uint64_t Time, uint64_t ProcessId, uint64_t ThreadId, uint32_t ModuleId, const char_t *ModuleName,
                                    const char_t *FileName, const char_t *FunctionName, uint32_t LineNumber, const void_t *SpecificData, uint32_t SpecificDataLength );
#endif//DEBUG_LOG_CALLBACK

class Log_SDK_Client
{
public:
    Log_SDK_Client(void);
    ~Log_SDK_Client(void);

#if defined( __linux__ )
    GMI_RESULT	Initialize( in_addr_t ServerIP, uint16_t ServerPort, const char_t *LogFilePath );
#elif defined( _WIN32 )
    GMI_RESULT	Initialize( ulong_t ServerIP, uint16_t ServerPort, const char_t *LogFilePath );
#endif
    GMI_RESULT  Deinitialize();
    void_t      SetUserLogCallback ( UserLogCallback  Callback )
    {
        m_UserLogCallback  = Callback;
    }
    void_t      SetDebugLogCallback( DebugLogCallback Callback )
    {
        m_DebugLogCallback = Callback;
    }

    GMI_RESULT  ConfigLog( enum LogType Type, enum LogTransportMode TransportMode, uint16_t StorageMediaType, enum LogStorageLimitMode StorageLimitMode, uint32_t StorageLimitParameter );
    GMI_RESULT  SetupDebugLog( boolean_t Enable, uint16_t WarningLevel, uint32_t BufferingRecordNumber );
    GMI_RESULT  QueryUserLog( uint16_t Type, uint16_t Subtype, uint64_t StartTime, uint64_t EndTime, uint8_t *Reply, uint32_t *ReplyLength );
    GMI_RESULT  QueryDebugLog( uint64_t StartTime, uint64_t EndTime, uint8_t *Reply, uint32_t *ReplyLength );

private:
    GMI_RESULT  Register();
    GMI_RESULT  Unregister();

    static void_t* ClientThreadEntry( void_t *Argument );
    void_t* ClientProcedure();

    GMI_RESULT ParseSession();
    GMI_RESULT ProcessNotify( ReferrencePtr<LogTcpSession,DefaultObjectDeleter,MultipleThreadModel> Session, void_t *Notify );

private:
    FileLogger                                                            m_LogClientOperationFile;
    boolean_t                                                             m_LogClientOperationFileOwner;
    GMI_Mutex                                                             m_Session_Mutex;
    ReferrencePtr<LogTcpSession,DefaultObjectDeleter,MultipleThreadModel> m_Client_Session;
    ApplicationPacket                                                     m_Reply;
    ApplicationPacket                                                     m_Request;
    UserLogCallback                                                       m_UserLogCallback;
    DebugLogCallback                                                      m_DebugLogCallback;

    GMI_Thread                                                            m_Thread;
    boolean_t                                                             m_ThreadExitFlag;
    boolean_t                                                             m_ThreadWorking;
};
