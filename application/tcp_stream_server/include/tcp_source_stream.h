
#ifndef __TCP_SOURCE_STREAM_H__
#define __TCP_SOURCE_STREAM_H__

#include <gmi_system_headers.h>
#include <ipc_media_data_client.h>

#define  MAX_STREAM_IDS     4
typedef void* ptr_t;

#define  MAX_FRAME_IDX  0xffffffff
#define  START_FRAME_IDX  0xffffffff

#define  I_FRAME_TYPE   1
#define  P_FRAME_TYPE  0


#define STREAM_PULL_MODE      1
#undef  STREAM_PUSH_MODE

#if defined(STREAM_PULL_MODE) && defined(STREAM_PUSH_MODE)
#error "can not set STREAM_PULL_MODE and STREAM_PUSH_MODE both"
#elif  !( defined(STREAM_PULL_MODE) || defined(STREAM_PUSH_MODE))
#error "must specify one of STREAM_PULL_MODE or STREAM_PUSH_MODE"
#endif

typedef struct
{
	unsigned char m_Type;
	unsigned short m_Width;
	unsigned short m_Height;
} format_data_t;


class TcpSourceStream
{
public:
	TcpSourceStream(int streamid,int maxpacks=3);
	~TcpSourceStream();
	int Start();
	int Stop();
	/***********************************
	to get the next stream 
	return value :
	< 0 for error code  espeically for -ENOSPC for size is less than expected
	          and *pSize store the really size to handle
	== 0 for nothing data read
	>0 for data filled into the pData	

	when 
	*pIdx is the index return get the data
	*pType return the type of stream
	***********************************/
	int GetNextStream(int *pIdx,void** ppData,int *pSize,int *pType);
	int GetSFormat(format_data_t *pFormat);

private:
	int __GetServerPort();
	int __GetClientStartPort();
#ifdef STREAM_PUSH_MODE
	static void StreamCallBack(void_t *UserData, uint32_t MediaType, uint32_t MediaId, const void_t *Frame, size_t FrameSize, const struct timeval *FramTS, const void_t *ExtraData, size_t ExtraDataLength );
	int __StreamCallBackImpl(uint32_t MediaType, uint32_t MediaId, const void_t *Frame, size_t FrameSize, const struct timeval *FramTS, const void_t *ExtraData, size_t ExtraDataLength );
#endif
#ifdef STREAM_PULL_MODE
	int __InitResource(void);
	int __ClearResource(void);
	static void* StreamPullCallBack(void* arg);
	int __StreamPullCallBackImpl();
#endif

	int __PushBack(void* pData,int datalen,int type,unsigned int idx);
	void __ClearVector(void);
	int __GetNextStream(int *pIdx,void** ppData,int *pSize,int *pType);

private:
	int m_StreamId;
	unsigned int m_MaxPacks;
	GMI_Mutex m_Mutex;
	int m_Initialized;
	int m_Registered;
	IPC_MediaDataClient m_IPCClient;
	std::vector<void*> m_DataVec;
	std::vector<int> m_DataLen;
	std::vector<int> m_DataType;
	std::vector<unsigned int> m_DataIdx;
#ifdef STREAM_PULL_MODE
	int m_ThreadRunning;
	int m_ThreadExited;
	GMI_Thread *m_pThread;
#endif
};

#endif /*__TCP_SOURCE_STREAM_H__*/

