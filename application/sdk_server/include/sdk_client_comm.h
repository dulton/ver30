
#ifndef __SDK_CLIENT_COMM_H__
#define __SDK_CLIENT_COMM_H__


#include <osal/gmi_system_headers.h>
#include <sys_env_types.h>
#include <user_auth_api.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t sessionid_t;
typedef uint32_t privledge_t;

typedef uint8_t typeid_t;
typedef uint16_t seqid_t;

#define  MAX_CLIENT_COMM_SIZE      0x40000
#define  CLIENT_COMM_PACK_SIZE     1400

typedef struct
{
    sessionid_t m_SesId;
	privledge_t m_Priv;
	int m_ServerPort;
	int m_LocalPort;
    seqid_t m_SeqId;
    int m_Type;
	int m_FHB;   /*heart beat flag*/
    int m_Frag;
    int m_DataId;
    int m_Offset;
    int m_Totalsize;
    unsigned int m_DataLen;
    uint8_t m_Data[MAX_CLIENT_COMM_SIZE];
} sdk_client_comm_t;


#define  PROTO_TO_HOST32    ntohl
#define  HOST_TO_PROTO32    htonl

#define  PROTO_TO_HOST16    ntohs
#define  HOST_TO_PROTO16    htons

static __inline__ uint64_t HOST_TO_PROTO64(uint64_t val)
{
	uint64_t rval=0;
	uint32_t lval = val&0xffffffff,hval = (val >> 32) & 0xffffffff;
	lval = HOST_TO_PROTO32(lval);
	hval = HOST_TO_PROTO32(hval);
	rval = hval;
	rval |= ((uint64_t)lval << 32);
	return rval;
}

static __inline__ uint64_t PROTO_TO_HOST64(uint64_t val)
{
	uint64_t rval=0;
	uint32_t lval = val&0xffffffff,hval = (val >> 32) & 0xffffffff;
	lval = PROTO_TO_HOST32(lval);
	hval = PROTO_TO_HOST32(hval);
	rval = hval;
	rval |= ((uint64_t)lval << 32);
	return rval;
}

#define  INNER_PROTO_TO_HOST32(val)    (val)
#define  INNER_HOST_TO_PROTO32(val)    (val)

#define  INNER_PROTO_TO_HOST16(val)    (val)
#define  INNER_HOST_TO_PROTO16(val)    (val)

static __inline__ uint64_t INNER_HOST_TO_PROTO64(uint64_t val)
{
	return val;
}

static __inline__ uint64_t INNER_PROTO_TO_HOST64(uint64_t val)
{
	return val;
}


#define  GMIS_PROTOCOL_TYPE_CONF        0x01
#define  GMIS_PROTOCOL_TYPE_LOG         0x11
#define  GMIS_PROTOCOL_TYPE_WARNING     0x12
#define  GMIS_PROTOCOL_TYPE_MEDIA_CTRL  0x21
#define  GMIS_PROTOCOL_TYPE_MEDIA_DATA  0x22
#define  GMIS_PROTOCOL_TYPE_UPGRADE     0x31

#define  GMIS_PROTOCOL_TYPE_SDK_TO_SYS  0x81

#define  GMIS_PROTOCOL_TYPE_LOGGIN      0x80


#define  GMIS_ERROR_LOGIN_FAILED        0x11

#define  GSSP_HEADER_BASE_LEN           20
#define  GSSP_HEADER_MAX_LEN            32

#define  GSSP_HEADER_FLAG_FPP           (1 << 0)
#define  GSSP_HEADER_FLAG_FCV           (1 << 1)
#define  GSSP_HEADER_FLAG_FHB           (1 << 2)
#define  GSSP_HEADER_FLAG_ENC_DEFAULT   (0 << 5)

#define  GSSP_HEADER_VERSION_MAJOR      (1 )
#define  GSSP_HEADER_VERSION_MINOR      (0 << 4)

#define  SDK_SERVER_PRIVATE_SESID       ID_SESSIONID_INTER_SDK


typedef struct
{
    uint8_t m_Magic[4];
    uint8_t m_Flag;
    uint8_t m_Version;
    uint16_t m_SeqNumber;
    uint16_t m_SessionId;
    uint8_t m_HeaderLen;
    uint8_t m_Type;
    uint32_t m_BodyLength;
    uint32_t m_CheckSum;
    uint32_t m_DataId;
    uint32_t m_TotalLength;
    uint32_t m_Offset;
} __attribute__((packed)) gssp_header_t;

sdk_client_comm_t* AllocateComm(int datasize);
void FreeComm(sdk_client_comm_t*& pComm);


#ifdef __cplusplus
};
#endif

#endif /*__SDK_CLIENT_COMM_H__*/

