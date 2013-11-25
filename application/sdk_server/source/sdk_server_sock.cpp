
#include <sdk_server_sock.h>
#include <sdk_server_debug.h>


SdkServerSock::SdkServerSock(int sock) : m_Sock(sock)
{
    m_ReadInit = 0;
    m_ReadLen  = 0;
    m_ReadTotalLen = 0;
    m_ReadHeaderLen = 0;
    memset(m_ReadGSSP,0,sizeof(m_ReadGSSP));
    m_pReadComm = NULL;

    m_WriteInit = 0;
    m_WriteLen  = 0;
    m_WriteTotalLen = 0;
    m_WriteHeaderLen = 0;
    memset(m_WriteGSSP,0,sizeof(m_WriteGSSP));
    m_pWriteComm = NULL;
}

void SdkServerSock::__FreeRead()
{
    this->m_ReadInit = 0;
    this->m_ReadLen = 0;
    this->m_ReadTotalLen = 0;
    this->m_ReadHeaderLen = 0;
    memset(m_ReadGSSP,0,sizeof(m_ReadGSSP));
    if(this->m_pReadComm)
    {
        free(this->m_pReadComm);
    }
    m_pReadComm = NULL;
    return;
}

void SdkServerSock::__FreeWrite()
{
    this->m_WriteInit = 0;
    this->m_WriteLen = 0;
    this->m_WriteTotalLen = 0;
    this->m_WriteHeaderLen = 0;
    memset(m_WriteGSSP,0,sizeof(m_WriteGSSP));
	FreeComm(this->m_pWriteComm);
    return;
}

SdkServerSock::~SdkServerSock()
{
    this->__FreeRead();
    this->__FreeWrite();
    if(this->m_Sock >= 0)
    {
        close(this->m_Sock);
    }
    this->m_Sock = -1;
}

int SdkServerSock::GetSocket()
{
    return this->m_Sock;
}

int SdkServerSock::IsWriteSth()
{
    if(this->m_WriteInit == 0)
    {
        return 0;
    }

    SDK_ASSERT(this->m_WriteLen <= this->m_WriteTotalLen);
    if(this->m_WriteLen == this->m_WriteTotalLen)
    {
        return 0;
    }

    return 1;
}


#define   READ_INIT_ASSERT() \
do\
{\
	SDK_ASSERT(this->m_ReadInit == 0);\
	SDK_ASSERT(this->m_ReadLen == 0);\
	SDK_ASSERT(this->m_ReadTotalLen == 0);\
	SDK_ASSERT(this->m_ReadHeaderLen == 0);\
	SDK_ASSERT(this->m_pReadComm == NULL);\
}while(0)

int SdkServerSock::__ReadInit()
{

    READ_INIT_ASSERT();
    this->m_pReadComm = (sdk_client_comm_t*)malloc(sizeof(*(this->m_pReadComm)));
    if(this->m_pReadComm == NULL)
    {
        return -ENOMEM;
    }
    this->m_ReadInit = 1;
    this->m_ReadLen = 0;
    /*we put the base length*/
    this->m_ReadTotalLen = GSSP_HEADER_BASE_LEN;
    this->m_ReadHeaderLen = GSSP_HEADER_BASE_LEN;
    memset(this->m_ReadGSSP,0,sizeof(this->m_ReadGSSP));

    return 0;
}


int SdkServerSock::Read()
{
    int ret;
    if(this->m_ReadInit == 0)
    {
        /*now we should init the read variable*/
        /*we put the read total len with the least len*/
        ret = this->__ReadInit();
        if(ret < 0)
        {
            this->__FreeRead();
            ERROR_INFO("\n");
            return ret;
        }
    }

    SDK_ASSERT(this->m_ReadInit);
    if(this->m_ReadLen == this->m_ReadTotalLen)
    {
        /*if it is total len ,so we just read all*/
        return 1;
    }

    /*now we should read the length*/
    if(this->m_ReadLen < this->m_ReadHeaderLen)
    {
        ret = this->__ReadNoneBlock(this->m_ReadGSSP + this->m_ReadLen,(this->m_ReadHeaderLen - this->m_ReadLen));
        if(ret < 0)
        {
            ERROR_INFO("\n");
            return ret;
        }
        this->m_ReadLen += ret;
        if(this->m_ReadLen < this->m_ReadHeaderLen)
        {
            return 0;
        }

        ret = this->__AdjustTotalLen();
        if(ret < 0)
        {
            ERROR_INFO("\n");
            return ret;
        }

        if(this->m_ReadLen < this->m_ReadHeaderLen)
        {
            ret = this->__ReadNoneBlock(this->m_ReadGSSP + this->m_ReadLen,(this->m_ReadHeaderLen - this->m_ReadLen));
            if(ret < 0)
            {
                ERROR_INFO("\n");
                return ret;
            }
            this->m_ReadLen += ret;
            if(this->m_ReadLen < this->m_ReadHeaderLen)
            {
                return 0;
            }
        }
    }

    if(this->m_ReadLen >= this->m_ReadHeaderLen)
    {
        ret = this->__ReadNoneBlock((((unsigned char*)this->m_pReadComm->m_Data)+(this->m_ReadLen - this->m_ReadHeaderLen)),(this->m_ReadTotalLen - this->m_ReadLen));
        if(ret < 0)
        {
            ERROR_INFO("\n");
            return ret;
        }
        this->m_ReadLen += ret;
        if(this->m_ReadLen < this->m_ReadTotalLen)
        {
            return 0;
        }
    }

    SDK_ASSERT(this->m_ReadLen == this->m_ReadTotalLen);
    ret = this->__ReadFillComm();
    if(ret < 0)
    {
        ERROR_INFO("\n");
        return ret;
    }
    return 1;
}


int SdkServerSock::__ReadNoneBlock(unsigned char * pData,int datalen)
{
    int ret;
    unsigned char *pCur=(unsigned char*)pData;
    int leftlen = datalen;
    int readlen = 0;

    while(leftlen > 0)
    {
        errno = 0;
        ret = recv(this->m_Sock,pCur,leftlen,MSG_DONTWAIT);
        if(ret < 0)
        {
            ret = -errno ? -errno : -1;
            if(errno != EWOULDBLOCK &&
                    errno != EAGAIN &&
                    errno != EINTR)
            {
                ERROR_INFO("read (%d)\n",errno);
                return ret;
            }
            return readlen;
        }
        else if(ret == 0)
        {
            ERROR_INFO("read (%d)\n",readlen);
            return -EPIPE;
        }
        leftlen -= ret;
        readlen += ret;
        pCur += ret;
    }
    return readlen;
}


int SdkServerSock::__AdjustTotalLen()
{
    gssp_header_t* pGsspHeader = (gssp_header_t*)this->m_ReadGSSP;
    uint32_t h32;

    if(pGsspHeader->m_Magic[0] != 'G' ||
            pGsspHeader->m_Magic[1] != 'S' ||
            pGsspHeader->m_Magic[2] != 'S' ||
            pGsspHeader->m_Magic[3] != 'P')
    {
        ERROR_INFO("\n");
        return -EINVAL;
    }

    if(pGsspHeader->m_HeaderLen > GSSP_HEADER_MAX_LEN)
    {
        ERROR_INFO("headerlen %d\n",pGsspHeader->m_HeaderLen);
        return -EINVAL;
    }
    this->m_ReadHeaderLen = pGsspHeader->m_HeaderLen;
    h32 = PROTO_TO_HOST32(pGsspHeader->m_BodyLength);
    if(h32 > sizeof(this->m_pReadComm->m_Data))
    {
        ERROR_INFO("\n");
        return -EINVAL;
    }
    this->m_ReadTotalLen = this->m_ReadHeaderLen + h32;
    return 0;
}

int SdkServerSock::__ReadFillComm()
{
    sdk_client_comm_t* pComm = this->m_pReadComm;
    gssp_header_t* pGsspHeader = (gssp_header_t*)this->m_ReadGSSP;
    uint32_t h32;
    uint16_t h16;
    SDK_ASSERT(this->m_ReadLen == this->m_ReadTotalLen);
    SDK_ASSERT(this->m_pReadComm);


    h16 = PROTO_TO_HOST16(pGsspHeader->m_SessionId);
    pComm->m_SesId = h16;
    h16 = PROTO_TO_HOST16(pGsspHeader->m_SeqNumber);
    pComm->m_SeqId = h16;
    pComm->m_ServerPort = 0;
    pComm->m_Type = pGsspHeader->m_Type;
	pComm->m_FHB = 0;
    if(pGsspHeader->m_Flag & GSSP_HEADER_FLAG_FHB)
    {
        pComm->m_FHB = 1;
    }

    pComm->m_Offset = 0;
    pComm->m_Totalsize = 0;
    pComm->m_DataId = 0;
    pComm->m_Frag = (this->m_ReadHeaderLen == GSSP_HEADER_BASE_LEN) ? 0 : 1;
    if(pComm->m_Frag)
    {
        h32 = PROTO_TO_HOST32(pGsspHeader->m_DataId);
        pComm->m_DataId = h32;
        h32 = PROTO_TO_HOST32(pGsspHeader->m_TotalLength);
        pComm->m_Totalsize = h32;
        h32 = PROTO_TO_HOST32(pGsspHeader->m_Offset);
        pComm->m_Offset = h32;
    }

    pComm->m_DataLen = this->m_ReadLen - this->m_ReadHeaderLen;

    if(pComm->m_DataLen >= CLIENT_COMM_PACK_SIZE)
    {
        DEBUG_BUFFER(pComm->m_Data,pComm->m_DataLen);
    }

    return 0;
}

sdk_client_comm_t* SdkServerSock::GetRead()
{
    sdk_client_comm_t* pComm=this->m_pReadComm;
    this->m_pReadComm = NULL;
    this->__FreeRead();
    return pComm;
}


void SdkServerSock::ClearRead()
{
    this->__FreeRead();
    return;
}

#define   WRITE_INIT_ASSERT() \
do\
{\
	SDK_ASSERT(this->m_WriteInit == 0);\
	SDK_ASSERT(this->m_WriteLen == 0);\
	SDK_ASSERT(this->m_WriteTotalLen == 0);\
	SDK_ASSERT(this->m_WriteHeaderLen == 0);\
	SDK_ASSERT(this->m_pWriteComm == NULL);\
}while(0)

int SdkServerSock::__AddWriteComm(sdk_client_comm_t * pComm)
{
    gssp_header_t* pGsspHeader= (gssp_header_t*)this->m_WriteGSSP;
    uint32_t n32;
    uint16_t n16;
    WRITE_INIT_ASSERT();
    this->m_WriteInit = 1;
    this->m_WriteLen = 0;
    this->m_WriteHeaderLen = GSSP_HEADER_BASE_LEN;
    if(pComm->m_Frag)
    {
        this->m_WriteHeaderLen= GSSP_HEADER_MAX_LEN;
    }
    this->m_WriteTotalLen = pComm->m_DataLen + this->m_WriteHeaderLen;

    pGsspHeader->m_Magic[0] = 'G';
    pGsspHeader->m_Magic[1] = 'S';
    pGsspHeader->m_Magic[2] = 'S';
    pGsspHeader->m_Magic[3] = 'P';

    pGsspHeader->m_Flag = 0;
    if(pComm->m_Frag)
    {
        pGsspHeader->m_Flag |= GSSP_HEADER_FLAG_FPP;
    }

    if(pComm->m_FHB)
    {
        pGsspHeader->m_Flag |= GSSP_HEADER_FLAG_FHB;
    }

    pGsspHeader->m_Flag |= GSSP_HEADER_FLAG_ENC_DEFAULT;

    pGsspHeader->m_Version =0;
    pGsspHeader->m_Version |= GSSP_HEADER_VERSION_MAJOR;
    pGsspHeader->m_Version |= GSSP_HEADER_VERSION_MINOR;

    n16 = HOST_TO_PROTO16(pComm->m_SeqId);
    pGsspHeader->m_SeqNumber = n16;
    n16 = HOST_TO_PROTO16(pComm->m_SesId);
    pGsspHeader->m_SessionId = n16;
    pGsspHeader->m_HeaderLen = this->m_WriteHeaderLen;
    pGsspHeader->m_Type = pComm->m_Type;

    n32 = HOST_TO_PROTO32(pComm->m_DataLen);
    pGsspHeader->m_BodyLength = n32;
    n32 = HOST_TO_PROTO32(0);
    pGsspHeader->m_CheckSum = n32;

    if(pComm->m_Frag)
    {
        n32 = HOST_TO_PROTO32(pComm->m_DataId);
        pGsspHeader->m_DataId = n32;
        n32 = HOST_TO_PROTO32(pComm->m_Totalsize);
        pGsspHeader->m_TotalLength = n32;
        n32 = HOST_TO_PROTO32(pComm->m_Offset);
        pGsspHeader->m_Offset = n32;
    }

    this->m_pWriteComm = pComm;
    //DEBUG_BUFFER((this->m_WriteGSSP),this->m_WriteHeaderLen);
    //DEBUG_BUFFER(this->m_pWriteComm->m_Data,this->m_pWriteComm->m_DataLen);

    return 0;

}

int SdkServerSock::PutData(sdk_client_comm_t * pComm)
{
    /*we must free data at first */
    this->__FreeWrite();
    return this->__AddWriteComm(pComm);
}

int SdkServerSock::__AddDirectWrite(unsigned char * pData,int datalen)
{
    int ret;

    WRITE_INIT_ASSERT();

    if(datalen >= (int)sizeof(this->m_pWriteComm->m_Data))
    {
        ret = -EINVAL;
        goto fail;
    }
    this->m_pWriteComm = AllocateComm(datalen);
    if(this->m_pWriteComm == NULL)
    {
        ret = -errno ? -errno : -1;
        goto fail;
    }

    /*now we should set for the job*/
    this->m_WriteLen = 0;
    this->m_WriteInit = 1;
    this->m_WriteTotalLen = datalen;
    /*we pretend nothing to write header*/
    this->m_WriteHeaderLen = 0;
    memcpy(this->m_pWriteComm->m_Data,pData,datalen);
	this->m_pWriteComm->m_DataLen = datalen;

    return 0;

fail:
    this->__FreeWrite();
    return ret;
}

int SdkServerSock::PutDirectData(unsigned char * pData,int datalen)
{
    this->__FreeWrite();
    return this->__AddDirectWrite(pData,datalen);
}

void SdkServerSock::ClearWrite()
{
    this->__FreeWrite();
    return;
}
int SdkServerSock::Write()
{
    int ret;
    if(this->m_WriteInit == 0)
    {
        DEBUG_INFO("\n");
        return 1;
    }

    if(this->m_WriteLen == this->m_WriteTotalLen)
    {
        return 1;
    }

	ret = this->__WriteNoneBlock();
	if (ret < 0)
	{
		return ret;
	}

    this->m_WriteLen += ret;
    if(this->m_WriteLen == this->m_WriteTotalLen)
    {
        return 1;
    }

    return 0;
}


int SdkServerSock::__WriteNoneBlock()
{
    int ret;
    unsigned char *pCur=NULL;
    int writelen = 0;
    struct iovec iov[2];
    int iovidx = 0;

	//DEBUG_INFO("headerlen %d datalen %d totallen %d\n",this->m_WriteHeaderLen,this->m_pWriteComm->m_DataLen,this->m_WriteTotalLen);
	SDK_ASSERT((this->m_WriteHeaderLen + this->m_pWriteComm->m_DataLen) == this->m_WriteTotalLen);
    if(this->m_WriteLen < this->m_WriteHeaderLen)
    {
        pCur = ((unsigned char*)&(this->m_WriteGSSP))+this->m_WriteLen;
        iov[iovidx].iov_base = pCur;
        iov[iovidx].iov_len = (this->m_WriteHeaderLen - this->m_WriteLen);
        iovidx ++;
    }

    if(this->m_WriteLen < this->m_WriteHeaderLen)
    {
        iov[iovidx].iov_base = this->m_pWriteComm->m_Data;
        iov[iovidx].iov_len = this->m_pWriteComm->m_DataLen;
        iovidx ++;
    }
    else
    {
        pCur = this->m_pWriteComm->m_Data + (this->m_WriteLen - this->m_WriteHeaderLen);
        iov[iovidx].iov_base = pCur;
        iov[iovidx].iov_len = (this->m_WriteTotalLen - this->m_WriteLen);
        iovidx ++;
    }

    ret = writev(this->m_Sock,iov,iovidx);
    if(ret < 0)
    {
        if(errno != EINTR && errno != EAGAIN &&
                errno != EWOULDBLOCK)
        {
            ret = -errno ? -errno : -1;
            ERROR_INFO("[%d] write error(%d)\n",this->m_Sock,ret);
            return ret;
        }
        return writelen;
    }

    writelen += ret;

    return writelen;
}

