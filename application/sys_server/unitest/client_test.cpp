
#include "../include/sys_env_types.h"
#include "../../../../module/gmi/rudp/include/gmi_rudp_api.h"


GMI_RESULT  FillPacketHeader(
    uint8_t	      *PacketHeader,
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
    memset( SysPkgHdPtr, 0, sizeof(SysPkgHeader) );
    memcpy( SysPkgHdPtr->s_SysMsgTag, MessageTag, sizeof(uint8_t) * GMI_SYS_MESSAGE_TAG_LENGTH);
    SysPkgHdPtr->s_Version = Version;
    SysPkgHdPtr->s_Code = MessageCode;
    SysPkgHdPtr->s_AttrCount = AttrCount;
    SysPkgHdPtr->s_TotalLen = TotalLen;
    SysPkgHdPtr->s_SessionId = SessionId;
    SysPkgHdPtr->s_SeqNum = SequenceNumber;
    SysPkgHdPtr->s_HeaderLen = sizeof(SysPkgHeader);

    return GMI_SUCCESS;
}


int main( int32_t argc, char_t *argv[])
{
    long_t Offset = 0;
    uint8_t SendBuf[1500] = {0};
    uint8_t RecvBuf[1500] = {0};
    long_t TotalLen;
    SysPkgHeader *SysPkgHdPtr;
    SysPkgAttrHeader *SysPkgAttrHdPtr;

    TotalLen = sizeof(SysPkgHeader) + sizeof(SysPkgAttrHeader);
    SysPkgHdPtr = (SysPkgHeader*)SendBuf;
    FillPacketHeader((uint8_t*)SysPkgHdPtr,
                     (uint8_t*)GMI_SYS_MESSAGE_TAG,
                     SYS_COMM_VERSION,
                     SYSCODE_GET_SYSCFG_REQ,
                     0,
                     TotalLen,
                     1,
                     1);

    PkgRudpSendInput RudpSendInput;
    PkgRudpSendOutput RudpSendOutput;
    GMI_RESULT Result;

    FD_HANDLE SockFd = GMI_RudpSocket(5678);
    if ( SockFd == NULL )
    {
        goto errExit;
    }

    memset( &RudpSendInput, 0, sizeof( PkgRudpSendInput ) );
    RudpSendInput.buffer = (char_t*)SendBuf;
    RudpSendInput.length = TotalLen;
    RudpSendInput.DstPort = 1234;
    Result = GMI_RudpSend(SockFd, &RudpSendInput, &RudpSendOutput);
    if ( FAILED( Result ) )
    {
        goto errExit;
    }

    PkgRudpRecvInput RudpRecvInput;
    PkgRudpRecvOutput RudpRecvOutput;
    RudpRecvInput.TimeoutMs = 200;
    RudpRecvOutput.buffer = (char_t*)RecvBuf;
    RudpRecvOutput.BufferLength = sizeof(RecvBuf);
    Result = GMI_RudpRecv(SockFd, &RudpRecvInput, &RudpRecvOutput);
    if ( FAILED( Result ))
    {
        printf("%s %d GMI_RudpRecv error\n", __func__, __LINE__);
        goto errExit;
    }

    SysPkgDeviceInfo *SysPkgDeviceInfoPtr;

    SysPkgDeviceInfoPtr = (SysPkgDeviceInfo*)(RecvBuf + sizeof(SysPkgHeader) + sizeof(SysPkgAttrHeader));

    printf("%s\n %d\n %s\n %s\n %s\n %s\n", SysPkgDeviceInfoPtr->s_DeviceName,
           SysPkgDeviceInfoPtr->s_DeviceId,
           SysPkgDeviceInfoPtr->s_DeviceModel,
           SysPkgDeviceInfoPtr->s_DeviceSerialNum,
           SysPkgDeviceInfoPtr->s_DeviceFwVer,
           SysPkgDeviceInfoPtr->s_DeviceHwVer);

errExit:

    exit(1);
}



