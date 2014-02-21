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
	struct AlarmUploadInf *TmpInfo = NULL;
	if((Parameter != NULL) && (ParameterLength >= sizeof(struct AlarmUploadInf)))
	{
		TmpInfo = (struct AlarmUploadInf *)Parameter;
	}
	else
	{
		printf("EventProcess Parameter NULL\n");
	}
    gettimeofday1( &CurrentTime, NULL );
    //printf( "EventProcess: UserData=%p, EventId=%d, Type=%d, Parameter=%p, ParameterLength=%d, current time=%ld:%06ld \n", UserData, EventId, Type, Parameter, ParameterLength, CurrentTime.tv_sec, CurrentTime.tv_usec );
	printf("****callback %ld:%06ld****\n", CurrentTime.tv_sec, CurrentTime.tv_usec);
	printf("EventId=%d\n", EventId);
	printf("Type=%d\n", Type);
	
	if(NULL != TmpInfo)
	{
		printf("s_AlarmId=%ld\n", (long)TmpInfo->s_AlarmId);
		printf("s_AlarmType=%d\n", TmpInfo->s_AlarmType);
		printf("s_AlarmLevel=%d\n", TmpInfo->s_AlarmLevel);
		printf("s_OnOff=%d(1-on,0-off)\n", TmpInfo->s_OnOff);
		printf("s_Time=%d:%06d\n", TmpInfo->s_TimeSec, TmpInfo->s_TimeUsec);
		printf("s_Description=%s\n", TmpInfo->s_Description);
		printf("s_IoNum=%d\n", TmpInfo->s_ExtraInfo.s_IoNum);
	}
	printf("****************\n\n");
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

	if(argc < 4)
	{
		fprintf(stderr, "******operate method*********\n");
		fprintf(stderr, "./X a b c\n");
		fprintf(stderr, "X--execut binary\n");
		fprintf(stderr, "a--sensitivity lower limit value\n");
		fprintf(stderr, "b--sensitivity upper limit value\n");
		fprintf(stderr, "c--white light invalid time(unit:s[0,60])\n");
		return -1;
	}

	#if 1
	AlarmOutputInfo TmpIoOutParam;
	memset(&TmpIoOutParam, 0, sizeof(TmpIoOutParam));
	TmpIoOutParam.s_EnableFlag = 1;
	TmpIoOutParam.s_OutputNumber = 0;
	strcpy(TmpIoOutParam.s_Name, "Io output 0");
	if((atoi(argv[3]) > 0) && (atoi(argv[3]) < 60))
	{
		TmpIoOutParam.s_DelayTime = atoi(argv[3]);
	}
	else
	{
		TmpIoOutParam.s_DelayTime = 5; //s
	}
	TmpIoOutParam.s_NormalStatus = e_AlarmOutputStatus_Closed;
	#if 0
	TmpIoOutParam.s_ScheduleTime[0].s_StartTime = 0;
	TmpIoOutParam.s_ScheduleTime[0].s_EndTime = 24*60;
	TmpIoOutParam.s_ScheduleTime[1].s_StartTime = 0;
	TmpIoOutParam.s_ScheduleTime[1].s_EndTime = 24*60;
	TmpIoOutParam.s_ScheduleTime[2].s_StartTime = 0;
	TmpIoOutParam.s_ScheduleTime[2].s_EndTime = 24*60;
	TmpIoOutParam.s_ScheduleTime[3].s_StartTime = 0;
	TmpIoOutParam.s_ScheduleTime[3].s_EndTime = 24*60;
	TmpIoOutParam.s_ScheduleTime[4].s_StartTime = 0;
	TmpIoOutParam.s_ScheduleTime[4].s_EndTime = 24*60;
	TmpIoOutParam.s_ScheduleTime[5].s_StartTime = 0;
	TmpIoOutParam.s_ScheduleTime[5].s_EndTime = 24*60;
	TmpIoOutParam.s_ScheduleTime[6].s_StartTime = 0;
	TmpIoOutParam.s_ScheduleTime[6].s_EndTime = 24*60;
	#endif
	Center.ConfigureGPIOAlarmOutput((void_t*)(&TmpIoOutParam),sizeof(TmpIoOutParam));
	
	#endif

	#if 1
	AlarmInputInfo TmpIoParam;
	memset(&TmpIoParam, 0, sizeof(TmpIoParam));
	TmpIoParam.s_EnableFlag = 1;
	TmpIoParam.s_InputNumber = 0;
	TmpIoParam.s_NormalStatus = e_AlarmInputStatus_Closed;
	strcpy(TmpIoParam.s_Name, "Io input 0");
	#if 0
	TmpIoParam.s_ScheduleTime[0].s_StartTime = 0;
	TmpIoParam.s_ScheduleTime[0].s_EndTime = 24*60;
	TmpIoParam.s_ScheduleTime[1].s_StartTime = 0;
	TmpIoParam.s_ScheduleTime[1].s_EndTime = 24*60;
	TmpIoParam.s_ScheduleTime[2].s_StartTime = 0;
	TmpIoParam.s_ScheduleTime[2].s_EndTime = 24*60;
	TmpIoParam.s_ScheduleTime[3].s_StartTime = 0;
	TmpIoParam.s_ScheduleTime[3].s_EndTime = 24*60;
	TmpIoParam.s_ScheduleTime[4].s_StartTime = 0;
	TmpIoParam.s_ScheduleTime[4].s_EndTime = 24*60;
	TmpIoParam.s_ScheduleTime[5].s_StartTime = 0;
	TmpIoParam.s_ScheduleTime[5].s_EndTime = 24*60;
	TmpIoParam.s_ScheduleTime[6].s_StartTime = 0;
	TmpIoParam.s_ScheduleTime[6].s_EndTime = 24*60;
	#endif
	TmpIoParam.s_LinkAlarmStrategy = 3;
	TmpIoParam.s_LinkAlarmExtInfo.s_IoNum = 0;
	Center.ConfigureGPIOAlarmInput((void_t*)(&TmpIoParam),sizeof(TmpIoParam));

	memset(&TmpIoParam, 0, sizeof(TmpIoParam));
	TmpIoParam.s_EnableFlag = 1;
	TmpIoParam.s_InputNumber = 2;
	strcpy(TmpIoParam.s_Name, "Io input 2");
	#if 0
	TmpIoParam.s_ScheduleTime[0].s_StartTime = 0;
	TmpIoParam.s_ScheduleTime[0].s_EndTime = 24*60;
	TmpIoParam.s_ScheduleTime[1].s_StartTime = 0;
	TmpIoParam.s_ScheduleTime[1].s_EndTime = 24*60;
	TmpIoParam.s_ScheduleTime[2].s_StartTime = 0;
	TmpIoParam.s_ScheduleTime[2].s_EndTime = 24*60;
	TmpIoParam.s_ScheduleTime[3].s_StartTime = 0;
	TmpIoParam.s_ScheduleTime[3].s_EndTime = 24*60;
	TmpIoParam.s_ScheduleTime[4].s_StartTime = 0;
	TmpIoParam.s_ScheduleTime[4].s_EndTime = 24*60;
	TmpIoParam.s_ScheduleTime[5].s_StartTime = 0;
	TmpIoParam.s_ScheduleTime[5].s_EndTime = 24*60;
	TmpIoParam.s_ScheduleTime[6].s_StartTime = 0;
	TmpIoParam.s_ScheduleTime[6].s_EndTime = 24*60;
	#endif
	TmpIoParam.s_LinkAlarmStrategy = 3;
	Center.ConfigureGPIOAlarmInput((void_t*)(&TmpIoParam),sizeof(TmpIoParam));
	#endif
	#if 1
	AlarmEventConfigInfo TmpParam;
	memset(&TmpParam, 0, sizeof(TmpParam));
	TmpParam.s_EnableFlag = 1;
	TmpParam.s_CheckTime = 400; //ms
	if((atoi(argv[1]) > 0) && (atoi(argv[1]) < 1000))
	{
		TmpParam.s_ExtData.s_HumanDetectExInfo.s_MinSensVal = atoi(argv[1]);
	}
	else
	{
		TmpParam.s_ExtData.s_HumanDetectExInfo.s_MinSensVal = 100;
	}
	if((atoi(argv[2]) > 0) && (atoi(argv[2]) < 1000))
	{
		TmpParam.s_ExtData.s_HumanDetectExInfo.s_MaxSensVal = atoi(argv[2]);
	}
	else
	{
		TmpParam.s_ExtData.s_HumanDetectExInfo.s_MaxSensVal = 450;
	}

	if(TmpParam.s_ExtData.s_HumanDetectExInfo.s_MinSensVal > TmpParam.s_ExtData.s_HumanDetectExInfo.s_MaxSensVal)
	{
		printf("lower limit value bigger than upper limit value, need default value[100,450].\n");
		TmpParam.s_ExtData.s_HumanDetectExInfo.s_MinSensVal = 100;
		TmpParam.s_ExtData.s_HumanDetectExInfo.s_MaxSensVal = 450;
	}
	TmpParam.s_LinkAlarmStrategy = 3;
	TmpParam.s_LinkAlarmExtInfo.s_IoNum = 0;

	#if 0
	TmpParam.s_ScheduleTime[0].s_StartTime = 0;
	TmpParam.s_ScheduleTime[0].s_EndTime = 24*60;
	TmpParam.s_ScheduleTime[1].s_StartTime = 0;
	TmpParam.s_ScheduleTime[1].s_EndTime = 24*60;
	TmpParam.s_ScheduleTime[2].s_StartTime = 0;
	TmpParam.s_ScheduleTime[2].s_EndTime = 24*60;
	TmpParam.s_ScheduleTime[3].s_StartTime = 0;
	TmpParam.s_ScheduleTime[3].s_EndTime = 24*60;
	TmpParam.s_ScheduleTime[4].s_StartTime = 0;
	TmpParam.s_ScheduleTime[4].s_EndTime = 24*60;
	TmpParam.s_ScheduleTime[5].s_StartTime = 0;
	TmpParam.s_ScheduleTime[5].s_EndTime = 24*60;
	TmpParam.s_ScheduleTime[6].s_StartTime = 0;
	TmpParam.s_ScheduleTime[6].s_EndTime = 24*60;
	#endif
	Center.ConfigureAlarmEvent(e_AlarmEventType_HumanDetect, (void_t*)(&TmpParam),sizeof(TmpParam));
	#endif

	AlarmScheduleTimeInfo TmpTimeParam;
	memset(&TmpTimeParam, 0, sizeof(TmpTimeParam));
	TmpTimeParam.s_ScheduleTime[0].s_StartTime = 0;
	TmpTimeParam.s_ScheduleTime[0].s_EndTime = 24*60;
	TmpTimeParam.s_ScheduleTime[1].s_StartTime = 0;
	TmpTimeParam.s_ScheduleTime[1].s_EndTime = 24*60;
	TmpTimeParam.s_ScheduleTime[2].s_StartTime = 0;
	TmpTimeParam.s_ScheduleTime[2].s_EndTime = 24*60;
	TmpTimeParam.s_ScheduleTime[3].s_StartTime = 0;
	TmpTimeParam.s_ScheduleTime[3].s_EndTime = 24*60;
	TmpTimeParam.s_ScheduleTime[4].s_StartTime = 0;
	TmpTimeParam.s_ScheduleTime[4].s_EndTime = 24*60;
	TmpTimeParam.s_ScheduleTime[5].s_StartTime = 0;
	TmpTimeParam.s_ScheduleTime[5].s_EndTime = 24*60;
	TmpTimeParam.s_ScheduleTime[6].s_StartTime = 0;
	TmpTimeParam.s_ScheduleTime[6].s_EndTime = 24*60;

	TmpTimeParam.s_Index = 0;
	Center.ConfigureAlarmScheduleTime(SCHEDULE_TIME_ID_HUMAN_DETECT, &TmpTimeParam, sizeof(TmpTimeParam));

	TmpTimeParam.s_Index = 0;
	Center.ConfigureAlarmScheduleTime(SCHEDULE_TIME_ID_ALARM_IN, &TmpTimeParam, sizeof(TmpTimeParam));

	TmpTimeParam.s_Index = 2;
	Center.ConfigureAlarmScheduleTime(SCHEDULE_TIME_ID_ALARM_IN, &TmpTimeParam, sizeof(TmpTimeParam));

	TmpTimeParam.s_Index = 0;
	Center.ConfigureAlarmScheduleTime(SCHEDULE_TIME_ID_ALARM_OUT, &TmpTimeParam, sizeof(TmpTimeParam));

//	int32_t n=5;
	while('a' != getchar())
	{
		GMI_Sleep(1000);
	}
	printf("continue...\n");
	#if 0
	memset(&TmpIoParam, 0, sizeof(TmpIoParam));
	TmpIoParam.s_EnableFlag = 0;
	TmpIoParam.s_InputNumber = 0;
	strcpy(TmpIoParam.s_Name, "Io input 0");
	TmpIoParam.s_ScheduleTime[1].s_StartTime = 15*60;
	TmpIoParam.s_ScheduleTime[1].s_EndTime = 18*60;
	TmpIoParam.s_ScheduleTime[3].s_StartTime = 5*60;
	TmpIoParam.s_ScheduleTime[3].s_EndTime = 18*60;
	TmpIoParam.s_LinkAlarmStrategy = 3;
	Center.ConfigureGPIOAlarmInput((void_t*)(&TmpIoParam),sizeof(TmpIoParam));
	printf("stop alarm input 0\n");
	#endif

	TmpParam.s_EnableFlag = 0;
	
	Center.ConfigureAlarmEvent(e_AlarmEventType_HumanDetect, (void_t*)(&TmpParam),sizeof(TmpParam));

	while('b' != getchar())
	{
		GMI_Sleep(1000);
	}

	
	TmpParam.s_EnableFlag = 1;
	TmpParam.s_CheckTime = 500;
	TmpParam.s_LinkAlarmStrategy = 2;
	Center.ConfigureAlarmEvent(e_AlarmEventType_HumanDetect, (void_t*)(&TmpParam),sizeof(TmpParam));

	while('c' != getchar())
	{
		GMI_Sleep(1000);
	}
	TmpParam.s_LinkAlarmStrategy = 3;
	Center.ConfigureAlarmEvent(e_AlarmEventType_HumanDetect, (void_t*)(&TmpParam),sizeof(TmpParam));
	TmpIoOutParam.s_DelayTime = 10;

	Center.ConfigureGPIOAlarmOutput((void_t*)(&TmpIoOutParam),sizeof(TmpIoOutParam));

	while('d' != getchar())
	{
		GMI_Sleep(1000);
	}
	TmpIoOutParam.s_EnableFlag = 0;

	Center.ConfigureGPIOAlarmOutput((void_t*)(&TmpIoOutParam),sizeof(TmpIoOutParam));
	
	while('q' != getchar())
	{
		GMI_Sleep(1000);
	}

	

	#if 0
    do
    {
        GMI_Sleep( 5000 );
    }
    while( 1 );
	#endif

    Center.Stop();
#endif

    return 0;
}
