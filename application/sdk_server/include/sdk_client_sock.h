

#ifndef __SDK_CLIENT_SOCK_H__
#define __SDK_CLIENT_SOCK_H__


#include <osal/gmi_system_headers.h>
#include <gmi_rudp.h>
#include <sdk_client_comm.h>


class SdkClientSock
{
public:
	SdkClientSock(int sock);
	~SdkClientSock();
	int Read();
	sdk_client_comm_t* GetRead();
	int IsWriteSth();
	int Write();
	int PushData(sdk_client_comm_t* pComm);
	void ClearRead();
	void ClearWrite();
	int Socket();
	int GetOverloadSize();
private:
	void __FreeRead();
	void __FreeWrite();
	int __ReadInit();
	int __MakeSureHeaderRight(int totallen);
	int __ReadNoneBlock();
	int __AdjustTotalLen();
	int __ReadFillComm();

	int __AddWriteComm(sdk_client_comm_t* pComm);
	int __WriteNoneBlock();
	
private:
	int m_Sock;
	int m_ReadInit;
	PkgRudpHeader m_ReadHeader;
	unsigned int m_ReadLen;
	unsigned int m_ReadTotalLen;
	unsigned int m_ReadHeaderLen;
	sdk_client_comm_t* m_pReadComm;
	struct sockaddr_in m_RcvAddr;
	unsigned int m_HasRcvAddr;

	int m_WriteInit;
	PkgRudpHeader m_WriteHeader;
	unsigned int m_WriteLen;
	unsigned int m_WriteTotalLen;
	unsigned int m_WriteHeaderLen;
	sdk_client_comm_t* m_pWriteComm;
	
};

#endif /*__SDK_CLIENT_SOCK_H__*/


