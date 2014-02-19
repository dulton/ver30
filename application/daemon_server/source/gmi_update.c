/******************************************************************************
modules		:    Daemon
name		:    gmi_update.c
function	:    system update server
author		:    minchao.wang
version		:    1.0.0.0.0
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/29/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
#include "gmi_includes.h"
#include "gmi_update.h"
#include "gmi_system.h"
#include "gmi_struct.h"
#include "gmi_config.h"
#include "gmi_daemon_thread.h"
#include "gmi_update.h"
#include "gmi_system_headers.h"
#include "gmi_common.h"
#include "gmi_debug.h"
#include "md5.h"
#include "gmi_brdwrapper.h"
#include "ipc_fw_v3.x_setting.h"
#include "ipc_fw_v3.x_resource.h"
#include "gmi_config_api.h"
#include "sys_info_readonly.h"

static int32_t l_SvrFd = -1;
static int32_t l_iStopTCPServer = 0;
static uint32_t l_Count = 0;
static pthread_mutex_t l_LockUpdateSys   = PTHREAD_MUTEX_INITIALIZER;

static boolean_t l_UpdateFlags = true;

#define GET_KERNEL_VERSION		"cat /proc/version | grep Number: | awk '{print $3}'"
#define GET_ROOTFS_VERSION		"cat /etc/version | grep Number: | awk '{print $3}'"


GMI_RESULT GMI_GetMacInfo(char_t *EthName, char_t *Mac)
{
    int32_t            Sock;
    struct sockaddr_in Sin;
    struct ifreq       Ifr;

    if (NULL == Mac)
    {
        return GMI_INVALID_PARAMETER;
    }

    Sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (Sock < 0)
    {
        perror("socket");
        return GMI_FAIL;
    }

    strncpy(Ifr.ifr_name, EthName, IFNAMSIZ);
    Ifr.ifr_name[IFNAMSIZ - 1] = 0;

    memcpy(&Sin, &Ifr.ifr_addr, sizeof(Sin));
    if (ioctl(Sock, SIOCGIFHWADDR, &Ifr) < 0)
    {
        perror("ioctl");
        return GMI_FAIL;
    }

    sprintf(Mac, "%02x:%02x:%02x:%02x:%02x:%02x", \
            (uint8_t)Ifr.ifr_hwaddr.sa_data[0], (uint8_t)Ifr.ifr_hwaddr.sa_data[1], (uint8_t)Ifr.ifr_hwaddr.sa_data[2], (uint8_t)Ifr.ifr_hwaddr.sa_data[3], (uint8_t)Ifr.ifr_hwaddr.sa_data[4], (uint8_t)Ifr.ifr_hwaddr.sa_data[5]);

    close(Sock);

    return GMI_SUCCESS;
}

/*=======================================================
name				:	GMI_GetVersion
function			:  Get System SVN version .
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     cmd  :cmd kernel version and rootfs version
                                              buf: config path
                                              length : receive platform string
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	7/22/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_GetVersion(const char_t *Cmd, char_t *Buf,int32_t Length)
{
    if(NULL == Cmd)
    {
        return GMI_INVALID_PARAMETER;
    }

    FILE *Fp = popen(Cmd, "r");
    if ( NULL == Fp )
    {
        return GMI_FAIL;
    }

    int32_t retval = fread(Buf, 1, Length, Fp);
    pclose(Fp);
    if ( retval < 0 )
    {
        return GMI_FAIL;
    }

    if(retval < Length)
        Buf[retval] = '\0';
    else
        Buf[Length] = '\0';

    return GMI_SUCCESS;
}

/*=======================================================
name				:	GMI_GetPlatformConfig
function			:  Get System Platform .
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     FileName  :config file
                                              ItemPath: config path
                                              platform : receive platform string
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	7/9/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_GetPlatformConfig(const char_t * FileName, const char_t *ItemPath, char_t *Platform)
{
    if(NULL == FileName || NULL == ItemPath)
    {
        return GMI_INVALID_PARAMETER;
    }

    DAEMON_PRINT_LOG(INFO,"GMI_GetPlatformConfig  Start!! ");

    GMI_RESULT Result = SysInfoReadInitialize();
    if (FAILED(Result))
    {
        return Result;
    }

    FD_HANDLE Handle;
    Result = SysInfoOpen(FileName, &Handle);
    if (FAILED(Result))
    {
        SysInfoReadDeinitialize();
        return Result;
    }

    char_t	Value[MIN_BUFFER_LENGTH] = {"0"};
    memset(Value, 0 ,sizeof(Value));
    Result = SysInfoRead(Handle, ItemPath, HW_CPU_KEY,  "A5S_66", Value);
    if (FAILED(Result))
    {
        SysInfoClose(Handle);
        SysInfoReadDeinitialize();
        return Result;
    }
    else
    {
        strcpy(Platform,Value);
    }

    SysInfoClose(Handle);
    SysInfoReadDeinitialize();

    return Result;
}

/*=======================================================
name				:	GMI_TCPReceive
function			:  TCP Receive function.
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     ifd  :socket id
                                              Buffer: receive buffer
                                              ulBufferSize : receive file length
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/13/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
long_t GMI_TCPReceive(
    int32_t           Fd,
    char_t          *Buffer,
    ulong_t BufferSize)
{
    ulong_t Left = BufferSize;
    ulong_t Read = 0;
    char_t *StartAddr     = Buffer;
    struct timeval Timeout;
    fd_set FdSet;

    while (Left > 0)
    {
        Timeout.tv_sec  = NETWORK_TIMEOUT;
        Timeout.tv_usec = 0;
        FD_ZERO(&FdSet);
        FD_SET(Fd, &FdSet);

        if (select(Fd+1, &FdSet, NULL, NULL, &Timeout) < 0)
        {
            return -1;
        }
        if (FD_ISSET(Fd, &FdSet))
        {
            Read = recv(Fd, StartAddr, Left, 0);
            if (Read < 0)
            {
                if (errno == EINTR)
                {
                    Read = 0;
                }
                else
                {
                    return -1;
                }
            }
            else if (Read == 0)
            {
                break;
            }
            Left     -= Read;
            StartAddr += Read;
        }
    }

    return Left;
}

/*=======================================================
name				:	GMI_GetDeviceIpAddr
function			:  Get Device IP addr.
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     Ip  :Device Ip
                                              EthName: Device eth Name
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/3/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_GetDeviceIpAddr(char_t *Ip, const char_t *EthName)
{
    if(NULL == EthName)
    {
        return GMI_INVALID_PARAMETER;
    }

    int32_t Sock;
    Sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (Sock == -1)
    {
        DAEMON_PRINT_LOG(ERROR,"socket fail !!!!!");
        return GMI_FAIL;
    }

    struct ifreq Ifr;
    strncpy(Ifr.ifr_name, EthName, IFNAMSIZ);
    Ifr.ifr_name[IFNAMSIZ - 1] = 0;
    if (ioctl(Sock, SIOCGIFADDR, &Ifr) < 0)
    {
        DAEMON_PRINT_LOG(ERROR,"socket ioctl !!!!!");
        return GMI_FAIL;
    }

    struct sockaddr_in Sin;
    memcpy(&Sin, &Ifr.ifr_addr, sizeof(Sin));
    strcpy(Ip , inet_ntoa(Sin.sin_addr));

    close(Sock);
    return GMI_SUCCESS;
}

/*=======================================================
name				:	GMI_GetDeviceIpAddr
function			:  Get Device IP addr.
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     Ip  :Device Ip
                                              EthName: Device eth Name
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/3/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
void GMI_MakeMessageHeader(UpdateBaseMessage *UpMessage, uint8_t MesType, uint16_t MesId, uint16_t MesLen)
{
    memcpy(UpMessage->s_MessageTag, GMI_UPDATE_MESSAGE_TAG, 4);
    UpMessage->s_HeaderFlags = 0;
    UpMessage->s_HeaderExtensionSize = 0;
    UpMessage->s_MajorVersion = GMI_UPDATE_MESSAGE_MAJOR_VERSION;
    UpMessage->s_MinorVersion =GMI_UPDATE_MESSAGE_MINOR_VERSION;
    UpMessage->s_SequenceNumber = htons(0);  //srand  create
    UpMessage->s_PaddingByteNumber = 0;
    UpMessage->s_MessageType = MesType;
    UpMessage->s_MessageId = htons(MesId);
    UpMessage->s_MessageLength = htons(MesLen);
}

/*=======================================================
name				:	GMI_ReportUpdateListResult
function			:  Report Update list Result
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/3/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_ReportUpdateListResult(char_t *Buf, uint16_t *BufLen, const char_t *Ip, int32_t Result)
{
    if (NULL == Ip)
    {
        return GMI_INVALID_PARAMETER;
    }

    uint16_t Len=0;

    if (GMI_REPORT_UPDATE_OK == Result)
    {
        Len += sprintf(Buf+Len, "<update from=\"%s\"  type=\"list\" code=\"0\">", Ip);
    }
    else if (GMI_REPORT_UPDATE_ERROR == Result)
    {
        Len += sprintf(Buf+Len, "<update from=\"%s\"  type=\"list\" code=\"20001025\">", Ip);
    }

    Len += sprintf(Buf+Len, "<estimatetime>138</estimatetime>");
    Len += sprintf(Buf+Len, "</update>");

    *BufLen = Len;

    return GMI_SUCCESS;
}

/*=======================================================================
name				:	GMI_ReportUpdateStatus
function			:  Report Update status buf package
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/3/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_ReportUpdateStatus(char_t *Buf, uint16_t *BufLen, const char_t *Ip, int32_t Type, int32_t  Schedule)
{

    if (NULL == Ip)
    {
        return GMI_INVALID_PARAMETER;
    }

    char_t Status[MIN_BUFFER_LENGTH]= {"0"};
    switch (Type)
    {
    case GMI_UPDATE_STATUS_DOWNLOAD:
        strcpy(Status,"downloading");
        break;
    case GMI_UPDATE_STATUS_UPDATING:
        strcpy(Status,"updating");
        break;
    case GMI_UPDATE_STATUS_UPDATED:
        strcpy(Status,"updatd");
        break;
    case GMI_UPDATE_STATUS_REBOOTING:
        strcpy(Status,"rebooting");
        break;
    case GMI_UPDATE_STATUS_RETRY:
        strcpy(Status,"retry");
        break;
    case GMI_UPDATE_STATUS_DISCONNECT:
        strcpy(Status,"disconnect");
        break;
    case GMI_UPDATE_STATUS_DONE:
        strcpy(Status,"downloaded");
        break;
    case GMI_UPDATE_STATUS_UPDATE_ERROR:
        strcpy(Status,"update error");
        break;
    case GMI_UPDATE_STATUS_MESSAGE_ERROR:
        strcpy(Status,"message error");
        break;
    default:
        return GMI_NOT_SUPPORT;
    }

    if (Schedule < 0)
    {
        Schedule = 0;
    }
    else if (Schedule > 100)
    {
        Schedule = 100;
    }

    uint16_t Len=0;

    Len += sprintf(Buf+Len, "<update from=\"%s\" type=\"notify\">", Ip);
    Len += sprintf(Buf+Len, "<status>%s</status>",Status);
    Len += sprintf(Buf+Len, "<progress>%d</progress>",Schedule);
    Len += sprintf(Buf+Len, "</update>");

    *BufLen = Len;

    return GMI_SUCCESS;

}


/*=======================================================
name				:	GMI_ReportRootStatus
function			:  Report Update status buf package
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/3/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_ReportRootStatus(char_t *Buf, uint16_t *BufLen, const char_t *Ip)
{
    if (NULL == Ip)
    {
        return GMI_INVALID_PARAMETER;
    }

    uint16_t Len = 0;
    Len += sprintf(Buf+Len, "<update from=\"%s\" type=\"notify\">", Ip);
    Len += sprintf(Buf+Len, "<status>reboot</status>");
    Len += sprintf(Buf+Len, "</update>");
    *BufLen = Len;

    return GMI_SUCCESS;
}

/*=======================================================
name				:	GMI_RequestDownloadFile
function			:  Request Download File package
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/3/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_RequestDownloadFile(char_t *Buf, uint16_t *BufLen, const char_t *Ip, const char_t  *Version, const char_t *Name, int32_t Offset)
{

    if (NULL==Ip || NULL==Version || NULL==Name)
    {
        return GMI_INVALID_PARAMETER;
    }

    uint16_t Len=0;

    Len += sprintf(Buf+Len, "<update from=\"%s\" type=\"download\">", Ip);
    Len += sprintf(Buf+Len, "<version>%s</version>",Version);
    Len += sprintf(Buf+Len, "<filename>%s</filename>",Name);
    Len += sprintf(Buf+Len, "<offset>%d</offset>",Offset);
    Len += sprintf(Buf+Len, "</update>");

    *BufLen = Len;

    return GMI_SUCCESS;
}

/*=======================================================
name				:	GMI_XmlComPart
function			:  Create System update Server
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/29/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_XmlComPart(char_t *Buf, const char_t *MatchStr1, const char_t *MatchStr2, char_t *Result)
{

    if ((NULL == Buf) ||(NULL == MatchStr1) ||(NULL == MatchStr2))
    {
        return GMI_INVALID_PARAMETER;
    }

    char_t *CurPos = NULL;
    char_t *SecPos = NULL;
    int32_t Len = 0;

    CurPos = strstr(Buf, MatchStr1);
    CurPos += strlen(MatchStr1);
    SecPos = strstr(CurPos, MatchStr2);
    Len = SecPos - CurPos;
    if (Len > 0)
    {
        strncpy(Result, CurPos, Len);
        Result[Len] = '\0';
    }

    DAEMON_PRINT_LOG(ERROR,"GMI_XmlComPart result is %s! ! ",Result);

    return GMI_SUCCESS;
}

/*=======================================================
name				:	GMI_XmlComIntercept
function			:  Intercept Recieve buf , base on  special character comparison.
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     Buf  :character buf in
                                              matchStr1   matchStr2 : special character
                                             result : Intercept string
                                             intercept: remain string
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/13/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_XmlComIntercept(char_t *Buf, const char_t *MatchStr1, const char_t *MatchStr2, char_t *Result, char_t *Intercept)
{

    if ((NULL == Buf)
            ||(NULL == MatchStr1)
            ||(NULL == MatchStr2)
            ||(NULL == Result))
    {
        printf("GMI_XmlComIntercept  error.\n");
        return GMI_INVALID_PARAMETER;
    }

    char_t *CurPos = NULL;
    char_t *SecPos = NULL;
    int32_t BufLen = 0;
    int32_t Length = 0;

    BufLen = strlen(Buf);
    CurPos = strstr(Buf, MatchStr1);
    SecPos = strstr(CurPos, MatchStr2);
    Length = SecPos - CurPos;
    if (Length > 0)
    {
        strncpy(Result, CurPos, Length);
        Result[Length] = '\0';
    }

    int32_t TmpLen = 0;
    TmpLen = BufLen - Length;
    if (TmpLen > 0)
    {
        strncpy(Intercept, SecPos+7, TmpLen);
        Intercept[TmpLen] = '\0';
    }

    return GMI_SUCCESS;
}

/*=======================================================
name				:	GMI_GetXmlFileContent
function			:  Get struct SystemUpdateData , from SrcBuf .
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     SrcBuf  : Srcouce buf
                                              Name : type Name
                                             SystemUpdateData : System update struct

return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/13/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_GetXmlFileContent(char_t *SrcBuf, const char_t *Name, SystemUpdateData *UpMessage)
{

    if (NULL == SrcBuf || NULL == Name)
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_GetXmlFileContent Error! ! ");
        return GMI_INVALID_PARAMETER;
    }

    int32_t Type = GMI_UPDATE_FILE_TYPE_NOTSUPPORT;
    if (0 == strcmp(Name,"kernel.bin"))
    {
        DAEMON_PRINT_LOG(INFO,"Update Name is  kernel.bin! ! ");
        Type = GMI_UPDATE_FILE_TYPE_KENREL;
    }
    else if (0 == strcmp(Name,"rootfs.bin"))
    {
        DAEMON_PRINT_LOG(INFO,"Update Name is  rootfs.bin! ! ");
        Type = GMI_UPDATE_FILE_TYPE_ROOTFS;
    }
    else if (0 == strcmp(Name,"app.bin"))
    {
        DAEMON_PRINT_LOG(INFO,"Update Name is  app.bin! ! ");
        Type = GMI_UPDATE_FILE_TYPE_APP;
    }
    else
    {
        DAEMON_PRINT_LOG(ERROR," Name is  %s!!!! ! ",Name);
        return GMI_FAIL;
    }

    char_t tmpBuf[MIN_BUFFER_LENGTH]= {"0"};
    GMI_RESULT Ret = GMI_FAIL;
    int32_t Length = 0;

    memset(tmpBuf, 0, sizeof(tmpBuf));
    if (Type  == GMI_UPDATE_FILE_TYPE_KENREL)
    {
        DAEMON_PRINT_LOG(INFO,"Update Name is  Kernel! ! ");
        UpMessage->s_KernelFlags = GMI_UPDATE_START_FLAGS;

        Ret = GMI_XmlComPart(SrcBuf,"<file name=\"","\"><size>",tmpBuf);
        if (SUCCEEDED(Ret))
        {
            Length = strlen(tmpBuf);
            strncpy(UpMessage->s_KernelName, tmpBuf, Length);
            UpMessage->s_KernelName[Length] = '\0';
        }
        else
        {
            DAEMON_PRINT_LOG(ERROR," Get Kernal Name  Error !!!! ! ");
            return Ret;
        }
        memset(tmpBuf, 0, sizeof(tmpBuf));
        Ret = GMI_XmlComPart(SrcBuf,"<size>","</size>",tmpBuf);
        if (SUCCEEDED(Ret))
        {
            UpMessage->s_KernelSize = atoi(tmpBuf);
        }
        else
        {
            DAEMON_PRINT_LOG(ERROR," Get Kernal Size Error !!!! ! ");
            return Ret;
        }

        memset(tmpBuf, 0, sizeof(tmpBuf));
        Ret = GMI_XmlComPart(SrcBuf,"<version>","</version>",tmpBuf);
        if (SUCCEEDED(Ret))
        {
            Length = strlen(tmpBuf);
            strncpy(UpMessage->s_KernelVersion, tmpBuf, Length);
            UpMessage->s_KernelVersion[Length] = '\0';
        }
        else
        {
            DAEMON_PRINT_LOG(ERROR," Get Kernal version Error !!!! ! ");
            return Ret;
        }

        memset(tmpBuf, 0, sizeof(tmpBuf));
        Ret = GMI_XmlComPart(SrcBuf,"<md5>","</md5>",tmpBuf);
        if (SUCCEEDED(Ret))
        {
            Length = strlen(tmpBuf);
            strncpy(UpMessage->s_KernelMd5, tmpBuf, Length);
        }
        else
        {
            DAEMON_PRINT_LOG(ERROR," Get Kernal Md5 Error !!!! ! ");
            return Ret;
        }

    }
    else if (Type ==  GMI_UPDATE_FILE_TYPE_ROOTFS)
    {
        UpMessage->s_RootfsFlags = GMI_UPDATE_START_FLAGS;
        Ret = GMI_XmlComPart(SrcBuf,"<file name=\"","\"><size>",tmpBuf);
        if (SUCCEEDED(Ret))
        {
            Length = strlen(tmpBuf);
            strncpy(UpMessage->s_RootfsName, tmpBuf, Length);
            UpMessage->s_RootfsName[Length] = '\0';
        }
        else
        {
            DAEMON_PRINT_LOG(ERROR," Get Rootfs Name  Error !!!! ! ");
            return Ret;
        }

        memset(tmpBuf, 0, sizeof(tmpBuf));
        Ret = GMI_XmlComPart(SrcBuf,"<size>","</size>",tmpBuf);
        if (SUCCEEDED(Ret))
        {
            UpMessage->s_RootfsSize = atoi(tmpBuf);
        }
        else
        {
            DAEMON_PRINT_LOG(ERROR," Get Rootfs size  Error !!!! ! ");
            return Ret;
        }

        memset(tmpBuf, 0, sizeof(tmpBuf));
        Ret = GMI_XmlComPart(SrcBuf,"<version>","</version>",tmpBuf);
        if (SUCCEEDED(Ret))
        {
            Length = strlen(tmpBuf);
            strncpy(UpMessage->s_RootfsVersion, tmpBuf, Length);
            UpMessage->s_RootfsVersion[Length] = '\0';
        }
        else
        {
            DAEMON_PRINT_LOG(ERROR," Get Rootfs version  Error !!!! ! ");
            return Ret;
        }

        memset(tmpBuf, 0, sizeof(tmpBuf));
        Ret = GMI_XmlComPart(SrcBuf,"<md5>","</md5>",tmpBuf);
        if (SUCCEEDED(Ret))
        {
            Length = strlen(tmpBuf);
            strncpy(UpMessage->s_RootfsMd5, tmpBuf, Length);
        }
        else
        {
            DAEMON_PRINT_LOG(ERROR," Get Rootfs md5  Error !!!! ! ");
            return Ret;
        }

    }
    else if (Type == GMI_UPDATE_FILE_TYPE_APP)
    {
        UpMessage->s_AppFlags = GMI_UPDATE_START_FLAGS;
        Ret = GMI_XmlComPart(SrcBuf,"<file name=\"","\"><size>",tmpBuf);
        if (SUCCEEDED(Ret))
        {
            Length = strlen(tmpBuf);
            strncpy(UpMessage->s_AppName, tmpBuf, Length);
            UpMessage->s_AppName[Length] = '\0';
        }
        else
        {
            DAEMON_PRINT_LOG(ERROR," Get App Name  Error !!!! ! ");
            return Ret;
        }

        memset(tmpBuf, 0, sizeof(tmpBuf));
        Ret = GMI_XmlComPart(SrcBuf,"<size>","</size>",tmpBuf);
        if (SUCCEEDED(Ret))
        {
            UpMessage->s_AppSize = atoi(tmpBuf);
        }
        else
        {
            DAEMON_PRINT_LOG(ERROR," Get App size  Error !!!! ! ");
            return Ret;
        }

        memset(tmpBuf, 0, sizeof(tmpBuf));
        Ret = GMI_XmlComPart(SrcBuf,"<version>","</version>",tmpBuf);
        if (SUCCEEDED(Ret))
        {
            Length = strlen(tmpBuf);
            strncpy(UpMessage->s_AppVersion, tmpBuf, Length);
            UpMessage->s_AppVersion[Length] = '\0';
        }
        else
        {
            DAEMON_PRINT_LOG(ERROR," Get App version  Error !!!! ! ");
            return Ret;
        }

        memset(tmpBuf, 0, sizeof(tmpBuf));
        Ret = GMI_XmlComPart(SrcBuf,"<md5>","</md5>",tmpBuf);
        if (SUCCEEDED(Ret))
        {
            Length = strlen(tmpBuf);
            strncpy(UpMessage->s_AppMd5, tmpBuf, Length);
        }
        else
        {
            DAEMON_PRINT_LOG(ERROR," Get App md5  Error !!!! ! ");
            return Ret;
        }
    }

    DAEMON_PRINT_LOG(INFO,"GMI_GetXmlFileContent OKl! ! ");

    return GMI_SUCCESS;
}

/*=======================================================
name				:	GMI_UpdateBufferAnalyze
function			:  Analyze recieve Update Buffer
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     SrcBuf  : Srcouce buf


return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/13/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_UpdateBufferAnalyze(char_t *SrcBuf, SystemUpdateData *UpMessage)
{

    char_t tmpBuf1[FILE_BUFFER_LENGTH] = {"0"};
    char_t tmpBuf2[FILE_BUFFER_LENGTH] = {"0"};
    char_t tmpBuf3[FILE_BUFFER_LENGTH] = {"0"};
    char_t dstBuf[FILE_BUFFER_LENGTH] = {"0"};
    char_t TmpBuf[FILE_BUFFER_LENGTH] = {"0"};

    memset(tmpBuf1, 0, sizeof(tmpBuf1));
    memset(tmpBuf2, 0, sizeof(tmpBuf2));
    memset(tmpBuf3, 0, sizeof(tmpBuf3));
    memset(dstBuf, 0, sizeof(dstBuf));

    GMI_RESULT Ret = GMI_FAIL;
    int32_t Length = 0;

    Ret = GMI_XmlComPart(SrcBuf,"<update from=\"","\" type=",dstBuf);
    if (SUCCEEDED(Ret))
    {
        Length =  strlen(dstBuf);
        strncpy(UpMessage->s_ServerIp, dstBuf, Length);
        UpMessage->s_ServerIp[Length] = '\0';
    }
    else
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_XmlComPart Error! ! ");
        return Ret;
    }

    memset(dstBuf, 0, sizeof(dstBuf));
    memset(TmpBuf, 0, sizeof(TmpBuf));
    Ret = GMI_XmlComPart(SrcBuf,"type=\"","\">",dstBuf);
    if (SUCCEEDED(Ret))
    {
        Length =  strlen(dstBuf);
        strncpy(TmpBuf, dstBuf, Length);
        TmpBuf[Length] = '\0';
        if (0 == strcmp(TmpBuf,"list"))
        {
            UpMessage->s_type = 0;
        }
    }
    else
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_XmlComPart Error! ! ");
        return Ret;
    }

    memset(dstBuf, 0, sizeof(dstBuf));
    Ret = GMI_XmlComPart(SrcBuf,"<version name=\"","\" /><filelist",dstBuf);
    if (SUCCEEDED(Ret))
    {
        Length =  strlen(dstBuf);
        strncpy(UpMessage->s_Version, dstBuf, Length);
        UpMessage->s_Version[Length] = '\0';
    }
    else
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_XmlComPart Error! ! ");
        return Ret;
    }

    int32_t count = 0;

    memset(dstBuf, 0, sizeof(dstBuf));
    Ret = GMI_XmlComPart(SrcBuf,"<filelist count=\"","\" totalsize",dstBuf);
    if (SUCCEEDED(Ret))
    {
        UpMessage->s_Count =atoi(dstBuf);
        count = UpMessage->s_Count;
        l_Count = UpMessage->s_Count;
    }
    else
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_XmlComPart Error! ! ");
        return Ret;
    }

    memset(dstBuf, 0, sizeof(dstBuf));
    Ret = GMI_XmlComPart(SrcBuf,"totalsize=\"","\"><file name",dstBuf);
    if (SUCCEEDED(Ret))
    {
        UpMessage->s_TotalSize = atoi(dstBuf);
    }
    else
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_XmlComPart Error! ! ");
        return Ret;
    }

    switch (count)
    {
    case 3:
        memset(dstBuf, 0, sizeof(dstBuf));
        Ret  = GMI_XmlComIntercept(SrcBuf,"<file ","</file>",dstBuf,tmpBuf1);
        if (SUCCEEDED(Ret))
        {
            memset(TmpBuf, 0, sizeof(TmpBuf));
            Ret = GMI_XmlComPart(dstBuf,"<file name=\"","\"><size>",TmpBuf);
            if (SUCCEEDED(Ret))
            {
                Ret = GMI_GetXmlFileContent(dstBuf,TmpBuf,UpMessage);
                if (FAILED(Ret))
                {
                    DAEMON_PRINT_LOG(ERROR,"GMI_GetXmlFileContent Error! ! ");
                    return GMI_FAIL;
                }
            }
        }
        else
        {
            DAEMON_PRINT_LOG(ERROR,"GMI_XmlComPart Error! ! ");
            return Ret;
        }

        Ret  = GMI_XmlComIntercept(tmpBuf1,"<file ","</file>",tmpBuf2,tmpBuf3);
        if (SUCCEEDED(Ret))
        {
            memset(TmpBuf, 0, sizeof(TmpBuf));
            Ret = GMI_XmlComPart(tmpBuf2,"<file name=\"","\"><size>",TmpBuf);
            if (SUCCEEDED(Ret))
            {
                Ret = GMI_GetXmlFileContent(tmpBuf2,TmpBuf,UpMessage);
                if (FAILED(Ret))
                {
                    DAEMON_PRINT_LOG(ERROR,"GMI_GetXmlFileContent Error! ! ");
                    return GMI_FAIL;
                }
            }
            memset(TmpBuf, 0, sizeof(TmpBuf));
            Ret = GMI_XmlComPart(tmpBuf3,"<file name=\"","\"><size>",TmpBuf);
            if (SUCCEEDED(Ret))
            {
                Ret = GMI_GetXmlFileContent(tmpBuf3,TmpBuf,UpMessage);
                if (FAILED(Ret))
                {
                    DAEMON_PRINT_LOG(ERROR,"GMI_GetXmlFileContent Error! ! ");
                    return GMI_FAIL;
                }
            }
        }
        else
        {
            DAEMON_PRINT_LOG(ERROR,"GMI_XmlComPart Error! ! ");
            return Ret;
        }
        break;
    case 2:
        memset(dstBuf, 0, sizeof(dstBuf));
        Ret  = GMI_XmlComIntercept(SrcBuf,"<file ","</file>",dstBuf,tmpBuf1);
        if (SUCCEEDED(Ret))
        {
            Ret = GMI_XmlComPart(dstBuf,"<file name=\"","\"><size>",TmpBuf);
            if (SUCCEEDED(Ret))
            {
                Ret = GMI_GetXmlFileContent(dstBuf,TmpBuf,UpMessage);
                if (FAILED(Ret))
                {
                    DAEMON_PRINT_LOG(ERROR,"GMI_GetXmlFileContent Error! ! ");
                    return GMI_FAIL;
                }
            }
            Ret = GMI_XmlComPart(tmpBuf1,"<file name=\"","\"><size>",TmpBuf);
            if (SUCCEEDED(Ret))
            {
                Ret =GMI_GetXmlFileContent(tmpBuf1,TmpBuf,UpMessage);
                if (FAILED(Ret))
                {
                    DAEMON_PRINT_LOG(ERROR,"GMI_GetXmlFileContent Error! ! ");
                    return GMI_FAIL;
                }
            }
        }
        break;
    case 1:
        Ret  = GMI_XmlComIntercept(SrcBuf,"<file ","</file>", dstBuf, tmpBuf1);
        if (SUCCEEDED(Ret))
        {
            memset(TmpBuf,0,sizeof(0));
            Ret = GMI_XmlComPart(dstBuf,"<file name=\"","\"><size>",TmpBuf);
            if (SUCCEEDED(Ret))
            {
                Ret = GMI_GetXmlFileContent(dstBuf,TmpBuf,UpMessage);
                if (FAILED(Ret))
                {
                    DAEMON_PRINT_LOG(ERROR,"GMI_GetXmlFileContent Error! ! ");
                    return GMI_FAIL;
                }
            }
        }
        break;
    default:
        break;
    }

    return GMI_SUCCESS;
}

/*=======================================================
name				:	GMI_RequestUpdatFile
function			:  Request Update File function
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     CltFd  : Socket Id
                                                type : type .for example app. kernel. rootfs
                                                FileLen    : update file Length

return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/13/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_RequestUpdatFile(long_t CltFd, int32_t type, int32_t FileLen)
{

    UpdateBaseMessage  ResHeader;
    bzero(&ResHeader, sizeof(UpdateBaseMessage));

    char_t Ip[MIN_BUFFER_LENGTH]= {"0"};
    char_t EthName[MIN_BUFFER_LENGTH] = {"eth0"};
    GMI_RESULT Ret = GMI_FAIL;

    Ret = GMI_GetDeviceIpAddr(Ip, EthName);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_GetDeviceIpAddr is Error! ! ");
    }

    uint16_t TmpBufferLen = 0;
    char_t tmpBuf[FILE_BUFFER_LENGTH] = {"0"};

    if (GMI_UPDATE_FILE_TYPE_KENREL == type)
    {
        Ret = GMI_RequestDownloadFile(tmpBuf, &TmpBufferLen, Ip, g_UpMessage->s_KernelVersion,g_UpMessage->s_KernelName,0);
        if (FAILED(Ret))
        {
            DAEMON_PRINT_LOG(ERROR,"GMI_RequestDownloadFile is Error! ! ");
            return  GMI_FAIL;
        }
    }
    else if (GMI_UPDATE_FILE_TYPE_ROOTFS == type)
    {
        Ret = GMI_RequestDownloadFile(tmpBuf, &TmpBufferLen, Ip, g_UpMessage->s_RootfsVersion,g_UpMessage->s_RootfsName,0);
        if (FAILED(Ret))
        {
            DAEMON_PRINT_LOG(ERROR,"GMI_RequestDownloadFile is Error! ! ");
            return  GMI_FAIL;
        }
    }
    else if (GMI_UPDATE_FILE_TYPE_APP == type)
    {
        Ret = GMI_RequestDownloadFile(tmpBuf, &TmpBufferLen, Ip, g_UpMessage->s_AppVersion,g_UpMessage->s_AppName,0);
        if (FAILED(Ret))
        {
            DAEMON_PRINT_LOG(ERROR,"GMI_RequestDownloadFile is Error! ! ");
            return  GMI_FAIL;
        }
    }

    GMI_MakeMessageHeader(&ResHeader, 1, 128, TmpBufferLen);
    char_t Buffer[FILE_BUFFER_LENGTH] = {"0"};

    memcpy(Buffer,&ResHeader,sizeof(UpdateBaseMessage));
    memcpy(Buffer+sizeof(UpdateBaseMessage), tmpBuf, TmpBufferLen);
    uint16_t SendBufferLen = 0;
    SendBufferLen = sizeof(UpdateBaseMessage)+TmpBufferLen;
    if (send(CltFd, Buffer, SendBufferLen, 0)  !=  SendBufferLen)
    {
        DAEMON_PRINT_LOG(ERROR,"send is Error! ! ");
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}

/*=======================================================
name				:	GMI_Md516To32
function			:  Tansform Md5 value 16B to 32B
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     CltFd  : Socket Id
                                                type : type .for example app. kernel. rootfs
                                                FileLen    : update file Length

return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/13/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_Md516To32(MD5_CTX *Md5, char_t *OutBuf)
{
    char_t *TmpBuf=NULL;
    TmpBuf = OutBuf;

    uint32_t i=0;
    for (i=0; i<sizeof(Md5->digest); i++)
    {
        sprintf(TmpBuf, "%02x", Md5->digest[i]);
        TmpBuf += 2;
    }

    return GMI_SUCCESS;
}

/*=======================================================
name				:	GMI_UpdateFileSave
function			:  Recieve and save Update file
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     CltFd  : Socket Id
                                    type : type .for example app. kernel. rootfs
                                    FileLen    : update file Length

return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/13/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_UpdateFileSave(long_t CltFd, char_t *Name, int32_t fileSize, int32_t type)
{

    FILE *File = NULL;
    char_t FileName[MIN_BUFFER_LENGTH]= {"0"};
    char_t tmpFileName[MIN_BUFFER_LENGTH]= {"0"};

    char_t *Buffer = NULL;
    char_t Md5[MIN_BUFFER_LENGTH]= {"0"};
    uint32_t BufLen = 0;
    uint32_t PayloadLen = 0;
    int32_t  errorLen = 0;
    MD5_CTX m_md5;

    sprintf(FileName, "%s%s", UPDATE_SPACE_FILE, Name);

    File = fopen(FileName, "wb");
    if (NULL == File)
    {
        DAEMON_PRINT_LOG(ERROR,"fopen file  Error! ! ");
        GMI_DeBugPrint("[%s][%d] fopen file  Error ",__func__,__LINE__);
        goto exit;
    }

    PayloadLen = fileSize;
    BufLen = ((PayloadLen/4096)+1) *4096;
    Buffer = (char_t *)malloc(BufLen);
    if (NULL == Buffer)
    {
        DAEMON_PRINT_LOG(ERROR,"Service malloc buffer error! ! ");
        GMI_DeBugPrint("[%s][%d] Service malloc buffer error ",__func__,__LINE__);
        goto exit;
    }

    memset(Buffer, 0, BufLen);

    GMI_DeBugPrint("[%s][%d]FileName	= %s ,file size =%d ",__func__,__LINE__,FileName,PayloadLen);

    errorLen = GMI_TCPReceive(CltFd, Buffer, PayloadLen);
    if (errorLen != 0)
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_TCPReceive error error! ! ");
        GMI_DeBugPrint("[%s][%d]GMI_TCPReceive  Error [errorLen] = %d",__func__,__LINE__,errorLen);
        goto exit;
    }

    //Md5 check
    MD5Init(&m_md5, 0);
    MD5Update(&m_md5, (uint8_t *)Buffer, PayloadLen);
    MD5Final(&m_md5);
    GMI_Md516To32(&m_md5, Md5);

    switch (type)
    {
    case GMI_UPDATE_FILE_TYPE_KENREL:
        if (strncmp(g_UpMessage->s_KernelMd5, (const char*)Md5, 32) != 0) {
            DAEMON_PRINT_LOG(ERROR,"Kernel Md5 Check Error ! ! ");
            GMI_DeBugPrint("[%s][%d]Kernel Md5 Check Error ,Kernel Md5 = %s",__func__,__LINE__,g_UpMessage->s_KernelMd5);
            GMI_DeBugPrint("[%s][%d]Kernel Md5 Check Error ,Kernel Md5 = %s",__func__,__LINE__,Md5);
            goto MD5ERROR;
        }
        break;
    case GMI_UPDATE_FILE_TYPE_ROOTFS:
        if (strncmp(g_UpMessage->s_RootfsMd5, (const char*)Md5, 32) != 0) {
            DAEMON_PRINT_LOG(ERROR,"Rootfs Md5 Check Error ! ! ");
            GMI_DeBugPrint("[%s][%d]Rootfs Md5 Check Error ,Rootfs Md5 = %s",__func__,__LINE__,g_UpMessage->s_RootfsMd5);
            GMI_DeBugPrint("[%s][%d]Rootfs Md5 Check Error ,Rootfs Md5 = %s",__func__,__LINE__,Md5);
            goto MD5ERROR;
        }
        break;
    case GMI_UPDATE_FILE_TYPE_APP:
        if (strncmp(g_UpMessage->s_AppMd5, (const char*)Md5, 32) != 0) {
            DAEMON_PRINT_LOG(ERROR,"App Md5 Check Error ! ! ");
            GMI_DeBugPrint("[%s][%d]App Md5 Check Error ,App  Md5 = %s ",__func__,__LINE__,g_UpMessage->s_AppMd5);
            GMI_DeBugPrint("[%s][%d]App Md5 Check Error ,App  Md5 = %s",__func__,__LINE__,Md5);
            goto MD5ERROR;
        }
        break;
    default:
        break;
    }

    //Save file
    if (fwrite(Buffer, 1, BufLen, File) != BufLen)
    {
        DAEMON_PRINT_LOG(ERROR,"Write file  error Error ! ! ");
        GMI_DeBugPrint("[%s][%d]Write file  Error ",__func__,__LINE__);
        goto exit;
    }

    if ( NULL != Buffer)
        free(Buffer);
    if (NULL != File)
    {
        fclose(File);
        File = NULL;
    }

    return GMI_SUCCESS;

exit:

    if (NULL != Buffer)
        free(Buffer);
    if (NULL !=File)
    {
        fclose(File);
        File = NULL;
    }
    return GMI_FAIL;

MD5ERROR:
    FILE *File1 = NULL;

    sprintf(tmpFileName, "%s","/tmp/tmp");

    File1 = fopen(tmpFileName, "wb");
    if (NULL == File1)
    {
        DAEMON_PRINT_LOG(ERROR,"fopen file  Error! ! ");
    }

    if (fwrite(Buffer, 1, BufLen, File1) != BufLen)
    {
        DAEMON_PRINT_LOG(ERROR,"Write file  error Error ! ! ");
        GMI_DeBugPrint("[%s][%d]Write file  Error ",__func__,__LINE__);
    }

    if (NULL != Buffer)
    {
        free(Buffer);
    }

    if (NULL !=File)
    {
        fclose(File);
        File = NULL;
    }

    if (NULL !=File1)
    {
        fclose(File1);
        File1 = NULL;
    }

    return GMI_FAIL;
}

/*=======================================================
name				:	GMI_ReportUpdateNotify
function			:  Report Update status notify
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     CltFd  : Socket Id
                                                Status : current stats
                                                schedule    : update file Length

return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/13/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_ReportUpdateNotify(long_t CltFd, int32_t Status, int32_t  schedule)
{
    char_t TmpBuffer[FILE_BUFFER_LENGTH] = {0};
    char_t Buffer[FILE_BUFFER_LENGTH] = {0};

    UpdateBaseMessage  ResHeader;
    bzero(Buffer, FILE_BUFFER_LENGTH);
    bzero(&ResHeader, sizeof(UpdateBaseMessage));

    GMI_RESULT Ret = GMI_FAIL;
    char_t Ip[MIN_BUFFER_LENGTH]= {"0"};
    char_t EthName[MIN_BUFFER_LENGTH] = {"eth0"};

    Ret = GMI_GetDeviceIpAddr(Ip, EthName);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_GetDeviceIpAddr is Error ! ! ");
    }

    uint16_t TmpLen = 0;
    if (GMI_UPDATE_STATUS_REBOOTING == Status)
    {
        Ret = GMI_ReportRootStatus(TmpBuffer, &TmpLen, Ip);
        if(FAILED(Ret))
        {
            return Ret;
        }
    }
    else
    {
        Ret = GMI_ReportUpdateStatus(TmpBuffer, &TmpLen, Ip, Status, schedule);
        if(FAILED(Ret))
        {
            return Ret;
        }
    }

    GMI_MakeMessageHeader(&ResHeader, 0, 0, TmpLen);

    memcpy(Buffer,&ResHeader,sizeof(UpdateBaseMessage));
    memcpy(Buffer+sizeof(UpdateBaseMessage), TmpBuffer, TmpLen);

    uint16_t SendLen = 0;
    SendLen = sizeof(UpdateBaseMessage)+TmpLen;
    if (send(CltFd, Buffer, SendLen, 0) != SendLen)
    {
        DAEMON_PRINT_LOG(ERROR,"send is Error ! ! ");
        Ret = GMI_FAIL;
    }

    return Ret;
}

/*=======================================================
name				:	GMI_AnalyzeMessage
function			:  Report Update status notify
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     CltFd  : Socket Id
                                                Status : current stats
                                                schedule    : update file Length

return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/13/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
long_t GMI_AnalyzeMessage(long_t CltFd, char_t *szBuffer, long_t CltAddr)
{
    if((CltFd < 0) || (NULL==szBuffer))
    {
        return  GMI_INVALID_PARAMETER;
    }

    char_t CmdBuffer[MAX_BUFFER_LENGTH];
    char_t KernelVersion[MIN_BUFFER_LENGTH], RootfsVersion[MIN_BUFFER_LENGTH], AppVersion[MIN_BUFFER_LENGTH];
    char_t KernelTmp[MIN_BUFFER_LENGTH],RootfsTmp[MIN_BUFFER_LENGTH],AppTmp[MIN_BUFFER_LENGTH];

    memset(KernelVersion, 0 ,sizeof(KernelVersion));
    memset(RootfsVersion, 0 ,sizeof(RootfsVersion));
    memset(AppVersion, 0 ,sizeof(AppVersion));
    memset(KernelTmp, 0 ,sizeof(KernelTmp));
    memset(RootfsTmp, 0 ,sizeof(RootfsTmp));
    memset(AppTmp, 0 ,sizeof(AppTmp));

    GMI_RESULT Ret = GMI_FAIL;
    Ret = GMI_GetVersion(GET_KERNEL_VERSION, KernelVersion,sizeof(KernelVersion));
    if(FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_GetVersion Kernel Version fail!!!\n");
        goto EXIT;
    }

    Ret = GMI_GetVersion(GET_ROOTFS_VERSION, RootfsVersion,sizeof(RootfsVersion));
    if(FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_GetVersion Rootfs Version fail!!!\n");
        goto EXIT;
    }

    GMI_DeBugPrint("[%s][%d]Update kernel Start\n",__func__,__LINE__);

    Ret = GMI_UpdateBufferAnalyze(szBuffer, g_UpMessage);
    if (SUCCEEDED(Ret))
    {
        //enable hardware watchdog
        if (l_UpdateFlags)
        {
            l_UpdateFlags = false;
            Ret =GMI_SysHwWatchDogDisable();
            if (FAILED(Ret))
            {
                DAEMON_PRINT_LOG(ERROR,"GMI_AnalyzeMessage->GMI_SysHwWatchDogDisable fail!!!\n");
                goto EXIT;
            }
            GMI_DeBugPrint("[%s][%d]GMI_SysHwWatchDogDisable Start Running l_UpdateFlags = %d ! ",__func__,__LINE__,l_UpdateFlags);
        }

        //Close app
        GMI_ApplicationUpdateQuit();
        memcpy(KernelTmp,g_UpMessage->s_KernelVersion+5,4);
        KernelTmp[4] =  '\0';

        int32_t UpdateFlags = -1;
        UpdateFlags = atoi(KernelTmp) - atoi(KernelVersion);
        GMI_DeBugPrint("[%s][%d]KernelTmp %d - KernelVersion %d  = UpdateFlags is %d\n",__func__,__LINE__,atoi(KernelTmp), atoi(KernelVersion), UpdateFlags);

        if(UpdateFlags > 0)
        {
            //send update message
            Ret = GMI_RequestUpdatFile(CltFd, GMI_UPDATE_FILE_TYPE_KENREL, 0);
            if (FAILED(Ret))
            {
                DAEMON_PRINT_LOG(ERROR,"GMI_RequestUpdatFile is Error ! ! ");
                goto EXIT;
            }

            Ret = GMI_UpdateFileSave(CltFd, g_UpMessage->s_KernelName, g_UpMessage->s_KernelSize, GMI_UPDATE_FILE_TYPE_KENREL);
            if (SUCCEEDED(Ret))
            {
                GMI_DeBugPrint("[%s][%d]Update kernel Start",__func__,__LINE__);
                //Update file
                memset(CmdBuffer, 0, 255);
                snprintf(CmdBuffer, 255, UPDATE_KERNEL_BIN);
                LOCK(&l_LockUpdateSys);
                GMI_DeBugPrint("[%s][%d]Update  Start",__func__,__LINE__);
                if (system(CmdBuffer) < 0)
                {
                    UNLOCK(&l_LockUpdateSys);
                    GMI_DeBugPrint("[%s][%d]DoUpdateKernel is Error",__func__,__LINE__);
                    Ret = GMI_ReportUpdateNotify(CltFd, GMI_UPDATE_STATUS_UPDATE_ERROR, 0);
                    if (FAILED(Ret))
                        GMI_DeBugPrint("[%s][%d]GMI_ReportUpdateNotify is Error",__func__,__LINE__);
                    goto EXIT;
                }
                GMI_DeBugPrint("[%s][%d]Update  End",__func__,__LINE__);
                UNLOCK(&l_LockUpdateSys);
                l_Count--;
                g_UpMessage->s_KernelFlags = GMI_UPDATE_END_FLAGS;
            }
            else if (FAILED(Ret))
            {
                l_Count--;
                GMI_ReportUpdateNotify(CltFd, GMI_UPDATE_STATUS_UPDATE_ERROR, 0);
                GMI_DeBugPrint("[%s][%d]Update kernel Error",__func__,__LINE__);
                goto EXIT;
            }

        }

        memcpy(RootfsTmp, g_UpMessage->s_RootfsVersion+5, 4);
        RootfsTmp[4] = '\0';
        UpdateFlags = atoi(RootfsTmp) - atoi(RootfsVersion);
        GMI_DeBugPrint("[%s][%d]RootfsTmp %d - RootfsVersion %d  = UpdateFlags is %d\n",__func__,__LINE__,atoi(RootfsTmp), atoi(RootfsVersion), UpdateFlags);

        if (UpdateFlags > 0)
        {
            Ret = GMI_RequestUpdatFile(CltFd, GMI_UPDATE_FILE_TYPE_ROOTFS, 0);
            if (FAILED(Ret))
            {
                DAEMON_PRINT_LOG(ERROR,"GMI_RequestUpdatFile is Error ! ! ");
                goto EXIT;
            }

            Ret = GMI_UpdateFileSave(CltFd, g_UpMessage->s_RootfsName, g_UpMessage->s_RootfsSize, GMI_UPDATE_FILE_TYPE_ROOTFS);
            if (SUCCEEDED(Ret))
            {
                GMI_DeBugPrint("[%s][%d]Update Rootfs Start",__func__,__LINE__);
                //Update file
                memset(CmdBuffer, 0, 255);
                snprintf(CmdBuffer, 255, UPDATE_ROOTFS_BIN);
                LOCK(&l_LockUpdateSys);
                GMI_DeBugPrint("[%s][%d]Update  Start",__func__,__LINE__);
                if (system(CmdBuffer) < 0)
                {
                    UNLOCK(&l_LockUpdateSys);
                    GMI_DeBugPrint("[%s][%d]DoUpdateRootfs is Error",__func__,__LINE__);
                    Ret = GMI_ReportUpdateNotify(CltFd, GMI_UPDATE_STATUS_UPDATE_ERROR, 0);
                    if (FAILED(Ret))
                        GMI_DeBugPrint("[%s][%d]GMI_ReportUpdateNotify is Error",__func__,__LINE__);
                    goto EXIT;
                }
                GMI_DeBugPrint("[%s][%d]Update	End",__func__,__LINE__);
                UNLOCK(&l_LockUpdateSys);
                l_Count--;
                g_UpMessage->s_RootfsFlags = GMI_UPDATE_END_FLAGS;
            }
            else if (FAILED(Ret))
            {
                l_Count--;
                GMI_ReportUpdateNotify(CltFd, GMI_UPDATE_STATUS_UPDATE_ERROR, 0);
                GMI_DeBugPrint("[%s][%d]Update Rootfs Error",__func__,__LINE__);
                goto EXIT;
            }

        }

        memcpy(AppTmp, g_UpMessage->s_AppVersion+5, 4);
        AppTmp[4] = '\0';

        if (GMI_UPDATE_START_FLAGS == g_UpMessage->s_AppFlags)
        {
            //send update message
            Ret = GMI_RequestUpdatFile(CltFd, GMI_UPDATE_FILE_TYPE_APP, 0);
            if (FAILED(Ret))
            {
                DAEMON_PRINT_LOG(ERROR,"GMI_RequestUpdatFile is Error ! ! ");
                goto EXIT;
            }

            DAEMON_PRINT_LOG(INFO,"GMI_RequestUpdatFile OK ! ");

            Ret = GMI_UpdateFileSave(CltFd, g_UpMessage->s_AppName, g_UpMessage->s_AppSize, GMI_UPDATE_FILE_TYPE_APP);
            if (SUCCEEDED(Ret))
            {
                //Update file
                GMI_DeBugPrint("[%s][%d]Update App Start",__func__,__LINE__);
                memset(CmdBuffer, 0, 255);
                snprintf(CmdBuffer, 255, UPDATE_APP_BIN);
                LOCK(&l_LockUpdateSys);
                GMI_DeBugPrint("[%s][%d]Update  Start",__func__,__LINE__);
                if (system(CmdBuffer) < 0)
                {
                    UNLOCK(&l_LockUpdateSys);
                    //report error
                    GMI_DeBugPrint("[%s][%d]DoUpdateApp is Error",__func__,__LINE__);
                    Ret = GMI_ReportUpdateNotify(CltFd, GMI_UPDATE_STATUS_UPDATE_ERROR, 0);
                    if (FAILED(Ret))
                        GMI_DeBugPrint("[%s][%d]GMI_ReportUpdateNotify is Error",__func__,__LINE__);
                    goto EXIT;
                }
                GMI_DeBugPrint("[%s][%d]Update  End",__func__,__LINE__);
                UNLOCK(&l_LockUpdateSys);
                l_Count--;
                g_UpMessage->s_AppFlags = GMI_UPDATE_END_FLAGS;
            }
            else if (FAILED(Ret))
            {
                l_Count--;
                GMI_ReportUpdateNotify(CltFd, GMI_UPDATE_STATUS_UPDATE_ERROR, 0);
                goto EXIT;
            }
        }

        Ret = GMI_ReportUpdateNotify(CltFd, GMI_UPDATE_STATUS_UPDATED, 100);
        if (FAILED(Ret))
        {
            DAEMON_PRINT_LOG(ERROR,"GMI_ReportUpdateNotify is Error ! ! ");
        }
        
    }
    else if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_ReportUpdateNotify is Error ! ! ");
        goto EXIT;
    }

    return GMI_SUCCESS;

EXIT:

    return GMI_FAIL;
}


/*=======================================================
name				:	GMI_ReportSystemPlatform
function			:  Report Update list Result
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/3/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_ReportSystemPlatform(char_t *Buf, uint16_t *BufLen, char_t *Ip, char_t *Platform)
{

    if (NULL == Ip)
    {
        return GMI_INVALID_PARAMETER;
    }

    uint16_t Length = 0;

    Length += sprintf(Buf+Length, "<update from=\"%s\"  type=\"get_info\" >", Ip);
    Length += sprintf(Buf+Length, "<platform>%s</platform>",Platform);
    Length += sprintf(Buf+Length, "</update>");

    *BufLen = Length;

    return GMI_SUCCESS;
}

/*=======================================================
name				:	GMI_ReportPlatformInfo
function			:  Report Get update list status
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     CltFd  : Socket Id
                                                result : current stats
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	7/8/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_ReportPlatformInfo(long_t CltFd)
{
    if(CltFd < 0)
    {
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Ret = GMI_FAIL;
    char_t Ip[MIN_BUFFER_LENGTH]= {"0"};
    char_t EthName[MIN_BUFFER_LENGTH] = {"eth0"};
    Ret = GMI_GetDeviceIpAddr(Ip, EthName);
    if (SUCCEEDED(Ret))
    {
        DAEMON_PRINT_LOG(INFO,"GMI_GetDeviceIpAddr is OK ! ! ");
    }

    char_t Platfrom[MIN_BUFFER_LENGTH] = {0};
    Ret = GMI_GetPlatformConfig(GMI_HARDWARE_CONFIG_FILE, GMI_HARDWARE_PATH,Platfrom);
    if (FAILED(Ret))
    {
        GMI_DeBugPrint("[%s][%d] GMI_GetPlatformConfig ERROR ! ",__func__,__LINE__);
    }

    if ((strcmp("A5S_55",Platfrom)==0) || (strcmp("A5S_66",Platfrom)==0) ||(strcmp("A5S_88",Platfrom)==0))
    {
        strcpy(Platfrom,SYSTEM_PLATFORM_AMBA);
    }
    else
    {
        GMI_DeBugPrint("[%s][%d]  GMI_GetPlatformConfig Platfrom is Error  %s! ",__func__,__LINE__,Platfrom);
        strcpy(Platfrom,SYSTEM_PLATFORM_AMBA);
    }

    char_t TmpBuffer[FILE_BUFFER_LENGTH] = {0};
    uint16_t TmpLen = 0;
    uint16_t SendLen = 0;
    GMI_ReportSystemPlatform(TmpBuffer, &TmpLen, Ip, Platfrom);

    UpdateBaseMessage  ResHeader;
    bzero(&ResHeader, sizeof(UpdateBaseMessage));
    GMI_MakeMessageHeader(&ResHeader, 0, 0, TmpLen);

    char_t Buffer[FILE_BUFFER_LENGTH] = {0};
    bzero(Buffer, FILE_BUFFER_LENGTH);
    memcpy(Buffer,&ResHeader,sizeof(UpdateBaseMessage));
    memcpy(Buffer+sizeof(UpdateBaseMessage), TmpBuffer, TmpLen);
    SendLen = sizeof(UpdateBaseMessage)+TmpLen;
    DAEMON_PRINT_LOG(INFO,"TmpBuffer is %s ! ! ",Buffer);
    if (send(CltFd, Buffer, SendLen, 0) != SendLen)
    {
        DAEMON_PRINT_LOG(ERROR,"TCP send  is Error ! ! ");
        Ret = GMI_FAIL;
    }

    return Ret;
}

/*=========================================================================
name				:	GMI_GetSystemPlatform
function			:  System Update ,PC client Get System hardware
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     CltFd  : Socket Id
                                                Status : current stats
                                                schedule    : update file Length

return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	7/8/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
long_t GMI_GetSystemPlatform(long_t CltFd, char_t *szBuffer, long_t CltAddr)
{
    if(CltFd < 0)
    {
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Ret = GMI_FAIL;
    GMI_DeBugPrint("[%s][%d]GMI_GetSystemPlatform Start",__func__,__LINE__);

    Ret = GMI_ReportPlatformInfo(CltFd);
    if (FAILED(Ret))
    {
        GMI_DeBugPrint("[%s][%d]GMI_GetSystemPlatform Error",__func__,__LINE__);
        return Ret;
    }

    return Ret;
}

/*=======================================================
name				:	GMI_Reboot
function			:  System Update OK . Cmd is reboot system
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     CltFd  : Socket Id
                                                Status : current stats
                                                schedule    : update file Length

return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	7/8/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
long_t GMI_Reboot(long_t CltFd, char_t *szBuffer, long_t CltAddr)
{
    if(CltFd < 0)
    {
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Ret = GMI_FAIL;

    GMI_DeBugPrint("[%s][%d]Update complete System will be Reboot",__func__,__LINE__);
    if (!l_UpdateFlags)
    {
        Ret = GMI_SysHwWatchDogEnable();
        if (FAILED(Ret))
        {
            DAEMON_PRINT_LOG(ERROR,"GMI_Reboot->TestHwWatchDogDisable fail!!!\n");
        }
    }

    Ret = GMI_ReportUpdateNotify(CltFd, GMI_UPDATE_STATUS_REBOOTING,100);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_ReportUpdateNotify is Error ! ! ");
    }

    GMI_ApplicationQuit(LOG_SERVER_ID);

    Ret = GMI_BrdHwReset();
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_Reboot! !!!");
    }

    return Ret;
}




/*=======================================================
name				:	GMI_ReportSystemPlatform
function			:  Report Update list Result
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/3/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_DeviceInfoPacket(char_t *Buf, uint16_t *BufLen)
{

    uint16_t Length = 0;
    GMI_RESULT Result = GMI_FAIL;
    char_t Ip[MIN_BUFFER_LENGTH]= {"0"};
    char_t EthName[MIN_BUFFER_LENGTH] = {"eth0"};
    char_t Mac[32];

    Result = GMI_GetDeviceIpAddr(Ip, EthName);
    if (SUCCEEDED(Result))
    {
        DAEMON_PRINT_LOG(INFO,"GMI_GetDeviceIpAddr is OK ! ! ");
    }

    memset(Mac, 0, sizeof(Mac));
    Result = GMI_GetMacInfo(EthName, Mac);
    if (FAILED(Result))
    {
        DAEMON_PRINT_LOG(INFO,"GMI_GetMacInfo is Fail ! ! ");
    }

    Result = SysInfoReadInitialize();
    if (FAILED(Result))
    {
        DAEMON_PRINT_LOG(INFO,"SysInfoReadInitialize is Fail ! ! ");
    }

    FD_HANDLE Handle;
   
    Result = SysInfoOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if (FAILED(Result))
    {
        DAEMON_PRINT_LOG(INFO,"SysInfoOpen is Fail ! ! ");
    }
    
    int32_t RTSP_Port = 554;
    Result = SysInfoRead(Handle, GMI_EXTERN_NETWORK_PORT_PATH, GMI_RTSP_SERVER_TCP_PORT_KEY, 554, &RTSP_Port);
    if (FAILED(Result))
    {
	RTSP_Port = 554;
    }

    int32_t HTTP_Port = 80;
    Result = SysInfoRead(Handle, GMI_EXTERN_NETWORK_PORT_PATH, GMI_HTTP_SERVER_PORT_KEY, 80, &HTTP_Port);
    if (FAILED(Result))
    {
	HTTP_Port = 80;
    }

    int32_t SDK_Port = 30000;
    Result = SysInfoRead(Handle, GMI_EXTERN_NETWORK_PORT_PATH, GMI_SDK_SERVER_PORT_KEY, 30000, &SDK_Port);
    if (FAILED(Result))
    {
	SDK_Port = 30000;
    }

    int32_t Upgrade_Port = 8000;
    Result = SysInfoRead(Handle, GMI_EXTERN_NETWORK_PORT_PATH, GMI_DAEMON_UPDATE_SERVER_PORT_KEY, 8000, &Upgrade_Port);
    if (FAILED(Result))
    {
	Upgrade_Port = 8000;
    }

    char_t Name[64];
    memset(Name,0,sizeof(Name));
    Result = SysInfoRead(Handle, DEVICE_INFO_PATH, DEVICE_NAME_KEY, DEVICE_NAME, Name);
    if (FAILED(Result))
    {
	strcpy(Name, DEVICE_NAME);
    }

    char_t Sn[64];
    memset(Sn,0,sizeof(Sn));
    Result = SysInfoRead(Handle, DEVICE_INFO_PATH, DEVICE_SN_KEY, DEVICE_SN, Sn);
    if (FAILED(Result))
    {
	strcpy(Sn, DEVICE_SN);
    }

    char_t HwVersion[64];
    memset(HwVersion,0,sizeof(HwVersion));
    Result = SysInfoRead(Handle, DEVICE_INFO_PATH, DEVICE_HWVER_KEY, DEVICE_HWVER, HwVersion);
    if (FAILED(Result))
    {
	strcpy(HwVersion, DEVICE_HWVER);
    }

    char_t FwVersion[64];
    memset(FwVersion,0,sizeof(FwVersion));
    Result = SysInfoRead(Handle, DEVICE_INFO_PATH, DEVICE_FWVER_KEY, DEVICE_FWVER, FwVersion);
    if (FAILED(Result))
    {
	strcpy(FwVersion, DEVICE_FWVER);
    }

    SysInfoClose(Handle);

    SysInfoReadDeinitialize();

    Length += sprintf(Buf+Length, "<Response operation=\"Search\" result=\"0\">");
    Length += sprintf(Buf+Length, "<NetworkInfo>");
    Length += sprintf(Buf+Length, "<InterfaceList num=\"1\">");
    Length += sprintf(Buf+Length, "<Interface name=\"eth0\" enable=\"1\" dhcp=\"0\">");
    Length += sprintf(Buf+Length, "<Address>%s</Address>", Ip);
    Length += sprintf(Buf+Length, "<Netmask>255.255.0.0</Netmask>");
    Length += sprintf(Buf+Length, "<Gateway>10.0.0.1</Gateway>");
    Length += sprintf(Buf+Length, "<MAC>%s</MAC>", Mac);
    Length += sprintf(Buf+Length, "</Interface>");
    Length += sprintf(Buf+Length, "</InterfaceList>");
    Length += sprintf(Buf+Length, "<DNSList num=\"2\">");
    Length += sprintf(Buf+Length, "<DNS>192.168.1.1</DNS>");
    Length += sprintf(Buf+Length, "<DNS>%s</DNS>", Ip);
    Length += sprintf(Buf+Length, "</DNSList>");
    Length += sprintf(Buf+Length, "</NetworkInfo>");
    Length += sprintf(Buf+Length, "<DeviceStatus>OnLine</DeviceStatus>");
    Length += sprintf(Buf+Length, "<DeviceInfo>");
    Length += sprintf(Buf+Length, "<Name>%s</Name>", Name);
    Length += sprintf(Buf+Length, "<SN>%s</SN>", Sn);
    Length += sprintf(Buf+Length, "<HwVersion>%s</HwVersion>", HwVersion);
    Length += sprintf(Buf+Length, "<FwVersion>%s</FwVersion>", FwVersion);
    Length += sprintf(Buf+Length, "<LastBootTime>%s</LastBootTime>", "20140211163524");
    Length += sprintf(Buf+Length, "</DeviceInfo>");
    Length += sprintf(Buf+Length, "<ServiceList>");
    Length += sprintf(Buf+Length, "<Service name=\"HTTP\" port=\"%d\"/>", HTTP_Port);
    Length += sprintf(Buf+Length, "<Service name=\"RTSP\" port=\"%d\"/>", RTSP_Port);
    Length += sprintf(Buf+Length, "<Service name=\"SDK\" port=\"%d\"/>", SDK_Port);
    Length += sprintf(Buf+Length, "<Service name=\"Upgrade\" port=\"%d\"/>", Upgrade_Port);
    Length += sprintf(Buf+Length, "</ServiceList>");
    Length += sprintf(Buf+Length, "</Response>");

    *BufLen = Length;

    return GMI_SUCCESS;
}

GMI_RESULT GMI_ReportDeviceInfo(long_t CltFd)
{
    if(CltFd < 0)
    {
        return GMI_INVALID_PARAMETER;
    }

    char_t TmpBuffer[2048] = {0};
    uint16_t TmpLen = 0;
    uint16_t SendLen = 0;
    GMI_DeviceInfoPacket(TmpBuffer, &TmpLen);

    UpdateBaseMessage  ResHeader;
    bzero(&ResHeader, sizeof(UpdateBaseMessage));
    GMI_MakeMessageHeader(&ResHeader, 0, 0, TmpLen);

    char_t Buffer[2048] = {0};
    bzero(Buffer, 2048);
    memcpy(Buffer,&ResHeader,sizeof(UpdateBaseMessage));
    memcpy(Buffer+sizeof(UpdateBaseMessage), TmpBuffer, TmpLen);
    SendLen = sizeof(UpdateBaseMessage)+TmpLen;
    DAEMON_PRINT_LOG(INFO,"TmpBuffer is %s ! ! ",Buffer);
    if (send(CltFd, Buffer, SendLen, 0) != SendLen)
    {
        DAEMON_PRINT_LOG(ERROR,"TCP send  is Error ! ! ");
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}

/*=========================================================================
name				:	GMI_GetSystemInform
function			:  System Update ,PC client Get System hardware
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     CltFd  : Socket Id
                                                Status : current stats
                                                schedule    : update file Length

return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	7/8/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
long_t GMI_GetDeviceInfo(long_t CltFd, char_t *szBuffer, long_t CltAddr)
{
    if(CltFd < 0)
    {
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Result = GMI_SUCCESS;
    GMI_DeBugPrint("[%s][%d]GMI_GetSystemPlatform Start",__func__,__LINE__);

    Result = GMI_ReportDeviceInfo(CltFd);
    if (FAILED(Result))
    {
        GMI_DeBugPrint("[%s][%d]GMI_GetSystemPlatform Error",__func__,__LINE__);
        return Result;
    }

    return Result;
}

/*=======================================================
name				:	GMI_NoSupport
function			:  Report Cmd not Support
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     CltFd  : Socket Id
                                                Status : current stats
                                                schedule    : update file Length

return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	7/8/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
long_t GMI_NoSupport(long_t CltFd, char_t *szBuffer, long_t CltAddr)
{
    int32_t Ret=0;

    DAEMON_PRINT_LOG(ERROR,"GMI_NoSupport ! !!!");
    GMI_DeBugPrint("[%s][%d]GMI_NoSupport Cmd is Not Support",__func__,__LINE__);

    return Ret;
}

/*=======================================================
name				:	GMI_ReportGetListStatus
function			:  Report Get update list status
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     CltFd  : Socket Id
                                                Result : current stats
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/13/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_ReportGetListStatus(long_t CltFd, int32_t Result)
{
    if(CltFd < 0)
    {
        return GMI_INVALID_PARAMETER;
    }

    char_t Ip[MIN_BUFFER_LENGTH]= {"0"};
    char_t EthName[MIN_BUFFER_LENGTH] = {"eth0"};
    GMI_RESULT Ret = GMI_FAIL;
    Ret = GMI_GetDeviceIpAddr(Ip, EthName);
    if (SUCCEEDED(Ret))
    {
        DAEMON_PRINT_LOG(INFO,"GMI_GetDeviceIpAddr is OK ! ! ");
    }

    char_t TmpBuffer[FILE_BUFFER_LENGTH] = {0};
    uint16_t TmpLen = 0;
    Ret = GMI_ReportUpdateListResult(TmpBuffer, &TmpLen, Ip, Result);
    if (FAILED(Ret))
    {
        return Ret;
    }

    UpdateBaseMessage  ResHeader;
    bzero(&ResHeader, sizeof(UpdateBaseMessage));
    GMI_MakeMessageHeader(&ResHeader, 0, 0, TmpLen);

    char_t Buffer[FILE_BUFFER_LENGTH] = {0};
    bzero(Buffer, FILE_BUFFER_LENGTH);
    memcpy(Buffer,&ResHeader,sizeof(UpdateBaseMessage));
    memcpy(Buffer+sizeof(UpdateBaseMessage), TmpBuffer, TmpLen);

    uint16_t SendLen = 0;
    SendLen = sizeof(UpdateBaseMessage)+TmpLen;
    DAEMON_PRINT_LOG(INFO,"TmpBuffer is %s ! ! ",Buffer);
    if (send(CltFd, Buffer, SendLen, 0) != SendLen)
    {
        DAEMON_PRINT_LOG(ERROR,"TCP send  is Error ! ! ");
        Ret = GMI_FAIL;
    }

    return Ret;
}

/*=======================================================
name				:	GMI_ClientService
function			:  Create System Update TCP Server
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     Port
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	5/10/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
static long_t GMI_ClientService(long_t CltFd, long_t CltAddr)
{
    if(CltFd < 0)
    {
        return GMI_INVALID_PARAMETER;
    }

    fd_set FdSet;
    int32_t MsgPayloadLen = 0;
    int32_t ReceiveBytes = 0;
    char_t Buffer[MAX_MSG_LEN]  = {0};

    UpdateBaseMessage  Header;
    DAEMON_PRINT_LOG(INFO,"GMI_ClientService  Start  ! ! ");

    struct timeval Timeout;

    pthread_detach(pthread_self());
    CREATE_LOCK(&l_LockUpdateSys);
    Timeout.tv_sec  = 5;
    Timeout.tv_usec = 0;
    FD_ZERO(&FdSet);
    FD_SET(CltFd, &FdSet);

    if (select(CltFd+1, &FdSet, NULL, NULL, &Timeout) > 0)
    {
        if (FD_ISSET(CltFd, &FdSet))
        {
            bzero(Buffer, MAX_MSG_LEN);
            bzero(&Header, sizeof(UpdateBaseMessage));
            ReceiveBytes = recv(CltFd, (char_t*)&Header, sizeof(UpdateBaseMessage), 0);
            if ( sizeof(UpdateBaseMessage) != ReceiveBytes )
            {
                DAEMON_PRINT_LOG(ERROR,"TCP recieve  is Error ! ! ");
                goto close_socket;
            }
            MsgPayloadLen = ntohs(Header.s_MessageLength);

            GMI_RESULT Ret = GMI_FAIL;
            //recv  xml message  and return xml message
            if ( MsgPayloadLen > 0)
            {
                if (recv(CltFd, Buffer, MsgPayloadLen, 0) != MsgPayloadLen)
                {
                    if (ntohs(Header.s_MessageId) == GMI_UPDATE_CMD_LIST)
                    {
                        //return read message error, return
                        Ret = GMI_ReportGetListStatus(CltFd, GMI_UPDATE_CMD_ERROR);
                        if (FAILED(Ret))
                        {
                            DAEMON_PRINT_LOG(ERROR,"GMI_ReportGetListStatus  is Error ! ! ");;
                        }
                    }
                }
                else
                {
                    if (ntohs(Header.s_MessageId) == GMI_UPDATE_CMD_LIST)
                    {
                        //return receve message OK, return
                        Ret = GMI_ReportGetListStatus(CltFd, GMI_UPDATE_CMD_OK);
                        if (FAILED(Ret))
                        {
                            DAEMON_PRINT_LOG(ERROR,"GMI_ReportGetListStatus  is Error ! ! ");;
                        }
                    }
                }
            }

            GMI_DeBugPrint("[%s][%d]Header.s_MessageId is %d",__func__,__LINE__,ntohs(Header.s_MessageId));

            switch (ntohs(Header.s_MessageId))
            {
            case GMI_UPDATE_CMD_LIST:
                GMI_AnalyzeMessage(CltFd, Buffer, CltAddr);
                break;
            case GMI_UPDATE_CMD_REBOOT:
                GMI_Reboot(CltFd, Buffer, CltAddr);
                break;
            case GMI_UPDATE_CMD_GET_INFO:
                GMI_GetSystemPlatform(CltFd, Buffer, CltAddr);
                break;
            case GMI_GET_DEVICE_INFO:
                GMI_GetDeviceInfo(CltFd, Buffer, CltAddr);
                break;
            default:
                GMI_NoSupport(CltFd, Buffer, CltAddr);
                break;
            }
        } /*  end of if(FD_ISSET...  */
    } /*  end of if(select...  */

close_socket:
    shutdown(CltFd, 2);
    close(CltFd);
    CltFd = -1;
    DAEMON_PRINT_LOG(ERROR,"TCP Soket is quit ! ! ");

    RELEASE_LOCK(&l_LockUpdateSys);
    free((struct sockaddr_in*)CltAddr);
    pthread_testcancel();
    pthread_exit(NULL);

}

/*=======================================================
name				:	GMI_CreateTCPServer
function			:  Create System Update TCP Server
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     Port
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/29/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_CreateTCPServer(const char_t *Ip, const ulong_t Port)
{
    struct sockaddr_in SvrAdd;
    int32_t AddrSize  = sizeof(struct sockaddr);
    int32_t CltFd      = -1;
    int32_t AddrReuse = 1;
    int32_t l_Tmp       = 0;
    struct linger tcpLinger;

    DAEMON_PRINT_LOG(INFO,"GMI_CreateTCPServer  Start  ! ! ");

    bzero((char_t*)&SvrAdd, sizeof(struct sockaddr_in));
    SvrAdd.sin_family      = AF_INET;
    SvrAdd.sin_port        = htons(Port&0xFFFF);
    SvrAdd.sin_addr.s_addr = htonl(INADDR_ANY);

    l_SvrFd = socket(AF_INET, SOCK_STREAM, 0);
    if (l_SvrFd == -1)
    {
        return GMI_FAIL;
    }

    l_Tmp = setsockopt(l_SvrFd, SOL_SOCKET, SO_REUSEADDR, (char_t*)&AddrReuse, sizeof(int));
    if (l_Tmp == -1)
    {
        goto exit;
    }

    l_Tmp = bind(l_SvrFd, (struct sockaddr*)&SvrAdd, sizeof(struct sockaddr_in));
    if (l_Tmp == -1)
    {
        goto exit;
    }


    l_Tmp = listen(l_SvrFd, 20);
    if (l_Tmp == -1)
    {
        goto exit;
    }

    DAEMON_PRINT_LOG(INFO,"Soket  Start  ! ! ");

    while (l_iStopTCPServer != 1)
    {
        struct sockaddr_in *CltAddr = (sockaddr_in*)malloc(sizeof(struct sockaddr_in));
        bzero(CltAddr, sizeof(struct sockaddr_in));
        AddrSize  = sizeof(struct sockaddr_in);
        CltFd = accept(l_SvrFd, (struct sockaddr*)CltAddr, (socklen_t *)&AddrSize);
        if (CltFd == -1)
        {
            if (NULL != CltAddr)
                free(CltAddr);
            continue;
        }
        l_Tmp = GMI_CreateTask(110, 81920, (fTaskEntryPoint)GMI_ClientService, CltFd, (long)CltAddr, 0, 0, 0, 0, 0);
        if (l_Tmp == -1)
        {
            tcpLinger.l_onoff = 1;
            tcpLinger.l_linger = 0;
            setsockopt(CltFd, SOL_SOCKET, SO_LINGER, (char_t*)&tcpLinger, sizeof(struct linger));
            shutdown(CltFd, 2);
            close(CltFd);
            CltFd = -1;
            if (NULL != CltAddr)
                free(CltAddr);
        }
    }

exit:
    shutdown(l_SvrFd, 2);
    close(l_SvrFd);
    l_SvrFd = -1;
    return GMI_SUCCESS;
}

/*=======================================================
name				:	GMI_UpdateServer
function			:  Create System update Server
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/29/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
static void* GMI_UpdateServer(void *param)
{
    DAEMON_PRINT_LOG(INFO,"GMI_UpdateServer  Start Port  ! ! ");

    if(g_UpdatePort < 1025)
    {
        GMI_DeBugPrint("[%s][%d] System Get UpdatePort = [%d] is low System default Port ,Use Update Default Port [8000]! ",__func__,__LINE__,g_UpdatePort);
        g_UpdatePort = GMI_DAEMON_UPDATE_SERVER_PORT;
    }

    GMI_CreateTCPServer("0.0.0.0", g_UpdatePort);

    return (void *)0;
}

/*=======================================================
name				:	GMI_SystemUpdateServerInit
function			:  Create System update Server
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	7/10/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT  GMI_SystemUpdateServerInit(void)
{
    GMI_RESULT Ret = GMI_FAIL;

    Ret = GMI_CreateTask(190, 8192, (fTaskEntryPoint)GMI_UpdateServer, 0, 0, 0, 0, 0, 0, 0);
    if (FAILED(Ret))
        return GMI_FAIL;

    return GMI_SUCCESS;

}

