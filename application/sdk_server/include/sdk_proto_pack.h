#ifndef __SDK_PROTO_PACK_H__
#define __SDK_PROTO_PACK_H__

#include <sdk_client_comm.h>

class SdkProtoPack
{
public:
	SdkProtoPack(int code,sessionid_t sesid,seqid_t seqid);
	~SdkProtoPack();
	int GetLength();
	void* GetPtr();
	int AddItem(int itemcode,void* pData,int datalen);
	void Reset(int code=0,sessionid_t sesid=0,seqid_t seqid=0);
private:
	void __ClearItemCode();
	int __FormatCode();
private:
	int m_Code;
	sessionid_t m_SesId;
	seqid_t m_SeqId;
	void* m_pFormat;
	int m_FormatLen;
	int m_Formated;
	std::vector<void*> m_pItemData;
	std::vector<int> m_ItemCode;
	std::vector<int> m_ItemLength;
};

#endif /*__SDK_PROTO_PACK_H__*/

