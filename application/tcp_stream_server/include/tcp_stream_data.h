
#ifndef __TCP_STREAM_DATA_H__
#define __TCP_STREAM_DATA_H__


#include <tcp_stream_acc.h>
#include <tcp_source_buffer.h>

typedef enum 
{
	stop_state  = 0,
	read_state  = 1,      /*this is for the read state of the streamid*/
	response_state = 2,   /*this is for the write response of the peer*/
	write_length_state = 3,     /*write length of data*/
	write_data_state = 4,        /*write data state*/
} tcp_stream_data_state_t;


class TcpStreamAcc;

class TcpStreamData
{
public:
	TcpStreamData(int sock,TcpStreamAcc* pAcc);
	~TcpStreamData();
	int StartData();
	int ResetToRunning(void);
	int GetSocket();
	int GetStreamId();
	int SetSourceBuffer(TcpSourceBuffer* pBuffer);

private:
	
	static void __WriteSocket(EV_P_ ev_io *w, int revents,void* arg);
	int __WriteSocketImpl();
	static void __ReadSocket(EV_P_ ev_io *w, int revents,void* arg);
	int __ReadSocketImpl();
	static void __Timeout(EV_P_ ev_timer *w, int revents,void* arg);
	void __Reset();
	int __StartReadIo();
	void __StopReadIo();
	int __StartTimer();
	void __StopTimer();
	int __StartWriteIo();
	void __StopWriteIo();
	int __ResetTimer();
	/*return value :
	    <0 for error
	    >0 for write length
	*/
	int __WriteNonBlock(void* pData,int size);
private:
	int m_Sock;
	int m_StreamId;
	tcp_stream_data_state_t m_State;
	int m_LengthSendSize;
	int m_HasInsertTimer;
	ev_timer m_EvTimer;
	int m_HasInsertWrite;
	ev_io m_EvIoWrite;
	int m_HasInsertRead;
	ev_io m_EvIoRead;
	TcpStreamAcc *m_pStreamAcc;
	TcpSourceBuffer *m_pSourceBuffer;
};

#endif /*__TCP_STREAM_DATA_H__*/
