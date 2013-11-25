#pragma once

#include "application_packet.h"
#include "gmi_system_headers.h"
#include "log_handler.h"
#include "log_tcp_session.h"

#define NETCARD_MAC_ADDRESS_LENGTH 6
#define LOG_TCP_MAX_CLINT_NUMBER   8
#define LOG_TCP_SERVER_SELECT_TIME 500

class Log_TCP_Server : public GMI_LogHandler
{
public:
    Log_TCP_Server(void);
    virtual ~Log_TCP_Server(void);

    virtual GMI_RESULT  Initialize( const void_t *Parameter, size_t ParameterLength );
    virtual GMI_RESULT  Deinitialize();
    virtual GMI_RESULT  UserLog( uint16_t Type, uint16_t Subtype, uint64_t LogTime, const char_t *UserName, uint32_t UserNameLength, const void_t *SpecificData, uint32_t SpecificDataLength );
    virtual GMI_RESULT  DebugLog( uint16_t Level, uint64_t LogTime, uint64_t ProcessId, uint64_t ThreadId, uint32_t ModuleId, const char_t *ModuleName, const char_t *FileName, const char_t *FunctionName, uint32_t LineNumber, const void_t *SpecificData, uint32_t SpecificDataLength );
    virtual boolean_t   IsPublisher()
    {
        return true;
    }

private:
    static void_t* ServerThreadEntry( void_t *Argument );
    void_t* ServerProcedure();

    GMI_RESULT ProcessSession();
    GMI_RESULT ProcessRequest( ReferrencePtr<LogTcpSession,DefaultObjectDeleter,MultipleThreadModel> Session, ApplicationPacket *Request );

    GMI_RESULT Register( ReferrencePtr<LogTcpSession,DefaultObjectDeleter,MultipleThreadModel> Session, ApplicationPacket *Request );
    GMI_RESULT Unregister( ReferrencePtr<LogTcpSession,DefaultObjectDeleter,MultipleThreadModel> Session, ApplicationPacket *Request );
    GMI_RESULT QueryUserLog( ReferrencePtr<LogTcpSession,DefaultObjectDeleter,MultipleThreadModel> Session, uint16_t Type, uint16_t Subtype, uint64_t StartTime, uint64_t EndTime, ApplicationPacket *Request );
    GMI_RESULT QueryDebugLog( ReferrencePtr<LogTcpSession,DefaultObjectDeleter,MultipleThreadModel> Session, uint64_t StartTime, uint64_t EndTime, ApplicationPacket *Request );

private:
    SafePtr<GMI_Socket>                                                                 m_Server_Socket;
    ulong_t                                                                             m_Server_IP;
    uint8_t                                                                             m_MacAddress[NETCARD_MAC_ADDRESS_LENGTH];
    fd_set                                                                              m_ReadFDs;
    fd_set                                                                              m_ExceptFDs;
    int32_t                                                                             m_MaxFD;

    GMI_Mutex                                                                           m_Session_Mutex;
    std::vector<ReferrencePtr<LogTcpSession,DefaultObjectDeleter,MultipleThreadModel> > m_Uninitialized_Sessions;
    std::vector<ReferrencePtr<LogTcpSession,DefaultObjectDeleter,MultipleThreadModel> > m_Initialized_Sessions;
    ApplicationPacket                                                                   m_Request;
    ApplicationPacket                                                                   m_Reply;

    GMI_Thread                                                                          m_Thread;
    boolean_t                                                                           m_ThreadExitFlag;
    boolean_t                                                                           m_ThreadWorking;
};
