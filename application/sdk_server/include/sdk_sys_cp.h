
#ifndef  __SDK_SYS_CP_H__
#define  __SDK_SYS_CP_H__

#include <sys_env_types.h>
#include <sdk_client_comm.h>


class SdkSysCp
{
public:
    SdkSysCp();
    ~SdkSysCp();
	int ParsePkg(sdk_client_comm_t* pComm);
	SysPkgHeader* GetPkgHeader(int& pkgsize);
	sdk_client_comm_t* GetClientComm();
	int SetCode(uint32_t code);
	int SetSessionSeq(uint32_t sesid,uint32_t seqid);
	int SetAttrCount(uint32_t attrcount);
	int SetData(void* pData,int datalen);
	int GetCode(uint32_t& code);
	int GetSessionSeq(uint32_t& sesid,uint32_t& seqid);
	int GetAttrCount(uint32_t& attrcount);
	int GetData(void*& pData ,uint32_t& datasize);
private:
    SysPkgHeader *m_pPkgHeader;
	int m_PkgSize;
	void *m_pData;
	uint32_t m_DataSize;
	uint32_t m_Code;
	uint32_t m_AttrCount;
	uint32_t m_SessionId;
	uint32_t m_SeqId;	
	int m_PkgInit;
	int m_DataInit;
	int m_CodeInit;
	int m_AttrCountInit;
	int m_SessionInit;
	int m_SeqIdInit;
};

#endif /*__SDK_SYS_CP_H__*/

