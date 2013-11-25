
#ifndef  __SDK_AUDIO_DEC_H__
#define  __SDK_AUDIO_DEC_H__


#include <ipc_media_data_dispatch/ipc_media_data_server.h>
#include <sdk_client_comm.h>
#include <media/gmi_media_ctrl.h>
#include <sys_stream_info.h>

class SdkAudioDec
{
public:
    SdkAudioDec(int maxpacks,int *pRunningBits=NULL);
    ~SdkAudioDec();
    int StartStream(AudioDecParam *pAudioDec);
    void StopStream();
    int ResumeStream(AudioDecParam* pAudioDec);
    int PauseStream();
    int PushAudioDec(sdk_client_comm_t*& pComm);

private:
    /*private functions*/
    static void* __ThreadFunc(void* arg);
    void* __ThreadImpl();
    int __GetBindPort();
    int __PauseStream();
    int __ResumeStream(AudioDecParam* pAudioDec);
    int __StartStream(AudioDecParam* pAudioDec);
    int __InitailizeDataServer(AudioDecParam* pAudioDec);
    void __DeInitializeDataServer();
    void __StopStream();
    void __ClearComms();
    int __PushAudioDec(sdk_client_comm_t*& pComm);
    int __GetAudioDec(sdk_client_comm_t*& pComm);
    int __WriteServerData(sdk_client_comm_t*& pComm);
    int __TransPtsToTm(uint64_t pts,struct timeval* pTm);

private:
    /*private data member*/
    unsigned int m_MaxPacks;
    int *m_pRunningBits;
    GMI_Mutex m_Mutex;
#ifdef NOT_MEDIA_DECODE_STREAM_PUT
    FILE* m_pFp;
#else
    int m_ServerInitialized;
    IPC_MediaDataServer *m_pDataServer;
#endif
    uint32_t m_LastFrameIdx;
    GMI_Thread *m_pThread;
    int m_ThreadRunning;
    int m_ThreadExited;
    std::vector<sdk_client_comm_t*> m_RecvComm;
    int m_StreamStarted;
};

#endif /*__SDK_AUDIO_DEC_H__*/

