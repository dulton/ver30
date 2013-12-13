#ifndef _HD_MANAGER_H_
#define _HD_MANAGER_H_

#include "gmi_storage_ctrl.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define MAX_ENCODER_NUM        0x000000001        /*  ��������(��Ƶͨ��)�� */

#define MIN(x, y)     (((x) > (y)) ? (y) : (x))
#define MAX(x, y)     (((x) > (y)) ? (x) : (y))

#define  LOCAL_RET_OK           0
#define  LOCAL_RET_ERR          -1
#define FALSE   0
#define TRUE    1


#define  HD_NORMAL         		0
#define  HD_ERROR          		1
#define  HD_FULL           		2

#define  FLAG_ALLDAY_REC_CLOSE  0
#define  FLAG_ALLDAY_REC_OPEN   1

#define  SRC_PATH_NAME_SD		"/dev/mmcblk0"
#define  DST_PATH_NAME_SD		"/mnt/sd1"

#define  DB_FILE_NAME			"videoRecord.db"	/*¼���ļ����ݿ�*/
#define  AV_FILE_NAME			".mp4"
#define  VIDEO_TABLE_NAME		"recordVideoTable"		/*¼��Ƭ�μ�¼��*/
#define  FILE_TABLE_NAME		"recordFileTable"		/*¼���ļ���Ϣ��¼��*/
#define  DB_FILE_BAK_NAME		"bakVideoRecord.db"	/*����¼���ļ����ݿ�*/

#define  DOS_O_CONTIG_CHK		0
#define  HD_BLOCK_SIZE     		512
#define  UPDATE_SEG_INTERVAL 	(120)
#define  MAX_FILE_RESULTS  		(8*500)

#define  GET_WRITABLE_FILE 		0x10000001				/*��ȡ��д�ļ����ƺ�Ƭ�ο�дλ��*/
#define  FINALIZE_FILE     		0x10000002
#define  UPDATE_TABLE_DB		0x10000003				/*�������ݿ��б��¼��Ϣ*/
#define	 UPDATE_RECORD_FILE		0x10000004				/*�����ļ�����ѭ������ʼ�ļ����ҵ���дλ��*/
#define  UPDATE_RECORD_FIRST	0x10000005				/*��������ʱ��������Ƭ��¼���¼��Ϣ*/

#define  ADD_TABLE_DB			0x10000006				/*�������ݿ��б��¼��Ϣ*/

#define  DEFAULT_FILE_MODE 		0644
#define  SECONDS_PER_DAY   		86400		//(24*60*60)
#define  STREAM_FILE_LEN   		(256<<20) 	//256MB      	/*Ԥ�����ļ��Ĵ�С*/

#define  MAX_LEN_RECORD			(32<<20)				/*����¼���ļ�����ֽ���*/

#define  MIN_SEG_REC_LEN   		(512<<10) 	//512KB
#define  MAX_SEG_PER_FILE  		(STREAM_FILE_LEN/MIN_SEG_REC_LEN)

#define SEG_REC_LEN_PER_FILE 	(MAX_SEG_PER_FILE*sizeof(SegmentIdxRecord))

#define TIME_INTERVAL_UPDATE	15						/*��С���¼��ʱ��(��д��Ƭ�δع̶���Ŀ�����)*/

//schedule��Ϣ��������Ϣ����
#define REC_TRIG_ALARMIN           0x01
#define REC_TRIG_MOTDETECT         0x02
#define REC_TRIG_TIME              0x03

//��Ϣ��������Ϣkey
#define MSG_TYPE_TRIG_REC          0x1001
#define MSG_TYPE_REC               0x1002

//stream��Ϣ��������Ϣ����
#define CMD_TYPE_STOP_REC          0x2001
#define CMD_TYPE_START_REC         0x2002
#define CMD_TYPE_REFRESH_I         0x2003

//¼���ļ�����
#define TYPE_REC_TIME              0x01
#define TYPE_REC_ALARM             0x02
#define TYPE_REC_MOTION            0x03
#define TYPE_REC_ALL               0x7F



/*  �������ӿ� */
#define CREATE_LOCK(lock) 		{pthread_mutex_init(lock,NULL);}
#define LOCK(lock) 				{pthread_mutex_lock(lock);}
#define UNLOCK(lock) 			{pthread_mutex_unlock(lock);}
#define RELEASE_LOCK(lock) 		{pthread_mutex_destroy(lock);}

#define ROUND_UP(x, align)      (((int)(x) + ((align) - 1)) & ~((align) - 1))
#define ROUND_DOWN(x, align)    ((int)(x) & ~((align) - 1))

/*����I֡��������*/
#define MAX_NUM_IFRAME            200

#define MEDIA_BUFFER_SIZE         0x00100000UL   /*  ��Ƶ�ص����ջ�������С,1M  */
#define AUDIO_BUFFER_SIZE         0x00020000UL   /*  ��Ƶ���ջ�������С,128K  */

/*�ֱ��ʳߴ�*/
#define RESOLUTION_1080P_WIDTH     1920
#define RESOLUTION_1080P_HEIGHT    1080
#define RESOLUTION_720P_WIDTH      1280
#define RESOLUTION_720P_HEIGHT     720
#define RESOLUTION_576P_WIDTH      720
#define RESOLUTION_576P_HEIGHT     576
#define RESOLUTION_D1_WIDTH        704
#define RESOLUTION_D1_HEIGHT       576
#define RESOLUTION_480P_WIDTH      640
#define RESOLUTION_480P_HEIGHT     480
#define RESOLUTION_CIF_WIDTH       352
#define RESOLUTION_CIF_HEIGHT      288


/*�汾*/
#define VERSION_STORAGE           "storage library v1.0"

/*�����ļ����ļ�ͷ��Ϣ*/
typedef struct tagFileIdxHeader
{
	uint32_t			s_FId;								/*���м�¼���(Ψһ��)*/
	uint32_t			s_ToRecordFiles;			 		/*�������ϵ��ܵ�¼���ļ���*/
	uint32_t			s_BuildFileNums;					/*�ѽ�����¼���ļ���Ŀ*/
	uint32_t			s_FirstRecordNo;					/*���ݱ��������¼�����*/
	uint32_t			s_LastRecordNo;						/*���ݱ�������¼�����*/
	uint32_t			s_CurWriteFileNo;		 			/*��ǰ��д��¼���ļ����*/
	uint32_t			s_CurWriteSegNo;		 			/*��ǰ��д�ļ���Ƭ�����*/	
	time_t			    s_FirstWriteTime;					/*����д��ʱ��*/
	time_t			    s_LastWriteTime; 					/*���д��ʱ��*/	
	uint32_t			s_PartStatus;						/*����״̬*/
	uint32_t			s_SegRecNo;							/*��ǰƬ��¼�����(���ݿ�Ƭ�α��еļ�¼���)*/
}FileIdxHeader;


/*�ļ���¼��Ƭ�μ�¼��Ϣ*/
typedef struct tagSegmentIdxRecord
{
	uint32_t		s_SId;								/*���м�¼���(Ψһ��)*/
	uint32_t		s_RecFileNo;						/*¼���ļ����*/
	uint32_t		s_RecSegNo;							/*¼��Ƭ�����*/
	uint32_t		s_RecLastSegLen;					/*Ƭ��¼����ռ���һ��Ƭ�δص�ʵ��ʹ�ô�С*/
	uint16_t		s_RecSegLen;						/*¼��Ƭ�γ���*/
	uint8_t			s_RecType;							/*¼������*/
	uint8_t			s_RecEncodeNo;						/*����ͨ����*/
	time_t			s_StartTime;						/*��ʼʱ��*/
	time_t			s_EndTime;							/*����ʱ��*/
	uint32_t        s_SegFileInfo;                      /*�ӵ͵��ߣ�bit0-bit7������Ƶ֡�ʣ�
	                                                                                          bit8-bit15������Ƶ֡�ʣ�
	                                                                                          bit16-bit19����ֱ���Ƶ�ֱ���(0-None,1-1080P,2-720P,3-576P,4-D1,5-480P,6-CIF)��
	                                                                                          bit20-bit23������Ƶ��������(0-None,1-G711a, 2-G711u, 3-G726)��
	                                                                                          bit24-bit27������Ƶ��������(0-None, 1-H264,2-MJPEG)*/
}SegmentIdxRecord;

/*ȫ���豸������Ϣ*/
typedef struct tagGlobalOperateInfo
{
	uint32_t			s_FId;								/*���м�¼���(Ψһ��)*/
	uint32_t			s_ToRecordFiles;			 		/*�������ϵ��ܵ�¼���ļ���*/
	uint32_t			s_BuildFileNums;					/*�ѽ�����¼���ļ���Ŀ*/
	uint32_t			s_FirstRecordNo;					/*���ݱ��������¼�����*/
	uint32_t			s_LastRecordNo;						/*���ݱ�������¼�����*/
	uint32_t			s_CurWriteFileNo;		 			/*��ǰ��д��¼���ļ����*/
	uint32_t			s_CurWriteSegNo;		 			/*��ǰ��д�ļ���Ƭ�����*/	
	time_t			    s_FirstWriteTime;					/*����д��ʱ��*/
	time_t			    s_LastWriteTime; 					/*���д��ʱ��*/
	uint32_t			s_PartStatus;						/*����״̬*/
	uint32_t			s_SegRecNo;							/*��ǰƬ��¼�����*/
	uint32_t			s_IsLinkDB;							/*�������ݿ��ʶ*/
}GlobalOperateInfo;


typedef struct tagRecordTriggerMsg
{
    long_t     s_MsgType;        /*��Ϣ����*/
	uint32_t   s_CmdType;        /*��������*/
    uint32_t   s_TriggerChans;   /*����ͨ��*/
    uint32_t   s_SegFileInfo;    /*�ӵ͵��ߣ�bit0-bit7������Ƶ֡�ʣ�
								  bit8-bit15������Ƶ֡�ʣ�
								  bit16-bit19����ֱ���Ƶ�ֱ���(0-None,1-1080P,2-720P,3-576P,4-D1,5-480P, 6-CIF)��
								  bit20-bit23������Ƶ��������(0-None,1-G711a, 2-G711u, 3-G726)��
								  bit24-bit27������Ƶ��������(0-None, 1-H264,2-MJPEG)*/
	char_t     s_Reserved[8];       
}RecordTriggerMsg;


/*  ������ */
typedef struct tagRecordDataBuffer
{
    char_t     *s_Buffer;
    ulong_t    s_WritePos;
    ulong_t    s_IFarmePos;
    ulong_t    s_ReadPos;
    ulong_t    s_Size;
    ulong_t    s_TotalWrite;
    ulong_t    s_FrameId;
}RecordDataBuffer;

typedef struct tagIFrameInfo
{
    time_t      s_Time;
    ulong_t     s_Idx;
}IFrameInfo;


/*  ��Ƶ¼����Ϣ���� */
typedef struct tagRecordInfoManage
{
	int32_t            s_StreamId;           /* ��Ƶ��ID*/ 
	int32_t            s_RecordMsgId;        /*  ¼����Ϣ����ID��*/
	int32_t            s_RecordStart;        /*  ¼��ʼ��־*/
	uint8_t            s_CurrRecType;        /*  ��ǰ¼������*/
	uint8_t            s_Res[3];
	IFrameInfo         s_IFrames[MAX_NUM_IFRAME];
	ulong_t            s_IFrameNums;         /*  ��ǰI֡��*/
    RecordDataBuffer   s_RecordBuffer;       /*  ��Ƶ¼�񻺳���*/ 

}RecordInfoManage;

typedef struct tagTimeSegment
{
    time_t            s_StartTime;        /*��ʼʱ��*/
    time_t            s_StopTime;         /*����ʱ��*/
}TimeSegment;

typedef struct tagRecordParam
{
    TimeSegment       s_RecordTimeSeg;    /*¼��ʱ���*/
	uint8_t           s_RecType;          /*¼������*/
	uint8_t           s_Res[3];
}RecordParam;

typedef struct tagRecordMsgType
{
	long_t     s_MsgType;          /*��Ϣ����*/
	uint32_t   s_CmdType;          /*��������*/
	uint8_t    s_RecordType;       /*¼������*/
	uint8_t    s_Res[3];           /*����*/
	ulong_t    s_Idx;              /*I֡λ��*/
	time_t     s_Time;             /*I֡ʱ��*/
}RecordMsgType;


typedef struct tagAlldayRecParam
{
	uint16_t	  s_IsAllDayRecord; /*�Ƿ�ȫ��¼��,0-�ر�,1-����ȫ��¼��*/
	uint8_t	      s_RecType;        /*¼������*/
	uint8_t	      s_Reserved;       
}AlldayRecParam;

/*��Ƶ¼�������������*/
typedef struct tagRecParamCfg
{
	ulong_t	          s_RecDelayTime;    /*  ¼����ʱʱ��*/
	ulong_t	          s_PreRecordTime;   /*  Ԥ¼ʱ��*/
	uint16_t	      s_IsEnableRecord;	   /*  ¼��ƻ�ʹ��λ*/
	uint16_t	      s_IsCyclicRecord;	   /*  �Ƿ�ѭ��¼��*/
	RecordParam	      s_RecordParam[7][4]; /*  ¼�����*/
	AlldayRecParam    s_AllDayRecParam[7]; /*  ȫ��¼�����*/
}RecParamCfg;

/*�߼��ļ��ļ���Ϣ*/
typedef struct tagSegFileInfo
{
	uint8_t   s_EncodeType;          //0-None, 1-H264,2-MJPEG
    uint8_t   s_AudioType;           //0-None,1-G711a, 2-G711u, 3-G726
    uint8_t   s_VideoFrame;          //��Ƶ֡��
    uint8_t   s_AudioFrame;          //��Ƶ֡��
    uint8_t   s_Reserved[4]; 
    uint16_t  s_VideoWide;           //��Ƶ��
    uint16_t  s_VideoHeight;          //��Ƶ��
}SegFileInfo;


/* ����ɵ�¼���ļ��������ļ��е�ƫ����*/
#define FINALIZED_FILE_REC_OFFSET(fileNo)		\
	(sizeof(FILE_IDX_HEADER)+(fileNo)*sizeof(FileIdxHeader))
	
/* ��Ƭ�μ�¼�������ļ��е�ƫ����*/
#define SEG_REC_OFFSET(files, fileNo, recNo)	\
	(sizeof(FILE_IDX_HEADER) + (files)*sizeof(FileIdxHeader)	\
	+ (fileNo)*MAX_SEG_PER_FILE*sizeof(SegmentIdxRecord)	\
	+ (recNo)*sizeof(SegmentIdxRecord))


/*¼��������ݳ�ʼ��*/
int32_t VidRecordInit();

/*¼��������ݷ���ʼ��*/
int32_t VidRecordUninit();

/* ��ʼ������ȫ�ֱ���*/
void_t InitHdPart();

/* ���������ļ�*/
int32_t  ModifyHdIdxFile(SegmentIdxRecord *InParam);

/* ����¼���ļ���¼��¼���ļ��е�Ƭ�μ�¼*/
int32_t  CreateHdRecFile();

/* ��ʼ��������Ϣ*/
//int  initHdPartion(unsigned int partNum);

/* ¼���ļ�������*/
int32_t  HdPart_Service(int32_t InCmdType, SegmentIdxRecord *InSegParam);

/* Ӳ�̸�ʽ��*/
int32_t Sdformat(int32_t FileSize);

/* ��ʼ��������Ϣ*/
int32_t InitPartion(int32_t FileSize);

/*����¼�����߳�*/
int32_t CreateRecProcessThread(int32_t Chan);

/*��ȡ����״̬*/
int32_t GetSDStatus(StorageStatusQueryResOut **QueryResPtr, 
	                     uint32_t QueryArrayNum,uint32_t  *QueryResNum);

/*����¼�����*/
int32_t SetRecConfigParam(RecordParamConfigIn *InParam);

/*����¼��ƻ�*/
int32_t SetRecScheduleConfig(RecordScheduleConfigIn *InParam);

/*¼�����֪ͨ*/
int32_t  RecordCtrlNotify(RecordCtrlIn *InParam);

/*¼���ļ���ѯ*/
int32_t QueryRecordFile(RecordFileQueryIn *RecordFileQueryPtr, uint32_t *CurQueryPosNo, 
                            RecordFileQueryResOut **RecordFileQueryResPtr, uint32_t QueryResArraySize,
                            uint32_t  *QueryResTotalNum, uint32_t  *QueryResCurNum);


#ifdef __cplusplus
}
#endif

#endif
