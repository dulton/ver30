
#ifndef __SDK_AUDIO_BUFFER_H__
#define __SDK_AUDIO_BUFFER_H__

#include <sdk_audio_stream.h>
#include <sdk_client_comm.h>
#include <sdk_client_buffer.h>

class SdkAudioBuffer
{
public:
    SdkAudioBuffer(sdk_audio_format_enum_t format,int maxpacks=10,int maxclients=10,int* pRunningBits=NULL);
    ~SdkAudioBuffer();
    int StartStream(sys_stream_info_t* pStreamInfo);
    void StopStream();
    int ResumeStream(sys_stream_info_t* pStreamInfo);
    int PauseStream();
    int GetStreamStarted();
    int GetClients(std::vector<int>& clisocks);
    int RegisterSock(int sock,sessionid_t sesid);
    void UnRegisterSock(int sock);
    int GetStreamData(int sock,struct iovec* pIoVec,int& iovlen,int& begin);
    /*if 1 it is forward at the end of the packet ,if 0 ,it is success ,negative error code*/
    int ForwardStreamData(int sock,struct iovec* pIoVec,int iovlen,int forwardlen);
    int PullStreamData(std::vector<int>& notifysock);
    void DebugClientBuffer(int sock);

private:
	/*functions */
	void __FreeResource();
	void __FreeStreamPack(stream_pack_t*& pPack);
	client_buffer_t* __AllocateClientBuffer(int sock,sessionid_t sesid);
	void __InitClientBuffer(client_buffer_t* pCBuffer,int sock,sessionid_t sesid);
	void __FreeClientBuffer(client_buffer_t* pCBuffer);
	void __ClearClientBuffers(void);
	stream_pack_t* __CopyPack(stream_pack_t* pPack);
	void __ChangePendingState();
	int __CopyBlock(client_buffer_t* pCBuffer,stream_pack_t* pPack,enum_cb_state_t state);
	int __CopyAudioStreamInfo(SysPkgAudioEncodeCfg *pCfg,sys_stream_info_t* pStreamInfo);

	int __FormatMessageHeader(client_buffer_t* pCBuffer,stream_pack_t* pCBlock,unsigned int idx);
	int __SetIov(client_buffer_t* pCBuffer,struct iovec* pIoVec,int& iovlen,int& begin);
	int __IsGreaterIdx(unsigned int aidx,unsigned int bidx);
	
	int __GetStreamDataEndState(client_buffer_t* pCBuffer,struct iovec* pIoVec,int& iovlen,int& begin);
	int __GetStreamDataSeqState(client_buffer_t* pCBuffer,struct iovec* pIoVec,int& iovlen,int& begin);
	int __GetStreamDataBlockState(client_buffer_t* pCBuffer,struct iovec* pIoVec,int& iovlen,int& begin);
	int __GetStreamDataPendingState(client_buffer_t* pCBuffer,struct iovec* pIoVec,int& iovlen,int& begin);

	int __HandleClientInsert(client_buffer_t* pCBuffer,unsigned int insertidx);
	int __InsertPack(stream_pack_t* pPack);
	int __PullData();
	int __GetNotify(std::vector<int>& notifyfds);

	int __ForwardAssert(client_buffer_t* pCBuffer,struct iovec* pIoVec,int iovlen,int forwardlen);
	int __ForwardStreamDataInner(client_buffer_t* pCBuffer,struct iovec* pIoVec,int iovlen,int forwardlen);
	enum_cb_state_t __ChangeClientBufferState(client_buffer_t* pCBuffer,enum_cb_state_t state,const char* file=NULL,int lineno=0);	
	void __DebugCBuffer(client_buffer_t* pCBuffer);

private:
	/*data members*/
	sdk_audio_format_enum_t m_Format;
	unsigned int m_MaxClients;
	unsigned int m_MaxPacks;
	int *m_pRunningBits;
	int m_Started;
	int m_AudioInfoInited;
	SysPkgAudioEncodeCfg m_AudioInfo;
	stream_pack_t** m_pPacks;
	unsigned int m_StartIdx;
	unsigned int m_EndIdx;
	unsigned int m_PackSize;
	std::vector<client_buffer_t*> m_pClientBuffers;
	SdkAudioStream* m_pStream;
};

#endif /*__SDK_AUDIO_BUFFER_H__*/

