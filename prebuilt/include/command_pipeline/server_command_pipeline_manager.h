#pragma once

#include "base_command_pipeline_manager.h"

class NotifyCommandParser;
class NotifyCommandProcessor;
class NotifyResultDispatcher;

class ServerCommandPipelineManager : public BaseCommandPipelineManager
{
public:
    ServerCommandPipelineManager(void);
    virtual ~ServerCommandPipelineManager(void);

    virtual GMI_RESULT  Initialize();
    virtual GMI_RESULT  Deinitialize();

    virtual GMI_RESULT  RegisterPacket( ReferrencePtr<BasePacket> Packet );
    virtual GMI_RESULT  UnregisterPacket();

    virtual GMI_RESULT  RegisterNotifiee( SafePtr<BaseNotifiee> Notifiee );
    virtual GMI_RESULT  UnregisterNotifiee( uint32_t NotifyId );

    virtual GMI_RESULT  RegisterCommandExecutor( SafePtr<BaseCommandExecutor> CommandExecutor );
    virtual GMI_RESULT  UnregisterCommandExecutor( uint32_t CommandId );

    virtual GMI_RESULT  Start( uint32_t LongTaskThreadNumber, uint32_t ShortTaskThreadNumber, uint32_t ThreadCheckInterval );
    virtual GMI_RESULT  Stop();
    virtual GMI_RESULT  Run( boolean_t UseCallerThreadDoDispatchLoop );

    virtual GMI_RESULT  AddNotifier( SafePtr<BaseNotifier> Notifier );
    virtual GMI_RESULT  AddCommandRequester( ReferrencePtr<BaseCommandRequester,DefaultObjectDeleter,MultipleThreadModel> CommandRequester );

    virtual GMI_RESULT  Parse( ReferrencePtr<BaseSession,DefaultObjectDeleter,MultipleThreadModel> Session );
    virtual GMI_RESULT  AddCommandExecutor( SafePtr<BaseCommandExecutor> CommandExecutor );

    virtual TASK_HANDLE Schedule1( uint64_t RelativeTime, DELAY_TASK_FUNCTION1 TaskFunction, DELAY_TASK_FUNCTION1 ResourceReleaseFunction, void_t *Argument );
    virtual TASK_HANDLE Schedule1( uint64_t RelativeTime, DELAY_TASK_FUNCTION2 TaskFunction, DELAY_TASK_FUNCTION2 ResourceReleaseFunction, void_t *Argument1, void_t *Argument2 );
    virtual TASK_HANDLE Schedule2( uint64_t SystemTime,   DELAY_TASK_FUNCTION1 TaskFunction, DELAY_TASK_FUNCTION1 ResourceReleaseFunction, void_t *Argument );
    virtual TASK_HANDLE Schedule2( uint64_t SystemTime,   DELAY_TASK_FUNCTION2 TaskFunction, DELAY_TASK_FUNCTION2 ResourceReleaseFunction, void_t *Argument1, void_t *Argument2 );
    virtual GMI_RESULT  Unschedule ( TASK_HANDLE& Task );
    virtual GMI_RESULT  Reschedule1( TASK_HANDLE& Task, uint64_t RelativeTime );
    virtual GMI_RESULT  Reschedule2( TASK_HANDLE& Task, uint64_t SystemTime );

private:
    ReferrencePtr< NotifyResultDispatcher > m_NotifyResultDispatcher;
    ReferrencePtr< NotifyCommandProcessor > m_NotifyCommandProcessor;
    SafePtr<NotifyCommandParser>	        m_NotifyCommandParser;
};
