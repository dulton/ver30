#ifndef __ALARM_SESSION_H__
#define __ALARM_SESSION_H__
#include "gmi_system_headers.h"

class AlarmSession
{
public:
	AlarmSession(uint16_t LocalPort, uint16_t SdkPort);
	~AlarmSession();
	GMI_RESULT Initialize();
	GMI_RESULT Deinitialize();	
	GMI_RESULT Send(const uint8_t *Buffer, size_t BufferSize, size_t *Transferred);	
private:
	FD_HANDLE m_RUDP_Socket;
	uint16_t m_LocalPort;
	uint16_t m_RemotePort;
	GMI_Mutex m_SDK_Mutex;
};

#endif

