#pragma once

#include "application_packet.h"
#include "file_logger.h"
#include "gmi_system_headers.h"
#include "log_header.h"

#if !defined( FILE_LOGGER_SERVER_MODULE_ID )
#define FILE_LOGGER_SERVER_MODULE_ID   0
#define FILE_LOGGER_SERVER_MODULE_NAME "log_server"
#endif//FILE_LOGGER_SERVER_MODULE_ID

class ApplicationPacket;
class GMI_LogHandler;
class Log_TCP_Server;
class ReliableUDPSession;
class ShareMemoryLogSession;

class ShareMemoryLogServer
{
public:
    ShareMemoryLogServer(void);
    ~ShareMemoryLogServer(void);

    GMI_RESULT	Initialize( uint16_t ServerPort, long_t ShareMemoryKey, size_t ShareMemorySize, long_t IpcMutexKey, const char_t *LogFilePath, uint32_t DebugLogLevel );
    GMI_RESULT  Deinitialize();
    GMI_RESULT  Register( GMI_LogHandler *Handler );
    GMI_RESULT  Unregister( GMI_LogHandler *Handler );
    GMI_RESULT  Run( boolean_t UseCallerThreadDoDispatchLoop );

    GMI_RESULT  UserLog( uint16_t Type, uint16_t Subtype, uint64_t LogTime, const char_t *UserName, uint32_t UserNameLength, const void_t *SpecificData, uint32_t SpecificDataLength );
    GMI_RESULT  DebugLog( uint16_t Level, uint64_t LogTime, uint64_t ProcessId, uint64_t ThreadId, uint32_t ModuleId, const char_t *ModuleName, const char_t *FileName, const char_t *FunctionName, uint32_t LineNumber, const void_t *SpecificData, uint32_t SpecificDataLength );

private:
    GMI_RESULT  Start();
    GMI_RESULT  Stop();
    GMI_RESULT  Restart();
    GMI_RESULT  Exit();

    GMI_RESULT  Register( uint16_t ClientPort );
    GMI_RESULT  Unregister( uint16_t ClientPort );
    GMI_RESULT  ConfigLog( enum LogType Type, enum LogTransportMode TransportMode, uint16_t StorageMediaType, enum LogStorageLimitMode StorageLimitMode, uint32_t StorageLimitParameter );
    GMI_RESULT  SetupDebugLog( boolean_t Enable, uint16_t WarningLevel, uint32_t BufferingRecordNumber );
    GMI_RESULT  QueryUserLog( uint16_t Type, uint16_t Subtype, uint64_t StartTime, uint64_t EndTime, uint8_t *Reply, uint32_t *ReplyLength );
    GMI_RESULT  QueryDebugLog( uint64_t StartTime, uint64_t EndTime, uint8_t *Reply, uint32_t *ReplyLength );

private:
    GMI_RESULT  ProcessRequest( const uint8_t *RequestPacket, size_t PacketLength, uint16_t ClientPort );
    GMI_RESULT  ProcessBufferedLog();

    static void_t* DispatchThread( void_t *Argument );
    void_t* DispatchEntry();

private:
    FileLogger                                                                     m_LogServerOperationFile;
    ReferrencePtr<GMI_Socket>                                                      m_UDP_Socket;
    ReferrencePtr<ReliableUDPSession,DefaultObjectDeleter,MultipleThreadModel>     m_ReliableUDPSession;
    ReferrencePtr<ShareMemoryLogSession,DefaultObjectDeleter,MultipleThreadModel>  m_ShareMemoryLogSession;
    long_t                                                                         m_ShareMemoryKey;
    size_t                                                                         m_ShareMemorySize;
    long_t                                                                         m_IpcMutextKey;
    uint8_t                                                                        m_LogBuffer[FILE_LOGGER_MAX_LOG_BUFFER];
    long_t                                                                         m_UserLogLevel;
    long_t                                                                         m_DebugLogLevel;
    std::vector<GMI_LogHandler*>                                                   m_LogHandlers;

    boolean_t                                                                      m_UseCallerThreadDoDispatchLoop;
    GMI_Thread							                                           m_DispatchThread;
    boolean_t                                                                      m_ThreadWorking;
    boolean_t                                                                      m_ThreadExitFlag;
};
