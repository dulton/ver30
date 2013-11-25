#ifndef __GMI_UDP_API_H__
#define __GMI_UDP_API_H__

#include "gmi_system_headers.h"

#ifdef __cplusplus
extern "C" {
#endif

//once transfer, user data max size
#define RUDP_MAXPKTSIZE 1400
//user data max size 
#define RUDP_MAX_PACKAGE_SIZE  (62*1024)

//default timeout value , ms
#define RUDP_DEFAULT_TIMEOUTMS 10 

#define RUDP_MESSAGE_TYPE_COMPLETE 0
#define RUDP_MESSAGE_TYPE_PARTIAL  1

//Send: Input param
typedef struct 
{	
    uint32_t s_AuthValue;
    uint16_t s_SessionId; 
	uint8_t  s_MessageType;//RUDP_MESSAGE_TYPE_COMPLETE/RUDP_MESSAGE_TYPE_PARTIAL, if s_MessageType is RUDP_MESSAGE_TYPE_COMPLETE， ignore s_PktTotalLength and s_MsgOffsetOfPkt.
    uint8_t *s_Buffer;//message buffer
    uint32_t s_SendLength;//message length
    uint32_t s_RemotePort;//remote destination port
    uint32_t s_TimeoutMS;//timeout, millisecond
    uint32_t s_LocalSvrPort;// Tell remote end own server port, if user do not want to use it, can set this variable zero.
	uint32_t s_PktTotalLength;//packet total length
	uint32_t s_MsgOffsetofPkt;//message offset within packet
    uint16_t s_SequenceNum;
    uint8_t  s_Reserved[2];
}PkgRudpSendInput;

//Send: Output param
typedef struct 
{
    uint32_t s_RealSendBytes;
}PkgRudpSendOutput;

//Recv: Input param
typedef struct
{ 
    uint32_t s_TimeoutMS;
}PkgRudpRecvInput;

//Recv: Output param
typedef struct
{
	uint16_t s_SessionId;
	uint32_t s_AuthValue;
	uint8_t  s_MessageType;//RUDP_MESSAGE_TYPE_COMPLETE/RUDP_MESSAGE_TYPE_PARTIAL, if s_MessageType is RUDP_MESSAGE_TYPE_COMPLETE， ignore s_PktTotalLength and s_MsgOffsetOfPkt.
    uint8_t *s_Buffer;//message buffer
    uint32_t s_BufferLength;//message buffer length
    uint32_t s_RecvLength;//message length
    uint32_t s_RemoteSvrPort;//remote server port
	uint32_t s_PktTotalLength;//packet total length
	uint32_t s_MsgOffsetofPkt;//message offset within packet
    uint16_t s_SequenceNum;
    uint8_t  s_Reserved[2];
}PkgRudpRecvOutput;


/*===============================================================
func name:GMI_RudpSocket
func:rudp socket create
input:local port
return:success--return socket handler, failed -- return NULL
--------------------------------------------------------------------*/
FD_HANDLE GMI_RudpSocket(uint32_t LocalPort);


/*===============================================================
func name:GMI_RudpSocketClose
func:rudp socket close
input:socket handle
return: return void
--------------------------------------------------------------------*/
void GMI_RudpSocketClose(FD_HANDLE SockFd);


/*===============================================================
func name:GMI_RudpSend
func:udpsend with timeout
input:RudpSendInput--send input information, for example, user data pointer, user datalength, timeout value, dstination port;
        RudpSendOutput -- send sucessfully, output information;
return:success--return GMI_SUCCESSr, 
	failed -- return GMI_FAIL, 
	param error -- return GMI_INVALID_PARAMETER
	send timeout -- return GMI_WAIT_TIMEOUT
---------------------------------------------------------------------*/
GMI_RESULT GMI_RudpSend(FD_HANDLE SockFd, PkgRudpSendInput *RudpSendInput, PkgRudpSendOutput *RudpSendOutput);


/*===============================================================
func name:GMI_RudpRecv
func:udprecv with timeout
input:RudpRecvInput--recv input information,  timeout value
        RudpRecvOutput -- recv sucessfully, output information, for example, received user data, received data length;
return:success--return GMI_SUCCESSr, 
	failed -- return GMI_FAIL, 
	param error -- return GMI_INVALID_PARAMETER
	recv timeout -- return GMI_WAIT_TIMEOUT
---------------------------------------------------------------------*/
GMI_RESULT GMI_RudpRecv(FD_HANDLE SockFd, PkgRudpRecvInput *RudpRecvInput, PkgRudpRecvOutput *RudpRecvOutput);


#ifdef __cplusplus
}
#endif

#endif


