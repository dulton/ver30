
#ifndef __SDK_SERVER_STREAM_H__
#define __SDK_SERVER_STREAM_H__


#define  STREAM_PULL_MODE  1
#undef   STREAM_PUSH_MODE

#if defined(STREAM_PULL_MODE) && defined(STREAM_PUSH_MODE)
#error "can not set STREAM_PULL_MODE and STREAM_PUSH_MODE both"
#elif  !( defined(STREAM_PULL_MODE) || defined(STREAM_PUSH_MODE))
#error "must specify one of STREAM_PULL_MODE or STREAM_PUSH_MODE"
#endif


#include <ipc_media_data_client.h>
#include <sys_stream_info.h>


class SdkServerStream
{
public:
	SdkServerStream(int streamid,int type,int maxpacks=10);
	~SdkServerStream();
	int StartStream();
	/*0 for already stopped ,1 last time has stream running*/
	int StopStream();

	int ResumeStream();
	int PauseStream();

	/*1 for get data ,0 for nothing get negative error code*/
	int PullStreamData(stream_pack_t* pPack);

private:
	void __ClearVectors();
#ifdef VIDEO_STREAM_EMULATE
#else
	int __GetServerPort();
	int __GetClientStartPort();
	int __InitIPC();
	void __ClearIPC();
#endif	/*VIDEO_STREAM_EMULATE*/
	int __PushStreamData(void* pData,uint32_t datalen,uint32_t datatype,uint32_t idx,uint64_t pts);
#ifdef STREAM_PULL_MODE
	static void* ThreadFunc(void* arg);
	void* ThreadImpl();
#endif	
private:
	int m_StreamId;
	int m_Type;
	unsigned int m_MaxPacks;
	GMI_Mutex m_Mutex;
#ifdef VIDEO_STREAM_EMULATE
#else
	int m_Initialized;
	int m_Registered;
	IPC_MediaDataClient m_IPCClient;
#endif /*VIDEO_STREAM_EMULATE*/
	std::vector<void*> m_DataVec;
	std::vector<int> m_DataLen;
	std::vector<uint32_t> m_DataType;
	std::vector<uint32_t> m_DataIdx;
	std::vector<uint64_t> m_DataPts;
#ifdef STREAM_PULL_MODE
	int m_ThreadRunning;
	int m_ThreadExited;
	GMI_Thread *m_pThread;
#endif
	
};

#endif /*__SDK_SERVER_STREAM_H__*/


