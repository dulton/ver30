#pragma once

#include "application_packet.h"
#include "file_logger.h"
#include "gmi_system_headers.h"
#include "log_header.h"

#define FILE_LOGGER_SERVER_MODULE_ID   0
#define FILE_LOGGER_SERVER_MODULE_NAME "log_server"

class ApplicationPacket;
class GMI_LogHandler;
class Log_TCP_Server;
class LogClientPeer;
class PipeSession;

class LogServer
{
public:
    LogServer(void);
    ~LogServer(void);

    GMI_RESULT	Initialize( const char_t *ServerPipeName, long_t ServerPipeMutexId, const char_t *LogFilePath );
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

    GMI_RESULT  Register( uint32_t ModuleId, const char_t *ModuleName, const char_t *ModulePipeName, long_t ModulePipeMutexId, const char_t *PeerPipeName, long_t PeerPipeMutexId, const ApplicationPacket& Request );
    GMI_RESULT  Unregister( uint32_t ModuleId, const ApplicationPacket& Request );
    GMI_RESULT  ConfigLog( enum LogType Type, enum LogTransportMode TransportMode, uint16_t StorageMediaType, enum LogStorageLimitMode StorageLimitMode, uint32_t StorageLimitParameter );
    GMI_RESULT  SetupDebugLog( boolean_t Enable, uint16_t WarningLevel, uint32_t BufferingRecordNumber );
    GMI_RESULT  QueryUserLog( uint16_t Type, uint16_t Subtype, uint64_t StartTime, uint64_t EndTime, uint8_t *Reply, uint32_t *ReplyLength );
    GMI_RESULT  QueryDebugLog( uint64_t StartTime, uint64_t EndTime, uint8_t *Reply, uint32_t *ReplyLength );

private:
    GMI_RESULT  ParseRequest( ReferrencePtr<PipeSession,DefaultObjectDeleter,MultipleThreadModel>& PipeSession );
    GMI_RESULT  ProcessRequest( ApplicationPacket& Request );

    static void_t* DispatchThread( void_t *Argument );
    void_t* DispatchEntry();

private:
    FileLogger                                                          m_LogServerOperationFile;
    ReferrencePtr<PipeSession,DefaultObjectDeleter,MultipleThreadModel> m_PipeSession;
    ApplicationPacket                                                   m_Request;
    boolean_t                                                           m_ExitFlag;
    SafePtr< char_t, DefaultObjectsDeleter >                            m_ServerPipeName;
    long_t                                                              m_ServerPipeMutexId;
    std::vector< SafePtr<LogClientPeer> >                               m_ClientPeers;

    long_t                                                              m_UserLogLevel;
    long_t                                                              m_DebugLogLevel;
    std::vector<GMI_LogHandler*>                                        m_LogHandlers;

    boolean_t                                                           m_UseCallerThreadDoDispatchLoop;
    GMI_Thread							                                m_DispatchThread;
    boolean_t                                                           m_ThreadWorking;
    boolean_t                                                           m_ThreadExitFlag;
};
