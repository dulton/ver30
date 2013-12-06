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

#define TYPE_STORAGE_SD     0
#define TYPE_STORAGE_USB    1
#define TYPE_STORAGE_NAS    2



//format parameter
typedef struct tagStorageFormat
{
    uint32_t   s_StorageDevice;                    //�洢�������ͣ�0-SD��Ĭ�ϣ���1-USB��2-NAS
    char_t     s_NasServerAddr[MAX_LEN_ADDR_DISK]; //���洢����ΪNASʱʹ��
    char_t     s_DiskAddr[MAX_LEN_ADDR_DISK];      //���洢����ΪNASʱʹ��
    uint32_t   s_RecFileSize;                      //¼���ļ���С��{64��128��256��512}��λMB��Ĭ��256MB
    uint32_t   s_Reserved[4];
}StorageFormatIn;

//��ʼ���������
typedef struct tagStorageInit
{
    uint32_t   s_StorageDevice;   //�洢�������ͣ�0-SD��Ĭ�ϣ���1-USB��2-NAS
    uint32_t   s_RecFileSize;                      //¼���ļ���С��{64��128��256��512}��λMB��Ĭ��256MB
    uint32_t   s_Reserved[4];
}StorageInitIn;

//����ʼ���������
typedef struct tagStorageUninit
{
    uint32_t   s_StorageDevice;   //�洢�������ͣ�0-SD��Ĭ�ϣ���1-USB��2-NAS
    uint32_t   s_Reserved[4];
}StorageUninitIn;

//״̬��ѯ�������
typedef struct tagStorageStatusQuery
{
    uint32_t   s_StorageDevice;   //�洢�������ͣ�0-SD��Ĭ�ϣ���1-USB��2-NAS��3-ALL
    uint32_t   s_Reserved[4];
}StorageStatusQueryIn;

//״̬��ѯ�������
typedef struct tagStorageStatusQueryRes
{
    uint32_t   s_StorageDiskNo;       //���̺ţ���1��ʼ
    uint32_t   s_TotalSpaceVolume;    //��������������λMB
    uint32_t   s_FreeSpaceVolume;     //����ʣ��ռ���������λMB
    uint8_t    s_DiskStatus;          //����״̬��0-������Ĭ�ϣ���1-�쳣��2-����
    uint8_t    s_DiskType;            //�������ͣ�0-SD��Ĭ�ϣ���1-USB��2-NAS
    uint8_t    s_DiskAttribute;       //�������ԣ�0-�ɶ�д��Ĭ�ϣ���1-��д��2-�ɶ�
    uint8_t    s_DiskFormatRate;      //���̸�ʽ�����ȣ�[0-100], ��λ%
    uint8_t    s_Reserved[4];
}StorageStatusQueryResOut;

//¼��ʱ��Ҫ���õ��������
typedef struct tagRecordParamConfig
{
     uint16_t   s_PreRecordTime;       //Ԥ¼ʱ�䣬��λ���룬{0��5��Ĭ�ϣ���10��15��20��25��30}
     uint16_t   s_DelayRecordTime;     //¼����ʱʱ�䣬��λ���룬{5��Ĭ�ϣ���10��30��60��   
                                       //120��300��600}
     uint8_t   s_IsRecordAudio;        //�Ƿ��¼��Ƶ��0-����¼��Ĭ�ϣ���1-��¼
     uint8_t    s_RecordStreamNo;      //�������ͣ�0-��������1-������1,2-������2,3-������3
     uint8_t    s_IsCirculerRec;       //�Ƿ�ѭ��¼��0-������Ĭ�ϣ���1-������
     uint8_t    s_RecordType;          //¼���ʽ��0-PS��Ĭ�ϣ���1-ES
     uint8_t    s_RecordIDueTime;      //¼�����ʱ�䣬��λ���죬0-�޹���ʱ�䣨Ĭ�ϣ�
     uint8_t    s_IsRedundancyRec;     //�Ƿ�����¼��0-��������Ĭ�ϣ���1-����
     uint8_t    s_RecStoreType;        //¼��洢��ʽ��0-SD��Ĭ�ϣ���1-USB��2-NAS
     uint8_t    s_Reserved[5];
}RecordParamConfigIn;

//¼��ƻ������������
typedef struct tagRecordScheduleConfig
{
    uint8_t   s_Enable;                //¼��ƻ��Ƿ����ñ�ǣ�0-δ���ã�1-����
    uint8_t   s_Reserved[11];        
    uint32_t  s_RecStartTime[7][4];    //¼����ʼʱ��
    uint32_t  s_RecEndTime[7][4];      //¼��ֹͣʱ��
}RecordScheduleConfigIn;

//NAS�����������
typedef struct tagNasParamConfig
{
    uint32_t   s_DiskNo;                           //���̺ţ���1��ʼ
    char_t     s_NasServerAddr[MAX_LEN_ADDR_DISK]; //NAS��ַ
    char_t     s_DiskAddr[MAX_LEN_ADDR_DISK];      //�����ļ���ַ
    uint8_t   s_Reserved[4];
}NasParamConfigIn;

//¼������������
typedef struct tagRecordCtrl
{
    uint8_t   s_RecCtrlFlag;          //¼����ƣ�0-ֹͣ¼��Ĭ�ϣ���1-��ʼ¼��
    uint8_t   s_RecTrigMode;          //¼�񴥷����ͣ�0-��ʱ¼��Ĭ�ϣ���1-�ֶ�¼��2-�ƶ����,3-����
    uint8_t   s_RecTrigChan;          //����ͨ����
    uint8_t   s_AlarmTrigNo;          //��������ͨ����
    uint8_t   s_Reserved[4]; 
}RecordCtrlIn;

//¼���ļ���ѯ�������
typedef struct tagRecordFileQuery
{
 	uint8_t   s_RecQueryType;         //¼���ļ���ѯ���ͣ�0-��ʱ�䣨Ĭ�ϣ���1-��¼�񴥷����ͣ�
                                      //2-��ʱ���¼�񴥷�����
    uint8_t   s_RecTrigMode;          //¼�񴥷����ͣ��ӵ͵���λ����1λΪ1-��ʱ¼��
                                      //��2λΪ1-�ֶ�¼�񣬵�3λΪ1-�ƶ���⣬��4λΪ1-����
    uint8_t   s_Reserved[6];          
    uint32_t  s_RecQueryTime[2];      //¼���ѯʱ��Σ�UTCʱ�䣬��λ����
}RecordFileQueryIn;

//¼���ļ���ѯ�������
typedef struct tagRecordFileQueryRes
{
    uint8_t   s_RecTrigMode;                       //¼�񴥷����ͣ��ӵ͵���λ����1λΪ1-��ʱ¼��
                                                   //��2λΪ1-�ֶ�¼�񣬵�3λΪ1-�ƶ���⣬
                                                   //��4λΪ1-����
    uint8_t   s_Reserved[3];             
    char_t    s_RecFileName[MAX_LEN_FILE_REC];     //¼���ļ�����
    uint32_t  s_RecFileSize;                       //¼���ļ���С����λ��MB
    uint32_t  s_RecFileTime[2];                    //¼���ļ���ʼʱ�䣬UTCʱ�䣬��λ����
}RecordFileQueryResOut;

//¼�����غͻطŲ�ѯ�������
typedef struct tagRecordDownReplayQuery
{
 	uint8_t   s_RecQueryType;                   //¼���ѯ���ͣ�0-���أ�1-�ط�
 	uint8_t   s_Reserved[7];          
    char_t   s_RecFileName[MAX_LEN_FILE_REC];   //¼���ļ����ƣ���ѯ��ʾ��web�ϵ��ļ����ƣ�,
                                                //��¼���ѯ����Ϊ����ʱ��
    uint32_t  s_RecReplayTime;                  //¼��ط�ʱ��㣬UTCʱ�䣬��λ���룬
                                                //��¼���ѯ����Ϊ�ط�ʱ��
}RecordDownReplayQueryIn;

typedef struct tagRecordDownReplayQueryRes
{
 	uint8_t   s_RecQueryType;                     //¼���ѯ���ͣ�0-���أ�1-�ط�
 	uint8_t   s_Reserved0[7];          
    char_t   s_RecFileName[MAX_LEN_FILE_REC];     //¼���ļ�·�������ƣ����������д洢���ļ����ƣ�
    uint32_t  s_RecFileOffset;                    //¼���ļ���ȡλ��
    uint32_t  s_RecFileSize;		              //¼���ļ���С���ط�ʱ¼���ļ���С������ǰʱ��㵽һ
                                                  //�����λ�õ�����������С
    uint16_t  s_RecFileTotalNum;                  //¼���ļ���������¼��ط�����
    uint16_t  s_RecFileReplayNum;                 //��Ҫ�طŵ�¼���ļ�������¼��ط����� 
    uint16_t  s_RecFileCurNo;                     //�طŵ�¼�����ݴ��ڵ�ǰ�ļ�����ţ���¼��ط����ã�
                                                  //��1��ʼ
    uint8_t  s_RecDiskTotalNum;                   // ������������¼��ط�����
    uint8_t  s_RecDiskCurNo;                      //��ǰ������ţ���¼��ط����ã���1��ʼ
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
input:DevStatusQueryPtr--query parameter;
output:DevStatusQueryResPtr-query result;
          DevStatusNum-query number;
return:success--return GMI_SUCCESS, 
	failed -- return ERROR CODE
---------------------------------------------------------------------*/
GMI_RESULT GMI_StorageDeviceStatusQuery(StorageStatusQueryIn *DevStatusQueryPtr,
	                                            StorageStatusQueryResOut **DevStatusQueryResPtr, uint32_t  *DevStatusNum);


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
        CurQueryPosNo--current query starting number;
output:RecordFileQueryResPtr-query result;
          QueryResTotalNum-query record total number;
          QueryResCurNum-current record number;
return:success--return GMI_SUCCESS, 
	failed -- return ERROR CODE
---------------------------------------------------------------------*/
GMI_RESULT GMI_RecordFileQuery(RecordFileQueryIn *RecordFileQueryPtr, uint32_t CurQueryPosNo, RecordFileQueryResOut **RecordFileQueryResPtr, 
	                                 uint32_t  *QueryResTotalNum, uint32_t  *QueryResCurNum);

/*===============================================================
func name:GMI_StorageVersionQuery
func:query storage version
output:StorageVer--version;
return:success--return GMI_SUCCESS, 
	failed -- return ERROR CODE
---------------------------------------------------------------------*/
GMI_RESULT  GMI_StorageVersionQuery(char_t *StorageVer);


#ifdef __cplusplus
}
#endif
#endif
