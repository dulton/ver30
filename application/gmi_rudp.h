#ifndef __GMI_UDP_H__
#define __GMI_UDP_H__
#ifdef __cplusplus
extern "C" {
#endif 

#define RUDP_VERSION 1
//used buffer max size
#define MAX_RUDP_BUFFER_SIZE 1460

#define GMI_RUDP_MESSAGE_TAG        "GRMT"
#define GMI_RUDP_MESSAGE_TAG_LENGTH 4

typedef struct _PkgRudpHeader{
    uint8_t  s_MsgTag[GMI_RUDP_MESSAGE_TAG_LENGTH];//magic
    uint16_t s_Version; 
    uint16_t s_SeqNum; 
    uint16_t s_SessionId;
    uint8_t  s_MessageType;//RUDP_MESSAGE_TYPE_PARTIAL/GMI_MESSAGE_TYPE_COMPLETE
    uint8_t  s_DataId;
    uint32_t s_AuthValue;
    uint16_t s_BodyLen;//message length
    uint16_t s_SvrPort;
    uint16_t s_PktTotalLength;//packet total length
    uint16_t s_MsgOffsetofPkt;//message offset within packet
    uint32_t s_CheckSum;
}PkgRudpHeader;

#ifdef __cplusplus
}
#endif
#endif

