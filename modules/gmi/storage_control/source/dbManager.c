#include "dbManager.h"
#include "storage_manager.h"
#include "stdio.h"


#define		PRT_ERR(x)	printf x
#define		PRT_TEST(x)	printf x

/*打开数据库文件*/
int openDbFile(sqlite3 **dbFd)
{
	int ret = -1;
	char tmpParam[256];

	memset(tmpParam, 0, sizeof(tmpParam));
	sprintf(tmpParam, "chmod -R 777 %s/", DST_PATH_NAME_SD);
	if(0 > system(tmpParam))
	{
		PRT_ERR(("%s error.\n", tmpParam));
	}

	memset(tmpParam, 0, sizeof(tmpParam));
	sprintf(tmpParam, "%s/%s", DST_PATH_NAME_SD, DB_FILE_NAME);
	//打开指定的数据库文件,如果不存在将创建一个同名的数据库文件
	ret = sqlite3_open(tmpParam, dbFd);
	
	memset(tmpParam, 0, sizeof(tmpParam));
	sprintf(tmpParam, "chmod -R 777 %s/%s", DST_PATH_NAME_SD, DB_FILE_NAME);
	if(0 > system(tmpParam))
	{
		PRT_ERR(("%s error.\n", tmpParam));
	}
	
	if(SQLITE_OK != ret)
	{
		PRT_ERR(("Can't open database: %s\n", sqlite3_errmsg(*dbFd)));
	}
	else
	{
		PRT_TEST(("have opened a sqlite3 database VideoRecord.db\n"));
	}
	
	return ret;
}

/*打开备份数据库文件*/
int openDbBackupFile(sqlite3 **dbFd)
{
	int ret = -1;
	char tmpParam[256];

	memset(tmpParam, 0, sizeof(tmpParam));
	sprintf(tmpParam, "chmod -R 777 %s/", DST_PATH_NAME_SD);
	if(0 > system(tmpParam))
	{
		PRT_ERR(("%s error.\n", tmpParam));
	}

	memset(tmpParam, 0, sizeof(tmpParam));
	sprintf(tmpParam, "%s/%s", DST_PATH_NAME_SD, DB_FILE_BAK_NAME);
	//打开指定的数据库文件,如果不存在将创建一个同名的数据库文件
	ret = sqlite3_open(tmpParam, dbFd);
	
	memset(tmpParam, 0, sizeof(tmpParam));
	sprintf(tmpParam, "chmod -R 777 %s/%s", DST_PATH_NAME_SD, DB_FILE_BAK_NAME);
	if(0 > system(tmpParam))
	{
		PRT_ERR(("%s error.\n", tmpParam));
	}
	
	if(SQLITE_OK != ret)
	{
		PRT_ERR(("Can't open database: %s\n", sqlite3_errmsg(*dbFd)));
	}
	else
	{
		PRT_TEST(("have opened a sqlite3 database bakVideoRecord.db\n"));
	}
	
	return ret;
}



/*关闭数据库文件*/
void closeDbFile(sqlite3 **dbFd)
{
	int ret = -1;
	if(NULL != *dbFd)
	{
		ret = sqlite3_close(*dbFd);
		if(SQLITE_OK != ret)
		{
			PRT_ERR(("closeDbFile: %s\n", sqlite3_errmsg(*dbFd)));
		}
		*dbFd = NULL;
	}
}

/*创建数据表*/
int createDbTable(sqlite3 *dbFd, const int cmdType)
{
	int ret = -1;
	char tmpParam[512];

	
	if(NULL == dbFd)
	{
		PRT_ERR(("createDbTable param NULL.\n"));
		return LOCAL_RET_ERR;
	}
	
	memset(tmpParam, 0, sizeof(tmpParam));

	switch(cmdType)
	{		
		case RECORD_ADD_SEG:
			/*判断表是否存在*/
			#if 1
			
			sprintf(tmpParam, "%s%s%s","SELECT * FROM ", VIDEO_TABLE_NAME, " LIMIT 1");
			if(SQLITE_OK == sqlite3_exec(dbFd , tmpParam , 0 , 0 , 0))
			{
				PRT_TEST(("have VIDEO_TABLE_NAME.\n"));
				ret = SQLITE_OK;
				return ret;
			}
			else
			{
				PRT_TEST(("select error: %s\n.", sqlite3_errmsg(dbFd)));
			}
			memset(tmpParam, 0, sizeof(tmpParam));
			#endif
			
			
			sprintf(tmpParam,"%s%s%s", "CREATE TABLE ", VIDEO_TABLE_NAME,"(s_SId INTEGER PRIMARY KEY,s_RecFileNo INTEGER,s_RecSegNo INTEGER,s_RecLastSegLen INTEGER,s_RecSegLen SMALLINTEGER,s_RecType TINYINTEGER,s_RecEncodeNo TINYINTEGER,s_StartTime INTEGER, s_EndTime INTEGER, s_SegFileInfo INTEGER)");
			ret = sqlite3_exec(dbFd , tmpParam , 0 , 0 , 0);
			break;
		case RECORD_ADD_FILE:
			/*判断表是否存在*/
			#if 1
			sprintf(tmpParam, "%s%s%s","SELECT * FROM ", FILE_TABLE_NAME, " LIMIT 1");
			if(SQLITE_OK == sqlite3_exec(dbFd , tmpParam , 0 , 0 , 0))
			{
				PRT_TEST(("have FILE_TABLE_NAME.\n"));
				ret = SQLITE_OK;
				return ret;
			}
			else
			{
				PRT_TEST(("select error: %s\n.", sqlite3_errmsg(dbFd)));
			}
			memset(tmpParam, 0, sizeof(tmpParam));
			#endif
			sprintf(tmpParam,"%s%s%s", "CREATE TABLE ", FILE_TABLE_NAME,"(s_FId INTEGER PRIMARY KEY,s_ToRecordFiles INTEGER,s_BuildFileNums INTEGER,s_FirstRecordNo INTEGER,s_LastRecordNo INTEGER,s_CurWriteFileNo INTEGER,s_CurWriteSegNo INTEGER, s_FirstWriteTime INTEGER, s_LastWriteTime INTEGER, s_PartStatus INTEGER, s_SegRecNo INTEGER)");
			ret = sqlite3_exec(dbFd , tmpParam , 0 , 0 , 0);
			break;
		default:
			PRT_ERR(("cmdType  error.\n"));
			return LOCAL_RET_ERR;
	}
	if(SQLITE_OK != ret)
	{
		PRT_ERR(("createDbTable: %s\n", sqlite3_errmsg(dbFd)));
	}
	return ret;	
}


/*查询数据记录，支持单条记录查询、批量时间段查询、批量录像类型查询*/
void queryDbRecord(sqlite3 *dbFd, const int cmdType, const int querytype, char *param, char **queryResult, int *rowResult)
{
	FileIdxHeader fileInfoRecord;
	SegmentIdxRecord segInfoRecord;
	char tmpParam[256];
	char tmpStr1[128];					/*保存数据表名称*/
	char tmpStr2[128];					/*保存查询条件*/
	char **azResult; 					/*二维数组存放查询结果*/
	int nrow = 0, ncolumn = 0;
	int ret = -1;
	int i = 0, j = 0;

	if((NULL == param)  || (NULL == dbFd))
	{
		PRT_ERR(("queryDbRecord param NULL.\n"));
		return;
	}

	memset(&fileInfoRecord, 0, sizeof(fileInfoRecord));
	memset(&segInfoRecord, 0, sizeof(segInfoRecord));
	memset(tmpParam, 0, sizeof(tmpParam));
	memset(tmpStr1, 0, sizeof(tmpStr1));
	memset(tmpStr2, 0, sizeof(tmpStr2));
	
	/*区分数据表类型*/
	switch(cmdType)
	{
		case RECORD_QUERY_SEG:
			sprintf(tmpStr1, "%s", VIDEO_TABLE_NAME);
			memcpy(&segInfoRecord, param, sizeof(segInfoRecord));
			
			break;
		case RECORD_QUERY_FILE:
			sprintf(tmpStr1, "%s", FILE_TABLE_NAME);
			memcpy(&fileInfoRecord, param, sizeof(fileInfoRecord));
			break;
		default:
			PRT_ERR(("cmdType  error.\n"));
			return;
	}

	PRT_TEST(("cmdType=%d, querytype=%d, s_SId=%d, s_RecType=%d, s_RtartTime=%d, s_EndTime=%d\n", 
		cmdType, querytype, segInfoRecord.s_SId, segInfoRecord.s_RecType, (int)(segInfoRecord.s_StartTime), (int)(segInfoRecord.s_EndTime)));
	
	/*区分录像查询类型*/
	switch(querytype)
	{
		case METHOD_QUERY_NO:
			if(RECORD_ADD_SEG == cmdType)
			{
				sprintf(tmpStr2, "%s%d", " WHERE s_SId=", segInfoRecord.s_SId);
			}
			else if(RECORD_ADD_FILE == cmdType)
			{
				sprintf(tmpStr2, "%s%d", " WHERE s_FId=", fileInfoRecord.s_FId);
			}
			break;
		case METHOD_QUERY_TYPE:
			if(RECORD_ADD_SEG == cmdType)
			{
				sprintf(tmpStr2, "%s%d%s%d", " WHERE s_SId>=", segInfoRecord.s_SId, " AND s_RecType=", segInfoRecord.s_RecType);
			}
			else if(RECORD_ADD_FILE == cmdType)
			{
				PRT_ERR(("no support query RECORD_ADD_FILE by METHOD_QUERY_TYPE.\n"));
			}
			break;
		case METHOD_QUERY_TIME:
			if(RECORD_ADD_SEG == cmdType)
			{
				sprintf(tmpStr2, "%s%d%s%d%s%d", " WHERE s_SId>=", segInfoRecord.s_SId, " AND s_StartTime<=", (int)(segInfoRecord.s_EndTime), " AND s_EndTime>=", (int)(segInfoRecord.s_StartTime));
			}
			else if(RECORD_ADD_FILE == cmdType)
			{
				PRT_ERR(("no support query RECORD_ADD_FILE by METHOD_QUERY_TIME.\n"));
			}
			break;
		case METHOD_QUERY_MIX:
			if(RECORD_ADD_SEG == cmdType)
			{
				sprintf(tmpStr2, "%s%d%s%d%s%d%s%d", " WHERE s_SId>=", segInfoRecord.s_SId, " AND s_RecType=", segInfoRecord.s_RecType," AND s_StartTime<=", (int)(segInfoRecord.s_EndTime), " AND s_EndTime>=", (int)(segInfoRecord.s_StartTime));
			}
			else if(RECORD_ADD_FILE == cmdType)
			{
				PRT_ERR(("no support query RECORD_ADD_FILE by METHOD_QUERY_TIME.\n"));
			}
			break;
		default:
			PRT_ERR(("querytype  error.\n"));
			return;
	}

	sprintf(tmpParam, "%s%s%s%s%d", "SELECT * FROM ", tmpStr1, tmpStr2, " LIMIT ", NUM_RECORD_MAX);	
	while(1)
	{
		/*查询数据库是处于busy状态时，需要等待片刻再查询*/
		if(SQLITE_OK != (ret = sqlite3_get_table( dbFd , tmpParam , &azResult , &nrow , &ncolumn , 0)))
		{
		   PRT_TEST(("database is locked continue.\n"));
		   if(strstr(sqlite3_errmsg(dbFd), "database is locked") )
		   {
			   usleep(50000);
			   continue;
		   }
		   break;
		}
		break;
	}


	#if 0
	if((SQLITE_OK == ret) && (0 != nrow))
	{
		/*返回查询结果*/
		for( i=ncolumn ; i<( nrow + 1 ) * ncolumn ; i++ )
		{
			j = i/ncolumn - 1;
			sprintf(queryResult[j]+strlen(queryResult[j]), "%s", azResult[i] );
		}
	}
	#else
	if((SQLITE_OK == ret) && (0 != nrow))
	{
		switch(cmdType)
		{
			case RECORD_QUERY_SEG:
				/*返回查询结果*/
				for( i=ncolumn ; i<( nrow + 1 ) * ncolumn ;)
				{
					segInfoRecord.s_SId = atoi(azResult[i]);
					segInfoRecord.s_RecFileNo = atoi(azResult[i+1]);
					segInfoRecord.s_RecSegNo = atoi(azResult[i+2]);
					segInfoRecord.s_RecLastSegLen = atoi(azResult[i+3]);
					segInfoRecord.s_RecSegLen = atoi(azResult[i+4]);
					segInfoRecord.s_RecType  = atoi(azResult[i+5]);
					segInfoRecord.s_RecEncodeNo = atoi(azResult[i+6]);
					segInfoRecord.s_StartTime = atoi(azResult[i+7]);
					segInfoRecord.s_EndTime = atoi(azResult[i+8]);
					segInfoRecord.s_SegFileInfo = atoi(azResult[i+9]);
					j = i/ncolumn - 1;
					memcpy(queryResult[j], &segInfoRecord, sizeof(segInfoRecord));
					i += ncolumn;
				}
				break;
			case RECORD_QUERY_FILE:
				/*返回查询结果*/
				for( i=ncolumn ; i<( nrow + 1 ) * ncolumn ;)
				{
					fileInfoRecord.s_FId = atoi(azResult[i]);
					fileInfoRecord.s_ToRecordFiles = atoi(azResult[i+1]);
					fileInfoRecord.s_BuildFileNums = atoi(azResult[i+2]);
					fileInfoRecord.s_FirstRecordNo = atoi(azResult[i+3]);
					fileInfoRecord.s_LastRecordNo = atoi(azResult[i+4]);
					fileInfoRecord.s_CurWriteFileNo = atoi(azResult[i+5]);
					fileInfoRecord.s_CurWriteSegNo = atoi(azResult[i+6]);
					fileInfoRecord.s_FirstWriteTime = atoi(azResult[i+7]);
					fileInfoRecord.s_LastWriteTime = atoi(azResult[i+8]);
					fileInfoRecord.s_PartStatus = atoi(azResult[i+9]);
					fileInfoRecord.s_SegRecNo = atoi(azResult[i+10]);
					
					j = i/ncolumn - 1;
					memcpy(queryResult[j], &fileInfoRecord, sizeof(fileInfoRecord));
					i += ncolumn;
				}
				break;
			default:
				break;
		}
	}
	#endif
	*rowResult = nrow;
	PRT_TEST(("nrow = %d, ncolum = %d\n", nrow,ncolumn));
	if(SQLITE_OK != ret)
	{
		PRT_ERR(("queryDbRecord: %s\n", sqlite3_errmsg(dbFd)));
	}
	
	//释放掉 azResult 的内存空间
	sqlite3_free_table(azResult);
}

/*增加片段录像和录像文件信息记录(默认表尾添加)*/
int addDbRecord(sqlite3 *dbFd, const int cmdType, char *param)
{
	int strLen = 0;
	FileIdxHeader fileInfoRecord;
	SegmentIdxRecord segInfoRecord;
	char tmpParam[256];
	int ret = -1;

	memset(tmpParam, 0, sizeof(tmpParam));
	if((NULL == param)  || (NULL == dbFd))
	{
		PRT_ERR(("addDbRecord param NULL.\n"));
		return LOCAL_RET_ERR;
	}
	
	switch(cmdType)
	{
		case RECORD_ADD_SEG:
			memcpy(&segInfoRecord, param, sizeof(segInfoRecord));		
			strLen = sprintf(tmpParam, "%s%s%s", "INSERT INTO ", VIDEO_TABLE_NAME, " VALUES(");
			sprintf(tmpParam+strLen, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", segInfoRecord.s_SId, segInfoRecord.s_RecFileNo, 
				segInfoRecord.s_RecSegNo, segInfoRecord.s_RecLastSegLen, segInfoRecord.s_RecSegLen, segInfoRecord.s_RecType, 
				segInfoRecord.s_RecEncodeNo, (int)(segInfoRecord.s_StartTime), (int)(segInfoRecord.s_EndTime), (int)(segInfoRecord.s_SegFileInfo),")");
			PRT_TEST(("%s\n", tmpParam));
			ret = sqlite3_exec(dbFd, tmpParam , 0 , 0 , 0);
			break;
		case RECORD_ADD_FILE:
			memcpy(&fileInfoRecord, param, sizeof(fileInfoRecord));	
			strLen = sprintf(tmpParam, "%s%s%s", "INSERT INTO ", FILE_TABLE_NAME, " VALUES(");
			sprintf(tmpParam+strLen, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", fileInfoRecord.s_FId, fileInfoRecord.s_ToRecordFiles,
				fileInfoRecord.s_BuildFileNums, fileInfoRecord.s_FirstRecordNo, fileInfoRecord.s_LastRecordNo, 
				fileInfoRecord.s_CurWriteFileNo, fileInfoRecord.s_CurWriteSegNo, (int)(fileInfoRecord.s_FirstWriteTime),
				(int)(fileInfoRecord.s_LastWriteTime), fileInfoRecord.s_PartStatus, fileInfoRecord.s_SegRecNo,")");
			PRT_TEST(("%s\n", tmpParam));
			ret = sqlite3_exec(dbFd, tmpParam , 0 , 0 , 0);
			break;
		default:
			PRT_ERR(("cmdType  error.\n"));
			return LOCAL_RET_ERR;
	}
	if(SQLITE_OK != ret)
	{
		PRT_ERR(("addDbRecord: %s\n", sqlite3_errmsg(dbFd)));
	}
	return ret;
}

/*更新录像文件信息记录*/
int updateDbRecord(sqlite3 *dbFd, const int cmdType, const int recNo, char *param)
{
	FileIdxHeader fileInfoRecord;
	SegmentIdxRecord segInfoRecord;
	char tmpParam[512];
	int ret = -1;

	memset(tmpParam, 0, sizeof(tmpParam));
	if((NULL == param) || (NULL == dbFd))
	{
		PRT_ERR(("updateDbRecord param NULL.\n"));
		return LOCAL_RET_ERR;
	}
	
	switch(cmdType)
	{
		case RECORD_UPDATE_SEG:
			memcpy(&segInfoRecord, param, sizeof(segInfoRecord));		
			sprintf(tmpParam, "%s%s%s", "UPDATE ", VIDEO_TABLE_NAME, " SET ");
			sprintf(tmpParam+strlen(tmpParam), "s_RecFileNo=%d,s_RecSegNo=%d,s_RecLastSegLen=%d,s_RecSegLen=%d,s_RecType=%d,s_RecEncodeNo=%d,s_StartTime=%d,s_EndTime=%d", 
				segInfoRecord.s_RecFileNo, segInfoRecord.s_RecSegNo, segInfoRecord.s_RecLastSegLen, segInfoRecord.s_RecSegLen, segInfoRecord.s_RecType, 
				segInfoRecord.s_RecEncodeNo, (int)(segInfoRecord.s_StartTime), (int)(segInfoRecord.s_EndTime), (int)(segInfoRecord.s_SegFileInfo));
			sprintf(tmpParam+strlen(tmpParam), "%s%d", " WHERE s_SId=", segInfoRecord.s_SId);	
			PRT_TEST(("%s\n", tmpParam));
			ret = sqlite3_exec(dbFd, tmpParam , 0 , 0 , 0);
			break;
		case RECORD_UPDATE_FILE:
			memcpy(&fileInfoRecord, param, sizeof(fileInfoRecord));	
			sprintf(tmpParam, "%s%s%s", "UPDATE ", FILE_TABLE_NAME, " SET ");
			sprintf(tmpParam+strlen(tmpParam), "s_ToRecordFiles=%d,s_BuildFileNums=%d,s_FirstRecordNo=%d,s_LastRecordNo=%d,s_CurWriteFileNo=%d,s_CurWriteSegNo=%d,s_FirstWriteTime=%d,s_LastWriteTime=%d,s_PartStatus=%d,s_SegRecNo=%d", 
				fileInfoRecord.s_ToRecordFiles, fileInfoRecord.s_BuildFileNums, fileInfoRecord.s_FirstRecordNo, 
				fileInfoRecord.s_LastRecordNo, fileInfoRecord.s_CurWriteFileNo, fileInfoRecord.s_CurWriteSegNo, 
				(int)(fileInfoRecord.s_FirstWriteTime), (int)(fileInfoRecord.s_LastWriteTime), fileInfoRecord.s_PartStatus, fileInfoRecord.s_SegRecNo);
			sprintf(tmpParam+strlen(tmpParam), "%s%d", " WHERE s_FId=", fileInfoRecord.s_FId);
			PRT_TEST(("%s\n", tmpParam));
			ret = sqlite3_exec(dbFd, tmpParam , 0 , 0 , 0);
			break;
		default:
			PRT_ERR(("cmdType  error.\n"));
			return LOCAL_RET_ERR;
	}
	if(SQLITE_OK != ret)
	{
		//PRT_ERR(("updateDbRecord: %s\n", sqlite3_errmsg(dbFd)));
		PRT_ERR(("updateDbRecord: error.\n"));
	}
	return ret;
}

/*删除片段录像记录(默认从头开始删除)*/
int deleteDbRecord(sqlite3 *dbFd, const int startNo, const int recNum)
{
	int ret = -1;
	int i = 0;
	int strLen = 0;
	char tmpParam[256];

	if(NULL == dbFd)
	{
		PRT_ERR(("deleteDbRecord param NULL.\n"));
		return LOCAL_RET_ERR;
	}

	memset(tmpParam, 0, sizeof(tmpParam));
	strLen = sprintf(tmpParam, "%s%s%s", "DELETE FROM ", VIDEO_TABLE_NAME," WHERE s_SId=");
	for(i = 0; i < recNum; i++)
	{
		sprintf(tmpParam+strLen, "%d", startNo+i);
		PRT_TEST(("%s\n", tmpParam));
		ret = sqlite3_exec(dbFd, tmpParam , 0 , 0 , 0);
		if(SQLITE_OK != ret)
		{
			PRT_ERR(("deleteDbRecord(%d): %s\n", startNo+i, sqlite3_errmsg(dbFd)));
			break;
		}
		
	}
	return ret;
}

void selectTableTail(sqlite3 *dbFd, int cmdType, int *id)
{
	char tmpParam[256];
 
	 if(NULL == dbFd)
	 {
		 PRT_ERR(("selectTableTail param NULL.\n"));
		 return;
	 } 

	memset(tmpParam, 0, sizeof(tmpParam));
	if(0 == cmdType)
	{
		sprintf(tmpParam, "%s", "SELECT * FROM recordVideoTable ORDER BY s_SId DESC LIMIT 1");
	}
	else
	{
		sprintf(tmpParam, "%s", "SELECT * FROM recordVideoTable ORDER BY s_SId LIMIT 1");
	}
	char **azResult; //二维数组存放结果
	int nrow = 0, ncolumn = 0;
	//查询数据

	sqlite3_get_table( dbFd , tmpParam , &azResult , &nrow , &ncolumn , 0);
	*id = atoi(azResult[ncolumn]);
	printf("id = %d azResult[%d] = %s nrow = %d ncolumn = %d\n", *id, ncolumn, azResult[ncolumn], nrow, ncolumn);
	//释放掉 azResult 的内存空间
	sqlite3_free_table(azResult);
}

/*备份主数据库文件数据到备用数据库文件*/
int backupMainDb(sqlite3 *pDb)
{
	int rc = LOCAL_RET_ERR;
	sqlite3 *pFile = NULL;
	sqlite3_backup *pBackup;
	struct timeval startTime, endTime;
	gettimeofday (&startTime , 0);

	if((NULL == pDb) || (SQLITE_IOERR == sqlite3_errcode(pDb)))
	{
		PRT_ERR(("backupMainDb main database param error.\n"));
		return rc;
	}
	
	rc = openDbBackupFile(&pFile);
	if((NULL != pFile) && (SQLITE_OK == rc))
	{
		pBackup = sqlite3_backup_init(pFile, "main", pDb, "main");
		if(pBackup)
		{
			do
			{
				rc = sqlite3_backup_step(pBackup, 50);
				if( rc==SQLITE_OK || rc==SQLITE_BUSY || rc==SQLITE_LOCKED)
				{
					sqlite3_sleep(250);
				}
			}while( rc==SQLITE_OK || rc==SQLITE_BUSY || rc==SQLITE_LOCKED );
			(void)sqlite3_backup_finish(pBackup);
		}
		else
		{
			//rc = sqlite3_errcode(pFile);
			PRT_ERR(("sqlite3_backup_init %s error.\n", sqlite3_errmsg(pFile)));
		}
	}
	else
	{
		//rc = sqlite3_errcode(pFile);
		PRT_ERR(("openDbBackupFile pFile NULL or rc %s error.\n", sqlite3_errmsg(pFile)));
	}
	closeDbFile(&pFile);
	
	gettimeofday (&endTime , 0);
	PRT_TEST(("need %d ms backupMainDb end...\n", (int)((endTime.tv_sec-startTime.tv_sec)*1000+(endTime.tv_usec-startTime.tv_usec)/1000)));
	return rc;
}

/*从备份数据库文件中恢复数据到主数据库文件*/
int recoverMainDb(sqlite3 **pDb)
{
	char tmpParam[256];
	int iFd = -1;

	memset(tmpParam, 0, sizeof(tmpParam));
	sprintf(tmpParam, "%s/%s", DST_PATH_NAME_SD, DB_FILE_BAK_NAME);

	/*先判断有无备份数据库文件*/
    iFd = open(tmpParam, O_RDONLY, 0);
	if (-1 == iFd)
	{
	    PRT_ERR(("have no %s.\n", tmpParam));
		return LOCAL_RET_ERR;
	}
	close(iFd);

	//(void)sqlite3_close(*pDb);
	closeDbFile(pDb);
	*pDb = NULL;

	memset(tmpParam, 0, sizeof(tmpParam));
	/*删除主数据库文件*/
	sprintf(tmpParam, "%s %s/%s", "rm -f ", DST_PATH_NAME_SD, DB_FILE_NAME);
	if(0 > system(tmpParam))
	{
		PRT_ERR(("%s error.\n", tmpParam));
	}
	sync();

	/*将备份数据库更名为主数据库文件*/
	memset(tmpParam, 0, sizeof(tmpParam));
	sprintf(tmpParam, "%s %s/%s %s/%s", "mv ", DST_PATH_NAME_SD, DB_FILE_BAK_NAME, DST_PATH_NAME_SD, DB_FILE_NAME);
	if(0 > system(tmpParam))
	{
		PRT_ERR(("%s error.\n", tmpParam));
	}
	sync();
	
	openDbFile(pDb);

	if(NULL == *pDb)
	{
		return LOCAL_RET_ERR;
	}
	
	return LOCAL_RET_OK;
}


