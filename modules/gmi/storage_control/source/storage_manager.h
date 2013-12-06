#ifndef _HD_MANAGER_H_
#define _HD_MANAGER_H_

#include "gmi_storage_ctrl.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define MAX_ENCODER_NUM        0x000000001UL        /*  最大编码器(视频通道)数 */

#define MIN(x, y)     (((x) > (y)) ? (y) : (x))
#define MAX(x, y)     (((x) > (y)) ? (x) : (y))

#define  LOCAL_RET_OK           0
#define  LOCAL_RET_ERR          -1
#define FALSE   0
#define TRUE    1


#define  HD_NORMAL         		0
#define  HD_FULL           		1
#define  HD_ERROR          		2

#define  SRC_PATH_NAME_SD		"/dev/mmcblk0"
#define  DST_PATH_NAME_SD		"/mnt/sd1"

#define  DB_FILE_NAME			"VideoRecord.db"	/*录像文件数据库*/
#define  AV_FILE_NAME			".mp4"
#define  VIDEO_TABLE_NAME		"recordVideoTable"		/*录像片段记录表*/
#define  FILE_TABLE_NAME		"recordFileTable"		/*录像文件信息记录表*/
#define  DB_FILE_BAK_NAME		"bakVideoRecord.db"	/*备份录像文件数据库*/

#define  DOS_O_CONTIG_CHK		0
#define  HD_BLOCK_SIZE     		512
#define  UPDATE_SEG_INTERVAL 	(120)
#define  MAX_FILE_RESULTS  		(8*500)

#define  GET_WRITABLE_FILE 		0x10000001				/*获取可写文件名称和片段可写位置*/
#define  FINALIZE_FILE     		0x10000002
#define  UPDATE_TABLE_DB		0x10000003				/*更新数据库中表记录信息*/
#define	 UPDATE_RECORD_FILE		0x10000004				/*新增文件或者循环到开始文件，找到可写位置*/
#define  UPDATE_RECORD_FIRST	0x10000005				/*磁盘已满时更新最早片段录像记录信息*/

#define  ADD_TABLE_DB			0x10000006				/*增加数据库中表记录信息*/

#define  DEFAULT_FILE_MODE 		0644
#define  SECONDS_PER_DAY   		86400		//(24*60*60)
#define  STREAM_FILE_LEN   		(256<<20) 	//256MB      	/*预分配文件的大小*/

#define  MAX_LEN_RECORD			(32<<20)				/*单个录像文件最大字节数*/

#define  MIN_SEG_REC_LEN   		(512<<10) 	//512KB
#define  MAX_SEG_PER_FILE  		(STREAM_FILE_LEN/MIN_SEG_REC_LEN)

#define SEG_REC_LEN_PER_FILE 	(MAX_SEG_PER_FILE*sizeof(SEGMENT_IDX_RECORD))

#define TIME_INTERVAL_UPDATE	15						/*最小更新间隔时间(已写了片段簇固定数目后更新)*/

/*  锁操作接口 */
#define CREATE_LOCK(lock) 		{pthread_mutex_init(lock,NULL);}
#define LOCK(lock) 				{pthread_mutex_lock(lock);}
#define UNLOCK(lock) 			{pthread_mutex_unlock(lock);}
#define RELEASE_LOCK(lock) 		{pthread_mutex_destroy(lock);}

#define ROUND_UP(x, align)      (((int)(x) + ((align) - 1)) & ~((align) - 1))
#define ROUND_DOWN(x, align)    ((int)(x) & ~((align) - 1))


/*索引文件的文件头信息*/
typedef struct tagFileIdxHeader
{
	uint32_t			s_FId;								/*表中记录序号(唯一性)*/
	uint32_t			s_ToRecordFiles;			 		/*这块分区上的总的录像文件数*/
	uint32_t			s_BuildFileNums;					/*已建立的录像文件数目*/
	uint32_t			s_FirstRecordNo;					/*数据表中最早记录的序号*/
	uint32_t			s_LastRecordNo;						/*数据表中最后记录的序号*/
	uint32_t			s_CurWriteFileNo;		 			/*当前可写的录像文件序号*/
	uint32_t			s_CurWriteSegNo;		 			/*当前可写文件的片段序号*/	
	time_t			    s_FirstWriteTime;					/*最早写入时间*/
	time_t			    s_LastWriteTime; 					/*最后写入时间*/	
	uint32_t			s_PartStatus;						/*分区状态*/
	uint32_t			s_SegRecNo;							/*当前片段录像序号(数据库片段表中的记录序号)*/
}FileIdxHeader;


/*文件中录像片段记录信息*/
typedef struct tagSegmentIdxRecord
{
	uint32_t		s_SId;								/*表中记录序号(唯一性)*/
	uint32_t		s_RecFileNo;						/*录像文件序号*/
	uint32_t		s_RecSegNo;							/*录像片段序号*/
	uint32_t		s_RecLastSegLen;					/*片段录像所占最后一个片段簇的实际使用大小*/
	uint16_t		s_RecSegLen;						/*录像片段长度*/
	uint8_t			s_RecType;							/*录像类型*/
	uint8_t			s_RecEncodeNo;						/*编码通道号*/
	time_t			s_StartTime;						/*开始时间*/
	time_t			s_EndTime;							/*结束时间*/
}SegmentIdxRecord;

/*全局设备操作信息*/
typedef struct tagGlobalOperateInfo
{
	uint32_t			s_FId;								/*表中记录序号(唯一性)*/
	uint32_t			s_ToRecordFiles;			 		/*这块分区上的总的录像文件数*/
	uint32_t			s_BuildFileNums;					/*已建立的录像文件数目*/
	uint32_t			s_FirstRecordNo;					/*数据表中最早记录的序号*/
	uint32_t			s_LastRecordNo;						/*数据表中最后记录的序号*/
	uint32_t			s_CurWriteFileNo;		 			/*当前可写的录像文件序号*/
	uint32_t			s_CurWriteSegNo;		 			/*当前可写文件的片段序号*/	
	time_t			    s_FirstWriteTime;					/*最早写入时间*/
	time_t			    s_LastWriteTime; 					/*最后写入时间*/
	uint32_t			s_PartStatus;						/*分区状态*/
	uint32_t			s_SegRecNo;							/*当前片段录像序号*/
	uint32_t			s_IsLinkDB;							/*连接数据库标识*/
}GlobalOperateInfo;


/* 已完成的录像文件在索引文件中的偏移量*/
#define FINALIZED_FILE_REC_OFFSET(fileNo)		\
	(sizeof(FILE_IDX_HEADER)+(fileNo)*sizeof(FileIdxHeader))
	
/* 流片段记录在索引文件中的偏移量*/
#define SEG_REC_OFFSET(files, fileNo, recNo)	\
	(sizeof(FILE_IDX_HEADER) + (files)*sizeof(FileIdxHeader)	\
	+ (fileNo)*MAX_SEG_PER_FILE*sizeof(SegmentIdxRecord)	\
	+ (recNo)*sizeof(SegmentIdxRecord))

/* 卸载设备*/
int32_t  UmountHdisk(char_t *InDstPathName);

/* 挂载设备*/
int32_t  MountHdisk(char_t *InSrcPathName, char_t *InDstPathName, char_t *InType);

/* 初始化分区全局变量*/
void_t InitHdPart();

/* 更新索引文件*/
int32_t  ModifyHdIdxFile(SegmentIdxRecord *InParam);

/* 创建录像文件记录和录像文件中的片段记录*/
int32_t  CreateHdRecFile();

/* 初始化分区信息*/
//int  initHdPartion(unsigned int partNum);

/* 录像文件管理函数*/
int32_t  HdPart_Service(int32_t InCmdType, SegmentIdxRecord *InSegParam);

/* 硬盘格式化*/
int32_t Sdformat(int32_t FileSize);

/* 初始化分区信息*/
int32_t InitPartion(int32_t FileSize);


#ifdef __cplusplus
}
#endif

#endif
