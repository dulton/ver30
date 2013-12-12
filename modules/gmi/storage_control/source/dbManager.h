#ifndef _DB_MANAGER_H_
#define _DB_MANAGER_H_

#include "sqlite3.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*��¼��ѯ��ʽ*/
#define		METHOD_QUERY_NO			0			/*¼���¼�Ų�ѯ*/
#define		METHOD_QUERY_TIME		1			/*¼��ʱ���ѯ*/
#define		METHOD_QUERY_TYPE		2			/*¼�����Ͳ�ѯ*/
#define		METHOD_QUERY_MIX		3			/*¼������+ʱ���ѯ*/


#define		RECORD_QUERY_SEG		0			/*Ƭ�μ�¼��ѯ*/
#define		RECORD_QUERY_FILE		1			/*�ļ���¼��ѯ*/

#define		FILE_QUERY_DB			0			/*�����ݿ��ļ���ѯ*/
#define		FILE_QUERY_DBBAK		1			/*�������ݿ��ļ���ѯ*/


/*��¼���ӷ�ʽ*/
#define		RECORD_ADD_SEG			0			/*Ƭ�μ�¼����*/
#define		RECORD_ADD_FILE			1			/*�ļ���¼����*/

/*��¼���·�ʽ*/
#define		RECORD_UPDATE_SEG		0			/*Ƭ�μ�¼����*/
#define		RECORD_UPDATE_FILE		1			/*�ļ���¼����*/

/*��ѯ��¼����*/
#define		NUM_RECORD_MAX			50			/*һ���Բ�ѯ��¼�����*/			

/*�����ݿ��ļ�*/
int openDbFile(sqlite3 **dbFd);

/*�ر����ݿ��ļ�*/
void closeDbFile(sqlite3 **dbFd);

/*�������ݱ�*/
int createDbTable(sqlite3 *dbFd, const int cmdType);

/*�򿪱������ݿ��ļ�*/
int openDbBackupFile(sqlite3 **dbFd);

/*��ѯ���ݼ�¼��֧�ֵ�����¼��ѯ��ʱ��β�ѯ��¼�����Ͳ�ѯ*/
void queryDbRecord(sqlite3 *dbFd, const int cmdType, const int querytype, char *param, char **queryResult, int *rowResult);

/*����Ƭ��¼���¼���ļ���Ϣ��¼*/
int addDbRecord(sqlite3 *dbFd, const int cmdType, char *param);

/*����¼���ļ���Ϣ��¼*/
int updateDbRecord(sqlite3 *dbFd, const int cmdType, const int recNo, char *param);

/*ɾ��Ƭ��¼���¼*/
int deleteDbRecord(sqlite3 *dbFd, const int startNo, int recNum);

/*��ѯ �������һ����¼ID*/
void selectTableTail(sqlite3 *dbFd, int cmdType, int *id);

/*���������ݿ��ļ����ݵ��������ݿ��ļ�*/
int backupMainDb(sqlite3 *pDb);

/*�ӱ������ݿ��ļ��лָ����ݵ������ݿ��ļ�*/
int recoverMainDb(sqlite3 **pDb);
#ifdef __cplusplus
}
#endif

#endif
