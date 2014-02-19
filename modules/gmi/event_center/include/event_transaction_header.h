// event_transaction_header.h

#if !defined( EVENT_TRANSACTION_HEADER )
#define EVENT_TRANSACTION_HEADER

// detector id
#define EVENT_DETECTOR_ID_ALARM_INPUT     1
#define EVENT_DETECTOR_ID_HUMAN_DETECT    2


// processor id
#define EVENT_PROCESSOR_ID_ALARM_OUTPUT  1
#define EVENT_PROCESSOR_ID_INFO_RECORD   2


enum AlarmEventType
{
    e_AlarmEventType_AlarmInput = 1,
    e_AlarmEventType_HumanDetect,
};



enum AlarmInputTriggerType
{
    e_AlarmInputTriggerType_UsuallyClosed = 1,
    e_AlarmInputTriggerType_UsuallyOpened,
};

enum AlarmInputStatus
{
    e_AlarmInputStatus_Closed = 0,
    e_AlarmInputStatus_Opened,
};

#if 1
enum TimeType
{
    e_TimeType_Absolute = 1, // uint64_t can express absolute time
    e_TimeType_Relative,     // uint64_t is microsecond unit
    e_TimeType_DayCycle,     // uint64_t: hour-minute-second-microsecond, 0xhhmmss00xxxxxx
    e_TimeType_WeekCycle,    // uint64_t: weekday-hour-minute-second-microsecond, 0xwwhhmmss00xxxxxx
    e_TimeType_MonthCycle,   // uint64_t: monthday-hour-minute-second-microsecond, 0xwwhhmmss00xxxxxx
};

struct ScheduleTimeInfo
{
    enum TimeType     s_TimeType;
    uint64_t          s_StartTime;
    uint64_t          s_EndTime;
};
#else
struct ScheduleTimeInfo
{
    uint32_t          s_StartTime;
    uint32_t          s_EndTime;
};

#endif

#define ALARM_INPUT_MAX_NAME_LENGTH  32
struct AlarmInputInfo
{
    uint32_t          s_EnableFlag;      //0-disable, 1-enbale
    uint32_t          s_InputNumber;     //range from 0-3
    char_t            s_Name[ALARM_INPUT_MAX_NAME_LENGTH];
    uint32_t          s_CheckTime; //unit:ms
    uint32_t          s_TriggerType;
   	ScheduleTimeInfo  s_ScheduleTime[7]; //7 days of every week 
	//every bit represents the certain AlarmStrategy;
    //0-Alarm output,1-Info record,2-...;
    //value 0-invalid, value 1-valid
	uint32_t          s_LinkAlarmStrategy;
	uint32_t          s_Reserverd[4];
};

enum AlarmOutputWorkMode
{
    e_AlarmOutputWorkMode_DelayAutoTrigger = 1,
    e_AlarmOutputWorkMode_ManualTrigger,
};

#define ALARM_OUTPUT_MAX_NAME_LENGTH  32
struct AlarmOutputInfo
{
	uint32_t          s_EnableFlag; //0-disable, 1-enbale
	uint32_t          s_OutputNumber; //range from 0-3
    char_t            s_Name[ALARM_OUTPUT_MAX_NAME_LENGTH];
	uint32_t          s_NormalStatus;
	uint32_t          s_AlarmStatus;
	uint32_t          s_WorkMode;
    // s_DelayTime is in second unit
    uint32_t          s_DelayTime; //unit:s
	ScheduleTimeInfo  s_ScheduleTime[7]; //7 days of every week 
	uint32_t          s_Reserverd[4];
    //uint32_t          s_ScheduleTimeNumber;
    //ScheduleTimeInfo  s_ScheduleTime[1];
};

struct HumanDetectInfo
{
	uint32_t          s_CheckTime;
    //uint32_t          s_ScheduleTimeNumber;
    //ScheduleTimeInfo  s_ScheduleTime[1];
};

#define FLAG_EVENT_DISABLE  0
#define FLAG_EVENT_ENABLE   1

struct AlarmEventConfigInfo
{
	uint32_t           s_EnableFlag;      //0-disable, 1-enbale
	ScheduleTimeInfo  s_ScheduleTime[7]; //7 days of every week 
	//every bit represents the certain AlarmStrategy;
    //0-Alarm output,1-Info record,2-...;
    //value 0-invalid, value 1-valid
	uint32_t          s_LinkAlarmStrategy;
	uint32_t          s_CheckTime;
	uint32_t          s_MinSensVal; //human detect use
	uint32_t          s_MaxSensVal; //human detect use
	uint32_t          s_Reserverd[4];
};

#define  MAX_NUM_EVENT_TYPE       10
#define MAX_NUM_GPIO_IN           4
#define MAX_NUM_GPIO_OUT          4

extern AlarmEventConfigInfo g_CurStartedEvent[MAX_NUM_EVENT_TYPE];
extern AlarmInputInfo g_CurStartedAlaramIn[MAX_NUM_GPIO_IN];


#endif//EVENT_TRANSACTION_HEADER
