
#ifndef __SDK_AUDIO_STREAM_H__
#define __SDK_AUDIO_STREAM_H__


#include <sys_stream_info.h>
#include <ipc_media_data_client.h>

typedef enum
{
    audio_format_default = 0,
    audio_format_g711 = 1,
} sdk_audio_format_enum_t;

#define AUDIO_PULL_MODE   1



class SdkAudioStream
{
public:
    SdkAudioStream(sdk_audio_format_enum_t format,int maxpackets,int *pRunningBits=NULL);
    ~SdkAudioStream();
    int StartStream();
    /*0 for already stopped ,1 last time has stream running*/
    int StopStream();

    int ResumeStream();
    int PauseStream();

    /*1 for get data ,0 for nothing get negative error code*/
    int PullStreamData(stream_pack_t* pPack);
private:
    /*functions*/
    void __ClearVectors();
#ifdef AUDIO_DUAL_FILE_EMULATE
#else
    int __GetServerPort();
    int __GetClientStartPort();
    int __InitIPC();
    void __ClearIPC();
#endif	
    int __PushStreamData(void* pData,uint32_t datalen,uint32_t idx,uint64_t pts);
#ifdef AUDIO_PULL_MODE
    static void* ThreadFunc(void* arg);
    void* __ThreadImpl();
#endif

private:
    /*data members*/
	sdk_audio_format_enum_t m_Format;
    unsigned int m_MaxPackets;
    int* m_pRunningBits;
    GMI_Mutex m_Mutex;
#ifdef AUDIO_DUAL_FILE_EMULATE
#else
    int m_Initialized;
    int m_Registered;
    IPC_MediaDataClient m_IPCClient;
#endif /*AUDIO_DUAL_FILE_EMULATE*/	
    std::vector<void*> m_DataVec;
    std::vector<unsigned int>  m_DataLenVec;
    std::vector<unsigned int>  m_FrameNumVec;
    std::vector<unsigned long long> m_PtsVec;
#ifdef AUDIO_PULL_MODE
    int m_ThreadRunning;
    int m_ThreadExited;
    GMI_Thread *m_pThread;
#endif
};


#endif

