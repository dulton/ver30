
#ifndef __SDK_SERVER_SOCK_H__
#define __SDK_SERVER_SOCK_H__

#include <sdk_client_comm.h>





class SdkServerSock
{
public:
	SdkServerSock(int sock);
	~SdkServerSock();
	/*is something write*/
	int IsWriteSth();
	/*return 1 means read all ,can getread ,0 for not read*/
	int Read();
	/******************************
	*
	* 1 for read 0 for none read negative error code
	******************************/
	sdk_client_comm_t* GetRead();	
	void ClearRead();
	void ClearWrite();
	int PutData(sdk_client_comm_t* pComm);
	int PutDirectData(unsigned char* pData,int datalen);
	int Write();
	int GetSocket();

private:
	void __FreeRead();
	void __FreeWrite();
	int __ReadInit();
	int __ReadNoneBlock(unsigned char* pData,int datalen);
	int __AdjustTotalLen();
	int __ReadFillComm();

	int __AddWriteComm(sdk_client_comm_t* pComm);
	int __WriteNoneBlock();
	int __AddDirectWrite(unsigned char* pData,int datalen);
	
private:
	int m_Sock;
	int m_ReadInit;
	int m_ReadLen;
	int m_ReadTotalLen;
	int m_ReadHeaderLen;
	uint8_t m_ReadGSSP[GSSP_HEADER_MAX_LEN];	
	sdk_client_comm_t* m_pReadComm;
	int m_WriteInit;
	int m_WriteLen;
	int m_WriteTotalLen;
	int m_WriteHeaderLen;
	uint8_t m_WriteGSSP[GSSP_HEADER_MAX_LEN];
	sdk_client_comm_t* m_pWriteComm;
	
};

#endif /*__SDK_SERVER_SOCK_H__*/

