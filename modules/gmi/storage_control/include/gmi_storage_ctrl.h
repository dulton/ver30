#ifndef __GMI_STORAGE_CTRL_H__
#define __GMI_STORAGE_CTRL_H__


#include "gmi_system_headers.h"
#include "gmi_errors.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define MAX_LEN_ADDR_DISK   64
#define MAX_LEN_FILE_REC    128

/*磁盘介质类型*/
#define TYPE_STORAGE_SD     0
#define TYPE_STORAGE_USB    1
#define TYPE_STORAGE_NAS    2

/*磁盘状态*/
#define STATUS_DISK_NORMAL   0    //正常
#define STATUS_DISK_ERR      1    //异常
#define STATUS_DISK_FULL     2    //已满
#define STATUS_DISK_NOFORMAT 3    //未格式化

/*磁盘属性*/
#define ATTR_DISK_WR         0    //可读写
#define ATTR_DISK_W          1    //可写
#define ATTR_DISK_R          2    //可读

/*录像类型*/
#define TYPE_REC_TRIG_TIME   0
#define TYPE_REC_TRIG_MANU   1
#define TYPE_REC_TRIG_MOTION 2
#define TYPE_REC_TRIG_ALARM  3
#define TYPE_REC_TRIG_ALL    10

/*录像查询类型*/
#define TYPE_REC_QUERY_TIME   0    //按时间查询
#define TYPE_REC_QUERY_TRIG   1    //按录像触发类型查询
#define TYPE_REC_QUERY_ALL    2    //按时间和类型查询

/*查询记录最大条数*/
#define	MAX_NUM_QUERY_RECORD  25			/*一次性查询记录最大数*/			

#define OPERATE_RECORD_DOWN   0
#define OPERATE_RECORD_REPLAY 1

//format parameter
typedef struct tagStorageFormat
{
    uint32_t   s_StorageDevice;                    //存储介质类型：0-SD（默认），1-USB，2-NAS
    char_t     s_NasServerAddr[MAX_LEN_ADDR_DISK]; //当存储介质为NAS时使用
    char_t     s_DiskAddr[MAX_LEN_ADDR_DISK];      //当存储介质为NAS时使用
    uint32_t   s_RecFileSize;                      //录像文件大小，{64、128、256、512}单位MB，默认256MB
    uint32_t   s_Reserved[4];
}StorageFormatIn;

//初始化输入参数
typedef struct tagStorageInit
{
    uint32_t   s_StorageDevice;   //存储介质类型：0-SD（默认），1-USB，2-NAS
    uint32_t   s_RecFileSize;     //录像文件大小，{64、128、256、512}单位MB，默认256MB
    uint32_t   s_Reserved[4];
}StorageInitIn;

//反初始化输入参数
typedef struct tagStorageUninit
{
    uint32_t   s_StorageDevice;   //存储介质类型：0-SD（默认），1-USB，2-NAS
    uint32_t   s_Reserved[4];
}StorageUninitIn;

//状态查询输入参数
typedef struct tagStorageStatusQuery
{
    uint32_t   s_StorageDevice;   //存储介质类型：0-SD（默认），1-USB，2-NAS，3-ALL
    uint32_t   s_Reserved[4];
}StorageStatusQueryIn;

//状态查询输出参数
typedef struct tagStorageStatusQueryRes
{
    uint32_t   s_StorageDiskNo;       //磁盘号：从1开始
    uint32_t   s_TotalSpaceVolume;    //磁盘总容量，单位MB
    uint32_t   s_FreeSpaceVolume;     //磁盘剩余空间容量，单位MB
    uint8_t    s_DiskStatus;          //磁盘状态：0-正常（默认）、1-异常、2-已满、3-未格式化
    uint8_t    s_DiskType;            //磁盘类型：0-SD（默认）、1-USB、2-NAS
    uint8_t    s_DiskAttribute;       //磁盘属性：0-可读写（默认）、1-可写、2-可读
    uint8_t    s_DiskFormatRate;      //磁盘格式化进度，[0-100], 单位%
    uint8_t    s_Reserved[4];
}StorageStatusQueryResOut;

//录像时需要配置的输入参数
typedef struct tagRecordParamConfig
{
     uint16_t   s_PreRecordTime;       //预录时间，单位：秒，{0、5（默认）、10、15、20、25、30}
     uint16_t   s_DelayRecordTime;     //录像延时时间，单位：秒，{5（默认）、10、30、60、   
                                       //120、300、600}
     uint8_t   s_IsRecordAudio;        //是否记录音频，0-不记录（默认），1-记录
     uint8_t    s_RecordStreamNo;      //码流类型：0-主码流，1-子码流1,2-子码流2,3-子码流3
     uint8_t    s_IsCirculerRec;       //是否循环录像，0-开启（默认），1-不开启
     uint8_t    s_RecordType;          //录像格式：0-PS（默认）、1-ES
     uint8_t    s_RecordIDueTime;      //录像过期时间，单位：天，0-无过期时间（默认）
     uint8_t    s_IsRedundancyRec;     //是否冗余录像：0-不开启（默认）、1-开启
     uint8_t    s_RecStoreType;        //录像存储方式：0-SD（默认），1-USB，2-NAS
     uint8_t    s_Reserved[5];
}RecordParamConfigIn;

//录像计划配置输入参数
typedef struct tagRecordScheduleConfig
{
    uint8_t   s_Enable;                //录像计划是否启用标记：0-未启用，1-启用
    uint8_t   s_Reserved[11];        
    uint32_t  s_RecStartTime[7][4];    //录像起始时间,高16位-时刻，低16位分钟
    uint32_t  s_RecEndTime[7][4];      //录像停止时间,高16位-时刻，低16位分钟
}RecordScheduleConfigIn;

//NAS配置输入参数
typedef struct tagNasParamConfig
{
    uint32_t   s_DiskNo;                           //磁盘号：从1开始
    char_t     s_NasServerAddr[MAX_LEN_ADDR_DISK]; //NAS地址
    char_t     s_DiskAddr[MAX_LEN_ADDR_DISK];      //磁盘文件地址
    uint8_t   s_Reserved[4];
}NasParamConfigIn;

//录像控制输入参数
typedef struct tagRecordCtrl
{
    uint8_t   s_RecCtrlFlag;          //录像控制：0-停止录像（默认），1-开始录像
    uint8_t   s_RecTrigMode;          //录像触发类型：0-定时录像（默认），1-手动录像，2-移动侦测,3-报警
    uint8_t   s_RecTrigChan;          //触发通道号
    uint8_t   s_AlarmTrigNo;          //报警触发通道号
    uint8_t   s_EncodeType;          //0-None, 1-H264,2-MJPEG
    uint8_t   s_AudioType;           //0-None,1-G711a, 2-G711u, 3-G726
    uint8_t   s_VideoFrame;          //视频帧率
    uint8_t   s_AudioFrame;          //音频帧率
    uint8_t   s_Reserved[4]; 
    uint16_t   s_VideoWide;           //视频宽
    uint16_t   s_VideoHeight;          //视频高
}RecordCtrlIn;

//录像文件查询输入参数
typedef struct tagRecordFileQuery
{
 	uint8_t   s_RecQueryType;         //录像文件查询类型：0-按时间（默认），1-按录像触发类型，
                                      //2-按时间和录像触发类型
    uint8_t   s_RecTrigMode;          //录像触发类型，从低到高位，第1位为1-定时录像
                                      //第2位为1-手动录像，第3位为1-移动侦测，第4位为1-报警
    uint8_t   s_Channel;              //录像通道号
    uint8_t   s_Reserved[5];          
    uint32_t  s_RecQueryTime[2];      //录像查询时间段，UTC时间，单位：秒
}RecordFileQueryIn;

//录像文件查询输出参数
typedef struct tagRecordFileQueryRes
{
    uint8_t   s_RecTrigMode;                       //录像触发类型，从低到高位，第1位为1-定时录像
                                                   //第2位为1-手动录像，第3位为1-移动侦测，
                                                   //第4位为1-报警
    uint8_t   s_Reserved[3];             
    char_t    s_RecFileName[MAX_LEN_FILE_REC];     //录像文件名称
    uint32_t  s_RecFileSize;                       //录像文件大小，单位：MB
    uint32_t  s_RecFileTime[2];                    //录像文件起始时间，UTC时间，单位：秒
}RecordFileQueryResOut;

//录像下载和回放查询输入参数
typedef struct tagReplayInfo
{
    uint32_t  s_RecReplayStartTime;    //录像回放起始时间点，UTC时间，单位：秒，
                                       //当录像查询类型为回放时用
    uint32_t  s_RecReplayEndTime;      //录像回放结束时间点，UTC时间，单位：秒，
                                       //当录像查询类型为回放时用
    uint32_t  s_LastReplayStartTime;   //上次录像回放文件的起始时间，UTC时间，
                                       //第一次进行回放查询时与录像回放起始时间点相同
    uint32_t  s_LastReplayEndTime;     //上次录像回放文件的终止时间，UTC时间，
                                       //第一次进行回放查询时与录像回放起始时间点相同
    uint16_t  s_ExpReplayFileNo;       //期望回放的录像文件在回放总文件数中的序号，从1开始
    uint16_t  s_Reserved;
}ReplayInfo ;

typedef struct tagRecordDownReplayQuery
{
    uint8_t      s_RecQueryType;                     //录像查询类型：0-下载，1-回放
    uint8_t      s_Reserved[7];          
    union 
    {
        char_t   s_RecFileName[MAX_LEN_FILE_REC];     //录像文件名称（查询显示在web上的文件名称）,
                                                      //当录像查询类型为下载时用
        ReplayInfo s_TimeRangeInfo;                   //录像回放起始和结束时间点，UTC时间，单位：秒，
                                                      //当录像查询类型为回放时用
    }RecDownReplayInfo;
}RecordDownReplayQueryIn;

//录像下载和回放查询输出参数
typedef struct tagRecordDownReplayQueryRes
{
    uint8_t   s_RecQueryType;        //录像查询类型：0-下载，1-回放
    uint8_t   s_StreamType;          //0-PS,1-ES    
	uint8_t   s_EncodeType; 		 //0-None, 1-H264,2-MJPEG	
    uint8_t   s_AudioType;			 //0-None,1-G711a, 2-G711u, 3-G726, 
    uint8_t   s_VideoFrame;          //视频帧率
    uint8_t   s_AudioFrame;          //音频帧率
    uint8_t   s_Reserved[2]; 
    uint16_t   s_VideoWide;           //视频宽
    uint16_t   s_VideoHeight;         //视频高           
    char_t    s_RecFileName[MAX_LEN_FILE_REC];     //录像文件路径和名称（真正磁盘中存储的文件名称）
    uint32_t  s_RecFileOffset;        //录像文件读取位置
    uint32_t  s_RecFileSize;		  //录像文件大小，回放时作为当前要回放的文件大小,
                                      //单位：字节
    uint16_t  s_RecFileReplayNum;     //需要回放的录像文件数，对录像回放有用 
    uint16_t  s_RecFileCurNo;         //回放的录像数据处于当前文件的序号，对录像回放有用，
                                      //从1开始
    uint32_t  s_CurFileStartTime;     //当前回放文件的起始时间点，UTC时间，单位：秒，
                                      //当录像查询类型为回放时用
    uint32_t  s_CurFileEndTime;       //当前回放文件的结束时间点，UTC时间，单位：秒，
                                      //当录像查询类型为回放时用
}RecordDownReplayQueryResOut;

/*===============================================================
func name:GMI_StorageDeviceFormat
func:format storage device
input:StorageFormatParamPtr--format parameter;
return:success--return GMI_SUCCESS, 
	failed -- return ERROR CODE
---------------------------------------------------------------------*/
GMI_RESULT GMI_StorageDeviceFormat(StorageFormatIn *StorageFormatParamPtr);

/*===============================================================
func name:GMI_StorageDeviceInit
func:initialize storage device
input:StorageInitParamPtr--initialized parameter;
return:success--return GMI_SUCCESS, 
	failed -- return ERROR CODE
---------------------------------------------------------------------*/
GMI_RESULT GMI_StorageDeviceInit(StorageInitIn *StorageInitParamPtr);

/*===============================================================
func name:GMI_StorageDeviceUninit
func:uninitialize storage device
input:StorageUninitParamPtr--uninitialized parameter;
return:success--return GMI_SUCCESS, 
	failed -- return ERROR CODE
---------------------------------------------------------------------*/
GMI_RESULT GMI_StorageDeviceUninit(StorageUninitIn *StorageUninitParamPtr);

/*===============================================================
func name:GMI_StorageDeviceStatusQuery
func:query status of  storage device
input:DevStatusQueryPtr-query parameter;
        DevStatusQueryNum-number of query array
output:DevStatusQueryResPtr-query result;
          DevStatusNum-query number;
return:success--return GMI_SUCCESS, 
	failed -- return ERROR CODE
---------------------------------------------------------------------*/
GMI_RESULT GMI_StorageDeviceStatusQuery(StorageStatusQueryIn *DevStatusQueryPtr,
	                                            StorageStatusQueryResOut **DevStatusQueryResPtr, 
	                                            uint32_t DevStatusQueryNum, uint32_t  *DevStatusNum);


/*===============================================================
func name:GMI_RecordParamConfig
func:config of  record
input:RecordParamConfigPtr--config parameter;
return:success--return GMI_SUCCESS, 
	failed -- return ERROR CODE
---------------------------------------------------------------------*/
GMI_RESULT GMI_RecordParamConfig(RecordParamConfigIn *RecordParamConfigPtr);

/*===============================================================
func name:GMI_RecordScheduleConfig
func:config Schedule of  record
input:RecordScheduleConfigPtr--Schedule parameter;
return:success--return GMI_SUCCESS, 
	failed -- return ERROR CODE
---------------------------------------------------------------------*/
GMI_RESULT  GMI_RecordScheduleConfig(RecordScheduleConfigIn *RecordScheduleConfigPtr);

/*===============================================================
func name:GMI_NasParamConfig
func:config NAS
input:NasParamConfigPtr--NAS parameter;
return:success--return GMI_SUCCESS, 
	failed -- return ERROR CODE
---------------------------------------------------------------------*/
GMI_RESULT  GMI_NasParamConfig(NasParamConfigIn *NasParamConfigPtr);

/*===============================================================
func name:GMI_RecordCtrl
func:control record
input:RecordCtrlPtr--control record parameter;
return:success--return GMI_SUCCESS, 
	failed -- return ERROR CODE
---------------------------------------------------------------------*/
GMI_RESULT  GMI_RecordCtrl(RecordCtrlIn *RecordCtrlPtr);

/*===============================================================
func name:GMI_RecordFileQuery
func:query record file
input:RecordFileQueryPtr--query condition of record;
        CurQueryPosNo--current query starting number+return the last record number of current query result;
        QueryResArraySize--size of array of query result
output:RecordFileQueryResPtr-query result;
          QueryResTotalNum-query record total number;
          QueryResCurNum-current record number;
return:success--return GMI_SUCCESS, 
	failed -- return ERROR CODE
---------------------------------------------------------------------*/
GMI_RESULT GMI_RecordFileQuery(RecordFileQueryIn *RecordFileQueryPtr, uint32_t *CurQueryPosNo, 
                                     RecordFileQueryResOut **RecordFileQueryResPtr, uint32_t QueryResArraySize, 
	                                 uint32_t  *QueryResTotalNum, uint32_t  *QueryResCurNum);


/*===============================================================
func name:GMI_RecordDownReplayQuery
func:query record file of download or replay
input:RecordDownReplayQueryPtr--query condition of record;       
        QueryResArraySize--size of array of query result
output:RecordDownReplayQueryResPtr-query result;
return:success--return GMI_SUCCESS, 
	failed -- return ERROR CODE
---------------------------------------------------------------------*/

GMI_RESULT GMI_RecordDownReplayQuery(RecordDownReplayQueryIn *RecordDownReplayQueryPtr,
       RecordDownReplayQueryResOut **RecordDownReplayQueryResPtr, uint32_t QueryResArraySize);


/*===============================================================
func name:GMI_StorageVersionQuery
func:query storage version
output:StorageVer--version;
          VerMaxLen--max length of version string 
return:success--return GMI_SUCCESS, 
	failed -- return ERROR CODE
---------------------------------------------------------------------*/
GMI_RESULT  GMI_StorageVersionQuery(char_t *StorageVer, int32_t VerMaxLen);


#ifdef __cplusplus
}
#endif
#endif
