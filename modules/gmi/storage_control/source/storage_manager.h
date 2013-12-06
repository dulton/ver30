#ifndef _HD_MANAGER_H_
#define _HD_MANAGER_H_

#include "gmi_storage_ctrl.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define MAX_ENCODER_NUM        0x000000001UL        /*  ��������(��Ƶͨ��)�� */

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

#define  DB_FILE_NAME			"VideoRecord.db"	/*¼���ļ����ݿ�*/
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

#define SEG_REC_LEN_PER_FILE 	(MAX_SEG_PER_FILE*sizeof(SEGMENT_IDX_RECORD))

#define TIME_INTERVAL_UPDATE	15						/*��С���¼��ʱ��(��д��Ƭ�δع̶���Ŀ�����)*/

/*  �������ӿ� */
#define CREATE_LOCK(lock) 		{pthread_mutex_init(lock,NULL);}
#define LOCK(lock) 				{pthread_mutex_lock(lock);}
#define UNLOCK(lock) 			{pthread_mutex_unlock(lock);}
#define RELEASE_LOCK(lock) 		{pthread_mutex_destroy(lock);}

#define ROUND_UP(x, align)      (((int)(x) + ((align) - 1)) & ~((align) - 1))
#define ROUND_DOWN(x, align)    ((int)(x) & ~((align) - 1))


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


/* ����ɵ�¼���ļ��������ļ��е�ƫ����*/
#define FINALIZED_FILE_REC_OFFSET(fileNo)		\
	(sizeof(FILE_IDX_HEADER)+(fileNo)*sizeof(FileIdxHeader))
	
/* ��Ƭ�μ�¼�������ļ��е�ƫ����*/
#define SEG_REC_OFFSET(files, fileNo, recNo)	\
	(sizeof(FILE_IDX_HEADER) + (files)*sizeof(FileIdxHeader)	\
	+ (fileNo)*MAX_SEG_PER_FILE*sizeof(SegmentIdxRecord)	\
	+ (recNo)*sizeof(SegmentIdxRecord))

/* ж���豸*/
int32_t  UmountHdisk(char_t *InDstPathName);

/* �����豸*/
int32_t  MountHdisk(char_t *InSrcPathName, char_t *InDstPathName, char_t *InType);

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


#ifdef __cplusplus
}
#endif

#endif
