
#include <stream_control.h>
#include <memory>
#include <gmi_config_api.h>
#include <sys_env_types.h>
#include <rudp/gmi_rudp_api.h>

#define  MAX_STREAM_IDS     4

typedef struct
{	
    uint16_t m_Count;
	uint16_t m_AudioCount;
    SysPkgEncodeCfg m_VideoInfo[MAX_STREAM_IDS];
	SysPkgAudioEncodeCfg m_AudioInfo[2];
} sys_stream_info_t;



#define  STREAM_START_OPCODE_REQ    1701
#define  STREAM_START_OPCODE_RESP   1702
#define  STREAM_STOP_OPCODE_REQ     1703
#define  STREAM_STOP_OPCODE_RESP    1704
#define  STREAM_PAUSE_OPCODE_REQ    1705
#define  STREAM_PAUSE_OPCODE_RESP   1706
#define  STREAM_RESUME_OPCODE_REQ   1707
#define  STREAM_RESUME_OPCODE_RESP  1708
#define  STREAM_QUERY_OPCODE_REQ    1709
#define  STREAM_QUERY_OPCODE_RESP   1710


#define  STREAM_STATE_RUNNING       2
#define  STREAM_STATE_PAUSE         1
#define  STREAM_STATE_STOPPED       0

typedef struct
{
    uint32_t m_OpCode;
    sys_stream_info_t m_StreamInfos;
} sys_stream_request_t;

typedef struct
{
    uint32_t m_OpCode;
    uint32_t m_Result;
} sys_stream_response_t;




GMI_RESULT __HandleStreamOpCodeNoInfo(FD_HANDLE hd,int sdkport,uint32_t opcode,int timeout)
{
    GMI_RESULT gmiret;
    std::auto_ptr<sys_stream_request_t> pReq2(new sys_stream_request_t);
    sys_stream_request_t *pReq=pReq2.get();
    std::auto_ptr<sys_stream_response_t> pResp2(new sys_stream_response_t);
    sys_stream_response_t *pResp = pResp2.get();
    std::auto_ptr<PkgRudpSendInput> pRudpInput2(new PkgRudpSendInput);
    PkgRudpSendInput *pRudpInput = pRudpInput2.get();
    std::auto_ptr<PkgRudpSendOutput> pRudpOutput2(new PkgRudpSendOutput);
    PkgRudpSendOutput *pRudpOutput= pRudpOutput2.get();
    std::auto_ptr<PkgRudpRecvInput> pRecvInput2(new PkgRudpRecvInput);
    PkgRudpRecvInput *pRecvInput = pRecvInput2.get();
    std::auto_ptr<PkgRudpRecvOutput> pRecvOutput2(new PkgRudpRecvOutput);
    PkgRudpRecvOutput *pRecvOutput = pRecvOutput2.get();


    pReq->m_OpCode = opcode;

    pRudpInput->s_SessionId = 0;
    pRudpInput->s_AuthValue = 0;
    pRudpInput->s_MessageType = RUDP_MESSAGE_TYPE_COMPLETE;
    pRudpInput->s_Buffer = (uint8_t*)pReq;
    pRudpInput->s_SendLength = sizeof(*pReq);
    pRudpInput->s_RemotePort = sdkport;
    pRudpInput->s_TimeoutMS = timeout * 1000;
    pRudpInput->s_LocalSvrPort = 0;
    pRudpInput->s_PktTotalLength  = 0;
    pRudpInput->s_MsgOffsetofPkt= 0;

    gmiret = GMI_RudpSend(hd,pRudpInput,pRudpOutput);

    if (gmiret != GMI_SUCCESS)
    {
        goto fail;
    }

    pRecvInput->s_TimeoutMS = timeout * 1000;
    pRecvOutput->s_Buffer = (uint8_t*)pResp;
    pRecvOutput->s_BufferLength = sizeof(*pResp);

    gmiret = GMI_RudpRecv(hd,pRecvInput,pRecvOutput);
    if (gmiret != GMI_SUCCESS)
    {
        goto fail;
    }

    if (pRecvOutput->s_RecvLength < sizeof(*pResp))
    {
        gmiret = GMI_NOT_ENOUGH_SPACE;
        goto fail;
    }

    if (pResp->m_OpCode != opcode)
    {
        gmiret = GMI_INVALID_PARAMETER;
        goto fail;
    }
    if (pResp->m_Result != 0)
    {
        gmiret = GMI_FAIL;
        goto fail;
    }


    return	GMI_SUCCESS;

fail:
    return gmiret;
}

GMI_RESULT __HandleStreamOpCodeWithInfo(FD_HANDLE hd,int sdkport, uint32_t opcode,SysPkgEncodeCfg* pVideoCfg,int count,SysPkgAudioEncodeCfg *pSysAudioCfgPtr, int AudioCount,int timeout)
{
    GMI_RESULT gmiret;
    std::auto_ptr<sys_stream_request_t> pReq2(new sys_stream_request_t);
    sys_stream_request_t *pReq=pReq2.get();
    std::auto_ptr<sys_stream_response_t> pResp2(new sys_stream_response_t);
    sys_stream_response_t *pResp = pResp2.get();
    std::auto_ptr<PkgRudpSendInput> pRudpInput2(new PkgRudpSendInput);
    PkgRudpSendInput *pRudpInput = pRudpInput2.get();
    std::auto_ptr<PkgRudpSendOutput> pRudpOutput2(new PkgRudpSendOutput);
    PkgRudpSendOutput *pRudpOutput= pRudpOutput2.get();
    std::auto_ptr<PkgRudpRecvInput> pRecvInput2(new PkgRudpRecvInput);
    PkgRudpRecvInput *pRecvInput = pRecvInput2.get();
    std::auto_ptr<PkgRudpRecvOutput> pRecvOutput2(new PkgRudpRecvOutput);
    PkgRudpRecvOutput *pRecvOutput = pRecvOutput2.get();
    unsigned int i;


    if (count > MAX_STREAM_IDS)
    {
        gmiret = GMI_INVALID_PARAMETER;
        goto fail;
    }

	if ((unsigned int)AudioCount > (sizeof(pReq->m_StreamInfos.m_AudioInfo) /sizeof(pReq->m_StreamInfos.m_AudioInfo[0])))
	{
		gmiret = GMI_INVALID_PARAMETER;
		goto fail;
	}

	if (pSysAudioCfgPtr == NULL && AudioCount > 0)
	{
		gmiret = GMI_INVALID_PARAMETER;
		goto fail;
	}

    pReq->m_OpCode = opcode;
    pReq->m_StreamInfos.m_Count = count;
	pReq->m_StreamInfos.m_AudioCount = AudioCount;
    for (i=0; (int)i<count; i++)
    {
        memcpy(&(pReq->m_StreamInfos.m_VideoInfo[i]),&(pVideoCfg[i]),sizeof(*pVideoCfg));
    }

	for (i=0;(int)i<AudioCount;i++)
	{
		memcpy(&(pReq->m_StreamInfos.m_AudioInfo[i]),&(pSysAudioCfgPtr[i]),sizeof(*pSysAudioCfgPtr));
	}

    pRudpInput->s_SessionId = 0;
    pRudpInput->s_AuthValue = 0;
    pRudpInput->s_MessageType = RUDP_MESSAGE_TYPE_COMPLETE;
    pRudpInput->s_Buffer = (uint8_t*)pReq;
    pRudpInput->s_SendLength = sizeof(*pReq);
    pRudpInput->s_RemotePort = sdkport;
    pRudpInput->s_TimeoutMS = timeout * 1000;
    pRudpInput->s_LocalSvrPort = 0;
    pRudpInput->s_PktTotalLength = 0;
    pRudpInput->s_MsgOffsetofPkt = 0;


    gmiret = GMI_RudpSend(hd,pRudpInput,pRudpOutput);
    if (gmiret != GMI_SUCCESS)
    {
        goto fail;
    }

    pRecvInput->s_TimeoutMS = timeout * 1000;
    pRecvOutput->s_Buffer = (uint8_t*)pResp;
    pRecvOutput->s_BufferLength = sizeof(*pResp);
    gmiret = GMI_RudpRecv(hd,pRecvInput,pRecvOutput);
    if(gmiret != GMI_SUCCESS)
    {
        goto fail;
    }

    if (pRecvOutput->s_RecvLength < sizeof(*pResp))
    {
        gmiret = GMI_INVALID_PARAMETER;
        goto fail;
    }

    if (pResp->m_OpCode != opcode)
    {
        gmiret = GMI_INVALID_PARAMETER;
        goto fail;
    }

    if (pResp->m_Result != 0)
    {
        gmiret = GMI_FAIL;
        goto fail;
    }


    return GMI_SUCCESS;
fail:
    return gmiret;
}

GMI_RESULT StopStreamTransfer(FD_HANDLE hd,int sdkport,int timeout)
{
    return __HandleStreamOpCodeNoInfo(hd,sdkport,STREAM_STOP_OPCODE_REQ,timeout);
}

GMI_RESULT StartStreamTransfer(FD_HANDLE hd,int sdkport,SysPkgEncodeCfg* pVideoCfg,int count,SysPkgAudioEncodeCfg *SysAudioCfgPtr, int AudioCount,int timeout)
{
    return __HandleStreamOpCodeWithInfo(hd,sdkport,STREAM_START_OPCODE_REQ,pVideoCfg,count,SysAudioCfgPtr,AudioCount,timeout);
}


GMI_RESULT PauseStreamTransfer(FD_HANDLE hd,int sdkport,int timeout)
{
    return __HandleStreamOpCodeNoInfo(hd,sdkport,STREAM_PAUSE_OPCODE_REQ,timeout);
}


GMI_RESULT ResumeStreamTransfer(FD_HANDLE hd,int sdkport,SysPkgEncodeCfg* pVideoCfg,int count,SysPkgAudioEncodeCfg *SysAudioCfgPtr, int AudioCount,int timeout)
{
    return __HandleStreamOpCodeWithInfo(hd,sdkport,STREAM_RESUME_OPCODE_REQ,pVideoCfg,count,SysAudioCfgPtr,AudioCount,timeout);
}


GMI_RESULT QueryStreamTransfer(FD_HANDLE hd,int sdkport,int timeout,int *pstarted)
{
    GMI_RESULT gmiret;
    std::auto_ptr<sys_stream_request_t> pReq2(new sys_stream_request_t);
    sys_stream_request_t *pReq=pReq2.get();
    std::auto_ptr<sys_stream_response_t> pResp2(new sys_stream_response_t);
    sys_stream_response_t *pResp = pResp2.get();
    std::auto_ptr<PkgRudpSendInput> pRudpInput2(new PkgRudpSendInput);
    PkgRudpSendInput *pRudpInput = pRudpInput2.get();
    std::auto_ptr<PkgRudpSendOutput> pRudpOutput2(new PkgRudpSendOutput);
    PkgRudpSendOutput *pRudpOutput= pRudpOutput2.get();
    std::auto_ptr<PkgRudpRecvInput> pRecvInput2(new PkgRudpRecvInput);
    PkgRudpRecvInput *pRecvInput = pRecvInput2.get();
    std::auto_ptr<PkgRudpRecvOutput> pRecvOutput2(new PkgRudpRecvOutput);
    PkgRudpRecvOutput *pRecvOutput = pRecvOutput2.get();

    if (pstarted == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }


    pReq->m_OpCode = STREAM_QUERY_OPCODE_REQ;

    pRudpInput->s_SessionId = 0;
    pRudpInput->s_AuthValue = 0;
    pRudpInput->s_MessageType = RUDP_MESSAGE_TYPE_COMPLETE;
    pRudpInput->s_Buffer = (uint8_t*)pReq;
    pRudpInput->s_SendLength = sizeof(*pReq);
    pRudpInput->s_RemotePort = sdkport;
    pRudpInput->s_TimeoutMS = timeout * 1000;
    pRudpInput->s_LocalSvrPort = 0;
    pRudpInput->s_PktTotalLength  = 0;
    pRudpInput->s_MsgOffsetofPkt= 0;

    gmiret = GMI_RudpSend(hd,pRudpInput,pRudpOutput);

    if (gmiret != GMI_SUCCESS)
    {
        goto fail;
    }

    pRecvInput->s_TimeoutMS = timeout * 1000;
    pRecvOutput->s_Buffer = (uint8_t*)pResp;
    pRecvOutput->s_BufferLength = sizeof(*pResp);

    gmiret = GMI_RudpRecv(hd,pRecvInput,pRecvOutput);
    if (gmiret != GMI_SUCCESS)
    {
        goto fail;
    }

    if (pRecvOutput->s_RecvLength < sizeof(*pResp))
    {
        gmiret = GMI_NOT_ENOUGH_SPACE;
        goto fail;
    }

    if (pResp->m_OpCode != STREAM_QUERY_OPCODE_REQ)
    {
        gmiret = GMI_INVALID_PARAMETER;
        goto fail;
    }

    *pstarted = pResp->m_Result;


    return	GMI_SUCCESS;

fail:
    return gmiret;
}


