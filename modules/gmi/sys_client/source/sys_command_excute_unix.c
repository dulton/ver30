#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include "log.h"
#include "sys_command_excute.h"
#include "unix_tcp_session.h"
#include "sys_utilitly_unix.h"
#include "gmi_rudp_api.h"
#include "gmi_rudp.h"
#include "sys_env_types.h"
#include "ipc_fw_v3.x_resource.h"
#include "gmi_system_headers.h"

//func declaration
GMI_RESULT ProcessGetRequest(uint16_t SessionId, uint32_t AuthValue, int32_t SocketFd, uint32_t MsgType, uint16_t *AttrCntPtr, SysAttr **SysAttrRspPtr);
GMI_RESULT ProcessSetRequest(uint16_t SessionId, uint32_t AuthValue, int32_t SocketFd, uint32_t MsgType, uint16_t ReqAttrCnt, SysAttr *SysReqAttrPtr);
GMI_RESULT ProcessGetRequestWithAttrs(uint16_t SessionId, uint32_t AuthValue, int32_t SocketFd, uint32_t MsgType, uint16_t ReqAttrCnt, SysAttr *SysReqAttrPtr, uint16_t *AttrCntPtr, SysAttr **SysAttrRspPtr);


static struct timeval l_ComTimeout;
static uint16_t l_TryCount;

GMI_RESULT SysCmdInit()
{
    l_TryCount = 1;
    l_ComTimeout.tv_sec  = 5;
    l_ComTimeout.tv_usec = 0;
    return GMI_SUCCESS;
}


/***********************************************************
*func:SysCmdInitExt
*input:TimeoutPtr-communication max timeout, TryCount-try count when communication fail.
*output:successfully,return GMI_SUCCESS;
*/
GMI_RESULT SysCmdInitExt(struct timeval *TimeoutPtr, uint16_t TryCount)
{
    if (NULL == TimeoutPtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    l_ComTimeout.tv_sec  = TimeoutPtr->tv_sec;
    l_ComTimeout.tv_usec = TimeoutPtr->tv_usec;
    l_TryCount = TryCount;

    return GMI_SUCCESS;
}


GMI_RESULT SysGetCmdExcuteWithAttrs(uint16_t SessionId, uint32_t AuthValue, uint16_t Code, uint16_t ReqAttrCnt, SysAttr *SysReqAttrPtr, uint16_t *RspAttrCntPtr, SysAttr **SysRspAttrPtr)
{
    int32_t SocketFd;
    GMI_RESULT Result = GMI_SUCCESS;
    SysAttr *SysRspAttrPtrTmp = NULL;

    Result = UnixTcpOpen(&SocketFd);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("UnixTcpOpen fail, Result = 0x%lx\n", Result);
        return GMI_FAIL;
    }

    Result = ProcessGetRequestWithAttrs(SessionId, AuthValue, SocketFd, Code, ReqAttrCnt, SysReqAttrPtr, RspAttrCntPtr, &SysRspAttrPtrTmp);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("ProcessGetRequestWithAttrs fail, Result = 0x%lx, Socket %d, Code %d\n", Result, SocketFd, Code);
        return Result;
    }
    *SysRspAttrPtr = SysRspAttrPtrTmp;

    UnixTcpClose(SocketFd);

    return GMI_SUCCESS;
}


GMI_RESULT SysGetCmdExcute(uint16_t SessionId, uint32_t AuthValue, uint16_t Code, uint16_t *RspAttrCntPtr, SysAttr **SysRspAttrPtr)
{
    int32_t SocketFd;
    GMI_RESULT Result = GMI_SUCCESS;
    SysAttr *SysRspAttrPtrTmp = NULL;

    Result = UnixTcpOpen(&SocketFd);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("UnixTcpOpen fail, Result = 0x%lx\n", Result);
        return GMI_FAIL;
    }

    Result = ProcessGetRequest(SessionId, AuthValue, SocketFd, Code, RspAttrCntPtr, &SysRspAttrPtrTmp);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("ProcessGetRequest fail, Result = 0x%lx, Socket %d, Code %d\n", Result, SocketFd, Code);
        return Result;
    }
    *SysRspAttrPtr = SysRspAttrPtrTmp;

    UnixTcpClose(SocketFd);

    return GMI_SUCCESS;
}


GMI_RESULT SysSetCmdExcute(uint16_t SessionId, uint32_t AuthValue, uint16_t Code, uint16_t ReqAttrCnt, SysAttr *SysReqAttrPtr)
{
    int32_t SocketFd;
    GMI_RESULT Result = GMI_SUCCESS;

    Result = UnixTcpOpen(&SocketFd);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("UnixTcpOpen fail, Result = 0x%lx\n", Result);
        return GMI_FAIL;
    }

    Result = ProcessSetRequest(SessionId, AuthValue, SocketFd, Code, ReqAttrCnt, SysReqAttrPtr);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("ProcessSetRequest fail, Result = 0x%lx, Socket %d, Code %d\n", Result, SocketFd, Code);
        return Result;
    }

    UnixTcpClose(SocketFd);

    return GMI_SUCCESS;
}


void SysGetCmdAttrFree(uint16_t RspAttrCnt, SysAttr *SysRspAttrPtr)
{

    if (SysRspAttrPtr != NULL
            && SysRspAttrPtr->s_FreePtr!= NULL)
    {
        free(SysRspAttrPtr->s_FreePtr);
    }

    if (SysRspAttrPtr != NULL)
    {
        free(SysRspAttrPtr);
    }

    return;
}


void SysCmdUnInit()
{
    return;
}


GMI_RESULT ProcessGetRequestWithAttrs(uint16_t SessionId, uint32_t AuthValue, int32_t SocketFd, uint32_t MsgType, uint16_t ReqAttrCnt, SysAttr *SysReqAttrPtr, uint16_t *AttrCntPtr, SysAttr **SysAttrRspPtr)
{
    uint8_t  *RecvBuffPtr = NULL;
    uint16_t ErrNum = 0;
    uint16_t i;
    uint16_t RecvBuffSize;
    uint16_t seqno;
    uint16_t SysPkgHdLen;
    uint16_t SysPkgAttrHdLen;
    uint32_t Offset = 0;
    uint32_t Transferred;
    GMI_RESULT Result = GMI_SUCCESS;
    SysPkgHeader *SysPkgHeaderPtr;
    SysPkgAttrHeader *SysPkgAttrHeaderPtr;

    *SysAttrRspPtr  = NULL;
    SysPkgHdLen     = sizeof(SysPkgHeader);
    SysPkgAttrHdLen = sizeof(SysPkgAttrHeader);

    srand(time((time_t *)NULL));
    seqno = (rand()%1000 + 1000);
    Result = SendSetCmdReqs(SessionId, AuthValue, SocketFd, MsgType, ReqAttrCnt, SysReqAttrPtr, seqno);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("SendSetCmdReqs fail, Result = 0x%lx, Socket %d, Code %d\n", Result, SocketFd, MsgType);
        return Result;
    }

    while (ErrNum++ <= l_TryCount)
    {
        PkgRudpHeader RudpHeader;
        memset(&RudpHeader, 0, sizeof(PkgRudpHeader));
        Result = UnixTcpReceive(SocketFd, &l_ComTimeout, (uint8_t*)&RudpHeader, sizeof(PkgRudpHeader), &Transferred);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("UnixTcpReceive rudp header error, Result = 0x%lx, Socket %d, Try %d\n", Result, SocketFd, ErrNum);
            continue;
        }

        if (0 != memcmp(RudpHeader.s_MsgTag, GMI_RUDP_MESSAGE_TAG, GMI_RUDP_MESSAGE_TAG_LENGTH)
                || RudpHeader.s_Version != RUDP_VERSION)
        {
            SYS_CLIENT_ERROR("rudp header error, version %d, Tag %c%c%c%c\n", RudpHeader.s_Version, RudpHeader.s_MsgTag[0], RudpHeader.s_MsgTag[1], RudpHeader.s_MsgTag[2], RudpHeader.s_MsgTag[3]);
            return GMI_FAIL;
        }

        RecvBuffSize = RudpHeader.s_BodyLen;
        RecvBuffPtr = (uint8_t*)malloc(RecvBuffSize);
        if (NULL == RecvBuffPtr)
        {
            SYS_CLIENT_ERROR("malloc fail\n");
            Result = GMI_OUT_OF_MEMORY;
            return Result;
        }

        memset(RecvBuffPtr, 0, RecvBuffSize);

        Result = UnixTcpReceive(SocketFd, &l_ComTimeout, RecvBuffPtr, RecvBuffSize, &Transferred);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("UnixTcpReceive rudp header error, Result = 0x%lx, Socket %d, Try %d\n", Result, SocketFd, ErrNum);
            if (NULL != RecvBuffPtr)
            {
                free(RecvBuffPtr);
                RecvBuffPtr = NULL;
            }
            continue;
        }

        //receive successfully, jump out;
        break;
    }

    if (NULL == RecvBuffPtr)
    {
        return GMI_FAIL;
    }

    SysPkgHeaderPtr = (SysPkgHeader *)RecvBuffPtr;
    SysPkgHeaderPtr->s_SeqNum    = ntohs(SysPkgHeaderPtr->s_SeqNum);
    SysPkgHeaderPtr->s_AttrCount = ntohs(SysPkgHeaderPtr->s_AttrCount);
    if (SysPkgHeaderPtr->s_SeqNum != seqno)
    {
        SYS_CLIENT_ERROR("SeqNum %d error\n", SysPkgHeaderPtr->s_SeqNum);
        Result = GMI_FAIL;
        if (NULL != RecvBuffPtr)
        {
            free(RecvBuffPtr);
            RecvBuffPtr = NULL;
        }
        return Result;
    }

    if (0 != memcmp( GMI_SYS_MESSAGE_TAG, SysPkgHeaderPtr->s_SysMsgTag, GMI_SYS_MESSAGE_TAG_LENGTH )
            || SysPkgHeaderPtr->s_Version != SYS_COMM_VERSION
            || SysPkgHeaderPtr->s_SeqNum != seqno )
    {
        SYS_CLIENT_ERROR("recv packet error\n");
        Result = GMI_FAIL;
        if (NULL != RecvBuffPtr)
        {
            free(RecvBuffPtr);
            RecvBuffPtr = NULL;
        }
        return Result;
    }

    (*SysAttrRspPtr) = (SysAttr*) malloc(sizeof(SysAttr) * SysPkgHeaderPtr->s_AttrCount);
    if (*SysAttrRspPtr == NULL)
    {
        SYS_CLIENT_ERROR("malloc fail\n");
        Result = GMI_OUT_OF_MEMORY;
        if (NULL != RecvBuffPtr)
        {
            free(RecvBuffPtr);
            RecvBuffPtr = NULL;
        }
        return Result;
    }

    Offset += SysPkgHdLen;
    for (i = 0; i < SysPkgHeaderPtr->s_AttrCount; i++)
    {
        SysPkgAttrHeaderPtr = (SysPkgAttrHeader *)(RecvBuffPtr + Offset);
        Offset += SysPkgAttrHdLen;

        (*SysAttrRspPtr)[i].s_AttrLength = ntohs(SysPkgAttrHeaderPtr->s_Length) - SysPkgAttrHdLen;
        (*SysAttrRspPtr)[i].s_Attr       = (void_t*)(RecvBuffPtr + Offset);
        (*SysAttrRspPtr)[i].s_Type       = ntohs(SysPkgAttrHeaderPtr->s_Type);

        Offset += (*SysAttrRspPtr)[i].s_AttrLength;
    }

    *AttrCntPtr = SysPkgHeaderPtr->s_AttrCount;
    (*SysAttrRspPtr)->s_FreePtr = (void_t*)RecvBuffPtr;

    return Result;
}


GMI_RESULT ProcessGetRequest(uint16_t SessionId, uint32_t AuthValue, int32_t SocketFd, uint32_t MsgType, uint16_t *AttrCntPtr, SysAttr **SysAttrRspPtr)
{
    uint8_t  *RecvBuffPtr = NULL;
    uint16_t ErrNum = 0;
    uint16_t i;
    uint16_t RecvBuffSize;
    uint16_t seqno;
    uint16_t SysPkgHdLen;
    uint16_t SysPkgAttrHdLen;
    uint32_t Offset = 0;
    uint32_t Transferred;
    GMI_RESULT Result = GMI_SUCCESS;
    SysPkgHeader *SysPkgHeaderPtr;
    SysPkgAttrHeader *SysPkgAttrHeaderPtr;

    *SysAttrRspPtr  = NULL;
    SysPkgHdLen     = sizeof(SysPkgHeader);
    SysPkgAttrHdLen = sizeof(SysPkgAttrHeader);

    srand(time((time_t *)NULL));
    seqno = (rand()%1000 + 1000);
    Result = SendGetCmdReq(SessionId, AuthValue, SocketFd, MsgType, 0, 0, seqno);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("SendGetCmdReq fail, Result = 0x%lx, Socket %d, Code %d\n", Result, SocketFd, MsgType);
        return Result;
    }

    while (ErrNum++ <= l_TryCount)
    {
        PkgRudpHeader RudpHeader;
        memset(&RudpHeader, 0, sizeof(PkgRudpHeader));
        Result = UnixTcpReceive(SocketFd, &l_ComTimeout, (uint8_t*)&RudpHeader, sizeof(PkgRudpHeader), &Transferred);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("UnixTcpReceive rudp header error, Result = 0x%lx, Socket %d, Try %d\n", Result, SocketFd, ErrNum);
            continue;
        }

        if (0 != memcmp(RudpHeader.s_MsgTag, GMI_RUDP_MESSAGE_TAG, GMI_RUDP_MESSAGE_TAG_LENGTH)
                || RudpHeader.s_Version != RUDP_VERSION)
        {
            SYS_CLIENT_ERROR("rudp header error, version %d, Tag %c%c%c%c\n", RudpHeader.s_Version, RudpHeader.s_MsgTag[0], RudpHeader.s_MsgTag[1], RudpHeader.s_MsgTag[2], RudpHeader.s_MsgTag[3]);
            return GMI_FAIL;
        }

        RecvBuffSize = RudpHeader.s_BodyLen;
        RecvBuffPtr = (uint8_t*)malloc(RecvBuffSize);
        if (NULL == RecvBuffPtr)
        {
            SYS_CLIENT_ERROR("malloc fail\n");
            Result = GMI_OUT_OF_MEMORY;
            return Result;
        }

        memset(RecvBuffPtr, 0, RecvBuffSize);

        Result = UnixTcpReceive(SocketFd, &l_ComTimeout, RecvBuffPtr, RecvBuffSize, &Transferred);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("UnixTcpReceive rudp header error, Result = 0x%lx, Socket %d, Try %d\n", Result, SocketFd, ErrNum);
            if (NULL != RecvBuffPtr)
            {
                free(RecvBuffPtr);
                RecvBuffPtr = NULL;
            }
            continue;
        }

        //receive successfully, jump out;
        break;
    }

    if (NULL == RecvBuffPtr)
    {
        return GMI_FAIL;
    }

    SysPkgHeaderPtr = (SysPkgHeader *)RecvBuffPtr;
    SysPkgHeaderPtr->s_SeqNum    = ntohs(SysPkgHeaderPtr->s_SeqNum);
    SysPkgHeaderPtr->s_AttrCount = ntohs(SysPkgHeaderPtr->s_AttrCount);
    if (SysPkgHeaderPtr->s_SeqNum != seqno)
    {
        SYS_CLIENT_ERROR("SeqNum %d error\n", SysPkgHeaderPtr->s_SeqNum);
        Result = GMI_FAIL;
        if (NULL != RecvBuffPtr)
        {
            free(RecvBuffPtr);
            RecvBuffPtr = NULL;
        }
        return Result;
    }

    if (0 != memcmp( GMI_SYS_MESSAGE_TAG, SysPkgHeaderPtr->s_SysMsgTag, GMI_SYS_MESSAGE_TAG_LENGTH )
            || SysPkgHeaderPtr->s_Version != SYS_COMM_VERSION
            || SysPkgHeaderPtr->s_SeqNum != seqno )
    {
        SYS_CLIENT_ERROR("recv packet error\n");
        Result = GMI_FAIL;
        if (NULL != RecvBuffPtr)
        {
            free(RecvBuffPtr);
            RecvBuffPtr = NULL;
        }
        return Result;
    }

    (*SysAttrRspPtr) = (SysAttr*) malloc(sizeof(SysAttr) * SysPkgHeaderPtr->s_AttrCount);
    if (*SysAttrRspPtr == NULL)
    {
        SYS_CLIENT_ERROR("malloc fail\n");
        Result = GMI_OUT_OF_MEMORY;
        if (NULL != RecvBuffPtr)
        {
            free(RecvBuffPtr);
            RecvBuffPtr = NULL;
        }
        return Result;
    }

    Offset += SysPkgHdLen;
    for (i = 0; i < SysPkgHeaderPtr->s_AttrCount; i++)
    {
        SysPkgAttrHeaderPtr = (SysPkgAttrHeader *)(RecvBuffPtr + Offset);
        Offset += SysPkgAttrHdLen;

        (*SysAttrRspPtr)[i].s_AttrLength = ntohs(SysPkgAttrHeaderPtr->s_Length) - SysPkgAttrHdLen;
        (*SysAttrRspPtr)[i].s_Attr       = (void_t*)(RecvBuffPtr + Offset);
        (*SysAttrRspPtr)[i].s_Type       = ntohs(SysPkgAttrHeaderPtr->s_Type);

        Offset += (*SysAttrRspPtr)[i].s_AttrLength;
    }

    *AttrCntPtr = SysPkgHeaderPtr->s_AttrCount;
    (*SysAttrRspPtr)->s_FreePtr = (void_t*)RecvBuffPtr;

    return Result;
}



GMI_RESULT ProcessSetRequest(uint16_t SessionId, uint32_t AuthValue, int32_t SocketFd, uint32_t MsgType, uint16_t ReqAttrCnt, SysAttr *SysReqAttrPtr)
{
    GMI_RESULT Result    = GMI_SUCCESS;
    uint16_t ErrNum      = 0;
    uint8_t *RecvBuffPtr = NULL;
    uint16_t RecvBuffSize;
    uint16_t seqno;
    uint32_t Transferred;
    SysPkgHeader *SysPkgHeaderPtr;
    SysPkgAttrHeader *SysPkgAttrHeaderPtr;
    SysPkgMessageCode *SysPkgMessageCodePtr;

    srand(time((time_t *)NULL));
    seqno = (rand()%1000 + 1000);
    if (1 == ReqAttrCnt)
    {
        Result = SendSetCmdReq(
                     SessionId,
                     AuthValue,
                     SocketFd,
                     MsgType,
                     ReqAttrCnt,
                     SysReqAttrPtr->s_Type,
                     SysReqAttrPtr->s_Attr,
                     SysReqAttrPtr->s_AttrLength,
                     seqno);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SendSetCmdReq fail, Result = 0x%lx\n", Result);
            return Result;
        }
    }
    else
    {
        Result = SendSetCmdReqs(
                     SessionId,
                     AuthValue,
                     SocketFd,
                     MsgType,
                     ReqAttrCnt,
                     SysReqAttrPtr,
                     seqno);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SendSetCmdReqs fail, Result = 0x%lx\n", Result);
            return Result;
        }
    }

    while (ErrNum++ <= l_TryCount)
    {
        PkgRudpHeader RudpHeader;
        memset(&RudpHeader, 0, sizeof(PkgRudpHeader));
        Result = UnixTcpReceive(SocketFd, &l_ComTimeout, (uint8_t*)&RudpHeader, sizeof(PkgRudpHeader), &Transferred);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("UnixTcpReceive rudp header error, Result = 0x%lx, Socket %d, Try %d\n", Result, SocketFd, ErrNum);
            continue;
        }

        if (0 != memcmp(RudpHeader.s_MsgTag, GMI_RUDP_MESSAGE_TAG, GMI_RUDP_MESSAGE_TAG_LENGTH)
                || RudpHeader.s_Version != RUDP_VERSION)
        {
            SYS_CLIENT_ERROR("rudp header error, version %d, Tag %c%c%c%c\n", RudpHeader.s_Version, RudpHeader.s_MsgTag[0], RudpHeader.s_MsgTag[1], RudpHeader.s_MsgTag[2], RudpHeader.s_MsgTag[3]);
            return GMI_FAIL;
        }

        RecvBuffSize = RudpHeader.s_BodyLen;
        RecvBuffPtr = (uint8_t*)malloc(RecvBuffSize);
        if (NULL == RecvBuffPtr)
        {
            SYS_CLIENT_ERROR("malloc fail\n");
            Result = GMI_OUT_OF_MEMORY;
            return Result;
        }

        memset(RecvBuffPtr, 0, RecvBuffSize);

        Result = UnixTcpReceive(SocketFd, &l_ComTimeout, RecvBuffPtr, RecvBuffSize, &Transferred);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("UnixTcpReceive rudp header error, Result = 0x%lx, Socket %d, Try %d\n", Result, SocketFd, ErrNum);
            if (NULL != RecvBuffPtr)
            {
                free(RecvBuffPtr);
                RecvBuffPtr = NULL;
            }
            continue;
        }

        //receive successfully, jump out;
        break;
    }

    if (NULL == RecvBuffPtr)
    {
        return GMI_FAIL;
    }

    SysPkgHeaderPtr = (SysPkgHeader *)RecvBuffPtr;
    SysPkgHeaderPtr->s_SeqNum = ntohs(SysPkgHeaderPtr->s_SeqNum);
    if (0 != memcmp(GMI_SYS_MESSAGE_TAG, SysPkgHeaderPtr->s_SysMsgTag, GMI_SYS_MESSAGE_TAG_LENGTH)
            || SysPkgHeaderPtr->s_Version != SYS_COMM_VERSION
            || SysPkgHeaderPtr->s_SeqNum != seqno)
    {
        SYS_CLIENT_ERROR("packet error\n");
        Result = GMI_FAIL;
        if (NULL != RecvBuffPtr)
        {
            free(RecvBuffPtr);
            RecvBuffPtr = NULL;
        }
        return Result;
    }

    SysPkgAttrHeaderPtr = (SysPkgAttrHeader*)(RecvBuffPtr + sizeof(SysPkgHeader));
    SysPkgAttrHeaderPtr->s_Type = ntohs(SysPkgAttrHeaderPtr->s_Type);
    if (SysPkgAttrHeaderPtr->s_Type != TYPE_MESSAGECODE)
    {
        SYS_CLIENT_ERROR("attribute error, Type %d\n", SysPkgAttrHeaderPtr->s_Type);
        Result = GMI_FAIL;
        if (NULL != RecvBuffPtr)
        {
            free(RecvBuffPtr);
            RecvBuffPtr = NULL;
        }
        return Result;
    }

    SysPkgMessageCodePtr = (SysPkgMessageCode*)(RecvBuffPtr + sizeof(SysPkgHeader) + sizeof(SysPkgAttrHeader));
    SysPkgMessageCodePtr->s_MessageCode = ntohl(SysPkgMessageCodePtr->s_MessageCode);
    if (SysPkgMessageCodePtr->s_MessageCode != RETCODE_OK)
    {
        SYS_CLIENT_ERROR("packet process error\n");
        Result = GMI_FAIL;
        if (NULL != RecvBuffPtr)
        {
            free(RecvBuffPtr);
            RecvBuffPtr = NULL;
        }
        return Result;
    }

    if (NULL != RecvBuffPtr)
    {
        free(RecvBuffPtr);
        RecvBuffPtr = NULL;
    }

    return Result;
}


