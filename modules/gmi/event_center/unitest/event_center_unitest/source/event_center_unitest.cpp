// event_center_unitest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "gmi_system_headers.h"

#include "event_center.h"
#include "event_transaction_center.h"
#include "simple_event_processor.h"
#include "simulated_event_detector.h"
#include "timer_task_queue.h"

void_t EventProcess( void_t *UserData, uint32_t EventId, enum EventType Type, void_t *Parameter, size_t ParameterLength )
{
    struct timeval CurrentTime;
    gettimeofday1( &CurrentTime, NULL );
    printf( "EventProcess: UserData=%p, EventId=%d, Type=%d, Parameter=%p, ParameterLength=%d, current time=%ld:%06ld \n", UserData, EventId, Type, Parameter, ParameterLength, CurrentTime.tv_sec, CurrentTime.tv_usec );
}

#if defined( _WIN32 )
int32_t _tmain( int32_t argc, _TCHAR* argv[] )
#elif defined( __linux__ )
int32_t main( int32_t argc, char_t* argv[] )
#endif
{
#if 0
    ReferrencePtr<EventCenter> Center( BaseMemoryManager::Instance().New<EventCenter>() );
    if ( NULL == Center.GetPtr() )
    {
        printf( "allocating EventCenter object failed \n" );
        return -1;
    }

    GMI_RESULT Result = Center->Initialize( NULL, 0 );
    if ( FAILED( Result ) )
    {
        printf( "EventCenter Initialize failed \n" );
        return -1;
    }

    ReferrencePtr<SimpleEventProcessor> Processor( BaseMemoryManager::Instance().New<SimpleEventProcessor>() );
    if ( NULL == Processor.GetPtr() )
    {
        Center->Deinitialize();
        printf( "allocating SimpleEventProcessor object failed \n" );
        return -1;
    }

    Processor->AddDetectorId( SIMULATED_EVENT_ID );

    Processor->SetEventCallback( EventProcess, NULL );

    Result = Center->RegisterEventProcessor( Processor, NULL, 0 );
    if ( FAILED( Result ) )
    {
        Center->Deinitialize();
        printf( "EventCenter RegisterEventProcessor failed \n" );
        return -1;
    }

    ReferrencePtr<SimulatedEventDetector> Detector( BaseMemoryManager::Instance().New<SimulatedEventDetector>() );
    if ( NULL == Detector.GetPtr() )
    {
        Center->UnregisterEventProcessor( Processor->GetId() );
        Center->Deinitialize();
        printf( "allocating Detector object failed \n" );
        return -1;
    }

    Result = Center->RegisterEventDetector( Detector, NULL, 0 );
    if ( FAILED( Result ) )
    {
        Center->UnregisterEventProcessor( Processor->GetId() );
        Center->Deinitialize();
        printf( "EventCenter RegisterEventDetector failed \n" );
        return -1;
    }

    Result = Center->Start();
    if ( FAILED( Result ) )
    {
        Center->UnregisterEventDetector( Detector->GetId() );
        Center->UnregisterEventProcessor( Processor->GetId() );
        Center->Deinitialize();
        printf( "EventCenter Start failed \n" );
        return -1;
    }

    do
    {
        GMI_Sleep( 1000 );
    }
    while( 1 );

    Center->Stop();
    Center->UnregisterEventDetector( Detector->GetId() );
    Center->UnregisterEventProcessor( Processor->GetId() );
    Center->Deinitialize();
#else
    EventTransactionCenter Center;
    GMI_RESULT Result = Center.Start( NULL, 0, EventProcess, NULL );
    if ( FAILED( Result ) )
    {
        return -1;
    }

    do
    {
        GMI_Sleep( 1000 );
    }
    while( 1 );

    Center.Stop();
#endif

    return 0;
}
