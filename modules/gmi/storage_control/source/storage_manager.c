#include "storage_common.h"
#include "storage_manager.h"
#include "stream_process.h"

#include "log_record.h"
#include "dbManager.h"

GlobalOperateInfo       g_HdPart;					/*ȫ��¼���ļ���Ϣ*/
pthread_mutex_t			g_LockHDPart;				/*ȫ��Ӳ�̲�����*/	
sqlite3					*g_DbFd = NULL;				/*���ݿ��ļ�������*/
int32_t				    g_DbStartBackup = FALSE;	/*�Ƿ�ʼ���ݲ���*/
int32_t				    g_DbBackupCount = 0;	    /*���ݳɹ��ļ�������*/
int32_t                 g_RecScheduleMsg = -1;
int32_t                 g_IsRecordStart[MAX_ENCODER_NUM]; /*��ͨ���Ƿ���¼���ʶ*/
RecordInfoManage  		g_VidRecConfig[MAX_ENCODER_NUM];
RecParamCfg             g_RecParamConfig;
uint32_t                g_SegFileInfo[MAX_ENCODER_NUM];   /*�߼��ļ����ļ���Ϣ*/
RecordParamConfigIn     g_RecRefenceParam;
RecordCtrlIn            g_RecDataInfo;


static int32_t l_RecProcessThreadcreated = 0;
static int32_t l_DbProcessThreadcreated = 0;
static int32_t l_IsStartDbProcessTask = 0;
int32_t l_IsStartDataRecTask[4] = {0};

static int32_t l_ChannelNum = 0;
int32_t l_StreamNum = 0;


static int32_t l_MaxSizePerFile = 0;

#define		PRT_ERR(x)	printf x
#define		PRT_TEST(x)	printf x

#define     FOREVER    while(1)


static int32_t UmountHdisk(char_t *InDstPathName)
{
	if(NULL == InDstPathName)
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");		
		return LOCAL_RET_ERR;
	}
	#if 0
    return umount(InDstPathName);
	#else
	char_t  MkfsCmd[64];
	memset(MkfsCmd, 0, sizeof(MkfsCmd));
	sprintf(MkfsCmd, "umount %s", DST_PATH_NAME_SD);
    if(0 < system(MkfsCmd))
	{
		PRT_ERR(("%s error.\n", MkfsCmd));
		return LOCAL_RET_ERR;
	}
	return LOCAL_RET_OK;
	#endif
}

static int32_t MountHdisk(char_t *InSrcPathName, char_t *InDstPathName, char_t *InType)
{
	if((NULL == InSrcPathName)
		|| (NULL == InDstPathName)
		|| (NULL == InType))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");				
		return LOCAL_RET_ERR;
	}
	#if 0
    return mount(InSrcPathName, InSrcPathName, InType, 32768, NULL);
	#else
	char_t  MkfsCmd[64];
	memset(MkfsCmd, 0, sizeof(MkfsCmd));
	sprintf(MkfsCmd, "mount -t vfat %s %s", SRC_PATH_NAME_SD, DST_PATH_NAME_SD);
    if(0 < system(MkfsCmd))
	{
		PRT_ERR(("%s error.\n", MkfsCmd));
		return LOCAL_RET_ERR;
	}
	return LOCAL_RET_OK;
	#endif
}


/*  �����յ�������д��¼�񻺳���*/
int32_t	VidAudDataToBuf(uint32_t Channel, char_t *PBuffer, int32_t Size, int32_t FrameType)
{
	RecordInfoManage *PVidRecMan = &g_VidRecConfig[Channel];
	RecordDataBuffer *PRecordBuffer = &PVidRecMan->s_RecordBuffer;
	RecordMsgType RecMsg;
	unsigned int  WritePos  = 0;
	unsigned int  RecLen    = 0;
	int MsgLen              = sizeof(RecordMsgType) - sizeof(long);	
	
	WritePos = PRecordBuffer->s_WritePos;
    RecLen   = MEDIA_BUFFER_SIZE - PRecordBuffer->s_WritePos;
	if(NULL == PRecordBuffer->s_Buffer)
	{
		return LOCAL_RET_ERR;
	}
	if (Size > RecLen)
	{
	    memcpy(PRecordBuffer->s_Buffer + PRecordBuffer->s_WritePos, PBuffer, RecLen);
		memcpy(PRecordBuffer->s_Buffer, PBuffer + RecLen, Size - RecLen);
	}
	else
	{
	    memcpy(PRecordBuffer->s_Buffer + PRecordBuffer->s_WritePos, PBuffer, Size);
	}
    PRecordBuffer->s_WritePos += Size;
	PRecordBuffer->s_WritePos %= MEDIA_BUFFER_SIZE;	

	memset(&RecMsg, 0, sizeof(RecordMsgType));
	RecMsg.s_MsgType = MSG_TYPE_REC;	
	if(FrameType == FRAME_TYPE_I)
	{		
	    RecMsg.s_CmdType = CMD_TYPE_REFRESH_I;
        RecMsg.s_Idx     = WritePos;
	    RecMsg.s_Time    = time(NULL);		
	} 
	msgsnd(PVidRecMan->s_RecordMsgId, &RecMsg, MsgLen, IPC_NOWAIT);
	return LOCAL_RET_OK;
}


static int32_t IsNoFormat()
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
	else
	{
		close(Fd);
		Fd = -1;
	}

	memset(TmpParam, 0, sizeof(TmpParam));
	sprintf(TmpParam, "%s/%s", DST_PATH_NAME_SD, DB_FILE_BAK_NAME);

	/*���ж����ޱ������ݿ��ļ�*/
	Fd = open(TmpParam, O_RDONLY, 0);
	if (-1 == Fd)
	{
		PRT_ERR(("have no %s.\n", TmpParam));
		Result1 = 1;
	}
	else
	{
		close(Fd);
		Fd = -1;
	}

	/*��Ҫ��ʽ���豸*/
	if(2 == (Result0+Result1))
	{
		return LOCAL_RET_OK;
	}
	else
	{
		return LOCAL_RET_ERR;
	}
}


static int32_t IsNeedNewDatabase()
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

/*����������ȡ*/
static int32_t GetDiskSpace(int32_t *TotalSpace, int32_t *FreeSpace)
{
	if((NULL == TotalSpace)
		|| (NULL == FreeSpace))
	{
		PRT_ERR(("GetDiskSpace Inparam error.\n"));
		return LOCAL_RET_ERR;
	}
	struct statfs	DiskStat;
	int32_t RetVal = LOCAL_RET_OK;
    do
    {
         /*�õ�sd��������*/
        if(statfs(DST_PATH_NAME_SD, &DiskStat) < 0) 
        {
            PRT_ERR(("get sd card space error. errno = %s\n", strerror(errno)));
            RetVal = LOCAL_RET_ERR;
            break;
        }
        /*��sd�������������MBΪ��λ*/
       *TotalSpace = ((DiskStat.f_bsize >> 10) * DiskStat.f_bavail) >> 10;
       PRT_TEST(("DiskSpace  = %d M\n", *TotalSpace));
       if(*TotalSpace < 150)
       {
           PRT_ERR(("DiskSpace is too small.\n"));
           RetVal = LOCAL_RET_ERR;
           break;
       }
	   *FreeSpace = ((DiskStat.f_bsize >> 10) * DiskStat.f_bfree) >> 10;
	   
    }while(0);
	return RetVal;	
}

static void_t InitHdPart()
{
	g_DbFd = NULL;
	memset(&g_HdPart, 0, sizeof(g_HdPart));
	
}


/*¼��������ݳ�ʼ��*/
int32_t VidRecordInit()
{
	int32_t            Tmp     = 0;
	RecordInfoManage  *PChannel = NULL;

	for(Tmp = 0; Tmp < MAX_ENCODER_NUM; ++Tmp) 
	{
		 PChannel = &(g_VidRecConfig[Tmp]);
		 PChannel->s_RecordBuffer.s_Buffer      = (char_t*)memalign(0x10, MEDIA_BUFFER_SIZE);
		 PChannel->s_RecordBuffer.s_WritePos   = 0;
		 PChannel->s_RecordBuffer.s_ReadPos    = 0;
		 PChannel->s_RecordBuffer.s_Size       = MEDIA_BUFFER_SIZE;
		 PChannel->s_RecordBuffer.s_TotalWrite = 0;
		 PChannel->s_IFrameNums  = 0;
		 PChannel->s_RecordMsgId = -1;
		 PChannel->s_RecordMsgId  = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
		 if(PChannel->s_RecordMsgId < 0)
		 {
			DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "msgget error.\n");				
		 	return LOCAL_RET_ERR;
		 }
		 memset(&(g_SegFileInfo[Tmp]), 0, sizeof(SegFileInfo));
		 g_IsRecordStart[Tmp] = FALSE;
	 }

	memset(&g_RecRefenceParam, 0, sizeof(g_RecRefenceParam));
	memset(&g_RecParamConfig, 0, sizeof(g_RecParamConfig));
	memset(&g_RecDataInfo, 0, sizeof(g_RecDataInfo));
	/*��ʼ��ȫ�ֱ���*/
	InitHdPart();
	return LOCAL_RET_OK;
}

/*¼��������ݷ���ʼ��*/
int32_t VidRecordUninit()
{
	int32_t            Tmp     = 0;
	RecordInfoManage  *PChannel = NULL;
	RecordMsgType      RecordMsg;
	int32_t            RecMsgLen  = sizeof(RecordMsgType) - sizeof(long_t);

	PRT_TEST(("VidRecordUninit start......\n"));
	for(Tmp = 0; Tmp < MAX_ENCODER_NUM; ++Tmp) 
	{	
		PChannel = &(g_VidRecConfig[Tmp]);
		if(NULL != PChannel)
		{
			if ((g_RecParamConfig.s_IsEnableRecord == FALSE) && (g_IsRecordStart[0] == TRUE))
			{
				memset(&RecordMsg, 0, sizeof(RecordMsgType));
				RecordMsg.s_MsgType        = MSG_TYPE_REC;
				RecordMsg.s_CmdType       = CMD_TYPE_STOP_REC;
				msgsnd(PChannel->s_RecordMsgId, &RecordMsg, RecMsgLen, 0);
				g_IsRecordStart[0]    = FALSE;
			}
			usleep(200000);
		
			if(PChannel->s_RecordMsgId > 0)
			{
				msgctl(PChannel->s_RecordMsgId, IPC_RMID, 0);
				PChannel->s_RecordMsgId = -1;
			}
			if(PChannel->s_RecordBuffer.s_Buffer != NULL)
			{
			 	free(PChannel->s_RecordBuffer.s_Buffer);
			 	PChannel->s_RecordBuffer.s_Buffer = NULL;
			}
		}
		else
		{
			PRT_TEST(("VidRecordUninit PChannel NULL\n"));
		}
		 g_SegFileInfo[Tmp] = 0;
	}

	if(g_RecScheduleMsg > 0)
	{
		msgctl(g_RecScheduleMsg, IPC_RMID, 0);
		g_RecScheduleMsg = -1;
	}

	l_DbProcessThreadcreated = 0;
	l_RecProcessThreadcreated = 0;
	l_IsStartDbProcessTask = 0;
	l_IsStartDataRecTask[0] = 0;
	memset(&g_RecRefenceParam, 0, sizeof(g_RecRefenceParam));
	memset(&g_RecDataInfo, 0, sizeof(g_RecDataInfo));
	sleep(2);
	PRT_TEST(("VidRecordUninit end......\n"));		
	return LOCAL_RET_OK;
}


static int32_t CreateDataReceivedThread(int32_t ChanId, int32_t StreamId)
{
	pthread_t DataRecTid;
    int32_t RetVal;

	if((StreamId<0) || (StreamId>1))
	{
		PRT_ERR(("CreateDataReceivedThread inParam error.\n"));
		return LOCAL_RET_ERR;
	}

	if(1 == l_IsStartDataRecTask[StreamId])
	{
        DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "DataReceived has created.");
		return LOCAL_RET_OK;
	}
	l_StreamNum = StreamId;
    RetVal = pthread_create(&DataRecTid, NULL, RecordDataReceiveTask, &l_StreamNum);
    if (0 != RetVal)
    {
        DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "RecordDataReceiveTask thread create error.");
		l_IsStartDbProcessTask = 0;
        return LOCAL_RET_ERR;
    }
	l_IsStartDataRecTask[StreamId] = 1;

	return LOCAL_RET_OK;
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
	char_t  MkfsCmd[64];
	
	PRT_TEST(("InitPartion start ......\n"));

	memset(StrFileName, 0, sizeof(StrFileName));
	memset(&FileParam, 0, sizeof(FileParam));
	/*���ж�����SD��*/
    Fd = open(SRC_PATH_NAME_SD, O_RDONLY, 0);
	if (Fd == -1)
	{
		if(-1 == (Fd = open(SRC_PATH_NAME_SD2, O_RDONLY, 0)))
		{
		    PRT_ERR(("have no sdisk.\n"));
			goto errExit;
		}
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
	#if 0
	RetVal = MountHdisk(SRC_PATH_NAME_SD, DST_PATH_NAME_SD, "vfat");
	sync();
	sleep(1);
	if (RetVal != LOCAL_RET_OK) {
	    PRT_ERR(("mount failed. errno = %s\n", strerror(errno)));
		/*����ʧ�ܺ��ж��Ƿ��Ѿ������أ������Ѿ����ص���ɵĴ��������������*/
		if(EBUSY != errno)
		{
			RetVal = LOCAL_RET_ERR;
			goto errExit;
		}
	}
	//#else
	memset(MkfsCmd, 0, sizeof(MkfsCmd));
	sprintf(MkfsCmd, "/bin/mount -t vfat %s %s", SRC_PATH_NAME_SD, DST_PATH_NAME_SD);
    system(MkfsCmd);
	sync();
	sleep(1);
	#endif

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
		queryDbRecord(g_DbFd, RECORD_QUERY_FILE, METHOD_QUERY_NO, (char_t*)&PartInfo, &QueryResult, 1, &RowRes);
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
				queryDbRecord(g_DbFd, RECORD_QUERY_SEG, METHOD_QUERY_NO, (char_t*)&SegPartInfo, &QueryResult, 1, &RowRes);
				/*Ƭ�μ�¼�в鲻��������¼*/
				if(RowRes <= 0)
				{
					IsNeedRecoverDB = TRUE;
				}
				else
				{
					RowRes = 0;
					SegPartInfo.s_SId = g_HdPart.s_LastRecordNo;
					queryDbRecord(g_DbFd, RECORD_QUERY_SEG, METHOD_QUERY_NO, (char_t*)&SegPartInfo, &QueryResult, 1, &RowRes);
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
			queryDbRecord(g_DbFd, RECORD_QUERY_FILE, METHOD_QUERY_NO, (char_t*)&PartInfo, &QueryResult, 1, &RowRes);
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
			queryDbRecord(g_DbFd, RECORD_QUERY_SEG, METHOD_QUERY_NO, (char_t*)&SegPartInfo, &QueryResult, 1, &RowRes);
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
				queryDbRecord(g_DbFd, RECORD_QUERY_SEG, METHOD_QUERY_NO, (char_t*)&SegPartInfo, &QueryResult, 1, &RowRes);
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
		PRT_TEST(("s_firstWriteTime = %d\n", (int)g_HdPart.s_FirstWriteTime));
		PRT_TEST(("s_lastWriteTime = %d\n", (int)g_HdPart.s_LastWriteTime));
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
	CreateDataReceivedThread(0,0);
	l_MaxSizePerFile = FileSize;
errExit:
	if(NULL != QueryResult)
	{
		free(QueryResult);
		QueryResult = NULL;	
	}
	PRT_TEST(("InitPartion end......\n"));
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

	PRT_TEST(("Sdformat start ......\n"));

	closeDbFile(&g_DbFd);

	CREATE_LOCK(&g_LockHDPart);
    LOCK(&g_LockHDPart);
    InitHdPart();
	VidRecordUninit();
	UNLOCK(&g_LockHDPart);

	/*���ж�����SD��*/
    Fd = open(SRC_PATH_NAME_SD, O_RDONLY, 0);
	if (Fd == -1) 
	{
		if(-1 == (Fd = open(SRC_PATH_NAME_SD2, O_RDONLY, 0)))
		{
		    PRT_ERR(("have no sdisk.\n"));
			goto errExit;
		}
	}
	close(Fd);

	/*�ж�����Ŀ���ļ���,���򴴽�Ŀ���ļ���*/
	if(opendir(DST_PATH_NAME_SD) == NULL)
	{
		PRT_ERR(("have no %s, so create\n",  DST_PATH_NAME_SD));
		if(mkdir(DST_PATH_NAME_SD, 0777) == -1)
		{
			PRT_ERR(("create %s failed\n",  DST_PATH_NAME_SD));
			goto errExit;
		}
	}

	/*ɾ�����ݿ��ļ�*/
	memset(MkfsCmd, 0, sizeof(MkfsCmd));
	sprintf(MkfsCmd, "%s %s%s", "rm -f ", DST_PATH_NAME_SD, "/*");
	system(MkfsCmd);
	sync();
	sleep(1);
	/*ж���豸*/
	#if 0
	RetVal = UmountHdisk(DST_PATH_NAME_SD);
	sync();
	sleep(1);
	if (RetVal != LOCAL_RET_OK) 
	{
	    PRT_ERR(("UmountHdisk failed. errno = %s\n",strerror(errno)));
	}
	//#else
	memset(MkfsCmd, 0, sizeof(MkfsCmd));
	sprintf(MkfsCmd, "/bin/umount %s", DST_PATH_NAME_SD);
    system(MkfsCmd);
	sync();
	sleep(1);
	#endif

	memset(MkfsCmd, 0, sizeof(MkfsCmd));
	sprintf(MkfsCmd, "%s %s", "mkfs.vfat", SRC_PATH_NAME_SD);
    system(MkfsCmd);
	sync();
    sleep(1);

	#if 0
	/*�����豸*/
	RetVal = MountHdisk(SRC_PATH_NAME_SD, DST_PATH_NAME_SD, "vfat");
	sync();
	sleep(1);
	if (RetVal != LOCAL_RET_OK) {
	    PRT_ERR(("mount failed. errno = %s\n",strerror(errno)));
		/*����ʧ�ܺ��ж��Ƿ��Ѿ������أ������Ѿ����ص���ɵĴ��������������*/
		if(EBUSY != errno)
		{
			Result = LOCAL_RET_ERR;
			goto errExit;
		}
	}
	//#else
	memset(MkfsCmd, 0, sizeof(MkfsCmd));
	sprintf(MkfsCmd, "/bin/mount -t vfat %s %s", SRC_PATH_NAME_SD, DST_PATH_NAME_SD);
    system(MkfsCmd);
	sync();
	#endif
		
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
   PRT_TEST(("DiskSpace  = %d MB[%d:%d:%d:%d]\n", DiskSpace, DiskStat.f_bsize, (int)DiskStat.f_bavail, (int)DiskStat.f_blocks, (int)DiskStat.f_bfree));
   if(DiskSpace < 150)
   {
   		PRT_ERR(("DiskSpace %d is too small.\n", DiskSpace));
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
	if(LOCAL_RET_OK == VidRecordInit())
	{
		if(LOCAL_RET_OK == InitPartion(FileSize))				//��ʽ�������³�ʼ��������Ҫ����
		{
			CreateRecProcessThread(0);
			CreateDbProcessThread();
		}
	}
errExit:	
	PRT_TEST(("Sdformat end ......\n"));
	return Result;
}

/*¼��ƻ�����*/
static void *RecordScheduleProcessTask(void *InParam)
{
    int32_t            i;
	int32_t            ChanIdx;
	int32_t            RetVal;
	int32_t            RecMsgLen  = sizeof(RecordMsgType) - sizeof(long_t);
	int32_t            TrigMsgLen = sizeof(RecordTriggerMsg) - sizeof(long);
	int32_t            CurrDay;
	time_t             Curhm;
    time_t             CurrTime;
	boolean_t          IsRecord;
	boolean_t          IsFindTimeSeg = FALSE;
	uint8_t            RecordType;
	time_t             RecStopTime[MAX_ENCODER_NUM] = {0};
	struct tm          CurrTm;
	struct tm          TempTm;
	RecordInfoManage   *PChan = NULL;
	RecordMsgType      RecordMsg;
	RecordParam        *PRecordParam = NULL;
	TimeSegment        TimeSegment;
	RecordTriggerMsg   RecTriggerMsg;

	pthread_detach(pthread_self()); 
    g_RecScheduleMsg = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
	if(-1 == g_RecScheduleMsg)
	{
	    DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "g_RecScheduleMsg msgget error.");
	    goto ErrExit;
	}
	
	for (i=0; i < MAX_ENCODER_NUM; i++) {
		g_IsRecordStart[i] = FALSE;
	}
	
	printf("RecordScheduleProcessTask start...\n");
	FOREVER 
	{
		if(g_RecScheduleMsg < 0)
		{	
			DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "RecordScheduleProcessTask exit.\n");							
			break;
		}
	    RetVal   = msgrcv(g_RecScheduleMsg, &RecTriggerMsg, TrigMsgLen, MSG_TYPE_TRIG_REC, IPC_NOWAIT);	    
		CurrTime = time(NULL);
		CurrTm   = *localtime(&CurrTime);
		Curhm    = (CurrTm.tm_hour << 16) | CurrTm.tm_min;
		CurrDay  = CurrTm.tm_wday;
		if (RetVal == TrigMsgLen) 
		{
		    switch(RecTriggerMsg.s_CmdType)
			{
				case REC_TRIG_ALARMIN:  /*������������¼��*/
				case REC_TRIG_MOTDETECT:  /*�ƶ��������¼��ֻ��������ǰͨ��¼��*/				
					DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "no support type of trigger %d.", RecTriggerMsg.s_CmdType);		
					break;
				default:
					if(RecTriggerMsg.s_TriggerChans < MAX_ENCODER_NUM)
					{
						g_SegFileInfo[RecTriggerMsg.s_TriggerChans] = RecTriggerMsg.s_SegFileInfo;
					}
					break;
		    }
		}
		for (ChanIdx = 0; ChanIdx < MAX_ENCODER_NUM; ChanIdx++)
		{
			PChan = &(g_VidRecConfig[ChanIdx]);
			if(IsFindTimeSeg == TRUE)
			{
				if(g_RecParamConfig.s_AllDayRecParam[CurrDay].s_IsAllDayRecord == FLAG_ALLDAY_REC_CLOSE)
				{
					
					for (i = 0; i < 4; i++) 
					{
				        PRecordParam = &(g_RecParamConfig.s_RecordParam[CurrDay][i]);
						if ((PRecordParam->s_RecordTimeSeg.s_StopTime > PRecordParam->s_RecordTimeSeg.s_StartTime)
							&& (Curhm >= PRecordParam->s_RecordTimeSeg.s_StartTime)
							&& (Curhm < PRecordParam->s_RecordTimeSeg.s_StopTime)) 
						{
							TempTm.tm_year = CurrTm.tm_year;
							TempTm.tm_mon  = CurrTm.tm_mon;
							TempTm.tm_mday = CurrTm.tm_mday;
							
							if (PRecordParam->s_RecordTimeSeg.s_StopTime == (24<<16)) 
							{
								
								TempTm.tm_hour = 23;
								TempTm.tm_min  = 59;
								TempTm.tm_sec  = 59;
								RecStopTime[ChanIdx] = mktime(&TempTm) + 1;
								
							}
							else 
							{
								
								TempTm.tm_hour = (PRecordParam->s_RecordTimeSeg.s_StopTime >>16) & 0xffff;
								TempTm.tm_min  = PRecordParam->s_RecordTimeSeg.s_StopTime & 0xffff;
								TempTm.tm_sec  = 0;
								RecStopTime[ChanIdx] = mktime(&TempTm);
							}								
						}
					}
				}
				else
				{
					
					TempTm.tm_year = CurrTm.tm_year;
					TempTm.tm_mon  = CurrTm.tm_mon;
					TempTm.tm_mday = CurrTm.tm_mday;
					TempTm.tm_hour = 23;
					TempTm.tm_min  = 59;
					TempTm.tm_sec  = 59;
					RecStopTime[ChanIdx] = mktime(&TempTm) + 1;
				}
			}
 			/*���¼���Ѿ��������ж�ֹͣ¼��������ɲ�����*/
		    if (TRUE == g_IsRecordStart[ChanIdx]) 
			{
				 if (CurrTime >= RecStopTime[ChanIdx])
				 {
				    memset(&RecordMsg, 0, sizeof(RecordMsgType));
		            RecordMsg.s_MsgType  = MSG_TYPE_REC;
					RecordMsg.s_CmdType = CMD_TYPE_STOP_REC;
					msgsnd(PChan->s_RecordMsgId, &RecordMsg, RecMsgLen, 0);
					g_IsRecordStart[ChanIdx] = FALSE;
		        }
		    }

			/*����Ƿ���Ҫ����¼��ƻ�*/
			if ((g_RecParamConfig.s_IsEnableRecord == TRUE) && (g_IsRecordStart[ChanIdx] == FALSE))
			{
			    IsFindTimeSeg = FALSE;
				/*ȫ��¼��*/
				if (1 == g_RecParamConfig.s_AllDayRecParam[CurrDay].s_IsAllDayRecord) 
				{   
					IsFindTimeSeg = TRUE;
					TempTm.tm_year = CurrTm.tm_year;
					TempTm.tm_mon  = CurrTm.tm_mon;
					TempTm.tm_mday = CurrTm.tm_mday;
					TempTm.tm_hour = 23;
					TempTm.tm_min  = 59;
					TempTm.tm_sec  = 59;
					TimeSegment.s_StopTime = mktime(&TempTm) + 1;
				} 
				else 
				{
					 /*¼��ʱ���*/
				    for (i = 0; i < 4; i++)
					{ 
				        PRecordParam = &(g_RecParamConfig.s_RecordParam[CurrDay][i]);
						if ((PRecordParam->s_RecordTimeSeg.s_StopTime > PRecordParam->s_RecordTimeSeg.s_StartTime)
							&& (Curhm >= PRecordParam->s_RecordTimeSeg.s_StartTime)
							&& (Curhm < PRecordParam->s_RecordTimeSeg.s_StopTime)) 
						{
							IsFindTimeSeg   = TRUE;
							TempTm.tm_year = CurrTm.tm_year;
							TempTm.tm_mon  = CurrTm.tm_mon;
							TempTm.tm_mday = CurrTm.tm_mday;
							if (PRecordParam->s_RecordTimeSeg.s_StopTime == (24<<16))
							{
								TempTm.tm_hour = 23;
								TempTm.tm_min  = 59;
								TempTm.tm_sec  = 59;
								TimeSegment.s_StopTime = mktime(&TempTm) + 1;
							}
							else
							{
								TempTm.tm_hour = (PRecordParam->s_RecordTimeSeg.s_StopTime >>16) & 0xffff;
								TempTm.tm_min  = PRecordParam->s_RecordTimeSeg.s_StopTime & 0xffff;
								TempTm.tm_sec  = 0;
								TimeSegment.s_StopTime = mktime(&TempTm);
							}
							break;
						}  
 				    }  
				} 

				IsRecord = FALSE;
				if (IsFindTimeSeg == TRUE)
				{
				    IsRecord      = TRUE;
				    RecordType = TYPE_REC_TIME;
				}
				else 
				{
				    IsRecord = FALSE;
				}

				if ((IsRecord == TRUE) && (g_IsRecordStart[ChanIdx] == FALSE))
				{
				    memset(&RecordMsg, 0, sizeof(RecordMsgType));
					RecordMsg.s_MsgType     = MSG_TYPE_REC;
					RecordMsg.s_CmdType    = CMD_TYPE_START_REC;
					RecordMsg.s_RecordType = RecordType;
					msgsnd(PChan->s_RecordMsgId, &RecordMsg, RecMsgLen, 0);
					g_IsRecordStart[ChanIdx] = TRUE;
					RecStopTime[ChanIdx]   = TimeSegment.s_StopTime;
				}
			} 

			PRT_TEST(("g_RecParamConfig.s_IsEnableRecord=%d, g_IsRecordStart[%d]=%d\n", 
				g_RecParamConfig.s_IsEnableRecord, ChanIdx, g_IsRecordStart[ChanIdx]));

			if ((g_RecParamConfig.s_IsEnableRecord == FALSE) && (g_IsRecordStart[ChanIdx] == TRUE))
			{
				memset(&RecordMsg, 0, sizeof(RecordMsgType));
				RecordMsg.s_MsgType        = MSG_TYPE_REC;
				RecordMsg.s_CmdType       = CMD_TYPE_STOP_REC;
				msgsnd(PChan->s_RecordMsgId, &RecordMsg, RecMsgLen, 0);
				g_IsRecordStart[ChanIdx]    = FALSE;	
			}
		}
		sleep(1);
	}

ErrExit:
	
	printf("RecordScheduleProcessTask stop00...\n");
	pthread_exit(NULL);
	
	printf("RecordScheduleProcessTask stop...\n");
}

/*ˢ��¼��BUF���I֡��Ϣ*/
static void RefreshIFrameInfo(int32_t Chan, ulong_t Idx, time_t Time)
{
    int32_t          i,j;
    int              PreRecordTime = 0;  /*Ԥ¼ʱ��*/
    IFrameInfo      *PIFrames;
	RecordInfoManage *PChan;
	
	PChan    = &(g_VidRecConfig[Chan]);
	PIFrames = PChan->s_IFrames;
    PreRecordTime = g_RecParamConfig.s_PreRecordTime;
   
	if (PChan->s_IFrameNums >= MAX_NUM_IFRAME)
	{
		PChan->s_IFrameNums = MAX_NUM_IFRAME - 1;
		memcpy((char *)&PIFrames[0], (char *)&PIFrames[1], (MAX_NUM_IFRAME-1)*sizeof(IFrameInfo));
	}
	
	if (PChan->s_RecordStart)
	{
	    PIFrames[PChan->s_IFrameNums].s_Idx = Idx;
	    PIFrames[PChan->s_IFrameNums].s_Time = Time;
	    PChan->s_IFrameNums++;
	}
	else
	{
		/*��Ԥ¼*/
	    if (PreRecordTime == 0)
		{ 
	        PChan->s_IFrameNums = 0;
	    }
		else if (PreRecordTime > 0)
	    { 
            for (i=0; i < PChan->s_IFrameNums; i++)
			{
                if ((Time - PIFrames[i].s_Time) < PreRecordTime)
				{
					break;
                }
            }
			if (i > 1)
			{
			    i--;
				PChan->s_IFrameNums -= i;
				 /*�޳�ǰi��I֡*/
				for(j=0; j<PChan->s_IFrameNums; j++)
				{
					PIFrames[j].s_Idx = PIFrames[j+i].s_Idx;
					PIFrames[j].s_Time  = PIFrames[j+i].s_Time;
				}
			}
	    }
	    PIFrames[PChan->s_IFrameNums].s_Idx = Idx;
	    PIFrames[PChan->s_IFrameNums].s_Time	= Time;
	    PChan->s_IFrameNums++;
	    PChan->s_RecordBuffer.s_ReadPos = PIFrames[0].s_Idx;
	}/*end else*/
}

static int32_t writeFile(int32_t FdId, char_t *Data, int32_t DataLen)
{
	int nLen = 0;
	if((FdId < 0) || (NULL == Data))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception,"writeFile param error.\n");		
		return LOCAL_RET_ERR;
	}
	if(DataLen != (nLen = write(FdId, Data, DataLen)))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception,"writeFile nLen %d dataLen %d error.\n", nLen, DataLen);
		return LOCAL_RET_ERR;
	}

	return LOCAL_RET_OK;
}

static int32_t hdPart_Service(int32_t CmdType, SegmentIdxRecord * SegParam)
{
	time_t   CurrTime;	
	char_t   StrFileName[64];
	int32_t  Fd = -1;
	SegmentIdxRecord SegTmpParam;
	char **QueryResult;
	int i = 0;
	int RowRes = 0;
	int FirstSegRecHeadPos = 0;	//�����Ƭ��¼�����ʼƬ��λ��(���)
	int FirstSegRecTailPos = 0;	//����Ƭ��¼���ĩβƬ��λ��(���)
	int CurSegRecHeadPos = 0;	//��ǰ��Ƭ��¼�����ʼƬ��λ��(���)
	int CurSegRecTailPos = 0;	//��ǰ��Ƭ��¼��Ľ�β(�Ѿ�д��)Ƭ��λ��(���)
	
	switch(CmdType)
	{
		case GET_WRITABLE_FILE:		/*��ȡ��д�ļ����ƺ�Ƭ�ο�дλ��*/
			PRT_TEST(("****GET_WRITABLE_FILE***\n"));
			LOCK(&g_LockHDPart);
			if(HD_ERROR == g_HdPart.s_PartStatus)
			{
				PRT_ERR(("disk is error!\n"));
				UNLOCK(&g_LockHDPart);
				goto errExit;
			}

			if((HD_FULL == g_HdPart.s_PartStatus) && (FALSE == g_RecParamConfig.s_IsCyclicRecord))
			{
				PRT_ERR(("disk is full!\n"));
				UNLOCK(&g_LockHDPart);
				goto errExit;
			}

			//g_hdPart.m_segRecNo = g_hdPart.m_lastRecordNo + 1;					
			UNLOCK(&g_LockHDPart);
			break;
		case UPDATE_TABLE_DB:		/*�����ļ�ͷ��Ϣ������Ƭ�μ�¼��Ϣ*/
			PRT_TEST(("****UPDATE_TABLE_DB***\n"));
			LOCK(&g_LockHDPart);
			/*�����ļ�ͷ��Ϣ*/
			updateDbRecord(g_DbFd, RECORD_UPDATE_FILE, 0, (char_t *)(&g_HdPart));
			updateDbRecord(g_DbFd, RECORD_UPDATE_SEG, 0, (char_t *)SegParam);
			UNLOCK(&g_LockHDPart);
			break;
		case ADD_TABLE_DB:
			PRT_TEST(("****ADD_TABLE_DB***\n"));
			LOCK(&g_LockHDPart);
			if(LOCAL_RET_OK != addDbRecord(g_DbFd, RECORD_ADD_SEG, (char_t *)SegParam))
			{
				g_HdPart.s_LastRecordNo = g_HdPart.s_SegRecNo;
				g_HdPart.s_SegRecNo = g_HdPart.s_LastRecordNo+1;
				SegParam->s_SId = g_HdPart.s_SegRecNo;
				addDbRecord(g_DbFd, RECORD_ADD_SEG, (char_t *)SegParam);
				PRT_ERR(("Last stop record abnormal\n"));
			}
			UNLOCK(&g_LockHDPart);
			break;
			
		case FINALIZE_FILE:
			PRT_TEST(("****FINALIZE_FILE***\n"));
			LOCK(&g_LockHDPart);			
			SegParam->s_SId = g_HdPart.s_SegRecNo;
			
			CurrTime = time(NULL);
			SegParam->s_EndTime = CurrTime;		
			g_HdPart.s_LastRecordNo = g_HdPart.s_SegRecNo;
			g_HdPart.s_SegRecNo = g_HdPart.s_LastRecordNo+1;
			g_HdPart.s_CurWriteSegNo = (SegParam->s_RecSegNo + SegParam->s_RecSegLen) % MAX_SEG_PER_FILE;
			UNLOCK(&g_LockHDPart);
			break;
		case UPDATE_RECORD_FIRST:
			PRT_TEST(("****UPDATE_RECORD_FIRST***\n"));
			LOCK(&g_LockHDPart);
			if(HD_FULL == g_HdPart.s_PartStatus)
			{
				QueryResult = (char**)malloc(sizeof(char *));
				memset(QueryResult, 0, sizeof(char *));
				QueryResult[0] = (char *)malloc(sizeof(SegmentIdxRecord) + sizeof(char));
				if(NULL == QueryResult[0])
				{
					DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception,"quertResult[0] malloc error, please reboot.\n");
					UNLOCK(&g_LockHDPart);
					free(QueryResult);
					break;
				}
				
				if(FALSE != g_RecParamConfig.s_IsCyclicRecord)	/*��������ʱ����ѭ������*/
				{
					while(1)
					{
						memset(&SegTmpParam, 0, sizeof(SegTmpParam));
						memset(QueryResult[0], 0, sizeof(SegmentIdxRecord) + sizeof(char));
						RowRes = 0;
						SegTmpParam.s_SId = g_HdPart.s_FirstRecordNo;		
						/*��ID������ѯ*/
						queryDbRecord(g_DbFd,RECORD_QUERY_SEG,METHOD_QUERY_NO,(char_t *)(&SegTmpParam),QueryResult, 1, &RowRes);
						if((NULL != QueryResult[0]) && (0 < RowRes))
						{
							memcpy(&SegTmpParam, QueryResult[0], sizeof(SegmentIdxRecord));
						}
						else
						{
							/*�����ֲ�ѯ�����¼�޽��ʱ�������¼Ҫ����˳��*/
							if(0 == RowRes)
							{
								DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception,"###first record %d no exist.####\n", g_HdPart.s_FirstRecordNo);
								g_HdPart.s_FirstRecordNo++;
								if(g_HdPart.s_FirstRecordNo >= g_HdPart.s_CurWriteSegNo)
								{
									break;
								}
								continue;
							}
							else
							{
								UNLOCK(&g_LockHDPart);
								break;
							}
						}

						FirstSegRecHeadPos = SegTmpParam.s_RecFileNo*MAX_SEG_PER_FILE + SegTmpParam.s_RecSegNo;
						FirstSegRecTailPos = FirstSegRecHeadPos + SegTmpParam.s_RecSegLen -1;
						CurSegRecHeadPos = SegParam->s_RecFileNo*MAX_SEG_PER_FILE + SegParam->s_RecSegNo;
						CurSegRecTailPos = CurSegRecHeadPos + SegParam->s_RecSegLen -1;

						/*����Ƭ�μ�¼�����λ��ͨ���ڵ�ǰƬ��¼��ʼ��ŵĺ��棬�������һ���ļ����и���ʱ��
						����Ƭ��¼���¼����λ�ڵ�һ���ļ����ͳ���������Ƭ��¼���ڵ�ǰƬ��¼���ǰ�棬��Ҫ����
						����Ƭ��¼��ľ���λ�á���������ļ�����Ƭ�δ���Ŀ���������ڵ�ǰƬ��¼��ʼ��ŵĺ��桿*/
						if(CurSegRecHeadPos > (FirstSegRecHeadPos + TIME_INTERVAL_UPDATE))
						{
							FirstSegRecHeadPos += g_HdPart.s_ToRecordFiles*MAX_SEG_PER_FILE;
							FirstSegRecTailPos += g_HdPart.s_ToRecordFiles*MAX_SEG_PER_FILE;
						}

						
						if((CurSegRecHeadPos < FirstSegRecTailPos) && (CurSegRecTailPos > FirstSegRecHeadPos))
						{
							if(FirstSegRecTailPos > CurSegRecTailPos)
							{
								//��������Ƭ��¼���¼
								SegTmpParam.s_RecFileNo = g_HdPart.s_CurWriteFileNo;
								SegTmpParam.s_RecSegNo = g_HdPart.s_CurWriteSegNo;
								//���������¼��ʱ��
								SegTmpParam.s_StartTime += (SegTmpParam.s_EndTime - SegTmpParam.s_StartTime)/SegTmpParam.s_RecSegLen;
								SegTmpParam.s_RecSegLen = FirstSegRecTailPos  - CurSegRecTailPos;
								updateDbRecord(g_DbFd, RECORD_UPDATE_SEG, 0, (char_t *)(&SegTmpParam));
								UNLOCK(&g_LockHDPart);
								break;
							}
							else
							{
								//ɾ������Ƭ��¼���¼
								deleteDbRecord(g_DbFd,SegTmpParam.s_SId, 1);
								g_HdPart.s_FirstRecordNo++;
							}
						}
						else if((FirstSegRecTailPos <= CurSegRecHeadPos) 
							|| ((FirstSegRecTailPos >= CurSegRecHeadPos) && (FirstSegRecTailPos <= CurSegRecTailPos)))
						{
							//ɾ������Ƭ��¼���¼
							deleteDbRecord(g_DbFd, SegTmpParam.s_SId, 1);
							g_HdPart.s_FirstRecordNo++;
						}
						else
						{
							//δ���ָ��ǣ�����Ҫ����
							UNLOCK(&g_LockHDPart);
							break;
						}

					}
					
					/*��������Ƭ��¼���¼ʱ��*/
					memset(&SegTmpParam, 0, sizeof(SegTmpParam));
					SegTmpParam.s_SId = g_HdPart.s_FirstRecordNo;
					queryDbRecord(g_DbFd,RECORD_QUERY_SEG,METHOD_QUERY_NO,(char_t *)(&SegTmpParam),QueryResult, 1, &RowRes);
					if((NULL != QueryResult[0]) && (0 < RowRes))
					{
						memcpy(&SegTmpParam, QueryResult[0], sizeof(SegmentIdxRecord));
						g_HdPart.s_FirstWriteTime = SegTmpParam.s_StartTime;
					}
				}
				free(QueryResult[0]);
				free(QueryResult);
			}
			UNLOCK(&g_LockHDPart);
			break;
		case UPDATE_RECORD_FILE:
			PRT_TEST(("****UPDATE_RECORD_FILE***\n"));
			LOCK(&g_LockHDPart);
			if(HD_FULL != g_HdPart.s_PartStatus)				/*����δ��ʱ�½�¼���ļ�*/
			{
				memset(StrFileName, 0, sizeof(StrFileName));
				sprintf(StrFileName, "%s/record%04d%s", DST_PATH_NAME_SD, g_HdPart.s_CurWriteFileNo+1, AV_FILE_NAME);
				if (LOCAL_RET_ERR == (Fd = open(StrFileName, O_CREAT|O_RDWR, DEFAULT_FILE_MODE)))
				{
				    PRT_ERR(("can not create file %s\n", StrFileName));
					UNLOCK(&g_LockHDPart);
					goto errExit;
				}
				close(Fd);
				g_HdPart.s_CurWriteFileNo = (g_HdPart.s_CurWriteFileNo + 1) % g_HdPart.s_ToRecordFiles;
				g_HdPart.s_CurWriteSegNo = 0;
				g_HdPart.s_BuildFileNums++;
			}
			else
			{	
				if(FALSE != g_RecParamConfig.s_IsCyclicRecord)	/*��������ʱ����ѭ������*/
				{
					if(g_HdPart.s_CurWriteSegNo >= MAX_SEG_PER_FILE-1)
					{
						g_HdPart.s_CurWriteFileNo = (g_HdPart.s_CurWriteFileNo + 1) % g_HdPart.s_ToRecordFiles;
					}
					g_HdPart.s_CurWriteSegNo = 0;
				}
				else
				{
					PRT_ERR(("write file no cycle%s\n", StrFileName));
					UNLOCK(&g_LockHDPart);
					goto errExit;
				}
			}
			UNLOCK(&g_LockHDPart);
			
			break;
		default:
			break;
	}
	return LOCAL_RET_OK;
errExit:
	return LOCAL_RET_ERR;
}

/*���ҿ�д�ļ�*/
static int32_t FindNewFile()
{
	int32_t Fd = -1;
	int32_t  Retry;
	SegmentIdxRecord SegParam;
	char_t StrFileName[64];
	int32_t StartOffSet;

	memset(&SegParam, 0, sizeof(SegmentIdxRecord));
	SegParam.s_SId = g_HdPart.s_SegRecNo;
	PRT_TEST(("segParam.s_SId=%d\n", SegParam.s_SId));
	if (hdPart_Service(GET_WRITABLE_FILE, &SegParam) == LOCAL_RET_ERR) {
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "can not find a writeable file. errno = %d\n", errno);
		return LOCAL_RET_ERR;
	}
	
	memset(StrFileName, 0, sizeof(StrFileName));
	LOCK(&g_LockHDPart);
	sprintf(StrFileName, "%s/record%04d%s", DST_PATH_NAME_SD, g_HdPart.s_CurWriteFileNo, AV_FILE_NAME);
	StartOffSet = g_HdPart.s_CurWriteSegNo * MIN_SEG_REC_LEN;
	Retry = 0;
	while(Retry < 3) 
	{
		Fd = open(StrFileName, O_RDWR|DOS_O_CONTIG_CHK, DEFAULT_FILE_MODE);
		if (Fd != LOCAL_RET_ERR) 
		{
			lseek(Fd, StartOffSet, SEEK_SET);
			break;
		}
		else 
		{
			DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "chan open file %s failed, retry = %d\n", StrFileName, Retry);
			usleep(100);
			Retry++;
		}
	}
	
	UNLOCK(&g_LockHDPart);
	return Fd;
}

/*¼�����������ļ�����*/
static void *RecordStreamProcessTask(void *InParam)
{
	int32_t   i;
	int32_t   WrLen1;
	int32_t   WrLen2;
	int32_t   RetVal;
	int32_t   SrcLen1        = 0;
	int32_t   SrcLen2        = 0;
	int32_t   RemSpace       = 0;
	int32_t   CurrFd 	     = -1;
	int32_t   MinRecBufSpace = 0;
	int32_t   MsgLen 	     = sizeof(RecordMsgType) - sizeof(long);
	boolean_t IsChangeFile    = FALSE;			/*Ƭ��¼��ʱ��û�и��������ļ���д����*/
	int32_t   NeedSegNum = 0; 						
	uint32_t  ReadIdx	= 0;
	uint32_t  WriteIdx	= 0;
	uint32_t  RecBufLen;
	uint32_t  StreamLen; 
	time_t	  CurrTime 	= 0;
	time_t	  LastTime 	= 0;
	char_t	  *PRecBufBase;
	int32_t Chan = *(int32_t*)InParam;
	RecordMsgType      RecMsg;
	RecordInfoManage   *PChan;
	SegmentIdxRecord   CurSegRec;
	IFrameInfo         *PIFrames;
	int32_t 		   UpdateTimes = 0;
	printf("Task test %d 00 \n", Chan);
	PChan				  = &(g_VidRecConfig[Chan]);
	PIFrames			  = PChan->s_IFrames;
	PChan->s_RecordStart  = FALSE;	
	PRecBufBase 		  = PChan->s_RecordBuffer.s_Buffer;	
	RecBufLen 		      = PChan->s_RecordBuffer.s_Size;
	printf("Task test 11 \n");
	pthread_detach(pthread_self());
	printf("RecordStreamProcessTask start...\n");
	memset(&CurSegRec, 0, sizeof(CurSegRec));
	FOREVER
	{
		if(PChan->s_RecordMsgId < 0)
		{	
			DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "RecordStreamProcessTask exit.\n");							
			break;
		}
		memset(&RecMsg, 0, sizeof(RecordMsgType));
		RetVal = msgrcv(PChan->s_RecordMsgId, &RecMsg, MsgLen, MSG_TYPE_REC, 0);
		if (RetVal != MsgLen)
		{
			PRT_ERR(("RetVal = %d MsgLen = %d\n",RetVal, MsgLen));
			continue;
		}
		
		switch(RecMsg.s_CmdType)
		{
			case CMD_TYPE_STOP_REC:
				PRT_TEST(("STOP_RECORD.\n"));
				if(!PChan->s_RecordStart)
				{
					PRT_ERR(("can not start record.\n"));
					break;
				}

				if(-1 == CurrFd)
				{
					PRT_ERR(("no opend file.\n"));
					break;
				}
				WriteIdx = PChan->s_RecordBuffer.s_WritePos;
				if (WriteIdx >= ReadIdx) 
				{
					SrcLen1 = WriteIdx - ReadIdx;
					SrcLen2 = 0;
				}
				else
				{
					SrcLen1 = RecBufLen - ReadIdx;
					SrcLen2 = WriteIdx;
				}
				StreamLen = SrcLen1 + SrcLen2;
				
				PRT_TEST(("stop--g_hdPart.m_partStatus = %d,StreamLen = %d, g_hdPart.m_curWriteSegNo=%d, CurSegRec.m_recSegNo=%d, curSegRec.m_recSegLen=%d\n",
					g_HdPart.s_PartStatus,StreamLen, g_HdPart.s_CurWriteSegNo, CurSegRec.s_RecSegNo, CurSegRec.s_RecSegLen));
				if (StreamLen > 0) 
				{
					WrLen1 = MIN(SrcLen1, StreamLen);
					WrLen2 = StreamLen - WrLen1;
					RemSpace = STREAM_FILE_LEN-(g_HdPart.s_CurWriteSegNo%MAX_SEG_PER_FILE)*MIN_SEG_REC_LEN;
					
					/*�ļ�ʣ�ಿ�ֿ�������ֹͣ¼��ʱ��Ƭ�β���*/
					if(StreamLen < RemSpace)
					{							
						if (WrLen1)
						{
						   writeFile(CurrFd, PRecBufBase + ReadIdx, WrLen1);
						}
						if (WrLen2)
						{
							writeFile(CurrFd, PRecBufBase, WrLen2);
						}
						ReadIdx = (ReadIdx + StreamLen) % RecBufLen;
						
					}
					/*Ƭ��¼�񲿷ִ����ļ�ʣ��ռ䣬��Ҫ�������ļ��ռ�*/
					else
					{
						if((NULL != g_HdPart.s_PartStatus)
							&& (g_HdPart.s_BuildFileNums == g_HdPart.s_ToRecordFiles))
						{
							g_HdPart.s_PartStatus = HD_FULL;
							hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
						}
						/*д���ļ�ʣ�ಿ�ֺ��ҵ����ļ���дλ�ü���д����*/
						if(WrLen1 <= RemSpace)
						{
							writeFile(CurrFd, PRecBufBase + ReadIdx, WrLen1);
							ReadIdx = (ReadIdx + WrLen1) % RecBufLen;
							RemSpace = RemSpace - WrLen1;
							if(WrLen2 < RemSpace)
							{
								writeFile(CurrFd, PRecBufBase + ReadIdx, WrLen2);
								ReadIdx = (ReadIdx + WrLen2) % RecBufLen;
							}
							else
							{
								writeFile(CurrFd, PRecBufBase + ReadIdx, RemSpace);
								ReadIdx = (ReadIdx + RemSpace) % RecBufLen;
								/*��cache �е�����д����̣�֪ͨ�ں��ͷ�cache*/
								fdatasync(CurrFd); 				
								posix_fadvise(CurrFd, 0, 0, POSIX_FADV_DONTNEED);						
								close(CurrFd);
								CurrFd = -1;
								
								/*�ҵ����ļ���д������λ��*/
								
								if(LOCAL_RET_ERR == hdPart_Service(UPDATE_RECORD_FILE, &CurSegRec))
								{
									PChan->s_RecordStart = FALSE;
									hdPart_Service(FINALIZE_FILE, &CurSegRec);
									hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
									PRT_ERR(("find file failure.\n"));
									break;
								}
								else
								{
									IsChangeFile = TRUE;
									
								}
								if(LOCAL_RET_ERR == (CurrFd = FindNewFile()))
								{
									PRT_ERR(("open file failure.\n"));
									hdPart_Service(FINALIZE_FILE, &CurSegRec);
									hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
									break;
								}
								else
								{
									/*����¼��Ƭ�β�������ļ��洢*/
									NeedSegNum = ROUND_UP(RemSpace+WrLen1, MIN_SEG_REC_LEN)/MIN_SEG_REC_LEN;
									CurSegRec.s_RecSegLen +=  NeedSegNum;
									CurSegRec.s_RecLastSegLen = (RemSpace+WrLen1)% MIN_SEG_REC_LEN;
									PRT_TEST(("000 new file write data, new record segment, m_recLastSegLen=%d.\n", CurSegRec.s_RecLastSegLen));
									hdPart_Service(FINALIZE_FILE, &CurSegRec);
									if(HD_FULL == g_HdPart.s_PartStatus)
									{
										hdPart_Service(UPDATE_RECORD_FIRST, &CurSegRec);
									}
									hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
									g_HdPart.s_LastWriteTime = CurSegRec.s_EndTime;
									CurSegRec.s_SId = g_HdPart.s_SegRecNo;	
									CurSegRec.s_RecFileNo = g_HdPart.s_CurWriteFileNo;	/*Ƭ��¼��ʼд���ݵ��ļ����*/
									CurSegRec.s_RecSegNo = g_HdPart.s_CurWriteSegNo;
									CurSegRec.s_RecSegLen = 0;
									CurSegRec.s_StartTime = g_HdPart.s_LastWriteTime;
									CurSegRec.s_EndTime = g_HdPart.s_LastWriteTime;
									CurSegRec.s_SegFileInfo = g_SegFileInfo[Chan];
									hdPart_Service(ADD_TABLE_DB, &CurSegRec);
								}
								writeFile(CurrFd, PRecBufBase + ReadIdx, WrLen2 - RemSpace);
								ReadIdx = (ReadIdx + WrLen2-RemSpace) % RecBufLen;
							}
						}
						else
						{
							writeFile(CurrFd, PRecBufBase + ReadIdx, RemSpace);
							ReadIdx = (ReadIdx + RemSpace) % RecBufLen;
							/*��cache �е�����д����̣�֪ͨ�ں��ͷ�cache*/
							fdatasync(CurrFd); 				
							posix_fadvise(CurrFd, 0, 0, POSIX_FADV_DONTNEED);						
							close(CurrFd);
							CurrFd = -1;
							WrLen1 = WrLen1 - RemSpace;
							/*�ҵ����ļ���д������λ��*/
							if(LOCAL_RET_ERR == hdPart_Service(UPDATE_RECORD_FILE, &CurSegRec))
							{
								PChan->s_RecordStart = FALSE;
								hdPart_Service(FINALIZE_FILE, &CurSegRec);
								hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
								PRT_ERR(("find file failure.\n"));
								break;
							}
							else
							{
								IsChangeFile = TRUE;
								
							}
							if(LOCAL_RET_ERR == (CurrFd = FindNewFile()))
							{
								PRT_ERR(("open file failure.\n"));
								hdPart_Service(FINALIZE_FILE, &CurSegRec);
								hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
								break;
							}
							else
							{
								/*����¼��Ƭ�β�������ļ��洢*/
								NeedSegNum = ROUND_UP(RemSpace, MIN_SEG_REC_LEN)/MIN_SEG_REC_LEN;
								CurSegRec.s_RecSegLen +=  NeedSegNum;
								CurSegRec.s_RecLastSegLen = RemSpace % MIN_SEG_REC_LEN;
								PRT_TEST(("new file write data, new record segment, s_recLastSegLen=%d.\n", CurSegRec.s_RecLastSegLen));
								hdPart_Service(FINALIZE_FILE, &CurSegRec);
								if(HD_FULL == g_HdPart.s_PartStatus)
								{
									hdPart_Service(UPDATE_RECORD_FIRST, &CurSegRec);
								}
								hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
								g_HdPart.s_LastWriteTime = CurSegRec.s_EndTime;
								CurSegRec.s_SId = g_HdPart.s_SegRecNo;	
								CurSegRec.s_RecFileNo = g_HdPart.s_CurWriteFileNo;	/*Ƭ��¼��ʼд���ݵ��ļ����*/
								CurSegRec.s_RecSegNo = g_HdPart.s_CurWriteSegNo;
								CurSegRec.s_RecSegLen = 0;
								CurSegRec.s_StartTime = g_HdPart.s_LastWriteTime;
								CurSegRec.s_EndTime = g_HdPart.s_LastWriteTime;
								CurSegRec.s_SegFileInfo = g_SegFileInfo[Chan];
								hdPart_Service(ADD_TABLE_DB, &CurSegRec);
							}

							if (WrLen1)
							{
								writeFile(CurrFd, PRecBufBase + ReadIdx, WrLen1);
								ReadIdx = (ReadIdx + WrLen1) % RecBufLen;
							}
							if (WrLen2)
							{
								writeFile(CurrFd, PRecBufBase + ReadIdx, WrLen2);
								ReadIdx = (ReadIdx + WrLen2) % RecBufLen;
							}
						}
						
					}
					NeedSegNum = ROUND_UP(StreamLen, MIN_SEG_REC_LEN)/MIN_SEG_REC_LEN;
					CurSegRec.s_RecSegLen +=  NeedSegNum;
					CurSegRec.s_RecLastSegLen = StreamLen % MIN_SEG_REC_LEN;
					
					/*���µ����ļ�д¼������ʱ����ǰдƬ��ָ��λ����ǰ�߸����ļ�ʱ���޸ģ��˴������ظ��޸�*/
					if(!IsChangeFile)
					{
						g_HdPart.s_CurWriteSegNo = (g_HdPart.s_CurWriteSegNo + NeedSegNum)%MAX_SEG_PER_FILE;
					}
					
					PRT_TEST(("stop--g_HdPart.s_curWriteFileNo = %d, curSegRec.s_recSegLen=%d,curSegRec.s_recLastSegLen=%d, g_HdPart.s_curWriteSegNo = %d\n",g_HdPart.s_CurWriteFileNo,CurSegRec.s_RecSegLen, CurSegRec.s_RecLastSegLen, g_HdPart.s_CurWriteSegNo));
					hdPart_Service(FINALIZE_FILE, &CurSegRec);
					//hdPart_Service(UPDATE_TABLE_DB, &curSegRec);
					if(HD_FULL == g_HdPart.s_PartStatus)
					{
						hdPart_Service(UPDATE_RECORD_FIRST, &CurSegRec);
					}
					hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
					/*��cache �е�����д����̣�֪ͨ�ں��ͷ�cache*/
					fdatasync(CurrFd); 				
					posix_fadvise(CurrFd, 0, 0, POSIX_FADV_DONTNEED);						
					close(CurrFd);
					CurrFd = -1;
					PChan->s_RecordStart = FALSE;
					IsChangeFile = FALSE;
					memset(&CurSegRec, 0, sizeof(SegmentIdxRecord));
				}
								
				break;
			case CMD_TYPE_START_REC:
				PRT_ERR(("START_RECORD.\n"));
				if (!PChan->s_RecordStart)
				{
					//
					//need add force I frame
					//
					CurSegRec.s_RecType = RecMsg.s_RecordType;					
					LastTime = CurrTime = time(NULL);
					g_HdPart.s_LastWriteTime = CurrTime;
					//����Ƭ��¼���ʱ����Ϊ����д��ʱ��
					if(0 == g_HdPart.s_FirstRecordNo)
					{
						g_HdPart.s_FirstWriteTime = g_HdPart.s_LastWriteTime;
						g_HdPart.s_FirstRecordNo = 1;		//����Ƭ��¼��ļ�¼�Ǵ����1��ʼ(�ļ���¼�Ǵ����0��ʼ)
					}
					PChan->s_CurrRecType	= RecMsg.s_RecordType;
					 /*ÿ�ο�ʼ¼�񣬶���Ҫ���ҿ�д�ļ�*/
					if(LOCAL_RET_ERR == (CurrFd = FindNewFile()))
					{
						PRT_ERR(("find file failure.\n"));
						break;
					}
					PChan->s_RecordStart	= TRUE;
					CurSegRec.s_SId = g_HdPart.s_SegRecNo;
					CurSegRec.s_RecFileNo = g_HdPart.s_CurWriteFileNo;	/*Ƭ��¼��ʼд���ݵ��ļ����*/
					CurSegRec.s_RecSegNo = g_HdPart.s_CurWriteSegNo;
					CurSegRec.s_RecSegLen = 0;
					CurSegRec.s_RecLastSegLen = 0;
					CurSegRec.s_StartTime = CurrTime;
					CurSegRec.s_EndTime = CurrTime;
					CurSegRec.s_SegFileInfo = g_SegFileInfo[Chan];
					if (CurrFd == -1)
					{
						PChan->s_RecordStart = FALSE;
						break;
					}
					hdPart_Service(ADD_TABLE_DB, &CurSegRec);
					UpdateTimes = 0;
				}
				break;
			case CMD_TYPE_REFRESH_I:
				RefreshIFrameInfo(Chan, RecMsg.s_Idx, RecMsg.s_Time);
				break;
			default:
				if(-1 == CurrFd)
				{
					break;
				}
				if(!PChan->s_RecordStart)
				{
					break;
				}		
				
				
				WriteIdx = PChan->s_RecordBuffer.s_WritePos;
				if (WriteIdx >= ReadIdx) 
				{
					SrcLen1 = WriteIdx - ReadIdx;
					SrcLen2 = 0;
				}
				else
				{
					SrcLen1 = RecBufLen - ReadIdx;
					SrcLen2 = WriteIdx;
				}
				/*�����ܳ���ת����512KB�ı�����ʣ�ಿ���´�д��*/
				StreamLen  = ROUND_DOWN(SrcLen1 + SrcLen2, MIN_SEG_REC_LEN);

				/*����������512KB�ı�����ʣ�ಿ���´�д��*/
				CurrTime	 = time(NULL);
				if ((StreamLen < MIN_SEG_REC_LEN)
					&& (CurrTime < (LastTime + UPDATE_SEG_INTERVAL))) 
				{
					break;
				}
				StreamLen = MIN(StreamLen, MIN_SEG_REC_LEN);
				if (StreamLen > 0) 
				{
					WrLen1 = MIN(SrcLen1, StreamLen);
					WrLen2 = StreamLen - WrLen1;
					RemSpace = STREAM_FILE_LEN-(g_HdPart.s_CurWriteSegNo%MAX_SEG_PER_FILE)*MIN_SEG_REC_LEN;
					
					PRT_TEST(("CurrFd = %d, g_HdPart.m_partStatus = %d,StreamLen = %d, RemSpace=%d, WrLen1=%d, WrLen2=%d, g_hdPart.m_curWriteSegNo=%d, CurSegRec.m_recSegNo=%d, CurSegRec.m_recSegLen=%d, CurSegRec.m_recLastSegLen=%d\n",
						CurrFd, g_HdPart.s_PartStatus,StreamLen, RemSpace, WrLen1, WrLen2, g_HdPart.s_CurWriteSegNo, CurSegRec.s_RecSegNo, CurSegRec.s_RecSegLen, CurSegRec.s_RecLastSegLen));
					/*�ļ�ʣ�ಿ�ֿ�������ֹͣ¼��ʱ��Ƭ�β���*/
					if(StreamLen < RemSpace)
					{							
						if (WrLen1)
						{
							writeFile(CurrFd, PRecBufBase + ReadIdx, WrLen1);		
							ReadIdx = (ReadIdx + WrLen1) % RecBufLen;
						}
						if (WrLen2)
						{
							writeFile(CurrFd, PRecBufBase+ReadIdx, WrLen2);
							ReadIdx = (ReadIdx + WrLen2) % RecBufLen;
						}						
					}
					/*Ƭ��¼�񲿷ִ����ļ�ʣ��ռ䣬��Ҫ�������ļ��ռ�*/
					else
					{
						if((HD_FULL != g_HdPart.s_PartStatus) 
							&& (g_HdPart.s_BuildFileNums == g_HdPart.s_ToRecordFiles))
						{
							g_HdPart.s_PartStatus = HD_FULL;					
							hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
						}
						/*д���ļ�ʣ�ಿ�ֺ��ҵ����ļ���дλ�ü���д����*/
						if(WrLen1 <= RemSpace)
						{
							writeFile(CurrFd, PRecBufBase + ReadIdx, WrLen1);
							ReadIdx = (ReadIdx + WrLen1) % RecBufLen;
							RemSpace = RemSpace - WrLen1;
							if(WrLen2 < RemSpace)
							{
								writeFile(CurrFd, PRecBufBase + ReadIdx, WrLen2);
								ReadIdx = (ReadIdx + WrLen2) % RecBufLen;
							}
							else
							{
								writeFile(CurrFd, PRecBufBase + ReadIdx, RemSpace);
								ReadIdx = (ReadIdx + RemSpace) % RecBufLen;
								/*��cache �е�����д����̣�֪ͨ�ں��ͷ�cache*/
								fdatasync(CurrFd); 				
								posix_fadvise(CurrFd, 0, 0, POSIX_FADV_DONTNEED);						
								close(CurrFd);
								PRT_TEST(("000close CurrFd.\n"));
								CurrFd = -1;
								/*�ҵ����ļ���д������λ��*/
								if(LOCAL_RET_ERR == hdPart_Service(UPDATE_RECORD_FILE, &CurSegRec))
								{
									PChan->s_RecordStart = FALSE;
									hdPart_Service(FINALIZE_FILE, &CurSegRec);
									hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
									PRT_ERR(("find file failure.\n"));
									break;
								}
								else
								{
									IsChangeFile = TRUE;
									
								}
								
								if(LOCAL_RET_ERR == (CurrFd = FindNewFile()))
								{
									PRT_ERR(("opne file failure.\n"));
									hdPart_Service(FINALIZE_FILE, &CurSegRec);
									hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
									break;
								}
								else
								{
									/*����¼��Ƭ�β�������ļ��洢*/
									NeedSegNum = ROUND_DOWN(RemSpace+WrLen1, MIN_SEG_REC_LEN)/MIN_SEG_REC_LEN;
									CurSegRec.s_RecSegLen +=  NeedSegNum;
									CurrTime = time(NULL);
									CurSegRec.s_EndTime = CurrTime; 
									PRT_TEST(("new file write data, new record segment.\n"));
									hdPart_Service(FINALIZE_FILE, &CurSegRec);
									if(HD_FULL == g_HdPart.s_PartStatus)
									{
										hdPart_Service(UPDATE_RECORD_FIRST, &CurSegRec);
									}
									hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
									g_HdPart.s_LastWriteTime = CurSegRec.s_EndTime;
									CurSegRec.s_SId = g_HdPart.s_SegRecNo;	
									CurSegRec.s_RecFileNo = g_HdPart.s_CurWriteFileNo;	/*Ƭ��¼��ʼд���ݵ��ļ����*/
									CurSegRec.s_RecSegNo = g_HdPart.s_CurWriteSegNo;
									CurSegRec.s_RecSegLen = 0;
									CurSegRec.s_StartTime = g_HdPart.s_LastWriteTime;
									CurSegRec.s_EndTime = g_HdPart.s_LastWriteTime;
									CurSegRec.s_SegFileInfo = g_SegFileInfo[Chan];
									hdPart_Service(ADD_TABLE_DB, &CurSegRec);
								}
								writeFile(CurrFd, PRecBufBase + ReadIdx, WrLen2 - RemSpace);
								ReadIdx = (ReadIdx + WrLen2-RemSpace) % RecBufLen;
							}
						}
						else
						{
							writeFile(CurrFd, PRecBufBase + ReadIdx, RemSpace);
							ReadIdx = (ReadIdx + RemSpace) % RecBufLen;
							/*��cache �е�����д����̣�֪ͨ�ں��ͷ�cache*/
							fdatasync(CurrFd); 				
							posix_fadvise(CurrFd, 0, 0, POSIX_FADV_DONTNEED);						
							close(CurrFd);
							PRT_TEST(("1111close iCurrFd.\n"));
							CurrFd = -1;
							WrLen1 = WrLen1 - RemSpace;
							/*�ҵ����ļ���д������λ��*/
							if(LOCAL_RET_ERR == hdPart_Service(UPDATE_RECORD_FILE, &CurSegRec))
							{
								PChan->s_RecordStart = FALSE;
								hdPart_Service(FINALIZE_FILE, &CurSegRec);
								hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
								PRT_ERR(("find file failure.\n"));
								break;
							}
							else
							{
								IsChangeFile = TRUE;
								
							}
							if(LOCAL_RET_ERR == (CurrFd = FindNewFile()))
							{
								PRT_ERR(("open file failure.\n"));
								hdPart_Service(FINALIZE_FILE, &CurSegRec);	
								hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
								break;
							}
							else
							{
								/*����¼��Ƭ�β�������ļ��洢�����Ϊ����Ƭ�ηֱ��ڲ�ͬ���ļ��ϴ洢*/
								NeedSegNum = ROUND_DOWN(RemSpace, MIN_SEG_REC_LEN)/MIN_SEG_REC_LEN;
								CurSegRec.s_RecSegLen +=  NeedSegNum;
								PRT_TEST(("new file write data, new record segment.\n"));
								hdPart_Service(FINALIZE_FILE, &CurSegRec);
								if(HD_FULL == g_HdPart.s_PartStatus)
								{
									hdPart_Service(UPDATE_RECORD_FIRST, &CurSegRec);
								}
								hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
								g_HdPart.s_LastWriteTime = CurSegRec.s_EndTime;
								CurSegRec.s_SId = g_HdPart.s_SegRecNo;	
								CurSegRec.s_RecFileNo = g_HdPart.s_CurWriteFileNo;	/*Ƭ��¼��ʼд���ݵ��ļ����*/
								CurSegRec.s_RecSegNo = g_HdPart.s_CurWriteSegNo;
								CurSegRec.s_RecSegLen = 0;
								CurSegRec.s_StartTime = g_HdPart.s_LastWriteTime;
								CurSegRec.s_EndTime = g_HdPart.s_LastWriteTime;
								CurSegRec.s_SegFileInfo = g_SegFileInfo[Chan];
								hdPart_Service(ADD_TABLE_DB, &CurSegRec);
							}
							if (WrLen1)
							{
								writeFile(CurrFd, PRecBufBase + ReadIdx, WrLen1);
								ReadIdx = (ReadIdx + WrLen1) % RecBufLen;
							}
							if (WrLen2)
							{
								writeFile(CurrFd, PRecBufBase + ReadIdx, WrLen2);
								ReadIdx = (ReadIdx + WrLen2) % RecBufLen;
							}
						}
						
					}
					/*��������ת����512KB�ı�����ʣ�ಿ���´�д��*/ 			
					NeedSegNum = ROUND_DOWN(StreamLen, MIN_SEG_REC_LEN)/MIN_SEG_REC_LEN;
					/*����Ϊ�ļ��ﵽ����ֵ�Լ��ر�ʱ���Ÿ���Ƭ��¼�񳤶�*/
					if(TRUE != IsChangeFile)
					{
						CurSegRec.s_RecSegLen +=  NeedSegNum;
					}
					
					/*���µ����ļ�д¼������ʱ����ǰдƬ��ָ��λ����ǰ�߸����ļ�ʱ���޸ģ��˴������ظ��޸�*/
					if(!IsChangeFile)
					{
						g_HdPart.s_CurWriteSegNo = (g_HdPart.s_CurWriteSegNo + NeedSegNum)%MAX_SEG_PER_FILE;
					}
					else
					{
						IsChangeFile = FALSE;
					}

					CurrTime = time(NULL);
					CurSegRec.s_EndTime = CurrTime; 

					/*��cache �е�����д����̣�֪ͨ�ں��ͷ�cache*/
					fdatasync(CurrFd); 				
					posix_fadvise(CurrFd, 0, 0, POSIX_FADV_DONTNEED);						
					LastTime = CurrTime;

					/*�ﵽ����¼���ļ�����ֵʱ��������ǰ��¼���¼����������Ƭ��¼���¼*/
					if(l_MaxSizePerFile <= 0)
					{
						l_MaxSizePerFile = MAX_LEN_RECORD;
					}
					if(CurSegRec.s_RecSegLen*MIN_SEG_REC_LEN >= l_MaxSizePerFile)
					{
						PRT_TEST(("new record segment.\n"));
						hdPart_Service(FINALIZE_FILE, &CurSegRec);
						if(HD_FULL == g_HdPart.s_PartStatus)
						{
							hdPart_Service(UPDATE_RECORD_FIRST, &CurSegRec);
						}
						hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
						g_HdPart.s_LastWriteTime = CurSegRec.s_EndTime;
						CurSegRec.s_SId = g_HdPart.s_SegRecNo;	
						CurSegRec.s_RecFileNo = g_HdPart.s_CurWriteFileNo;	/*Ƭ��¼��ʼд���ݵ��ļ����*/
						CurSegRec.s_RecSegNo = g_HdPart.s_CurWriteSegNo;
						CurSegRec.s_RecSegLen = 0;
						CurSegRec.s_StartTime = g_HdPart.s_LastWriteTime;
						CurSegRec.s_EndTime = g_HdPart.s_LastWriteTime;
						CurSegRec.s_SegFileInfo = g_SegFileInfo[Chan];
						hdPart_Service(ADD_TABLE_DB, &CurSegRec);
					}
					/*����ÿ��д�ļ����ݶ��������ݿ����Ϣ*/
					else if(UpdateTimes++ > TIME_INTERVAL_UPDATE)
					{
						UpdateTimes = 0;
						PRT_TEST(("**g_hdPart.m_curWriteFileNo = %d, curSegRec.m_recSegLen=%d,curSegRec.m_recLastSegLen=%d,g_hdPart.m_curWriteSegNo = %d\n",
							g_HdPart.s_CurWriteFileNo,CurSegRec.s_RecSegLen, CurSegRec.s_RecLastSegLen, g_HdPart.s_CurWriteSegNo));
						if(HD_FULL == g_HdPart.s_PartStatus)
						{
							hdPart_Service(UPDATE_RECORD_FIRST, &CurSegRec);
						}
						hdPart_Service(UPDATE_TABLE_DB, &CurSegRec);
					}				
				}
				break;
		}
		
		if (!PChan->s_RecordStart) {
			/*δ��ʼ¼��ʱ������¼��BUF�������Ϣ*/
			WriteIdx	= PChan->s_RecordBuffer.s_WritePos;
			StreamLen = (RecBufLen - PChan->s_RecordBuffer.s_ReadPos + WriteIdx) % RecBufLen;
			/* ����I֡���棬ֻ����uiRecBufLen-iMinRecBufSpace������, iMinRecBufSpace������ֹ���*/
			while ((StreamLen + MinRecBufSpace) >= RecBufLen) {
				if (PChan->s_IFrameNums > 1) {
					PChan->s_IFrameNums--;
					for (i=0; i < PChan->s_IFrameNums; i++) {
						PIFrames[i].s_Idx  = PIFrames[i+1].s_Idx;
						PIFrames[i].s_Time	 = PIFrames[i+1].s_Time;
					}
					PChan->s_RecordBuffer.s_ReadPos = PIFrames[0].s_Idx;
				}else {
					PChan->s_IFrameNums = 0;
					PChan->s_RecordBuffer.s_ReadPos = WriteIdx;
				}
				StreamLen = (RecBufLen - PChan->s_RecordBuffer.s_ReadPos + WriteIdx) % RecBufLen;
			}
			continue;
		}
		
	}
	
	printf("RecordStreamProcessTask stop00...\n");
	pthread_exit(NULL);
	
	printf("RecordStreamProcessTask stop...\n");
}


int32_t CreateRecProcessThread(int32_t Chan)
{
    pthread_t ScheduleTid, StreamTId;
    int32_t RetVal;

	if(1 == l_RecProcessThreadcreated)
	{
        DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "RecordProcess thread has created.");
		return LOCAL_RET_OK;
	}
    RetVal = pthread_create(&ScheduleTid, NULL, RecordScheduleProcessTask, NULL);
    if (0 != RetVal)
    {
        DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "RecordScheduleProcess thread create error.");
        return LOCAL_RET_ERR;
    }
	printf("123\n");
	l_ChannelNum = Chan;
    RetVal = pthread_create(&StreamTId, NULL, RecordStreamProcessTask, &l_ChannelNum);
    if (0 != RetVal)
    {
        DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "RecordStreamProcess thread create error.");
        return LOCAL_RET_ERR;
    }
	l_RecProcessThreadcreated = 1;
	printf("345\n");

    return LOCAL_RET_OK;
}


/*���ݿ��ļ������߳�*/
static void *RecordDbBackupTask(void *InParam)
{
	int32_t CycleNum = 60;
	pthread_detach(pthread_self());
	
	printf("RecordDbBackupTask start...\n");
	while(1)
	{
		g_DbStartBackup = TRUE;
		backupMainDb(g_DbFd);
		if((g_DbBackupCount++) > 65535)
		{
			g_DbBackupCount = 0;
		}
		g_DbStartBackup = FALSE;

		while(CycleNum--)
		{
			if(0 == l_IsStartDbProcessTask)
			{	
				DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "RecordDbBackupTask exit.\n");							
				goto ErrExit;
			}
			sleep(1);
		}
		CycleNum = 60;
	}
ErrExit:
	
	printf("RecordDbBackupTask stop000...\n");
	pthread_exit(NULL);
	printf("RecordDbBackupTask stop...\n");
}

/*�쳣�����߳�*/
static void *ExceptionProcTask(void *InParam)
{
	int oldBackupOkCount = 0;
	int exceptionNum = 0;
	int32_t CycleNum = 6;
	
	pthread_detach(pthread_self());
	printf("ExceptionProcTask start...\n");
	while(1)
	{
		/*ͨ����ʱ����ʱ���쳣���������쳣*/
		if(g_DbStartBackup)
		{
			if(oldBackupOkCount == g_DbBackupCount)
			{
				exceptionNum++;
			}
			else
			{
				oldBackupOkCount = g_DbBackupCount;
				exceptionNum = 0;
			}
		}
		else
		{
			oldBackupOkCount = g_DbBackupCount;
			exceptionNum = 0;
		}

		if(exceptionNum > 10)
		{
			exceptionNum = 0;
			g_HdPart.s_PartStatus = HD_ERROR;
			DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "[ALARM]disk lost or error");
		}
		
		while(CycleNum--)
		{
			if(0 == l_IsStartDbProcessTask)
			{	
				DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "ExceptionProcTask exit.\n");							
				goto ErrExit;
			}
			sleep(1);
		}
		CycleNum = 6;
	}
ErrExit:
	
	printf("ExceptionProcTask stop000...\n");
	pthread_exit(NULL);
	
	printf("ExceptionProcTask stop...\n");
}

int32_t CreateDbProcessThread(void)
{
    pthread_t MainDbTid, BakDbTId;
    int32_t RetVal;

	l_IsStartDbProcessTask = 1;
	if(1 == l_DbProcessThreadcreated)
	{
        DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "RecordProcess thread has created.");
		return LOCAL_RET_OK;
	}

    RetVal = pthread_create(&MainDbTid, NULL, RecordDbBackupTask, NULL);
    if (0 != RetVal)
    {
        DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "RecordScheduleProcess thread create error.");
		l_IsStartDbProcessTask = 0;
        return LOCAL_RET_ERR;
    }
	
    RetVal = pthread_create(&BakDbTId, NULL, ExceptionProcTask, NULL);
    if (0 != RetVal)
    {
        DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "RecordStreamProcess thread create error.");
		l_IsStartDbProcessTask = 0;
        return LOCAL_RET_ERR;
    }
	l_DbProcessThreadcreated = 1;

    return LOCAL_RET_OK;
}


/*��ȡ����״̬*/
int32_t GetSDStatus(StorageStatusQueryResOut **QueryResPtr, 
	                     uint32_t QueryArrayNum,uint32_t  *QueryResNum)
{
	int32_t Status = 0;
	int32_t TotalSpace = 0;
	int32_t FreeSpace = 0;
	if((NULL == QueryResPtr)
		|| (NULL == QueryResNum)
		|| (NULL == *QueryResPtr)
		|| (0 == QueryArrayNum))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");
		return LOCAL_RET_ERR;
	}

	(*QueryResPtr)[0].s_StorageDiskNo = 1;
	(*QueryResPtr)[0].s_DiskStatus = TYPE_STORAGE_SD;
	(*QueryResPtr)[0].s_DiskAttribute = ATTR_DISK_WR;

	if(LOCAL_RET_OK == IsNoFormat())
	{
		Status = STATUS_DISK_NOFORMAT;
	}
	else
	{
		switch(g_HdPart.s_PartStatus)
		{
			case HD_ERROR:
				Status = STATUS_DISK_ERR;
				break;
			case HD_FULL:
				Status = STATUS_DISK_FULL;
				break;
			case HD_NORMAL:
			default:
				Status = STATUS_DISK_NORMAL;
				break;
		}
		(*QueryResPtr)[0].s_DiskFormatRate = 100;
	}

	if(LOCAL_RET_OK != GetDiskSpace(&TotalSpace, &FreeSpace))
	{
		Status = STATUS_DISK_NOFORMAT;
	}
	(*QueryResPtr)[0].s_DiskStatus = Status;
	(*QueryResPtr)[0].s_TotalSpaceVolume = TotalSpace;
	(*QueryResPtr)[0].s_FreeSpaceVolume = FreeSpace;
	*QueryResNum = 1;

	return LOCAL_RET_OK;
}


/*����¼�����*/
int32_t SetRecConfigParam(RecordParamConfigIn *InParam)
{
	if(NULL == InParam)
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");
		return LOCAL_RET_ERR;
	}

	memcpy(&g_RecRefenceParam, InParam, sizeof(g_RecRefenceParam));
	g_RecParamConfig.s_PreRecordTime = g_RecRefenceParam.s_PreRecordTime;
	g_RecParamConfig.s_RecDelayTime = g_RecRefenceParam.s_DelayRecordTime;
	g_RecParamConfig.s_IsCyclicRecord = ((g_RecRefenceParam.s_IsCirculerRec==1)?FALSE:TRUE);

	return LOCAL_RET_OK;
}

/*����¼��ƻ�*/
int32_t SetRecScheduleConfig(RecordScheduleConfigIn *InParam)
{
	int32_t i,j;
	if(NULL == InParam)
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");
		return LOCAL_RET_ERR;
	}

	g_RecParamConfig.s_IsEnableRecord = InParam->s_Enable;
	PRT_TEST(("g_RecParamConfig.s_IsEnableRecord=%d\n", g_RecParamConfig.s_IsEnableRecord));
	for(i=0; i<7; i++)
	{
		for(j=0; j<4; j++)
		{
			g_RecParamConfig.s_RecordParam[i][j].s_RecordTimeSeg.s_StartTime = InParam->s_RecStartTime[i][j];
			g_RecParamConfig.s_RecordParam[i][j].s_RecordTimeSeg.s_StopTime = InParam->s_RecEndTime[i][j];
			if((0 == InParam->s_RecStartTime[i][j]) && ((24<<16) == InParam->s_RecEndTime[i][j]))
			{
				g_RecParamConfig.s_AllDayRecParam[i].s_IsAllDayRecord = FLAG_ALLDAY_REC_OPEN;
				break;
			}
			else
			{
				g_RecParamConfig.s_AllDayRecParam[i].s_IsAllDayRecord = FLAG_ALLDAY_REC_CLOSE;
			}
		}
	}

	return LOCAL_RET_OK;
}	

int32_t  RecordCtrlNotify(RecordCtrlIn *InParam)
{
	int32_t  Chan = 0;
	uint32_t TmpInfo = 0;
	int32_t  TmpResolutionIdx = 0;
	if((NULL == InParam)
		|| (InParam->s_RecTrigChan >= MAX_ENCODER_NUM)
		|| ((InParam->s_AudioFrame <= 0) && (InParam->s_AudioFrame > 127))
		|| ((InParam->s_AudioType< 0) && (InParam->s_AudioType > 3))
		|| ((InParam->s_VideoFrame <= 0) && (InParam->s_AudioFrame > 127))
		|| ((InParam->s_VideoWide <= 0) && (InParam->s_VideoWide > RESOLUTION_1080P_WIDTH))
		|| ((InParam->s_VideoHeight <= 0) && (InParam->s_VideoHeight > RESOLUTION_1080P_HEIGHT))
		|| ((InParam->s_EncodeType < 0) && (InParam->s_EncodeType > 2))
	   )
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam error.\n");
		return LOCAL_RET_ERR;
	}

	memcpy(&g_RecDataInfo, InParam, sizeof(RecordCtrlIn));

	Chan = InParam->s_RecTrigChan;
	TmpInfo = (InParam->s_AudioFrame) & 0xFF;
	TmpInfo = (TmpInfo | (InParam->s_VideoFrame << 8)) & 0xFFFF;
	if((RESOLUTION_1080P_WIDTH == InParam->s_VideoWide) 
		&& (RESOLUTION_1080P_HEIGHT== InParam->s_VideoHeight))
	{
		TmpResolutionIdx = 1;
	}
	else if((RESOLUTION_720P_WIDTH == InParam->s_VideoWide) 
		&& (RESOLUTION_720P_HEIGHT== InParam->s_VideoHeight))
	{
		TmpResolutionIdx = 2;
	}
	else if((RESOLUTION_576P_WIDTH == InParam->s_VideoWide) 
		&& (RESOLUTION_576P_HEIGHT== InParam->s_VideoHeight))
	{
		TmpResolutionIdx = 3;
	}
	else if((RESOLUTION_D1_WIDTH == InParam->s_VideoWide) 
		&& (RESOLUTION_D1_HEIGHT== InParam->s_VideoHeight))
	{
		TmpResolutionIdx = 4;
	}
	else if((RESOLUTION_480P_WIDTH == InParam->s_VideoWide) 
		&& (RESOLUTION_480P_HEIGHT== InParam->s_VideoHeight))
	{
		TmpResolutionIdx = 5;
	}
	else if((RESOLUTION_CIF_WIDTH == InParam->s_VideoWide) 
		&& (RESOLUTION_CIF_HEIGHT== InParam->s_VideoHeight))
	{
		TmpResolutionIdx = 6;
	}
	TmpInfo = (TmpInfo | (TmpResolutionIdx << 16)) & 0xFFFFF;

	TmpInfo = (TmpInfo | (InParam->s_AudioType << 20)) & 0xFFFFFF;
	TmpInfo = (TmpInfo | (InParam->s_EncodeType << 24)) & 0x0FFFFFFF;

	g_SegFileInfo[Chan] = TmpInfo;


	switch(InParam->s_RecTrigMode)
	{
		case TYPE_REC_TRIG_MANU:
		case TYPE_REC_TRIG_MOTION:
		case TYPE_REC_TRIG_ALARM:
			DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "s_RecTrigMode %d no support.\n", InParam->s_RecTrigMode);
		    return LOCAL_RET_ERR;
			break;
		case TYPE_REC_TRIG_TIME:
		default:
			break;
	}

	return LOCAL_RET_OK;
}


/*�����ϲ�ͳһ�ϴ�¼���ļ�������*/
static void MakeRecFileName(int32_t RecType,  SegmentIdxRecord *PQueryResult, int32_t QueryResultSize, RecordFileQueryResOut **RecordFileQueryResPtr, uint32_t QueryResArraySize)
{
	int32_t i;
	char_t StrFileName[64];
	int32_t StrLen = 0;
	int32_t RecordSize = 0;

	if((NULL == PQueryResult) 
		|| (NULL == RecordFileQueryResPtr)
		|| (NULL == *RecordFileQueryResPtr)
		|| (NUM_RECORD_MAX < QueryResultSize)
		|| (NUM_RECORD_MAX < QueryResArraySize))
	{
		PRT_ERR(("mkRecFileName param error.\n"));
		return;
	}

	for(i=0; i<QueryResultSize; i++)
	{
		memset(StrFileName, 0, sizeof(StrFileName));
		StrLen = sprintf(StrFileName, "record_%02d_%04d_%04d_%d_%d", PQueryResult[i].s_RecEncodeNo, PQueryResult[i].s_RecFileNo,
			    PQueryResult[i].s_RecSegNo, PQueryResult[i].s_StartTime, PQueryResult[i].s_EndTime);
		RecordFileQueryResPtr[i]->s_RecTrigMode = RecType;
		memcpy(RecordFileQueryResPtr[i]->s_RecFileName, StrFileName, MIN(StrLen, MAX_LEN_FILE_REC));
		/*Ƭ��¼������һ��Ƭ�δص�ʵ��ʹ�õĴ�С����¼�񳤶ȼ��㷽ʽ*/
		if((PQueryResult[i].s_RecLastSegLen <= 0) || (PQueryResult[i].s_RecLastSegLen >= MIN_SEG_REC_LEN))
		{
			RecordSize = MIN_SEG_REC_LEN * PQueryResult[i].s_RecSegLen;
		}
		else
		{
			if(PQueryResult[i].s_RecSegLen > 0)
			{
				RecordSize = MIN_SEG_REC_LEN * (PQueryResult[i].s_RecSegLen-1) + PQueryResult[i].s_RecLastSegLen;
			}
			else
			{
				RecordSize = PQueryResult[i].s_RecLastSegLen;
			}
		}
		RecordFileQueryResPtr[i]->s_RecFileSize = RecordSize;
		RecordFileQueryResPtr[i]->s_RecFileTime[0] = PQueryResult[i].s_StartTime;
		RecordFileQueryResPtr[i]->s_RecFileTime[1] = PQueryResult[i].s_EndTime;
		if(i == QueryResArraySize-1)
		{
			break;
		}
	}
	
}


/* ����¼���ļ���¼*/
static int SearchRecordFiles(RecordFileQueryIn *RecordFileQueryPtr, uint32_t *CurQueryPosNo, 
                                 RecordFileQueryResOut **RecordFileQueryResPtr, uint32_t QueryResArraySize,
                                 uint32_t  *QueryResTotalNum, uint32_t  *QueryResCurNum)
{

	printf("111111\n");
	if((RecordFileQueryPtr->s_Channel >=  MAX_ENCODER_NUM)
		|| (RecordFileQueryPtr->s_RecQueryTime[0] >=  RecordFileQueryPtr->s_RecQueryTime[1])
		|| (RecordFileQueryPtr->s_RecQueryType > TYPE_REC_QUERY_ALL))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");
		return LOCAL_RET_ERR;
	}
	printf("2222\n");

	int32_t RetResult = LOCAL_RET_OK;
	SegmentIdxRecord SegParam;
	SegmentIdxRecord *PQueryResult = NULL;
	char_t **QueryResult;
	int32_t i = 0;
	int32_t RowRes = 0;
	int32_t RecordTotalNum = 0;
	int32_t StartQueryNo = 0;
	int32_t RealRecordNum = 0;

	memset(&SegParam, 0, sizeof(SegmentIdxRecord));
	PQueryResult = (SegmentIdxRecord *)malloc(NUM_RECORD_MAX * sizeof(SegmentIdxRecord));
	if(NULL == PQueryResult)
	{
		PRT_ERR(("pQueryResult malloc error, please reboot.\n"));
		RetResult = LOCAL_RET_ERR;
		goto ErrExit;
	}
	QueryResult = (char**)malloc(NUM_RECORD_MAX * sizeof(char *));
	memset(QueryResult, 0, NUM_RECORD_MAX * sizeof(char *));
	for(i = 0; i < NUM_RECORD_MAX; i++)
	{
		QueryResult[i] = (char *)malloc(sizeof(SegmentIdxRecord) + sizeof(char));
		if(NULL == QueryResult[i])
		{
			PRT_ERR(("queryResult[i] malloc error, please reboot.\n"));
			RetResult = LOCAL_RET_ERR;
			goto ErrExit;
		}
		memset(QueryResult[i], 0, sizeof(SegmentIdxRecord) + sizeof(char));
	}

	if(NULL != g_DbFd)
	{
		do
		{
			RowRes = 0;
			memset(&SegParam, 0, sizeof(SegParam));
			if(0 < *CurQueryPosNo)
			{
				StartQueryNo = *CurQueryPosNo+1;
			}
			SegParam.s_SId = StartQueryNo; 					//��ʼ��ѯ�ļ�¼��

			if(TYPE_REC_QUERY_TIME == RecordFileQueryPtr->s_RecQueryType)
			{
				SegParam.s_RecType = TYPE_REC_TIME;
			}
			else if(TYPE_REC_QUERY_TRIG == RecordFileQueryPtr->s_RecQueryType)
			{
				if((RecordFileQueryPtr->s_RecTrigMode)&0x04)
				{
					SegParam.s_RecType = TYPE_REC_MOTION;
				}
				else if((RecordFileQueryPtr->s_RecTrigMode)&0x08)
				{
					SegParam.s_RecType = TYPE_REC_ALARM;
				}
				else
				{
					SegParam.s_RecType = TYPE_REC_TIME;
				}
			}

			SegParam.s_StartTime = RecordFileQueryPtr->s_RecQueryTime[0];
			SegParam.s_EndTime = RecordFileQueryPtr->s_RecQueryTime[1];
			
			/*ȫ��¼��*/
			if(TYPE_REC_TIME == SegParam.s_RecType)
			{
				LOCK(&g_LockHDPart);
				queryDbRecord(g_DbFd,RECORD_QUERY_SEG,METHOD_QUERY_TIME,(char_t*)&SegParam,QueryResult, NUM_RECORD_MAX, &RowRes);
				UNLOCK(&g_LockHDPart);
			}
			/*һ��¼��*/
			else if(TYPE_REC_TIME != SegParam.s_RecType)
			{
				LOCK(&g_LockHDPart);
				queryDbRecord(g_DbFd,RECORD_QUERY_SEG,METHOD_QUERY_MIX,(char_t*)&SegParam,QueryResult, NUM_RECORD_MAX, &RowRes);
				UNLOCK(&g_LockHDPart);
			}
			else
			{
				PRT_ERR(("query record recType %d error.\n", SegParam.s_RecType));
				RetResult = LOCAL_RET_ERR;
				goto ErrExit;
			}

			RealRecordNum = MIN(RowRes, NUM_RECORD_MAX);
				
			if((NULL != QueryResult[0]) && (RealRecordNum > 0))
			{
				for(i = 0; i < RealRecordNum; i++)
				{
					memset(&SegParam, 0, sizeof(SegParam));
					memcpy(&SegParam, QueryResult[i], sizeof(SegmentIdxRecord));
					memcpy(&(PQueryResult[i]), &SegParam, sizeof(SegmentIdxRecord));
				}	
				StartQueryNo = PQueryResult[RealRecordNum-1].s_SId;
			}
			RecordTotalNum = RowRes;
			MakeRecFileName(RecordFileQueryPtr->s_RecTrigMode, PQueryResult, RealRecordNum, RecordFileQueryResPtr, QueryResArraySize);
			PRT_TEST(("select over,recordTotalNum = %d, RealRecordNum = %d\n", RecordTotalNum, RealRecordNum));
			
		}while(0);
	}

	*QueryResTotalNum = RecordTotalNum;
	*QueryResCurNum = RealRecordNum;
	*CurQueryPosNo = StartQueryNo;
ErrExit:
	if(NULL != PQueryResult)
	{
		free(PQueryResult);
		PQueryResult = NULL;
	}
	for(i = 0; i < NUM_RECORD_MAX; i++)
	{
		if(NULL != QueryResult[i])
		{
			free(QueryResult[i]);
			QueryResult[i] = NULL;
		}
	}
	if(NULL != QueryResult)
	{
		free(QueryResult);
		QueryResult = NULL;
	}
	
    return RetResult;
}



int32_t QueryRecordFile(RecordFileQueryIn *RecordFileQueryPtr, uint32_t *CurQueryPosNo, 
                            RecordFileQueryResOut **RecordFileQueryResPtr, uint32_t QueryResArraySize,
                            uint32_t  *QueryResTotalNum, uint32_t  *QueryResCurNum)
{
	if((NULL == RecordFileQueryPtr)
		|| (NULL == CurQueryPosNo)
		|| (NULL == RecordFileQueryResPtr)
		|| (NULL == *RecordFileQueryResPtr)
		|| (NULL == QueryResTotalNum)
		|| (NULL == QueryResCurNum))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");
		return LOCAL_RET_ERR;
	}

	printf("RecordFileQueryPtr->s_RecQueryType = %d\n", RecordFileQueryPtr->s_RecQueryType);
	switch(RecordFileQueryPtr->s_RecQueryType)
	{
		case TYPE_REC_QUERY_TRIG:
			break;
		case TYPE_REC_QUERY_ALL:
			break;
		case TYPE_REC_QUERY_TIME:
		default:
			SearchRecordFiles(RecordFileQueryPtr, CurQueryPosNo, RecordFileQueryResPtr, 
				QueryResArraySize, QueryResTotalNum, QueryResCurNum);
			break;
	}

	return LOCAL_RET_OK;
		
}

/*Filename:record_XX(ͨ����)_XXXX(�ļ���)_XXXX(Ƭ�κ�)_XXXXXXXXXX(��ʼʱ��)_XXXXXXXXXX(��ֹʱ��)*/
static int32_t FindDownFileInfo(char_t *FileName, RecordDownReplayQueryResOut **RecordDownReplayQueryResPtr)
{
	uint32_t StartTime = 0;
	uint32_t EndTime = 0;
	char_t  *CurPos = NULL;
	SegmentIdxRecord SegParam;
	char_t **QueryResult;
	int32_t RowRes = 0;
	int32_t RetVal = LOCAL_RET_OK;
	int32_t RecordSize = 0;
	char_t StrName[MAX_LEN_FILE_REC];
	int32_t Width = 0;
	int32_t Height = 0;
	
	if((NULL == FileName)
		|| (41 > strlen(FileName))
		|| (NULL == RecordDownReplayQueryResPtr)
		|| (NULL == *RecordDownReplayQueryResPtr))
	{
		PRT_ERR(("FindDownFileInfo  InParam error.\n"));
		return LOCAL_RET_ERR;
	}

	CurPos = FileName+20; //ƫ������ʼʱ��
	StartTime = atoi(CurPos);
	CurPos = FileName+31; //ƫ������ֹʱ��
	EndTime = atoi(CurPos);

	QueryResult = (char**)malloc(1 * sizeof(char *));
	memset(QueryResult, 0, 1 * sizeof(char *));
	QueryResult[0] = (char *)malloc(sizeof(SegmentIdxRecord) + sizeof(char));
	if(NULL == QueryResult[0])
	{
		PRT_ERR(("queryResult[0] malloc error, please reboot.\n"));
		return LOCAL_RET_ERR;
	}
	memset(QueryResult[0], 0, sizeof(SegmentIdxRecord) + sizeof(char));

	do
	{
		memset(&SegParam, 0, sizeof(SegParam));
		SegParam.s_SId = 0;		
		SegParam.s_RecType = TYPE_REC_TIME;
		SegParam.s_StartTime = StartTime;
		SegParam.s_EndTime = EndTime;
		queryDbRecord(g_DbFd,RECORD_QUERY_SEG,METHOD_QUERY_TIME,(char_t*)&SegParam,QueryResult, 1, &RowRes);

		if(RowRes < 0)
		{
			RetVal = LOCAL_RET_ERR;
			break;
		}
		
		memset(&SegParam, 0, sizeof(SegParam));
		memcpy(&SegParam, QueryResult[0], sizeof(SegmentIdxRecord));
		
		(*RecordDownReplayQueryResPtr)[0].s_RecQueryType = OPERATE_RECORD_DOWN;
		(*RecordDownReplayQueryResPtr)[0].s_RecFileOffset = SegParam.s_RecSegNo*MIN_SEG_REC_LEN;
		
		/*Ƭ��¼������һ��Ƭ�δص�ʵ��ʹ�õĴ�С����¼�񳤶ȼ��㷽ʽ*/
		if((SegParam.s_RecLastSegLen <= 0) || (SegParam.s_RecLastSegLen >= MIN_SEG_REC_LEN))
		{
			RecordSize = MIN_SEG_REC_LEN * SegParam.s_RecSegLen;
		}
		else
		{
			if(SegParam.s_RecSegLen > 0)
			{
				RecordSize = MIN_SEG_REC_LEN * (SegParam.s_RecSegLen-1) + SegParam.s_RecLastSegLen;
			}
			else
			{
				RecordSize = SegParam.s_RecLastSegLen;
			}
		}
		(*RecordDownReplayQueryResPtr)[0].s_RecFileSize = RecordSize;
		memset(StrName, 0, sizeof(StrName));
	    sprintf(StrName, "%s/record%04d%s", DST_PATH_NAME_SD, SegParam.s_RecFileNo, AV_FILE_NAME);
		memcpy((*RecordDownReplayQueryResPtr)[0].s_RecFileName, StrName, strlen(StrName));
		(*RecordDownReplayQueryResPtr)[0].s_StreamType = 0;
		(*RecordDownReplayQueryResPtr)[0].s_EncodeType = (SegParam.s_SegFileInfo >> 24)&0x0F;
		(*RecordDownReplayQueryResPtr)[0].s_AudioType = (SegParam.s_SegFileInfo >> 20)&0x0F;
		(*RecordDownReplayQueryResPtr)[0].s_VideoFrame = (SegParam.s_SegFileInfo >> 8)&0xFF;
		(*RecordDownReplayQueryResPtr)[0].s_AudioFrame = (SegParam.s_SegFileInfo)&0xFF;

		switch(((SegParam.s_SegFileInfo >> 16)&0x0F))
		{
			case 1:
				Width = RESOLUTION_1080P_WIDTH;
				Height = RESOLUTION_1080P_HEIGHT;
				break;
			case 2:
				Width = RESOLUTION_720P_WIDTH;
				Height = RESOLUTION_720P_HEIGHT;
				break;
			case 3:
				Width = RESOLUTION_576P_WIDTH;
				Height = RESOLUTION_576P_HEIGHT;
				break;
			case 4:
				Width = RESOLUTION_D1_WIDTH;
				Height = RESOLUTION_D1_HEIGHT;
				break;
			case 5:
				Width = RESOLUTION_480P_WIDTH;
				Height = RESOLUTION_480P_HEIGHT;
				break;
			case 6:
				Width = RESOLUTION_CIF_WIDTH;
				Height = RESOLUTION_CIF_HEIGHT;
				break;
			default:
				break;
		}
		
		(*RecordDownReplayQueryResPtr)[0].s_VideoWide = Width;
		(*RecordDownReplayQueryResPtr)[0].s_VideoHeight = Height;
		(*RecordDownReplayQueryResPtr)[0].s_RecFileReplayNum = 0;
		(*RecordDownReplayQueryResPtr)[0].s_RecFileCurNo = 0;
		(*RecordDownReplayQueryResPtr)[0].s_CurFileStartTime = SegParam.s_StartTime;
		(*RecordDownReplayQueryResPtr)[0].s_CurFileEndTime = SegParam.s_EndTime;
	}while(0);

	if(NULL != QueryResult)
	{
		if(NULL != QueryResult[0])
		{
			free(QueryResult[0]);
			QueryResult[0] = NULL;
		}
		free(QueryResult);
		QueryResult = NULL;
	}
	return RetVal;
}

int32_t QueryDownReplayRecordFile(RecordDownReplayQueryIn *RecordDownReplayQueryPtr,
       RecordDownReplayQueryResOut **RecordDownReplayQueryResPtr, uint32_t QueryResArraySize)
{
	if((NULL == RecordDownReplayQueryPtr)
		|| ((OPERATE_RECORD_DOWN != RecordDownReplayQueryPtr->s_RecQueryType)
		&& (OPERATE_RECORD_REPLAY != RecordDownReplayQueryPtr->s_RecQueryType))
		|| (NULL == RecordDownReplayQueryResPtr)
		|| (NULL == *RecordDownReplayQueryResPtr)
		|| (0 >= QueryResArraySize))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");
		return LOCAL_RET_ERR;
	}

	switch(RecordDownReplayQueryPtr->s_RecQueryType)
	{
		case OPERATE_RECORD_REPLAY:
			break;
		case OPERATE_RECORD_DOWN:
	    default:
			if(LOCAL_RET_OK != FindDownFileInfo(RecordDownReplayQueryPtr->RecDownReplayInfo.s_RecFileName, RecordDownReplayQueryResPtr))
			{
				return LOCAL_RET_ERR;
			}
			break;
	}
	
	return LOCAL_RET_OK;
}

