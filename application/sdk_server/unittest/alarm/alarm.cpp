
#include <sdk_proto_pack.h>
#include <sdk_proto_parse.h>
#include <ctype.h>
#include <memory>
#include <rudp/gmi_rudp_api.h>
#include <sys_env_types.h>

#include <assert.h>
#define SDK_ASSERT  assert
#define ERROR_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__ASSERT_FUNCTION,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)
#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__ASSERT_FUNCTION,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)
#define SETERRNO(ret)  (errno = (ret))
#define GETERRNO() ( errno ? errno : 1)

#include <sdk_proto_pack.cpp>
#include <sdk_proto_parse.cpp>


#define TYPE_WARNING_INFO      122
#define SYSCODE_GET_ALARM_REQ  1229
#define SYSCODE_GET_ALARM_RSP  1230

#if 0
typedef struct tagAlarmInfo
{
    uint64_t  s_WarningId;
    int32_t  s_WarningType;
    int32_t  s_WarningLevel;
    int32_t   s_OnOff;
    uint8_t   s_Time[36];
    uint8_t   s_Devid[64];
    uint8_t   s_Description[128] ;
    union
    {
        uint32_t  s_IoNum;
    } s_ExtraInfo;
} SysPkgAlarmInfo;
#endif

#define CHECK_SKIP_SPACE(...)  \
do\
{\
	if (pEndPtr == NULL)\
	{\
		err = EINVAL;\
		ERROR_INFO(__VA_ARGS__);\
		goto fail;\
	}\
    pCurPtr = pEndPtr;\
    while(isspace(*pCurPtr))\
    {\
        pCurPtr ++;\
    }\
}\
while(0)

SysPkgAlarmInfo* ParseLine(const char* pLine)
{
    int err=0;
    char* pCurPtr,*pEndPtr=NULL;
    char* pCopyPtr=NULL;
    SysPkgAlarmInfo* pAlarmInfo=NULL;

    pCopyPtr = strdup(pLine);
    if(pCopyPtr == NULL)
    {
        err= GETERRNO();
        goto fail;
    }

    pCurPtr=pCopyPtr;

    pAlarmInfo = (SysPkgAlarmInfo*)calloc(sizeof(*pAlarmInfo),1);
    if(pAlarmInfo == NULL)
    {
        err = GETERRNO();
        goto fail;
    }

    pAlarmInfo->s_WaringId = strtoull(pCurPtr,&pEndPtr,10);
    CHECK_SKIP_SPACE("(%s) not valid for id\n",pCurPtr);

    pAlarmInfo->s_WaringType = strtoul(pCurPtr,&pEndPtr,10);
    CHECK_SKIP_SPACE("(%s) not valid for type\n",pCurPtr);

    pAlarmInfo->s_WaringLevel = strtoul(pCurPtr,&pEndPtr,10);
    CHECK_SKIP_SPACE("(%s) not valid for level\n",pCurPtr);

    pAlarmInfo->s_OnOff = strtoul(pCurPtr,&pEndPtr,10);
    CHECK_SKIP_SPACE("(%s) not valid for onoff\n",pCurPtr);

    pEndPtr = strchr(pCurPtr,' ');
    if(pEndPtr == NULL)
    {
        err = EINVAL;
        ERROR_INFO("(%s) not valid for time\n",pCurPtr);
        goto fail;
    }
    *pEndPtr = '\0';
    strncpy((char*)pAlarmInfo->s_Time,pCurPtr,sizeof(pAlarmInfo->s_Time));

    pCurPtr = pEndPtr+1;
    while(isspace(*pCurPtr))
    {
        pCurPtr++;
    }

    pEndPtr = strchr(pCurPtr,' ');
    if(pEndPtr == NULL)
    {
        err = EINVAL;
        ERROR_INFO("(%s) not valid for devid\n",pCurPtr);
        goto fail;
    }
    *pEndPtr = '\0';
    strncpy((char*)pAlarmInfo->s_DevId,pCurPtr,sizeof(pAlarmInfo->s_DevId));
    pCurPtr = pEndPtr+1;
    while(isspace(*pCurPtr))
    {
        pCurPtr++;
    }

    strncpy((char*)pAlarmInfo->s_Description,pCurPtr,sizeof(pAlarmInfo->s_Description));

    if(pCopyPtr)
    {
        free(pCopyPtr);
    }
    pCopyPtr = NULL;
    return pAlarmInfo;
fail:
    if(pAlarmInfo)
    {
        free(pAlarmInfo);
    }
    pAlarmInfo = NULL;
    if(pCopyPtr)
    {
        free(pCopyPtr);
    }
    pCopyPtr = NULL;
    SETERRNO(err);
    return NULL;

}

int HostToProtoAlarmInfo(SysPkgAlarmInfo * pAlarmInfo)
{
    pAlarmInfo->s_WaringId = HOST_TO_PROTO64(pAlarmInfo->s_WaringId);
    pAlarmInfo->s_WaringType = HOST_TO_PROTO32(pAlarmInfo->s_WaringType);
    pAlarmInfo->s_WaringLevel = HOST_TO_PROTO32(pAlarmInfo->s_WaringLevel);
    pAlarmInfo->s_OnOff = HOST_TO_PROTO32(pAlarmInfo->s_OnOff);
    return 0;
}

int SendAlarm(FD_HANDLE handle,int rport,SysPkgAlarmInfo* pAlarmInfo,sessionid_t sesid,seqid_t seqid,privledge_t priv)
{
    std::auto_ptr<SdkProtoPack> pPack2(new SdkProtoPack(SYSCODE_GET_ALARM_RSP,sesid,seqid));
    SdkProtoPack* pPack=pPack2.get();
    std::auto_ptr<char> pChar2(new char[1500]);
    char* pChar = pChar2.get();
    std::auto_ptr<SdkProtoParse> pParse2(new SdkProtoParse());
    SdkProtoParse* pParse = pParse2.get();
    int ret,packlen,code;
    void* pPtr=NULL;
    PkgRudpSendInput sinput= {0};
    PkgRudpSendOutput soutput= {0};
    PkgRudpRecvInput rinput= {0};
    PkgRudpRecvOutput routput= {0};
    SysPkgMessageCode* pMessage=NULL;
    int messagelen=0;
    GMI_RESULT gmiret;
    sessionid_t getsesid;
    seqid_t getseqid;
	DEBUG_INFO("sizeof(SysPkgAttrHeader) %d sizeof(SysPkgAlarmInfo) %d\n",sizeof(SysPkgAttrHeader),sizeof(SysPkgAlarmInfo));
    ret = pPack->AddItem(TYPE_WARNING_INFO,pAlarmInfo,sizeof(*pAlarmInfo));
    if(ret < 0)
    {
        ret = GETERRNO();
        goto fail;
    }

    packlen = pPack->GetLength();
    if(packlen < 0)
    {
        ret = GETERRNO();
        goto fail;
    }

    pPtr = pPack->GetPtr();
    if(pPtr == NULL)
    {
        ret = GETERRNO();
        goto fail;
    }

    sinput.s_AuthValue = priv;
    sinput.s_SessionId = sesid;
    sinput.s_MessageType = RUDP_MESSAGE_TYPE_COMPLETE;
    sinput.s_Buffer =(uint8_t*) pPtr;
    sinput.s_SendLength = packlen;
    sinput.s_RemotePort = rport;
    sinput.s_TimeoutMS = 1000;
    sinput.s_LocalSvrPort = 0;
    sinput.s_PktTotalLength = packlen;
    sinput.s_MsgOffsetofPkt = 0;
    sinput.s_SequenceNum = seqid;

    gmiret = GMI_RudpSend(handle,&sinput,&soutput);
    if(FAILED(gmiret))
    {
        ret = GETERRNO();
        ERROR_INFO("Send Error(%d)\n",ret);
        goto fail;
    }

    rinput.s_TimeoutMS = 1000;
    routput.s_Buffer =(uint8_t*) pChar;
    routput.s_BufferLength = 1500;

    gmiret = GMI_RudpRecv(handle,&rinput,&routput);
    if(FAILED(gmiret))
    {
        ret = GETERRNO();
        ERROR_INFO("Receive Error(%d)\n",ret);
        goto fail;
    }

    ret = pParse->Parse(pChar,routput.s_RecvLength);
    if(ret < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("Parse Error (%d)\n",ret);
        goto fail;
    }

    ret = pParse->GetCode(code);
    if(ret < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("Get Code Error (%d)\n",ret);
        goto fail;
    }

    if(code != SYSCODE_GET_ALARM_REQ)
    {
        ret = EINVAL;
        ERROR_INFO("code (%d) != SYSCODE_GET_ALARM_REQ (%d)\n",code,SYSCODE_GET_ALARM_REQ);
        goto fail;
    }

    ret = pParse->GetSessionSeq(getsesid,getseqid);
    if(ret < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("GetSeqSes Error(%d)\n",ret);
        goto fail;
    }

    if(getsesid != sesid ||
            getseqid != seqid)
    {
        ret = EINVAL;
        ERROR_INFO("sesid (%d) != (%d) or seqid (%d) != (%d)\n",
                   getsesid,sesid,getseqid,seqid);
        goto fail;
    }

    ret = pParse->GetAttr(0,code,pPtr,messagelen);
    if(ret < 0)
    {
        ret = GETERRNO();
        ERROR_INFO("GetAttr(0) Error(%d)\n",ret);
        goto fail;
    }
    pMessage = (SysPkgMessageCode*)pPtr;

    if(code != TYPE_MESSAGECODE)
    {
        ret = EINVAL;
        ERROR_INFO("Attr(%d) != TYPE_MESSAGECODE(%d)\n",code,TYPE_MESSAGECODE);
        goto fail;
    }

    if(messagelen < (int)sizeof(*pMessage))
    {
        ret = EINVAL;
        ERROR_INFO("MessageLen(%d) < sizeof(%d)\n",messagelen,sizeof(*pMessage));
        goto fail;
    }

    if(PROTO_TO_HOST32(pMessage->s_MessageCode) != 0)
    {
        ret = EFAULT;
        ERROR_INFO("messagecode(%d) != 0\n",PROTO_TO_HOST32(pMessage->s_MessageCode));
        goto fail;
    }

    DEBUG_INFO("warningid %lld\n",PROTO_TO_HOST64(pAlarmInfo->s_WaringId));
    DEBUG_INFO("warningtype %d\n",PROTO_TO_HOST32(pAlarmInfo->s_WaringType));
    DEBUG_INFO("warninglevel %d\n",PROTO_TO_HOST32(pAlarmInfo->s_WaringLevel));
    DEBUG_INFO("OnOff %d\n",PROTO_TO_HOST32(pAlarmInfo->s_OnOff));
    DEBUG_INFO("Time (%s)\n",pAlarmInfo->s_Time);
    DEBUG_INFO("devid (%s)\n",pAlarmInfo->s_DevId);
    DEBUG_INFO("description (%s)\n",pAlarmInfo->s_Description);
	DEBUG_INFO("Success\n");

    return 0;
fail:
    assert(ret > 0);

    SETERRNO(ret);
    return -ret;
}

#define RPORT_DEFAULT   57024
#define LPORT_DEFAULT   30112
#define SES_DEFAULT     100
#define SEQNUM_DEFAULT  130
#define PRIV_DEFAULT    101

static int st_Rport=RPORT_DEFAULT;
static int st_Lport=LPORT_DEFAULT;
static const char* st_File=NULL;
static int st_SessionId=SES_DEFAULT;
static int st_Priv=PRIV_DEFAULT;
static int st_SeqNum=SEQNUM_DEFAULT;


void Usage(int ec,const char* fmt,...)
{
    FILE* fp=stderr;
    va_list ap;
    if(ec == 0)
    {
        fp = stdout;
    }
    if(fmt)
    {
        va_start(ap,fmt);
        vfprintf(fp,fmt,ap);
        fprintf(fp,"\n");
    }

    fprintf(fp,"alarm [OPTIONS] file\n");
    fprintf(fp,"\t-h|--help           to display this information\n");
    fprintf(fp,"\t-r|--rport  port    to specify remote port default is (%d)\n",RPORT_DEFAULT);
    fprintf(fp,"\t-l|--lport  port    to specify local port default is (%d)\n",LPORT_DEFAULT);
    fprintf(fp,"\t-s|--sesid  sesid   to specify session id default is (%d)\n",SES_DEFAULT);
    fprintf(fp,"\t-S|--seqnum seqnum  to specify seqnum  default is (%d)\n",SEQNUM_DEFAULT);
    fprintf(fp,"\t-p|--priv   priv    to specify priviledge default is (%d)\n",PRIV_DEFAULT);
    exit(ec);
}

void ParseParam(int argc,char* argv[])
{
    int i,idx=-1;

    for(i=1; i<argc; i++)
    {
        if(strcmp(argv[i],"-h")==0 ||
                strcmp(argv[i],"--help")==0)
        {
            Usage(0,NULL);
        }
        else if(strcmp(argv[i],"-r")==0 ||
                strcmp(argv[i],"--rport")==0)
        {
            if((i+1) >= argc)
            {
                Usage(3,"%s need an arg",argv[i]);
            }
            st_Rport = atoi(argv[i+1]);
            i+=1;
        }
        else if(strcmp(argv[i],"-l")==0 ||
                strcmp(argv[i],"--lport")==0)
        {
            if((i+1) >= argc)
            {
                Usage(3,"%s need an arg",argv[i]);
            }
            st_Lport = atoi(argv[i+1]);
            i+=1;
        }
        else if(strcmp(argv[i],"-s")==0 ||
                strcmp(argv[i],"--sesid")==0)
        {
            if((i+1) >= argc)
            {
                Usage(3,"%s need an arg",argv[i]);
            }
            st_SessionId= atoi(argv[i+1]);
            i+=1;
        }
        else if(strcmp(argv[i],"-S")==0 ||
                strcmp(argv[i],"--seqnum")==0)
        {
            if((i+1) >= argc)
            {
                Usage(3,"%s need an arg",argv[i]);
            }
            st_SeqNum= atoi(argv[i+1]);
            i+=1;
        }
        else if(strcmp(argv[i],"-p")==0 ||
                strcmp(argv[i],"--priv")==0)
        {
            if((i+1) >= argc)
            {
                Usage(3,"%s need an arg",argv[i]);
            }
            st_Priv = atoi(argv[i+1]);
            i+=1;
        }
        else
        {
            idx = i;
            break;
        }
    }

    if(idx == -1)
    {
        Usage(3,"Must Specify a file");
    }
    st_File = argv[idx];
    return ;
}

int SendFile(const char* file,int lport,int rport,int sesid,int seqid,int priv)
{
    FD_HANDLE SockFd=NULL;
    FILE* fp=NULL;
    char* pLine=NULL,*pCRLF=NULL;
    size_t linesize=0;
    int ret;
    SysPkgAlarmInfo* pAlarmInfo=NULL;

    SockFd = GMI_RudpSocket(lport);
    if(SockFd == NULL)
    {
        ret = GETERRNO();
        ERROR_INFO("Socket lPort(%d) Error(%d)\n",lport,ret);
        goto fail;
    }

    fp = fopen(file,"r");
    if(fp == NULL)
    {
        ret=  GETERRNO();
        ERROR_INFO("File (%s) Error(%d)\n",file,ret);
        goto fail;
    }

    while(1)
    {
        ret = getline(&pLine,&linesize,fp);
        if(ret == -1)
        {
            break;
        }

        pCRLF = strchr(pLine,'\r');
        if(pCRLF)
        {
            *pCRLF = '\0';
        }

        pCRLF = strchr(pLine,'\n');
        if(pCRLF)
        {
            *pCRLF = '\0';
        }

        pAlarmInfo = ParseLine(pLine);
        if(pAlarmInfo == NULL)
        {
            ret = GETERRNO();
            ERROR_INFO("Parse(%s) Error(%d)\n",pLine,ret);
            goto fail;
        }

        ret = HostToProtoAlarmInfo(pAlarmInfo);
        if(ret < 0)
        {
            ret = GETERRNO();
            ERROR_INFO("HostToProtoAlarmInfo Error(%d)\n",ret);
            goto fail;
        }

        ret = SendAlarm(SockFd,rport,pAlarmInfo,sesid,seqid,priv);
        if(ret < 0)
        {
            ret =GETERRNO();
            ERROR_INFO("Send(%s) Error(%d)\n",pLine,ret);
            goto fail;
        }

        free(pAlarmInfo);
        pAlarmInfo = NULL;
    }

    if(pAlarmInfo)
    {
        free(pAlarmInfo);
    }
    pAlarmInfo = NULL;
    if(fp)
    {
        fclose(fp);
    }
    fp=NULL;
    if(SockFd)
    {
        GMI_RudpSocketClose(SockFd);
    }
    SockFd = NULL;
    if(pLine)
    {
        free(pLine);
    }
    pLine = NULL;
    linesize =0;

    return 0;
fail:
    assert(ret > 0);
    if(pAlarmInfo)
    {
        free(pAlarmInfo);
    }
    pAlarmInfo = NULL;
    if(fp)
    {
        fclose(fp);
    }
    fp=NULL;
    if(SockFd)
    {
        GMI_RudpSocketClose(SockFd);
    }
    SockFd = NULL;
    if(pLine)
    {
        free(pLine);
    }
    pLine = NULL;
    linesize =0;
    SETERRNO(ret);
    return -ret;

}


int main(int argc,char* argv[])
{
    int ret;

    ParseParam(argc,argv);

    ret = SendFile(st_File,st_Lport,st_Rport,st_SessionId,st_SeqNum,st_Priv);
    if(ret < 0)
    {
        return -3;
    }

    return 0;
}

