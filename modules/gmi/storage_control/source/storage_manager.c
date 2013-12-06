#include <sys/mount.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <dirent.h>

#include "storage_manager.h"
#include "log_record.h"
#include "dbManager.h"

GlobalOperateInfo       g_HdPart;					/*ȫ��¼���ļ���Ϣ*/
pthread_mutex_t			g_LockHDPart;				/*ȫ��Ӳ�̲�����*/	
sqlite3					*g_DbFd = NULL;				/*���ݿ��ļ�������*/

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

	/*���ж��������ݿ��ļ�*/
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

	/*���ж����ޱ������ݿ��ļ�*/
	Fd = open(TmpParam, O_RDONLY, 0);
	if (-1 == Fd)
	{
		PRT_ERR(("have no %s.\n", TmpParam));
		Result1 = 1;
	}
	close(Fd);

	/*��Ҫ�½����ݿ��ļ�*/
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
	/*���ж�����SD��*/
    Fd = open(SRC_PATH_NAME_SD, O_RDONLY, 0);
	if (Fd == -1) {
	    PRT_ERR(("have no sdisk.\n"));
		goto errExit;
	}
	close(Fd);

	/*�ж�����Ŀ���ļ���,���򴴽�Ŀ���ļ���*/
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
		/*����ʧ�ܺ��ж��Ƿ��Ѿ������أ������Ѿ����ص���ɵĴ��������������*/
		if(EBUSY != errno)
		{
			RetVal = LOCAL_RET_ERR;
			goto errExit;
		}
	}

	/*��ʼ��ȫ�ֱ���*/
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

	/*�ж��Ƿ����ⲿ��ʽ���õĴ��̣�������������Ƚ������ݿ��ļ��͵�һ��¼����ļ�*/
	if(LOCAL_RET_OK == IsNeedNewDatabase())
	{
		/*�õ�sd��������*/
	   if (statfs(DST_PATH_NAME_SD, &DiskStat) < 0) 
	   {
		   PRT_ERR(("get sd card space error. errno = %s\n", strerror(errno)));
		   RetVal = LOCAL_RET_ERR;
		   goto errExit;
	   }
	   /*��sd�������������MBΪ��λ*/
	   DiskSpace = ((DiskStat.f_bsize >> 10) * DiskStat.f_bavail) >> 10;
	   PRT_TEST(("DiskSpace  = %d M\n", DiskSpace));
	   if(DiskSpace < 150)
	   {
	   		PRT_ERR(("DiskSpace is too small.\n"));
			RetVal = LOCAL_RET_ERR;
			goto errExit;
	   }
	   /*����¼���ļ�������Ϊ�����ļ�(���ݿ��ļ�+�����ļ�)Ԥ���ռ�*/
	   if(FileSize > 32)
	   {
	       RecordFiles = (DiskSpace * 997 / 1000 - 120) / FileSize; 
	   }
	   else
	   {
	       RecordFiles = (DiskSpace * 997 / 1000 - 120) / (STREAM_FILE_LEN>>20);
	   }

	   /*������һ¼���ļ�*/
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
	   
		/*��ʼ���ļ���Ϣ��¼(��һ��)*/
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
			/*�����ݿ��ļ��쳣ʱ���Իָ�*/
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
		/*�ж������ݿ��ļ��Ƿ�������*/
		/*�ļ���¼���в鲻������¼��Ƭ�μ�¼����Ϣ*/
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
				/*Ƭ�μ�¼�в鲻��������¼*/
				if(RowRes <= 0)
				{
					IsNeedRecoverDB = TRUE;
				}
				else
				{
					RowRes = 0;
					SegPartInfo.s_SId = g_HdPart.s_LastRecordNo;
					queryDbRecord(g_DbFd, RECORD_QUERY_SEG, METHOD_QUERY_NO, (char_t*)&SegPartInfo, &QueryResult, &RowRes);
					/*Ƭ�μ�¼�в鲻�����һ����¼*/
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
			/*�����ݿ��ļ��ָ������²����ļ���¼��Ϣ*/
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
			/*��ȡ��ǰ�����ļ�״̬*/
			memcpy(&g_HdPart, QueryResult, sizeof(FileIdxHeader));

			memset(&SegPartInfo, 0, sizeof(SegPartInfo));
			RowRes = 0;
			SegPartInfo.s_SId = g_HdPart.s_FirstRecordNo;
			queryDbRecord(g_DbFd, RECORD_QUERY_SEG, METHOD_QUERY_NO, (char_t*)&SegPartInfo, &QueryResult, &RowRes);
			/*Ƭ�μ�¼�в鲻��������¼*/
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
				/*Ƭ�μ�¼�в鲻�����һ����¼*/
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
		g_HdPart.s_SegRecNo = g_HdPart.s_LastRecordNo+1;	/*��֤��ǰƬ��¼���������µ�(��δ��ʹ��)*/
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

	/*ɾ�����ݿ��ļ�*/
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
	/*�����豸*/
	RetVal = MountHdisk(SRC_PATH_NAME_SD, DST_PATH_NAME_SD, "vfat");
	if (RetVal != LOCAL_RET_OK) {
	    PRT_ERR(("mount failed. errno = %s\n",strerror(errno)));
		/*����ʧ�ܺ��ж��Ƿ��Ѿ������أ������Ѿ����ص���ɵĴ��������������*/
		if(EBUSY != errno)
		{
			Result = LOCAL_RET_ERR;
			goto errExit;
		}
	}
	sleep(1);
	
	LOCK(&g_LockHDPart);
	/*�½����ݿ��ļ�*/
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
	
	/*�õ�sd��������*/
   if (statfs(DST_PATH_NAME_SD, &DiskStat) < 0) 
   {
	   PRT_ERR(("get sd card space error. errno = %s\n", strerror(errno))); 
	   Result = LOCAL_RET_ERR;
	   UNLOCK(&g_LockHDPart);
	   closeDbFile(&g_DbFd);
	   goto errExit;
   }
   /*��sd�������������MBΪ��λ*/
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

   /*����¼���ļ�������Ϊ�����ļ�(���ݿ��ļ�+�����ļ�)Ԥ���ռ�*/
   if(FileSize > 32)
   {
   	   RecordFiles = (DiskSpace * 997 / 1000 - 120) / FileSize;
   }
   else
   {
   	   RecordFiles = (DiskSpace * 997 / 1000 - 120) / (STREAM_FILE_LEN>>20);
   }

   /*������һ¼���ļ�*/
    memset(StrFileName, 0, sizeof(StrFileName));
    sprintf(StrFileName, "%s/record%04d%s", DST_PATH_NAME_SD, 0, AV_FILE_NAME);
	if (0 > (Fd = open(StrFileName, O_CREAT|O_RDWR, DEFAULT_FILE_MODE)))
	{
	    PRT_ERR(("can not create file %s\n", StrFileName));
		UNLOCK(&g_LockHDPart);
		goto errExit;
	}
	close(Fd);
   
	/*��ʼ���ļ���Ϣ��¼(��һ��)*/
	memset(&FileParam, 0, sizeof(FileParam));
	FileParam.s_ToRecordFiles = RecordFiles;
	FileParam.s_PartStatus = HD_NORMAL;
	FileParam.s_BuildFileNums = 1;
	if(SQLITE_OK != addDbRecord(g_DbFd, RECORD_ADD_FILE, (char_t *)(&FileParam)))
	{
		PRT_ERR(("RECORD_ADD_FILE error.\n"));
	}

	/*��ȡ��ǰ�����ļ�״̬*/
	memcpy(&g_HdPart, &FileParam, sizeof(FileParam));
	g_HdPart.s_IsLinkDB = 1;
	
	closeDbFile(&g_DbFd);
	sync();
	sleep(1);
	UNLOCK(&g_LockHDPart);
	PRT_TEST(("format hdisk successful\n"));
	InitPartion(FileSize);				//��ʽ�������³�ʼ��������Ҫ����

errExit:
	return Result;
}


