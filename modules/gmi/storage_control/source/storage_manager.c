#include <sys/mount.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <dirent.h>

#include "storage_manager.h"
#include "log_record.h"
#include "dbManager.h"

GlobalOperateInfo       g_HdPart;					/*全局录像文件信息*/
pthread_mutex_t			g_LockHDPart;				/*全局硬盘操作锁*/	
sqlite3					*g_DbFd = NULL;				/*数据库文件描述符*/

#define		PRT_ERR(x)	printf x
#define		PRT_TEST(x)	printf x

int32_t UmountHdisk(char_t *InDstPathName)
{
	if(NULL == InDstPathName)
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");		
		return LOCAL_RET_ERR;
	}
    return umount(InDstPathName);
}

int32_t MountHdisk(char_t *InSrcPathName, char_t *InDstPathName, char_t *InType)
{
	if((NULL == InSrcPathName)
		|| (NULL == InDstPathName)
		|| (NULL == InType))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");				
		return LOCAL_RET_ERR;
	}
    return mount(InSrcPathName, InSrcPathName, InType, 32768, NULL);
}

int32_t IsNeedNewDatabase()
{
	char_t TmpParam[256];
	int32_t Fd = -1;
	int Result0 = 0, Result1 = 0;

	memset(TmpParam, 0, sizeof(TmpParam));
	sprintf(TmpParam, "%s/%s", DST_PATH_NAME_SD, DB_FILE_NAME);

	/*先判断有无数据库文件*/
	Fd = open(TmpParam, O_RDONLY, 0);
	if (-1 == Fd)
	{
		PRT_ERR(("have no %s.\n", TmpParam));
		Result0 = 1;
	}
	close(Fd);
	Fd = -1;

	memset(TmpParam, 0, sizeof(TmpParam));
	sprintf(TmpParam, "%s/%s", DST_PATH_NAME_SD, DB_FILE_BAK_NAME);

	/*先判断有无备份数据库文件*/
	Fd = open(TmpParam, O_RDONLY, 0);
	if (-1 == Fd)
	{
		PRT_ERR(("have no %s.\n", TmpParam));
		Result1 = 1;
	}
	close(Fd);

	/*需要新建数据库文件*/
	if(2 == (Result0+Result1))
	{
		return LOCAL_RET_OK;
	}
	else
	{
		return LOCAL_RET_ERR;
	}
}

void_t InitHdPart()
{
	g_DbFd = NULL;
	memset(&g_HdPart, 0, sizeof(g_HdPart));
	
}

int32_t InitPartion(int32_t FileSize)
{
	int32_t Fd = -1;
	FileIdxHeader PartInfo;
	SegmentIdxRecord SegPartInfo;
	char_t *QueryResult = NULL;
	int32_t RetVal = LOCAL_RET_OK;
	int32_t RowRes = 0;
	int32_t IsNeedRecoverDB = FALSE;
	struct statfs	DiskStat;
	int32_t    RecordFiles = 0;
	uint32_t  DiskSpace = 0;
	char_t   StrFileName[64];
	FileIdxHeader FileParam;

	memset(StrFileName, 0, sizeof(StrFileName));
	memset(&FileParam, 0, sizeof(FileParam));
	/*先判断有无SD卡*/
    Fd = open(SRC_PATH_NAME_SD, O_RDONLY, 0);
	if (Fd == -1) {
	    PRT_ERR(("have no sdisk.\n"));
		goto errExit;
	}
	close(Fd);

	/*判断有无目标文件夹,否则创建目标文件夹*/
	if(opendir(DST_PATH_NAME_SD) == NULL)
	{
		PRT_ERR(("[%s] have no %s, so create\n", __func__, DST_PATH_NAME_SD));
		if(mkdir(DST_PATH_NAME_SD, 0777) == -1)
		{
			PRT_ERR(("[%s] create %s failed\n", __func__, DST_PATH_NAME_SD));
			RetVal = LOCAL_RET_ERR;
			goto errExit;
		}
	}

	sleep(1);
	RetVal = MountHdisk(SRC_PATH_NAME_SD, DST_PATH_NAME_SD, "vfat");
	if (RetVal != LOCAL_RET_OK) {
	    PRT_ERR(("mount failed. errno = %s\n", strerror(errno)));
		/*挂载失败后，判断是否已经被挂载，对于已经挂载的造成的错误无需结束流程*/
		if(EBUSY != errno)
		{
			RetVal = LOCAL_RET_ERR;
			goto errExit;
		}
	}

	/*初始化全局变量*/
	InitHdPart();

	memset(&PartInfo, 0, sizeof(PartInfo));
	PartInfo.s_FId = 0;
	QueryResult = (char_t*)malloc(sizeof(FileIdxHeader)+sizeof(char_t));
	if(NULL == QueryResult)
	{
		PRT_ERR(("queryResult malloc error, please reboot.\n"));
		RetVal = LOCAL_RET_ERR;
		goto errExit;
	}
	memset(QueryResult, 0, sizeof(FileIdxHeader)+sizeof(char));

	/*判断是否是外部格式化好的磁盘，对于这种情况先建立数据库文件和第一个录像大文件*/
	if(LOCAL_RET_OK == IsNeedNewDatabase())
	{
		/*得到sd卡的容量*/
	   if (statfs(DST_PATH_NAME_SD, &DiskStat) < 0) 
	   {
		   PRT_ERR(("get sd card space error. errno = %s\n", strerror(errno)));
		   RetVal = LOCAL_RET_ERR;
		   goto errExit;
	   }
	   /*将sd卡容量换算成以MB为单位*/
	   DiskSpace = ((DiskStat.f_bsize >> 10) * DiskStat.f_bavail) >> 10;
	   PRT_TEST(("DiskSpace  = %d M\n", DiskSpace));
	   if(DiskSpace < 150)
	   {
	   		PRT_ERR(("DiskSpace is too small.\n"));
			RetVal = LOCAL_RET_ERR;
			goto errExit;
	   }
	   /*计算录像文件数，并为索引文件(数据库文件+备份文件)预留空间*/
	   if(FileSize > 32)
	   {
	       RecordFiles = (DiskSpace * 997 / 1000 - 120) / FileSize; 
	   }
	   else
	   {
	       RecordFiles = (DiskSpace * 997 / 1000 - 120) / (STREAM_FILE_LEN>>20);
	   }

	   /*创建第一录像文件*/
	    memset(StrFileName, 0, sizeof(StrFileName));
	    sprintf(StrFileName, "%s/record%04d%s", DST_PATH_NAME_SD, 0, AV_FILE_NAME);
		if (0 > (Fd = open(StrFileName, O_CREAT|O_RDWR, DEFAULT_FILE_MODE)))
		{
		    PRT_ERR(("can not create file %s\n", StrFileName));
			goto errExit;
		}
		close(Fd);

		openDbFile(&g_DbFd);
		if(NULL == g_DbFd)
		{
			PRT_ERR(("openDbFile error.\n"));
			RetVal = LOCAL_RET_ERR;
			goto errExit;
		}
		
		if(SQLITE_OK != createDbTable(g_DbFd, RECORD_ADD_SEG))
		{
			PRT_ERR(("createDbTable error.\n"));
			RetVal = LOCAL_RET_ERR;
			closeDbFile(&g_DbFd);
			goto errExit;
		}

		if(SQLITE_OK != createDbTable(g_DbFd, RECORD_ADD_FILE))
		{
			PRT_ERR(("createDbTable error.\n"));
			RetVal = LOCAL_RET_ERR;
			closeDbFile(&g_DbFd);
			goto errExit;
		}
	   
		/*初始化文件信息记录(仅一条)*/
		memset(&FileParam, 0, sizeof(FileParam));
		FileParam.s_ToRecordFiles = RecordFiles;
		FileParam.s_PartStatus = HD_NORMAL;
		FileParam.s_BuildFileNums = 1;
		if(SQLITE_OK != addDbRecord(g_DbFd, RECORD_ADD_FILE, (char_t*)&FileParam))
		{
			PRT_ERR(("RECORD_ADD_FILE error.\n"));
		}
	}
	else
	{
		openDbFile(&g_DbFd);
		if(NULL == g_DbFd)
		{
		    PRT_ERR(("open database failed.\n"));
			RetVal = LOCAL_RET_ERR;
			closeDbFile(&g_DbFd);
			/*主数据库文件异常时尝试恢复*/
			if(LOCAL_RET_OK != recoverMainDb(&g_DbFd))
			{
				PRT_ERR(("open main database error to recover main database failed, please try again after reboot or format disk.\n"));
				goto errExit;
			}
		}
	}
	
	if(NULL != g_DbFd)
	{
		queryDbRecord(g_DbFd, RECORD_QUERY_FILE, METHOD_QUERY_NO, (char_t*)&PartInfo, &QueryResult, &RowRes);
		sleep(1);		
	}

	if(NULL != QueryResult)
	{
		/*判断主数据库文件是否有问题*/
		/*文件记录表中查不到关于录像片段记录的信息*/
		if(RowRes <= 0)
		{
			IsNeedRecoverDB = TRUE;
		}
		else
		{
			memcpy(&g_HdPart, QueryResult, sizeof(FileIdxHeader));
			if(0 == g_HdPart.s_SegRecNo)
			{
				PRT_ERR(("this is using database firstly after format.\n"));
			}
			else
			{
				memset(&SegPartInfo, 0, sizeof(SegPartInfo));
				RowRes = 0;
				SegPartInfo.s_SId = g_HdPart.s_FirstRecordNo;
				queryDbRecord(g_DbFd, RECORD_QUERY_SEG, METHOD_QUERY_NO, (char_t*)&SegPartInfo, &QueryResult, &RowRes);
				/*片段记录中查不到首条记录*/
				if(RowRes <= 0)
				{
					IsNeedRecoverDB = TRUE;
				}
				else
				{
					RowRes = 0;
					SegPartInfo.s_SId = g_HdPart.s_LastRecordNo;
					queryDbRecord(g_DbFd, RECORD_QUERY_SEG, METHOD_QUERY_NO, (char_t*)&SegPartInfo, &QueryResult, &RowRes);
					/*片段记录中查不到最后一条记录*/
					if(RowRes <= 0)
					{
						IsNeedRecoverDB = TRUE;
					}
				}
			}
			
		}

		if(TRUE == IsNeedRecoverDB)
		{
			IsNeedRecoverDB = FALSE;
			PRT_TEST(("main database data error, please using backup database.\n"));
			if(LOCAL_RET_OK != recoverMainDb(&g_DbFd))
			{
				PRT_TEST(("recover main database failed, please try again after reboot or format disk.\n"));
				RetVal = LOCAL_RET_ERR;
				closeDbFile(&g_DbFd);
				goto errExit;
			}
			
			memset(&PartInfo, 0, sizeof(PartInfo));
			memset(QueryResult, 0, sizeof(FileIdxHeader)+sizeof(char));
			RowRes = 0;
			/*主数据库文件恢复后重新查找文件记录信息*/
			queryDbRecord(g_DbFd, RECORD_QUERY_FILE, METHOD_QUERY_NO, (char_t*)&PartInfo, &QueryResult, &RowRes);
			if(RowRes <= 0)
			{
				PRT_TEST(("recover main database error, please try again after reboot or format disk.\n"));
				RetVal = LOCAL_RET_ERR;
				closeDbFile(&g_DbFd);
				goto errExit;
			}
		
			memset(&g_HdPart, 0, sizeof(g_HdPart));	
			PRT_TEST(("%s\n", QueryResult));	
			/*获取当前分区文件状态*/
			memcpy(&g_HdPart, QueryResult, sizeof(FileIdxHeader));

			memset(&SegPartInfo, 0, sizeof(SegPartInfo));
			RowRes = 0;
			SegPartInfo.s_SId = g_HdPart.s_FirstRecordNo;
			queryDbRecord(g_DbFd, RECORD_QUERY_SEG, METHOD_QUERY_NO, (char_t*)&SegPartInfo, &QueryResult, &RowRes);
			/*片段记录中查不到首条记录*/
			if(RowRes <= 0)
			{
				PRT_ERR(("queryDbRecord backup-main database error, please try again after reboot or format disk.\n"));
				RetVal = LOCAL_RET_ERR;
				closeDbFile(&g_DbFd);
				goto errExit;
			}
			else
			{
				RowRes = 0;
				SegPartInfo.s_SId = g_HdPart.s_LastRecordNo;
				queryDbRecord(g_DbFd, RECORD_QUERY_SEG, METHOD_QUERY_NO, (char_t*)&SegPartInfo, &QueryResult, &RowRes);
				/*片段记录中查不到最后一条记录*/
				if(RowRes <= 0)
				{
					PRT_ERR(("queryDbRecord backup-main database error, please try again after reboot or format disk.\n"));
					RetVal = LOCAL_RET_ERR;
					closeDbFile(&g_DbFd);
					goto errExit;
				}
			}
		}
		
		g_HdPart.s_IsLinkDB = 1;
		g_HdPart.s_SegRecNo = g_HdPart.s_LastRecordNo+1;	/*保证当前片段录像编号是最新的(还未被使用)*/
		/*********/
		PRT_TEST(("s_fId = %d\n", g_HdPart.s_FId));
		PRT_TEST(("s_toRecordFiles = %d\n", g_HdPart.s_ToRecordFiles));
		PRT_TEST(("s_buildFileNums = %d\n", g_HdPart.s_BuildFileNums));
		PRT_TEST(("s_firstRecordNo = %d\n", g_HdPart.s_FirstRecordNo));
		PRT_TEST(("s_lastRecordNo = %d\n", g_HdPart.s_LastRecordNo));
		PRT_TEST(("s_curWriteFileNo = %d\n", g_HdPart.s_CurWriteFileNo));
		PRT_TEST(("s_curWriteSegNo = %d\n", g_HdPart.s_CurWriteSegNo));
		PRT_TEST(("s_firstWriteTime = %d\n", g_HdPart.s_FirstWriteTime));
		PRT_TEST(("s_lastWriteTime = %d\n", g_HdPart.s_LastWriteTime));
		PRT_TEST(("s_partStatus = %d\n", g_HdPart.s_PartStatus));
		PRT_TEST(("s_segRecNo = %d\n", g_HdPart.s_SegRecNo));
		PRT_TEST(("s_isLinkDB = %d\n", g_HdPart.s_IsLinkDB));
		/*********/
	}
	else
	{
		PRT_ERR((" no infomation of part, please format disk.\n"));
		RetVal = LOCAL_RET_ERR;
		closeDbFile(&g_DbFd);
		goto errExit;
	}

	free(QueryResult);
	QueryResult = NULL;
	RetVal = LOCAL_RET_OK;
errExit:
	if(NULL != QueryResult)
	{
		free(QueryResult);
		QueryResult = NULL;	
	}
	return RetVal;

}

int32_t Sdformat(int32_t FileSize)
{
	int32_t RetVal = LOCAL_RET_OK;
	int32_t Result  = LOCAL_RET_OK;
	char_t  StrFileName[64];
	char_t  MkfsCmd[64];
	FileIdxHeader FileParam;
	struct statfs	DiskStat;
	int32_t    RecordFiles     = 0;
	uint32_t DiskSpace;
	int32_t Fd = -1;

	closeDbFile(&g_DbFd);

	CREATE_LOCK(&g_LockHDPart);
    LOCK(&g_LockHDPart);
    InitHdPart();
	UNLOCK(&g_LockHDPart);

	/*删除数据库文件*/
	sprintf(MkfsCmd, "%s %s%s", "rm -f ", DST_PATH_NAME_SD, "/*");
	if(0 < system(MkfsCmd))
	{
		PRT_ERR(("%s error.\n", MkfsCmd));
	}

	sprintf(MkfsCmd, "%s %s", "mkfs.vfat", SRC_PATH_NAME_SD);
    if(0 < system(MkfsCmd))
	{
		PRT_ERR(("%s error.\n", MkfsCmd));
	}
	
    sleep(1);
	/*挂载设备*/
	RetVal = MountHdisk(SRC_PATH_NAME_SD, DST_PATH_NAME_SD, "vfat");
	if (RetVal != LOCAL_RET_OK) {
	    PRT_ERR(("mount failed. errno = %s\n",strerror(errno)));
		/*挂载失败后，判断是否已经被挂载，对于已经挂载的造成的错误无需结束流程*/
		if(EBUSY != errno)
		{
			Result = LOCAL_RET_ERR;
			goto errExit;
		}
	}
	sleep(1);
	
	LOCK(&g_LockHDPart);
	/*新建数据库文件*/
	openDbFile(&g_DbFd);
	if(NULL != g_DbFd)
	{
		if(SQLITE_OK != createDbTable(g_DbFd, RECORD_ADD_SEG))
		{
			PRT_ERR(("createDbTable error.\n"));
			Result = LOCAL_RET_ERR;
			UNLOCK(&g_LockHDPart);
			closeDbFile(&g_DbFd);
			goto errExit;
		}

		if(SQLITE_OK != createDbTable(g_DbFd, RECORD_ADD_FILE))
		{
			PRT_ERR(("createDbTable error.\n"));
			Result = LOCAL_RET_ERR;
			UNLOCK(&g_LockHDPart);
			closeDbFile(&g_DbFd);
			goto errExit;
		}
		
	}
	else
	{
	    PRT_ERR(("create database failed.\n"));
		Result = LOCAL_RET_ERR;
		UNLOCK(&g_LockHDPart);
		closeDbFile(&g_DbFd);
		goto errExit;
	}
	sync();
	sleep(1);
	
	/*得到sd卡的容量*/
   if (statfs(DST_PATH_NAME_SD, &DiskStat) < 0) 
   {
	   PRT_ERR(("get sd card space error. errno = %s\n", strerror(errno))); 
	   Result = LOCAL_RET_ERR;
	   UNLOCK(&g_LockHDPart);
	   closeDbFile(&g_DbFd);
	   goto errExit;
   }
   /*将sd卡容量换算成以MB为单位*/
   DiskSpace = ((DiskStat.f_bsize >> 10) * DiskStat.f_bavail) >> 10;
   PRT_TEST(("DiskSpace  = %d MB[%d:%d:%d:%d]\n", DiskSpace, DiskStat.f_bsize, DiskStat.f_bavail, DiskStat.f_blocks, DiskStat.f_bfree));
   if(DiskSpace < 150)
   {
   		PRT_ERR(("DiskSpace is too small.\n"));
		Result = LOCAL_RET_ERR;
		UNLOCK(&g_LockHDPart);
		closeDbFile(&g_DbFd);
		goto errExit;
   }

   /*计算录像文件数，并为索引文件(数据库文件+备份文件)预留空间*/
   if(FileSize > 32)
   {
   	   RecordFiles = (DiskSpace * 997 / 1000 - 120) / FileSize;
   }
   else
   {
   	   RecordFiles = (DiskSpace * 997 / 1000 - 120) / (STREAM_FILE_LEN>>20);
   }

   /*创建第一录像文件*/
    memset(StrFileName, 0, sizeof(StrFileName));
    sprintf(StrFileName, "%s/record%04d%s", DST_PATH_NAME_SD, 0, AV_FILE_NAME);
	if (0 > (Fd = open(StrFileName, O_CREAT|O_RDWR, DEFAULT_FILE_MODE)))
	{
	    PRT_ERR(("can not create file %s\n", StrFileName));
		UNLOCK(&g_LockHDPart);
		goto errExit;
	}
	close(Fd);
   
	/*初始化文件信息记录(仅一条)*/
	memset(&FileParam, 0, sizeof(FileParam));
	FileParam.s_ToRecordFiles = RecordFiles;
	FileParam.s_PartStatus = HD_NORMAL;
	FileParam.s_BuildFileNums = 1;
	if(SQLITE_OK != addDbRecord(g_DbFd, RECORD_ADD_FILE, (char_t *)(&FileParam)))
	{
		PRT_ERR(("RECORD_ADD_FILE error.\n"));
	}

	/*获取当前分区文件状态*/
	memcpy(&g_HdPart, &FileParam, sizeof(FileParam));
	g_HdPart.s_IsLinkDB = 1;
	
	closeDbFile(&g_DbFd);
	sync();
	sleep(1);
	UNLOCK(&g_LockHDPart);
	PRT_TEST(("format hdisk successful\n"));
	InitPartion(FileSize);				//格式化后重新初始化，不需要重启

errExit:
	return Result;
}


