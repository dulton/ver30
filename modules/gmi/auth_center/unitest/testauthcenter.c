#include "auth_center_api.h"
#include "gmi_system_headers.h"
#include <stdlib.h> 
#include "gmi_rudp_api.h"



#define SESSIONID			 0
#define SINGLEUSERMAXLINK  2
#define ALLUSERMAXLINK     5

//e10adc3949ba59abbe56e057f20f883e   123456-Md5

//login£º./XX login username password portnum sessionId
//logout£º./XX logout sessionId portnum

#if 1
int32_t  main(int32_t argc, char_t* argv[])
{
	UserAuthRefInfo InputData;
	UserAuthResInfo OutputData;
	int32_t RetValue = 0;
	int32_t TmpData = 0;
	int32_t IsLogin = 0;
	int32_t LocalPort = 0;
	int SessionId = 0;
	char RandomNumber[128];
	UserAuthLinkedSessionId LinkSessionIdData;
	uint32_t TmpSessionId[128];
	int32_t TmpN = 0;
	#if 0
	char TmpPasswordBuf[128];
	int n_i = 0;
	int TmpNum = 0;
	int TmpCount = 0;
	#endif
	int MoudleId = 0;

	memset(TmpSessionId, 0, sizeof(TmpSessionId));
	memset(&LinkSessionIdData, 0, sizeof(LinkSessionIdData));
	LinkSessionIdData.s_LinkedSessionId = TmpSessionId;

	fprintf(stderr, "argc=%d\n", argc);
	
	if(argc < 3)
	{
		fprintf(stderr, "param too less.\n");

		return -1;
	}
	else
	{
		if((0 == memcmp(argv[1], "login", strlen("login"))))
		{
			if(argc < 7)
			{
				fprintf(stderr, "login param need 6.\n");

				return -1;
			}
			IsLogin = 1;
			LocalPort = atoi(argv[4]);;
			SessionId = atoi(argv[5]);
			MoudleId = atoi(argv[6]);
			if((MoudleId < ID_MOUDLE_REST_SDK) || (MoudleId > ID_MOUDLE_REST_ALL))
			{
				fprintf(stderr, "clear moudleId param error.\n");

				return -1;
			}
		}
		else if((0 == memcmp(argv[1], "logout", strlen("logout"))))
		{
			TmpData = atoi(argv[2]);
			if((TmpData < 0) || (TmpData > 65534))
			{
				fprintf(stderr, "logout sessionId param error.\n");

				return -1;
			}
			LocalPort = atoi(argv[3]);
			if((LocalPort <= 0) || (LocalPort > 65535))
			{
				fprintf(stderr, "logout port param error.\n");

				return -1;
			}
			IsLogin = 2;
		}
		else if((0 == memcmp(argv[1], "clear", strlen("clear"))))
		{
			LocalPort = atoi(argv[2]);
			if((LocalPort <= 0) || (LocalPort > 65535))
			{
				fprintf(stderr, "clear port param error.\n");

				return -1;
			}
			MoudleId = atoi(argv[3]);
			if((MoudleId < ID_MOUDLE_REST_SDK) || (MoudleId > ID_MOUDLE_REST_ALL))
			{
				fprintf(stderr, "clear moudleId param error.\n");

				return -1;
			}
			IsLogin = 3;
		}
		else if((0 == memcmp(argv[1], "getid", strlen("getid"))))
		{
			LocalPort = atoi(argv[2]);
			if((LocalPort <= 0) || (LocalPort > 65535))
			{
				fprintf(stderr, "getid port param error.\n");

				return -1;
			}
			IsLogin = 4;
		}
		else
		{
			
          fprintf(stderr, "commander error.\n");

			 return -1;
		}
	}

	printf("localPort=%d\n", LocalPort);

	memset(&InputData, 0, sizeof(InputData));
	memset(&OutputData, 0, sizeof(OutputData));	

	if(1 == IsLogin)
	{
		memcpy(InputData.s_Username, argv[2], strlen(argv[2]));
		InputData.s_MoudleId = MoudleId;
		#if 1
		memcpy(InputData.s_Password, argv[3], strlen(argv[3]));
		#else
		memset(TmpPasswordBuf, 0, sizeof(TmpPasswordBuf));
		memcpy(TmpPasswordBuf, argv[3], strlen(argv[3]));
		printf("TmpPasswordBuf=%s\n", TmpPasswordBuf);
	
		for(n_i=0; n_i<strlen(TmpPasswordBuf); )
		{
			TmpNum = 0;
			if(TmpPasswordBuf[n_i] >= 'a')
			{
				TmpNum = ((int)(TmpPasswordBuf[n_i]-'a'+10))*16;
			}
			else
			{
				TmpNum = ((int)(TmpPasswordBuf[n_i]-'0'))*16;
			}
			if(TmpPasswordBuf[n_i+1] >= 'a')
			{
				TmpNum += (int)(TmpPasswordBuf[n_i+1]-'a'+10);
			}
			else
			{
				TmpNum += (int)(TmpPasswordBuf[n_i+1]-'0');
			}
			 
			 if(TmpNum > 0x7f)
			 {
				InputData.s_Password[TmpCount] = TmpNum- 0xFF-1;
			 }
			 else
			 {
			 	InputData.s_Password[TmpCount] = TmpNum;
			 }
			 printf("%02x", InputData.s_Password[TmpCount]);
			 n_i += 2;
			 TmpCount += 1;
		}
		printf("\n");
		
		#endif
		
		InputData.s_DataType = TYPE_AUTH_LOGIN;
		InputData.s_UsernameEncType = TYPE_ENCRYPTION_TEXT;
		//InputData.s_PasswordEncType = TYPE_ENCRYPTION_DES;
		InputData.s_PasswordEncType = TYPE_ENCRYPTION_MD5_RTSP;
		InputData.s_SessionId = SessionId;
		InputData.s_SingleUserMaxLinkNum = SINGLEUSERMAXLINK;
		InputData.s_AllUserMaxLinkNum = ALLUSERMAXLINK;
		InputData.s_UserAuthExtDataLen = 0;
		InputData.s_UserAuthExtData = NULL;
		InputData.s_MoudleId = ID_MOUDLE_REST_RTSP;
		#if 1
		UserAuthExtInfo TmpUserAuthExtInfo;
		memset(&TmpUserAuthExtInfo, 0, sizeof(TmpUserAuthExtInfo));
		sprintf(TmpUserAuthExtInfo.s_Realm , "%s", "abcdef");
		sprintf(TmpUserAuthExtInfo.s_Cmd, "%s", "hello");
		sprintf(TmpUserAuthExtInfo.s_Nonce, "%s", "123456789");
		sprintf(TmpUserAuthExtInfo.s_Url, "%s", "http123.213");
		InputData.s_UserAuthExtDataLen = sizeof(TmpUserAuthExtInfo);
		InputData.s_UserAuthExtData = (char_t*)(&TmpUserAuthExtInfo);
		#else
		InputData.s_UserAuthExtDataLen = 8;
		memset(RandomNumber, 0, sizeof(RandomNumber));
		sprintf(RandomNumber, "%s", "12345678");
		InputData.s_UserAuthExtData = RandomNumber;
		#endif
		
		RetValue = GMI_UserAuthCheck(&InputData, &OutputData, 1234);
		fprintf(stderr, "\n\n*************\n");
		fprintf(stderr, "username:%s\n", InputData.s_Username);
		fprintf(stderr, "password:%s\n", InputData.s_Password);
		fprintf(stderr, "usernameEnc:%d\n", InputData.s_UsernameEncType);
		fprintf(stderr, "passwordEnc:%d\n", InputData.s_PasswordEncType);
		fprintf(stderr, "sessionId:%d\n", InputData.s_SessionId);
		fprintf(stderr, "singleMaxLink:%d\n", InputData.s_SingleUserMaxLinkNum);
		fprintf(stderr, "totalMaxLink:%d\n", InputData.s_AllUserMaxLinkNum);
		fprintf(stderr, "authExtDataLen:%d\n", InputData.s_UserAuthExtDataLen);
		fprintf(stderr, "*************\n\n");
	}
	else if(2 == IsLogin)
	{
		RetValue = GMI_UserLogoutNotify(TmpData, 1234);
	}
	else if(3 == IsLogin)
	{
		
		RetValue = GMI_UserLogInDataReset(MoudleId, 1234);
	}
	else if(4 == IsLogin)
	{
		RetValue = GMI_UserQueryLogInSessionId(1234, &LinkSessionIdData);
		fprintf(stderr, "\n******s_LinkedNum:%d******\n", (int)(LinkSessionIdData.s_LinkedNum));
		for(TmpN=0; TmpN<(int)(LinkSessionIdData.s_LinkedNum); TmpN++)
		{
			fprintf(stderr, "  %d  \n", LinkSessionIdData.s_LinkedSessionId[TmpN]);
		}
		
		fprintf(stderr, "*************\n\n");
	}
	else
	{
		fprintf(stderr, "IsLogin %d...\n", IsLogin);
		return -1;
	}

	if(FAILED(RetValue))
	{
		RetValue = OutputData.s_AuthResult;
		switch(RetValue)
		{
			case GMI_CODE_ERR_PARAM:
				fprintf(stderr, "GMI_CODE_ERR_PARAM error...\n");
				break;
			case GMI_CODE_ERR_USER:
				fprintf(stderr, "GMI_CODE_ERR_USER error...\n");
				break;
			case GMI_CODE_ERR_PASSWORD:
				fprintf(stderr, "GMI_CODE_ERR_PASSWORD error...\n");
				break;
			case GMI_CODE_ERR_SESSIONID:
				fprintf(stderr, "GMI_CODE_ERR_SESSIONID error...\n");
				break;
			case GMI_CODE_ERR_SINGLELINK:
				fprintf(stderr, "GMI_CODE_ERR_SINGLELINK error...\n");
				break;
			case GMI_CODE_ERR_ALLLINK:
				fprintf(stderr, "GMI_CODE_ERR_ALLLINK error...\n");
				break;
			default:
				fprintf(stderr, "no type of  error...\n");
				break;
		}
	}
	else
	{
		if(1 == IsLogin)
		{
			fprintf(stderr, "Login OK...\n");
			fprintf(stderr, "sessionId=%d, authValue=%d\n", OutputData.s_SessionId, OutputData.s_AuthValue);
		}
		else if(2 == IsLogin)
		{
			fprintf(stderr, "Logout OK...\n");
		}
		else if(3 == IsLogin)
		{
			fprintf(stderr, "clear OK...\n");
		}
		else if(4 == IsLogin)
		{
			fprintf(stderr, "getid OK...\n");
		}
	}

	

	
	while(1)
	{
		sleep(10);
	}
	return 0;
}

#else
int32_t  main(int32_t argc, char_t* argv[])
{
	UserAuthRefInfo InputData;
	UserAuthResInfo OutputData;
	int32_t RetValue = 0;
	int32_t TmpData = 0;
	int32_t IsLogin = 0;
	int32_t LocalPort = 0;
	GMI_RESULT Result     = GMI_SUCCESS;
	FD_HANDLE  Sock       = NULL;
	PkgRudpSendInput  RudpSendInput  = {0};
    PkgRudpSendOutput RudpSendOutput = {0};
    PkgRudpRecvInput  RudpRecvInput  = {0};
    PkgRudpRecvOutput RudpRecvOutput = {0};
	char_t RecBuf[512] = {0};
	uint8_t SndBuf[1024] = {0};
	int SessionId = 0;
	
	if(argc < 4)
	{
		fprintf(stderr, "param too less.\n");

		return -1;
	}
	else
	{
		if((0 == memcmp(argv[1], "login", strlen("login"))))
		{
			if(argc < 6)
			{
				fprintf(stderr, "login param need 6.\n");

				return -1;
			}
			IsLogin = 1;
			LocalPort = atoi(argv[4]);;
			SessionId = atoi(argv[5]);
		}
		else if((0 == memcmp(argv[1], "logout", strlen("logout"))))
		{
			TmpData = atoi(argv[2]);
			if((TmpData < 0) || (TmpData > 65534))
			{
				fprintf(stderr, "logout sessionId param error.\n");

				return -1;
			}
			LocalPort = atoi(argv[3]);
			if((LocalPort <= 0) || (TmpData > 65535))
			{
				fprintf(stderr, "logout port param error.\n");

				return -1;
			}
		}
		else
		{
			
          fprintf(stderr, "commander error.\n");

			 return -1;
		}
	}

	printf("localPort=%d\n", LocalPort);

	memset(&InputData, 0, sizeof(InputData));
	memset(&OutputData, 0, sizeof(OutputData));
	Sock = GMI_RudpSocket(LocalPort); 
	if (Sock == NULL)
	{
		printf("GMI_RudpSocket error\n");
		return -1;
	}
			

	if(1 == IsLogin)
	{
		memcpy(InputData.s_Username, argv[2], strlen(argv[2]));
		memcpy(InputData.s_Password, argv[3], strlen(argv[3]));
		
		InputData.s_DataType = TYPE_AUTH_LOGIN;
		InputData.s_UsernameEncType = TYPE_ENCRYPTION_TEXT;
		InputData.s_PasswordEncType = TYPE_ENCRYPTION_TEXT;
		InputData.s_SessionId = SessionId;
		InputData.s_SingleUserMaxLinkNum = SINGLEUSERMAXLINK;
		InputData.s_AllUserMaxLinkNum = ALLUSERMAXLINK;
		InputData.s_UserAuthExtDataLen = 0;
		InputData.s_UserAuthExtData = NULL;

		RudpSendInput.s_LocalSvrPort = LocalPort;
		RudpSendInput.s_RemotePort = LOCAL_AUTH_PORT;
		memcpy(SndBuf, &InputData, sizeof(InputData));
        RudpSendInput.s_Buffer = SndBuf;
		RudpSendInput.s_SendLength = sizeof(InputData)+InputData.s_UserAuthExtDataLen;
		RudpSendInput.s_TimeoutMS = 0;
		RudpSendInput.s_MessageType = RUDP_MESSAGE_TYPE_COMPLETE;
		
		//RetValue = GMI_UserAuthCheck(&InputData, &OutputData);
		fprintf(stderr, "\n\n*************\n");
		fprintf(stderr, "username:%s\n", InputData.s_Username);
		fprintf(stderr, "password:%s\n", InputData.s_Password);
		fprintf(stderr, "usernameEnc:%d\n", InputData.s_UsernameEncType);
		fprintf(stderr, "passwordEnc:%d\n", InputData.s_PasswordEncType);
		fprintf(stderr, "sessionId:%d\n", InputData.s_SessionId);
		fprintf(stderr, "singleMaxLink:%d\n", InputData.s_SingleUserMaxLinkNum);
		fprintf(stderr, "totalMaxLink:%d\n", InputData.s_AllUserMaxLinkNum);
		fprintf(stderr, "authExtDataLen:%d\n", InputData.s_UserAuthExtDataLen);
		fprintf(stderr, "*************\n\n");
	}
	else
	{
		InputData.s_DataType = TYPE_AUTH_LOGOUT;
		InputData.s_SessionId = TmpData;
		RudpSendInput.s_LocalSvrPort = LocalPort;
		RudpSendInput.s_RemotePort = LOCAL_AUTH_PORT;
        memcpy(SndBuf, &InputData, sizeof(InputData));
        RudpSendInput.s_Buffer = SndBuf;
		RudpSendInput.s_SendLength = sizeof(InputData)+InputData.s_UserAuthExtDataLen;
		RudpSendInput.s_TimeoutMS = 0;
		RudpSendInput.s_MessageType = RUDP_MESSAGE_TYPE_COMPLETE;
	}

	 Result = GMI_RudpSend(Sock, &RudpSendInput, &RudpSendOutput);
	if(GMI_SUCCESS != Result)
	{
		fprintf(stderr, "GMI_RudpSend send error.\n");
		if(NULL != Sock)
		{
			GMI_RudpSocketClose(Sock);
		}
		return -1;
	}
	
	memset(&RudpRecvInput, 0, sizeof(PkgRudpRecvInput));
	RudpRecvInput.s_TimeoutMS = 1000;
	RudpRecvOutput.s_Buffer = (uint8_t*)RecBuf;
	RudpRecvOutput.s_BufferLength = sizeof(RecBuf);
	
	Result = GMI_RudpRecv(Sock, &RudpRecvInput, &RudpRecvOutput);
	if (FAILED(Result))
    {
        fprintf(stderr, "GMI_RudpRecv error \n");
		if(NULL != Sock)
		{
			GMI_RudpSocketClose(Sock);
		}
		return -1;
    }
	if(RudpRecvOutput.s_RecvLength >= sizeof(OutputData))
	{
		memcpy(&OutputData, RudpRecvOutput.s_Buffer, sizeof(OutputData));
	}
	else
	{
		fprintf(stderr, "GMI_RudpRecv param error \n");
		if(NULL != Sock)
		{
			GMI_RudpSocketClose(Sock);
		}
		return -1;
	}

	RetValue = OutputData.s_AuthResult;

	switch(RetValue)
	{
		case GMI_CODE_SUCCESS:
			if(1 == IsLogin)
			{
				fprintf(stderr, "Login OK...\n");
				fprintf(stderr, "sessionId=%d, authValue=%d\n", OutputData.s_SessionId, OutputData.s_AuthValue);
			}
			else
			{
				fprintf(stderr, "Logout OK...\n");
			}
			break;
		case GMI_CODE_ERR_PARAM:
			fprintf(stderr, "GMI_CODE_ERR_PARAM error...\n");
			break;
		case GMI_CODE_ERR_USER:
			fprintf(stderr, "GMI_CODE_ERR_USER error...\n");
			break;
		case GMI_CODE_ERR_PASSWORD:
			fprintf(stderr, "GMI_CODE_ERR_PASSWORD error...\n");
			break;
		case GMI_CODE_ERR_SESSIONID:
			fprintf(stderr, "GMI_CODE_ERR_SESSIONID error...\n");
			break;
		case GMI_CODE_ERR_SINGLELINK:
			fprintf(stderr, "GMI_CODE_ERR_SINGLELINK error...\n");
			break;
		case GMI_CODE_ERR_ALLLINK:
			fprintf(stderr, "GMI_CODE_ERR_ALLLINK error...\n");
			break;
		default:
			fprintf(stderr, "no type of  error...\n");
			break;
	}

	if(NULL != Sock)
	{
		GMI_RudpSocketClose(Sock);
	}
	while(1)
	{
		sleep(10);
	}
	return 0;
}
#endif

