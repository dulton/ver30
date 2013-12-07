
#include <sdk_proto_parse.h>
#include <sys_env_types.h>

#ifdef SDK_UNITTEST
#include <assert.h>
#define SDK_ASSERT  assert
#define ERROR_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__ASSERT_FUNCTION,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)
#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__ASSERT_FUNCTION,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)
#define SETERRNO(ret)  (errno = (ret))
#define GETERRNO() ( errno ? errno : 1)
#else
#include <sdk_server_debug.h>
#endif

#undef VECS_EQUAL
#define VECS_EQUAL()  \
do\
{\
	SDK_ASSERT(m_AttrOffsetVecs.size() == m_AttrVecs.size());\
	SDK_ASSERT(m_AttrVecs.size() == m_AttrLenVecs.size());\
}while(0)

SdkProtoParse::SdkProtoParse()
{
    m_Parsed = 0;
    m_pData = NULL;
    m_DataLen = 0;
    m_Code = 0;
    m_SesId = 0;
    m_SeqId = 0;
    SDK_ASSERT(m_AttrOffsetVecs.size() == 0);
    VECS_EQUAL();
}

void SdkProtoParse::__Reset()
{
    this->m_AttrOffsetVecs.clear();
    this->m_AttrLenVecs.clear();
    this->m_AttrVecs.clear();
    if(this->m_pData)
    {
        free(this->m_pData);
    }
    this->m_pData = NULL;
    this->m_DataLen = 0;
    this->m_Code = 0;
    this->m_SesId = 0;
    this->m_SeqId = 0;
    this->m_Parsed = 0;
}

SdkProtoParse::~SdkProtoParse()
{
    VECS_EQUAL();
    this->__Reset();
}

int SdkProtoParse::Parse(void * pData,int datalen)
{
    int ret;
    SysPkgHeader* pHeader=NULL;
    SysPkgAttrHeader* pAttr=NULL;
    unsigned char* pCurPtr,*pBasePtr;
    int parselen=0;
    unsigned int attrcount,i;
    uint16_t attr,attrlen,attroff;
    VECS_EQUAL();
    this->__Reset();

    if(datalen < (int)sizeof(*pHeader))
    {
        ret = EINVAL;
        goto fail;
    }

    /*now first to check for the valid*/
    this->m_pData = calloc(datalen,1);
    if(this->m_pData == NULL)
    {
        ret = GETERRNO();
        goto fail;
    }
    this->m_DataLen = datalen;

    memcpy(this->m_pData,pData,datalen);
    pHeader = (SysPkgHeader*)this->m_pData;
    /*now check for the tag*/
    if(pHeader->s_SysMsgTag[0] != 'G' ||
            pHeader->s_SysMsgTag[1] != 'S' ||
            pHeader->s_SysMsgTag[2] != 'M' ||
            pHeader->s_SysMsgTag[3] != 'T')
    {
        ret = EINVAL;
        ERROR_INFO("header tag[0x%02x:0x%02x:0x%02x:0x%02x]not valid\n",
                   pHeader->s_SysMsgTag[0],
                   pHeader->s_SysMsgTag[1],
                   pHeader->s_SysMsgTag[2],
                   pHeader->s_SysMsgTag[3]);
        goto fail;
    }

    if(pHeader->s_Version != 1)
    {
        ret =EINVAL;
        ERROR_INFO("header version (0x%02x)\n",pHeader->s_Version);
        goto fail;
    }

    if(pHeader->s_HeaderLen!= sizeof(*pHeader))
    {
        ret = EINVAL;
        ERROR_INFO("headerlen 0x%02x\n",pHeader->s_HeaderLen);
        goto fail;
    }

    if(PROTO_TO_HOST16(pHeader->s_TotalLen) > datalen)
    {
        ret = EINVAL;
        ERROR_INFO("totallen (%d) > datalen(%d)\n",
                   PROTO_TO_HOST16(pHeader->s_TotalLen),
                   datalen);
        goto fail;
    }

    this->m_Code = PROTO_TO_HOST16(pHeader->s_Code);
    this->m_SeqId = PROTO_TO_HOST16(pHeader->s_SeqNum);
    this->m_SesId = PROTO_TO_HOST16(pHeader->s_SessionId);

    attrcount = PROTO_TO_HOST16(pHeader->s_AttrCount);

    parselen = sizeof(*pHeader);
    pBasePtr =(unsigned char*)this->m_pData;
    pCurPtr = pBasePtr;
    pCurPtr += parselen;

    for(i=0; i<attrcount; i++)
    {
        pAttr = (SysPkgAttrHeader*)pCurPtr;
        attr = PROTO_TO_HOST16(pAttr->s_Type);
        attrlen = PROTO_TO_HOST16(pAttr->s_Length);
        if((attrlen + parselen) > datalen || (attrlen < sizeof(*pAttr)))
        {
            ret = EINVAL;
            ERROR_INFO("[%d]attrlen(%d) + parselen(%d) > datalen(%d) or < sizeof(%d)\n",i,
                       attrlen,parselen,datalen,sizeof(*pAttr));
            goto fail;
        }
        pCurPtr += sizeof(*pAttr);
        parselen += sizeof(*pAttr);
        attroff = (pCurPtr - pBasePtr);
        this->m_AttrLenVecs.push_back((attrlen - sizeof(*pAttr)));
        this->m_AttrVecs.push_back(attr);
        this->m_AttrOffsetVecs.push_back(attroff);
        pCurPtr += (attrlen - sizeof(*pAttr));
        parselen += (attrlen - sizeof(*pAttr));
    }


    VECS_EQUAL();
    this->m_Parsed = 1;

    return (int) attrcount;
fail:
    SDK_ASSERT(ret > 0);
    this->__Reset();
    SETERRNO(ret);
    return -ret;
}


int SdkProtoParse::GetAttrCount()
{
    int ret;
    if(this->m_Parsed == 0)
    {
        ret =ENOENT;
        SETERRNO(ret);
        return -ret;
    }

    return this->m_AttrVecs.size();
}

int SdkProtoParse::GetCode(int & code)
{
    int ret;
    if(this->m_Parsed == 0)
    {
        ret =ENOENT;
        SETERRNO(ret);
        return -ret;
    }

    code = this->m_Code;
    return 0;
}

int SdkProtoParse::GetSessionSeq(sessionid_t & sesid,seqid_t & seqid)
{
    int ret;
    if(this->m_Parsed == 0)
    {
        ret =ENOENT;
        SETERRNO(ret);
        return -ret;
    }

    sesid = this->m_SesId;
    seqid = this->m_SeqId;

    return 0;
}

int SdkProtoParse::GetAttr(int idx,int & attr,void * & pData,int & datalen)
{
    int ret;
    if(this->m_Parsed == 0)
    {
        ret =ENOENT;
        SETERRNO(ret);
        return -ret;
    }

    if(idx >= (int)this->m_AttrVecs.size())
    {
        ret = ERANGE;
        SETERRNO(ret);
        return -ret;
    }

    attr = this->m_AttrVecs[idx];
    pData = (void*)((unsigned char*)this->m_pData + this->m_AttrOffsetVecs[idx]);
    datalen = this->m_AttrLenVecs[idx];

    return 0;
}



