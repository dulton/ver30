

#include "auth_center_api.h"
#include "gmi_system_headers.h"
#include <stdlib.h> 
#include "gmi_rudp_api.h"
#include "ipc_fw_v3.x_resource.h"


#define USER_LOGIN_PORT     61234
#define USER_LOGOUT_PORT    61235

static uint16_t l_SeqNum = 0;

GMI_RESULT GMI_UserAuthCheck(UserAuthRefInfo *UserAuthInputData, UserAuthResInfo *UserAuthOutputData, const uint16_t InPortNum)
{
	UserAuthRefInfo *InputData       = NULL;
	UserAuthResInfo *OutputData      = NULL;
	int32_t LocalPort                = USER_LOGIN_PORT;
	GMI_RESULT Result                 = GMI_SUCCESS;
	FD_HANDLE  Sock                   = NULL;
	PkgRudpSendInput  RudpSendInput  = {0};
    PkgRudpSendOutput RudpSendOutput = {0};
    PkgRudpRecvInput  RudpRecvInput  = {0};
    PkgRudpRecvOutput RudpRecvOutput = {0};
	uint8_t RecBuf[512] = {0};
	uint8_t SndBuf[1024] = {0};

	if((NULL == UserAuthInputData)
		|| (NULL == UserAuthOutputData)
		||('\0' == UserAuthInputData->s_Username)
		//|| ('\0' == UserAuthInputData->s_Password)
		|| ((0 < UserAuthInputData->s_UserAuthExtDataLen) && (NULL == UserAuthInputData->s_UserAuthExtData))
		//|| (0 == UserAuthInputData->s_SingleUserMaxLinkNum)
		//|| (0 == UserAuthInputData->s_AllUserMaxLinkNum)
		|| ((TYPE_AUTH_LOGIN != UserAuthInputData->s_DataType) && (TYPE_AUTH_LOGOUT != UserAuthInputData->s_DataType)))

	{
		fprintf(stderr, "input param error\n");
		return GMI_FAIL;
	}

	if(InPortNum > 0)
	{
		LocalPort = InPortNum;
	}

	Sock = GMI_RudpSocket(LocalPort);
	if(Sock == NULL)
	{
		fprintf(stderr, "GMI_RudpSocket error\n");
		return GMI_FAIL;
	}

	InputData = UserAuthInputData;
	OutputData = UserAuthOutputData;

	memset(&RudpSendInput, 0,sizeof(RudpSendInput));
	memset(SndBuf, 0, sizeof(SndBuf));
	RudpSendInput.s_LocalSvrPort = LocalPort;
	RudpSendInput.s_RemotePort = LOCAL_AUTH_PORT;
	memcpy(SndBuf, InputData, sizeof(UserAuthRefInfo));
	if(InputData->s_UserAuthExtDataLen > 0)
	{
		memcpy(SndBuf+sizeof(UserAuthRefInfo), InputData->s_UserAuthExtData, InputData->s_UserAuthExtDataLen);
	}
    RudpSendInput.s_Buffer = SndBuf;
	RudpSendInput.s_SendLength = sizeof(UserAuthRefInfo)+InputData->s_UserAuthExtDataLen;
	RudpSendInput.s_TimeoutMS = 0;
	RudpSendInput.s_MessageType = RUDP_MESSAGE_TYPE_COMPLETE;
	l_SeqNum++;
	if(0xFFFF <= l_SeqNum)
	{
		l_SeqNum = 1;
	}
	RudpSendInput.s_SequenceNum = l_SeqNum;

	Result = GMI_RudpSend(Sock, &RudpSendInput, &RudpSendOutput);
	if(FAILED(Result))
	{
		fprintf(stderr, "GMI_RudpSend send error\n");
		if(NULL != Sock)
		{
			GMI_RudpSocketClose(Sock);
		}
		return GMI_FAIL;
	}

	memset(&RudpRecvInput, 0, sizeof(PkgRudpRecvInput));
	memset(RecBuf, 0, sizeof(RecBuf));
	RudpRecvInput.s_TimeoutMS = 1000;
	RudpRecvOutput.s_Buffer = RecBuf;
	RudpRecvOutput.s_BufferLength = sizeof(RecBuf);
	
	Result = GMI_RudpRecv(Sock, &RudpRecvInput, &RudpRecvOutput);
	if(FAILED(Result))
    {
        fprintf(stderr, "GMI_RudpRecv error \n");
		if(NULL != Sock)
		{
			GMI_RudpSocketClose(Sock);
		}
		return GMI_FAIL;
    }
	else
	{
		if(RudpSendInput.s_SequenceNum != RudpRecvOutput.s_SequenceNum)
		{
			fprintf(stderr, "GMI_RudpRecv SeqNum %d is not match\n", RudpRecvOutput.s_SequenceNum);
			if(NULL != Sock)
			{
				GMI_RudpSocketClose(Sock);
			}
			return GMI_FAIL;
		}
	}

	if(RudpRecvOutput.s_RecvLength >= sizeof(UserAuthResInfo))
	{
		memcpy(OutputData, RudpRecvOutput.s_Buffer, sizeof(UserAuthResInfo));
	}
	else
	{
		fprintf(stderr, "GMI_RudpRecv param error \n");
		if(NULL != Sock)
		{
			GMI_RudpSocketClose(Sock);
		}
		return GMI_FAIL;
	}

	if(NULL != Sock)
	{
		GMI_RudpSocketClose(Sock);
	}

	if(GMI_CODE_SUCCESS == OutputData->s_AuthResult)
	{
		return GMI_SUCCESS;
	}
	else
	{
		return GMI_FAIL;
	}
	
}

GMI_RESULT GMI_UserLogoutNotify(const uint16_t  sessionId, const uint16_t InPortNum)
{
	UserAuthRefInfo InputData;
	UserAuthResInfo OutputData;
	int32_t LocalPort                = USER_LOGIN_PORT;
	GMI_RESULT Result                 = GMI_SUCCESS;
	FD_HANDLE  Sock                   = NULL;
	PkgRudpSendInput  RudpSendInput  = {0};
    PkgRudpSendOutput RudpSendOutput = {0};
    PkgRudpRecvInput  RudpRecvInput  = {0};
    PkgRudpRecvOutput RudpRecvOutput = {0};
	uint8_t RecBuf[512]                = {0};
	uint8_t SndBuf[1024]              = {0};

	if(0 == sessionId)
	{
		fprintf(stderr, "sessionId error\n");
		return GMI_FAIL;
	}

	if(InPortNum > 0)
	{
		LocalPort = InPortNum;
	}

	Sock = GMI_RudpSocket(LocalPort);
	if(Sock == NULL)
	{
		fprintf(stderr, "GMI_RudpSocket error\n");
		return GMI_FAIL;
	}

	memset(&InputData, 0, sizeof(InputData));
	memset(&OutputData, 0, sizeof(OutputData));
	memset(&RudpSendInput, 0,sizeof(RudpSendInput));
	memset(SndBuf, 0, sizeof(SndBuf));
	InputData.s_DataType = TYPE_AUTH_LOGOUT;
	InputData.s_SessionId = sessionId;
	RudpSendInput.s_LocalSvrPort = LocalPort;
	RudpSendInput.s_RemotePort = LOCAL_AUTH_PORT;
    memcpy(SndBuf, &InputData, sizeof(InputData));
    RudpSendInput.s_Buffer = SndBuf;
	RudpSendInput.s_SendLength = sizeof(InputData)+InputData.s_UserAuthExtDataLen;
	RudpSendInput.s_TimeoutMS = 0;
	RudpSendInput.s_MessageType = RUDP_MESSAGE_TYPE_COMPLETE;
	l_SeqNum++;
	if(0xFFFF <= l_SeqNum)
	{
		l_SeqNum = 1;
	}
	RudpSendInput.s_SequenceNum = l_SeqNum;

	Result = GMI_RudpSend(Sock, &RudpSendInput, &RudpSendOutput);
	if(FAILED(Result))
	{
		fprintf(stderr, "GMI_RudpSend send error\n");
		if(NULL != Sock)
		{
			GMI_RudpSocketClose(Sock);
		}
		return GMI_FAIL;
	}

	memset(&RudpRecvInput, 0, sizeof(PkgRudpRecvInput));
	memset(RecBuf, 0, sizeof(RecBuf));
	RudpRecvInput.s_TimeoutMS = 1000;
	RudpRecvOutput.s_Buffer = RecBuf;
	RudpRecvOutput.s_BufferLength = sizeof(RecBuf);
	
	Result = GMI_RudpRecv(Sock, &RudpRecvInput, &RudpRecvOutput);
	if(FAILED(Result))
    {
        fprintf(stderr, "GMI_RudpRecv error \n");
		if(NULL != Sock)
		{
			GMI_RudpSocketClose(Sock);
		}
		return GMI_FAIL;
    }
	else
	{
		if(RudpSendInput.s_SequenceNum != RudpRecvOutput.s_SequenceNum)
		{
			fprintf(stderr, "GMI_RudpRecv SeqNum %d is not match\n", RudpRecvOutput.s_SequenceNum);
			if(NULL != Sock)
			{
				GMI_RudpSocketClose(Sock);
			}
			return GMI_FAIL;
		}
	}

	if(RudpRecvOutput.s_RecvLength >= sizeof(UserAuthResInfo))
	{
		memcpy(&OutputData, RudpRecvOutput.s_Buffer, sizeof(UserAuthResInfo));
	}
	else
	{
		fprintf(stderr, "GMI_RudpRecv param error \n");
		if(NULL != Sock)
		{
			GMI_RudpSocketClose(Sock);
		}
		return GMI_FAIL;
	}

	if(NULL != Sock)
	{
		GMI_RudpSocketClose(Sock);
	}

	if(GMI_CODE_SUCCESS == OutputData.s_AuthResult)
	{
		return GMI_SUCCESS;
	}
	else
	{
		return GMI_FAIL;
	}
	
	
}


GMI_RESULT GMI_UserLogInDataReset( const uint32_t MoudleId, const uint16_t InPortNum)
{
	UserAuthRefInfo InputData;
	UserAuthResInfo OutputData;
	int32_t LocalPort                = USER_LOGIN_PORT;
	GMI_RESULT Result                 = GMI_SUCCESS;
	FD_HANDLE  Sock                   = NULL;
	PkgRudpSendInput  RudpSendInput  = {0};
    PkgRudpSendOutput RudpSendOutput = {0};
    PkgRudpRecvInput  RudpRecvInput  = {0};
    PkgRudpRecvOutput RudpRecvOutput = {0};
	uint8_t RecBuf[512]                = {0};
	uint8_t SndBuf[1024]              = {0};

	
	if(InPortNum > 0)
	{
		LocalPort = InPortNum;
	}

	Sock = GMI_RudpSocket(LocalPort);
	if(Sock == NULL)
	{
		fprintf(stderr, "GMI_RudpSocket error\n");
		return GMI_FAIL;
	}

	memset(&InputData, 0, sizeof(InputData));
	memset(&OutputData, 0, sizeof(OutputData));
	memset(&RudpSendInput, 0,sizeof(RudpSendInput));
	memset(SndBuf, 0, sizeof(SndBuf));
	InputData.s_DataType = TYPE_AUTH_RESET;
	InputData.s_MoudleId = MoudleId;
	RudpSendInput.s_LocalSvrPort = LocalPort;
	RudpSendInput.s_RemotePort = LOCAL_AUTH_PORT;
    memcpy(SndBuf, &InputData, sizeof(InputData));
    RudpSendInput.s_Buffer = SndBuf;
	RudpSendInput.s_SendLength = sizeof(InputData)+InputData.s_UserAuthExtDataLen;
	RudpSendInput.s_TimeoutMS = 0;
	RudpSendInput.s_MessageType = RUDP_MESSAGE_TYPE_COMPLETE;
	l_SeqNum++;
	if(0xFFFF <= l_SeqNum)
	{
		l_SeqNum = 1;
	}
	RudpSendInput.s_SequenceNum = l_SeqNum;

	Result = GMI_RudpSend(Sock, &RudpSendInput, &RudpSendOutput);
	if(FAILED(Result))
	{
		fprintf(stderr, "GMI_RudpSend send error\n");
		if(NULL != Sock)
		{
			GMI_RudpSocketClose(Sock);
		}
		return GMI_FAIL;
	}

	memset(&RudpRecvInput, 0, sizeof(PkgRudpRecvInput));
	memset(RecBuf, 0, sizeof(RecBuf));
	RudpRecvInput.s_TimeoutMS = 1000;
	RudpRecvOutput.s_Buffer = RecBuf;
	RudpRecvOutput.s_BufferLength = sizeof(RecBuf);
	
	Result = GMI_RudpRecv(Sock, &RudpRecvInput, &RudpRecvOutput);
	if(FAILED(Result))
    {
        fprintf(stderr, "GMI_RudpRecv error \n");
		if(NULL != Sock)
		{
			GMI_RudpSocketClose(Sock);
		}
		return GMI_FAIL;
    }
	else
	{
		if(RudpSendInput.s_SequenceNum != RudpRecvOutput.s_SequenceNum)
		{
			fprintf(stderr, "GMI_RudpRecv SeqNum %d is not match\n", RudpRecvOutput.s_SequenceNum);
			if(NULL != Sock)
			{
				GMI_RudpSocketClose(Sock);
			}
			return GMI_FAIL;
		}
	}

	if(RudpRecvOutput.s_RecvLength >= sizeof(UserAuthResInfo))
	{
		memcpy(&OutputData, RudpRecvOutput.s_Buffer, sizeof(UserAuthResInfo));
	}
	else
	{
		fprintf(stderr, "GMI_RudpRecv param error \n");
		if(NULL != Sock)
		{
			GMI_RudpSocketClose(Sock);
		}
		return GMI_FAIL;
	}

	if(NULL != Sock)
	{
		GMI_RudpSocketClose(Sock);
	}

	if(GMI_CODE_SUCCESS == OutputData.s_AuthResult)
	{
		return GMI_SUCCESS;
	}
	else
	{
		return GMI_FAIL;
	}
	
	
}


GMI_RESULT GMI_UserQueryLogInSessionId(const uint16_t InPortNum, UserAuthLinkedSessionId *SessionIdOutData)
{
	UserAuthRefInfo InputData;
	UserAuthResInfo OutputData;
	UserAuthLinkedSessionId SessionIdData;
	int32_t LocalPort                = USER_LOGIN_PORT;
	GMI_RESULT Result                 = GMI_SUCCESS;
	FD_HANDLE  Sock                   = NULL;
	PkgRudpSendInput  RudpSendInput  = {0};
    PkgRudpSendOutput RudpSendOutput = {0};
    PkgRudpRecvInput  RudpRecvInput  = {0};
    PkgRudpRecvOutput RudpRecvOutput = {0};
	uint8_t RecBuf[512]                = {0};
	uint8_t SndBuf[1024]              = {0};

	if((NULL == SessionIdOutData)
		|| (NULL == SessionIdOutData->s_LinkedSessionId))
	{
		fprintf(stderr, "input param error\n");
		return GMI_FAIL;
	}

	//SessionIdData = SessionIdOutData;
	
	if(InPortNum > 0)
	{
		LocalPort = InPortNum;
	}

	Sock = GMI_RudpSocket(LocalPort);
	if(Sock == NULL)
	{
		fprintf(stderr, "GMI_RudpSocket error\n");
		return GMI_FAIL;
	}

	memset(&InputData, 0, sizeof(InputData));
	memset(&OutputData, 0, sizeof(OutputData));
	memset(&SessionIdData, 0, sizeof(SessionIdData));
	memset(&RudpSendInput, 0,sizeof(RudpSendInput));
	memset(SndBuf, 0, sizeof(SndBuf));
	InputData.s_DataType = TYPE_AUTH_GETSESSIONID;
	RudpSendInput.s_LocalSvrPort = LocalPort;
	RudpSendInput.s_RemotePort = LOCAL_AUTH_PORT;
    memcpy(SndBuf, &InputData, sizeof(InputData));
    RudpSendInput.s_Buffer = SndBuf;
	RudpSendInput.s_SendLength = sizeof(InputData)+InputData.s_UserAuthExtDataLen;
	RudpSendInput.s_TimeoutMS = 0;
	RudpSendInput.s_MessageType = RUDP_MESSAGE_TYPE_COMPLETE;
	l_SeqNum++;
	if(0xFFFF <= l_SeqNum)
	{
		l_SeqNum = 1;
	}
	RudpSendInput.s_SequenceNum = l_SeqNum;

	Result = GMI_RudpSend(Sock, &RudpSendInput, &RudpSendOutput);
	if(FAILED(Result))
	{
		fprintf(stderr, "GMI_RudpSend send error\n");
		if(NULL != Sock)
		{
			GMI_RudpSocketClose(Sock);
		}
		return GMI_FAIL;
	}

	memset(&RudpRecvInput, 0, sizeof(PkgRudpRecvInput));
	memset(RecBuf, 0, sizeof(RecBuf));
	RudpRecvInput.s_TimeoutMS = 1000;
	RudpRecvOutput.s_Buffer = RecBuf;
	RudpRecvOutput.s_BufferLength = sizeof(RecBuf);
	
	Result = GMI_RudpRecv(Sock, &RudpRecvInput, &RudpRecvOutput);
	if(FAILED(Result))
    {
        fprintf(stderr, "GMI_RudpRecv error \n");
		if(NULL != Sock)
		{
			GMI_RudpSocketClose(Sock);
		}
		return GMI_FAIL;
    }
	else
	{
		if(RudpSendInput.s_SequenceNum != RudpRecvOutput.s_SequenceNum)
		{
			fprintf(stderr, "GMI_RudpRecv SeqNum %d is not match\n", RudpRecvOutput.s_SequenceNum);
			if(NULL != Sock)
			{
				GMI_RudpSocketClose(Sock);
			}
			return GMI_FAIL;
		}
	}

	if(RudpRecvOutput.s_RecvLength >= sizeof(UserAuthResInfo)+sizeof(UserAuthLinkedSessionId))
	{
		memcpy(&OutputData, RudpRecvOutput.s_Buffer, sizeof(UserAuthResInfo));
		memcpy(&SessionIdData, RudpRecvOutput.s_Buffer+sizeof(UserAuthResInfo), sizeof(UserAuthLinkedSessionId));
		SessionIdData.s_LinkedSessionId = SessionIdOutData->s_LinkedSessionId;
		SessionIdOutData->s_LinkedNum = SessionIdData.s_LinkedNum;
		if((NULL != SessionIdData.s_LinkedSessionId) 
			&& (SessionIdData.s_LinkedNum > 0) 
			&& (RudpRecvOutput.s_RecvLength >= sizeof(UserAuthResInfo)+sizeof(UserAuthLinkedSessionId)+SessionIdData.s_LinkedNum*sizeof(uint32_t)))
		{	
			memcpy(SessionIdData.s_LinkedSessionId, RudpRecvOutput.s_Buffer+sizeof(UserAuthResInfo)+sizeof(UserAuthLinkedSessionId), SessionIdData.s_LinkedNum*sizeof(uint32_t));
		}
		else
		{
			fprintf(stderr, "GMI_RudpRecv linknum %d error \n", SessionIdData.s_LinkedNum);
			if(NULL != Sock)
			{
				GMI_RudpSocketClose(Sock);
			}
			return GMI_FAIL;
		}
	}
	else
	{
		fprintf(stderr, "GMI_RudpRecv param error \n");
		if(NULL != Sock)
		{
			GMI_RudpSocketClose(Sock);
		}
		return GMI_FAIL;
	}
	//fprintf(stderr, "GMI_UserQueryLogInSessionId  5555 \n");

	if(NULL != Sock)
	{
		GMI_RudpSocketClose(Sock);
	}
	//fprintf(stderr, "GMI_UserQueryLogInSessionId  6666 \n");

	if(GMI_CODE_SUCCESS == OutputData.s_AuthResult)
	{
		return GMI_SUCCESS;
	}
	else
	{
		return GMI_FAIL;
	}
	
	
}


