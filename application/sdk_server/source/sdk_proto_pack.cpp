
#include <sdk_proto_pack.h>
#include <sys_env_types.h>

#ifdef SDK_UNITTEST
#include <assert.h>
#undef SDK_ASSERT
#undef ERROR_INFO
#undef DEBUG_INFO
#undef SETERRNO
#undef GETERRNO
#define SDK_ASSERT  assert
#define ERROR_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__ASSERT_FUNCTION,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)
#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__ASSERT_FUNCTION,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)
#define SETERRNO(ret)  (errno = (ret))
#define GETERRNO() ( errno ? errno : 1)
#else
#include <sdk_server_debug.h>
#endif

#undef VECS_EQUAL
#define  VECS_EQUAL() \
do\
{\
	SDK_ASSERT(this->m_pItemData.size() == this->m_ItemCode.size());\
	SDK_ASSERT(this->m_ItemCode.size() == this->m_ItemLength.size());\
}while(0)

SdkProtoPack::SdkProtoPack(int code,sessionid_t sesid,seqid_t seqid) : m_Code(code),
    m_SesId(sesid),m_SeqId(seqid)
{
    m_pFormat = NULL;
    m_FormatLen = 0;
    m_Formated = 0;
    SDK_ASSERT(m_pItemData.size() == 0);
    SDK_ASSERT(m_ItemCode.size() == 0);
    SDK_ASSERT(m_ItemLength.size() == 0);
}

SdkProtoPack::~SdkProtoPack()
{
    this->Reset();
    this->m_Code = 0;
    this->m_SeqId = 0;
    this->m_SesId = 0;
}

void SdkProtoPack::__ClearItemCode()
{
    void *pItem=NULL;
    VECS_EQUAL();
    while(this->m_pItemData.size() > 0)
    {
        SDK_ASSERT(pItem == NULL);
        pItem = this->m_pItemData[0];
        this->m_pItemData.erase(this->m_pItemData.begin());
        this->m_ItemCode.erase(this->m_ItemCode.begin());
        this->m_ItemLength.erase(this->m_ItemLength.begin());
        free(pItem);
        pItem = NULL;
    }
    return ;
}

void SdkProtoPack::Reset(int code,sessionid_t sesid,seqid_t seqid)
{
    this->m_Formated = 0;
    if(this->m_pFormat)
    {
        free(this->m_pFormat);
    }
    this->m_pFormat = NULL;
    this->m_FormatLen = 0;
    this->__ClearItemCode();
    if(code != 0)
    {
        this->m_Code = code;
    }
    if(sesid != 0)
    {
        this->m_SesId = sesid;
    }
    if(seqid != 0)
    {
        this->m_SeqId = seqid;
    }
    return;
}

int SdkProtoPack::AddItem(int itemcode,void * pData,int datalen)
{
    void* pItemData=NULL;
    int ret;

    pItemData = calloc(1,datalen);
    if(pItemData == NULL)
    {
        ret = GETERRNO();
        SETERRNO(ret);
        return -ret;
    }

    VECS_EQUAL();
    memcpy(pItemData,pData,datalen);
    this->m_pItemData.push_back(pItemData);
    this->m_ItemCode.push_back(itemcode);
    this->m_ItemLength.push_back(datalen);
    /*this will give new format data*/
    this->m_Formated = 0;
    return 0;
}

int SdkProtoPack::__FormatCode()
{
    int callen=0;
    int offset=0;
    int ret;
    SysPkgHeader* pSysPkgHeader=NULL;
    SysPkgAttrHeader* pAttr=NULL;
    void* pCurPtr=NULL;
    unsigned int i;
    SDK_ASSERT(this->m_Formated == 0);
    callen = sizeof(*pSysPkgHeader);

    for(i=0; i<this->m_ItemLength.size(); i++)
    {
        callen += sizeof(*pAttr);
        callen += this->m_ItemLength[i];
    }

    if(this->m_pFormat)
    {
        free(this->m_pFormat);
    }
    this->m_pFormat = NULL;
    this->m_FormatLen = callen;
    this->m_pFormat = calloc(callen ,1);
    if(this->m_pFormat == NULL)
    {
        ret = GETERRNO();
        SETERRNO(ret);
        return -ret;
    }

    offset = 0;
    SDK_ASSERT(offset <= callen);
    VECS_EQUAL();
    pSysPkgHeader = (SysPkgHeader*)this->m_pFormat;

    /*now first to set for sys header*/
    pSysPkgHeader->s_SysMsgTag[0] = 'G';
    pSysPkgHeader->s_SysMsgTag[1] = 'S';
    pSysPkgHeader->s_SysMsgTag[2] = 'M';
    pSysPkgHeader->s_SysMsgTag[3] = 'T';
    pSysPkgHeader->s_Version = 1;
    pSysPkgHeader->s_HeaderLen = sizeof(*pSysPkgHeader);
    pSysPkgHeader->s_Code = HOST_TO_PROTO16(this->m_Code);
    pSysPkgHeader->s_AttrCount = HOST_TO_PROTO16(this->m_ItemLength.size());
    pSysPkgHeader->s_SeqNum = HOST_TO_PROTO16(this->m_SeqId);
    pSysPkgHeader->s_SessionId = HOST_TO_PROTO16(this->m_SesId);
    pSysPkgHeader->s_TotalLen = HOST_TO_PROTO16(callen);

    offset += sizeof(*pSysPkgHeader);
    for(i=0; i<this->m_ItemLength.size() ; i++)
    {
        SDK_ASSERT(offset <= callen);
        SDK_ASSERT((offset + this->m_ItemLength[i]) <= callen);
        pAttr = (SysPkgAttrHeader*)((unsigned char*)this->m_pFormat + offset);
        pAttr->s_Type = HOST_TO_PROTO16(this->m_ItemCode[i]);
        pAttr->s_Length = HOST_TO_PROTO16((sizeof(*pAttr) + this->m_ItemLength[i]));
        offset += sizeof(*pAttr);
        pCurPtr =(void*)((unsigned char*)this->m_pFormat + offset);
        memcpy(pCurPtr,this->m_pItemData[i],this->m_ItemLength[i]);
        offset += this->m_ItemLength[i];
    }
    this->m_Formated = 1;
    return 0;
}

void* SdkProtoPack::GetPtr()
{
    int ret;
    if(this->m_Formated== 0)
    {
        ret = this->__FormatCode();
        if(ret < 0)
        {
            ret = GETERRNO();
            SETERRNO(ret);
            return NULL;
        }
    }

    return this->m_pFormat;
}

int SdkProtoPack::GetLength()
{
    int ret;
    if(this->m_Formated== 0)
    {
        ret = this->__FormatCode();
        if(ret < 0)
        {
            ret = GETERRNO();
            SETERRNO(ret);
            return -ret;
        }
    }

    return this->m_FormatLen;
}


