#include <stdlib.h>
#include <arpa/inet.h>
#include "sys_utilitly_unix.h"
#include "unix_tcp_session.h"
#include "gmi_rudp.h"
#include "sys_env_types.h"
#include "gmi_system_headers.h"
#include "log.h"

GMI_RESULT FillRudpHeader(
    uint8_t  *RudpHeaderPtr,
    const uint8_t *MessageTag,
    uint16_t Version,
    uint32_t AuthValue,
    uint16_t SessionId,
    uint16_t SeqNum,
    uint16_t BodyLen)
{
    PkgRudpHeader *RudpHeader = (PkgRudpHeader*)RudpHeaderPtr;

    memset(RudpHeader, 0, sizeof(PkgRudpHeader));
    memcpy(RudpHeader->s_MsgTag, MessageTag, sizeof(uint8_t) * GMI_RUDP_MESSAGE_TAG_LENGTH);
    RudpHeader->s_AuthValue = AuthValue;
    RudpHeader->s_SeqNum    = SessionId;
    RudpHeader->s_SessionId = SeqNum;
    RudpHeader->s_Version   = Version;
    RudpHeader->s_BodyLen   = BodyLen;

    return GMI_SUCCESS;
}


GMI_RESULT  SysFillPacketHeader(
    uint8_t       *PacketHeader,
    const uint8_t *MessageTag,
    uint8_t        Version,
    uint16_t       MessageCode,
    uint16_t       AttrCount,
    uint16_t       TotalLen,
    uint16_t       SessionId,
    uint16_t       SequenceNumber)
{
    SysPkgHeader *SysPkgHdPtr;

    SysPkgHdPtr = (SysPkgHeader*)PacketHeader;
    memset(SysPkgHdPtr, 0, sizeof(SysPkgHeader));
    memcpy(SysPkgHdPtr->s_SysMsgTag, MessageTag, sizeof(uint8_t) * GMI_SYS_MESSAGE_TAG_LENGTH);
    SysPkgHdPtr->s_Version   = Version;
    SysPkgHdPtr->s_Code      = htons(MessageCode);
    SysPkgHdPtr->s_AttrCount = htons(AttrCount);
    SysPkgHdPtr->s_TotalLen  = htons(TotalLen);
    SysPkgHdPtr->s_SessionId = htons(SessionId);
    SysPkgHdPtr->s_SeqNum    = htons(SequenceNumber);
    SysPkgHdPtr->s_HeaderLen = sizeof(SysPkgHeader);

    return GMI_SUCCESS;
}


GMI_RESULT SysFillPacketAttrHeader(
    uint8_t        *PacketAttrHeader,
    uint16_t        Type,
    uint16_t        AttrLen)
{
    SysPkgAttrHeader *SysPkgAttrHdPtr;

    SysPkgAttrHdPtr = (SysPkgAttrHeader*)PacketAttrHeader;
    memset(SysPkgAttrHdPtr, 0, sizeof(SysPkgAttrHeader));
    SysPkgAttrHdPtr->s_Type   = htons(Type);
    SysPkgAttrHdPtr->s_Length = htons(AttrLen);

    return GMI_SUCCESS;
}


//ReqAttrCnt = 0 : get all attrs
//ReqAttrCnt = 1 : get AttrType
GMI_RESULT SendGetCmdReq(uint16_t SessionId, uint32_t AuthValue, int32_t SocketFd, uint16_t Code, uint16_t ReqAttrCnt, uint16_t AttrType, uint16_t SeqNum)
{
    //uint16_t ReqAttrCnt;
    uint32_t Offset = 0;
    uint32_t TotalLen;
    uint32_t Transferred;
    uint8_t  *SendBuffPtr = NULL;
    uint16_t SysPkgHdLen;
    uint16_t SysPkgAttrHdLen;
    uint16_t SysPkgAttrLen;
    GMI_RESULT Result = GMI_SUCCESS;

    //send attr
    SysPkgHdLen = sizeof(SysPkgHeader);
    SysPkgAttrHdLen = sizeof(SysPkgAttrHeader);
    SysPkgAttrLen = SysPkgAttrHdLen;
    if (ReqAttrCnt == 0)
    {
        TotalLen = SysPkgHdLen;
    }
    else
    {
        TotalLen = SysPkgHdLen + SysPkgAttrLen;
    }

    //send rudp header
    PkgRudpHeader RudpHeader;
    FillRudpHeader((uint8_t*)&RudpHeader,
                   (uint8_t*)GMI_RUDP_MESSAGE_TAG,
                   RUDP_VERSION,
                   AuthValue,
                   SessionId,
                   0,
                   TotalLen);
    Result = UnixTcpSend(SocketFd, (uint8_t*)&RudpHeader, sizeof(PkgRudpHeader), &Transferred);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("Socket %d UnixTcpSend fail, Result = 0x%lx\n", SocketFd, Result);
        return Result;
    }

    //send app packet
    SendBuffPtr = (uint8_t*)malloc(TotalLen);
    if (SendBuffPtr == NULL)
    {
        Result = GMI_OUT_OF_MEMORY;
        return Result;
    }

    SysFillPacketHeader(SendBuffPtr,
                        (uint8_t*)GMI_SYS_MESSAGE_TAG,
                        SYS_COMM_VERSION,
                        Code,
                        ReqAttrCnt,
                        TotalLen,
                        1,
                        SeqNum);

    if (ReqAttrCnt != 0)
    {
        Offset += SysPkgHdLen;
        SysFillPacketAttrHeader((SendBuffPtr + Offset),
                                AttrType,
                                SysPkgAttrLen);
    }

    Result = UnixTcpSend(SocketFd, SendBuffPtr, TotalLen, &Transferred);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("Socket %d UnixTcpSend fail, Result = 0x%lx\n", SocketFd, Result);
        if (SendBuffPtr != NULL)
        {
            free(SendBuffPtr);
            SendBuffPtr = NULL;
        }
        return Result;
    }

    if (SendBuffPtr != NULL)
    {
        free(SendBuffPtr);
        SendBuffPtr = NULL;
    }
    return Result;
}


GMI_RESULT SendSetCmdReq(
    uint16_t SessionId,
    uint32_t AuthValue,
    int32_t  SocketFd,
    uint16_t Code,
    uint16_t ReqAttrCnt,
    uint16_t AttrType,
    void_t  *Attr,
    uint16_t AttrLength,
    uint16_t SeqNum)
{
    uint32_t Offset = 0;
    uint32_t TotalLen;
    uint32_t Transferred;
    uint8_t  *SendBuffPtr = NULL;
    uint16_t SysPkgHdLen;
    uint16_t SysPkgAttrHdLen;
    uint16_t SysPkgAttrLen;
    GMI_RESULT Result = GMI_SUCCESS;

    if (ReqAttrCnt != 1)
    {
        return GMI_INVALID_PARAMETER;
    }


    SysPkgHdLen     = sizeof(SysPkgHeader);
    SysPkgAttrHdLen = sizeof(SysPkgAttrHeader);
    SysPkgAttrLen   = SysPkgAttrHdLen + AttrLength;
    TotalLen    = SysPkgHdLen + SysPkgAttrLen ;

    //send rudp header
    PkgRudpHeader RudpHeader;
    FillRudpHeader((uint8_t*)&RudpHeader,
                   (uint8_t*)GMI_RUDP_MESSAGE_TAG,
                   RUDP_VERSION,
                   AuthValue,
                   SessionId,
                   0,
                   TotalLen);
    Result = UnixTcpSend(SocketFd, (uint8_t*)&RudpHeader, sizeof(PkgRudpHeader), &Transferred);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("Socket %d UnixTcpSend fail, Result = 0x%lx\n", SocketFd, Result);
        return Result;
    }

    //send app packet
    SendBuffPtr = (uint8_t*)malloc(TotalLen);
    if (SendBuffPtr == NULL)
    {
        Result = GMI_OUT_OF_MEMORY;
        return Result;
    }

    SysFillPacketHeader(SendBuffPtr,
                        (uint8_t*)GMI_SYS_MESSAGE_TAG,
                        SYS_COMM_VERSION,
                        Code,
                        ReqAttrCnt,
                        TotalLen,
                        1,
                        SeqNum);


    Offset += SysPkgHdLen;
    SysFillPacketAttrHeader((SendBuffPtr + Offset),
                            AttrType,
                            SysPkgAttrLen);
    Offset += SysPkgAttrHdLen;
    memcpy((SendBuffPtr + Offset), Attr, AttrLength);

    Result = UnixTcpSend(SocketFd, SendBuffPtr, TotalLen, &Transferred);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("Socket %d UnixTcpSend fail, Result = 0x%lx\n", SocketFd, Result);
        if (SendBuffPtr != NULL)
        {
            free(SendBuffPtr);
            SendBuffPtr = NULL;
        }
        return Result;
    }

    if (SendBuffPtr != NULL)
    {
        free(SendBuffPtr);
        SendBuffPtr = NULL;
    }
    return Result;
}


GMI_RESULT SendSetCmdReqs(
    uint16_t SessionId,
    uint32_t AuthValue,
    int32_t  SocketFd,
    uint16_t Code,
    uint16_t ReqAttrCnt,
    SysAttr *SysReqAttrPtr,
    uint16_t SeqNum)
{
    uint8_t *SendBuffPtr = NULL;
    uint32_t Offset      = 0;
    uint32_t TotalLen;
    uint32_t Transferred;
    uint16_t SysPkgHdLen;
    uint16_t SysPkgAttrHdLen;
    uint16_t SysPkgAttrLen;
    uint16_t Id;
    GMI_RESULT Result = GMI_SUCCESS;

    if (ReqAttrCnt < 1)
    {
        return GMI_INVALID_PARAMETER;
    }

    //send attr
    SysPkgHdLen     = sizeof(SysPkgHeader);
    SysPkgAttrHdLen = sizeof(SysPkgAttrHeader);
    SysPkgAttrLen   = 0;
    for (Id = 0; Id < ReqAttrCnt; Id++)
    {
        SysPkgAttrLen += SysPkgAttrHdLen;
        SysPkgAttrLen += SysReqAttrPtr[Id].s_AttrLength;
    }

    TotalLen    = SysPkgHdLen + SysPkgAttrLen;

    //send rudp header
    PkgRudpHeader RudpHeader;
    FillRudpHeader((uint8_t*)&RudpHeader,
                   (uint8_t*)GMI_RUDP_MESSAGE_TAG,
                   RUDP_VERSION,
                   AuthValue,
                   SessionId,
                   0,
                   TotalLen);
    Result = UnixTcpSend(SocketFd, (uint8_t*)&RudpHeader, sizeof(PkgRudpHeader), &Transferred);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("Socket %d UnixTcpSend fail, Result = 0x%lx\n", SocketFd, Result);
        return Result;
    }

    //send app packet
    SendBuffPtr = (uint8_t*)malloc(TotalLen);
    if (SendBuffPtr == NULL)
    {
        Result = GMI_OUT_OF_MEMORY;
        SYS_CLIENT_ERROR("malloc fail\n");
        return Result;
    }

    Offset = 0;
    //fill packet header
    SysFillPacketHeader((SendBuffPtr + Offset),
                        (uint8_t*)GMI_SYS_MESSAGE_TAG,
                        SYS_COMM_VERSION,
                        Code,
                        ReqAttrCnt,
                        TotalLen,
                        1,
                        SeqNum);


    Offset += SysPkgHdLen;

    //fill packet body
    for (Id = 0; Id < ReqAttrCnt; Id++)
    {
        SysFillPacketAttrHeader((SendBuffPtr + Offset),
                                SysReqAttrPtr[Id].s_Type,
                                (SysPkgAttrHdLen + SysReqAttrPtr[Id].s_AttrLength));
        Offset += SysPkgAttrHdLen;
        memcpy((SendBuffPtr + Offset), SysReqAttrPtr[Id].s_Attr, SysReqAttrPtr[Id].s_AttrLength);
        Offset += SysReqAttrPtr[Id].s_AttrLength;
    }

    //write
    Result = UnixTcpSend(SocketFd, SendBuffPtr, TotalLen, &Transferred);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("Socket %d UnixTcpSend fail, Result = 0x%lx\n", SocketFd, Result);
        if (SendBuffPtr != NULL)
        {
            free(SendBuffPtr);
            SendBuffPtr = NULL;
        }
        return Result;
    }

    if (SendBuffPtr != NULL)
    {
        free(SendBuffPtr);
        SendBuffPtr = NULL;
    }
    return Result;
}


void GMI_TimeDelay(uint8_t Sec, uint32_t Usec)
{
    struct timeval Delay;

    Delay.tv_sec  = Sec;
    Delay.tv_usec = Usec;

    select(0, NULL, NULL, NULL, &Delay);

    return;
}


GMI_RESULT NetReadMacChar(char_t EthName[32], char_t Mac[6])
{
    int32_t            Sock;
    struct sockaddr_in Sin;
    struct ifreq       Ifr;

    if (NULL == Mac)
    {
        return GMI_INVALID_PARAMETER;
    }

    Sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (0 > Sock)
    {
        perror("socket");
        return GMI_FAIL;
    }

    strncpy(Ifr.ifr_name, EthName, IFNAMSIZ);
    Ifr.ifr_name[IFNAMSIZ - 1] = 0;

    memcpy(&Sin, &Ifr.ifr_addr, sizeof(Sin));
    if (0 > ioctl(Sock, SIOCGIFHWADDR, &Ifr))
    {
        perror("ioctl");
        return GMI_FAIL;
    }

    Mac[0] = Ifr.ifr_hwaddr.sa_data[0];
    Mac[1] = Ifr.ifr_hwaddr.sa_data[1];
    Mac[2] = Ifr.ifr_hwaddr.sa_data[2];
    Mac[3] = Ifr.ifr_hwaddr.sa_data[3];
    Mac[4] = Ifr.ifr_hwaddr.sa_data[4];
    Mac[5] = Ifr.ifr_hwaddr.sa_data[5];

    close(Sock);

    return GMI_SUCCESS;
}


unsigned long long ntohll(unsigned long long val)
{
    return (((unsigned long long )htonl((int)((val << 32) >> 32))) << 32) | (unsigned int)htonl((int)(val >> 32));
}


unsigned long long htonll(unsigned long long val)
{
    return (((unsigned long long )htonl((int)((val << 32) >> 32))) << 32) | (unsigned int)htonl((int)(val >> 32));    
}


