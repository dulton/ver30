
#ifndef __SDK_CLIENT_UNIX_SOCK_H__
#define __SDK_CLIENT_UNIX_SOCK_H__

#include <sdk_client_comm.h>
#include <gmi_rudp.h>

class SdkClientUnixSock
{
public:
    SdkClientUnixSock(int sock);
    ~SdkClientUnixSock();
    void ClearRead();
    void ClearWrite();
    int PushData(sdk_client_comm_t* pComm);
    sdk_client_comm_t* GetRead();
    int Read();
    int Write();
    int Socket();

private:
	int __AddWriteComm(sdk_client_comm_t * pComm);
	int __ReadInit();
	int __ReadFillHeader();
	int __ReadFillComm();
	int __ReadNoneBlock();
	int __WriteNoneBlock();
private:
    int m_Sock;
	int m_ReadInit;
	PkgRudpHeader m_ReadHeader;
	unsigned int m_ReadLen;
	unsigned int m_ReadTotalLen;
	unsigned int m_ReadHeaderLen;
	sdk_client_comm_t* m_pReadComm;

	int m_WriteInit;
	PkgRudpHeader m_WriteHeader;
	unsigned int m_WriteLen;
	unsigned int m_WriteTotalLen;
	unsigned int m_WriteHeaderLen;
	sdk_client_comm_t* m_pWriteComm;	
};

#endif /*__SDK_CLIENT_UNIX_SOCK_H__*/

