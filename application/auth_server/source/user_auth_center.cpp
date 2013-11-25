
#include "user_auth_center.h"
#include "user_auth_api.h"
#include "log_record.h"

UserSessionId UserAuthentication::m_UserSessionId[MAX_USER_LINK_NUM] = {};
UserLinkNum UserAuthentication::m_UserLinkNum[MAX_USER_ACCOUNT_NUM] = {};
uint32_t UserAuthentication::m_AllUserLinkTotalNum = 0;
uint32_t UserAuthentication::m_AllLoginUserTotalNum = 0;
uint16_t UserAuthentication::m_SessionIdSeed = 0;

UserAuthentication::UserAuthentication(void)
{
	m_RdWrLock = NULL;
}

UserAuthentication::~UserAuthentication(void)
{
    	
}


GMI_RESULT UserAuthentication::Initialize(pthread_rwlock_t * PLock)
{	
	if(NULL != PLock)
	{	
		m_RdWrLock = PLock;
	}
	return GMI_TMP_SUCCESS;
}

GMI_RESULT UserAuthentication::Deinitialize(void)
{
	return GMI_TMP_SUCCESS;
}


GMI_RESULT UserAuthentication::GetUserInfoFormFile(uint32_t *OutUserNum, UserAccountInfo *OutUserInfo)
{
    FILE*             FileHandle       = NULL;
    uint32_t          TmpUserNum       = 0;
	char_t            TmpStr[512];
	UserAccountInfo   TmpUserData;
	UserAccountInfo   *TmpUserInfoPtr = NULL;

	if((NULL == OutUserNum)
		|| (NULL == OutUserInfo)
		)
	{
		return GMI_TMP_FAIL;
	}

	TmpUserInfoPtr = OutUserInfo;

	FileHandle = fopen(USER_INFO_FILE_PATH, "r");

	if(NULL == FileHandle)
	{
		fprintf(stderr, "%s-%d: userinfo file open failed \n", __FILE__, __LINE__);
		return GMI_TMP_FAIL;
	}

	for(;;)
	{
        memset(TmpStr, 0 ,sizeof(TmpStr));
		memset(&TmpUserData, 0, sizeof(TmpUserData));

        if(NULL == fgets(TmpStr, sizeof(TmpStr), FileHandle))
        {
        	break;
        }
		else
		{
			if(0 < strlen(TmpStr))
			{
		    	sscanf(TmpStr, "%s %s %u", TmpUserData.s_Username, TmpUserData.s_Password, &(TmpUserData.s_AuthValue));
				if(0 < strlen(TmpUserData.s_Username))
				{
					if(TmpUserNum >= MAX_USER_ACCOUNT_NUM)
					{
						break;
					}
					memcpy(TmpUserInfoPtr, &TmpUserData, sizeof(TmpUserData));
					PRT_TEST(("TmpStr=%s\n", TmpStr));
					PRT_TEST(("s_Username=%s\n", TmpUserData.s_Username));
					PRT_TEST(("s_Password=%s\n", TmpUserData.s_Password));
					PRT_TEST(("s_AuthValue=%d\n", TmpUserData.s_AuthValue));
					TmpUserNum++;
					TmpUserInfoPtr++;
					
				}
			}
			
		}
	}
	

	*OutUserNum = TmpUserNum;

	if(NULL != FileHandle)
	{
		fclose(FileHandle);
		FileHandle = NULL;
	}

	return GMI_TMP_SUCCESS;
}

GMI_RESULT UserAuthentication::CheckSessionIdUnique(const uint16_t InSessionId)
{
	int32_t i = 0;
	
	int32_t IsUnique = 1;

	if(0 == InSessionId)
	{
		return GMI_TMP_FAIL;
	}

	for(i=0; i<MAX_USER_LINK_NUM; i++)
	{
		if(m_UserSessionId[i].s_SessionId == InSessionId)
		{
			IsUnique = 0;
			break;
		}
	}

	if(0 == IsUnique)
	{
		return GMI_TMP_FAIL;
	}

	return GMI_TMP_SUCCESS;
	
}


GMI_RESULT UserAuthentication::SetUserSessionIdRecord(const uint16_t InSessionId, char_t *InOutUserName, const uint32_t MoudleId, const uint32_t ModifyFlag)
{
	int32_t i = 0;
	
	if((NULL == InOutUserName)
		|| (0 == InSessionId)
		|| ((GMI_RECORD_DEL != ModifyFlag) && (GMI_RECORD_ADD != ModifyFlag))
		)
    {
    	return GMI_TMP_FAIL;
    }

	if(GMI_RECORD_DEL == ModifyFlag)
	{
		for(i=0; i<MAX_USER_LINK_NUM; i++)
		{
			if(m_UserSessionId[i].s_SessionId == InSessionId)
			{
				memcpy(InOutUserName, m_UserSessionId[i].s_Username, strlen(m_UserSessionId[i].s_Username));
				memset(m_UserSessionId[i].s_Username, 0, sizeof(m_UserSessionId[i].s_Username));
				m_UserSessionId[i].s_SessionId = 0;
				m_UserSessionId[i].s_MoudleId = 0;
				break;
			}
		}
	}
	else
	{
		for(i=0; i<MAX_USER_LINK_NUM; i++)
		{
			if((0 == m_UserSessionId[i].s_SessionId)
				&&('\0' == m_UserSessionId[i].s_Username[0])
				)
			{
				memcpy(m_UserSessionId[i].s_Username, InOutUserName, strlen(InOutUserName));
				m_UserSessionId[i].s_SessionId = InSessionId;
				m_UserSessionId[i].s_MoudleId = MoudleId;
				break;
			}
		}
	}

	if(MAX_USER_LINK_NUM <= i)
	{
		return GMI_TMP_FAIL;
	}
	PRT_TEST(("\n********222SetUserSessionIdRecord2222*******\n"));
	PRT_TEST(("username   sessionId   moudleId\n"));
	for(i=0; i<MAX_USER_LINK_NUM; i++)
	{
		if(0 != m_UserSessionId[i].s_SessionId)
		{
			PRT_TEST(("%s       %d       %d\n", m_UserSessionId[i].s_Username, m_UserSessionId[i].s_SessionId, m_UserSessionId[i].s_MoudleId));
		}
	}
	PRT_TEST(("***********************\n\n"));

	return GMI_TMP_SUCCESS;
}

GMI_RESULT UserAuthentication::GetUserSessionIdRecord(UserSessionId **OutRecord)
{
	if((NULL == OutRecord) || (NULL == *OutRecord))
	{
		return GMI_TMP_FAIL;
	}

	memcpy(*OutRecord, m_UserSessionId, sizeof(UserSessionId)*MAX_USER_LINK_NUM);

	return GMI_TMP_SUCCESS;
}

GMI_RESULT UserAuthentication::GetUserLinkNumRecord(UserLinkNum *OutRecord)
{
	if(NULL == OutRecord)
	{
		return GMI_TMP_FAIL;
	}

	memcpy(OutRecord, m_UserLinkNum, sizeof(UserLinkNum)*MAX_USER_ACCOUNT_NUM);

	return GMI_TMP_SUCCESS;
}




GMI_RESULT UserAuthentication::SetUserLinkNumRecord(const char_t *InUserName, const uint32_t ModifyFlag)
{
	int32_t i = 0;
	int32_t IsNeedFindRecord = 1;
	
	if((NULL == InUserName)
		|| ((GMI_RECORD_DEL != ModifyFlag) && (GMI_RECORD_ADD != ModifyFlag))
		)
    {
    	return GMI_TMP_FAIL;
    }


	for(i=0; i<MAX_USER_ACCOUNT_NUM; i++)
	{
		if((strlen(InUserName) == strlen(m_UserLinkNum[i].s_Username))
			&& (0 == memcmp(m_UserLinkNum[i].s_Username, InUserName, strlen(InUserName))))
		{
			if(GMI_RECORD_DEL == ModifyFlag)
			{
				if(m_UserLinkNum[i].s_LinkNum > 0)
				{
					m_UserLinkNum[i].s_LinkNum--;
					if(0 < m_AllUserLinkTotalNum)
					{
						m_AllUserLinkTotalNum--;
					}
				}

				if(0 == m_UserLinkNum[i].s_LinkNum)
				{
					memset(m_UserLinkNum[i].s_Username, 0, sizeof(m_UserLinkNum[i].s_Username));
					if(0 < m_AllLoginUserTotalNum)
					{
						m_AllLoginUserTotalNum--;
					}
				}
			}
			else
			{
				m_UserLinkNum[i].s_LinkNum++;
				if(MAX_USER_LINK_NUM > m_AllUserLinkTotalNum)
				{
					m_AllUserLinkTotalNum++;
				}
				
			}
			IsNeedFindRecord = 0;
			break;
		}
	}

	//add new record
	if(1 == IsNeedFindRecord)
	{
		if(GMI_RECORD_ADD == ModifyFlag)
		{
			if(MAX_USER_ACCOUNT_NUM > m_AllLoginUserTotalNum)
			{
				for(i=0; i<MAX_USER_ACCOUNT_NUM; i++)
				{
					if('\0' == m_UserLinkNum[i].s_Username[0])
					{
						memcpy(m_UserLinkNum[i].s_Username, InUserName, strlen(InUserName));
						m_UserLinkNum[i].s_LinkNum = 1;
						
						break;
					}
				}
				if(MAX_USER_ACCOUNT_NUM > i)
				{
					m_AllLoginUserTotalNum++;
					if(MAX_USER_LINK_NUM > m_AllUserLinkTotalNum)
					{
						m_AllUserLinkTotalNum++;
					}
				}
				else
				{
					return GMI_TMP_FAIL;
				}
			}
			else
			{
				return GMI_TMP_FAIL;
			}
		}
		else
		{
			return GMI_TMP_FAIL;
		}
	}

	PRT_TEST(("IsNeedFindRecord=%d, InUserName=%s, m_AllLoginUserTotalNum=%d, m_AllUserLinkTotalNum=%d\n", 
		IsNeedFindRecord, InUserName, m_AllLoginUserTotalNum, m_AllUserLinkTotalNum));	
	PRT_TEST(("\n********2222SetUserLinkNumRecord222*******\n"));
	PRT_TEST(("username   linknum\n"));
	for(i=0; i<MAX_USER_ACCOUNT_NUM; i++)
	{
		if('\0' != m_UserLinkNum[i].s_Username[0])
		{
			PRT_TEST(("%s      %d\n", m_UserLinkNum[i].s_Username, m_UserLinkNum[i].s_LinkNum));
		}
	}
	PRT_TEST(("***********************\n\n"));

	return GMI_TMP_SUCCESS;
	
}


GMI_RESULT UserAuthentication::CheckUserNameValid(const char_t *InUserName, const uint16_t InUserNameEncType, char_t *OutPassword, uint32_t *OutAuthValue)
{
	#ifndef TYPE_DATABASE_MANAGE_USEINFO
	UserAccountInfo UserAccountData[MAX_USER_ACCOUNT_NUM];
	uint32_t i = 0;
	uint32_t UserAccountNum = 0;
	int32_t ResValue = GMI_TMP_SUCCESS;
	#endif

	//no support encrypted username
	if((0 != InUserNameEncType)
		|| (NULL == InUserName)
		|| (NULL == OutPassword)
		|| (NULL == OutAuthValue))
	{
		return GMI_TMP_FAIL;
	}

	#ifndef TYPE_DATABASE_MANAGE_USEINFO
	for(i=0; i<MAX_USER_ACCOUNT_NUM; i++)
	{
		memset(&(UserAccountData[i]), 0, sizeof(UserAccountData[i]));
	}
	#endif

    PRT_TEST(("*********CheckUserNameValid***********\n"));
	PRT_TEST(("InUserName = %s, InUserNameEncType=%d\n", InUserName, InUserNameEncType));

	#ifndef TYPE_DATABASE_MANAGE_USEINFO
	pthread_rwlock_rdlock(m_RdWrLock);
	if(GMI_TMP_FAIL == GetUserInfoFormFile(&UserAccountNum, UserAccountData))
	{
		pthread_rwlock_unlock(m_RdWrLock);
		return GMI_TMP_FAIL;
	}
	pthread_rwlock_unlock(m_RdWrLock);

	PRT_TEST(("UserAccountNum=%d\n", UserAccountNum));

	for(i=0; i<UserAccountNum; i++)
	{
		PRT_TEST(("UserAccountData[%d].s_Username=%s\n", i, UserAccountData[i].s_Username));
		if((strlen(InUserName) == strlen(UserAccountData[i].s_Username))
			&& (0 == memcmp(UserAccountData[i].s_Username, InUserName, strlen(InUserName))))
		{
			memcpy(OutPassword, UserAccountData[i].s_Password, strlen(UserAccountData[i].s_Password));
			*OutAuthValue = UserAccountData[i].s_AuthValue;
			break;
		}
	}

	if(i >= UserAccountNum)
	{
		return GMI_TMP_FAIL;
	}
	#else
	int32_t ResValue = GMI_TMP_SUCCESS;
	sqlite3 *DbHd = NULL;
	int32_t RetValue = 0;
	char_t *ErrMsg = NULL;
	char_t **DbResult;
	int32_t Row = 0;
	int32_t Column = 0;
	int32_t Index;
	const char_t *SelectStr = "select * from %s where %s = '%s'";	
	char_t TmpQueryStr[256];
	PRT_TEST(("*********sqlite3_open***********\n"));

	RetValue = sqlite3_open(GMI_USERS_FILE_NAME, &DbHd);
	if(SQLITE_OK != RetValue)
	{
		fprintf(stderr, "sqlite3_open error\n");
		return GMI_FAIL_OPENFILE;
	}
	PRT_TEST(("*********sqlite3_open end***********\n"));

	if(strlen(InUserName) <= 0)
	{
		fprintf(stderr, "InUserName len error\n");
		ResValue = GMI_FAIL_NOUSER;
	}
	else
	{
		memset(TmpQueryStr, 0, sizeof(TmpQueryStr));
		sprintf(TmpQueryStr, SelectStr, GMI_USERS_TABLE_NAME, GMI_USERS_TABLE_FIELD_NAME, InUserName);
		PRT_TEST(("*********sqlite3_get_table start***********\n"));

		RetValue = sqlite3_get_table(DbHd, TmpQueryStr, &DbResult, &Row, &Column, &ErrMsg);
		
		PRT_TEST(("*********sqlite3_get_table end***********\n"));
		if(SQLITE_OK != RetValue)
		{
			fprintf(stderr, "sqlite3_get_table %s error\n", ErrMsg);
			ResValue = GMI_FAIL_NOUSER;
		}
		else
		{			
			Index = Column;
			if((Row > 0) && (Column > 4))
			{
				PRT_TEST(( "sqlite3_get_table %s %s %s %s %s\n", InUserName, DbResult[1], DbResult[2], DbResult[3], DbResult[4]));
				PRT_TEST(( "sqlite3_get_table %s %s %s %s %s\n", InUserName, DbResult[Index+1], DbResult[Index+2], DbResult[Index+3], DbResult[Index+4]));
			}
			
			if(((Row > 0) && (Column > 4))
				&& (NULL != DbResult[Index+1])
				&& (strlen(InUserName) == strlen(DbResult[Index+1]))
				&& (0 == memcmp(DbResult[Index+1], InUserName, strlen(InUserName))))
			{
				if(0 < strlen(DbResult[Index+2]))
				{
					memcpy(OutPassword, DbResult[Index+2], strlen(DbResult[Index+2]));
				}
				if(0 < strlen(DbResult[Index+3]))
				{
					//role+level
					*OutAuthValue = ((atoi(DbResult[Index+3])<<24) & 0xFF000000) | (atoi(DbResult[Index+4]) & 0x00FFFFFF);
				}
			}
			else
			{
				ResValue = GMI_FAIL_NOUSER;
			}
		}
		sqlite3_free_table(DbResult);

	}
	
	if(NULL != DbHd)
	{
		sqlite3_close(DbHd);
		DbHd = NULL;
	}	
	#endif
	
	PRT_TEST(("******************\n"));
	return ResValue;
	
	
}



GMI_RESULT UserAuthentication::CheckUserLinkNumValid(const char_t *InUserName, const uint16_t InSingleUserMaxLinkNum, const uint16_t InAllUserMaxLinkNum)
{
	int32_t i = 0;
	int32_t CurUserLinkNum = 0;

	if((NULL == InUserName)
		|| (0 == InSingleUserMaxLinkNum)
		|| (0 == InAllUserMaxLinkNum))
	{
		return GMI_TMP_FAIL;
	}

	//compare linking numbers between current linking numbers and total linking numbers of all users
	if((m_AllUserLinkTotalNum >= InAllUserMaxLinkNum) 
		|| (m_AllUserLinkTotalNum >= MAX_USER_LINK_NUM))
	{
		DEBUG_LOG_TMP(&LogClientHdTmp, e_DebugLogLevel_Exception, "all user link %d over Max value\n", m_AllUserLinkTotalNum);
		return GMI_FAIL_ALLLINK;
	}

	for(i=0; i<MAX_USER_ACCOUNT_NUM; i++)
	{
		if((strlen(InUserName) == strlen(m_UserLinkNum[i].s_Username))
			&& (0 == memcmp(m_UserLinkNum[i].s_Username, InUserName, strlen(InUserName))))
		{
			CurUserLinkNum = m_UserLinkNum[i].s_LinkNum;
			break;
		}
	}

	//compare linking numbers between current linking numbers and total linking numbers of single user
	if(CurUserLinkNum >= InSingleUserMaxLinkNum)
	{
		DEBUG_LOG_TMP(&LogClientHdTmp, e_DebugLogLevel_Exception, "user %s link %d over Max value\n", InUserName, CurUserLinkNum);
		return GMI_FAIL_SINGLELINK;
	}

	PRT_TEST(("*******CheckUserLinkNumValid********\n"));
	PRT_TEST(("m_AllUserLinkTotalNum=%d\n",m_AllUserLinkTotalNum));
	PRT_TEST(("InSingleUserMaxLinkNum=%d\n",InSingleUserMaxLinkNum));
	PRT_TEST(("InAllUserMaxLinkNum=%d\n",InAllUserMaxLinkNum));
	PRT_TEST(("CurUserLinkNum=%d\n",CurUserLinkNum));
	PRT_TEST(("***********************************\n"));

	return GMI_TMP_SUCCESS;

	
}


GMI_RESULT UserAuthentication::CheckSessionIdValid(const uint16_t InSessionId, const char_t *InUserName)
{
	int32_t i = 0;
	int32_t IsUserValid = 0;
	if(0 == InSessionId)
	{
		return GMI_TMP_FAIL;
	}
	
	if(GMI_TMP_FAIL == CheckSessionIdUnique(InSessionId))
    {
		for(i=0; i<MAX_USER_LINK_NUM; i++)
		{
			if(m_UserSessionId[i].s_SessionId == InSessionId)
			{
				if((strlen(InUserName) == strlen(m_UserSessionId[i].s_Username))
					&& (0 == memcmp(InUserName, m_UserSessionId[i].s_Username, strlen(InUserName))))
				{
					IsUserValid = 1;
				}
				break;
			}
		}
	
    	if(1 == IsUserValid)
    	{
            return GMI_TMP_SUCCESS;
    	}
		else
		{
		    return GMI_TMP_FAIL;
		}
    }

	return GMI_TMP_FAIL;
}



GMI_RESULT UserAuthentication::CreateSessionId(uint16_t *OutSessionId)
{
	uint16_t LoopNum = 0;
	uint16_t SessionId = 0;

	if(NULL == OutSessionId)
	{
		return GMI_TMP_FAIL;
	}

	for(;;)
	{
        m_SessionIdSeed++;
		if(MAX_NUM_SESSIONID < m_SessionIdSeed)
		{
			m_SessionIdSeed = 1;
		}

		SessionId = m_SessionIdSeed;

		if(GMI_TMP_SUCCESS == CheckSessionIdUnique(SessionId))
		{
			break;
		}
		
        LoopNum++;
		if(MAX_NUM_SESSIONID < LoopNum)
		{
			break;
		}
	}

	if(MAX_NUM_SESSIONID < LoopNum)
	{
	    return GMI_TMP_FAIL;
	}
	else
	{
        *OutSessionId = SessionId;
	}

	return GMI_TMP_SUCCESS;
}

GMI_RESULT UserAuthentication::ClearRecordInfo(uint32_t MoudleId)
{
	int i = 0, j = 0;
	switch(MoudleId)
	{
		case ID_MOUDLE_REST_ALL:
			memset(m_UserLinkNum, 0, sizeof(UserLinkNum)*MAX_USER_ACCOUNT_NUM);
			memset(m_UserSessionId, 0, sizeof(UserSessionId)*MAX_USER_LINK_NUM);
			m_AllLoginUserTotalNum = 0;
			m_AllUserLinkTotalNum = 0;
			break;
		case ID_MOUDLE_REST_SDK:
		case ID_MOUDLE_REST_ONVIF:
		case ID_MOUDLE_REST_GB:
		case ID_MOUDLE_REST_WEB:
		case ID_MOUDLE_REST_RTSP:
			for(i=0; i<MAX_USER_LINK_NUM; i++)
			{
				if(MoudleId == m_UserSessionId[i].s_MoudleId)
				{
					if(m_AllUserLinkTotalNum > 0)
					{
						m_AllUserLinkTotalNum--;
					}
					for(j=0; j<MAX_USER_ACCOUNT_NUM; j++)
					{
						if((strlen(m_UserSessionId[i].s_Username) == strlen(m_UserLinkNum[j].s_Username))
					    && (0 == memcmp(m_UserSessionId[i].s_Username, m_UserLinkNum[j].s_Username, strlen(m_UserLinkNum[j].s_Username))))
						{
							if(m_UserLinkNum[j].s_LinkNum > 0)
							{
								m_UserLinkNum[j].s_LinkNum--;
							}

							if(0 == m_UserLinkNum[j].s_LinkNum)
							{
								if(m_AllLoginUserTotalNum > 0)
								{
									m_AllLoginUserTotalNum--;
								}
								memset(&m_UserLinkNum[j], 0, sizeof(UserLinkNum));
							}
						}
					}		
					memset(&(m_UserSessionId[i]), 0, sizeof(UserSessionId));
				}
			}
			break;
		default:
			DEBUG_LOG_TMP(&LogClientHdTmp, e_DebugLogLevel_Exception, "MoudleId %x error.", MoudleId);
			break;
	}

	PRT_TEST(("MoudleId=%d, m_AllLoginUserTotalNum=%d, m_AllUserLinkTotalNum=%d\n", 
		MoudleId, m_AllLoginUserTotalNum, m_AllUserLinkTotalNum));
	PRT_TEST(("**********username-linknum*************\n"));
	PRT_TEST(("username   linknum\n"));
	for(i=0; i<MAX_USER_ACCOUNT_NUM; i++)
	{
		if('\0' != m_UserLinkNum[i].s_Username[0])
		{
			PRT_TEST(("%s      %d\n", m_UserLinkNum[i].s_Username, m_UserLinkNum[i].s_LinkNum));
		}
	}
	PRT_TEST(("***********************\n\n"));
	PRT_TEST(("**********username-sessionId*************\n"));
	PRT_TEST(("username   sessionId   moudleId\n"));
	for(i=0; i<MAX_USER_LINK_NUM; i++)
	{
		if(0 != m_UserSessionId[i].s_SessionId)
		{
			PRT_TEST(("%s       %d       %d\n", m_UserSessionId[i].s_Username, m_UserSessionId[i].s_SessionId, m_UserSessionId[i].s_MoudleId));
		}
	}
	PRT_TEST(("***********************\n\n"));
	
	return GMI_TMP_SUCCESS;
}

