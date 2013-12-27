// event_transaction_header.h

#if !defined( EVENT_TRANSACTION_HEADER )
#define EVENT_TRANSACTION_HEADER

// detector id
#define EVENT_DETECTOR_ID_ALARM_INPUT    1

// processor id
#define EVENT_PROCESSOR_ID_ALARM_OUTPUT  1

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

#define ALARM_INPUT_MAX_NAME_LENGTH  32
struct AlarmInputInfo
{
    uint32_t          s_InputNumber;
    char_t            s_Name[ALARM_INPUT_MAX_NAME_LENGTH];
    uint32_t          s_CheckTime;
    uint32_t          s_TriggerType;
    uint32_t          s_ScheduleTimeNumber;
    ScheduleTimeInfo  s_ScheduleTime[1];
};

enum AlarmOutputWorkMode
{
    e_AlarmOutputWorkMode_DelayAutoTrigger = 1,
    e_AlarmOutputWorkMode_ManualTrigger,
};

#define ALARM_OUTPUT_MAX_NAME_LENGTH  32
struct AlarmOutputInfo
{
    uint32_t          s_OutputNumber;
    char_t            s_Name[ALARM_OUTPUT_MAX_NAME_LENGTH];
    uint32_t          s_WorkMode;
    // s_DelayTime is in second unit
    uint32_t          s_DelayTime;
    uint32_t          s_ScheduleTimeNumber;
    ScheduleTimeInfo  s_ScheduleTime[1];
};

#endif//EVENT_TRANSACTION_HEADER
