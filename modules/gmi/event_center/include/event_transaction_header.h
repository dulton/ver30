// event_transaction_header.h

#if !defined( EVENT_TRANSACTION_HEADER )
#define EVENT_TRANSACTION_HEADER

// detector id
#define EVENT_DETECTOR_ID_ALARM_INPUT     1
#define EVENT_DETECTOR_ID_HUMAN_DETECT    2

#define EVENT_DETECTOR_ID_MOTION_DETECT   3
#define EVENT_DETECTOR_ID_VCOVER_DETECT   4
#define EVENT_DETECTOR_ID_VLOSE_DETECT    5
#define EVENT_DETECTOR_ID_ABNORMAL_DETECT 6



// processor id
#define EVENT_PROCESSOR_ID_ALARM_OUTPUT  1
#define EVENT_PROCESSOR_ID_INFO_UPLOAD   2

#define EVENT_PROCESSOR_ID_ALARM_AUDIO   3
#define EVENT_PROCESSOR_ID_LINK_MAIL     4
#define EVENT_PROCESSOR_ID_UPLOAD_FTP    5
#define EVENT_PROCESSOR_ID_LINK_RECORD   6
#define EVENT_PROCESSOR_ID_LINK_PTZ      7


//schedule id 
#define SCHEDULE_TIME_ID_ALARM_IN        1
#define SCHEDULE_TIME_ID_ALARM_OUT       2
#define SCHEDULE_TIME_ID_HUMAN_DETECT    3


//
#define ALARM_INPUT_MAX_NAME_LENGTH   32
#define ALARM_OUTPUT_MAX_NAME_LENGTH  32
#define FLAG_EVENT_DISABLE            0
#define FLAG_EVENT_ENABLE             1
#define MAX_NUM_EVENT_TYPE           10
#define MAX_NUM_GPIO_IN               4
#define MAX_NUM_GPIO_OUT              4
#define MAX_SEG_TIME_PERDAY           4

enum AlarmEventType
{
    e_AlarmEventType_AlarmInput = 1,
    e_AlarmEventType_HumanDetect,
};


enum AlarmInputStatus
{
    e_AlarmInputStatus_Closed = 0,
    e_AlarmInputStatus_Opened,
};

enum AlarmOutputStatus
{
    e_AlarmOutputStatus_Closed = 0,
    e_AlarmOutputStatus_Opened,
};


/*
enum TimeType
{
    e_TimeType_Absolute = 1, // uint64_t can express absolute time
    e_TimeType_Relative,     // uint64_t is microsecond unit
    e_TimeType_DayCycle,     // uint64_t: hour-minute-second-microsecond, 0xhhmmss00xxxxxx
    e_TimeType_WeekCycle,    // uint64_t: weekday-hour-minute-second-microsecond, 0xwwhhmmss00xxxxxx
    e_TimeType_MonthCycle,   // uint64_t: monthday-hour-minute-second-microsecond, 0xwwhhmmss00xxxxxx
};
*/

enum AlarmOutputWorkMode
{
    e_AlarmOutputWorkMode_DelayAutoTrigger = 1,
    e_AlarmOutputWorkMode_ManualTrigger,
};


struct ScheduleTimeInfo
{
    //enum TimeType     s_TimeType;
    uint32_t          s_StartTime;   //unit:min
    uint32_t          s_EndTime;     //unit:min
};

struct AlarmOutputInfo
{
	uint32_t          s_EnableFlag; //0-disable, 1-enbale
	uint32_t          s_OutputNumber; //range from 0-3
    char_t            s_Name[ALARM_OUTPUT_MAX_NAME_LENGTH];
	AlarmOutputStatus s_NormalStatus;
	//uint32_t          s_AlarmStatus;
	uint32_t          s_WorkMode;
    uint32_t          s_DelayTime; //unit:second
	ScheduleTimeInfo  s_ScheduleTime[7][MAX_SEG_TIME_PERDAY]; //7 days of every week 
	uint32_t          s_Reserverd[4];
};

struct AlarmInputInfo
{
    uint32_t          s_EnableFlag;      //0-disable, 1-enbale
    uint32_t          s_InputNumber;     //range from 0-3
    char_t            s_Name[ALARM_INPUT_MAX_NAME_LENGTH];
    uint32_t          s_CheckTime; //unit:ms
    AlarmInputStatus  s_NormalStatus;
   	ScheduleTimeInfo  s_ScheduleTime[7][MAX_SEG_TIME_PERDAY]; //7 days of every week 
	//every bit represents the certain AlarmStrategy;
    //0-Alarm output,1-Info record,2-...;
    //value 0-invalid, value 1-valid
	uint32_t          s_LinkAlarmStrategy;
	struct 
    {
        uint8_t  s_IoNum;               //sequence number:0, 1, 2 ...
		uint8_t  s_OperateCmd;          //when link PTZ:0-none, 1-Preset Points ,2-cruise, 3-scan
		uint8_t  s_OperateSeqNum;       //when link PTZ:sequence number:0, 1, 2 ...
    }s_LinkAlarmExtInfo;                //when link alarm out, need IO number.
	uint32_t          s_Reserverd[4];
};

struct HumanDetectData
{
	uint32_t          s_MinSensVal; //human detect use
	uint32_t          s_MaxSensVal; //human detect use
	uint32_t          s_Reserved[4];
};

union UnionExternData
{
    HumanDetectData s_HumanDetectExInfo;
};


struct AlarmEventConfigInfo
{
	uint32_t          s_EnableFlag;      //0-disable, 1-enbale
	ScheduleTimeInfo  s_ScheduleTime[7][MAX_SEG_TIME_PERDAY]; //7 days of every week 
	//every bit represents the certain AlarmStrategy;
    //0-Alarm output,1-Info record,2-...;
    //value 0-invalid, value 1-valid
	uint32_t          s_LinkAlarmStrategy;
	union 
    {
        uint32_t s_IoNum;
    }s_LinkAlarmExtInfo;                //when link alarm out, need IO number.
	uint32_t          s_CheckTime;
	uint32_t          s_Reserverd[4];
	UnionExternData   s_ExtData;
};

struct AlarmScheduleTimeInfo
{
	uint32_t          s_Index;            //when alarm in/out, need IO number
	ScheduleTimeInfo  s_ScheduleTime[7][MAX_SEG_TIME_PERDAY];  //7 days of every week 
	uint8_t           s_Reserved[8];
};

struct AlarmUploadInf
{
    uint64_t s_AlarmId;
    int32_t  s_AlarmType;
	int32_t  s_AlarmLevel;
    int32_t  s_OnOff; //0 for off, 1 for on;
    uint32_t  s_TimeSec;   //unit:s
	uint32_t  s_TimeUsec;  //unit:us
    char_t  s_Description[128];
    union 
    {
        uint32_t s_IoNum;
    }s_ExtraInfo;
	uint8_t  s_Reserved[8];
};




extern AlarmEventConfigInfo g_CurStartedEvent[MAX_NUM_EVENT_TYPE];
extern AlarmInputInfo g_CurStartedAlarmIn[MAX_NUM_GPIO_IN];
extern AlarmOutputInfo g_CurStartedAlarmOut[MAX_NUM_GPIO_OUT];
extern uint64_t        g_AlarmMessageId;

#endif//EVENT_TRANSACTION_HEADER
