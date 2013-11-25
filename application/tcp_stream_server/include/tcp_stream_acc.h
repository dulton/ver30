
#ifndef __TCP_STREAM_ACC_H__
#define __TCP_STREAM_ACC_H__


#include <ev.h>
#include <tcp_stream_data.h>
#include <tcp_source_buffer.h>
#include <stdlib.h>
#include <stdio.h>


class TcpStreamData;
class TcpSourceBuffer;
class TcpStreamAcc
{
public:
	TcpStreamAcc(int port,int *pRunningBits=NULL,int maxclients=10,int maxpackets=10);
	~TcpStreamAcc();
	int RunLoop(void);
	void FreeTcpStreamData(TcpStreamData* pStreamData);
	int AllocateStreamSource(TcpStreamData* pStreamData,int streamid);
private:
	static void __ReadStreamSource(EV_P_ ev_timer *w, int revents,void* arg);
	int __ReadStreamSourceImpl();
	static void __ReadTimeout(EV_P_ ev_timer *w, int revents,void* arg);
	static void __AcceptStreamClient(EV_P_ ev_io *w, int revents,void* arg);
	int __AcceptStreamClientImpl();
	static void __SigTimer(EV_P_ ev_timer *w, int revents,void* arg);
	

	void __ResetAcc();
	int __SetSocketUnBlock(int sock);
	int __BindPort();
	int __InitSigTimer();
	void __StopSigTimer();
	int __InitTimer();
	int __ResetReadTimer(void);
	void __StopTimer();
	int __ResetTimeoutTimer();
	int __InitAccIo();
	void __BreakRunLoop();
	void __TestRunBits();
	int __ExpandStreamData(TcpStreamData* pData);
	void __ShrinkStreamData(TcpStreamData* pData);
	int __InitStreamSource();
private:	
	int *m_pRunbits;
	int m_Port;
	int m_MaxClients;
	int m_MaxPackets;
	int m_AccSock;
	int m_HasInsertTimer;
	ev_timer  m_EvTimerRead;
	int m_HasInsertTimerOut;
	ev_timer m_EvTimerOut;
	int m_HasInsertIO;
	ev_io  m_EvIoAcc;
	int m_HasInsertSigTimer;
	ev_timer m_EvTimerSig;
	int m_Streams;
	TcpSourceBuffer **m_pSourceBufferArray;
	int m_StreamDatas;
	TcpStreamData **m_pStreamDataArray;
	
};


#endif   /*__TCP_STREAM_ACC_H__*/


