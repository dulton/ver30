#ifndef __SYS_UTILITLY_UNIX_H__
#define __SYS_UTILITLY_UNIX_H__

#include "sys_command_excute.h"
#include "gmi_system_headers.h"

#ifdef __cpluscplus
extern "C"
{
#endif

    GMI_RESULT  SysFillPacketHeader(
        uint8_t       *PacketHeader,
        const uint8_t *MessageTag,
        uint8_t        Version,
        uint16_t       MessageCode,
        uint16_t       AttrCount,
        uint16_t       TotalLen,
        uint16_t       SessionId,
        uint16_t       SequenceNumber);


    GMI_RESULT SysFillPacketAttrHeader(
        uint8_t        *PacketAttrHeader,
        uint16_t        Type,
        uint16_t        AttrLen);


    GMI_RESULT SendGetCmdReq(
        uint16_t SessionId,
        uint32_t AuthValue,
        int32_t  SocketFd,
        uint16_t Code,
        uint16_t ReqAttrCnt,
        uint16_t AttrType,
        uint16_t SeqNum );

    GMI_RESULT SendSetCmdReq(
        uint16_t SessionId,
        uint32_t AuthValue,
        int32_t  SocketFd,
        uint16_t Code,
        uint16_t ReqAttrCnt,
        uint16_t AttrType,
        void_t  *Attr,
        uint16_t AttrLength,
        uint16_t SeqNum);

    GMI_RESULT SendSetCmdReqs(
        uint16_t SessionId,
        uint32_t AuthValue,
        int32_t  SocketFd,
        uint16_t Code,
        uint16_t ReqAttrCnt,
        SysAttr *SysReqAttrPtr,
        uint16_t SeqNum);

    void GMI_TimeDelay(uint8_t Sec, uint32_t Usec);

    GMI_RESULT NetReadMacChar(char_t EthName[32], char_t Mac[6]);

    unsigned long long ntohll(unsigned long long val);
    unsigned long long htonll(unsigned long long val);
    
#ifdef __cpluscplus
}
#endif
#endif



