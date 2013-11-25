
#include <sdk_client_unix_sock.h>
#include <sdk_server_debug.h>
#include <rudp/gmi_rudp_api.h>

#define MAX_RUDP_PACKET_SIZE 0xffff

SdkClientUnixSock::SdkClientUnixSock(int sock) : m_Sock(sock)
{
    m_ReadInit = 0;
    memset(&m_ReadHeader,0,sizeof(m_ReadHeader));
    m_ReadLen = 0;
    m_ReadTotalLen  = 0;
    m_ReadHeaderLen = 0;
    m_pReadComm = NULL;


    m_WriteInit = 0;
    memset(&m_WriteHeader,0,sizeof(m_WriteHeader));
    m_WriteLen = 0;
    m_WriteTotalLen = 0;
    m_WriteHeaderLen = 0;
    m_pWriteComm = NULL;
}

void SdkClientUnixSock::ClearRead()
{
    /*now first to free comm*/
    FreeComm(this->m_pReadComm);
    memset(&(this->m_ReadHeader),0,sizeof(this->m_ReadHeader));
    this->m_ReadLen = 0;
    this->m_ReadTotalLen = 0;
    this->m_ReadHeaderLen = 0;
    this->m_ReadInit = 0;
    return ;
}

void SdkClientUnixSock::ClearWrite()
{
    FreeComm(this->m_pWriteComm);
    memset(&(this->m_WriteHeader),0,sizeof(this->m_WriteHeader));
    this->m_WriteLen = 0;
    this->m_WriteTotalLen = 0;
    this->m_WriteHeaderLen = 0;
    this->m_WriteInit = 0;
    return ;
}

SdkClientUnixSock::~SdkClientUnixSock()
{
    this->ClearRead();
    this->ClearWrite();
    if(this->m_Sock >= 0)
    {
        close(this->m_Sock);
    }
    this->m_Sock = -1;
}

int SdkClientUnixSock::__ReadNoneBlock()
{
    int ret;
    unsigned char *pData=NULL;
    int curlen,readlen = 0;
    if((this->m_ReadLen) < this->m_ReadHeaderLen)
    {
		SDK_ASSERT(this->m_pReadComm == NULL);
        pData = ((unsigned char*)&(this->m_ReadHeader)) + this->m_ReadLen;
        curlen = (this->m_ReadHeaderLen - this->m_ReadLen);
    }
    else
    {
		SDK_ASSERT(this->m_pReadComm);
        pData = this->m_pReadComm->m_Data + (this->m_ReadLen - this->m_ReadHeaderLen);
        curlen = (this->m_ReadTotalLen - this->m_ReadLen);
    }


    errno = 0;
    if(curlen > 0)
    {
        ret = read(this->m_Sock,pData,curlen);
        if(ret < 0)
        {
            ret = -errno ? -errno : -1;
            if(errno != EWOULDBLOCK &&
                    errno != EAGAIN &&
                    errno != EINTR)
            {
                return ret;
            }
            DEBUG_INFO("ret %d\n",ret);
            return readlen;
        }
        else if(ret == 0)
        {
			ERROR_INFO("read(0)\n");
            return -EPIPE;
        }


        /*now test if the */
        curlen = ret;
        readlen += curlen;
    }
    return readlen;

}



int SdkClientUnixSock::__ReadInit()
{
    SDK_ASSERT(this->m_pReadComm == NULL);

    /*now fill the read */
    this->m_ReadLen = 0;
    this->m_ReadHeaderLen = sizeof(this->m_ReadHeader);
    this->m_ReadTotalLen = this->m_ReadHeaderLen;
    this->m_ReadInit = 1;
    return 0;
}

int SdkClientUnixSock::__ReadFillHeader()
{
    int ret;
    uint16_t h16;
    SDK_ASSERT(this->m_ReadInit);
    SDK_ASSERT(this->m_ReadLen == this->m_ReadHeaderLen);
    SDK_ASSERT(this->m_pReadComm == NULL);

    /*now check for the header */
    if(this->m_ReadHeader.s_MsgTag[0] != 'G' ||
            this->m_ReadHeader.s_MsgTag[1] != 'R' ||
            this->m_ReadHeader.s_MsgTag[2] != 'M' ||
            this->m_ReadHeader.s_MsgTag[3] != 'T')
    {
        errno = EINVAL;
        ret = -errno;
        return ret;
    }

    h16 = INNER_HOST_TO_PROTO16(this->m_ReadHeader.s_BodyLen);
    if(h16 > MAX_RUDP_PACKET_SIZE)
    {
        errno = EINVAL;
        ret = -errno ;
        return ret;
    }

    this->m_pReadComm = AllocateComm(h16);
    if(this->m_pReadComm == NULL)
    {
        ret = -errno ? -errno : -1;
        return ret;
    }
    this->m_pReadComm->m_DataLen = h16;
    this->m_ReadTotalLen = this->m_ReadHeaderLen + h16;
    return 0;
}

int SdkClientUnixSock::__ReadFillComm()
{
    int ret;
    uint32_t h32;
    uint16_t h16;

    SDK_ASSERT(this->m_ReadInit);
    SDK_ASSERT(this->m_pReadComm);
    SDK_ASSERT(this->m_ReadLen == this->m_ReadTotalLen);

    /*now to test if the header partial*/

    /*now to put session id*/
    h16 = INNER_HOST_TO_PROTO16(this->m_ReadHeader.s_SessionId);
    this->m_pReadComm->m_SesId = h16;
    h32 = INNER_HOST_TO_PROTO32(this->m_ReadHeader.s_AuthValue);
    this->m_pReadComm->m_Priv = h32;
    h16 = INNER_HOST_TO_PROTO16(this->m_ReadHeader.s_SvrPort);
    this->m_pReadComm->m_ServerPort = h16;
    this->m_pReadComm->m_LocalPort = 0;
    h16 = INNER_HOST_TO_PROTO16(this->m_ReadHeader.s_SeqNum);
    this->m_pReadComm->m_SeqId = h16;
    this->m_pReadComm->m_Type = GMIS_PROTOCOL_TYPE_SDK_TO_SYS;
    this->m_pReadComm->m_FHB = 0;
    if(this->m_ReadHeader.s_MessageType & RUDP_MESSAGE_TYPE_PARTIAL)
    {
        this->m_pReadComm->m_Frag = 1;
        this->m_pReadComm->m_DataId = this->m_ReadHeader.s_DataId;
        h16 = INNER_HOST_TO_PROTO16(this->m_ReadHeader.s_MsgOffsetofPkt);
        this->m_pReadComm->m_Offset = h16;
        h16 = INNER_HOST_TO_PROTO16(this->m_ReadHeader.s_PktTotalLength);
        this->m_pReadComm->m_Totalsize = h16;

        /*now check for the validation*/
        if(this->m_pReadComm->m_Offset >= this->m_pReadComm->m_Totalsize)
        {
            errno = EINVAL;
            ret = -errno;
			ERROR_INFO("read frag offset %d > totalsize %d\n",
				this->m_pReadComm->m_Offset,
				this->m_pReadComm->m_Totalsize);
            return ret;
        }

        if((this->m_pReadComm->m_Offset + this->m_pReadComm->m_DataLen) >
                this->m_pReadComm->m_Totalsize)
        {
            errno = EINVAL;
            ret = -errno;
			ERROR_INFO("read offset %d + datalen %d > totalsize %d\n",
				this->m_pReadComm->m_Offset,
				this->m_pReadComm->m_DataLen,
				this->m_pReadComm->m_Totalsize);
            return ret;
        }
    }
    else
    {
        this->m_pReadComm->m_Frag = 0;
        this->m_pReadComm->m_DataId = 0;
        this->m_pReadComm->m_Offset = 0;
        this->m_pReadComm->m_Totalsize = 0;
    }
    h16 = INNER_HOST_TO_PROTO16(this->m_ReadHeader.s_BodyLen);
    SDK_ASSERT(this->m_ReadTotalLen == (this->m_ReadHeaderLen + h16));
    SDK_ASSERT(this->m_pReadComm->m_DataLen == h16);

	return 0;
}

int SdkClientUnixSock::Read()
{
    int ret;
    if(this->m_ReadInit &&
            (this->m_ReadLen == this->m_ReadTotalLen))
    {
        return 1;
    }

    if(this->m_ReadInit == 0)
    {
        ret = this->__ReadInit();
        if(ret < 0)
        {
            this->ClearRead();
            return ret;
        }
    }

    if(this->m_ReadLen < this->m_ReadHeaderLen)
    {
        ret = this->__ReadNoneBlock();
        if(ret < 0)
        {
            this->ClearRead();
            return ret;
        }
        this->m_ReadLen += ret;
        if(this->m_ReadLen < this->m_ReadHeaderLen)
        {
            /*not read over ,so return */
            return 0;
        }

        ret = this->__ReadFillHeader();
        if(ret < 0)
        {
            this->ClearRead();
            return ret;
        }
    }


    ret = this->__ReadNoneBlock();
    if(ret < 0)
    {
        this->ClearRead();
        return ret;
    }


    this->m_ReadLen += ret;
    if(this->m_ReadLen  < this->m_ReadTotalLen)
    {
        return 0;
    }

    ret = this->__ReadFillComm();
    if(ret < 0)
    {
        this->ClearRead();
        return ret;
    }
    return 1;
}



sdk_client_comm_t* SdkClientUnixSock::GetRead()
{
    sdk_client_comm_t* pComm=this->m_pReadComm;
    if(this->m_ReadInit== 0 ||
            (this->m_ReadLen < this->m_ReadTotalLen))
    {
        return NULL;
    }

    /*mask pComm and not free it*/
    this->m_pReadComm = NULL;
    this->ClearRead();
    return pComm;
}


int SdkClientUnixSock::__WriteNoneBlock()
{
    int ret;
    struct iovec iov[2];
    int iovlen = 2;
    int iovidx = 0;
    int writelen = 0;
    int leftlen = this->m_WriteTotalLen - this->m_WriteLen;
    sdk_client_comm_t *pComm=this->m_pWriteComm;
    unsigned char* pData;

    while(leftlen > 0)
    {
        iovidx = 0;
        if((this->m_WriteLen + writelen)< this->m_WriteHeaderLen)
        {
            pData = (unsigned char*) &(this->m_WriteHeader);
            pData += (this->m_WriteLen + writelen);
            iov[iovidx].iov_base = pData;
            iov[iovidx].iov_len = this->m_WriteHeaderLen - this->m_WriteLen - writelen;
            iovidx += 1;
        }

        if((this->m_WriteLen + writelen) > this->m_WriteHeaderLen)
        {
            pData = (unsigned char*)(pComm->m_Data);
            pData += ((this->m_WriteLen + writelen) - this->m_WriteHeaderLen);
            iov[iovidx].iov_base = pData;
            iov[iovidx].iov_len = (this->m_WriteTotalLen - this->m_WriteLen - writelen);
            iovidx += 1;
        }
        else
        {
            pData = (unsigned char*)(pComm->m_Data);
            iov[iovidx].iov_base = pData;
            iov[iovidx].iov_len = pComm->m_DataLen;
            iovidx += 1;
        }

        iovlen = iovidx;
        errno = 0;
        ret = writev(this->m_Sock,iov,iovlen);
        if(ret < 0)
        {
            ret = -errno ? -errno : -1;
            if(errno != EWOULDBLOCK &&
                    errno != EAGAIN &&
                    errno != EINTR)
            {
                return ret;
            }
            return writelen;
        }
        leftlen -= ret;
        writelen += ret;
    }
    return writelen;
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


int SdkClientUnixSock::__AddWriteComm(sdk_client_comm_t * pComm)
{
    uint32_t n32;
    uint16_t n16;
    int ret;

    WRITE_INIT_ASSERT();

    if(pComm->m_DataLen > MAX_RUDP_PACKET_SIZE ||
            pComm->m_Totalsize > MAX_RUDP_PACKET_SIZE)
    {
        ret = -EINVAL;
        errno = EINVAL;
        return ret;
    }

    this->m_WriteInit = 1;
    this->m_WriteLen = 0;
    this->m_WriteHeaderLen = sizeof(this->m_WriteHeader);
    this->m_WriteTotalLen = this->m_WriteHeaderLen + pComm->m_DataLen;


    this->m_WriteHeader.s_MsgTag[0] = 'G';
    this->m_WriteHeader.s_MsgTag[1] = 'R';
    this->m_WriteHeader.s_MsgTag[2] = 'M';
    this->m_WriteHeader.s_MsgTag[3] = 'T';

    n16 = INNER_HOST_TO_PROTO16(RUDP_VERSION);
    this->m_WriteHeader.s_Version = n16;
    n16 = INNER_HOST_TO_PROTO16(pComm->m_SeqId);
    this->m_WriteHeader.s_SeqNum =  n16;
    n16 = INNER_HOST_TO_PROTO16(pComm->m_SesId);
    this->m_WriteHeader.s_SessionId = n16;
    n32 = INNER_HOST_TO_PROTO32(pComm->m_Priv);
    this->m_WriteHeader.s_AuthValue = n32;
    n16 = INNER_HOST_TO_PROTO16(pComm->m_DataLen);
    this->m_WriteHeader.s_BodyLen = n16;
    n16 = INNER_HOST_TO_PROTO16(pComm->m_LocalPort);
    this->m_WriteHeader.s_SvrPort = n16;

    this->m_WriteHeader.s_MessageType  =0;


    this->m_WriteHeader.s_MessageType = RUDP_MESSAGE_TYPE_COMPLETE;
    if(pComm->m_Frag)
    {
        this->m_WriteHeader.s_MessageType |= RUDP_MESSAGE_TYPE_PARTIAL;
    }

    n16 = INNER_HOST_TO_PROTO16(pComm->m_Totalsize);
    this->m_WriteHeader.s_PktTotalLength = n16;
    n16 = INNER_HOST_TO_PROTO16(pComm->m_Offset);
    this->m_WriteHeader.s_MsgOffsetofPkt = n16;

    n32 = INNER_HOST_TO_PROTO32(0);
    this->m_WriteHeader.s_CheckSum = n32;
    n32 = INNER_HOST_TO_PROTO32(pComm->m_DataId);
    this->m_WriteHeader.s_DataId = n32;
    this->m_pWriteComm = pComm;

    //DEBUG_BUFFER((unsigned char*)&(this->m_WriteHeader),this->m_WriteHeaderLen);
    //DEBUG_BUFFER(pComm->m_Data,pComm->m_DataLen);

    return 0;
}


int SdkClientUnixSock::PushData(sdk_client_comm_t * pComm)
{
    /*we must free data at first */
    this->ClearWrite();
    return this->__AddWriteComm(pComm);
}


int SdkClientUnixSock::Socket()
{
    return this->m_Sock;
}


int SdkClientUnixSock::Write()
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

    if(this->m_WriteLen < this->m_WriteTotalLen)
    {
        ret = this->__WriteNoneBlock();
        if(ret < 0)
        {
            return ret;
        }
        this->m_WriteLen += ret;
    }
    if(this->m_WriteLen == this->m_WriteTotalLen)
    {
        return 1;
    }

    return 0;
}


