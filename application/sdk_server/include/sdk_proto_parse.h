#ifndef __SDK_PROTO_PARSE_H__
#define __SDK_PROTO_PARSE_H__

#include <sdk_client_comm.h>

class SdkProtoParse
{
public:
	SdkProtoParse();
	~SdkProtoParse();
	int Parse(void* pData,int datalen);
	int GetAttrCount();
	int GetCode(int& code);
	int GetSessionSeq(sessionid_t& sesid,seqid_t& seqid);
	int GetAttr(int idx,int& attr,void*& pData,int& datalen);
private:
	void __Reset();
private:
	int m_Parsed;
	void* m_pData;
	int m_DataLen;
	int m_Code;
	sessionid_t m_SesId;
	seqid_t m_SeqId;
	std::vector<int> m_AttrOffsetVecs;
	std::vector<int> m_AttrVecs;
	std::vector<int> m_AttrLenVecs;
};

#endif /*__SDK_PROTO_PARSE_H__*/