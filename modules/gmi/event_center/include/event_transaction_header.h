// event_transaction_header.h

#if !defined( EVENT_TRANSACTION_HEADER )
#define EVENT_TRANSACTION_HEADER

// detector id
#define EVENT_DETECTOR_ID_ALARM_INPUT     1
#define EVENT_DETECTOR_ID_HUMAN_DETECT    2

#define EVENT_DETECTOR_ID_MOTION_DETECT   3
#define EVENT_DETECTOR_ID_VCOVER_DETECT   4  //video cover
#define EVENT_DETECTOR_ID_VLOSE_DETECT    5  //video lose
#define EVENT_DETECTOR_ID_ABNORMAL_DETECT 6  //other abnormal



// processor id
#define EVENT_PROCESSOR_ID_ALARM_OUTPUT  1
#define EVENT_PROCESSOR_ID_INFO_UPLOAD   2

#define EVENT_PROCESSOR_ID_ALARM_AUDIO   3
#define EVENT_PROCESSOR_ID_LINK_MAIL     4
#define EVENT_PROCESSOR_ID_UPLOAD_FTP    5
#define EVENT_PROCESSOR_ID_LINK_RECORD   6
#define EVENT_PROCESSOR_ID_LINK_PTZ      7
#define EVENT_PROCESSOR_ID_LINK_CAP      8
#define EVENT_PROCESSOR_ID_LINK_LIGHT    9

//schedule id 
#define SCHEDULE_TIME_ID_ALARM_IN        1
#define SCHEDULE_TIME_ID_ALARM_OUT       2
#define SCHEDULE_TIME_ID_HUMAN_DETECT    3


//Max value
#define ALARM_INPUT_MAX_NAME_LENGTH   32
#define ALARM_OUTPUT_MAX_NAME_LENGTH  32
#define MAX_NUM_EVENT_TYPE            10
#define MAX_NUM_GPIO_IN               4
#define MAX_NUM_GPIO_OUT              4
#define MAX_SEG_TIME_PERDAY           4

//event flag
#define FLAG_EVENT_DISABLE            0
#define FLAG_EVENT_ENABLE             1

#if 0
enum AlarmEventType
{
    e_AlarmEventType_AlarmInput = 1,
    e_AlarmEventType_HumanDetect,
};
#endif


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

struct AlarmOutputInfo
{
	uint32_t          s_EnableFlag;  //0-disable, 1-enbale
	uint32_t          s_OutputNumber; //range from 0-3
    char_t            s_Name[ALARM_OUTPUT_MAX_NAME_LENGTH];
	AlarmOutputStatus s_NormalStatus; //io status
    uint32_t          s_DelayTime;    //unit:second
	uint32_t          s_Reserverd[4];
};

struct LinkAlarmExtInfo
{
    uint8_t  s_IoNum;               //sequence number:every bit,0-output1, 1-output2, 2-output3 , when bit value:1- valid,0-invalid 
	uint8_t  s_OperateCmd;          //when link PTZ:0-none, 1-Preset Points ,2-cruise, 3-scan
	uint16_t s_OperateSeqNum;       //when link PTZ:sequence number:0, 1, 2 ...
	uint16_t s_DelayTime;           //when link light,unit: sencond
	uint16_t s_DutyRatio;           //when link light,reserved
	uint8_t  s_Reserverd[4];
};


struct AlarmInputInfo
{
    uint32_t          s_EnableFlag;      //0-disable, 1-enbale
    uint32_t          s_InputNumber;     //range from 0-3
    char_t            s_Name[ALARM_INPUT_MAX_NAME_LENGTH];
    uint32_t          s_CheckTime;       //poll time, unit:ms
    AlarmInputStatus  s_NormalStatus;    //io status
	//every bit represents the certain AlarmStrategy;
    //0-Alarm output,1-Info upload,2-audio, 3-mail, 4-FTP, 5-record, 6-PTZ, 7-capture picture, 8-light;
    //value 0-invalid, value 1-valid
	uint32_t          s_LinkAlarmStrategy;
	LinkAlarmExtInfo  s_LinkAlarmExtInfo;  //when link alarm out, PTZ      
	uint32_t          s_Reserverd[4];
};

struct HumanDetectData
{
	uint32_t          s_MinSensVal; //human detect use
	uint32_t          s_MaxSensVal; //human detect use
	uint32_t          s_Sensitivity; //range from 0-100
	uint32_t          s_Reserved[4];
};

union UnionExternData
{
    HumanDetectData s_HumanDetectExInfo;
};


struct AlarmEventConfigInfo
{
	uint32_t          s_AlarmEventType;  //type:EVENT_DETECTOR_ID_XX
	uint32_t          s_EnableFlag;      //0-disable, 1-enbale
	//every bit represents the certain AlarmStrategy;
    //0-Alarm output,1-Info upload,2-audio, 3-mail, 4-FTP, 5-record, 6-PTZ, 7-capture picture,  8-light;
    //value 0-invalid, value 1-valid
	uint32_t          s_LinkAlarmStrategy;
	LinkAlarmExtInfo  s_LinkAlarmExtInfo;    //when link alarm out, PTZ.
	uint32_t          s_CheckTime;           //poll time,unit:ms
	uint32_t          s_Reserverd[4];
	UnionExternData   s_ExtData;
};

struct ScheduleTimeInfo
{
    //enum TimeType     s_TimeType;
    uint32_t          s_StartTime;   //unit:min
    uint32_t          s_EndTime;     //unit:min
};

struct AlarmScheduleTimeInfo
{
	uint32_t          s_ScheduleId;       //type:SCHEDULE_TIME_ID_XX
	uint32_t          s_Index;            //when alarm in/out, need IO number, range:0-3
	ScheduleTimeInfo  s_ScheduleTime[7][MAX_SEG_TIME_PERDAY];  //7 days of every week 
	uint8_t           s_Reserved[8];
};

union AlarmUploadExtraInfo
{
	uint32_t s_IoNum;
};

struct AlarmUploadInf
{
    uint64_t  s_AlarmId;    //sequence number,from1-0xEFFFFFFF
    int32_t   s_AlarmType;  //type:EVENT_DETECTOR_ID_XX
	int32_t   s_AlarmLevel; //alarm level,default:0
    int32_t   s_OnOff;      //0 for off, 1 for on;
    uint32_t  s_TimeSec;    //unit:s
	uint32_t  s_TimeUsec;   //unit:us
    char_t    s_Description[128];
    AlarmUploadExtraInfo s_ExtraInfo;
	uint8_t   s_Reserved[8];
};


#endif//EVENT_TRANSACTION_HEADER
