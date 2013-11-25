#ifndef __SYS_COMMAND_EXCUTE_H__
#define __SYS_COMMAND_EXCUTE_H__

#include "gmi_system_headers.h"

typedef struct
{
    uint16_t  s_Type;
    void_t   *s_Attr;
    uint16_t  s_AttrLength;
    void_t   *s_FreePtr;
} SysAttr;

typedef GMI_RESULT (*FuncGetMsg)(void_t *SessionHdPtr, uint32_t MsgType, uint16_t AttrType, uint16_t AttrLength, uint16_t *AttrCntPtr, SysAttr **SysAttrRspPtr );
typedef struct
{
    uint32_t       s_MsgType;
    uint16_t       s_AttrType;
    uint16_t       s_AttrLength;
    FuncGetMsg     s_FuncMsg;
} GetMsgItem;

GMI_RESULT SysCmdInit();
GMI_RESULT SysCmdInitExt(struct timeval *TimeoutPtr, uint16_t TryCount);
GMI_RESULT SysCmdInit(uint16_t LocalSysRudp_ClientPort_Start, uint16_t LocalSysRudp_ClientPort_End, \
                      uint16_t LocalSysRudp_ServerPort_Start, uint16_t LocalSysRudp_ServerPort_End,
                      uint16_t RemoteSysRudp_Port);
GMI_RESULT SysCmdInit(uint16_t LocalRudpCPort, uint16_t LocalRudpSPort, uint16_t RemotRudpPort);
GMI_RESULT SysGetCmdExcuteWithAttrs(uint16_t SessionId, uint32_t AuthValue, uint16_t Code, uint16_t ReqAttrCnt, SysAttr *SysReqAttrPtr, uint16_t *RspAttrCntPtr, SysAttr **SysRspAttrPtr);
GMI_RESULT SysGetCmdExcute(uint16_t SessionId, uint32_t AuthValue, uint16_t Code, uint16_t *RspAttrCntPtr, SysAttr **SysRspAttrPtr);
void SysGetCmdAttrFree(uint16_t RspAttrCnt, SysAttr *SysRspAttrPtr);
GMI_RESULT SysSetCmdExcute(uint16_t SessionId, uint32_t AuthValue, uint16_t Code, uint16_t ReqAttrCntPtr, SysAttr *SysReqAttrPtr);
void SysCmdUnInit();

#endif

