#include "event_transaction_center.h"

#include "alarm_input.h"
#include "alarm_output.h"
#include "event_process_inforecord.h"
#include "human_detect.h"

EventTransactionCenter::EventTransactionCenter()
    : m_Center( NULL )
    , m_Callback( NULL )
    , m_UserData( NULL )
{
}

EventTransactionCenter::~EventTransactionCenter(void)
{
}

GMI_RESULT EventTransactionCenter::Start( const void_t *Parameter, size_t ParameterLength, EventCallback Callback, void_t *UserData )
{
    m_Callback = Callback;
    m_UserData = UserData;

    m_Center = BaseMemoryManager::Instance().New<EventCenter>();
    if ( NULL == m_Center.GetPtr() )
    {
        printf( "allocating EventCenter object failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    GMI_RESULT Result = m_Center->Initialize( NULL, 0 );
    if ( FAILED( Result ) )
    {
        printf( "EventCenter Initialize failed \n" );
        return Result;
    }

    Result = StartGPIOAlarmOutput();
    if ( FAILED( Result ) )
    {
        m_Center->Deinitialize();
        m_Center = NULL;
        return Result;
    }

    Result = StartGPIOAlarmInput();
    if ( FAILED( Result ) )
    {
        StopGPIOAlarmOutput();
        m_Center->Deinitialize();
        m_Center = NULL;
        return Result;
    }


	Result = StartHumanDetect();
    if ( FAILED( Result ) )
    {
        StopGPIOAlarmOutput();
        m_Center->Deinitialize();
        m_Center = NULL;
        return Result;
    }

	Result = StartAlarmInfoRecord();
    if ( FAILED( Result ) )
    {
        StopGPIOAlarmInput();
        StopGPIOAlarmOutput();
		StopHumanDetect();
        m_Center->Deinitialize();
        m_Center = NULL;
        return Result;
    }

	

    Result = m_Center->Start();
    if ( FAILED( Result ) )
    {
        StopGPIOAlarmInput();
        StopGPIOAlarmOutput();
		StopHumanDetect();
		StopAlarmInfoRecord();
        m_Center->Deinitialize();
        m_Center = NULL;
        printf( "EventCenter Start failed \n" );
        return Result;
    }

    return Result;
}

GMI_RESULT EventTransactionCenter::Stop()
{
    GMI_RESULT Result = m_Center->Stop();
    if ( FAILED( Result ) )
    {
        return Result;
    }

    Result = StopGPIOAlarmInput();
    if ( FAILED( Result ) )
    {
        return Result;
    }

    Result = StopGPIOAlarmOutput();
    if ( FAILED( Result ) )
    {
        return Result;
    }
		
    Result = StopHumanDetect();
    if ( FAILED( Result ) )
    {
        return Result;
    }

	Result = StopAlarmInfoRecord();
    if ( FAILED( Result ) )
    {
        return Result;
    }

    Result = m_Center->Deinitialize();

    if ( FAILED( Result ) )
    {
        return Result;
    }

    return Result;
}

GMI_RESULT EventTransactionCenter::ConfigureGPIOAlarmInput( const void_t *Parameter, size_t ParameterLength )
{
    return GMI_NOT_IMPLEMENT;
}

GMI_RESULT EventTransactionCenter::ConfigureGPIOAlarmOutput( const void_t *Parameter, size_t ParameterLength )
{
    return GMI_NOT_IMPLEMENT;
}

GMI_RESULT EventTransactionCenter::StartGPIOAlarmInput()
{
    ReferrencePtr<AlarmInput> AlarmInputDetector( BaseMemoryManager::Instance().New<AlarmInput>( e_EventDetectorType_Passive, EVENT_DETECTOR_ID_ALARM_INPUT ) );
    if ( NULL == AlarmInputDetector.GetPtr() )
    {
        m_Center->Deinitialize();
        printf( "allocating AlarmInputDetector object failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    struct AlarmInputInfo Info;
    Info.s_InputNumber = 0;
#if defined( __linux__ )
    strcpy( Info.s_Name, "AlarmOutput" );
#elif defined( _WIN32 )
    strcpy_s( Info.s_Name, "AlarmOutput" );
#endif
    Info.s_CheckTime = 100;
    Info.s_TriggerType = e_AlarmInputTriggerType_UsuallyClosed;
    Info.s_ScheduleTimeNumber = 1;
    Info.s_ScheduleTime[0].s_StartTime = 0x106000000000000;//Monday, AM 06:00:00
    Info.s_ScheduleTime[0].s_EndTime   = 0x112000000000000;//Monday, PM 06:00:00

    GMI_RESULT Result = m_Center->RegisterEventDetector( AlarmInputDetector, &Info, sizeof(Info) );
    if ( FAILED( Result ) )
    {
        m_Center->Deinitialize();
        printf( "EventCenter RegisterEventDetector failed \n" );
        return Result;
    }

    return Result;
}

GMI_RESULT EventTransactionCenter::StopGPIOAlarmInput()
{
    GMI_RESULT Result = m_Center->UnregisterEventDetector( EVENT_DETECTOR_ID_ALARM_INPUT );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    return Result;
}

GMI_RESULT EventTransactionCenter::StartGPIOAlarmOutput()
{
    ReferrencePtr<AlarmOutput> AlarmOutputProcessor( BaseMemoryManager::Instance().New<AlarmOutput>( EVENT_PROCESSOR_ID_ALARM_OUTPUT ) );
    if ( NULL == AlarmOutputProcessor.GetPtr() )
    {
        m_Center->Deinitialize();
        printf( "allocating AlarmOutputProcessor object failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    AlarmOutputProcessor->AddDetectorId( EVENT_DETECTOR_ID_ALARM_INPUT );
	AlarmOutputProcessor->AddDetectorId( EVENT_DETECTOR_ID_HUMAN_DETECT);

    AlarmOutputProcessor->SetEventCallback( m_Callback, m_UserData );

    struct AlarmOutputInfo Info;
    Info.s_OutputNumber = 0;
#if defined( __linux__ )
    strcpy( Info.s_Name, "AlarmInput" );
#elif defined( _WIN32 )
    strcpy_s( Info.s_Name, "AlarmInput" );
#endif
    Info.s_WorkMode = e_AlarmOutputWorkMode_DelayAutoTrigger;
    Info.s_DelayTime = 10;
    Info.s_ScheduleTimeNumber = 1;
    Info.s_ScheduleTime[0].s_TimeType  = e_TimeType_WeekCycle;
    Info.s_ScheduleTime[0].s_StartTime = 0x106000000000000;//Monday, AM 06:00:00
    Info.s_ScheduleTime[0].s_EndTime   = 0x112000000000000;//Monday, PM 06:00:00

    GMI_RESULT Result = m_Center->RegisterEventProcessor( AlarmOutputProcessor, &Info, sizeof(Info) );
    if ( FAILED( Result ) )
    {
        m_Center->Deinitialize();
        printf( "EventCenter RegisterEventProcessor failed \n" );
        return Result;
    }

    return Result;
}

GMI_RESULT EventTransactionCenter::StopGPIOAlarmOutput()
{
    GMI_RESULT Result = m_Center->UnregisterEventProcessor( EVENT_PROCESSOR_ID_ALARM_OUTPUT );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    return Result;
}

GMI_RESULT EventTransactionCenter::StartAlarmInfoRecord()
{
    ReferrencePtr<EventProcessInfoRecord> AlarmInfoRecordProcessor( BaseMemoryManager::Instance().New<EventProcessInfoRecord>( EVENT_PROCESSOR_ID_INFO_RECORD ) );
    if ( NULL == AlarmInfoRecordProcessor.GetPtr() )
    {
        m_Center->Deinitialize();
        printf( "allocating AlarmInfoRecordProcessor object failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    AlarmInfoRecordProcessor->AddDetectorId( EVENT_DETECTOR_ID_ALARM_INPUT );
	
	AlarmInfoRecordProcessor->AddDetectorId( EVENT_DETECTOR_ID_HUMAN_DETECT);

    AlarmInfoRecordProcessor->SetEventCallback( m_Callback, m_UserData );

    GMI_RESULT Result = m_Center->RegisterEventProcessor( AlarmInfoRecordProcessor, NULL, 0);
    if ( FAILED( Result ) )
    {
        m_Center->Deinitialize();
        printf( "EventCenter RegisterEventProcessor failed \n" );
        return Result;
    }

    return Result;
}

GMI_RESULT EventTransactionCenter::StopAlarmInfoRecord()
{
    GMI_RESULT Result = m_Center->UnregisterEventProcessor( EVENT_PROCESSOR_ID_INFO_RECORD );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    return Result;
}


GMI_RESULT EventTransactionCenter::StartHumanDetect()
{
    ReferrencePtr<HumanDetect> HumanDetector( BaseMemoryManager::Instance().New<HumanDetect>( e_EventDetectorType_Passive, EVENT_DETECTOR_ID_HUMAN_DETECT) );
    if ( NULL == HumanDetector.GetPtr() )
    {
        m_Center->Deinitialize();
        printf( "allocating HumanDetector object failed \n" );
        return GMI_OUT_OF_MEMORY;
    }

    struct HumanDetectInfo Info;
    Info.s_CheckTime = 1000;
    Info.s_ScheduleTimeNumber = 1;
    Info.s_ScheduleTime[0].s_StartTime = 0x106000000000000;//Monday, AM 06:00:00
    Info.s_ScheduleTime[0].s_EndTime   = 0x112000000000000;//Monday, PM 06:00:00

    GMI_RESULT Result = m_Center->RegisterEventDetector( HumanDetector, &Info, sizeof(Info) );
    if ( FAILED( Result ) )
    {
        m_Center->Deinitialize();
        printf( "EventCenter RegisterEventDetector failed \n" );
        return Result;
    }

    return Result;
}

GMI_RESULT EventTransactionCenter::StopHumanDetect()
{
    GMI_RESULT Result = m_Center->UnregisterEventDetector( EVENT_DETECTOR_ID_HUMAN_DETECT);
    if ( FAILED( Result ) )
    {
        return Result;
    }

    return Result;
}

