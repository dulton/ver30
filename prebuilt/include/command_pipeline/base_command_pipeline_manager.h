#pragma once

#include "base_command_executor.h"
#include "base_command_requester.h"
#include "base_notifier.h"
#include "base_notifiee.h"
#include "base_packet.h"
#include "base_session.h"
#include "gmi_system_headers.h"
#include "timer_task_queue.h"

class BaseCommandPipelineManager
{
protected:
    BaseCommandPipelineManager(void);
public:
    virtual ~BaseCommandPipelineManager(void);

    virtual GMI_RESULT  Initialize() = 0;
    virtual GMI_RESULT  Deinitialize() = 0;

    virtual GMI_RESULT  RegisterPacket( ReferrencePtr<BasePacket> Packet ) = 0;
    virtual GMI_RESULT  UnregisterPacket() = 0;

    virtual GMI_RESULT  RegisterNotifiee( SafePtr<BaseNotifiee> Notifiee ) = 0;
    virtual GMI_RESULT  UnregisterNotifiee( uint32_t NotifyId ) = 0;

    virtual GMI_RESULT  RegisterCommandExecutor( SafePtr<BaseCommandExecutor> CommandExecutor ) = 0;
    virtual GMI_RESULT  UnregisterCommandExecutor( uint32_t CommandId ) = 0;

    virtual GMI_RESULT  Start( uint32_t LongTaskThreadNumber, uint32_t ShortTaskThreadNumber, uint32_t ThreadCheckInterval ) = 0;
    virtual GMI_RESULT  Stop() = 0;
    virtual GMI_RESULT  Run( boolean_t UseCallerThreadDoDispatchLoop ) = 0;

    virtual GMI_RESULT  AddNotifier( SafePtr<BaseNotifier> Notifier ) = 0;
    virtual GMI_RESULT  AddCommandRequester( ReferrencePtr<BaseCommandRequester,DefaultObjectDeleter,MultipleThreadModel> CommandRequester ) = 0;

    virtual GMI_RESULT  Parse( ReferrencePtr<BaseSession,DefaultObjectDeleter,MultipleThreadModel> Session ) = 0;
    virtual GMI_RESULT  AddCommandExecutor( SafePtr<BaseCommandExecutor> CommandExecutor ) = 0;

    virtual TASK_HANDLE Schedule1( uint64_t RelativeTime, DELAY_TASK_FUNCTION1 TaskFunction, DELAY_TASK_FUNCTION1 ResourceReleaseFunction, void_t *Argument ) = 0;
    virtual TASK_HANDLE Schedule1( uint64_t RelativeTime, DELAY_TASK_FUNCTION2 TaskFunction, DELAY_TASK_FUNCTION2 ResourceReleaseFunction, void_t *Argument1, void_t *Argument2 ) = 0;
    virtual TASK_HANDLE Schedule2( uint64_t SystemTime,   DELAY_TASK_FUNCTION1 TaskFunction, DELAY_TASK_FUNCTION1 ResourceReleaseFunction, void_t *Argument ) = 0;
    virtual TASK_HANDLE Schedule2( uint64_t SystemTime,   DELAY_TASK_FUNCTION2 TaskFunction, DELAY_TASK_FUNCTION2 ResourceReleaseFunction, void_t *Argument1, void_t *Argument2 ) = 0;
    virtual GMI_RESULT  Unschedule ( TASK_HANDLE& Task ) = 0;
    virtual GMI_RESULT  Reschedule1( TASK_HANDLE& Task, uint64_t RelativeTime ) = 0;
    virtual GMI_RESULT  Reschedule2( TASK_HANDLE& Task, uint64_t SystemTime ) = 0;
};
