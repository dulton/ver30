
#include <sdk_sys_cp.h>
#include <sdk_server_debug.h>
#include <sys_stream_info.h>

/*******************************************
* for sys cp protocol handling
*******************************************/
SdkSysCp::SdkSysCp()
{
    m_pPkgHeader = NULL;
    m_PkgSize = 0;
    m_pData = NULL;
    m_DataSize = 0;
    m_Code = 0;
    m_AttrCount = 0;
    m_SessionId = 0;
    m_SeqId = 0;
    m_PkgInit = 0;
    m_DataInit = 0;
    m_CodeInit = 0;
    m_AttrCountInit = 0;
    m_SessionInit = 0;
    m_SeqIdInit = 0;
}

int SdkSysCp::ParsePkg(sdk_client_comm_t *pComm)
{
    int ret;
    SysPkgHeader* pPkgHeader=NULL;
    void* pData=NULL;
    int datasize=0;
    uint16_t n16;
    if(pComm->m_Frag)
    {
        ret = -EINVAL;
        ERROR_INFO("Frag set\n");
        goto fail;
    }
    if(pComm->m_DataLen < sizeof(*pPkgHeader))
    {
        ret = -EINVAL;
        ERROR_INFO("datalen(%d) < sizeof(%d)\n",pComm->m_DataLen,sizeof(*pPkgHeader));
        goto fail;
    }
    pPkgHeader =(SysPkgHeader*) malloc(pComm->m_DataLen);
    if(pPkgHeader == NULL)
    {
        ret = -errno ? -errno : -1;
        ERROR_INFO("malloc pkgheader size %d error (%d)\n",pComm->m_DataLen,ret);
        goto fail;
    }

    memcpy(pPkgHeader,pComm->m_Data,pComm->m_DataLen);
    /*now we should get the seq and sesid*/
    this->m_SessionId = pComm->m_SesId;
    this->m_SeqId = pComm->m_SeqId;

    /*now to get the buffer*/
    this->m_AttrCount = PROTO_TO_HOST16(pPkgHeader->s_AttrCount);
    if(pPkgHeader->s_HeaderLen != sizeof(*pPkgHeader))
    {
        ret = -EINVAL;
        ERROR_INFO("package headerlen (%d) != sizeof(%d)\n",
                   pPkgHeader->s_HeaderLen,sizeof(*pPkgHeader));
        goto fail;
    }

    /*now check data*/
    datasize = pComm->m_DataLen - sizeof(*pPkgHeader);

    n16 = PROTO_TO_HOST16(pPkgHeader->s_TotalLen);
    if(n16 != pComm->m_DataLen)
    {
        ret = -EINVAL;
        ERROR_INFO("totallen(%d) != datalen(%d)\n",
                   n16,pComm->m_DataLen);
        goto fail;
    }

    n16 = PROTO_TO_HOST16(pPkgHeader->s_SessionId);
    if(n16 != this->m_SessionId)
    {
        ret = -EINVAL;
        ERROR_INFO("sessionid (%d) != comm sessionid (%d)\n",
                   n16,this->m_SessionId);
        goto fail;
    }

    n16 = PROTO_TO_HOST16(pPkgHeader->s_SeqNum);
    if(n16 != this->m_SeqId)
    {
        ret = -EINVAL;
        ERROR_INFO("seqid (%d) != comm seqid (%d)\n",
                   n16,this->m_SeqId);
        goto fail;
    }
	n16 = PROTO_TO_HOST16(pPkgHeader->s_Code);
	this->m_Code = n16;

    if(datasize > 0)
    {
        pData = malloc(datasize);
        if(pData == NULL)
        {
            ret = -errno ? -errno : -1;
            ERROR_INFO("malloc datasize %d error(%d)\n",datasize,ret);
            goto fail;
        }

        memcpy(pData,(void*)((ptr_t)pPkgHeader+sizeof(*pPkgHeader)),datasize);
    }

    if(this->m_pPkgHeader)
    {
        free(this->m_pPkgHeader);
    }
    this->m_pPkgHeader = NULL;
    if(this->m_pData)
    {
        free(this->m_pData);
    }
    this->m_pData = NULL;

    this->m_pPkgHeader = pPkgHeader;
    this->m_PkgSize = pComm->m_DataLen;
    this->m_pData = pData;
    this->m_DataSize = datasize;
    pPkgHeader = NULL;
    pData = NULL;


    this->m_PkgInit = 1;
    this->m_DataInit = 1;
    this->m_AttrCountInit = 1;
    this->m_SessionInit = 1;
    this->m_SeqIdInit = 1;
    this->m_CodeInit = 1;

    return 1;
fail:
    if(pPkgHeader)
    {
        free(pPkgHeader);
    }
    pPkgHeader = NULL;
    if(pData)
    {
        free(pData);
    }
    pData = NULL;
    return ret;
}

SdkSysCp::~SdkSysCp()
{
    this->m_SeqId = 0;
    this->m_SessionId = 0;
    this->m_PkgSize = 0;
    this->m_Code = 0;
    this->m_AttrCount = 0;
    this->m_DataSize = 0;
    if(this->m_pData)
    {
        SDK_ASSERT(this->m_DataInit);
        free(this->m_pData);
    }
    this->m_pData = NULL;
    if(this->m_pPkgHeader)
    {
        SDK_ASSERT(this->m_PkgInit);
        free(this->m_pPkgHeader);
    }
    this->m_pPkgHeader = NULL;

    this->m_PkgInit = 0;
    this->m_DataInit = 0;
    this->m_CodeInit = 0;
    this->m_AttrCountInit = 0;
    this->m_SessionInit = 0;
    this->m_SeqIdInit = 0;
}


int SdkSysCp::SetCode(uint32_t code)
{
    this->m_Code = code;
    this->m_CodeInit = 1;
    return 0;
}

int SdkSysCp::SetSessionSeq(uint32_t sesid,uint32_t seqid)
{
    this->m_SeqId = seqid;
    this->m_SessionId = sesid;
    this->m_SeqIdInit = 1;
    this->m_SessionInit = 1;
    return 0;
}

int SdkSysCp::SetAttrCount(uint32_t attrcount)
{
    this->m_AttrCount = attrcount;
    this->m_AttrCountInit = 1;
    return 0;
}

int SdkSysCp::SetData(void * pData,int datalen)
{
    int ret;

    if(datalen > 0 && pData == NULL)
    {
        return -EINVAL;
    }

    if(this->m_pData)
    {
        free(this->m_pData);
    }
    this->m_pData = NULL;
    this->m_DataSize = 0;


    if(datalen > 0)
    {
        this->m_pData = malloc(datalen);
        if(this->m_pData == NULL)
        {
            ret = -errno ? -errno : -1;
            ERROR_INFO("datalen(%d) malloc error(%d)\n",
                       datalen,ret);
            return ret;
        }
        memcpy(this->m_pData,pData,datalen);
    }
    this->m_DataSize = datalen;
    this->m_DataInit = 1;
    return 0;
}

int SdkSysCp::GetCode(uint32_t & code)
{
    if(this->m_CodeInit == 0)
    {
        ERROR_INFO("CodeInit(%d)\n",this->m_CodeInit);
        return -ENODATA;
    }
    code = this->m_Code;
    return 0;
}

int SdkSysCp::GetSessionSeq(uint32_t & sesid,uint32_t & seqid)
{
    if(this->m_SessionInit == 0 || this->m_SeqIdInit == 0)
    {
        ERROR_INFO("SessionInit (%d) SeqIdInit(%d)\n",
                   this->m_SessionInit,this->m_SeqIdInit);
        return -ENODATA;
    }
    sesid = this->m_SessionId;
    seqid  = this->m_SeqId;
    return 0;
}

int SdkSysCp::GetAttrCount(uint32_t& attrcount)
{
    if(this->m_AttrCountInit == 0)
    {
        ERROR_INFO("AttrCountInit (%d)\n",this->m_AttrCountInit);
        return -ENODATA;
    }
    attrcount = this->m_AttrCount;
    return 0;
}

SysPkgHeader* SdkSysCp::GetPkgHeader(int& pkgsize)
{
    SysPkgHeader* pPkgHdr=NULL;
    int size=0;
    /*now we should form the pkg size*/
    if(this->m_pPkgHeader)
    {
        pkgsize = this->m_PkgSize;
        pPkgHdr = (SysPkgHeader*)malloc(pkgsize);
        if(pPkgHdr == NULL)
        {
            return NULL;
        }
        memcpy(pPkgHdr,this->m_pPkgHeader,pkgsize);
        return pPkgHdr;
    }

    if(this->m_CodeInit == 0 ||
            this->m_AttrCountInit == 0 ||
            this->m_SessionInit == 0 ||
            this->m_DataInit == 0 ||
            this->m_SeqIdInit == 0)
    {
        ERROR_INFO("CodeInit(%d) AttrCountInit(%d) SessionInit(%d) DataInit(%d) SeqIdInit(%d)\n",
                   this->m_CodeInit,
                   this->m_AttrCountInit,
                   this->m_SessionInit,
                   this->m_DataInit,
                   this->m_SeqIdInit);
        return NULL;
    }

    /*now to format the package*/
    size = this->m_DataSize + sizeof(*pPkgHdr);
    pPkgHdr =(SysPkgHeader*) malloc(size);
    if(pPkgHdr == NULL)
    {
        return NULL;
    }

    /*now format the header*/
    pPkgHdr->s_SysMsgTag[0] = 'G';
    pPkgHdr->s_SysMsgTag[1] = 'S';
    pPkgHdr->s_SysMsgTag[2] = 'M';
    pPkgHdr->s_SysMsgTag[3] = 'T';

    pPkgHdr->s_Version = 1;
    pPkgHdr->s_HeaderLen = sizeof(*pPkgHdr);
    pPkgHdr->s_Code = HOST_TO_PROTO16(this->m_Code);
    pPkgHdr->s_AttrCount = HOST_TO_PROTO16(this->m_AttrCount);
    pPkgHdr->s_SeqNum = HOST_TO_PROTO16(this->m_SeqId);
    pPkgHdr->s_SessionId = HOST_TO_PROTO16(this->m_SessionId);
    pPkgHdr->s_TotalLen = HOST_TO_PROTO16(this->m_DataSize + sizeof(*pPkgHdr));
    if(this->m_DataSize)
    {
        SDK_ASSERT(this->m_pData);
        memcpy((void*)((ptr_t)pPkgHdr+sizeof(*pPkgHdr)),this->m_pData,this->m_DataSize);
    }
    pkgsize = size;

    return pPkgHdr;


}

int SdkSysCp::GetData(void * & pData,uint32_t & datasize)
{
    void* pNewData=NULL;
    int newsize=0;
    int ret;
    if(pData || datasize)
    {
        return -EINVAL;
    }

    if(this->m_DataInit == 0 && this->m_PkgInit == 0)
    {
        return -ENODATA;
    }


    if(this->m_pData)
    {
        pNewData = malloc(this->m_DataSize);
        if(pNewData == NULL)
        {
            ret = -errno ? -errno : -1;
            goto fail;
        }
        newsize = this->m_DataSize;
        memcpy(pNewData,this->m_pData,newsize);
    }

    pData = pNewData;
    datasize = newsize;


    return newsize;

fail:
    if(pNewData)
    {
        free(pNewData);
    }
    pNewData = NULL;
    return ret;
}

sdk_client_comm_t* SdkSysCp::GetClientComm()
{
    SysPkgHeader* pPkgHeader=NULL;
    int pkgsize=0;
    sdk_client_comm_t* pComm=NULL;

    pPkgHeader = this->GetPkgHeader(pkgsize);
    if(pPkgHeader == NULL)
    {
        return NULL;
    }

    pComm = AllocateComm(pkgsize);
    if(pComm == NULL)
    {
        goto fail;
    }

    if(this->m_SessionInit == 0 ||
            this->m_SeqIdInit == 0)
    {
        ERROR_INFO("SessionInit(%d) SeqIdInit(%d)\n",
                   this->m_SessionInit,this->m_SeqIdInit);
        goto fail;
    }

    pComm->m_SesId = this->m_SessionId;
    pComm->m_Priv = 101;
    pComm->m_ServerPort = 0;
    pComm->m_LocalPort = 0;
    pComm->m_SeqId = this->m_SeqId;
    pComm->m_Type = GMIS_PROTOCOL_TYPE_SDK_TO_SYS;
    pComm->m_FHB = 0;
    pComm->m_Frag = 0;
    pComm->m_DataId = 0;
    pComm->m_Offset = 0;
    pComm->m_Totalsize = 0;
    pComm->m_DataLen = pkgsize;
    memcpy(pComm->m_Data,pPkgHeader,pkgsize);

    if(pPkgHeader)
    {
        free(pPkgHeader);
    }
    pPkgHeader = NULL;
    return pComm;
fail:
    if(pPkgHeader)
    {
        free(pPkgHeader);
    }
    pPkgHeader = NULL;
    FreeComm(pComm);
    return NULL;
}

