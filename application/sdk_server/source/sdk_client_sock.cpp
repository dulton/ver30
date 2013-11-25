
#include <sdk_client_sock.h>
#include <sdk_server_debug.h>
#include <rudp/gmi_rudp_api.h>
#include <gmi_rudp.h>

static const char* st_GrmtHeader= "GRMT";

SdkClientSock::SdkClientSock(int sock) : m_Sock(sock)
{
    m_ReadInit = 0;
    memset((&m_ReadHeader),0,sizeof(m_ReadHeader));
    m_ReadLen = 0;
    m_ReadTotalLen = 0;
    m_ReadHeaderLen = 0;
    m_pReadComm = NULL;
    memset(&m_RcvAddr,0,sizeof(m_RcvAddr));
    m_HasRcvAddr = 0;

    m_WriteInit = 0;
    memset(&m_WriteHeader,0,sizeof(m_WriteHeader));
    m_WriteLen = 0;
    m_WriteTotalLen = 0;
    m_WriteHeaderLen = 0;
    m_pWriteComm = NULL;
}

int SdkClientSock::IsWriteSth()
{
    if (this->m_WriteInit == 0)
    {
        return 0;
    }
    if (this->m_WriteLen == this->m_WriteTotalLen)
    {
        return 0;
    }

    return 1;
}

void SdkClientSock::__FreeRead()
{
    this->m_ReadInit = 0;
    memset(&(this->m_ReadHeader), 0 ,sizeof(this->m_ReadHeader));
    this->m_ReadLen = 0;
    this->m_ReadTotalLen = 0;
    this->m_ReadHeaderLen = 0;
    memset(&(this->m_RcvAddr),0,sizeof(this->m_RcvAddr));
    this->m_HasRcvAddr = 0;

    if (this->m_pReadComm )
    {
    	DEBUG_INFO("free comm %p\n",this->m_pReadComm);
        free(this->m_pReadComm);
    }
    this->m_pReadComm = NULL;
    return ;
}

void SdkClientSock::__FreeWrite()
{
    this->m_WriteInit = 0;
    memset(&(this->m_WriteHeader), 0 ,sizeof(this->m_WriteHeader));
    this->m_WriteLen = 0;
    this->m_WriteTotalLen = 0;
    this->m_WriteHeaderLen = 0;

    if (this->m_pWriteComm )
    {
        free(this->m_pWriteComm);
    }
    this->m_pWriteComm = NULL;
    return ;
}

SdkClientSock::~SdkClientSock()
{
    this->__FreeRead();
    this->__FreeWrite();
    if (this->m_Sock >= 0)
    {
        close(this->m_Sock);
    }
    this->m_Sock = -1;
}


#define   READ_INIT_ASSERT() \
do\
{\
	SDK_ASSERT(this->m_ReadInit == 0);\
	SDK_ASSERT(this->m_ReadLen == 0);\
	SDK_ASSERT(this->m_ReadTotalLen == 0);\
	SDK_ASSERT(this->m_ReadHeaderLen == 0);\
	SDK_ASSERT(this->m_pReadComm == NULL);\
	SDK_ASSERT(this->m_HasRcvAddr == 0);\
}while(0)

int SdkClientSock::__ReadInit()
{
    //READ_INIT_ASSERT();
    this->m_ReadInit = 1;
    this->m_ReadHeaderLen = sizeof(this->m_ReadHeader);
    memset(&(this->m_ReadHeader),0,sizeof(this->m_ReadHeader));
    this->m_ReadTotalLen = this->m_ReadHeaderLen + sizeof(this->m_pReadComm->m_Data);
    memset(&(this->m_RcvAddr),0,sizeof(this->m_RcvAddr));
    this->m_HasRcvAddr = 0;
    this->m_ReadLen = 0;
    if (this->m_pReadComm == NULL)
    {
        this->m_pReadComm = (sdk_client_comm_t*)calloc(sizeof(*(this->m_pReadComm)),1);
        if(this->m_pReadComm == NULL)
        {
            return -ENOMEM;
        }
    }

    return 0;
}




int SdkClientSock::__AdjustTotalLen()
{
    uint16_t h16;
    SDK_ASSERT(this->m_ReadHeaderLen <= this->m_ReadLen);
    h16 = INNER_PROTO_TO_HOST16(this->m_ReadHeader.s_BodyLen);
    if (h16 > sizeof(this->m_pReadComm->m_Data))
    {
        return -ENOSPC;
    }
    this->m_ReadTotalLen = this->m_ReadHeaderLen + h16;

    return 0;
}

int SdkClientSock::__ReadNoneBlock()
{
    int ret,res;
    unsigned char *pData=NULL;
    int maxlen = this->m_ReadTotalLen - this->m_ReadLen;
    int leftlen = maxlen;
    int readlen = 0,curlen;
    uint16_t h16;
    struct sockaddr_in saddr;
    struct msghdr mhdr;
    struct iovec iovec[2];
    int iovlen,iovidx;
    SDK_ASSERT(this->m_pReadComm);
    while(leftlen > 0)
    {

        mhdr.msg_name = (void*) &saddr;
        mhdr.msg_namelen = sizeof(saddr);
        mhdr.msg_control = NULL;
        mhdr.msg_controllen = 0;
        iovidx = 0;
        if ((this->m_ReadLen + readlen) < this->m_ReadHeaderLen)
        {
            pData = (unsigned char*)&(this->m_ReadHeader);
            pData += (this->m_ReadLen + readlen);
            iovec[iovidx].iov_base = pData;
            iovec[iovidx].iov_len = (this->m_ReadHeaderLen - this->m_ReadLen - readlen);
            iovidx += 1;
        }

        if ((this->m_ReadLen + readlen) <= this->m_ReadHeaderLen)
        {
            iovec[iovidx].iov_base = this->m_pReadComm->m_Data;
            iovec[iovidx].iov_len = this->m_ReadTotalLen - this->m_ReadHeaderLen;
            iovidx += 1;
        }
        else
        {
            pData = this->m_pReadComm->m_Data;
            pData += (this->m_ReadLen + readlen) - this->m_ReadHeaderLen;
            iovec[iovidx].iov_base = pData;
            iovec[iovidx].iov_len = this->m_ReadTotalLen - (this->m_ReadLen + readlen) ;
            iovidx += 1;
        }
        iovlen = iovidx;

        mhdr.msg_iov = iovec;
        mhdr.msg_iovlen = iovlen;
        mhdr.msg_flags = 0;
        errno = 0;
        memset(&saddr,0,sizeof(saddr));
        ret = recvmsg(this->m_Sock,&mhdr,MSG_DONTWAIT);
        if (ret < 0)
        {
            ret = -errno ? -errno : -1;
            if (errno != EWOULDBLOCK &&
                    errno != EAGAIN &&
                    errno != EINTR)
            {
                return ret;
            }
            DEBUG_INFO("ret %d\n",ret);
            return readlen;
        }
        else if (ret == 0)
        {
            return -EPIPE;
        }

        /*now test if the */
        curlen = ret;
        if ((this->m_ReadLen + readlen)< strlen(st_GrmtHeader) &&
                (this->m_ReadLen + readlen + curlen) >= strlen(st_GrmtHeader))
        {
            ret = this->__MakeSureHeaderRight((this->m_ReadLen + readlen + curlen));
            if (ret == 0)
            {
                /*we did not read any header of the buffer ,so just discard all readin and make a fresh start*/
                res = this->__ReadInit();
                if (res < 0)
                {
                    return res;
                }
                return 0;
            }
        }

        if ((this->m_ReadLen + readlen) < this->m_ReadHeaderLen &&
                (this->m_ReadLen + readlen + curlen) >= this->m_ReadHeaderLen)
        {
            /*now we should read for the total len*/
            h16 = INNER_PROTO_TO_HOST16(this->m_ReadHeader.s_BodyLen);
            if (h16 > sizeof(this->m_pReadComm->m_Data))
            {
                this->__ReadInit();
                return -EINVAL;
            }
            this->m_ReadTotalLen = h16 + this->m_ReadHeaderLen;
            maxlen = this->m_ReadTotalLen - this->m_ReadLen;
            leftlen = maxlen - readlen;

            if ((leftlen - curlen) < 0)
            {
                this->__ReadInit();
                return -EINVAL;
            }
        }



        if (this->m_HasRcvAddr == 0)
        {
            memcpy(&(this->m_RcvAddr),mhdr.msg_name,mhdr.msg_namelen);
            this->m_HasRcvAddr = mhdr.msg_namelen;
        }
        else
        {
            /*to make sure we receive from the same source*/
            if (mhdr.msg_namelen != this->m_HasRcvAddr)
            {
                return -EPIPE;
            }

            if (memcmp(&(this->m_RcvAddr),mhdr.msg_name,mhdr.msg_namelen))
            {
                return -EPIPE;
            }
        }
        leftlen -= curlen;
        readlen += curlen;
    }
    return readlen;

}

int SdkClientSock::__ReadFillComm()
{
    sdk_client_comm_t* pComm = this->m_pReadComm;
    uint16_t h16;
    uint32_t h32;


    SDK_ASSERT(this->m_ReadLen == this->m_ReadTotalLen);
    SDK_ASSERT(pComm);

	if (this->m_HasRcvAddr == 0)
	{
		ERROR_INFO("not fill addr\n");
		return -EINVAL;
	}

    h16 = INNER_PROTO_TO_HOST16(this->m_ReadHeader.s_SessionId);
    pComm->m_SesId = h16;

    h16 = INNER_PROTO_TO_HOST16(this->m_ReadHeader.s_SeqNum);
    pComm->m_SeqId = h16;
    h32 = INNER_PROTO_TO_HOST32(this->m_ReadHeader.s_AuthValue);
    pComm->m_Priv = h32;

    h16 = INNER_PROTO_TO_HOST16(this->m_ReadHeader.s_SvrPort);
	if (h16 == 0)
	{
		/*to change into the port ok*/
		h16 = ntohs(this->m_RcvAddr.sin_port);
	}
    pComm->m_ServerPort = h16;

    pComm->m_Frag = 0;
    pComm->m_Offset = 0;
    pComm->m_Totalsize = 0;
	pComm->m_DataId = 0;
    if (this->m_ReadHeader.s_MessageType & RUDP_MESSAGE_TYPE_PARTIAL)
    {
        pComm->m_Frag = 1;
        h16 = INNER_PROTO_TO_HOST16(this->m_ReadHeader.s_PktTotalLength);
        pComm->m_Totalsize = h16;
        h16 = INNER_PROTO_TO_HOST16(this->m_ReadHeader.s_MsgOffsetofPkt);
        pComm->m_Offset = h16;
		h32 = INNER_PROTO_TO_HOST32(this->m_ReadHeader.s_DataId);
		pComm->m_DataId = h32;
    }
	pComm->m_DataLen = this->m_ReadTotalLen - this->m_ReadHeaderLen;
	//DEBUG_BUFFER((unsigned char*)&(this->m_ReadHeader),this->m_ReadHeaderLen);
	//DEBUG_BUFFER(pComm->m_Data,pComm->m_DataLen);
    return 0;

}

int SdkClientSock::__MakeSureHeaderRight(int totallen)
{
    unsigned char* pHeader=(unsigned char*)&(this->m_ReadHeader);
    if (totallen >= (int)strlen(st_GrmtHeader))
    {
        if (memcmp(pHeader,st_GrmtHeader,strlen(st_GrmtHeader))==0)
        {
            /*we find the header ,so this is ok*/
            return 1;
        }
    }
    return 0;
}


int SdkClientSock::Read()
{
    int ret;
    if (this->m_ReadInit &&
            (this->m_ReadLen == this->m_ReadTotalLen))
    {
        return 1;
    }

    if (this->m_ReadInit == 0)
    {
        ret = this->__ReadInit();
        if (ret < 0)
        {
            return ret;
        }
    }

    /*we put here ,because it will return error ,to come here */
    if (this->m_pReadComm == NULL)
    {
        ret = this->__ReadInit();
        if (ret < 0)
        {
            return ret;
        }
    }

    ret = this->__ReadNoneBlock();
    if (ret < 0)
    {
        this->__ReadInit();
        return ret;
    }


    this->m_ReadLen += ret;
    if (this->m_ReadLen  < this->m_ReadTotalLen)
    {
        return 0;
    }

    ret = this->__ReadFillComm();
    if (ret < 0)
    {
        return ret;
    }
    return 1;
}



sdk_client_comm_t* SdkClientSock::GetRead()
{
    sdk_client_comm_t* pComm=this->m_pReadComm;
    if (this->m_ReadInit== 0 ||
            (this->m_ReadLen < this->m_ReadTotalLen))
    {
        return NULL;
    }

    /*we do not make sure this is ok*/
    this->m_pReadComm = NULL;
    this->__FreeRead();
    return pComm;
}

void SdkClientSock::ClearRead()
{
    this->__FreeRead();
    return;
}


int SdkClientSock::Write()
{
    int ret;
    if (this->m_WriteInit == 0)
    {
        DEBUG_INFO("\n");
        return 1;
    }

    if (this->m_WriteLen == this->m_WriteTotalLen)
    {
        return 1;
    }

    if (this->m_WriteLen < this->m_WriteTotalLen)
    {
        ret = this->__WriteNoneBlock();
        if (ret < 0)
        {
            return ret;
        }
        this->m_WriteLen += ret;
    }
    if (this->m_WriteLen == this->m_WriteTotalLen)
    {
        return 1;
    }

    return 0;
}


int SdkClientSock::__WriteNoneBlock()
{
    int ret;
    struct iovec iov[2];
    int iovlen = 2;
    int iovidx = 0;
    struct msghdr mhdr;
    int writelen = 0;
    int leftlen = this->m_WriteTotalLen - this->m_WriteLen;
    sdk_client_comm_t *pComm=this->m_pWriteComm;
    struct sockaddr_in saddr;
    unsigned char* pData;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(this->m_pWriteComm->m_ServerPort);
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    while(leftlen > 0)
    {
        iovidx = 0;
        if ((this->m_WriteLen + writelen)< this->m_WriteHeaderLen)
        {
            pData = (unsigned char*) &(this->m_WriteHeader);
            pData += (this->m_WriteLen + writelen);
            iov[iovidx].iov_base = pData;
            iov[iovidx].iov_len = this->m_WriteHeaderLen - this->m_WriteLen - writelen;
            iovidx += 1;
        }

        if ((this->m_WriteLen + writelen) > this->m_WriteHeaderLen)
        {
            pData = (unsigned char*) (pComm->m_Data);
            pData += (this->m_WriteLen + writelen) - this->m_WriteHeaderLen;
            iov[iovidx].iov_base = pData;
            iov[iovidx].iov_len = (this->m_WriteTotalLen - this->m_WriteLen - writelen);
            iovidx += 1;
        }
        else
        {
            pData = (unsigned char*) (pComm->m_Data);
            iov[iovidx].iov_base = pData;
            iov[iovidx].iov_len = pComm->m_DataLen;
            iovidx += 1;
        }

        iovlen = iovidx;
        mhdr.msg_name = (void*)&saddr;
        mhdr.msg_namelen = sizeof(saddr);
        mhdr.msg_iov = iov;
        mhdr.msg_iovlen = iovlen;
        mhdr.msg_control = NULL;
        mhdr.msg_controllen = 0;
        mhdr.msg_flags = 0;
        errno = 0;
        ret = sendmsg(this->m_Sock,&mhdr,MSG_DONTWAIT);
        if (ret < 0)
        {
            ret = -errno ? -errno : -1;
            if (errno != EWOULDBLOCK &&
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


int SdkClientSock::__AddWriteComm(sdk_client_comm_t * pComm)
{
    uint32_t n32;
    uint16_t n16;

    WRITE_INIT_ASSERT();

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
    if (pComm->m_Frag)
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


int SdkClientSock::PushData(sdk_client_comm_t * pComm)
{
    /*we must free data at first */
    this->__FreeWrite();
    return this->__AddWriteComm(pComm);
}

void SdkClientSock::ClearWrite()
{
    this->__FreeWrite();
    return;
}

int SdkClientSock::Socket()
{
    return this->m_Sock;
}

int SdkClientSock::GetOverloadSize()
{
	return sizeof(this->m_ReadHeader);
}

