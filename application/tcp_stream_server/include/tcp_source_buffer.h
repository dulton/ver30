
#ifndef  __TCP_SOURCE_DATA_H__
#define __TCP_SOURCE_DATA_H__

#include <tcp_source_stream.h>


typedef struct
{
	unsigned int m_DataNumber;
	int m_DataType;
	void* m_pData;	
	int m_DataLen;
} stream_data_info_t;


typedef struct
{
	int m_ClientFd;
	int m_DataIdx;
	unsigned int m_SourceIdx;
	int m_DataOffset;
	int m_ClientState;
	stream_data_info_t* m_pCopyData;
}client_data_info_t;



typedef enum
{
	stream_end_state = 0,              /*not running */
	stream_i_frame_state =1,        /*it must be i-frame */
	stream_seq_state = 2,              /* it can be either i-frame or p-frame ,but be sequence*/
} enum_tcp_source_buffer_state_t;

typedef enum
{
	client_start_state  =0 ,
	client_i_frame_state = 1,
	client_block_state = 2,
	client_seq_state = 3,
} enum_client_data_info_state_t;

class TcpSourceBuffer
{
public:
	TcpSourceBuffer(int maxpack,int maxclients,int streamid);
	~TcpSourceBuffer();

	int GetStreamFormat(format_data_t *pFormat);

	int InitStream();

	/***********************************
	to pull data from the stream source ,
	return value 
	<0 error code
	0 no data pull
	>0 for data pulled 
	   if >=2  it seems the number of clients to reset getting data
	   see NotifyFds array , this will
	   set fd client to reset to running
	***********************************/
	int PullData(int *pNotifyFds,int maxfds);
	/***********************************
	to clear all the data reference to the fd,this nothing return
	***********************************/
	void ClearData(int fd);
	/***********************************
	get data from fd
	<0 error code
	1 for has data
	0 for not has data
	***********************************/
	int  GetData(int fd,void** ppData,int& datalen);
	/***********************************
	forward data ,return value
	<0 error code
	1 for can read more data
	0 for nothing to read
	***********************************/
	int ForwardData(int fd,void* pData,int pushlen);
	/***********************************
	<0 error code
	set the fd into the register
	***********************************/
	int StartData(int fd);
	/***********************************
	number of clients that used this stream
	***********************************/
	int GetClients();
private:
	int __ExpandClients(int fd);
	void __ShrinkClients(int fd);
	void __CloseStream();
	int __InitStream();
	int __ResetIFrameState();
	int __ResetFdIFrame(int fd);
	int __InsertFrame(void* pData,int datalen,int type,int idx);
	int __PullDataSeq();
	int __PullDataIFrame();
	int __PullDataStart();
	int __GetNotifiedFds(enum_tcp_source_buffer_state_t oldstate,int *pNotifyFds,int maxfds);
	int __IsStreamDataUsed(int pos);
	int __CopyStreamDataUsed(int pos);
	unsigned int __GetDataNumber(int pos);
	int __WakeupAllSocks(int *pNotifyFds,int maxfds);
	int __GetDataStartState(client_data_info_t* pCData,void** ppData,int& datalen);
	int __GetDataBlockState(client_data_info_t* pCData,void** ppData,int& datalen);
	int __GetDataSeqState(client_data_info_t* pCData,void** ppData,int& datalen);
	int __GetDataIFrameState(client_data_info_t* pCData,void** ppData,int& datalen);
	int __ForwardDataIFrameState(client_data_info_t* pCData,void* pData,int pushlen);
	int __ForwardDataSeqState(client_data_info_t* pCData,void* pData,int pushlen);
	int __ForwardDataBlockState(client_data_info_t* pCData,void* pData,int pushlen);

private:
	int m_StreamId;
	int m_MaxPacks;
	int m_MaxClients;
	enum_tcp_source_buffer_state_t m_State;
	TcpSourceStream *m_pStream;
	stream_data_info_t **m_pStreamData;
	int m_LastIFrame;
	int m_StartIdx;
	int m_EndIdx;
	int m_BufferLength;
	unsigned int m_StartNumber;
	unsigned int m_EndNumber;
	int m_ClientsNumber;
	client_data_info_t **m_pClientDataInfo;	
};

#endif  /*__TCP_SOURCE_DATA_H__*/

