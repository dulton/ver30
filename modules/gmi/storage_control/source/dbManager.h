#ifndef _DB_MANAGER_H_
#define _DB_MANAGER_H_

#include "sqlite3.h"

/*记录查询方式*/
#define		METHOD_QUERY_NO			0			/*录像记录号查询*/
#define		METHOD_QUERY_TIME		1			/*录像时间查询*/
#define		METHOD_QUERY_TYPE		2			/*录像类型查询*/
#define		METHOD_QUERY_MIX		3			/*录像类型+时间查询*/


#define		RECORD_QUERY_SEG		0			/*片段记录查询*/
#define		RECORD_QUERY_FILE		1			/*文件记录查询*/

#define		FILE_QUERY_DB			0			/*主数据库文件查询*/
#define		FILE_QUERY_DBBAK		1			/*备份数据库文件查询*/


/*记录增加方式*/
#define		RECORD_ADD_SEG			0			/*片段记录增加*/
#define		RECORD_ADD_FILE			1			/*文件记录增加*/

/*记录更新方式*/
#define		RECORD_UPDATE_SEG		0			/*片段记录更新*/
#define		RECORD_UPDATE_FILE		1			/*文件记录更新*/

/*查询记录条数*/
#define		NUM_RECORD_MAX			50			/*一次性查询记录最大数*/			

/*打开数据库文件*/
int openDbFile(sqlite3 **dbFd);

/*关闭数据库文件*/
void closeDbFile(sqlite3 **dbFd);

/*创建数据表*/
int createDbTable(sqlite3 *dbFd, const int cmdType);

/*打开备份数据库文件*/
int openDbBackupFile(sqlite3 **dbFd);

/*查询数据记录，支持单条记录查询、时间段查询、录像类型查询*/
void queryDbRecord(sqlite3 *dbFd, const int cmdType, const int querytype, char *param, char **queryResult, int *rowResult);

/*增加片段录像和录像文件信息记录*/
int addDbRecord(sqlite3 *dbFd, const int cmdType, char *param);

/*更新录像文件信息记录*/
int updateDbRecord(sqlite3 *dbFd, const int cmdType, const int recNo, char *param);

/*删除片段录像记录*/
int deleteDbRecord(sqlite3 *dbFd, const int startNo, int recNum);

/*查询 表中最后一条记录ID*/
void selectTableTail(sqlite3 *dbFd, int cmdType, int *id);

/*备份主数据库文件数据到备用数据库文件*/
int backupMainDb(sqlite3 *pDb);

/*从备份数据库文件中恢复数据到主数据库文件*/
int recoverMainDb(sqlite3 **pDb);

#endif
