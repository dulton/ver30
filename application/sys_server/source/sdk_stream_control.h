#ifndef __SDK_STREAM_CONTROL_H__
#define __SDK_STREAM_CONTROL_H__
#include "sys_env_types.h"
#include "gmi_system_headers.h"

#define SDK_STREAM_STATE_START     (2)
#define SDK_STREAM_STATE_STOP      (0)
#define SDK_STREAM_STATE_PAUSE     (1)

class SdkStreamControl
{
public:
	SdkStreamControl(uint16_t LocalPort, uint16_t SdkPort);
	~SdkStreamControl();
	GMI_RESULT Initialize();
	GMI_RESULT Deinitialize();	
	GMI_RESULT Query(int32_t Timeout, int32_t *Started);
	GMI_RESULT Start(SysPkgEncodeCfg *SysEncodeCfg, int32_t VideoCount, SysPkgAudioEncodeCfg *SysAudioCfg, int32_t AudioCount, int32_t Timeout);
	GMI_RESULT Stop(int32_t Timeout);
	GMI_RESULT Pause(int32_t Timeout);
	GMI_RESULT Resume(SysPkgEncodeCfg *SysEncodeCfg, int32_t VideoCount, SysPkgAudioEncodeCfg *SysAudioCfg, int32_t AudioCount, int32_t Timeout);
private:
	GMI_RESULT Lock();
	GMI_RESULT Unlock();
private:
	FD_HANDLE m_RUDP_Socket;
	uint16_t m_SDK_Port;
	uint16_t m_Local_Port;	
	GMI_Mutex m_SDK_Mutex;
};

#endif
