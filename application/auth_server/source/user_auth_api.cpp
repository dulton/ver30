
#include "user_auth_api.h"
#include "user_auth_center.h"
#include "gmi_rudp_api.h"
#include "md5.h"
#include "des.h"
#include <sys/shm.h>
#include <pthread.h>
#include "ipc_fw_v3.x_resource.h"
#include "ipc_fw_v3.x_setting.h"
#include "gmi_daemon_heartbeat_api.h"
#include "log_record.h"
#ifdef LOG_SERVER_OK
#include "application_packet.h"
#include "share_memory_log_client.h"
#include "gmi_config_api.h"


ShareMemoryLogClient *LogClientHdTmp = NULL;
#endif

#define  TMP_MAX_ALLUSERS_LINKNUM    32
#define  TMP_MAX_SINGLEUSER_LINKNUM  32


#define ID_MOUDLE_REST_SDK          0x1   //sdk server reset linknum and sessionId
#define ID_MOUDLE_REST_ONVIF        0x2  //onvif server
#define ID_MOUDLE_REST_GB           0x3   //GB
#define ID_MOUDLE_REST_WEB          0x4   //WEB
#define ID_MOUDLE_REST_CONFIG       0x5   //CONFIG TOOL
#define ID_MOUDLE_REST_RTSP         0x6   //rtsp server
#define ID_MOUDLE_REST_ALL          0xF   //all login info of linknums and sessionId

char_t g_MoudleStrMsg[64];

static char_t *MoudleIdToStr(int32_t MoudleId)
{
	memset(g_MoudleStrMsg, 0, sizeof(g_MoudleStrMsg));
	switch(MoudleId)
	{
		case ID_MOUDLE_REST_SDK:
			sprintf(g_MoudleStrMsg, "%s", "MOUDLE_SDK");
			break;
		case ID_MOUDLE_REST_ONVIF:
			sprintf(g_MoudleStrMsg, "%s", "MOUDLE_ONVIF");
			break;
		case ID_MOUDLE_REST_GB:
			sprintf(g_MoudleStrMsg, "%s", "MOUDLE_GB");
			break;
		case ID_MOUDLE_REST_WEB:
			sprintf(g_MoudleStrMsg, "%s", "MOUDLE_WEB");
			break;
		case ID_MOUDLE_REST_CONFIG:
			sprintf(g_MoudleStrMsg, "%s", "MOUDLE_CONFIG");
			break;
		case ID_MOUDLE_REST_RTSP:
			sprintf(g_MoudleStrMsg, "%s", "MOUDLE_RTSP");
			break;
		case ID_MOUDLE_REST_ALL:
			sprintf(g_MoudleStrMsg, "%s", "MOUDLE_ALL");
			break;
		default:
			sprintf(g_MoudleStrMsg, "%s", "MOUDLE_UNKNOW");
			break;
	}
	return g_MoudleStrMsg;
}


GMI_RESULT CalcMd5Value(const char_t *InBuf, const uint32_t InBufLen, char_t *OutBuf)
{
	MD5_CTX TmpCtx;
	uint32_t i = 0;
	char_t *TmpPtr = NULL;

	if((NULL == InBuf)
		|| (0 == InBufLen)
		|| (NULL == OutBuf))
	{
		return GMI_TMP_FAIL;
	}
	
    MD5Init(&TmpCtx, 0);
    MD5Update( &TmpCtx, (unsigned char*)InBuf, InBufLen);
    MD5Final(&TmpCtx);
	
	TmpPtr = OutBuf;
	
	for(i=0; i<sizeof(TmpCtx.digest); i++)
	{
		sprintf(TmpPtr, "%02x", TmpCtx.digest[i]);
		TmpPtr += 2;
	}
	PRT_TEST(("Password MD5 Result: %s\n", OutBuf));

	return GMI_TMP_SUCCESS;
}

GMI_RESULT CalcDesEncValue(char_t *InBuf, uint32_t InBufLen, char_t *InKeyBuf, char_t *OutBuf, int32_t *OutBufLen)
{	
	if((NULL == InBuf)
		|| (0 == InBufLen)
		|| (LEN_DES_PASSWORD < InBufLen)
		|| (NULL == InKeyBuf)
		|| (NULL == OutBuf)
		|| (NULL == OutBufLen))
	{
		DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]InParam error.");
		return GMI_TMP_FAIL;
	}	

	#if 0
	int32_t i = 0;
	printf("plainpassword %s key %s\n", InBuf, InKeyBuf);
	for(i = 0; i < (LEN_DES_PASSWORD-InBufLen); i++)
	{
		InBuf[InBufLen+i] = 0x00;
	}
	#endif
	InBufLen = LEN_DES_PASSWORD;
	if(0 > DES_Encrypt(InBuf, InBufLen, InKeyBuf, OutBuf, OutBufLen))
	{
		DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]DES_Encrypt error.");
		return GMI_TMP_FAIL;
	}

	#if 0
	//printf("CalcDesEncValue %s\n", OutBuf);
	int i=0;
	for(i=0; i<LEN_DES_PASSWORD; i++)
	{
		printf("%02x", OutBuf[i]);
	}
	printf("\n"); 
	printf("OutBufLen=%d\n", *OutBufLen);

	
	char plainText[128] = {0};
	int plainTextLen = 0;
	if(0 <= DES_Decrypt(OutBuf, *OutBufLen, InKeyBuf, plainText, &plainTextLen))
	{
		printf("DES_Decrypt Result %s\n", plainText);
		for(i=0; i<plainTextLen; i++)
		{
			printf("%02x", plainText[i]);
		}
		printf("\n"); 
		
	}
	#endif
	
	return GMI_TMP_SUCCESS;
}


GMI_RESULT UserAuthReset(uint32_t MoudleId)
{
	UserAuthentication UserAuthObject;
	
	DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Info, "[AUTH]MoudleId %s Reset.", MoudleIdToStr(MoudleId));
	if(GMI_TMP_FAIL == UserAuthObject.ClearRecordInfo(MoudleId))
	{
		DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception,"[AUTH]ClearRecordInfo(Id-%s) error.", MoudleIdToStr(MoudleId));
		return GMI_TMP_FAIL;
	}
	return GMI_TMP_SUCCESS;
}


GMI_RESULT UserAuthCheck(UserAuthRefInfo *UserAuthInputData, UserAuthResInfo *UserAuthOutputData, pthread_rwlock_t *pRdWrLock)
{
	UserAuthentication UserAuthObject;
	char_t TmpPassword[MAX_LEN_PASSWORD];
	char_t TmpEncPassword[MAX_LEN_PASSWORD];
	int32_t TmpEncPasswordLen = 0;
	uint32_t TmpAuthValue = 0;
	uint16_t TmpSessionId = 0;
	int32_t RetValue = -1;
	int32_t SessionValid = 0;
	char_t TmpCheckString[512];
	int32_t TmpStrLen = 0;
	char_t TmpMd5Str1[MAX_LEN_PASSWORD];
	char_t TmpMd5Str2[MAX_LEN_PASSWORD];
	UserAuthExtInfo TmpUserAuthExtInfo;
	//int32_t i = 0;
	

	if((NULL == UserAuthInputData)
		|| (NULL == UserAuthOutputData)
		|| (NULL == pRdWrLock))
	{
		DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception,"[AUTH]inParam NULL.");
		return GMI_CODE_ERR_PARAM;
	}	

	if(('\0' == UserAuthInputData->s_Username[0])
		//|| ('\0' == UserAuthInputData->s_Password[0])
		|| ((0 < UserAuthInputData->s_UserAuthExtDataLen) && (NULL == UserAuthInputData->s_UserAuthExtData))
		//|| (0 == UserAuthInputData->s_SingleUserMaxLinkNum)
		//|| (0 == UserAuthInputData->s_AllUserMaxLinkNum)
		|| ((TYPE_AUTH_LOGIN != UserAuthInputData->s_DataType) && (TYPE_AUTH_LOGOUT != UserAuthInputData->s_DataType)))
	{
		DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception,"[AUTH]inParam(Id-%s) error.", MoudleIdToStr(UserAuthInputData->s_MoudleId));
		return GMI_CODE_ERR_PARAM;
	}

	if(GMI_TMP_FAIL == UserAuthObject.Initialize(pRdWrLock))
	{	
		DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception,"[AUTH]UserAuthObject Initialize error.");
		return GMI_CODE_ERR_INTER;
	}

	//check validness of username
	memset(TmpPassword, 0, sizeof(TmpPassword));
	if(GMI_TMP_SUCCESS != UserAuthObject.CheckUserNameValid(UserAuthInputData->s_Username, UserAuthInputData->s_UsernameEncType, TmpPassword, &TmpAuthValue))
	{
		DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception,"[AUTH]CheckUserNameValid(Id-%s) %s error.", MoudleIdToStr(UserAuthInputData->s_MoudleId), UserAuthInputData->s_Username);
		return GMI_CODE_ERR_USER;
	}
	else
	{
	    UserAuthOutputData->s_AuthValue = TmpAuthValue;
	}

	if('\0' == TmpPassword[0])
	{
		DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]query password(Id-%s) error.", MoudleIdToStr(UserAuthInputData->s_MoudleId));
		return GMI_CODE_ERR_PASSWORD;
	}

	//check validness of password
	memset(TmpEncPassword, 0, sizeof(TmpEncPassword));
	PRT_TEST(("s_PasswordEncType=%d\n", UserAuthInputData->s_PasswordEncType));
	switch(UserAuthInputData->s_PasswordEncType)
	{
		case TYPE_ENCRYPTION_TEXT:
			memcpy(TmpEncPassword, TmpPassword, strlen(TmpPassword));
			break;
			
		case TYPE_ENCRYPTION_MD5:
			//when password is text
			#if 0
			if(GMI_TMP_FAIL == CalcMd5Value(TmpPassword, strlen(TmpPassword), TmpEncPassword))
			{
				ERR_PRINT("TYPE_ENCRYPTION_MD5 CalcMd5Value error.\n");
				return GMI_CODE_ERR_PARAM;
			}
			#else
			memcpy(TmpEncPassword, TmpPassword, strlen(TmpPassword));
			#endif
		    break;	
		case TYPE_ENCRYPTION_MSCHAP:
			PRT_TEST(("TYPE_ENCRYPTION_CUSTOM0 000\n"));
			if((0 == UserAuthInputData->s_UserAuthExtDataLen) 
				|| (NULL == UserAuthInputData->s_UserAuthExtData)
				|| (128 < UserAuthInputData->s_UserAuthExtDataLen))
			{
				PRT_TEST(("TYPE_ENCRYPTION_CUSTOM0 01001\n"));
				ERR_PRINT("TYPE_ENCRYPTION_CUSTOM0 inParam error.\n");
				return GMI_CODE_ERR_PARAM;
			}
			PRT_TEST(("TYPE_ENCRYPTION_CUSTOM0 111\n"));

			memset(TmpCheckString, 0, sizeof(TmpCheckString));
			TmpStrLen = sprintf(TmpCheckString, "%s", TmpPassword);
			PRT_TEST(("TYPE_ENCRYPTION_CUSTOM0 222\n"));
			memcpy(TmpCheckString+TmpStrLen, UserAuthInputData->s_UserAuthExtData, UserAuthInputData->s_UserAuthExtDataLen);
			PRT_TEST(("TmpCheckString=%s\n", TmpCheckString));
			PRT_TEST(("TYPE_ENCRYPTION_CUSTOM0 333\n"));
			if(GMI_TMP_FAIL == CalcMd5Value(TmpCheckString, strlen(TmpCheckString), TmpEncPassword))
			{
				DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception,"[AUTH]TYPE_ENCRYPTION_CUSTOM0 CalcMd5Value error.");
				return GMI_CODE_ERR_PARAM;
			}
			break;
		case TYPE_ENCRYPTION_DES:
			if((0 == UserAuthInputData->s_UserAuthExtDataLen) 
				|| (NULL == UserAuthInputData->s_UserAuthExtData)
				|| (8 < UserAuthInputData->s_UserAuthExtDataLen))
			{
				return GMI_CODE_ERR_PARAM;
			}

			CalcDesEncValue(TmpPassword, strlen(TmpPassword), UserAuthInputData->s_UserAuthExtData, TmpEncPassword, &TmpEncPasswordLen);
			
			break;
		case TYPE_ENCRYPTION_MD5_RTSP:
			//md5(md5(<username>:<realm>:<password>):<nonce>:md5(<cmd>:<url>))
			if((0 == UserAuthInputData->s_UserAuthExtDataLen) 
				|| (sizeof(UserAuthExtInfo) > UserAuthInputData->s_UserAuthExtDataLen)
				|| (NULL == UserAuthInputData->s_UserAuthExtData)
				|| (sizeof(UserAuthExtInfo) > UserAuthInputData->s_UserAuthExtDataLen))
			{
				ERR_PRINT("TYPE_ENCRYPTION_MD5_RTSP inParam error.\n");
				return GMI_CODE_ERR_PARAM;
			}
			memset(&TmpUserAuthExtInfo, 0, sizeof(TmpUserAuthExtInfo));
			memcpy(&TmpUserAuthExtInfo, UserAuthInputData->s_UserAuthExtData, sizeof(UserAuthExtInfo));
			
			memset(TmpCheckString, 0, sizeof(TmpCheckString));
			sprintf(TmpCheckString, "%s:%s:%s", UserAuthInputData->s_Username, TmpUserAuthExtInfo.s_Realm, TmpPassword);
			if(GMI_TMP_FAIL == CalcMd5Value(TmpCheckString, strlen(TmpCheckString), TmpMd5Str1))
			{
				DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception,"[AUTH]TYPE_ENCRYPTION_MD5_RTSP0 CalcMd5Value error.");
				return GMI_CODE_ERR_PARAM;
			}

			memset(TmpCheckString, 0, sizeof(TmpCheckString));
			sprintf(TmpCheckString, "%s:%s", TmpUserAuthExtInfo.s_Cmd, TmpUserAuthExtInfo.s_Url);
			if(GMI_TMP_FAIL == CalcMd5Value(TmpCheckString, strlen(TmpCheckString), TmpMd5Str2))
			{
				DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception,"[AUTH]TYPE_ENCRYPTION_MD5_RTSP1 CalcMd5Value error.");
				return GMI_CODE_ERR_PARAM;
			}

			memset(TmpCheckString, 0, sizeof(TmpCheckString));
			sprintf(TmpCheckString, "%s:%s:%s", TmpMd5Str1, TmpUserAuthExtInfo.s_Nonce, TmpMd5Str2);
			if(GMI_TMP_FAIL == CalcMd5Value(TmpCheckString, strlen(TmpCheckString), TmpEncPassword))
			{
				DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception,"[AUTH]TYPE_ENCRYPTION_MD5_RTSP2 CalcMd5Value error.");
				return GMI_CODE_ERR_PARAM;
			}
			break;
		default:
			memcpy(TmpEncPassword, TmpPassword, strlen(TmpPassword));
			break;
	}

	if(TYPE_ENCRYPTION_DES != UserAuthInputData->s_PasswordEncType)
	{
		
		PRT_TEST(("TmpEncPassword=%s, s_Password=%s\n", TmpEncPassword, UserAuthInputData->s_Password));
		if( (strlen(TmpEncPassword) != strlen(UserAuthInputData->s_Password))
			|| (0 != memcmp(TmpEncPassword, UserAuthInputData->s_Password, strlen(UserAuthInputData->s_Password))))
		{
			DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception,"[AUTH]check password(Id-%s) error.", MoudleIdToStr(UserAuthInputData->s_MoudleId));
			return GMI_CODE_ERR_PASSWORD;
		}
	}
	else
	{
		#if 0
		printf("s_Password = ");
		for(i=0; i<LEN_DES_PASSWORD; i++)
		{
			printf("%02x", UserAuthInputData->s_Password[i]);
		}
		printf("\n");
		printf("TmpEncPassword = ");
		for(i=0; i<LEN_DES_PASSWORD; i++)
		{
			printf("%02x", TmpEncPassword[i]);
		}
		printf("\n");
		#endif
		#if 0
		DEBUG_LOG_TMP(&LogClientHdTmp, e_DebugLogLevel_Exception, "input Password = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x.", 
			UserAuthInputData->s_Password[0], UserAuthInputData->s_Password[1] ,UserAuthInputData->s_Password[2], UserAuthInputData->s_Password[3], UserAuthInputData->s_Password[4], UserAuthInputData->s_Password[5] ,UserAuthInputData->s_Password[6], 
			UserAuthInputData->s_Password[7], UserAuthInputData->s_Password[8], UserAuthInputData->s_Password[9] ,UserAuthInputData->s_Password[10], UserAuthInputData->s_Password[11],
			UserAuthInputData->s_Password[12], UserAuthInputData->s_Password[13] ,UserAuthInputData->s_Password[14], UserAuthInputData->s_Password[15], UserAuthInputData->s_Password[16], UserAuthInputData->s_Password[17] ,UserAuthInputData->s_Password[18], 
			UserAuthInputData->s_Password[19], UserAuthInputData->s_Password[20], UserAuthInputData->s_Password[21] ,UserAuthInputData->s_Password[22], UserAuthInputData->s_Password[23],
		    UserAuthInputData->s_Password[24], UserAuthInputData->s_Password[25] ,UserAuthInputData->s_Password[26], UserAuthInputData->s_Password[27], UserAuthInputData->s_Password[28], UserAuthInputData->s_Password[29] ,UserAuthInputData->s_Password[30], UserAuthInputData->s_Password[31]);
	
		DEBUG_LOG_TMP(&LogClientHdTmp, e_DebugLogLevel_Exception, "key len=%d  key = %02x %02x %02x %02x %02x %02x %02x %02x.", UserAuthInputData->s_UserAuthExtDataLen,
			UserAuthInputData->s_UserAuthExtData[0], UserAuthInputData->s_UserAuthExtData[1], UserAuthInputData->s_UserAuthExtData[2], UserAuthInputData->s_UserAuthExtData[3], 
			UserAuthInputData->s_UserAuthExtData[4], UserAuthInputData->s_UserAuthExtData[5], UserAuthInputData->s_UserAuthExtData[6], UserAuthInputData->s_UserAuthExtData[7]);
		#endif

		#if 0
		DEBUG_LOG_TMP(&LogClientHdTmp, e_DebugLogLevel_Exception, "plain Password = %s.", TmpPassword);
		DEBUG_LOG_TMP(&LogClientHdTmp, e_DebugLogLevel_Exception, "cipherText Password = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x.", 
			TmpEncPassword[0], TmpEncPassword[1] ,TmpEncPassword[2], TmpEncPassword[3], TmpEncPassword[4], TmpEncPassword[5] ,TmpEncPassword[6], TmpEncPassword[7], TmpEncPassword[8], TmpEncPassword[9] ,TmpEncPassword[10], TmpEncPassword[11],
			TmpEncPassword[12], TmpEncPassword[13] ,TmpEncPassword[14], TmpEncPassword[15], TmpEncPassword[16], TmpEncPassword[17] ,TmpEncPassword[18], TmpEncPassword[19], TmpEncPassword[20], TmpEncPassword[21] ,TmpEncPassword[22], TmpEncPassword[23],
		    TmpEncPassword[24], TmpEncPassword[25] ,TmpEncPassword[26], TmpEncPassword[27], TmpEncPassword[28], TmpEncPassword[29] ,TmpEncPassword[30], TmpEncPassword[31]);
		#endif
		if(0 != memcmp(TmpEncPassword, UserAuthInputData->s_Password, LEN_DES_PASSWORD))
		{
			//ERR_PRINT("check password error.\n");
			DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]check password(Id-%s) error.", MoudleIdToStr(UserAuthInputData->s_MoudleId));
			return GMI_CODE_ERR_PASSWORD;
		}
	}

	if((ID_MOUDLE_REST_CONFIG != UserAuthInputData->s_MoudleId)
		&& (ID_MOUDLE_REST_WEB != UserAuthInputData->s_MoudleId))
	{
		//check validness of sessionId
		if(GMI_TMP_FAIL == UserAuthObject.CheckSessionIdValid(UserAuthInputData->s_SessionId, UserAuthInputData->s_Username))
		{
			//check linking numbers
			//RetValue = UserAuthObject.CheckUserLinkNumValid(UserAuthInputData->s_Username, UserAuthInputData->s_SingleUserMaxLinkNum, UserAuthInputData->s_AllUserMaxLinkNum);
			RetValue = UserAuthObject.CheckUserLinkNumValid(UserAuthInputData->s_Username, TMP_MAX_SINGLEUSER_LINKNUM, TMP_MAX_ALLUSERS_LINKNUM);
			if(GMI_FAIL_ALLLINK == RetValue)
			{
				//ERR_PRINT("CheckUserLinkNumValid GMI_FAIL_ALLLINK.\n");
				DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]CheckUserLinkNumValid(Id-%s) GMI_FAIL_ALLLINK maxsiglinknum %d error.", MoudleIdToStr(UserAuthInputData->s_MoudleId), TMP_MAX_SINGLEUSER_LINKNUM);
				return GMI_CODE_ERR_ALLLINK;
			}
			else if(GMI_FAIL_SINGLELINK == RetValue)
			{
				DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]CheckUserLinkNumValid(Id-%s) GMI_FAIL_SINGLELINK maxalllinknum %d .", MoudleIdToStr(UserAuthInputData->s_MoudleId), TMP_MAX_ALLUSERS_LINKNUM);
				return GMI_CODE_ERR_SINGLELINK;
			}
			else if(GMI_TMP_FAIL == RetValue)
			{
				DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]CheckUserLinkNumValid(Id-%s) error.", MoudleIdToStr(UserAuthInputData->s_MoudleId));
				return GMI_CODE_ERR_PARAM;
			}
			else
			{
				if(GMI_TMP_FAIL == UserAuthObject.CreateSessionId(&TmpSessionId))
				{
					DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]CreateSessionId(Id-%s) error.", MoudleIdToStr(UserAuthInputData->s_MoudleId));
					return GMI_CODE_ERR_SESSIONID;
				}
				else
				{
					UserAuthOutputData->s_SessionId = TmpSessionId;
					
				}
			}
		}
		else
		{
			UserAuthOutputData->s_SessionId = UserAuthInputData->s_SessionId;
			SessionValid = 1;
		}

		if(0 == SessionValid)
		{
			//update record of user-sessionId and user-linking numbers
			if(GMI_TMP_FAIL == UserAuthObject.SetUserSessionIdRecord(UserAuthOutputData->s_SessionId, UserAuthInputData->s_Username, UserAuthInputData->s_MoudleId, GMI_RECORD_ADD))	
			//if(GMI_TMP_FAIL == UserAuthObject.SetUserSessionIdRecord(UserAuthOutputData->s_SessionId, UserAuthInputData->s_Username, 15, GMI_RECORD_ADD))
			{
				DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]SetUserSessionIdRecord(Id-%s) error.", MoudleIdToStr(UserAuthInputData->s_MoudleId));
				return GMI_CODE_ERR_INTER;
			}
			else
			{
				if(GMI_TMP_FAIL == UserAuthObject.SetUserLinkNumRecord(UserAuthInputData->s_Username, GMI_RECORD_ADD))
				{
					DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]SetUserLinkNumRecord(Id-%s) error.", MoudleIdToStr(UserAuthInputData->s_MoudleId));
					return GMI_CODE_ERR_INTER;
				}
					
			}
		}
	}
	else
	{
		if(ID_MOUDLE_REST_CONFIG == UserAuthInputData->s_MoudleId)
		{
			if((strlen(UserAuthInputData->s_Username) == strlen(GMI_SUPER_USER_NAME))
				&& (0 == memcpy(UserAuthInputData->s_Username, GMI_SUPER_USER_NAME, strlen(GMI_SUPER_USER_NAME))))
			{
				UserAuthOutputData->s_SessionId = ID_SESSIONID_INTER_CONFIGROOT;
			}
			else
			{
				UserAuthOutputData->s_SessionId = ID_SESSIONID_INTER_CONFIGADMIN;
			}
		}
	}
		
	if(GMI_TMP_FAIL == UserAuthObject.Deinitialize())
	{
		return GMI_CODE_ERR_INTER;
	}

    

	return GMI_CODE_SUCCESS;
	
}

GMI_RESULT UserLogout(uint16_t  SessionId, uint32_t MoudleId)
{

	UserAuthentication UserAuthObject;
    char_t UserName[MAX_LEN_USERNAME];
	
	memset(UserName, 0, sizeof(UserName));
	
	if(0 == SessionId)
	{
		DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception,"[AUTH]sessionId(Id-%s) error.", MoudleIdToStr(MoudleId));
		return GMI_CODE_ERR_SESSIONID;
	}

	if(GMI_TMP_SUCCESS == UserAuthObject.SetUserSessionIdRecord(SessionId, UserName, 0, GMI_RECORD_DEL))
	{
		if(GMI_TMP_SUCCESS == UserAuthObject.SetUserLinkNumRecord(UserName, GMI_RECORD_DEL))
		{
			return GMI_CODE_SUCCESS;
		}
		else
		{
			DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception,"[AUTH]UserLogout SetUserLinkNumRecord(Id-%s) error.", MoudleIdToStr(MoudleId));
		}
	}
	else
	{
		DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception,"[AUTH]UserLogout SetUserSessionIdRecord(Id-%s) sessionId %d error.", MoudleIdToStr(MoudleId), SessionId);
	}

	return GMI_CODE_ERR_SESSIONID;
}


GMI_RESULT UserQuerySessionId(UserAuthLinkedSessionId *OutLinkedId)
{
	UserAuthentication UserAuthObject;
	UserSessionId TmpUserSession[MAX_USER_LINK_NUM];
	UserSessionId *PtrUserSession = TmpUserSession;
	int32_t i = 0;
	int32_t IdSum = 0;

	if(NULL == OutLinkedId)
	{
		DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]OutLinkedId error.");
		return GMI_CODE_ERR_PARAM;
	}

	memset(TmpUserSession, 0, MAX_USER_LINK_NUM*sizeof(UserSessionId));
	if(GMI_TMP_SUCCESS != UserAuthObject.GetUserSessionIdRecord(&PtrUserSession))
	{
		DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]GetUserSessionIdRecord error.");
		return GMI_CODE_ERR_INTER;
	}

	PRT_TEST(("******UserQuerySessionId*******\n"));
	for(i=0; i<MAX_USER_LINK_NUM; i++)
	{
		if(TmpUserSession[i].s_SessionId > 0)
		{
			OutLinkedId->s_LinkedSessionId[IdSum] = TmpUserSession[i].s_SessionId;		
			PRT_TEST(("%d\n", OutLinkedId->s_LinkedSessionId[IdSum]));
			IdSum++;	
		}
	}
	PRT_TEST(("*******sum %d******\n", IdSum));

	OutLinkedId->s_LinkedNum = IdSum;

	return GMI_CODE_SUCCESS;
}


static void SignalHandler(int signum)
{
    if (signum == SIGINT)
    {
        //printf("receive SIGINT (Ctrl-C) signal\n");
        exit(0);
    }
    else if (signum == SIGIO)
    {
        //printf("receive SIGIO signal\n");
    }
    else if (signum == SIGPIPE)
    {
        //printf("receive SIGPIPE signal\n");
    }
    else if (signum == SIGHUP)
    {
        //printf("receive SIGHUP signal\n");
        exit(0);
    }
    else if (signum == SIGQUIT)
    {
        //printf("receive SIGQUIT signal\n");
        exit(0);
    }
    else if (signum == SIGSEGV)
    {
        sleep(1);
    }
    else
    {
        //printf("receive unknown signal. signal = %d\n", signum);
    }
    return;
}

#ifdef LOG_SERVER_OK
#define AUTH_CENTER_SERVER_CONFIG_PATH                "/Config/auth_center_server/"
#define AUTH_CENTER_SERVER_CONFIG_HEARTBEAT_INTERVAL  "heartbeat_interval"
#define AUTH_CENTER_SERVER_CONFIG_SERVER_ADDRESS      "server_address"
#define AUTH_CENTER_SERVER_CONFIG_SERVER_PORT         "command_port"

#define AUTH_CENTER_SERVER_CONFIG_LOG_SERVER_PORT     "log_server_port"
#define AUTH_CENTER_SERVER_CONFIG_LOG_CLIENT_PORT     "log_client_port"
#define AUTH_CENTER_SERVER_CONFIG_DEBUG_LOG_LEVEL     "debug_log_level"

static GMI_RESULT GetAuthCenterServerLogConfig( uint32_t *ModuleId, char_t *ModuleName, uint16_t *ServerPort, uint16_t *ClientPort, uint32_t *DebugLogLevel )
{
    *ModuleId = GMI_LOG_MODULE_AUTHENTICATION_ID;

    strcpy( ModuleName, GMI_LOG_MODULE_AUTHENTICATION_NAME );

    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_RESOURCE_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        fprintf(stderr, "GetAuthCenterServerLogConfig, GMI_XmlOpen(%s), Result=%x \n", GMI_RESOURCE_CONFIG_FILE_NAME, (uint32_t) Result );
        return Result;
    }

    int32_t TempServerPort = 0;
    fprintf(stderr, "GetAuthCenterServerLogConfig, DefaultLogServerPort=%d \n", LOG_SERVER_DEFAULT_SERVER_PORT );
    Result = GMI_XmlRead(Handle, AUTH_CENTER_SERVER_CONFIG_PATH, AUTH_CENTER_SERVER_CONFIG_LOG_SERVER_PORT, LOG_SERVER_DEFAULT_SERVER_PORT, &TempServerPort, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        fprintf(stderr, "GetAuthCenterServerLogConfig, GMI_XmlRead, Result=%x \n", (uint32_t) Result );
        return Result;
    }
    fprintf(stderr, "GetAuthCenterServerLogConfig, DefaultLogServerPort=%d, LogServerPort=%d \n", LOG_SERVER_DEFAULT_SERVER_PORT, TempServerPort );
    *ServerPort = (uint16_t)TempServerPort;

    int32_t TempClientPort = 0;
    fprintf(stderr, "GetAuthCenterServerLogConfig, DefaultLogClientPort=%d \n", LOG_MEDIA_CENTER_DEFAULT_PORT );
    Result = GMI_XmlRead(Handle, AUTH_CENTER_SERVER_CONFIG_PATH, AUTH_CENTER_SERVER_CONFIG_LOG_CLIENT_PORT, LOG_AUTHENTICATION_DEFAULT_PORT, &TempClientPort, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        fprintf(stderr, "GetAuthCenterServerLogConfig, GMI_XmlRead, Result=%x \n", (uint32_t) Result );
        return Result;
    }
    fprintf(stderr, "GetAuthCenterServerLogConfig, DefaultLogClientPort=%d, LogClientPort=%d \n", LOG_AUTHENTICATION_DEFAULT_PORT, TempClientPort );
    *ClientPort = (uint16_t)TempClientPort;

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        fprintf(stderr, "GetAuthCenterServerLogConfig, GMI_XmlFileSave(%s), Result=%x \n", GMI_RESOURCE_CONFIG_FILE_NAME, (uint32_t) Result );
        return Result;
    }

    Result = GMI_XmlOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        fprintf(stderr, "GetAuthCenterServerLogConfig, GMI_XmlOpen(%s), Result=%x \n", GMI_SETTING_CONFIG_FILE_NAME, (uint32_t) Result );
        return Result;
    }

    fprintf(stderr, "media center server, GetMediaCenterLogLevel, Default_MediaCenterLogLevel=%d \n", GMI_LOG_MODULE_AUTHENTICATION_DEBUG_LOG_LEVEL );
    Result = GMI_XmlRead(Handle, AUTH_CENTER_SERVER_CONFIG_PATH, AUTH_CENTER_SERVER_CONFIG_DEBUG_LOG_LEVEL, GMI_LOG_MODULE_AUTHENTICATION_DEBUG_LOG_LEVEL, (int32_t *) DebugLogLevel, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        fprintf(stderr, "GetAuthCenterServerLogConfig, GMI_XmlRead, Result=%x \n", (uint32_t) Result );
        return Result;
    }
    fprintf(stderr, "GetAuthCenterServerLogConfig, Default_MediaCenterLogLevel=%d, Config_MediaCenterLogLevel=%d \n", GMI_LOG_MODULE_AUTHENTICATION_DEBUG_LOG_LEVEL, *DebugLogLevel );

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        fprintf(stderr, "GetAuthCenterServerLogConfig, GMI_XmlFileSave(%s), Result=%x \n", GMI_SETTING_CONFIG_FILE_NAME, (uint32_t) Result );
        return Result;
    }
	
    return GMI_SUCCESS;
}
#endif

int main(int argc, char* argv[])
{
	uint8_t RecvBuf[1024] = {0};
	uint8_t SendBuf[1024] = {0};
	uint8_t TmpRecvBuf[1024] = {0};
	uint32_t TmpRecvBufLen = 0;
	uint32_t CompleteBufLen = 0;
	uint32_t LocalPort = LOCAL_AUTH_PORT;
	GMI_RESULT RetValue = GMI_CODE_SUCCESS;
	FD_HANDLE SockFd = NULL;
	UserAuthRefInfo *UserAuthInputData;
	UserAuthResInfo UserAuthOutputData;
	PkgRudpRecvInput  RudpRecvInput  = {0};
	PkgRudpRecvOutput RudpRecvOutput = {0};
	PkgRudpSendInput  RudpSendInput  = {0};
	PkgRudpSendOutput RudpSendOutput = {0};

	UserAuthLinkedSessionId UserLinkedId;

	int32_t DaemonNumber = 10;
    DAEMON_DATA_CFG DaemonData;
    uint32_t  DaemonFlags = 0;
	long_t DaemonRet = 0;
	struct timeval Timeout;
	fd_set FdSet;
	int32_t RetSelect = -1;	 

	int TmpSignal;
    struct sigaction SigAction;
    sigset_t NewMask;
    sigset_t OldMask;

	uint32_t ModuleId = 0;
    char_t   ModuleName[MAX_PATH_LENGTH] = {0};
    uint16_t ServerPort = 0;
    uint16_t ClientPort = 0;
    uint32_t ServerDebugLogLevel = 0;

	#ifdef LOG_SERVER_OK

    GMI_RESULT Result = GetAuthCenterServerLogConfig( &ModuleId, ModuleName, &ServerPort, &ClientPort, &ServerDebugLogLevel);
    if ( FAILED( Result ) )
    {
        printf( "Auth Center get log client config error \n" );
        return Result;
    }

	
    ShareMemoryLogClient Client;
    Result = Client.Initialize( ModuleId, ModuleName, ServerPort, ClientPort, GMI_IPC_LOG_FILE_PATH, ServerDebugLogLevel );
    if ( FAILED( Result ) )
    {
        printf( "Auth Center log client initialization error \n" );
        return Result;
    }

    // log operation is low speed for now, to test other function, we comment it out
    LogClientHdTmp = &Client;
	#endif
	
    //signal
    SigAction.sa_handler = SignalHandler;
    sigfillset(&SigAction.sa_mask);
    SigAction.sa_flags = SA_NOMASK;
    sigemptyset(&NewMask);
    for (TmpSignal=1; TmpSignal<=_NSIG; ++TmpSignal)
    {
        if ((TmpSignal == SIGIO)
                || (TmpSignal == SIGPOLL)
                || (TmpSignal == SIGINT)
                || (TmpSignal == SIGQUIT)
                || (TmpSignal == SIGHUP)
                || (TmpSignal == SIGPIPE)
                || (TmpSignal == SIGSEGV)
           )
        {
            sigaction(TmpSignal, &SigAction, NULL);
        }
        else
        {
            sigaddset(&NewMask, TmpSignal);
        }
    }
    sigprocmask(SIG_BLOCK, &NewMask, &OldMask);

	
	DaemonRet = GMI_DaemonInit( &DaemonData, AUTH_SERVER_ID, GMI_DAEMON_HEARDBEAT_SERVER, GMI_DAEMON_HEARTBEAT_AUTH);
	if (FAILED(DaemonRet))
	{
		DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]GMI_DaemonInit Error.");
	}
	
	while (DaemonNumber)
	{
		DaemonRet = GMI_DaemonRegister(&DaemonData);
		if (SUCCEEDED(DaemonRet))
		{
			//fprintf(stderr, "Auth_server Client Server Heartbeat register OK!\n");
			break;
		}
		DaemonNumber--;
		sleep(1);
	}

	if(DaemonNumber <= 0)
	{
		DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]Client Server Heartbeat register fail.");
		return -1;
	}

	
	memset(&UserLinkedId, 0, sizeof(UserLinkedId));
	uint32_t UserSessionId[MAX_USER_LINK_NUM];
	
	UserLinkedId.s_LinkedSessionId = UserSessionId;

	#if 0
	pthread_rwlock_t *pRdWrLock = NULL;
	void *ShareMemory = (void*)0;
	int32_t ShmId = 0;
	#else
	pthread_rwlock_t RdWrLock;
	#endif

	RudpRecvInput.s_TimeoutMS = 2000;
    SockFd = GMI_RudpSocket(LocalPort);
	if(NULL == SockFd)
	{
		DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]rudpsock error.");
		return -1;
	}
	
	RudpRecvOutput.s_Buffer = RecvBuf;
	RudpRecvOutput.s_BufferLength = sizeof(RecvBuf);
	
	RudpSendInput.s_TimeoutMS	 = 1000;
	RudpSendInput.s_LocalSvrPort = 0;

	RudpSendInput.s_Buffer = SendBuf;

	#if 0
	ShmId = shmget((key_t)KEY_SHM_NUM, sizeof(pthread_rwlock_t)*2, 0666|IPC_CREAT);
	if(0 > ShmId)
	{
		fprintf(stderr, "%s-%d:share memory create error\n", __FILE__, __LINE__);
		return -1;
	}
	ShareMemory = shmat(ShmId, 0, 0);
	if((void *)-1 == ShareMemory)
	{
		fprintf(stderr, "%s-%d:rdwrLock error\n", __FILE__, __LINE__);
		return -1;
	}
	pRdWrLock = (pthread_rwlock_t *)ShareMemory;
	if(0 > pthread_rwlock_init(pRdWrLock, NULL))
	{
		fprintf(stderr, "pthread_rwlock_init failed\n");
		return -1;
	}
	#else
	if(0 > pthread_rwlock_init(&RdWrLock, NULL))
	{
		DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]pthread_rwlock_init failed.");
		return -1;
	}
	#endif

	UserAuthReset(ID_MOUDLE_REST_ALL);
	
	for(;;)
	{
		memset(&UserAuthOutputData, 0, sizeof(UserAuthOutputData));
		memset(RecvBuf, 0, sizeof(RecvBuf));

	    Timeout.tv_sec  = 1;
        Timeout.tv_usec = 0;
        FD_ZERO(&FdSet);
        FD_SET((int32_t)SockFd, &FdSet);
        RetSelect = select((int32_t)SockFd+1, &FdSet, NULL, NULL, &Timeout);
        if (RetSelect <= 0)
        {
        	DaemonRet = GMI_DaemonReport(&DaemonData, &DaemonFlags);
            if (SUCCEEDED(DaemonRet))
            {
                if (APPLICATION_QUIT == DaemonFlags)
                {
                    DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]Auth_server Will be Quit.");
                    break;
                }
                else if (APPLICATION_RUNNING == DaemonFlags)
                {
                    //fprintf(stderr, "Auth_server Heartbeat Send OK!\n");
                }

            }
			else
			{
				DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]Auth_server Heartbeat Send fail.");
			}
            continue;
        }

		RudpSendInput.s_TimeoutMS	 = 1000;
		RetValue = GMI_RudpRecv(SockFd, &RudpRecvInput, &RudpRecvOutput);
		

		if(FAILED(RetValue))
		{	
			DaemonRet = GMI_DaemonReport(&DaemonData, &DaemonFlags);
            if (SUCCEEDED(DaemonRet))
            {
                if (APPLICATION_QUIT == DaemonFlags)
                {
                    DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]Auth_server Will be Quit.");
                    break;
                }
                else if (APPLICATION_RUNNING == DaemonFlags)
                {
                    //fprintf(stderr, "Auth_server Heartbeat Send OK!\n");
                }

            }
			else
			{
				DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]Auth_server Heartbeat Send fail.");
			}
			
			continue;
		}

		CompleteBufLen = RudpRecvOutput.s_BufferLength;

		switch(RudpRecvOutput.s_MessageType)
		{
			//case GMI_MESSAGE_TYPE_PARTIAL:
			case RUDP_MESSAGE_TYPE_PARTIAL:
				if(1024 <= TmpRecvBufLen+RudpRecvOutput.s_BufferLength)
				{
					DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception, "[AUTH]partial data length error.");
				}
				memcpy(TmpRecvBuf+TmpRecvBufLen, RudpRecvOutput.s_Buffer, RudpRecvOutput.s_BufferLength);
				TmpRecvBufLen += RudpRecvOutput.s_BufferLength;
				
				if(RudpRecvOutput.s_PktTotalLength > RudpRecvOutput.s_MsgOffsetofPkt+RudpRecvOutput.s_BufferLength)
				{
					break;
				}
				CompleteBufLen = TmpRecvBufLen;
				
			case RUDP_MESSAGE_TYPE_COMPLETE:
				memset(TmpRecvBuf, 0, sizeof(TmpRecvBuf));
				TmpRecvBufLen = 0;
				if(CompleteBufLen >= sizeof(UserAuthRefInfo))
				{
					UserAuthInputData = (UserAuthRefInfo *)(RudpRecvOutput.s_Buffer);
					if(UserAuthInputData->s_UserAuthExtDataLen > 0)
					{
						UserAuthInputData->s_UserAuthExtData = (char_t*)(RudpRecvOutput.s_Buffer+sizeof(UserAuthRefInfo));
					}

					switch(UserAuthInputData->s_DataType)
					{
						case TYPE_AUTH_LOGIN:
							#if 0
							RetValue = UserAuthCheck(UserAuthInputData, &UserAuthOutputData, pRdWrLock);
							#else
							RetValue = UserAuthCheck(UserAuthInputData, &UserAuthOutputData, &RdWrLock);
							#endif
							break;
						case TYPE_AUTH_LOGOUT:
							RetValue = UserLogout(UserAuthInputData->s_SessionId, UserAuthInputData->s_MoudleId);
							break;
						case TYPE_AUTH_RESET:							
							//fprintf(stderr, "UserAuthInputData->s_DataType=%d\n", UserAuthInputData->s_DataType);
							RetValue = UserAuthReset(UserAuthInputData->s_MoudleId);
							break;
						case TYPE_AUTH_GETSESSIONID:
							UserLinkedId.s_LinkedNum = 0;
							memset(UserSessionId, 0, sizeof(UserSessionId));
							RetValue = UserQuerySessionId(&UserLinkedId);
							break;
						default:
							RetValue = GMI_CODE_ERR_PARAM;
							break;
					}

					
					

					switch(RetValue)
					{
						case GMI_CODE_SUCCESS:
							if(TYPE_AUTH_LOGIN == UserAuthInputData->s_DataType)
							{
								DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Info, "[AUTH]Login(Id-%s, sId-%d, auth-0x%x) OK.", MoudleIdToStr(UserAuthInputData->s_MoudleId), UserAuthOutputData.s_SessionId, UserAuthOutputData.s_AuthValue);
								PRT_TEST(("Login OK...\n"));
								PRT_TEST(("sessionId=%d, authValue=%d\n", UserAuthOutputData.s_SessionId, UserAuthOutputData.s_AuthValue));
							}
							else if(TYPE_AUTH_LOGOUT == UserAuthInputData->s_DataType)
							{
								DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Info, "[AUTH]Logout(Id-%s, sId-%d) OK.", MoudleIdToStr(UserAuthInputData->s_MoudleId), UserAuthInputData->s_SessionId);
								PRT_TEST(("Logout OK...\n"));
							}
							else if(TYPE_AUTH_RESET == UserAuthInputData->s_DataType)
							{
								DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Info, "[AUTH]Reset(Id-%s) OK.", MoudleIdToStr(UserAuthInputData->s_MoudleId));
								PRT_TEST(("Reset OK...\n"));
							}
							else if(TYPE_AUTH_GETSESSIONID == UserAuthInputData->s_DataType)
							{
								DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Info, "[AUTH]GetSessionId(Id-%s) OK.", MoudleIdToStr(UserAuthInputData->s_MoudleId));
								PRT_TEST(("GetSessionId OK...\n"));
							}
							break;
						case GMI_CODE_ERR_PARAM:
							PRT_TEST(("GMI_CODE_ERR_PARAM error...\n"));
							break;
						case GMI_CODE_ERR_USER:
							PRT_TEST(("GMI_CODE_ERR_USER error...\n"));
							break;
						case GMI_CODE_ERR_PASSWORD:
							PRT_TEST(("GMI_CODE_ERR_PASSWORD error...\n"));
							break;
						case GMI_CODE_ERR_SESSIONID:
							PRT_TEST(("GMI_CODE_ERR_SESSIONID error...\n"));
							break;
						case GMI_CODE_ERR_SINGLELINK:
							PRT_TEST(("GMI_CODE_ERR_SINGLELINK error...\n"));
							break;
						case GMI_CODE_ERR_ALLLINK:
							PRT_TEST(("GMI_CODE_ERR_ALLLINK error...\n"));
							break;
						default:
							PRT_TEST(("auth center internal error...\n"));
							break;
					}

					UserAuthOutputData.s_DataType = UserAuthInputData->s_DataType;
					UserAuthOutputData.s_AuthResult = RetValue;
					if(TYPE_AUTH_GETSESSIONID != UserAuthInputData->s_DataType)
					{
						memcpy(RudpSendInput.s_Buffer, &UserAuthOutputData, sizeof(UserAuthOutputData));
						RudpSendInput.s_SendLength = sizeof(UserAuthOutputData);
					}
					else
					{
						memcpy(RudpSendInput.s_Buffer, &UserAuthOutputData, sizeof(UserAuthOutputData));
						memcpy(RudpSendInput.s_Buffer+sizeof(UserAuthOutputData), &UserLinkedId, sizeof(UserLinkedId));
						memcpy(RudpSendInput.s_Buffer+sizeof(UserAuthOutputData)+sizeof(UserLinkedId), UserLinkedId.s_LinkedSessionId, sizeof(uint32_t)*UserLinkedId.s_LinkedNum);
						RudpSendInput.s_SendLength = sizeof(UserAuthOutputData)+sizeof(UserLinkedId)+(sizeof(uint32_t)*UserLinkedId.s_LinkedNum);
					}
					RudpSendInput.s_MessageType = RUDP_MESSAGE_TYPE_COMPLETE;
					RudpSendInput.s_LocalSvrPort = LOCAL_AUTH_PORT;
					RudpSendInput.s_RemotePort	 = RudpRecvOutput.s_RemoteSvrPort;
					RudpSendInput.s_SequenceNum = RudpRecvOutput.s_SequenceNum;

					RetValue = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
				
					if(FAILED(RetValue))
					{
						DEBUG_LOG_TMP(LogClientHdTmp, e_DebugLogLevel_Exception,  "[AUTH]Ack data error.");
					}
					
				}
				break;
				
			default:
				memset(TmpRecvBuf, 0, sizeof(TmpRecvBuf));
				TmpRecvBufLen = 0;
				PRT_TEST(("s_MessageType error\n"));
				break;
		}
		
	}


	GMI_RudpSocketClose(SockFd);
	
	GMI_DaemonUnInit(&DaemonData);

	#if 0
	pthread_rwlock_destroy(pRdWrLock);
	if(-1 == shmdt(ShareMemory))
	{
		fprintf(stderr, "%s-%d: shmdt error\n", __FILE__, __LINE__);
		return -1;
	}
	if(-1 == shmctl(ShmId, IPC_RMID, 0))
	{
		fprintf(stderr, "%s-%d: shmctl(IPC_RMID) error\n", __FILE__, __LINE__);
		return -1;
	}
	#else
	pthread_rwlock_destroy(&RdWrLock);
	#endif

	#ifdef LOG_SERVER_OK
    Client.Deinitialize();
	LogClientHdTmp = NULL;
	#endif
	
	return 0;
}




