#if !defined( EVENT_COMMON_HEADER )
#define EVENT_COMMON_HEADER

struct AlarmEventConfigInfoEx
{
	AlarmEventConfigInfo  s_AlarmEventConfigInfo;
	ScheduleTimeInfo      s_ScheduleTime[7][MAX_SEG_TIME_PERDAY]; //7 days of every week 
};

struct AlarmInputInfoEx
{
    AlarmInputInfo    s_AlarmInputInfo;
   	ScheduleTimeInfo  s_ScheduleTime[7][MAX_SEG_TIME_PERDAY]; //7 days of every week 
};

struct AlarmOutputInfoEx
{
	AlarmOutputInfo   s_AlarmOutputInfo;
	ScheduleTimeInfo  s_ScheduleTime[7][MAX_SEG_TIME_PERDAY]; //7 days of every week 
};

extern AlarmEventConfigInfoEx  g_CurStartedEvent[MAX_NUM_EVENT_TYPE];
extern AlarmInputInfoEx        g_CurStartedAlarmIn[MAX_NUM_GPIO_IN];
extern AlarmOutputInfoEx       g_CurStartedAlarmOut[MAX_NUM_GPIO_OUT];
extern uint64_t                g_AlarmMessageId;


#endif

