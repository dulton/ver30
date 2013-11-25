
#ifndef __SDK_AUDIO_DBUFFER_H__
#define __SDK_AUDIO_DBUFFER_H__

#include <sdk_audio_dec.h>


class SdkAudioDBuffer
{
public:
    SdkAudioDBuffer(int maxpacks,int *pRunningBits=NULL);
	~SdkAudioDBuffer();
	int StartStream(AudioDecParam *pAudioDec);
	void StopStream();
	int PauseStream();
	int ResumeStream(AudioDecParam *pAudioDec);
	int RegisterSock(int sock);
	int UnregisterSock(int sock);
	int PushDecodeData(int sock,sdk_client_comm_t*& pComm);
	int GetClients(std::vector<int>& clients);

private:
	/*functions*/
	int __StartStream(AudioDecParam *pAudioDec);
	void __StopStream();
	int __PauseStream();
	int __ResumeStream(AudioDecParam *pAudioDec);
private:
	/*data members*/
	unsigned int m_MaxPacks;
	int *m_pRunningBits;
	SdkAudioDec* m_pAudioDec;
	int m_RegisterSock;
	int m_StreamStarted;
	
};

#endif /*__SDK_AUDIO_DBUFFER_H__*/

