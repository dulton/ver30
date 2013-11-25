
#include <sdk_audio_dbuffer.h>
#include <sdk_server_debug.h>

SdkAudioDBuffer::SdkAudioDBuffer(int maxpacks,int * pRunningBits) :
    m_MaxPacks(maxpacks),
    m_pRunningBits(pRunningBits)
{
    m_pAudioDec = NULL;
    m_RegisterSock = -1;
    m_StreamStarted = 0;
}


void SdkAudioDBuffer::__StopStream()
{
    if(this->m_pAudioDec)
    {
        delete this->m_pAudioDec;
    }
    this->m_pAudioDec = NULL;

    this->m_RegisterSock = -1;
    this->m_StreamStarted = 0;

    return ;
}

void SdkAudioDBuffer::StopStream()
{
    return this->__StopStream();
}

SdkAudioDBuffer::~SdkAudioDBuffer()
{
    this->StopStream();
    this->m_pRunningBits = NULL;
    this->m_MaxPacks = 0;
}

int SdkAudioDBuffer::__StartStream(AudioDecParam * pAudioDec)
{
    int ret;

    SDK_ASSERT(this->m_pAudioDec == NULL);

    this->m_pAudioDec = new SdkAudioDec(this->m_MaxPacks,this->m_pRunningBits);

    ret =  this->m_pAudioDec->StartStream(pAudioDec);
    if(ret < 0)
    {
        this->__StopStream();
        return ret;
    }

    this->m_StreamStarted = 1;
    return 0;
}

int SdkAudioDBuffer::StartStream(AudioDecParam * pAudioDec)
{
    this->__StopStream();
    return this->__StartStream(pAudioDec);
}

int SdkAudioDBuffer::__PauseStream()
{

	SDK_ASSERT(this->m_pAudioDec);
    return this->m_pAudioDec->PauseStream();
}

int SdkAudioDBuffer::__ResumeStream(AudioDecParam * pAudioDec)
{
    SDK_ASSERT(this->m_pAudioDec);
    return this->m_pAudioDec->ResumeStream(pAudioDec);
}

int SdkAudioDBuffer::PauseStream()
{
    if(this->m_StreamStarted == 0)
    {
        return -EPERM;
    }
    return this->__PauseStream();
}

int SdkAudioDBuffer::ResumeStream(AudioDecParam * pAudioDec)
{
	if (this->m_StreamStarted == 0)
	{
		return -EPERM;
	}
    return this->__ResumeStream(pAudioDec);
}


int SdkAudioDBuffer::RegisterSock(int sock)
{
	if (this->m_RegisterSock >= 0)
	{
		return -EEXIST;
	}
	if (this->m_StreamStarted == 0)
	{
		return -ENODEV;
	}
	this->m_RegisterSock = sock;
	return 0; 
}

int SdkAudioDBuffer::UnregisterSock(int sock)
{
	int ret=-EFAULT;
	if (this->m_RegisterSock == sock)
	{
		this->m_RegisterSock = -1;
		ret = 0;
	}
	return ret;
}

int SdkAudioDBuffer::PushDecodeData(int sock,sdk_client_comm_t * & pComm)
{
	/*now we should test */
	if (sock != this->m_RegisterSock)
	{
		return -EINVAL;
	}

	if (this->m_StreamStarted == 0)
	{
		return -EPERM;
	}

	SDK_ASSERT(this->m_pAudioDec);
	return this->m_pAudioDec->PushAudioDec(pComm);
}

int SdkAudioDBuffer::GetClients( std::vector<int>& clients)
{
	
	clients.clear();
	if (this->m_RegisterSock < 0)
	{
		return 0;
	}

	clients.push_back(this->m_RegisterSock);
	return 1;
}
